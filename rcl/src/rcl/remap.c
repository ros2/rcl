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


rcl_ret_t
rcl_remap_name(
  const char * input_name,
  rcl_node_t * node,
  rcl_allocator_t error_allocator,
  char ** output_name)
{
  // TODO(sloretz) remap topic name
  return RCL_RET_ERROR;
}


rcl_ret_t
rcl_remap_node_name(
  rcl_node_t * node,
  rcl_allocator_t error_allocator,
  char ** output_name)
{
  // TODO(sloretz) remap node name
  *output_name = NULL;
  return RCL_RET_OK;
}


/// Set the output namespace if the replacement rule applies
/// \return True if the rule set the output namespace
rcl_ret_t
_rcl_remap_namespace(
  rcl_remap_t * remap_rule,
  rcl_allocator_t allocator,
  char ** output_namespace)
{
  if (remap_rule->replacement != NULL) {
    size_t len = strlen(remap_rule->replacement);
    if (len > 0) {
      // plus 1 for terminating \0
      *output_namespace = allocator.allocate(sizeof(char *) * len + 1, allocator.state);
      if (NULL == output_namespace) {
        return RCL_RET_BAD_ALLOC;
      }
      strncpy(*output_namespace, remap_rule->replacement, len + 1);
      return RCL_RET_OK;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_remap_namespace(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_namespace)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return RCL_RET_INVALID_ARGUMENT);
  // TODO(sloretz) Use the node name to filter namespace replacement rules
  RCL_CHECK_ARGUMENT_FOR_NULL(node_name, RCL_RET_INVALID_ARGUMENT, allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_namespace, RCL_RET_INVALID_ARGUMENT, allocator);
  if (NULL != local_arguments && NULL == local_arguments->impl) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  *output_namespace = NULL;

  // Look at local rules first
  if (NULL != local_arguments) {
    rcl_ret_t ret = _rcl_remap_namespace(
      &(local_arguments->impl->namespace_replacement), allocator, output_namespace);
    if (NULL != output_namespace || RCL_RET_OK != ret) {
      return ret;
    }
  }

  // Look at global remap rules
  if (use_global_arguments) {
    return _rcl_remap_namespace(
      &(__rcl_arguments.impl->namespace_replacement), allocator, output_namespace);
  }
  return RCL_RET_OK;
}


#if __cplusplus
}
#endif
