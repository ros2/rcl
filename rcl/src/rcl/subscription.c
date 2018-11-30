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

#include "rcl/subscription.h"

#include <stdio.h>

#include "./common.h"
#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcl/remap.h"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"

typedef struct rcl_subscription_impl_t
{
  rcl_subscription_options_t options;
  rmw_subscription_t * rmw_handle;
} rcl_subscription_impl_t;

rcl_subscription_t
rcl_get_zero_initialized_subscription()
{
  static rcl_subscription_t null_subscription = {0};
  return null_subscription;
}

rcl_ret_t
rcl_subscription_init(
  rcl_subscription_t * subscription,
  const rcl_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rcl_subscription_options_t * options)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  // Check options and allocator first, so the allocator can be used in errors.
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t * allocator = (rcl_allocator_t *)&options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing subscription for topic name '%s'", topic_name);
  if (subscription->impl) {
    RCL_SET_ERROR_MSG("subscription already initialized, or memory was uninitialized");
    return RCL_RET_ALREADY_INIT;
  }
  // Expand the given topic name.
  rcutils_allocator_t rcutils_allocator = *allocator;  // implicit conversion to rcutils version
  rcutils_string_map_t substitutions_map = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions_map, 0, rcutils_allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
    if (RCUTILS_RET_BAD_ALLOC == rcutils_ret) {
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
    if (RCL_RET_BAD_ALLOC == ret) {
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
  // Allocate memory for the implementation struct.
  subscription->impl = (rcl_subscription_impl_t *)allocator->allocate(
    sizeof(rcl_subscription_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "allocating memory failed", ret = RCL_RET_BAD_ALLOC; goto cleanup);
  // Fill out the implemenation struct.
  // rmw_handle
  // TODO(wjwwood): pass allocator once supported in rmw api.
  subscription->impl->rmw_handle = rmw_create_subscription(
    rcl_node_get_rmw_handle(node),
    type_support,
    remapped_topic_name,
    &(options->qos),
    options->ignore_local_publications);
  if (!subscription->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    goto fail;
  }
  // options
  subscription->impl->options = *options;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Subscription initialized");
  ret = RCL_RET_OK;
  goto cleanup;
fail:
  if (subscription->impl) {
    allocator->deallocate(subscription->impl, allocator->state);
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
rcl_subscription_fini(rcl_subscription_t * subscription, rcl_node_t * node)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing subscription");
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_SUBSCRIPTION_INVALID);
  if (!rcl_node_is_valid_except_context(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  if (subscription->impl) {
    rcl_allocator_t allocator = subscription->impl->options.allocator;
    rmw_node_t * rmw_node = rcl_node_get_rmw_handle(node);
    if (!rmw_node) {
      return RCL_RET_INVALID_ARGUMENT;
    }
    rmw_ret_t ret =
      rmw_destroy_subscription(rmw_node, subscription->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string().str);
      result = RCL_RET_ERROR;
    }
    allocator.deallocate(subscription->impl, allocator.state);
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Subscription finalized");
  return result;
}

rcl_subscription_options_t
rcl_subscription_get_default_options()
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_subscription_options_t default_options = {
    .ignore_local_publications = false,
  };
  // Must set the allocator and qos after because they are not a compile time constant.
  default_options.qos = rmw_qos_profile_default;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

rcl_ret_t
rcl_take(
  const rcl_subscription_t * subscription,
  void * ros_message,
  rmw_message_info_t * message_info)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Subscription taking message");
  if (!rcl_subscription_is_valid(subscription)) {
    return RCL_RET_SUBSCRIPTION_INVALID;  // error message already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_message, RCL_RET_INVALID_ARGUMENT);

  // If message_info is NULL, use a place holder which can be discarded.
  rmw_message_info_t dummy_message_info;
  rmw_message_info_t * message_info_local = message_info ? message_info : &dummy_message_info;
  // Call rmw_take_with_info.
  bool taken = false;
  rmw_ret_t ret =
    rmw_take_with_info(subscription->impl->rmw_handle, ros_message, &taken, message_info_local);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    if (RMW_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Subscription take succeeded: %s", taken ? "true" : "false");
  if (!taken) {
    return RCL_RET_SUBSCRIPTION_TAKE_FAILED;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_take_serialized_message(
  const rcl_subscription_t * subscription,
  rcl_serialized_message_t * serialized_message,
  rmw_message_info_t * message_info)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Subscription taking serialized message");
  if (!rcl_subscription_is_valid(subscription)) {
    return RCL_RET_SUBSCRIPTION_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(serialized_message, RCL_RET_INVALID_ARGUMENT);
  // If message_info is NULL, use a place holder which can be discarded.
  rmw_message_info_t dummy_message_info;
  rmw_message_info_t * message_info_local = message_info ? message_info : &dummy_message_info;
  // Call rmw_take_with_info.
  bool taken = false;
  rmw_ret_t ret = rmw_take_serialized_message_with_info(
    subscription->impl->rmw_handle, serialized_message, &taken, message_info_local);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    if (RMW_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Subscription serialized take succeeded: %s", taken ? "true" : "false");
  if (!taken) {
    return RCL_RET_SUBSCRIPTION_TAKE_FAILED;
  }
  return RCL_RET_OK;
}

const char *
rcl_subscription_get_topic_name(const rcl_subscription_t * subscription)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return NULL;  // error already set
  }
  return subscription->impl->rmw_handle->topic_name;
}

#define _subscription_get_options(subscription) & subscription->impl->options

const rcl_subscription_options_t *
rcl_subscription_get_options(const rcl_subscription_t * subscription)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return NULL;  // error already set
  }
  return _subscription_get_options(subscription);
}

rmw_subscription_t *
rcl_subscription_get_rmw_handle(const rcl_subscription_t * subscription)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return NULL;  // error already  set
  }
  return subscription->impl->rmw_handle;
}

bool
rcl_subscription_is_valid(const rcl_subscription_t * subscription)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(subscription, "subscription pointer is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "subscription's implementation is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl->rmw_handle, "subscription's rmw handle is invalid", return false);
  return true;
}

rmw_ret_t
rcl_subscription_get_publisher_count(
  const rcl_subscription_t * subscription,
  size_t * publisher_count)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return RCL_RET_SUBSCRIPTION_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher_count, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t ret = rmw_subscription_count_matched_publishers(subscription->impl->rmw_handle,
      publisher_count);

  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return rcl_convert_rmw_ret_to_rcl_ret(ret);
  }
  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
