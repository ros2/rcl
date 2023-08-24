// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#include "rcl/node_type_cache.h"
#include "rcl/type_description_conversions.h"

#include "rcl/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/types/hash_map.h"

#include "./context_impl.h"
#include "./node_impl.h"

typedef struct rcl_type_info_with_registration_count_t
{
  /// Counter to keep track of registrations
  size_t num_registrations;

  /// The actual type info.
  rcl_type_info_t type_info;
} rcl_type_info_with_registration_count_t;

static size_t get_type_hash_hashmap_key(const void * key)
{
  // Reinterpret-cast the first sizeof(size_t) bytes of the hash value
  const rosidl_type_hash_t * type_hash = key;
  return *(size_t *)type_hash->value;
}

static int cmp_type_hash(const void * val1, const void * val2)
{
  return memcmp(val1, val2, sizeof(rosidl_type_hash_t));
}

rcl_ret_t rcl_node_type_cache_init(rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node->impl, RCL_RET_NODE_INVALID);
  if (NULL != node->impl->registered_types_by_type_hash.impl) {
    // already initialized
    return RCL_RET_OK;
  }

  rcutils_ret_t ret = rcutils_hash_map_init(
    &node->impl->registered_types_by_type_hash, 2, sizeof(rosidl_type_hash_t),
    sizeof(rcl_type_info_with_registration_count_t),
    get_type_hash_hashmap_key, cmp_type_hash,
    &node->context->impl->allocator);

  if (RCUTILS_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to initialize type cache hash map");
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

rcl_ret_t rcl_node_type_cache_fini(rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node->impl, RCL_RET_NODE_INVALID);

  // Clean up any remaining types.
  rosidl_type_hash_t key;
  rcl_type_info_with_registration_count_t type_info_with_registrations;
  rcutils_ret_t hash_map_ret = rcutils_hash_map_get_next_key_and_data(
    &node->impl->registered_types_by_type_hash, NULL, &key,
    &type_info_with_registrations);

  if (RCUTILS_RET_NOT_INITIALIZED == hash_map_ret) {
    return RCL_RET_NOT_INIT;
  }

  while (RCUTILS_RET_OK == hash_map_ret) {
    hash_map_ret = rcutils_hash_map_unset(
      &node->impl->registered_types_by_type_hash, &key);
    if (hash_map_ret != RCUTILS_RET_OK) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to clear out type informations [%s] during shutdown; memory "
        "will be leaked.",
        rcutils_get_error_string().str);
      break;
    }

    type_description_interfaces__msg__TypeDescription__destroy(
      type_info_with_registrations.type_info.type_description);
    type_description_interfaces__msg__TypeSource__Sequence__destroy(
      type_info_with_registrations.type_info.type_sources);

    hash_map_ret = rcutils_hash_map_get_next_key_and_data(
      &node->impl->registered_types_by_type_hash, NULL, &key,
      &type_info_with_registrations);
  }

  rcutils_ret_t rcutils_ret =
    rcutils_hash_map_fini(&node->impl->registered_types_by_type_hash);

  return RCUTILS_RET_OK == rcutils_ret ? RCL_RET_OK : RCL_RET_ERROR;
}

rcl_ret_t rcl_node_type_cache_get_type_info(
  const rcl_node_t * node,
  const rosidl_type_hash_t * type_hash,
  rcl_type_info_t * type_info)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node->impl, RCL_RET_NODE_INVALID);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_hash, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_info, RCL_RET_INVALID_ARGUMENT);

  rcl_type_info_with_registration_count_t type_info_with_registrations;

  rcutils_ret_t ret =
    rcutils_hash_map_get(
    &node->impl->registered_types_by_type_hash,
    type_hash, &type_info_with_registrations);
  if (RCUTILS_RET_OK == ret) {
    *type_info = type_info_with_registrations.type_info;
    return RCL_RET_OK;
  } else if (RCUTILS_RET_NOT_INITIALIZED == ret) {
    return RCL_RET_NOT_INIT;
  }

  return RCL_RET_ERROR;
}

