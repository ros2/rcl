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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/rcl.h"
#include "rmw/rmw.h"

#include "./common.h"

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  size_t actual_domain_id;
  rmw_node_t * rmw_node_handle;
  uint64_t rcl_instance_id;
  rcl_guard_condition_t * graph_guard_condition;
} rcl_node_impl_t;

rcl_node_t
rcl_get_zero_initialized_node()
{
  static rcl_node_t null_node = {0};
  return null_node;
}

rcl_strings_t
rcl_get_zero_initialized_strings()
{
  static rcl_strings_t null_strings;
  null_strings.count = 0;
  null_strings.data = NULL;
  null_strings.allocator = rcl_get_default_allocator();
  return null_strings;
}

rcl_ret_t
rcl_get_node_names(rcl_strings_t* strings)
{

  RCL_CHECK_ARGUMENT_FOR_NULL(strings, RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_FOR_NULL_WITH_MSG(
    strings->allocator.allocate,
    "invalid allocator, allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    strings->allocator.deallocate,
    "invalid allocator, deallocate not set", return RCL_RET_INVALID_ARGUMENT);

  rmw_ros_meta_t* ros_meta_data = rmw_get_node_names();

  strings->count = ros_meta_data->count;
  strings->data = (char **)strings->allocator.allocate(sizeof(char *) * strings->count, strings->allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(strings->data, "allocating memory failed", return RCL_RET_BAD_ALLOC);

  memset(strings->data, 0, sizeof(char **) * strings->count);
  int i;
  //copy the node names to the structure
  for(i = 0; i < ros_meta_data->count; i++){
    int len_string = strlen(ros_meta_data->node_names[i].data)+1;
    strings->data[i] = (char *)strings->allocator.allocate(sizeof(char)*len_string, strings->allocator.state);
    //TODO deallocate the rest of the array
    RCL_CHECK_FOR_NULL_WITH_MSG(strings->data[i], "allocating memory failed", return RCL_RET_BAD_ALLOC);
    memcpy(strings->data[i], ros_meta_data->node_names[i].data, len_string);
  }

  rmw_destroy_ros_meta(ros_meta_data);

  return RCL_RET_OK;
}

rcl_ret_t
rcl_node_init(rcl_node_t * node, const char * name, const rcl_node_options_t * options)
{
  size_t domain_id = 0;
  const char * ros_domain_id;
  const rmw_guard_condition_t * rmw_graph_guard_condition = NULL;
  rcl_guard_condition_options_t graph_guard_condition_options =
    rcl_guard_condition_get_default_options();
  rcl_ret_t ret;
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  RCL_CHECK_ARGUMENT_FOR_NULL(name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (node->impl) {
    RCL_SET_ERROR_MSG("node already initialized, or struct memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  // Make sure rcl has been initialized.
  if (!rcl_ok()) {
    RCL_SET_ERROR_MSG("rcl_init() has not been called");
    return RCL_RET_NOT_INIT;
  }
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Allocate space for the implementation struct.
  node->impl = (rcl_node_impl_t *)allocator->allocate(sizeof(rcl_node_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  node->impl->rmw_node_handle = NULL;
  node->impl->graph_guard_condition = NULL;
  // Initialize node impl.
  // node name
  size_t name_len = strlen(name);
  if (name_len == 0) {
    RCL_SET_ERROR_MSG("node name cannot be empty string");
    goto fail;
  }
  // node options (assume it is trivially copyable)
  node->impl->options = *options;
  // node rmw_node_handle
  if (node->impl->options.domain_id == RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID) {
    // Find the domain ID set by the environment.
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
  } else {
    domain_id = node->impl->options.domain_id;
  }
  // actual domain id
  node->impl->actual_domain_id = domain_id;
  node->impl->rmw_node_handle = rmw_create_node(name, domain_id);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->rmw_node_handle, rmw_get_error_string_safe(), goto fail);
  // instance id
  node->impl->rcl_instance_id = rcl_get_instance_id();
  // graph guard condition
  rmw_graph_guard_condition = rmw_node_get_graph_guard_condition(node->impl->rmw_node_handle);
  if (!rmw_graph_guard_condition) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    goto fail;
  }
  node->impl->graph_guard_condition = (rcl_guard_condition_t *)allocator->allocate(
    sizeof(rcl_guard_condition_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->graph_guard_condition,
    "allocating memory failed",
    goto fail
  );
  *node->impl->graph_guard_condition = rcl_get_zero_initialized_guard_condition();
  graph_guard_condition_options.allocator = *allocator;
  ret = rcl_guard_condition_init_from_rmw(
    node->impl->graph_guard_condition,
    rmw_graph_guard_condition,
    graph_guard_condition_options);
  if (ret != RCL_RET_OK) {
    // error message already set
    goto fail;
  }
  return RCL_RET_OK;
fail:
  if (node->impl) {
    if (node->impl->rmw_node_handle) {
      ret = rmw_destroy_node(node->impl->rmw_node_handle);
      if (ret != RMW_RET_OK) {
        fprintf(stderr,
          "failed to fini rmw node in error recovery: %s\n", rmw_get_error_string_safe()
        );
      }
    }
    if (node->impl->graph_guard_condition) {
      ret = rcl_guard_condition_fini(node->impl->graph_guard_condition);
      if (ret != RCL_RET_OK) {
        fprintf(stderr,
          "failed to fini guard condition in error recovery: %s\n", rcl_get_error_string_safe()
        );
      }
      allocator->deallocate(node->impl->graph_guard_condition, allocator->state);
    }
    allocator->deallocate(node->impl, allocator->state);
  }
  *node = rcl_get_zero_initialized_node();
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
  node->impl = NULL;
  return result;
}

bool
rcl_node_is_valid(const rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, false);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "rcl node implementation is invalid", return false);
  if (node->impl->rcl_instance_id != rcl_get_instance_id()) {
    RCL_SET_ERROR_MSG("rcl node is invalid, rcl instance id does not match");
    return false;
  }
  return true;
}

rcl_ret_t
rcl_strings_fini(rcl_strings_t* strings)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  rcl_allocator_t allocator = strings->allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
      allocator.deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);

  if (strings->data!=NULL) {
    fail_ret = RCL_RET_BAD_ALLOC;
    if (strings->data) {
      unsigned int i;
      for (i = 0; i < strings->count; ++i) {
        if (strings->data[i]) {
          // Use the old allocator.
          allocator.deallocate(strings->data[i], allocator.state);
        }
      }
      // Use the old allocator.
      allocator.deallocate(strings->data, allocator.state);
    }
    strings->count = 0;
    strings->data = NULL;
    return RCL_RET_OK;
  }
  return fail_ret;
}

rcl_node_options_t
rcl_node_get_default_options()
{
  static rcl_node_options_t default_options = {
    .domain_id = RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID,
  };
  // Must set the allocator after because it is not a compile time constant.
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

const char *
rcl_node_get_name(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node)) {
    return NULL;
  }
  return node->impl->rmw_node_handle->name;
}

const rcl_node_options_t *
rcl_node_get_options(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node)) {
    return NULL;
  }
  return &node->impl->options;
}

rcl_ret_t
rcl_node_get_domain_id(const rcl_node_t * node, size_t * domain_id)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(domain_id, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  *domain_id = node->impl->actual_domain_id;
  return RCL_RET_OK;
}

rmw_node_t *
rcl_node_get_rmw_handle(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node)) {
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

const struct rcl_guard_condition_t *
rcl_node_get_graph_guard_condition(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node)) {
    return NULL;
  }
  return node->impl->graph_guard_condition;
}

#if __cplusplus
}
#endif
