// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include "rcl/publisher.h"

#include <stdio.h>
#include <string.h>

#include "./common.h"
#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcl/remap.h"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"

typedef struct rcl_publisher_impl_t
{
  rcl_publisher_options_t options;
  rmw_publisher_t * rmw_handle;
} rcl_publisher_impl_t;

rcl_publisher_t
rcl_get_zero_initialized_publisher()
{
  static rcl_publisher_t null_publisher = {0};
  return null_publisher;
}

rcl_ret_t
rcl_publisher_init(
  rcl_publisher_t * publisher,
  const rcl_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rcl_publisher_options_t * options)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  // Check options and allocator first, so allocator can be used with errors.
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t * allocator = (rcl_allocator_t *)&options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, RCL_RET_INVALID_ARGUMENT);
  if (publisher->impl) {
    RCL_SET_ERROR_MSG("publisher already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing publisher for topic name '%s'", topic_name);
  // Expand the given topic name.
  rcutils_allocator_t rcutils_allocator = *allocator;  // implicit conversion to rcutils version
  rcutils_string_map_t substitutions_map = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions_map, 0, rcutils_allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
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
        "failed to fini string_map (%d) during error handling: %s",
        rcutils_ret,
        rcutils_get_error_string().str);
    }
    if (ret == RCL_RET_BAD_ALLOC) {
      return ret;
    }
    return RCL_RET_ERROR;
  }
  char * expanded_topic_name = NULL;
  char * remapped_topic_name = NULL;
  ret = rcl_expand_topic_name(
    topic_name,
    rcl_node_get_name(node),
    rcl_node_get_namespace(node),
    &substitutions_map,
    *allocator,
    &expanded_topic_name);
  rcutils_ret = rcutils_string_map_fini(&substitutions_map);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
    ret = RCL_RET_ERROR;
    goto cleanup;
  }
  if (ret != RCL_RET_OK) {
    if (ret == RCL_RET_TOPIC_NAME_INVALID || ret == RCL_RET_UNKNOWN_SUBSTITUTION) {
      ret = RCL_RET_TOPIC_NAME_INVALID;
    } else {
      ret = RCL_RET_ERROR;
    }
    goto cleanup;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Expanded topic name '%s'", expanded_topic_name);

  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (NULL == node_options) {
    ret = RCL_RET_ERROR;
    goto cleanup;
  }
  rcl_arguments_t * global_args = NULL;
  if (node_options->use_global_arguments) {
    global_args = &(node->context->global_arguments);
  }
  ret = rcl_remap_topic_name(
    &(node_options->arguments), global_args, expanded_topic_name,
    rcl_node_get_name(node), rcl_node_get_namespace(node), *allocator, &remapped_topic_name);
  if (RCL_RET_OK != ret) {
    goto fail;
  } else if (NULL == remapped_topic_name) {
    remapped_topic_name = expanded_topic_name;
    expanded_topic_name = NULL;
  }

  // Validate the expanded topic name.
  int validation_result;
  rmw_ret_t rmw_ret = rmw_validate_full_topic_name(remapped_topic_name, &validation_result, NULL);
  if (rmw_ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    ret = RCL_RET_ERROR;
    goto cleanup;
  }
  if (validation_result != RMW_TOPIC_VALID) {
    RCL_SET_ERROR_MSG(rmw_full_topic_name_validation_result_string(validation_result));
    ret = RCL_RET_TOPIC_NAME_INVALID;
    goto cleanup;
  }
  // Allocate space for the implementation struct.
  publisher->impl = (rcl_publisher_impl_t *)allocator->allocate(
    sizeof(rcl_publisher_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "allocating memory failed", ret = RCL_RET_BAD_ALLOC; goto cleanup);
  // Fill out implementation struct.
  // rmw handle (create rmw publisher)
  // TODO(wjwwood): pass along the allocator to rmw when it supports it
  publisher->impl->rmw_handle = rmw_create_publisher(
    rcl_node_get_rmw_handle(node),
    type_support,
    remapped_topic_name,
    &(options->qos));
  RCL_CHECK_FOR_NULL_WITH_MSG(publisher->impl->rmw_handle,
    rmw_get_error_string().str, goto fail);
  // options
  publisher->impl->options = *options;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Publisher initialized");
  goto cleanup;
fail:
  if (publisher->impl) {
    allocator->deallocate(publisher->impl, allocator->state);
  }
  ret = fail_ret;
  // Fall through to cleanup
cleanup:
  if (NULL != expanded_topic_name) {
    allocator->deallocate(expanded_topic_name, allocator->state);
  }
  if (NULL != remapped_topic_name) {
    allocator->deallocate(remapped_topic_name, allocator->state);
  }
  return ret;
}

rcl_ret_t
rcl_publisher_fini(rcl_publisher_t * publisher, rcl_node_t * node)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, RCL_RET_PUBLISHER_INVALID);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing publisher");
  if (publisher->impl) {
    rcl_allocator_t allocator = publisher->impl->options.allocator;
    rmw_node_t * rmw_node = rcl_node_get_rmw_handle(node);
    if (!rmw_node) {
      return RCL_RET_INVALID_ARGUMENT;
    }
    rmw_ret_t ret =
      rmw_destroy_publisher(rmw_node, publisher->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      result = RCL_RET_ERROR;
    }
    allocator.deallocate(publisher->impl, allocator.state);
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Publisher finalized");
  return result;
}