rcl_ret_t rcl_node_type_cache_register_type(
  const rcl_node_t * node, const rosidl_type_hash_t * type_hash,
  const rosidl_runtime_c__type_description__TypeDescription * type_description,
  const rosidl_runtime_c__type_description__TypeSource__Sequence * type_description_sources)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node->impl, RCL_RET_NODE_INVALID);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_hash, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_description, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_description_sources, RCL_RET_INVALID_ARGUMENT);

  rcl_type_info_with_registration_count_t type_info_with_registrations;

  const rcutils_ret_t rcutils_ret = rcutils_hash_map_get(
    &node->impl->registered_types_by_type_hash,
    type_hash, &type_info_with_registrations);

  if (RCUTILS_RET_OK == rcutils_ret) {
    // If the type already exists, we only have to increment the registration
    // count.
    type_info_with_registrations.num_registrations++;
  } else if (RCUTILS_RET_NOT_FOUND == rcutils_ret) {
    // First registration of this type
    type_info_with_registrations.num_registrations = 1;

    // Convert type description struct to type description message struct.
    type_info_with_registrations.type_info.type_description =
      rcl_convert_type_description_runtime_to_msg(type_description);
    if (type_info_with_registrations.type_info.type_description == NULL) {
      // rcl_convert_type_description_runtime_to_msg already does rcutils_set_error
      return RCL_RET_ERROR;
    }

    // Convert type sources struct to type sources message struct.
    type_info_with_registrations.type_info.type_sources =
      rcl_convert_type_source_sequence_runtime_to_msg(type_description_sources);
    if (type_info_with_registrations.type_info.type_sources == NULL) {
      // rcl_convert_type_source_sequence_runtime_to_msg already does rcutils_set_error
      type_description_interfaces__msg__TypeDescription__destroy(
        type_info_with_registrations.type_info.type_description);
      return RCL_RET_ERROR;
    }
  } else {
    return RCL_RET_ERROR;
  }

  // Update the hash map entry.
  if (RCUTILS_RET_OK !=
    rcutils_hash_map_set(
      &node->impl->registered_types_by_type_hash,
      type_hash, &type_info_with_registrations))
  {
    RCL_SET_ERROR_MSG("Failed to update type info");
    type_description_interfaces__msg__TypeDescription__destroy(
      type_info_with_registrations.type_info.type_description);
    type_description_interfaces__msg__TypeSource__Sequence__destroy(
      type_info_with_registrations.type_info.type_sources);
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

rcl_ret_t rcl_node_type_cache_unregister_type(
  const rcl_node_t * node,
  const rosidl_type_hash_t * type_hash)
{
  rcl_type_info_with_registration_count_t type_info;

  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node->impl, RCL_RET_NODE_INVALID);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_hash, RCL_RET_INVALID_ARGUMENT);

  if (RCUTILS_RET_OK !=
    rcutils_hash_map_get(
      &node->impl->registered_types_by_type_hash,
      type_hash, &type_info))
  {
    RCL_SET_ERROR_MSG("Failed to unregister type, hash not present in map.");
    return RCL_RET_ERROR;
  }

  if (--type_info.num_registrations > 0) {
    if (RCUTILS_RET_OK !=
      rcutils_hash_map_set(
        &node->impl->registered_types_by_type_hash,
        type_hash, &type_info))
    {
      RCL_SET_ERROR_MSG("Failed to update type info");
      return RCL_RET_ERROR;
    }
  } else {
    if (RCUTILS_RET_OK !=
      rcutils_hash_map_unset(
        &node->impl->registered_types_by_type_hash,
        type_hash))
    {
      RCL_SET_ERROR_MSG("Failed to unregister type info");
      return RCL_RET_ERROR;
    }

    type_description_interfaces__msg__TypeDescription__destroy(
      type_info.type_info.type_description);
    type_description_interfaces__msg__TypeSource__Sequence__destroy(
      type_info.type_info.type_sources);
  }

  return RCL_RET_OK;
}
