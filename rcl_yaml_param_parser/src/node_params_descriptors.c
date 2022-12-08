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

#include "./impl/node_params_descriptors.h"
#include "./impl/types.h"
#include "./impl/yaml_descriptor.h"

#define INIT_NUM_PARAMS_DESCRIPTORS_PER_NODE 128U

rcutils_ret_t node_params_descriptors_init(
  rcl_node_params_descriptors_t * node_descriptors,
  const rcutils_allocator_t allocator)
{
  return node_params_descriptors_init_with_capacity(
    node_descriptors, INIT_NUM_PARAMS_DESCRIPTORS_PER_NODE, allocator);
}

rcutils_ret_t node_params_descriptors_init_with_capacity(
  rcl_node_params_descriptors_t * node_descriptors,
  size_t capacity,
  const rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_descriptors, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);
  if (capacity == 0) {
    RCUTILS_SET_ERROR_MSG("capacity can't be zero");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  node_descriptors->parameter_names = allocator.zero_allocate(
    capacity, sizeof(char *), allocator.state);
  if (NULL == node_descriptors->parameter_names) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for node parameter names");
    return RCUTILS_RET_BAD_ALLOC;
  }

  node_descriptors->parameter_descriptors = allocator.zero_allocate(
    capacity, sizeof(rcl_param_descriptor_t), allocator.state);
  if (NULL == node_descriptors->parameter_descriptors) {
    allocator.deallocate(node_descriptors->parameter_names, allocator.state);
    node_descriptors->parameter_names = NULL;
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for node parameter descriptors");
    return RCUTILS_RET_BAD_ALLOC;
  }

  node_descriptors->num_descriptors = 0U;
  node_descriptors->capacity_descriptors = capacity;
  return RCUTILS_RET_OK;
}

rcutils_ret_t node_params_descriptors_reallocate(
  rcl_node_params_descriptors_t * node_descriptors,
  size_t new_capacity,
  const rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_descriptors, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);
  // invalid if new_capacity is less than num_descriptors
  if (new_capacity < node_descriptors->num_descriptors) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "new capacity '%zu' must be greater than or equal to '%zu'",
      new_capacity,
      node_descriptors->num_descriptors);
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  void * parameter_names = allocator.reallocate(
    node_descriptors->parameter_names, new_capacity * sizeof(char *), allocator.state);
  if (NULL == parameter_names) {
    RCUTILS_SET_ERROR_MSG("Failed to reallocate node parameter names");
    return RCUTILS_RET_BAD_ALLOC;
  }
  node_descriptors->parameter_names = parameter_names;
  // zero initialization for the added memory
  if (new_capacity > node_descriptors->capacity_descriptors) {
    memset(
      node_descriptors->parameter_names + node_descriptors->capacity_descriptors, 0,
      (new_capacity - node_descriptors->capacity_descriptors) * sizeof(char *));
  }

  void * parameter_descriptors = allocator.reallocate(
    node_descriptors->parameter_descriptors,
    new_capacity * sizeof(rcl_param_descriptor_t), allocator.state);
  if (NULL == parameter_descriptors) {
    RCUTILS_SET_ERROR_MSG("Failed to reallocate node parameter values");
    return RCUTILS_RET_BAD_ALLOC;
  }
  node_descriptors->parameter_descriptors = parameter_descriptors;
  // zero initialization for the added memory
  if (new_capacity > node_descriptors->capacity_descriptors) {
    memset(
      &node_descriptors->parameter_descriptors[node_descriptors->capacity_descriptors], 0,
      (new_capacity - node_descriptors->capacity_descriptors) * sizeof(rcl_param_descriptor_t));
  }

  node_descriptors->capacity_descriptors = new_capacity;
  return RCUTILS_RET_OK;
}

void rcl_yaml_node_params_descriptors_fini(
  rcl_node_params_descriptors_t * node_descriptors,
  const rcutils_allocator_t allocator)
{
  if (NULL == node_descriptors) {
    return;
  }

  if (NULL != node_descriptors->parameter_names) {
    for (size_t parameter_idx = 0U; parameter_idx < node_descriptors->num_descriptors;
      parameter_idx++)
    {
      char * param_name = node_descriptors->parameter_names[parameter_idx];
      if (NULL != param_name) {
        allocator.deallocate(param_name, allocator.state);
      }
    }
    allocator.deallocate(node_descriptors->parameter_names, allocator.state);
    node_descriptors->parameter_names = NULL;
  }

  if (NULL != node_descriptors->parameter_descriptors) {
    for (size_t parameter_idx = 0U; parameter_idx < node_descriptors->num_descriptors;
      parameter_idx++)
    {
      rcl_yaml_descriptor_fini(
        &(node_descriptors->parameter_descriptors[parameter_idx]), allocator);
    }

    allocator.deallocate(node_descriptors->parameter_descriptors, allocator.state);
    node_descriptors->parameter_descriptors = NULL;
  }

  node_descriptors->num_descriptors = 0;
  node_descriptors->capacity_descriptors = 0;
}
