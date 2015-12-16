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

#include "rcl/node.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/rcl.h"
#include "rmw/rmw.h"

#include "./common.h"

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  rmw_node_t * rmw_node_handle;
  uint64_t rcl_instance_id;
} rcl_node_impl_t;

rcl_node_t
rcl_get_uninitialized_node()
{
  static rcl_node_t null_node = {0};
  return null_node;
}

rcl_ret_t
rcl_node_init(rcl_node_t * node, const char * name, const rcl_node_options_t * options)
{
  size_t domain_id = 0;
  const char * ros_domain_id;
  rcl_ret_t ret;
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  RCL_CHECK_ARGUMENT_FOR_NULL(name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (node->impl) {
    RCL_SET_ERROR_MSG("node already initialized, or struct memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Allocate space for the implementation struct.
  node->impl = (rcl_node_impl_t *)allocator->allocate(sizeof(rcl_node_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  // Initialize node impl.
  // node name
  size_t name_len = strlen(name);
  if (name_len == 0) {
    RCL_SET_ERROR_MSG("node name cannot be empty string");
    goto fail;
  }
  // node options
  node->impl->options = *options;
  // node rmw_node_handle
  // First determine the ROS_DOMAIN_ID.
  // The result of rcl_impl_getenv on Windows is only valid until the next call to rcl_impl_getenv.
  ret = rcl_impl_getenv("ROS_DOMAIN_ID", &ros_domain_id);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  if (ros_domain_id) {
    unsigned long number = strtoul(ros_domain_id, NULL, 0);  // NOLINT(runtime/int)
    if (number == ULONG_MAX) {
      RCL_SET_ERROR_MSG("failed to interpret ROS_DOMAIN_ID as integral number");
      goto fail;
    }
    domain_id = (size_t)number;
  }
  node->impl->rmw_node_handle = rmw_create_node(name, domain_id);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->rmw_node_handle, rmw_get_error_string_safe(), goto fail);
  node->impl->rcl_instance_id = rcl_get_instance_id();
  return RCL_RET_OK;
fail:
  if (node->impl) {
    allocator->deallocate(node->impl, allocator->state);
  }
  return fail_ret;
}

rcl_ret_t
rcl_node_fini(rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!node->impl) {
    // Repeat calls to fini or calling fini on a zero initialized node is ok.
    return RCL_RET_OK;
  }
  rcl_ret_t result = RCL_RET_OK;
  rmw_ret_t ret = rmw_destroy_node(node->impl->rmw_node_handle);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    result = RCL_RET_ERROR;
  }
  rcl_allocator_t allocator = node->impl->options.allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator.deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  allocator.deallocate(node->impl, allocator.state);
  return result;
}

rcl_node_options_t
rcl_node_get_default_options()
{
  static rcl_node_options_t default_options = {
    .no_parameters = false,
    .domain_id = RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID,
  };
  // Must set the allocator after because it is not a compile time constant.
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

const char *
rcl_node_get_name(const rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "node implementation is invalid", return NULL);
  if (node->impl->rcl_instance_id != rcl_get_instance_id()) {
    RCL_SET_ERROR_MSG("rcl node is invalid, rcl instance id does not match");
    return NULL;
  }
  return node->impl->rmw_node_handle->name;
}

const rcl_node_options_t *
rcl_node_get_options(const rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "node implementation is invalid", return NULL);
  if (node->impl->rcl_instance_id != rcl_get_instance_id()) {
    RCL_SET_ERROR_MSG("rcl node is invalid, rcl instance id does not match");
    return NULL;
  }
  return &node->impl->options;
}

rmw_node_t *
rcl_node_get_rmw_handle(const rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "node implementation is invalid", return NULL);
  if (node->impl->rcl_instance_id != rcl_get_instance_id()) {
    RCL_SET_ERROR_MSG("rcl node is invalid, rcl instance id does not match");
    return NULL;
  }
  return node->impl->rmw_node_handle;
}

uint64_t
rcl_node_get_rcl_instance_id(const rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, 0);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "node implementation is invalid", return 0);
  return node->impl->rcl_instance_id;
}

#if __cplusplus
}
#endif
