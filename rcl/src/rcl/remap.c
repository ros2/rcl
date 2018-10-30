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

#include "./arguments_impl.h"
#include "./remap_impl.h"
#include "rcl/error_handling.h"
#include "rcl/expand_topic_name.h"
#include "rcutils/allocator.h"
#include "rcutils/strdup.h"
#include "rcutils/types/string_map.h"

#ifdef __cplusplus
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
  rule.allocator = rcutils_get_zero_initialized_allocator();
  return rule;
}

rcl_ret_t
rcl_remap_copy(
  const rcl_remap_t * rule,
  rcl_remap_t * rule_out)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(rule, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(rule_out, RCL_RET_INVALID_ARGUMENT);

  rcl_allocator_t allocator = rule->allocator;
  rule_out->allocator = allocator;
  rule_out->type = rule->type;
  if (NULL != rule->node_name) {
    rule_out->node_name = rcutils_strdup(rule->node_name, allocator);
    if (NULL == rule_out->node_name) {
      goto fail;
    }
  }
  if (NULL != rule->match) {
    rule_out->match = rcutils_strdup(rule->match, allocator);
    if (NULL == rule_out->match) {
      goto fail;
    }
  }
  if (NULL != rule->replacement) {
    rule_out->replacement = rcutils_strdup(rule->replacement, allocator);
    if (NULL == rule_out->replacement) {
      goto fail;
    }
  }
  return RCL_RET_OK;
fail:
  if (RCL_RET_OK != rcl_remap_fini(rule_out)) {
    RCL_SET_ERROR_MSG("Error while finalizing remap rule due to another error");
  }
  return RCL_RET_BAD_ALLOC;
}

rcl_ret_t
rcl_remap_fini(
  rcl_remap_t * rule)
{
  if (NULL != rule->node_name) {
    rule->allocator.deallocate(rule->node_name, rule->allocator.state);
    rule->node_name = NULL;
  }
  if (NULL != rule->match) {
    rule->allocator.deallocate(rule->match, rule->allocator.state);
    rule->match = NULL;
  }
  if (NULL != rule->replacement) {
    rule->allocator.deallocate(rule->replacement, rule->allocator.state);
    rule->replacement = NULL;
  }
  rule->allocator = rcutils_get_zero_initialized_allocator();
  return RCL_RET_OK;
}

/// Get the first matching rule in a chain.
/// \return RCL_RET_OK if no errors occurred while searching for a rule
RCL_LOCAL
rcl_ret_t
_rcl_remap_first_match(
  rcl_remap_t * remap_rules,
  int num_rules,
  rcl_remap_type_t type_bitmask,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  const rcutils_string_map_t * substitutions,
  rcutils_allocator_t allocator,
  rcl_remap_t ** output_rule)
{
  *output_rule = NULL;
  for (int i = 0; i < num_rules; ++i) {
    rcl_remap_t * rule = &(remap_rules[i]);
    if (!(rule->type & type_bitmask)) {
      // Not the type of remap rule we're looking fore
      continue;
    }
    if (rule->node_name != NULL && 0 != strcmp(rule->node_name, node_name)) {
      // Rule has a node name prefix and the supplied node name didn't match
      continue;
    }
    bool matched = false;
    if (rule->type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
      // topic and service rules need the match side to be expanded to a FQN
      char * expanded_match = NULL;
      rcl_ret_t ret = rcl_expand_topic_name(
        rule->match, node_name, node_namespace, substitutions, allocator, &expanded_match);
      if (RCL_RET_OK != ret) {
        rcl_reset_error();
        if (
          RCL_RET_NODE_INVALID_NAMESPACE == ret ||
          RCL_RET_NODE_INVALID_NAME == ret ||
          RCL_RET_BAD_ALLOC == ret)
        {
          // these are probably going to happen again. Stop processing rules
          return ret;
        }
        continue;
      }
      matched = (0 == strcmp(expanded_match, name));
      allocator.deallocate(expanded_match, allocator.state);
    } else {
      // nodename and namespace replacement apply if the type and node name prefix checks passed
      matched = true;
    }
    if (matched) {
      *output_rule = rule;
      break;
    }
  }
  return RCL_RET_OK;
}

