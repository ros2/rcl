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

#include "rcutils/strdup.h"

#include "./impl/node_params.h"
#include "./impl/types.h"
#include "./impl/yaml_variant.h"

#define INIT_NUM_PARAMS_PER_NODE 128U

rcutils_ret_t node_params_init(
  rcl_node_params_t * node_params,
  const rcutils_allocator_t allocator)
{
  return node_params_init_with_capacity(node_params, INIT_NUM_PARAMS_PER_NODE, allocator);
}

rcutils_ret_t node_params_init_with_capacity(
  rcl_node_params_t * node_params,
  size_t capacity,
  const rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_params, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);
  if (capacity == 0) {
    RCUTILS_SET_ERROR_MSG("capacity can't be zero");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  rcutils_hash_map_t map = rcutils_get_zero_initialized_hash_map();
  rcutils_ret_t ret = rcutils_hash_map_init(
    &map, capacity, sizeof(char *), sizeof(rcl_variant_t *),
    rcutils_hash_map_string_hash_func, rcutils_hash_map_string_cmp_func, &allocator);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SET_ERROR_MSG("Failed to initialize a hash map for node parameters");
    return ret;
  }

  node_params->node_params_map = map;
  return RCUTILS_RET_OK;
}

rcutils_ret_t node_params_copy(
  const rcl_node_params_t * src,
  rcl_node_params_t * dest,
  const rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(src, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dest, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  char * parameter_name = NULL;
  rcl_variant_t * parameter_value = NULL;
  char * param_name = NULL;
  rcl_variant_t * param_value = NULL;
  rcutils_ret_t ret = rcutils_hash_map_get_next_key_and_data(
    &src->node_params_map, NULL, &parameter_name, &parameter_value);
  while (RCUTILS_RET_OK == ret) {
    if (parameter_name == NULL || parameter_value == NULL) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error getting a key and data from a hash map\n");
      ret = RCUTILS_RET_ERROR;
      goto fail;
    }
    param_name = rcutils_strdup(parameter_name, allocator);
    if (param_name == NULL) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating parameter name\n");
      ret = RCUTILS_RET_BAD_ALLOC;
      goto fail;
    }

    param_value = allocator.allocate(
      sizeof(rcl_variant_t), allocator.state);
    if (param_value == NULL) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating parameter value\n");
      ret = RCUTILS_RET_BAD_ALLOC;
      goto fail;
    }
    memset(param_value, 0, sizeof(rcl_variant_t));

    if (!rcl_yaml_variant_copy(param_value, parameter_value, allocator)) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating parameter value\n");
      ret = RCUTILS_RET_ERROR;
      goto fail;
    }

    ret = rcutils_hash_map_set(&dest->node_params_map, &param_name, &param_value);
    if (ret != RCUTILS_RET_OK) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error setting a hash map\n");
      ret = RCUTILS_RET_ERROR;
      goto after_copy;
    }

    param_name = NULL;
    param_value = NULL;
    ret = rcutils_hash_map_get_next_key_and_data(
      &src->node_params_map, &parameter_name, &parameter_name, &parameter_value);
  }

  return RCUTILS_RET_OK;

after_copy:
  rcl_yaml_variant_fini(param_value, allocator);
fail:
  if (param_value) {
    allocator.deallocate(param_value, allocator.state);
  }
  if (param_name) {
    allocator.deallocate(param_name, allocator.state);
  }

  return ret;
}

void rcl_yaml_node_params_fini(
  rcl_node_params_t * node_params_st,
  const rcutils_allocator_t allocator)
{
  if (NULL == node_params_st) {
    return;
  }

  char * param_name = NULL;
  rcl_variant_t * param_value = NULL;
  rcutils_ret_t ret = rcutils_hash_map_get_next_key_and_data(
    &node_params_st->node_params_map, NULL, &param_name, &param_value);
  while (RCUTILS_RET_OK == ret) {
    rcl_yaml_variant_fini(param_value, allocator);
    ret = rcutils_hash_map_unset(&node_params_st->node_params_map, &param_name);
    if (RCUTILS_RET_OK != ret) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to unset key '%s' for hash map of a node parameter", param_name);
      return;
    }

    allocator.deallocate(param_name, allocator.state);
    param_name = NULL;
    allocator.deallocate(param_value, allocator.state);
    param_value = NULL;
    ret = rcutils_hash_map_get_next_key_and_data(
      &node_params_st->node_params_map, NULL, &param_name, &param_value);
  }
  if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES != ret) {
    RCUTILS_SET_ERROR_MSG("Failed to get next item for node parameters");
    return;
  }

  ret = rcutils_hash_map_fini(&node_params_st->node_params_map);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SET_ERROR_MSG("Failed to deallocate hash map of a node parameter");
  }
}
