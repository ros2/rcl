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

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "rcl/publisher.h"

#include "rcl/time.h"
#include "rcl/types.h"
#include "rcl/error_handling.h"
#include "rcl/node.h"
#include "rcl/graph.h"
#include "rcl/publisher.h"

#include "rmw/error_handling.h"
#include "rmw/rmw.h"

#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"

#include "tracetools/tracetools.h"


#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"

#include "rcl_interfaces/msg/service_event_type.h"
#include "rcl_interfaces/msg/service_event.h"



#include "./service_impl.h"


rcl_service_t
rcl_get_zero_initialized_service()
{
  static rcl_service_t null_service = {0};
  return null_service;
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
  char * service_event_name = NULL;
  // Preallocate space for for service_event_name

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
    goto cleanup;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Expanded and remapped service name '%s'", remapped_service_name);

  // Allocate space for the implementation struct.
  service->impl = (rcl_service_impl_t *)allocator->allocate(
    sizeof(rcl_service_impl_t), allocator->state);
      
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service->impl, "allocating memory failed", ret = RCL_RET_BAD_ALLOC; goto cleanup);

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
    goto fail;
  }

  // TODO(ihasdapie): Wrap this up as a fn introspection.c w/ `initialize_introspection_utils` and _fini, etc
  service->impl->introspection_utils = (rcl_service_introspection_utils_t *) allocator->allocate(
      sizeof(rcl_service_introspection_utils_t), allocator->state);
  // rcl_service_introspection_utils_t * iutils = service->impl->introspection_utils;
  
  // the reason why we can't use a macro and concatenate the Type to get what we want is 
  // because there we had always {goal status result} but here we can have any service type name...
  // TODO(ihasdapie): Need to get the package and message name somehow
  char * tsfile = "libexample_interfaces__rosidl_typesupport_c.so";
  void* tslib = dlopen(tsfile, RTLD_LAZY);
  RCL_CHECK_FOR_NULL_WITH_MSG(tslib, "dlopen failed", ret = RCL_RET_ERROR; goto cleanup);
  // there are macros for building these in rosidl_typesupport_interface but still need the msg package name
  const char* ts_req_func_name = "rosidl_typesupport_c__get_message_type_support_handle__example_interfaces__srv__AddTwoInts_Request";
  const char* ts_resp_func_name = "rosidl_typesupport_c__get_message_type_support_handle__example_interfaces__srv__AddTwoInts_Response";

  // TODO(ihasdapie): squash warning  "ISO C forbids conversion of function pointer to object pointer type"
  rosidl_message_type_support_t* (* ts_func_handle)() = (rosidl_message_type_support_t*(*)()) dlsym(tslib, ts_resp_func_name);
  RCL_CHECK_FOR_NULL_WITH_MSG(ts_func_handle, "looking up response type support failed", ret = RCL_RET_ERROR; goto cleanup);
  service->impl->introspection_utils->response_type_support = ts_func_handle();
  ts_func_handle = (rosidl_message_type_support_t*(*)()) dlsym(tslib, ts_req_func_name);
  RCL_CHECK_FOR_NULL_WITH_MSG(ts_func_handle, "looking up response type support failed", ret = RCL_RET_ERROR; goto cleanup);
  service->impl->introspection_utils->request_type_support = ts_func_handle();

  service_event_name = (char*) allocator->zero_allocate(
      strlen(remapped_service_name) + strlen(RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX) + 1,
      sizeof(char), allocator->state);
  strcpy(service_event_name, remapped_service_name);
  strcat(service_event_name, RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX);

  // Make a publisher
  service->impl->introspection_utils->publisher = allocator->allocate(sizeof(rcl_publisher_t), allocator->state);
  *service->impl->introspection_utils->publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * service_event_typesupport = 
    ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, ServiceEvent);  // ROSIDL_GET_MSG_TYPE_SUPPORT gives an int?? And complier warns that it is being converted to a ptr
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(service->impl->introspection_utils->publisher, node,
    service_event_typesupport, service_event_name, &publisher_options);

  // there has to be a macro for this, right?
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    goto fail;
  }

  // make a clock
  service->impl->introspection_utils->clock = allocator->allocate(sizeof(rcl_clock_t), allocator->state);
  ret = rcl_clock_init(RCL_STEADY_TIME, service->impl->introspection_utils->clock, allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    goto fail;
  }
  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "Clock initialized");

  // get actual qos, and store it
  rmw_ret_t rmw_ret = rmw_service_request_subscription_get_actual_qos(
    service->impl->rmw_handle,
    &service->impl->actual_request_subscription_qos);

  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    goto fail;
  }

  rmw_ret = rmw_service_response_publisher_get_actual_qos(
    service->impl->rmw_handle,
    &service->impl->actual_response_publisher_qos);

  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    goto fail;
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
  goto cleanup;
fail:
  if (service->impl) {
    allocator->deallocate(service->impl->introspection_utils, allocator->state);
    allocator->deallocate(service->impl, allocator->state);
    service->impl->introspection_utils = NULL;
    service->impl = NULL;
    // iutils= NULL;
  }
  ret = fail_ret;
  // Fall through to clean up
cleanup:
  allocator->deallocate(remapped_service_name, allocator->state);
  allocator->deallocate(service_event_name, allocator->state);
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
    rmw_ret_t ret = rmw_destroy_service(rmw_node, service->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      result = RCL_RET_ERROR;
    }
    allocator.deallocate(service->impl->introspection_utils, allocator.state);
    allocator.deallocate(service->impl, allocator.state);
    service->impl->introspection_utils = NULL;
    service->impl = NULL;

    ret = rcl_publisher_fini(service->impl->introspection_utils->publisher, node);
    ret = rcl_clock_fini(service->impl->introspection_utils->clock);
    if (ret != RCL_RET_OK) {
      RCL_SET_ERROR_MSG(rcl_get_error_string().str);
      result = RCL_RET_ERROR;
    }
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
  default_options.allocator = rcl_get_default_allocator();
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

  /* send_introspection_message(
    service,
    rcl_interfaces__msg__ServiceEventType__REQUEST_RECEIVED,
    ros_request,
    request_header,
    options); */

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

  // publish out the introspected content
  send_introspection_message(
    service,
    rcl_interfaces__msg__ServiceEventType__RESPONSE_SENT,
    ros_response,
    request_header,
    options);
  
  if (rmw_send_response(
      service->impl->rmw_handle, request_header, ros_response) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
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

#ifdef __cplusplus
}
#endif
