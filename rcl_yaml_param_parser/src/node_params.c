// Copyright 2018 Apex.AI, Inc.
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

#include "./impl/node_params.h"
#include "./impl/types.h"
#include "./impl/yaml_variant.h"

#include "rcutils/error_handling.h"

rcutils_ret_t node_params_init(
  rcl_node_params_t * node_params,
  const rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_params, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  node_params->parameter_names = allocator.zero_allocate(
    MAX_NUM_PARAMS_PER_NODE, sizeof(char *), allocator.state);
  if (NULL == node_params->parameter_names) {
    return RCUTILS_RET_BAD_ALLOC;
  }

  node_params->parameter_values = allocator.zero_allocate(
    MAX_NUM_PARAMS_PER_NODE, sizeof(rcl_variant_t), allocator.state);
  if (NULL == node_params->parameter_values) {
    allocator.deallocate(node_params->parameter_names, allocator.state);
    node_params->parameter_names = NULL;
    return RCUTILS_RET_BAD_ALLOC;
  }

  return RCUTILS_RET_OK;
}

void rcl_yaml_node_params_fini(
  rcl_node_params_t * node_params_st,
  const rcutils_allocator_t allocator)
{
  if (NULL == node_params_st) {
    return;
  }

  if (NULL != node_params_st->parameter_names) {
    for (size_t parameter_idx = 0U; parameter_idx < node_params_st->num_params;
      parameter_idx++)
    {
      char * param_name = node_params_st->parameter_names[parameter_idx];
      if (NULL != param_name) {
        allocator.deallocate(param_name, allocator.state);
      }
    }
    allocator.deallocate(node_params_st->parameter_names, allocator.state);
    node_params_st->parameter_names = NULL;
  }

  if (NULL != node_params_st->parameter_values) {
    for (size_t parameter_idx = 0U; parameter_idx < node_params_st->num_params;
      parameter_idx++)
    {
      rcl_yaml_variant_fini(&(node_params_st->parameter_values[parameter_idx]), allocator);
    }

    allocator.deallocate(node_params_st->parameter_values, allocator.state);
    node_params_st->parameter_values = NULL;
  }
}