rcl_publisher_options_t
rcl_publisher_get_default_options()
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_publisher_options_t default_options;
  // Must set the allocator and qos after because they are not a compile time constant.
  default_options.qos = rmw_qos_profile_default;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

rcl_ret_t
rcl_publish(const rcl_publisher_t * publisher, const void * ros_message)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Publisher publishing message");
  if (!rcl_publisher_is_valid(publisher)) {
    return RCL_RET_PUBLISHER_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_message, RCL_RET_INVALID_ARGUMENT);
  if (rmw_publish(publisher->impl->rmw_handle, ros_message) != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_publish_serialized_message(
  const rcl_publisher_t * publisher, const rcl_serialized_message_t * serialized_message)
{
  if (!rcl_publisher_is_valid(publisher)) {
    return RCL_RET_PUBLISHER_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(serialized_message, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t ret = rmw_publish_serialized_message(publisher->impl->rmw_handle, serialized_message);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    if (ret == RMW_RET_BAD_ALLOC) {
      return RCL_RET_BAD_ALLOC;
    }
    return RMW_RET_ERROR;
  }
  return RCL_RET_OK;
}

const char *
rcl_publisher_get_topic_name(const rcl_publisher_t * publisher)
{
  if (!rcl_publisher_is_valid(publisher)) {
    return NULL;  // error already set
  }
  return publisher->impl->rmw_handle->topic_name;
}

#define _publisher_get_options(pub) & pub->impl->options

const rcl_publisher_options_t *
rcl_publisher_get_options(const rcl_publisher_t * publisher)
{
  if (!rcl_publisher_is_valid(publisher)) {
    return NULL;  // error already set
  }
  return _publisher_get_options(publisher);
}

rmw_publisher_t *
rcl_publisher_get_rmw_handle(const rcl_publisher_t * publisher)
{
  if (!rcl_publisher_is_valid(publisher)) {
    return NULL;  // error already set
  }
  return publisher->impl->rmw_handle;
}

bool
rcl_publisher_is_valid(const rcl_publisher_t * publisher)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(publisher, "publisher pointer is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "publisher implementation is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl->rmw_handle, "publisher's rmw handle is invalid", return false);
  return true;
}

rmw_ret_t
rcl_publisher_get_subscription_count(
  const rcl_publisher_t * publisher,
  size_t * subscription_count)
{
  if (!rcl_publisher_is_valid(publisher)) {
    return RCL_RET_PUBLISHER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription_count, RCL_RET_INVALID_ARGUMENT);

  rmw_ret_t ret = rmw_publisher_count_matched_subscriptions(publisher->impl->rmw_handle,
      subscription_count);

  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return rcl_convert_rmw_ret_to_rcl_ret(ret);
  }
  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
