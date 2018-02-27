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

#include "rcl/types.h"

#if __cplusplus
extern "C"
{
#endif

typedef struct rcl_remap_t
{
  /// \brief a node name that this rule is limited to, or NULL if it applies to any node
  char * node_name;
  /// \brief match portion of a rule
  char * match;
  /// \brief replacement portion of a rule
  char * replacement;
} rcl_remap_t;


/// \brief Get an rcl_remap_t structure initialized with NULL
rcl_remap_t
rcl_remap_get_zero_initialized();

/// \brief Deallocate memory used in an rcl_remap_t structure
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] 
 * \param[in] 
 * \param[in] 
 * \param[in] allocator a valid allocator to use
 * \return `RCL_RET_OK` if the structure was free'd
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_NODE_INVALID_NAME` if the name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
rcl_ret_t
rcl_remap_fini(
  rcl_remap_t * rule,
  rcl_allocator_t allocator);



#if __cplusplus
}
#endif

#endif  // RCL__REMAP_IMPL_H_
