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

#if __cplusplus
extern "C"
{
#endif

#include "rcl/service.h"

#include <stdio.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"

typedef struct rcl_service_impl_t
{
  rcl_service_options_t options;
  rmw_service_t * rmw_handle;
} rcl_service_impl_t;

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
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  // Check options and allocator first, so the allocator can be used in errors.
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(service, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, *allocator);
  if (!node->impl) {
    RCL_SET_ERROR_MSG("invalid node", *allocator);
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_name, RCL_RET_INVALID_ARGUMENT, *allocator);
  if (service->impl) {
    RCL_SET_ERROR_MSG("service already initialized, or memory was unintialized", *allocator);
    return RCL_RET_ALREADY_INIT;
  }
  // Expand the given service name.
  rcutils_allocator_t rcutils_allocator = *allocator;  // implicit conversion to rcutils version
  rcutils_string_map_t substitutions_map = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions_map, 0, rcutils_allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string_safe(), *allocator)
    if (rcutils_ret == RCUTILS_RET_BAD_ALLOC) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  rcl_ret_t ret = rcl_get_default_topic_name_substitutions(&substitutions_map);
  if (ret != RCL_RET_OK) {
    rcutils_ret = rcutils_string_map_fini(&substitutions_map);
    if (rcutils_ret != RCUTILS_RET_OK) {
      RCUTILS_LOG_ERROR_NAMED(
        "rcl",
        "failed to fini string_map (%d) during error handling: %s",
        rcutils_ret,
        rcutils_get_error_string_safe())
    }
    if (ret == RCL_RET_BAD_ALLOC) {
      return ret;
    }
    return RCL_RET_ERROR;
  }
  char * expanded_service_name = NULL;
  ret = rcl_expand_topic_name(
    service_name,
    rcl_node_get_name(node),
    rcl_node_get_namespace(node),
    &substitutions_map,
    *allocator,
    &expanded_service_name);
  rcutils_ret = rcutils_string_map_fini(&substitutions_map);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string_safe(), *allocator)
    allocator->deallocate(expanded_service_name, allocator->state);
    return RCL_RET_ERROR;
  }
  if (ret != RCL_RET_OK) {
    if (ret == RCL_RET_BAD_ALLOC) {
      return ret;
    } else if (ret == RCL_RET_TOPIC_NAME_INVALID || ret == RCL_RET_UNKNOWN_SUBSTITUTION) {
      return RCL_RET_SERVICE_NAME_INVALID;
    } else {
      return RCL_RET_ERROR;
    }
  }
  // Validate the expanded service name.
  int validation_result;
  rmw_ret_t rmw_ret = rmw_validate_full_topic_name(expanded_service_name, &validation_result, NULL);
  if (rmw_ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    return RCL_RET_ERROR;
  }
  if (validation_result != RMW_TOPIC_VALID) {
    RCL_SET_ERROR_MSG(rmw_full_topic_name_validation_result_string(validation_result), *allocator)
    return RCL_RET_SERVICE_NAME_INVALID;
  }
  // Allocate space for the implementation struct.
  service->impl = (rcl_service_impl_t *)allocator->allocate(
    sizeof(rcl_service_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC, *allocator);

  if (RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL == options->qos.durability) {
    RCUTILS_LOG_WARN_NAMED(
      "rcl",
      "Warning: Setting QoS durability to 'transient local' for service servers "
      "can cause them to receive requests from clients that have since terminated.")
  }
  // Fill out implementation struct.
  // rmw handle (create rmw service)
  // TODO(wjwwood): pass along the allocator to rmw when it supports it
  service->impl->rmw_handle = rmw_create_service(
    rcl_node_get_rmw_handle(node),
    type_support,
    expanded_service_name,
    &options->qos);
  allocator->deallocate(expanded_service_name, allocator->state);
  if (!service->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    goto fail;
  }
  // options
  service->impl->options = *options;
  return RCL_RET_OK;
fail:
  if (service->impl) {
    allocator->deallocate(service->impl, allocator->state);
  }
  return fail_ret;
}

rcl_ret_t
rcl_service_fini(rcl_service_t * service, rcl_node_t * node)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(service, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (service->impl) {
    rcl_allocator_t allocator = service->impl->options.allocator;
    rmw_node_t * rmw_node = rcl_node_get_rmw_handle(node);
    if (!rmw_node) {
      return RCL_RET_INVALID_ARGUMENT;
    }
    rmw_ret_t ret = rmw_destroy_service(rmw_node, service->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), allocator);
      result = RCL_RET_ERROR;
    }
    allocator.deallocate(service->impl, allocator.state);
  }
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
  RCL_CHECK_FOR_NULL_WITH_MSG(
    service->impl->rmw_handle, "service is invalid", return NULL, options->allocator);
  return service->impl->rmw_handle->service_name;
}

/* *INDENT-OFF* */
#define _service_get_options(service) &service->impl->options
/* *INDENT-ON* */

const rcl_service_options_t *
rcl_service_get_options(const rcl_service_t * service)
{
  if (!rcl_service_is_valid(service)) {
    return NULL;
  }
  return _service_get_options(service);
}

rmw_service_t *
rcl_service_get_rmw_handle(const rcl_service_t * service)
{
  if (!rcl_service_is_valid(service)) {
    return NULL;
  }
  return service->impl->rmw_handle;
}

rcl_ret_t
rcl_take_request(
  const rcl_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_request)
{
  const rcl_service_options_t * options = rcl_service_get_options(service);
  RCL_CHECK_ARGUMENT_FOR_NULL(request_header, RCL_RET_INVALID_ARGUMENT, options->allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_request, RCL_RET_INVALID_ARGUMENT, options->allocator);

  bool taken = false;
  if (rmw_take_request(
      service->impl->rmw_handle, request_header, ros_request, &taken) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), options->allocator);
    return RCL_RET_ERROR;
  }
  if (!taken) {
    return RCL_RET_SERVICE_TAKE_FAILED;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_send_response(
  const rcl_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_response)
{
  const rcl_service_options_t * options = rcl_service_get_options(service);
  RCL_CHECK_ARGUMENT_FOR_NULL(request_header, RCL_RET_INVALID_ARGUMENT, options->allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_response, RCL_RET_INVALID_ARGUMENT, options->allocator);

  if (rmw_send_response(
      service->impl->rmw_handle, request_header, ros_response) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), options->allocator);
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

bool
rcl_service_is_valid(const rcl_service_t * service)
{
  const rcl_service_options_t * options;
  RCL_CHECK_ARGUMENT_FOR_NULL(service, false, rcl_get_default_allocator());
  options = _service_get_options(service);
  RCL_CHECK_FOR_NULL_WITH_MSG(options,
    "service's options pointer is invalid",
    return false,
    rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(service->impl,
    "service implementation is invalid",
    return false,
    options->allocator);
  RCL_CHECK_FOR_NULL_WITH_MSG(service->impl->rmw_handle,
    "service's rmw handle is invalid",
    return false,
    options->allocator);
  return true;
}

#if __cplusplus
}
#endif
