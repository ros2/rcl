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


/// Remap a name the output name if a rule applies
rcl_ret_t
_rcl_remap_name(
  rcl_remap_t * remap_rule,
  int num_rules,
  const char * node_name,
  const char * input_name,
  rcl_allocator_t allocator,
  char ** output_name)
{
  for (int i = 0; i < num_rules; ++i) {
    // If something is null it means apply the replacement regardless
    if (NULL != remap_rule->match && NULL != input_name) {
      if (0 != strcmp(remap_rule->match, input_name)) {
        // This rule does not match
        continue;
      }
    }

    if (NULL != remap_rule->replacement) {
      size_t len = strlen(remap_rule->replacement);
      if (len > 0) {
        // plus 1 for terminating \0
        *output_name = allocator.allocate(sizeof(char *) * len + 1, allocator.state);
        if (NULL == output_name) {
          return RCL_RET_BAD_ALLOC;
        }
        strncpy(*output_name, remap_rule->replacement, len + 1);
        return RCL_RET_OK;
      }
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_remap_topic_name(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  const char * input_name,
  rcl_allocator_t allocator,
  char ** output_name)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(input_name, RCL_RET_INVALID_ARGUMENT, allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_name, RCL_RET_INVALID_ARGUMENT, allocator);
  if (NULL != local_arguments && NULL == local_arguments->impl) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  *output_name = NULL;

  // Look at local rules first
  if (NULL != local_arguments) {
    rcl_ret_t ret = _rcl_remap_name(local_arguments->impl->topic_remaps,
      local_arguments->impl->num_topic_remaps, node_name, input_name, allocator, output_name);
    if (NULL != output_name || RCL_RET_OK != ret) {
      // A remap rule matched or there was an error, either way this call is done
      return ret;
    }
  }

  if (use_global_arguments) {
    return _rcl_remap_name(__rcl_arguments.impl->topic_remaps,
      __rcl_arguments.impl->num_topic_remaps, node_name, input_name, allocator, output_name);
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_remap_service_name(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  const char * input_name,
  rcl_allocator_t allocator,
  char ** output_name)
{
  // TODO(sloretz) expand grammar to service specific remap rules
  return rcl_remap_topic_name(
    local_arguments, use_global_arguments, node_name, input_name, allocator, output_name);
}

rcl_ret_t
rcl_remap_node_name(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_name)
{
  // TODO(sloretz) remap node name
  *output_name = NULL;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_remap_node_namespace(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_namespace)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_namespace, RCL_RET_INVALID_ARGUMENT, allocator);
  if (NULL != local_arguments && NULL == local_arguments->impl) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  *output_namespace = NULL;

  // Look at local rules first if they were provided
  if (NULL != local_arguments) {
    rcl_ret_t ret = _rcl_remap_name(&(local_arguments->impl->namespace_replacement), 1, node_name,
      NULL, allocator, output_namespace);
    if (NULL != output_namespace || RCL_RET_OK != ret) {
      // A remap rule matched or there was an error, either way this call is done
      return ret;
    }
  }

  // Look at global remap rules
  if (use_global_arguments) {
    return _rcl_remap_name(&(__rcl_arguments.impl->namespace_replacement), 1, node_name, NULL,
      allocator, output_namespace);
  }
  return RCL_RET_OK;
}


#if __cplusplus
}
#endif
