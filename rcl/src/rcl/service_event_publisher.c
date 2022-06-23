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

#include "rcl/service_event_publisher.h"

#include <string.h>

#include "rcl/service_introspection.h"

#include "./client_impl.h"
#include "./service_impl.h"

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/error_handling.h"
#include "rcl/publisher.h"
#include "rcl/node.h"
#include "rcl/time.h"
#include "rcl/types.h"
#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"
#include "rcutils/shared_library.h"
#include "rmw/error_handling.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"
#include "service_msgs/msg/service_event_info.h"

rcl_service_event_publisher_t rcl_get_zero_initialized_service_event_publisher()
{
  static rcl_service_event_publisher_t zero_service_event_publisher = {0};
  return zero_service_event_publisher;
}

rcl_service_event_publisher_options_t
rcl_service_event_publisher_get_default_options()
{
  static rcl_service_event_publisher_options_t default_options;
  // Must set the options after because they are not a compile time constant.
  default_options._enabled = true;
  default_options._content_enabled = true;
  default_options.publisher_options = rcl_publisher_get_default_options();
  default_options.clock = NULL;
  return default_options;
}

bool
rcl_service_event_publisher_is_valid(const rcl_service_event_publisher_t * service_event_publisher)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_event_publisher, "service_event_publisher is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_event_publisher->impl, "service_event_publisher's implementation is invalid",
    return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_event_publisher->impl->service_type_support,
    "service_event_publisher's service type support is invalid", return false);
  if (!rcl_clock_valid(service_event_publisher->impl->options.clock)) {
    RCL_SET_ERROR_MSG("service_event_publisher's clock is invalid");
    return false;
  }
  return true;
}

rcl_ret_t rcl_service_event_publisher_init(
  rcl_service_event_publisher_t * service_event_publisher,
  const rcl_node_t * node,
  const rcl_service_event_publisher_options_t * options,
  const char * service_name,
  const rosidl_service_type_support_t * service_type_support)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ALREADY_INIT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_TOPIC_NAME_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_PUBLISHER_INVALID);

  RCL_CHECK_ARGUMENT_FOR_NULL(service_event_publisher, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_type_support, RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ALLOCATOR_WITH_MSG(
    &options->publisher_options.allocator,
    "allocator is invalid", return RCL_RET_ERROR);
  rcl_allocator_t allocator = options->publisher_options.allocator;

  rcl_ret_t ret = RCL_RET_OK;

  if (service_event_publisher->impl) {
    rcutils_reset_error();
    RCL_SET_ERROR_MSG("service event publisher already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }

  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }

  if (!rcl_clock_valid(options->clock)) {
    rcutils_reset_error();
    RCL_SET_ERROR_MSG("clock is invalid");
    return RCL_RET_ERROR;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing service introspection for service name '%s'", service_name);
  service_event_publisher->impl = (rcl_service_event_publisher_impl_t *) allocator.allocate(
    sizeof(rcl_service_event_publisher_impl_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_event_publisher->impl, "allocating memory for rcl_service_event_publisher failed",
    return RCL_RET_BAD_ALLOC;);

  // Typesupports have static lifetimes
  service_event_publisher->impl->service_type_support = service_type_support;
  service_event_publisher->impl->options = *options;

  size_t topic_length = strlen(service_name) + strlen(RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX) + 1;
  service_event_publisher->impl->service_event_topic_name = (char *) allocator.allocate(
    topic_length, allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_event_publisher->impl->service_event_topic_name,
    "allocating memory for service introspection topic name failed",
    ret = RCL_RET_BAD_ALLOC; goto free_impl;);

  snprintf(
    service_event_publisher->impl->service_event_topic_name,
    topic_length,
    "%s%s", service_name, RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX);

  service_event_publisher->impl->options._enabled = false;
  service_event_publisher->impl->publisher = NULL;
  ret = rcl_service_introspection_enable(service_event_publisher, node);
  if (ret != RCL_RET_OK) {
    goto free_topic_name;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Service introspection for service '%s' initialized", service_name);
  return RCL_RET_OK;

free_topic_name:
  allocator.deallocate(service_event_publisher->impl->service_event_topic_name, allocator.state);

free_impl:
  allocator.deallocate(service_event_publisher->impl, allocator.state);
  service_event_publisher->impl = NULL;

  return ret;
}

rcl_ret_t rcl_service_event_publisher_fini(
  rcl_service_event_publisher_t * service_event_publisher,
  rcl_node_t * node)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ALREADY_SHUTDOWN);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_PUBLISHER_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);

  // Skip checking service_event_publisher and node as they will
  // be checked by rcl_service_introspection_disable
  rcl_ret_t ret = rcl_service_introspection_disable(service_event_publisher, node);
  if (RCL_RET_OK != ret && RCL_RET_ALREADY_SHUTDOWN != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    rcutils_reset_error();
    return ret;
  }
  rcl_allocator_t allocator = service_event_publisher->impl->options.publisher_options.allocator;

  allocator.deallocate(service_event_publisher->impl->service_event_topic_name, allocator.state);
  service_event_publisher->impl->service_event_topic_name = NULL;

  allocator.deallocate(service_event_publisher->impl, allocator.state);
  service_event_publisher->impl = NULL;

  return RCL_RET_OK;
}

rcl_ret_t rcl_send_service_event_message(
  const rcl_service_event_publisher_t * service_event_publisher,
  const uint8_t event_type,
  const void * ros_response_request,
  const int64_t sequence_number,
  const uint8_t guid[16])
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_PUBLISHER_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);

  RCL_CHECK_ARGUMENT_FOR_NULL(ros_response_request, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(guid, "guid is NULL", return RCL_RET_INVALID_ARGUMENT);

  if (!rcl_service_event_publisher_is_valid(service_event_publisher)) {
    return RCL_RET_ERROR;
  }

  // early exit if service introspection disabled during runtime
  if (!service_event_publisher->impl->options._enabled) {
    return RCL_RET_OK;
  }

  rcl_allocator_t allocator = service_event_publisher->impl->options.publisher_options.allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  if (!rcl_publisher_is_valid(service_event_publisher->impl->publisher)) {
    return RCL_RET_PUBLISHER_INVALID;
  }

  rcl_ret_t ret;

  rcl_time_point_value_t now;
  ret = rcl_clock_get_now(service_event_publisher->impl->options.clock, &now);
  if (RMW_RET_OK != ret) {
    rcutils_reset_error();
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  rosidl_service_introspection_info_t info = {
    .event_type = event_type,
    .stamp_sec = RCL_NS_TO_S(now),
    .stamp_nanosec = now % (1000LL * 1000LL * 1000LL),
    .sequence_number = sequence_number,
  };

  memcpy(info.client_gid, guid, 16);

  void * service_introspection_message;
  if (!service_event_publisher->impl->options._content_enabled) {
    ros_response_request = NULL;
  }
  switch (event_type) {
    case service_msgs__msg__ServiceEventInfo__REQUEST_RECEIVED:
    case service_msgs__msg__ServiceEventInfo__REQUEST_SENT:
      service_introspection_message =
        service_event_publisher->impl->service_type_support->event_message_create_handle_function(
        &info, &allocator, ros_response_request, NULL);
      break;
    case service_msgs__msg__ServiceEventInfo__RESPONSE_RECEIVED:
    case service_msgs__msg__ServiceEventInfo__RESPONSE_SENT:
      service_introspection_message =
        service_event_publisher->impl->service_type_support->event_message_create_handle_function(
        &info, &allocator, NULL, ros_response_request);
      break;
    default:
      rcutils_reset_error();
      RCL_SET_ERROR_MSG("unsupported event type");
      return RCL_RET_ERROR;
  }
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_introspection_message, "service_introspection_message is NULL", return RCL_RET_ERROR);

  // and publish it out!
  // TODO(ihasdapie): Publisher context can become invalidated on shutdown
  ret = rcl_publish(service_event_publisher->impl->publisher, service_introspection_message, NULL);
  // clean up before error checking
  service_event_publisher->impl->service_type_support->event_message_destroy_handle_function(
    service_introspection_message, &allocator);
  if (RCL_RET_OK != ret) {
    rcutils_reset_error();
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
  }

  return ret;
}

