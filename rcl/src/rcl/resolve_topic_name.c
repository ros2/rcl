// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rcl/resolve_topic_name.h"

#include "rcutils/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/types/string_map.h"

#include "rmw/error_handling.h"
#include "rmw/validate_full_topic_name.h"

#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcl/remap.h"

rcl_ret_t
rcl_resolve_topic_name(
  const rcl_arguments_t * local_args,
  const rcl_arguments_t * global_args,
  const char * input_topic_name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  bool only_expand,
  char ** output_topic_name)
{
  // the other arguments are checked by rcl_expand_topic_name() and rcl_remap_topic_name()
  RCL_CHECK_ARGUMENT_FOR_NULL(output_topic_name, RCL_RET_INVALID_ARGUMENT);
  // Create default topic name substitutions map
  rcutils_string_map_t substitutions_map = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions_map, 0, allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    const char * error = rcutils_get_error_string().str;
    rcutils_reset_error();
    RCL_SET_ERROR_MSG(error);
    if (RCUTILS_RET_BAD_ALLOC == rcutils_ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  rcl_ret_t ret = rcl_get_default_topic_name_substitutions(&substitutions_map);
  if (ret != RCL_RET_OK) {
    if (RCL_RET_BAD_ALLOC != ret) {
      ret = RCL_RET_ERROR;
    }
    goto cleanup;
  }
  // expand topic name
  char * expanded_topic_name = NULL;
  char * remapped_topic_name = NULL;
  ret = rcl_expand_topic_name(
    input_topic_name,
    node_name,
    node_namespace,
    &substitutions_map,
    allocator,
    &expanded_topic_name);
  if (RCL_RET_OK != ret) {
    goto cleanup;
  }
  if (!only_expand) {
    // remap topic name
    ret = rcl_remap_topic_name(
      local_args, global_args, expanded_topic_name,
      node_name, node_namespace, allocator, &remapped_topic_name);
    if (RCL_RET_OK != ret) {
      goto cleanup;
    }
  }
  if (NULL == remapped_topic_name) {
    remapped_topic_name = expanded_topic_name;
    expanded_topic_name = NULL;
  }
  // validate the result
  int validation_result;
  rmw_ret_t rmw_ret = rmw_validate_full_topic_name(remapped_topic_name, &validation_result, NULL);
  if (rmw_ret != RMW_RET_OK) {
    const char * error = rmw_get_error_string().str;
    rmw_reset_error();
    RCL_SET_ERROR_MSG(error);
    ret = RCL_RET_ERROR;
    goto cleanup;
  }
  if (validation_result != RMW_TOPIC_VALID) {
    RCL_SET_ERROR_MSG(rmw_full_topic_name_validation_result_string(validation_result));
    ret = RCL_RET_TOPIC_NAME_INVALID;
    goto cleanup;
  }
  *output_topic_name = remapped_topic_name;
  return RCL_RET_OK;

cleanup:
  rcutils_ret = rcutils_string_map_fini(&substitutions_map);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME,
      "failed to fini string_map (%d) during error handling: %s",
      rcutils_ret,
      rcutils_get_error_string().str);
    rcutils_reset_error();
  }
  allocator.deallocate(expanded_topic_name, allocator.state);
  return ret;
}
