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

#if __cplusplus
extern "C"
{
#endif

#include "rcl/subscription.h"

#include <stdio.h>

#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
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
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT, *allocator);
  if (subscription->impl) {
    RCL_SET_ERROR_MSG("subscription already initialized, or memory was uninitialized", *allocator);
    return RCL_RET_ALREADY_INIT;
  }
  // Expand the given topic name.
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
  char * expanded_topic_name = NULL;
  ret = rcl_expand_topic_name(
    topic_name,
    rcl_node_get_name(node),
    rcl_node_get_namespace(node),
    &substitutions_map,
    *allocator,
    &expanded_topic_name);
  rcutils_ret = rcutils_string_map_fini(&substitutions_map);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string_safe(), *allocator)
    allocator->deallocate(expanded_topic_name, allocator->state);
    return RCL_RET_ERROR;
  }
  if (ret != RCL_RET_OK) {
    if (ret == RCL_RET_BAD_ALLOC) {
      return ret;
    } else if (ret == RCL_RET_TOPIC_NAME_INVALID || ret == RCL_RET_UNKNOWN_SUBSTITUTION) {
      return RCL_RET_TOPIC_NAME_INVALID;
    } else {
      return RCL_RET_ERROR;
    }
  }
  // Validate the expanded topic name.
  int validation_result;
  rmw_ret_t rmw_ret = rmw_validate_full_topic_name(expanded_topic_name, &validation_result, NULL);
  if (rmw_ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    return RCL_RET_ERROR;
  }
  if (validation_result != RMW_TOPIC_VALID) {
    RCL_SET_ERROR_MSG(rmw_full_topic_name_validation_result_string(validation_result), *allocator)
    return RCL_RET_TOPIC_NAME_INVALID;
  }
  // Allocate memory for the implementation struct.
  subscription->impl = (rcl_subscription_impl_t *)allocator->allocate(
    sizeof(rcl_subscription_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC, *allocator);
  // Fill out the implemenation struct.
  // rmw_handle
  // TODO(wjwwood): pass allocator once supported in rmw api.
  subscription->impl->rmw_handle = rmw_create_subscription(
    rcl_node_get_rmw_handle(node),
    type_support,
    expanded_topic_name,
    &(options->qos),
    options->ignore_local_publications);
  allocator->deallocate(expanded_topic_name, allocator->state);
  if (!subscription->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    goto fail;
  }
  // options
  subscription->impl->options = *options;
  return RCL_RET_OK;
fail:
  if (subscription->impl) {
    allocator->deallocate(subscription->impl, allocator->state);
  }
  return fail_ret;
}

rcl_ret_t
rcl_subscription_fini(rcl_subscription_t * subscription, rcl_node_t * node)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (subscription->impl) {
    rcl_allocator_t allocator = subscription->impl->options.allocator;
    rmw_node_t * rmw_node = rcl_node_get_rmw_handle(node);
    if (!rmw_node) {
      return RCL_RET_INVALID_ARGUMENT;
    }
    rmw_ret_t ret =
      rmw_destroy_subscription(rmw_node, subscription->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), allocator);
      result = RCL_RET_ERROR;
    }
    allocator.deallocate(subscription->impl, allocator.state);
  }
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
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_subscription_options_t * options = rcl_subscription_get_options(subscription);
  if (!options) {
    return RCL_RET_SUBSCRIPTION_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_message, RCL_RET_INVALID_ARGUMENT, options->allocator);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "subscription is invalid",
    return RCL_RET_SUBSCRIPTION_INVALID, options->allocator);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl->rmw_handle,
    "subscription is invalid", return RCL_RET_SUBSCRIPTION_INVALID, options->allocator);
  // If message_info is NULL, use a place holder which can be discarded.
  rmw_message_info_t dummy_message_info;
  rmw_message_info_t * message_info_local = message_info ? message_info : &dummy_message_info;
  // Call rmw_take_with_info.
  bool taken = false;
  rmw_ret_t ret =
    rmw_take_with_info(subscription->impl->rmw_handle, ros_message, &taken, message_info_local);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), options->allocator);
    return RCL_RET_ERROR;
  }
  if (!taken) {
    return RCL_RET_SUBSCRIPTION_TAKE_FAILED;
  }
  return RCL_RET_OK;
}

const char *
rcl_subscription_get_topic_name(const rcl_subscription_t * subscription)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return NULL;
  }
  return subscription->impl->rmw_handle->topic_name;
}

/* *INDENT-OFF* */
#define _subscription_get_options(subscription) &subscription->impl->options
/* *INDENT-ON* */

const rcl_subscription_options_t *
rcl_subscription_get_options(const rcl_subscription_t * subscription)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return NULL;
  }
  return _subscription_get_options(subscription);
}

rmw_subscription_t *
rcl_subscription_get_rmw_handle(const rcl_subscription_t * subscription)
{
  if (!rcl_subscription_is_valid(subscription)) {
    return NULL;
  }
  return subscription->impl->rmw_handle;
}

bool
rcl_subscription_is_valid(const rcl_subscription_t * subscription)
{
  const rcl_subscription_options_t * options;
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, false, rcl_get_default_allocator());
  options = _subscription_get_options(subscription);
  RCL_CHECK_FOR_NULL_WITH_MSG(options,
    "subscription's option pointer is invalid",
    return false,
    rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(subscription->impl,
    "subscription implementation is invalid",
    return false,
    options->allocator);
  RCL_CHECK_FOR_NULL_WITH_MSG(subscription->impl->rmw_handle,
    "subscription implementation rmw_handle is invalid",
    return false,
    options->allocator);
  return true;
}

#if __cplusplus
}
#endif
