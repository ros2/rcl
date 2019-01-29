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

#ifndef RCL__REMAP_IMPL_H_
#define RCL__REMAP_IMPL_H_

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/remap.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Enum doubles as a bitmask for rule sthat apply to both topics and services.
typedef enum rcl_remap_type_t
{
  RCL_UNKNOWN_REMAP = 0,
  RCL_TOPIC_REMAP = 1u << 0,
  RCL_SERVICE_REMAP = 1u << 1,
  RCL_NODENAME_REMAP = 1u << 2,
  RCL_NAMESPACE_REMAP = 1u << 3
} rcl_remap_type_t;

typedef struct rcl_remap_impl_t
{
  /// Bitmask indicating what type of rule this is.
  rcl_remap_type_t type;
  /// A node name that this rule is limited to, or NULL if it applies to any node.
  char * node_name;
  /// Match portion of a rule, or NULL if node name or namespace replacement.
  char * match;
  /// Replacement portion of a rule.
  char * replacement;

  /// Allocator used to allocate objects in this struct
  rcl_allocator_t allocator;
} rcl_remap_impl_t;

/// Get an rcl_remap_t structure initialized with NULL.
rcl_remap_t
rcl_remap_get_zero_initialized();

/// Copy one remap structure into another.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] rule The structure to be copied.
 *  Its allocator is used to copy memory into the new structure.
 * \param[out] rule_out A zero-initialized rcl_remap_t structure to be copied into.
 * \return `RCL_RET_OK` if the structure was copied successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_remap_copy(
  const rcl_remap_t * rule,
  rcl_remap_t * rule_out);

/// Reclaim resources used in an rcl_remap_t structure.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] rule A rule to deallocate back to a zero initialized state.
 * \return `RCL_RET_OK` if the structure was free'd, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_NODE_INVALID_NAME` if the name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_WARN_UNUSED
rcl_ret_t
rcl_remap_fini(
  rcl_remap_t * rule);

#ifdef __cplusplus
}
#endif

#endif  // RCL__REMAP_IMPL_H_
