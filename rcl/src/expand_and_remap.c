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

#include "rcl/expand_and_remap.h"

#include "rcutils/logging_macros.h"

#include "rcl/expand_topic_name.h"
#include "rcl/remap.h"

rcl_ret_t
rcl_expand_and_remap_name(
  const char * input_topic_name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t * allocator,
  bool is_service,
  char ** output_topic_name)
{
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
  ret = rcl_expand_topic_name(
    input_topic_name,
    node_name,
    node_namespace,
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
}
