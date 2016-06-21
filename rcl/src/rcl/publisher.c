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

#include "rcl/publisher.h"

#include <string.h>

#include "./common.h"
#include "rmw/rmw.h"

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
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!node->impl) {
    RCL_SET_ERROR_MSG("invalid node");
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  if (publisher->impl) {
    RCL_SET_ERROR_MSG("publisher already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Allocate space for the implementation struct.
  publisher->impl = (rcl_publisher_impl_t *)allocator->allocate(
    sizeof(rcl_publisher_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  // Fill out implementation struct.
  // rmw handle (create rmw publisher)
  // TODO(wjwwood): pass along the allocator to rmw when it supports it
  publisher->impl->rmw_handle = rmw_create_publisher(
    rcl_node_get_rmw_handle(node),
    type_support,
    topic_name,
    &(options->qos));
  if (!publisher->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    goto fail;
  }
  // options
  publisher->impl->options = *options;
  return RCL_RET_OK;
fail:
  if (publisher->impl) {
    allocator->deallocate(publisher->impl, allocator->state);
  }
  return fail_ret;
}

rcl_ret_t
rcl_publisher_fini(rcl_publisher_t * publisher, rcl_node_t * node)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (publisher->impl) {
    rmw_ret_t ret =
      rmw_destroy_publisher(rcl_node_get_rmw_handle(node), publisher->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
      result = RCL_RET_ERROR;
    }
    rcl_allocator_t allocator = publisher->impl->options.allocator;
    allocator.deallocate(publisher->impl, allocator.state);
  }
  return result;
}

rcl_publisher_options_t
rcl_publisher_get_default_options()
{
  static rcl_publisher_options_t default_options;
  // Must set the allocator and qos after because they are not a compile time constant.
  default_options.qos = rmw_qos_profile_default;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

rcl_ret_t
rcl_publish(const rcl_publisher_t * publisher, const void * ros_message)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_message, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "publisher is invalid", return RCL_RET_PUBLISHER_INVALID);
  if (rmw_publish(publisher->impl->rmw_handle, ros_message) != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

const char *
rcl_publisher_get_topic_name(const rcl_publisher_t * publisher)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "publisher is invalid", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl->rmw_handle, "publisher is invalid", return NULL);
  return publisher->impl->rmw_handle->topic_name;
}

const rcl_publisher_options_t *
rcl_publisher_get_options(const rcl_publisher_t * publisher)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "publisher is invalid", return NULL);
  return &publisher->impl->options;
}

rmw_publisher_t *
rcl_publisher_get_rmw_handle(const rcl_publisher_t * publisher)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(publisher, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher->impl, "publisher is invalid", return NULL);
  return publisher->impl->rmw_handle;
}

#if __cplusplus
}
#endif
