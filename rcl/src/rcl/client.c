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

#include "rcl/client.h"

#include <stdio.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcl/remap.h"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"

#include "./common.h"
#include "./stdatomic_helper.h"

typedef struct rcl_client_impl_t
{
  rcl_client_options_t options;
  rmw_client_t * rmw_handle;
  atomic_int_least64_t sequence_number;
} rcl_client_impl_t;

rcl_client_t
rcl_get_zero_initialized_client()
{
  static rcl_client_t null_client = {0};
  return null_client;
}

rcl_ret_t
rcl_client_init(
  rcl_client_t * client,
  const rcl_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rcl_client_options_t * options)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  // check the options and allocator first, so the allocator can be passed to errors
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_allocator_t * allocator = (rcl_allocator_t *)&options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, *allocator);
  if (!rcl_node_is_valid(node, allocator)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_name, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing client for service name '%s'", service_name)
  if (client->impl) {
    RCL_SET_ERROR_MSG("client already initialized, or memory was unintialized", *allocator);
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
        ROS_PACKAGE_NAME,
        "failed to fini string_map (%d) during error handling: %s\n",
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
    if (ret == RCL_RET_TOPIC_NAME_INVALID || ret == RCL_RET_UNKNOWN_SUBSTITUTION) {
      ret = RCL_RET_SERVICE_NAME_INVALID;
    } else {
      ret = RCL_RET_ERROR;
    }
    goto cleanup;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Expanded service name '%s'", expanded_service_name)

  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (NULL == node_options) {
    ret = RCL_RET_ERROR;
    goto cleanup;
  }
  char * remapped_service_name = NULL;
  ret = rcl_remap_service_name(
    &(node_options->arguments),
    node_options->use_global_arguments,
    expanded_service_name,
    rcl_node_get_name(node),
    rcl_node_get_namespace(node),
    *allocator,
    &remapped_service_name);
  if (RCL_RET_OK != ret) {
    goto fail;
  } else if (NULL == remapped_service_name) {
    remapped_service_name = expanded_service_name;
    expanded_service_name = NULL;
  }

  // Validate the expanded service name.
  int validation_result;
  rmw_ret_t rmw_ret = rmw_validate_full_topic_name(remapped_service_name, &validation_result, NULL);
  if (rmw_ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    ret = RCL_RET_ERROR;
    goto cleanup;
  }
  if (validation_result != RMW_TOPIC_VALID) {
    RCL_SET_ERROR_MSG(rmw_full_topic_name_validation_result_string(validation_result), *allocator)
    ret = RCL_RET_SERVICE_NAME_INVALID;
    goto cleanup;
  }
  // Allocate space for the implementation struct.
  client->impl = (rcl_client_impl_t *)allocator->allocate(
    sizeof(rcl_client_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "allocating memory failed", ret = RCL_RET_BAD_ALLOC; goto cleanup, *allocator);
  // Fill out implementation struct.
  // rmw handle (create rmw client)
  // TODO(wjwwood): pass along the allocator to rmw when it supports it
  client->impl->rmw_handle = rmw_create_client(
    rcl_node_get_rmw_handle(node),
    type_support,
    remapped_service_name,
    &options->qos);
  if (!client->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    goto fail;
  }
  // options
  client->impl->options = *options;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Client initialized")
  ret = RCL_RET_OK;
  goto cleanup;
fail:
  if (client->impl) {
    allocator->deallocate(client->impl, allocator->state);
  }
  ret = fail_ret;
  // Fall through to cleanup
cleanup:
  if (NULL != expanded_service_name) {
    allocator->deallocate(expanded_service_name, allocator->state);
  }
  if (NULL != remapped_service_name) {
    allocator->deallocate(remapped_service_name, allocator->state);
  }
  return ret;
}

rcl_ret_t
rcl_client_fini(rcl_client_t * client, rcl_node_t * node)
{
  (void)node;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing client")
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (!rcl_node_is_valid(node, NULL)) {
    return RCL_RET_NODE_INVALID;
  }
  if (client->impl) {
    rcl_allocator_t allocator = client->impl->options.allocator;
    rmw_node_t * rmw_node = rcl_node_get_rmw_handle(node);
    if (!rmw_node) {
      return RCL_RET_INVALID_ARGUMENT;
    }
    rmw_ret_t ret = rmw_destroy_client(rmw_node, client->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), allocator);
      result = RCL_RET_ERROR;
    }
    allocator.deallocate(client->impl, allocator.state);
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Client finalized")
  return result;
}

rcl_client_options_t
rcl_client_get_default_options()
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_client_options_t default_options;
  // Must set the allocator and qos after because they are not a compile time constant.
  default_options.qos = rmw_qos_profile_services_default;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

const char *
rcl_client_get_service_name(const rcl_client_t * client)
{
  if (!rcl_client_is_valid(client, NULL)) {
    return NULL;  // error already set
  }
  return client->impl->rmw_handle->service_name;
}

#define _client_get_options(client) & client->impl->options;

const rcl_client_options_t *
rcl_client_get_options(const rcl_client_t * client)
{
  if (!rcl_client_is_valid(client, NULL)) {
    return NULL;  // error already set
  }
  return _client_get_options(client);
}

rmw_client_t *
rcl_client_get_rmw_handle(const rcl_client_t * client)
{
  if (!rcl_client_is_valid(client, NULL)) {
    return NULL;  // error already set
  }
  return client->impl->rmw_handle;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_send_request(const rcl_client_t * client, const void * ros_request, int64_t * sequence_number)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Client sending service request")
  if (!rcl_client_is_valid(client, NULL)) {
    return RCL_RET_CLIENT_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_request, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(
    sequence_number, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  *sequence_number = rcl_atomic_load_int64_t(&client->impl->sequence_number);
  if (rmw_send_request(
      client->impl->rmw_handle, ros_request, sequence_number) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), client->impl->options.allocator);
    return RCL_RET_ERROR;
  }
  rcl_atomic_exchange_int64_t(&client->impl->sequence_number, *sequence_number);
  return RCL_RET_OK;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_take_response(
  const rcl_client_t * client,
  rmw_request_id_t * request_header,
  void * ros_response)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Client taking service response")
  if (!rcl_client_is_valid(client, NULL)) {
    return RCL_RET_CLIENT_INVALID;
  }

  RCL_CHECK_ARGUMENT_FOR_NULL(
    request_header, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_response, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());

  bool taken = false;
  if (rmw_take_response(
      client->impl->rmw_handle, request_header, ros_response, &taken) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), client->impl->options.allocator);
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Client take response succeeded: %s", taken ? "true" : "false")
  if (!taken) {
    return RCL_RET_CLIENT_TAKE_FAILED;
  }
  return RCL_RET_OK;
}

bool rcl_client_is_valid(const rcl_client_t * client, rcl_allocator_t * error_msg_allocator)
{
  const rcl_client_options_t * options;
  rcl_allocator_t alloc =
    error_msg_allocator ? *error_msg_allocator : rcl_get_default_allocator();
  RCL_CHECK_ALLOCATOR_WITH_MSG(&alloc, "allocator is invalid", return false);
  RCL_CHECK_ARGUMENT_FOR_NULL(client, false, alloc);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "client's rmw implementation is invalid", return false, alloc);
  options = _client_get_options(client);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    options, "client's options pointer is invalid", return false, alloc);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl->rmw_handle, "client's rmw handle is invalid", return false, alloc);
  return true;
}
#if __cplusplus
}
#endif
