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

#include "rmw/rmw.h"
#include "./common.h"

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
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  if (subscription->impl) {
    RCL_SET_ERROR_MSG("subscription already initialized, or memory was uninitialized");
    return RCL_RET_ALREADY_INIT;
  }
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Allocate memory for the implementation struct.
  subscription->impl = (rcl_subscription_impl_t *)allocator->allocate(
    sizeof(rcl_subscription_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  // Fill out the implemenation struct.
  // rmw_handle
  // TODO(wjwwood): pass allocator once supported in rmw api.
  subscription->impl->rmw_handle = rmw_create_subscription(
    rcl_node_get_rmw_handle(node),
    type_support,
    topic_name,
    &(options->qos),
    options->ignore_local_publications);
  if (!subscription->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
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
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (subscription->impl) {
    rmw_ret_t ret =
      rmw_destroy_subscription(rcl_node_get_rmw_handle(node), subscription->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
      result = RCL_RET_ERROR;
    }
    rcl_allocator_t allocator = subscription->impl->options.allocator;
    allocator.deallocate(subscription->impl, allocator.state);
  }
  return result;
}

rcl_subscription_options_t
rcl_subscription_get_default_options()
{
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
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_message, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "subscription is invalid", return RCL_RET_SUBSCRIPTION_INVALID);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl->rmw_handle,
    "subscription is invalid", return RCL_RET_SUBSCRIPTION_INVALID);
  // If message_info is NULL, use a place holder which can be discarded.
  rmw_message_info_t dummy_message_info;
  rmw_message_info_t * message_info_local = message_info ? message_info : &dummy_message_info;
  // Call rmw_take_with_info.
  bool taken = false;
  rmw_ret_t ret =
    rmw_take_with_info(subscription->impl->rmw_handle, ros_message, &taken, message_info_local);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  if (!taken) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_SUBSCRIPTION_TAKE_FAILED;
  }
  return RCL_RET_OK;
}

const char *
rcl_subscription_get_topic_name(const rcl_subscription_t * subscription)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "subscription is invalid", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl->rmw_handle,
    "subscription is invalid", return NULL);
  return subscription->impl->rmw_handle->topic_name;
}

const rcl_subscription_options_t *
rcl_subscription_get_options(const rcl_subscription_t * subscription)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "subscription is invalid", return NULL);
  return &subscription->impl->options;
}

rmw_subscription_t *
rcl_subscription_get_rmw_handle(const rcl_subscription_t * subscription)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription->impl, "subscription is invalid", return NULL);
  return subscription->impl->rmw_handle;
}

#if __cplusplus
}
#endif
