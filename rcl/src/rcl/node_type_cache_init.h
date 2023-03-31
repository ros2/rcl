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

#ifndef RCL__NODE_TYPE_CACHE_INIT_H_
#define RCL__NODE_TYPE_CACHE_INIT_H_

#include "rcl/node.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

/// Initialize the node's type cache.
/**
 * This function initializes hash map of the node's type cache such that types
 * can be registered and retrieved.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node the handle to the node whose type cache should be initialized
 * \return #RCL_RET_OK if the node's type cache was successfully initialized, or
 * \return #RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 * \return #RCL_RET_NODE_INVALID if the given `node` is invalid, or
 * \return #RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t rcl_node_type_cache_init(rcl_node_t * node);

/// Finalize the node's type cache.
/**
 * This function clears the hash map of the node's type cache and deallocates
 * used memory.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node the handle to the node whose type cache should be finalized
 * \return #RCL_RET_OK if the node's type cache was successfully finalized, or
 * \return #RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 * \return #RCL_RET_NODE_INVALID if the given `node` is invalid, or
 * \return #RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t rcl_node_type_cache_fini(rcl_node_t * node);

#endif  // RCL__NODE_TYPE_CACHE_INIT_H_
