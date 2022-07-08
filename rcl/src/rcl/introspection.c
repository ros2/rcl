// Copyright 2022 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef __cplusplus
extern "C" {
#endif

#include "rcl/introspection.h"

#include <string.h>

#include "./client_impl.h"
#include "./service_impl.h"
#include "builtin_interfaces/msg/time.h"
#include "dlfcn.h"
#include "rcl/error_handling.h"
#include "rcl_interfaces/msg/service_event.h"
#include "rcl_interfaces/msg/service_event_info.h"
#include "rcl_interfaces/msg/service_event_type.h"
#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"
#include "rcutils/shared_library.h"
#include "rmw/error_handling.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_typesupport_c/type_support_map.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"
#include "unique_identifier_msgs/msg/uuid.h"

rcl_service_introspection_utils_t rcl_get_zero_initialized_introspection_utils()
{
  static rcl_service_introspection_utils_t null_introspection_utils = {
    .response_type_support = NULL,
    .request_type_support = NULL,
    .publisher = NULL,
    .clock = NULL,
    .service_name = NULL,
    .service_type_name = NULL,
    .service_event_topic_name = NULL,
    ._enabled = true,
    ._content_enabled = true,
  };
  return null_introspection_utils;
}

rcl_ret_t rcl_service_typesupport_to_message_typesupport(
  const rosidl_service_type_support_t * service_typesupport,
  rosidl_message_type_support_t ** request_typesupport,
  rosidl_message_type_support_t ** response_typesupport, const rcl_allocator_t * allocator)
{
  rcutils_ret_t ret;
  type_support_map_t * map = (type_support_map_t *)service_typesupport->data;
  // TODO(ihasdapie): change to #define or some constant
  const char * typesupport_library_fmt = "lib%s__rosidl_typesupport_c.so";
  const char * service_message_fmt =  // package_name, type name, Request/Response
    "rosidl_typesupport_c__get_message_type_support_handle__%s__srv__%s_%s";

  const char * service_type_name = rcl_service_get_service_type_name(service_typesupport);

  // build out typesupport library and symbol names
  char * typesupport_library_name = allocator->allocate(
    sizeof(char) * ((strlen(typesupport_library_fmt) - 2) + strlen(map->package_name) + 1),
    allocator->state);
  char * request_message_symbol = allocator->allocate(
    sizeof(char) * ((strlen(service_message_fmt) - 6) + strlen(map->package_name) +
                    strlen(service_type_name) + strlen("Request") + 1),
    allocator->state);
  char * response_message_symbol = allocator->allocate(
    sizeof(char) * ((strlen(service_message_fmt) - 6) + strlen(map->package_name) +
                    strlen(service_type_name) + strlen("Request") + 1),
    allocator->state);

  sprintf(typesupport_library_name, typesupport_library_fmt, map->package_name);
  sprintf(
    request_message_symbol, service_message_fmt, map->package_name, service_type_name, "Request");
  sprintf(
    response_message_symbol, service_message_fmt, map->package_name, service_type_name, "Response");

  rcutils_shared_library_t typesupport_library = rcutils_get_zero_initialized_shared_library();
  ret = rcutils_load_shared_library(&typesupport_library, typesupport_library_name, *allocator);
  if (RCUTILS_RET_OK != ret) {
    return RCL_RET_ERROR;
  }

  rosidl_message_type_support_t * (*req_typesupport_func_handle)() =
    (rosidl_message_type_support_t * (*)())
      rcutils_get_symbol(&typesupport_library, request_message_symbol);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    req_typesupport_func_handle, "Looking up request type support failed", return RCL_RET_ERROR);

  rosidl_message_type_support_t * (*resp_typesupport_func_handle)() =
    (rosidl_message_type_support_t * (*)())
      rcutils_get_symbol(&typesupport_library, response_message_symbol);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    resp_typesupport_func_handle, "Looking up response type support failed", return RCL_RET_ERROR);

  *request_typesupport = req_typesupport_func_handle();
  *response_typesupport = resp_typesupport_func_handle();
  allocator->deallocate(typesupport_library_name, allocator->state);
  allocator->deallocate(request_message_symbol, allocator->state);
  allocator->deallocate(response_message_symbol, allocator->state);

  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_init(
  rcl_service_introspection_utils_t * introspection_utils,
  const rosidl_service_type_support_t * service_type_support, const char * service_name,
  const rcl_node_t * node, rcl_allocator_t * allocator)
{
  rcl_ret_t ret;

  introspection_utils->service_name =
    allocator->allocate(strlen(service_name) + 1, allocator->state);
  strcpy(introspection_utils->service_name, service_name);

  const char * service_type_name = rcl_service_get_service_type_name(service_type_support);
  introspection_utils->service_type_name =
    allocator->allocate(strlen(service_type_name) + 1, allocator->state);
  strcpy(introspection_utils->service_type_name, service_type_name);

  rcl_service_typesupport_to_message_typesupport(
    service_type_support, &introspection_utils->request_type_support,
    &introspection_utils->response_type_support, allocator);

  // Make a publisher
  char * service_event_topic_name = (char *)allocator->zero_allocate(
    strlen(service_name) + strlen(RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX) + 1, sizeof(char),
    allocator->state);
  strcpy(service_event_topic_name, service_name);
  strcat(service_event_topic_name, RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX);

  introspection_utils->service_event_topic_name =
    allocator->allocate(strlen(service_event_topic_name) + 1, allocator->state);

  strcpy(introspection_utils->service_event_topic_name, service_event_topic_name);

  introspection_utils->publisher = allocator->allocate(sizeof(rcl_publisher_t), allocator->state);
  *introspection_utils->publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * service_event_typesupport =
    ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, ServiceEvent);

  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(
    introspection_utils->publisher, node, service_event_typesupport, service_event_topic_name,
    &publisher_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  // make a clock
  introspection_utils->clock = allocator->allocate(sizeof(rcl_clock_t), allocator->state);
  ret = rcl_clock_init(RCL_STEADY_TIME, introspection_utils->clock, allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  allocator->deallocate(service_event_topic_name, allocator->state);

  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_fini(
  rcl_service_introspection_utils_t * introspection_utils, rcl_allocator_t * allocator,
  rcl_node_t * node)
{
  rcl_ret_t ret;
  ret = rcl_publisher_fini(introspection_utils->publisher, node);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }
  ret = rcl_clock_fini(introspection_utils->clock);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  allocator->deallocate(introspection_utils->publisher, allocator->state);
  allocator->deallocate(introspection_utils->clock, allocator->state);
  allocator->deallocate(introspection_utils->service_name, allocator->state);
  allocator->deallocate(introspection_utils->service_event_topic_name, allocator->state);
  allocator->deallocate(introspection_utils->service_type_name, allocator->state);

  introspection_utils->publisher = NULL;
  introspection_utils->clock = NULL;
  introspection_utils->service_name = NULL;
  introspection_utils->service_event_topic_name = NULL;
  introspection_utils->service_type_name = NULL;

  return RCL_RET_OK;
}

rcl_ret_t rcl_introspection_send_message(
  const rcl_service_introspection_utils_t * introspection_utils, const uint8_t event_type,
  const void * ros_response_request, const int64_t sequence_number, const uint8_t uuid[16],
  const rcl_allocator_t * allocator)
{
  // Early exit of service introspection if it isn't enabled
  // TODO(ihasdapie): Different return code?
  if (!introspection_utils->_enabled) return RCL_RET_OK;

  rcl_ret_t ret;

  rcl_interfaces__msg__ServiceEvent msg;
  rcl_interfaces__msg__ServiceEvent__init(&msg);

  rcl_interfaces__msg__ServiceEventInfo msg_info;
  rcl_interfaces__msg__ServiceEventInfo__init(&msg_info);

  rcl_serialized_message_t serialized_message = rmw_get_zero_initialized_serialized_message();
  if (introspection_utils->_content_enabled) {
    // build serialized message
    ret = rmw_serialized_message_init(&serialized_message, 0U, allocator);
    rosidl_message_type_support_t * serialized_message_ts;
    switch (event_type) {
      case rcl_interfaces__msg__ServiceEventType__REQUEST_SENT:
      case rcl_interfaces__msg__ServiceEventType__REQUEST_RECEIVED:
        serialized_message_ts = introspection_utils->request_type_support;
        break;
      case rcl_interfaces__msg__ServiceEventType__RESPONSE_SENT:
      case rcl_interfaces__msg__ServiceEventType__RESPONSE_RECEIVED:
        serialized_message_ts = introspection_utils->response_type_support;
        break;
      default:
        RCL_SET_ERROR_MSG("Invalid event type");
        return RCL_RET_ERROR;
    }

    ret = rmw_serialize(ros_response_request, serialized_message_ts, &serialized_message);
    if (RMW_RET_OK != ret) {
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      return RCL_RET_ERROR;
    }
    rosidl_runtime_c__octet__Sequence__init(
      &msg.serialized_event, serialized_message.buffer_length);
    memcpy(msg.serialized_event.data, serialized_message.buffer, serialized_message.buffer_length);
  }

  // populate info message
  msg_info.event_type = event_type;
  msg_info.sequence_number = sequence_number;
  rosidl_runtime_c__String__assign(&msg_info.service_name, introspection_utils->service_name);
  rosidl_runtime_c__String__assign(&msg_info.event_name, introspection_utils->service_type_name);

  builtin_interfaces__msg__Time timestamp;
  builtin_interfaces__msg__Time__init(&timestamp);

  rcl_time_point_value_t now;
  ret = rcl_clock_get_now(introspection_utils->clock, &now);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  timestamp.sec = RCL_NS_TO_S(now);
  timestamp.nanosec = now % (1000LL * 1000LL * 1000LL);
  msg_info.stamp = timestamp;

  unique_identifier_msgs__msg__UUID uuid_msg;
  unique_identifier_msgs__msg__UUID__init(&uuid_msg);
  memcpy(uuid_msg.uuid, uuid, 16 * sizeof(uint8_t));
  msg_info.client_id = uuid_msg;

  msg.info = msg_info;

  // and publish it out!
  ret = rcl_publish(introspection_utils->publisher, &msg, NULL);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  // clean up
  if (introspection_utils->_content_enabled) {
    ret = rmw_serialized_message_fini(&serialized_message);
    if (RMW_RET_OK != ret) {
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      return RCL_RET_ERROR;
    }
  }

  rcl_interfaces__msg__ServiceEvent__fini(&msg);
  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_enable(
  rcl_service_introspection_utils_t * introspection_utils, const rcl_node_t * node,
  rcl_allocator_t * allocator)
{
  rcl_ret_t ret;

  introspection_utils->publisher = allocator->allocate(sizeof(rcl_publisher_t), allocator->state);
  *introspection_utils->publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * service_event_typesupport =
    ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, ServiceEvent);

  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(
    introspection_utils->publisher, node, service_event_typesupport,
    introspection_utils->service_event_topic_name, &publisher_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  introspection_utils->clock = allocator->allocate(sizeof(rcl_clock_t), allocator->state);
  ret = rcl_clock_init(RCL_STEADY_TIME, introspection_utils->clock, allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  introspection_utils->_enabled = true;
  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_disable(
  rcl_service_introspection_utils_t * introspection_utils, rcl_node_t * node,
  const rcl_allocator_t * allocator)
{
  rcl_ret_t ret;
  ret = rcl_publisher_fini(introspection_utils->publisher, node);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  ret = rcl_clock_fini(introspection_utils->clock);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  allocator->deallocate(introspection_utils->publisher, allocator->state);
  allocator->deallocate(introspection_utils->clock, allocator->state);
  introspection_utils->publisher = NULL;
  introspection_utils->clock = NULL;

  introspection_utils->_enabled = false;
  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_enable_service_events(
  rcl_service_t * service, rcl_node_t * node)
{
  rcl_service_introspection_utils_t * introspection_utils = service->impl->introspection_utils;
  rcl_ret_t ret =
    rcl_service_introspection_enable(introspection_utils, node, &service->impl->options.allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_disable_service_events(
  rcl_service_t * service, rcl_node_t * node)
{
  rcl_service_introspection_utils_t * introspection_utils = service->impl->introspection_utils;
  rcl_ret_t ret =
    rcl_service_introspection_disable(introspection_utils, node, &service->impl->options.allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_enable_client_events(rcl_client_t * client, rcl_node_t * node)
{
  rcl_service_introspection_utils_t * introspection_utils = client->impl->introspection_utils;
  rcl_ret_t ret =
    rcl_service_introspection_enable(introspection_utils, node, &client->impl->options.allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_disable_client_events(rcl_client_t * client, rcl_node_t * node)
{
  rcl_service_introspection_utils_t * introspection_utils = client->impl->introspection_utils;
  rcl_ret_t ret =
    rcl_service_introspection_disable(introspection_utils, node, &client->impl->options.allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

void rcl_service_introspection_enable_client_content(rcl_client_t * client)
{
  client->impl->introspection_utils->_content_enabled = true;
}

void rcl_service_introspection_enable_service_content(rcl_service_t * service)
{
  service->impl->introspection_utils->_content_enabled = true;
}

void rcl_service_introspection_disable_client_content(rcl_client_t * client)
{
  client->impl->introspection_utils->_content_enabled = false;
}

void rcl_service_introspection_disable_service_content(rcl_service_t * service)
{
  service->impl->introspection_utils->_content_enabled = false;
}

#ifdef __cplusplus
}
#endif
