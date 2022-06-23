// Copyright 2016 Open Source Robotics Foundation, Inc.
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
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/time.h"
#include "rcl/types.h"
#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "service_msgs/msg/service_event_info.h"
#include "tracetools/tracetools.h"

#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"

#include "./common.h"
#include "./service_event_publisher.h"
#include "./service_impl.h"


rcl_service_t
rcl_get_zero_initialized_service()
{
  static rcl_service_t null_service = {0};
  return null_service;
}

static
rcl_ret_t
unconfigure_service_introspection(
  rcl_node_t * node,
  struct rcl_service_impl_s * service_impl,
  rcl_allocator_t * allocator)
{
  if (!service_impl->service_event_publisher) {
    return RCL_RET_OK;
  }

  rcl_ret_t ret = rcl_service_event_publisher_fini(service_impl->service_event_publisher, node);

  allocator->deallocate(service_impl->service_event_publisher, allocator->state);
  service_impl->service_event_publisher = NULL;

  return ret;
}

static
rcl_ret_t
configure_service_introspection(
  const rcl_node_t * node,
  struct rcl_service_impl_s * service_impl,
  rcl_allocator_t * allocator,
  const rcl_service_options_t * options,
  const rosidl_service_type_support_t * type_support,
  const char * remapped_service_name)
{
  if (!rcl_node_get_options(node)->enable_service_introspection) {
    return RCL_RET_OK;
  }

  service_impl->service_event_publisher = allocator->zero_allocate(
    1, sizeof(rcl_service_event_publisher_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service_impl->service_event_publisher, "allocating memory failed", return RCL_RET_BAD_ALLOC;);

  rcl_service_event_publisher_options_t service_event_options =
    rcl_service_event_publisher_get_default_options();
  service_event_options.publisher_options = options->event_publisher_options;
  service_event_options.clock = options->clock;

  *service_impl->service_event_publisher = rcl_get_zero_initialized_service_event_publisher();
  rcl_ret_t ret = rcl_service_event_publisher_init(
    service_impl->service_event_publisher, node, &service_event_options,
    remapped_service_name, type_support);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    allocator->deallocate(service_impl->service_event_publisher, allocator->state);
    service_impl->service_event_publisher = NULL;
    return ret;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_service_init(
  rcl_service_t * service,
  const rcl_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rcl_service_options_t * options)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ALREADY_INIT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_SERVICE_NAME_INVALID);

  rcl_ret_t fail_ret = RCL_RET_ERROR;

  // Check options and allocator first, so the allocator can be used in errors.
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t * allocator = (rcl_allocator_t *)&options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(service, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_name, RCL_RET_INVALID_ARGUMENT);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing service for service name '%s'", service_name);
  if (service->impl) {
    RCL_SET_ERROR_MSG("service already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }

  // Expand and remap the given service name.
  char * remapped_service_name = NULL;
  rcl_ret_t ret = rcl_node_resolve_name(
    node,
    service_name,
    *allocator,
    true,
    false,
    &remapped_service_name);
  if (ret != RCL_RET_OK) {
    if (ret == RCL_RET_SERVICE_NAME_INVALID || ret == RCL_RET_UNKNOWN_SUBSTITUTION) {
      ret = RCL_RET_SERVICE_NAME_INVALID;
    } else if (ret != RCL_RET_BAD_ALLOC) {
      ret = RCL_RET_ERROR;
    }
    return ret;
  }
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Expanded and remapped service name '%s'", remapped_service_name);

  // Allocate space for the implementation struct.
  service->impl = (rcl_service_impl_t *)allocator->zero_allocate(
    1, sizeof(rcl_service_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service->impl, "allocating memory failed",
    ret = RCL_RET_BAD_ALLOC; goto free_remapped_service_name);

  if (RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL == options->qos.durability) {
    RCUTILS_LOG_WARN_NAMED(
      ROS_PACKAGE_NAME,
      "Warning: Setting QoS durability to 'transient local' for service servers "
      "can cause them to receive requests from clients that have since terminated.");
  }
  // Fill out implementation struct.
  // rmw handle (create rmw service)
  // TODO(wjwwood): pass along the allocator to rmw when it supports it
  service->impl->rmw_handle = rmw_create_service(
    rcl_node_get_rmw_handle(node),
    type_support,
    remapped_service_name,
    &options->qos);
  if (!service->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    ret = RCL_RET_ERROR;
    goto free_service_impl;
  }

  ret = configure_service_introspection(
    node, service->impl, allocator, options, type_support, remapped_service_name);
  if (RCL_RET_OK != ret) {
    goto destroy_service;
  }

  // get actual qos, and store it
  rmw_ret_t rmw_ret = rmw_service_request_subscription_get_actual_qos(
    service->impl->rmw_handle,
    &service->impl->actual_request_subscription_qos);
  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    ret = rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
    goto unconfigure_introspection;
  }

  rmw_ret = rmw_service_response_publisher_get_actual_qos(
    service->impl->rmw_handle,
    &service->impl->actual_response_publisher_qos);
  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    ret = rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
    goto unconfigure_introspection;
  }

  // ROS specific namespacing conventions is not retrieved by get_actual_qos
  service->impl->actual_request_subscription_qos.avoid_ros_namespace_conventions =
    options->qos.avoid_ros_namespace_conventions;
  service->impl->actual_response_publisher_qos.avoid_ros_namespace_conventions =
    options->qos.avoid_ros_namespace_conventions;

  // options
  service->impl->options = *options;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Service initialized");
  ret = RCL_RET_OK;
  TRACEPOINT(
    rcl_service_init,
    (const void *)service,
    (const void *)node,
    (const void *)service->impl->rmw_handle,
    remapped_service_name);

  goto free_remapped_service_name;

unconfigure_introspection:
  // TODO(clalancette): I don't love casting away the const from node here,
  // but the cleanup path goes deep and I didn't want to change 6 or so
  // different signatures.
  fail_ret = unconfigure_service_introspection((rcl_node_t *)node, service->impl, allocator);
  if (RCL_RET_OK != fail_ret) {
    // TODO(clalancette): print the error message here
  }

destroy_service:
  rmw_ret = rmw_destroy_service(rcl_node_get_rmw_handle(node), service->impl->rmw_handle);
  if (RMW_RET_OK != rmw_ret) {
    // TODO(clalancette): print the error message here
  }

free_service_impl:
  allocator->deallocate(service->impl, allocator->state);
  service->impl = NULL;

free_remapped_service_name:
  allocator->deallocate(remapped_service_name, allocator->state);
  return ret;
}

rcl_ret_t
rcl_service_fini(rcl_service_t * service, rcl_node_t * node)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_SERVICE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing service");
  RCL_CHECK_ARGUMENT_FOR_NULL(service, RCL_RET_SERVICE_INVALID);
  if (!rcl_node_is_valid_except_context(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }

  rcl_ret_t result = RCL_RET_OK;
  if (service->impl) {
    rcl_allocator_t allocator = service->impl->options.allocator;
    rmw_node_t * rmw_node = rcl_node_get_rmw_handle(node);
    if (!rmw_node) {
      return RCL_RET_INVALID_ARGUMENT;
    }

    rcl_ret_t rcl_ret = unconfigure_service_introspection(node, service->impl, &allocator);
    if (RCL_RET_OK != rcl_ret) {
      RCL_SET_ERROR_MSG(rcl_get_error_string().str);
      result = rcl_ret;
    }

    rmw_ret_t ret = rmw_destroy_service(rmw_node, service->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      result = RCL_RET_ERROR;
    }

    allocator.deallocate(service->impl, allocator.state);
    service->impl = NULL;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Service finalized");
  return result;
}

rcl_service_options_t
rcl_service_get_default_options()
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_service_options_t default_options;
  // Must set the allocator and qos after because they are not a compile time constant.
  default_options.qos = rmw_qos_profile_services_default;
  default_options.event_publisher_options = rcl_publisher_get_default_options();
  default_options.allocator = rcl_get_default_allocator();
  default_options.enable_service_introspection = false;
  default_options.clock = NULL;
  return default_options;
}

const char *
rcl_service_get_service_name(const rcl_service_t * service)
{
  const rcl_service_options_t * options = rcl_service_get_options(service);
  if (!options) {
    return NULL;
  }
  RCL_CHECK_FOR_NULL_WITH_MSG(service->impl->rmw_handle, "service is invalid", return NULL);
  return service->impl->rmw_handle->service_name;
}

#define _service_get_options(service) & service->impl->options

const rcl_service_options_t *
rcl_service_get_options(const rcl_service_t * service)
{
  if (!rcl_service_is_valid(service)) {
    return NULL;  // error already set
  }
  return _service_get_options(service);
}

rmw_service_t *
rcl_service_get_rmw_handle(const rcl_service_t * service)
{
  if (!rcl_service_is_valid(service)) {
    return NULL;  // error already set
  }
  return service->impl->rmw_handle;
}

rcl_ret_t
rcl_take_request_with_info(
  const rcl_service_t * service,
  rmw_service_info_t * request_header,
  void * ros_request)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Service server taking service request");
  if (!rcl_service_is_valid(service)) {
    return RCL_RET_SERVICE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(request_header, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_request, RCL_RET_INVALID_ARGUMENT);
  const rcl_service_options_t * options = rcl_service_get_options(service);
  RCL_CHECK_FOR_NULL_WITH_MSG(options, "Failed to get service options", return RCL_RET_ERROR);

  bool taken = false;
  rmw_ret_t ret = rmw_take_request(
    service->impl->rmw_handle, request_header, ros_request, &taken);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    if (RMW_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Service take request succeeded: %s", taken ? "true" : "false");
  if (!taken) {
    return RCL_RET_SERVICE_TAKE_FAILED;
  }
  if (rcl_service_get_options(service)->enable_service_introspection) {
    rcl_ret_t rclret = rcl_send_service_event_message(
      service->impl->service_event_publisher,
      service_msgs__msg__ServiceEventInfo__REQUEST_RECEIVED,
      ros_request,
      request_header->request_id.sequence_number,
      request_header->request_id.writer_guid);
    if (RCL_RET_OK != rclret) {
      RCL_SET_ERROR_MSG(rcl_get_error_string().str);
      return rclret;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_take_request(
  const rcl_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_request)
{
  rmw_service_info_t header;
  header.request_id = *request_header;
  rcl_ret_t ret = rcl_take_request_with_info(service, &header, ros_request);
  *request_header = header.request_id;
  return ret;
}

rcl_ret_t
rcl_send_response(
  const rcl_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_response)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Sending service response");
  if (!rcl_service_is_valid(service)) {
    return RCL_RET_SERVICE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(request_header, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_response, RCL_RET_INVALID_ARGUMENT);
  const rcl_service_options_t * options = rcl_service_get_options(service);
  RCL_CHECK_FOR_NULL_WITH_MSG(options, "Failed to get service options", return RCL_RET_ERROR);

  if (rmw_send_response(
      service->impl->rmw_handle, request_header, ros_response) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  // publish out the introspected content
  if (rcl_service_get_options(service)->enable_service_introspection) {
    rcl_ret_t ret = rcl_send_service_event_message(
      service->impl->service_event_publisher,
      service_msgs__msg__ServiceEventInfo__RESPONSE_SENT,
      ros_response,
      request_header->sequence_number,
      request_header->writer_guid);
    if (RCL_RET_OK != ret) {
      RCL_SET_ERROR_MSG(rcl_get_error_string().str);
      return ret;
    }
  }
  return RCL_RET_OK;
}

bool
rcl_service_is_valid(const rcl_service_t * service)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(service, "service pointer is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service->impl, "service's implementation is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service->impl->rmw_handle, "service's rmw handle is invalid", return false);
  return true;
}

const rmw_qos_profile_t *
rcl_service_request_subscription_get_actual_qos(const rcl_service_t * service)
{
  if (!rcl_service_is_valid(service)) {
    return NULL;
  }
  return &service->impl->actual_request_subscription_qos;
}

const rmw_qos_profile_t *
rcl_service_response_publisher_get_actual_qos(const rcl_service_t * service)
{
  if (!rcl_service_is_valid(service)) {
    return NULL;
  }
  return &service->impl->actual_response_publisher_qos;
}

rcl_ret_t
rcl_service_set_on_new_request_callback(
  const rcl_service_t * service,
  rcl_event_callback_t callback,
  const void * user_data)
{
  if (!rcl_service_is_valid(service)) {
    // error state already set
    return RCL_RET_INVALID_ARGUMENT;
  }

  return rmw_service_set_on_new_request_callback(
    service->impl->rmw_handle,
    callback,
    user_data);
}

rcl_ret_t
rcl_service_introspection_configure_server_service_events(
  rcl_service_t * service,
  rcl_node_t * node,
  bool enable)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(service, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service->impl, RCL_RET_INVALID_ARGUMENT);

  if (enable) {
    return rcl_service_introspection_enable(service->impl->service_event_publisher, node);
  }
  return rcl_service_introspection_disable(service->impl->service_event_publisher, node);
}

rcl_ret_t
rcl_service_introspection_configure_server_service_event_message_payload(
  rcl_service_t * service,
  bool enable)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(service, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service->impl, RCL_RET_INVALID_ARGUMENT);

  service->impl->service_event_publisher->impl->options._content_enabled = enable;

  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