rcl_ret_t rcl_service_introspection_enable(
  rcl_service_event_publisher_t * service_event_publisher,
  const rcl_node_t * node)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_TOPIC_NAME_INVALID);

  if (!rcl_service_event_publisher_is_valid(service_event_publisher)) {
    return RCL_RET_ERROR;
  }

  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  // need to check if node_opt is disabled

  // Early exit if already enabled
  if (service_event_publisher->impl->options._enabled) {
    return RCL_RET_OK;
  }

  rcl_allocator_t allocator = service_event_publisher->impl->options.publisher_options.allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_ERROR);

  service_event_publisher->impl->publisher = allocator.allocate(
    sizeof(rcl_publisher_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_event_publisher->impl->publisher,
    "allocate service_event_publisher failed in enable", return RCL_RET_BAD_ALLOC);
  *service_event_publisher->impl->publisher = rcl_get_zero_initialized_publisher();
  rcl_ret_t ret = rcl_publisher_init(
    service_event_publisher->impl->publisher, node,
    service_event_publisher->impl->service_type_support->event_typesupport,
    service_event_publisher->impl->service_event_topic_name,
    &service_event_publisher->impl->options.publisher_options);
  if (RCL_RET_OK != ret) {
    allocator.deallocate(service_event_publisher->impl->publisher, allocator.state);
    service_event_publisher->impl->publisher = NULL;
    rcutils_reset_error();
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return ret;
  }

  service_event_publisher->impl->options._enabled = true;
  return RCL_RET_OK;
}

rcl_ret_t rcl_service_introspection_disable(
  rcl_service_event_publisher_t * service_event_publisher,
  rcl_node_t * node)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_PUBLISHER_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ALREADY_SHUTDOWN);

  if (!rcl_service_event_publisher_is_valid(service_event_publisher)) {
    return RCL_RET_ERROR;
  }

  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }

  // Early exit if already disabled
  if (!service_event_publisher->impl->options._enabled) {
    return RCL_RET_OK;
  }

  rcl_allocator_t allocator = service_event_publisher->impl->options.publisher_options.allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_ERROR);

  if (service_event_publisher->impl->publisher) {
    rcl_ret_t ret = rcl_publisher_fini(service_event_publisher->impl->publisher, node);
    allocator.deallocate(service_event_publisher->impl->publisher, allocator.state);
    service_event_publisher->impl->publisher = NULL;
    if (RCL_RET_OK != ret) {
      rcutils_reset_error();
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      return ret;
    }
  }

  service_event_publisher->impl->options._enabled = false;
  return RCL_RET_OK;
}