/// Remap from one name to another using rules matching a given type bitmask.
RCL_LOCAL
rcl_ret_t
_rcl_remap_name(
  const rcl_arguments_t * local_arguments,
  const rcl_arguments_t * global_arguments,
  rcl_remap_type_t type_bitmask,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  const rcutils_string_map_t * substitutions,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_name, RCL_RET_INVALID_ARGUMENT);
  if (NULL != local_arguments && NULL == local_arguments->impl) {
    local_arguments = NULL;
  }
  if (NULL != global_arguments && NULL == global_arguments->impl) {
    global_arguments = NULL;
  }
  if (NULL == local_arguments && NULL == global_arguments) {
    RCL_SET_ERROR_MSG("local_arguments invalid and not using global arguments");
    return RCL_RET_INVALID_ARGUMENT;
  }

  *output_name = NULL;
  rcl_remap_t * rule = NULL;

  // Look at local rules first
  if (NULL != local_arguments) {
    rcl_ret_t ret = _rcl_remap_first_match(
      local_arguments->impl->remap_rules, local_arguments->impl->num_remap_rules, type_bitmask,
      name, node_name, node_namespace, substitutions, allocator, &rule);
    if (ret != RCL_RET_OK) {
      return ret;
    }
  }
  // Check global rules if no local rule matched
  if (NULL == rule && NULL != global_arguments) {
    rcl_ret_t ret = _rcl_remap_first_match(
      global_arguments->impl->remap_rules, global_arguments->impl->num_remap_rules, type_bitmask,
      name, node_name, node_namespace, substitutions, allocator, &rule);
    if (ret != RCL_RET_OK) {
      return ret;
    }
  }
  // Do the remapping
  if (NULL != rule) {
    if (rule->type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
      // topic and service rules need the replacement to be expanded to a FQN
      rcl_ret_t ret = rcl_expand_topic_name(
        rule->replacement, node_name, node_namespace, substitutions, allocator, output_name);
      if (RCL_RET_OK != ret) {
        return ret;
      }
    } else {
      // nodename and namespace rules don't need replacment expanded
      *output_name = rcutils_strdup(rule->replacement, allocator);
    }
    if (NULL == *output_name) {
      RCL_SET_ERROR_MSG("Failed to set output");
      return RCL_RET_ERROR;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_remap_topic_name(
  const rcl_arguments_t * local_arguments,
  const rcl_arguments_t * global_arguments,
  const char * topic_name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);

  rcutils_string_map_t substitutions = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions, 0, allocator);
  rcl_ret_t ret = RCL_RET_ERROR;
  if (RCUTILS_RET_OK == rcutils_ret) {
    ret = rcl_get_default_topic_name_substitutions(&substitutions);
    if (RCL_RET_OK == ret) {
      ret = _rcl_remap_name(
        local_arguments, global_arguments, RCL_TOPIC_REMAP, topic_name, node_name,
        node_namespace, &substitutions, allocator, output_name);
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
  const rcl_arguments_t * global_arguments,
  const char * service_name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_name, RCL_RET_INVALID_ARGUMENT);

  rcutils_string_map_t substitutions = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcutils_ret = rcutils_string_map_init(&substitutions, 0, allocator);
  rcl_ret_t ret = RCL_RET_ERROR;
  if (rcutils_ret == RCUTILS_RET_OK) {
    ret = rcl_get_default_topic_name_substitutions(&substitutions);
    if (ret == RCL_RET_OK) {
      ret = _rcl_remap_name(
        local_arguments, global_arguments, RCL_SERVICE_REMAP, service_name, node_name,
        node_namespace, &substitutions, allocator, output_name);
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
  const rcl_arguments_t * global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  return _rcl_remap_name(
    local_arguments, global_arguments, RCL_NODENAME_REMAP, NULL, node_name, NULL, NULL,
    allocator, output_name);
}

rcl_ret_t
rcl_remap_node_namespace(
  const rcl_arguments_t * local_arguments,
  const rcl_arguments_t * global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_namespace)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  return _rcl_remap_name(
    local_arguments, global_arguments, RCL_NAMESPACE_REMAP, NULL, node_name, NULL, NULL,
    allocator, output_namespace);
}

#ifdef __cplusplus
}
#endif
