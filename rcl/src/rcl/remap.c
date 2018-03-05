// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include "rcl/remap.h"

#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcutils/strdup.h"
#include "rcutils/types/string_map.h"
#include "./arguments_impl.h"
#include "./remap_impl.h"

#if __cplusplus
extern "C"
{
#endif

rcl_remap_t
rcl_remap_get_zero_initialized()
{
  rcl_remap_t rule;
  rule.type = RCL_UNKNOWN_REMAP;
  rule.node_name = NULL;
  rule.match = NULL;
  rule.replacement = NULL;
  return rule;
}

rcl_ret_t
rcl_remap_fini(
  rcl_remap_t * rule,
  rcl_allocator_t allocator)
{
  if (NULL != rule->node_name) {
    allocator.deallocate(rule->node_name, allocator.state);
    rule->node_name = NULL;
  }
  if (NULL != rule->match) {
    allocator.deallocate(rule->match, allocator.state);
    rule->match = NULL;
  }
  if (NULL != rule->replacement) {
    allocator.deallocate(rule->replacement, allocator.state);
    rule->replacement = NULL;
  }
  return RCL_RET_OK;
}

/// Get the first matching rule in a chain
/// \return NULL if no rule in the chain matched
rcl_remap_t *
_rcl_remap_first_match(
  rcl_remap_t * remap_rules,
  int num_rules,
  rcl_remap_type_t type_bitmask,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  const rcutils_string_map_t * substitutions,
  rcutils_allocator_t allocator)
{
  for (int i = 0; i < num_rules; ++i) {
    rcl_remap_t * rule = &(remap_rules[i]);
    if (rule->type & type_bitmask) {
      if (NULL == rule->node_name || NULL == node_name || 0 == strcmp(rule->node_name, node_name)) {
        bool matched = false;
        if (rule->type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
          // topic and service rules need the match side to be expanded to a FQN
          char * expanded_match = NULL;
          rcl_ret_t ret = rcl_expand_topic_name(rule->match, node_name, node_namespace,
              substitutions, allocator, &expanded_match);
          if (ret != RCL_RET_OK) {
            // Expansion failed with this rule, try another
            continue;
          }
          matched = (0 == strcmp(expanded_match, name));
          allocator.deallocate(expanded_match, allocator.state);
        } else {
          // nodename and namespace replacement don't need the match side expanded
          matched = (NULL == rule->match || NULL == name || 0 == strcmp(rule->match, name));
        }
        if (matched) {
          return rule;
        }
      }
    }
  }
  return NULL;
}

/// Remap from one name to another using rules matching a given type bitmask
rcl_ret_t
_rcl_remap_name(
  const rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  rcl_remap_type_t type_bitmask,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  const rcutils_string_map_t * substitutions,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(output_name, RCL_RET_INVALID_ARGUMENT, allocator);
  if (NULL != local_arguments && NULL == local_arguments->impl) {
    local_arguments = NULL;
  }

  *output_name = NULL;
  rcl_remap_t * rule = NULL;

  // Look at local rules first
  if (NULL != local_arguments) {
    rule = _rcl_remap_first_match(local_arguments->impl->remap_rules,
        local_arguments->impl->num_remap_rules, type_bitmask, name, node_name, node_namespace,
        substitutions, allocator);
  }
  // Check global rules if no local rule matched
  if (NULL == rule && use_global_arguments) {
    rule = _rcl_remap_first_match(__rcl_arguments.impl->remap_rules,
        __rcl_arguments.impl->num_remap_rules, type_bitmask, name, node_name, node_namespace,
        substitutions, allocator);
  }
  // Do the remapping
  if (NULL != rule) {
    if (rule->type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
      // topic and service rules need the replacement to be expanded to a FQN
      rcl_ret_t ret = rcl_expand_topic_name(rule->replacement, node_name, node_namespace,
          substitutions, allocator, output_name);
      if (RCL_RET_OK != ret) {
        return ret;
      }
    } else {
      // nodename and namespace rules don't need replacment expanded
      *output_name = rcutils_strdup(rule->replacement, allocator);
    }
    if (NULL == output_name) {
      return RCL_RET_ERROR;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_remap_topic_name(
  const rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  rcutils_string_map_t substitutions = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions, 0, allocator);
  rcl_ret_t ret = RCL_RET_ERROR;
  if (RCUTILS_RET_OK == rcutils_ret) {
    ret = rcl_get_default_topic_name_substitutions(&substitutions);
    if (RCL_RET_OK == ret) {
      ret = _rcl_remap_name(local_arguments, use_global_arguments, RCL_TOPIC_REMAP, name,
          node_name, node_namespace, &substitutions, allocator, output_name);
    }
  }
  if (RCUTILS_RET_OK != rcutils_string_map_fini(&substitutions)) {
    return RCL_RET_ERROR;
  }
  return ret;
}

rcl_ret_t
rcl_remap_service_name(
  const rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(name, RCL_RET_INVALID_ARGUMENT, allocator);

  rcutils_string_map_t substitutions = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions, 0, allocator);
  rcl_ret_t ret = RCL_RET_ERROR;
  if (rcutils_ret == RCUTILS_RET_OK) {
    ret = rcl_get_default_topic_name_substitutions(&substitutions);
    if (ret == RCL_RET_OK) {
      ret = _rcl_remap_name(local_arguments, use_global_arguments, RCL_SERVICE_REMAP, name,
          node_name, node_namespace, &substitutions, allocator, output_name);
    }
  }
  if (RCUTILS_RET_OK != rcutils_string_map_fini(&substitutions)) {
    return RCL_RET_ERROR;
  }
  return ret;
}

rcl_ret_t
rcl_remap_node_name(
  const rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  return _rcl_remap_name(local_arguments, use_global_arguments, RCL_NODENAME_REMAP, NULL, node_name,
           NULL, NULL, allocator, output_name);
}

rcl_ret_t
rcl_remap_node_namespace(
  const rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_namespace)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  return _rcl_remap_name(local_arguments, use_global_arguments, RCL_NAMESPACE_REMAP, NULL,
           node_name, NULL, NULL, allocator, output_namespace);
}

#if __cplusplus
}
#endif
