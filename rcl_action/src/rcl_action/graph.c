// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include <assert.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/node.h"
#include "rcutils/strdup.h"

#include "rcl_action/graph.h"

static
rcl_ret_t
_filter_action_names(
  rcl_names_and_types_t * topic_names_and_types,
  rcl_allocator_t * allocator,
  rcl_names_and_types_t * action_names_and_types)
{
  assert(topic_names_and_types);
  assert(allocator);
  assert(action_names_and_types);

  // Assumption: actions provide a topic name with the suffix "/_action/feedback"
  // and it has type with the suffix "_FeedbackMessage"
  const char * action_name_suffix = "/_action/feedback";
  const char * action_type_suffix = "_FeedbackMessage";

  rcl_ret_t ret;
  const size_t num_names = topic_names_and_types->names.size;
  char ** names = topic_names_and_types->names.data;

  // Count number of actions to determine how much memory to allocate
  size_t num_actions = 0u;
  for (size_t i = 0u; i < num_names; ++i) {
    const char * suffix_index = strstr(names[i], action_name_suffix);
    if (suffix_index) {
      num_actions++;
    }
  }

  if (0u == num_actions) {
    return RCL_RET_OK;
  }

  ret = rcl_names_and_types_init(action_names_and_types, num_actions, allocator);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = RCL_RET_OK;

  // Prune names/types that are not actions (ie. do not contain the suffix)
  const size_t suffix_len = strlen(action_name_suffix);
  size_t j = 0u;
  for (size_t i = 0u; i < num_names; ++i) {
    const char * suffix_index = strstr(names[i], action_name_suffix);
    if (suffix_index) {
      const size_t action_name_len = strlen(names[i]) - suffix_len;
      char * action_name = rcutils_strndup(names[i], action_name_len, *allocator);
      if (!action_name) {
        RCL_SET_ERROR_MSG("Failed to allocate memory for action name");
        ret = RCL_RET_BAD_ALLOC;
        break;
      }

      action_names_and_types->names.data[j] = action_name;

      // Allocate storage for type list
      rcutils_ret_t rcutils_ret = rcutils_string_array_init(
        &action_names_and_types->types[j],
        topic_names_and_types->types[i].size,
        allocator);
      if (RCUTILS_RET_OK != rcutils_ret) {
        RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
        ret = RCL_RET_BAD_ALLOC;
        break;
      }

      // Populate types list
      for (size_t k = 0u; k < topic_names_and_types->types[i].size; ++k) {
        char * type_name = topic_names_and_types->types[i].data[k];
        size_t action_type_len = strlen(type_name);
        // Trim type name suffix, if provided
        if (action_type_suffix) {
          const size_t type_suffix_len = strlen(action_type_suffix);
          const char * type_suffix_index = strstr(type_name, action_type_suffix);
          if (type_suffix_index) {
            action_type_len = strlen(type_name) - type_suffix_len;
          }
        }
        char * action_type_name = rcutils_strndup(type_name, action_type_len, *allocator);
        if (!action_type_name) {
          RCL_SET_ERROR_MSG("Failed to allocate memory for action type");
          ret = RCL_RET_BAD_ALLOC;
          break;
        }
        action_names_and_types->types[j].data[k] = action_type_name;
      }
      j++;
    }
  }

  // Cleanup if there is an error
  if (RCL_RET_OK != ret) {
    rcl_ret_t fini_ret = rcl_names_and_types_fini(action_names_and_types);
    (void)fini_ret;  // Error already set
  }

  return ret;
}

rcl_ret_t
rcl_action_get_client_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * action_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_names_and_types, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();
  ret = rcl_get_subscriber_names_and_types_by_node(
    node, allocator, false, node_name, node_namespace, &topic_names_and_types);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _filter_action_names(
    &topic_names_and_types,
    allocator,
    action_names_and_types);
  return ret;
}

rcl_ret_t
rcl_action_get_server_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * action_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_names_and_types, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();
  ret = rcl_get_publisher_names_and_types_by_node(
    node, allocator, false, node_name, node_namespace, &topic_names_and_types);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _filter_action_names(
    &topic_names_and_types,
    allocator,
    action_names_and_types);
  return ret;
}

rcl_ret_t
rcl_action_get_names_and_types(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  rcl_names_and_types_t * action_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_names_and_types, RCL_RET_INVALID_ARGUMENT);
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();
  rcl_ret_t ret = rcl_get_topic_names_and_types(node, allocator, false, &topic_names_and_types);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _filter_action_names(
    &topic_names_and_types,
    allocator,
    action_names_and_types);
  return ret;
}

#ifdef __cplusplus
}
#endif
