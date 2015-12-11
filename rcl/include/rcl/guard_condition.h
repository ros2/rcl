// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__GUARD_CONDITION_H_
#define RCL__GUARD_CONDITION_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/node.h"

/// Internal rcl guard condition implementation struct.
struct rcl_guard_condition_impl_t;

/// Handle for a rcl guard condition.
typedef struct rcl_guard_condition_t
{
  struct rcl_guard_condition_impl_t * impl;
} rcl_guard_condition_t;

/// Options available for a rcl guard condition.
typedef struct rcl_guard_condition_options_t
{
  /// Custom allocator for the guard condition, used for internal allocations.
  rcl_allocator_t allocator;
} rcl_guard_condition_options_t;

/// Return a rcl_guard_condition_t struct with members set to NULL.
rcl_guard_condition_t
rcl_get_zero_initialized_guard_condition();

/// Initialize a rcl guard_condition.
/* After calling this function on a rcl_guard_condition_t, it can be passed to
 * rcl_wait() and then concurrently it can be triggered to wake-up rcl_wait().
 *
 * The given rcl_node_t must be valid and the resulting rcl_guard_condition_t
 * is only valid as long as the given rcl_node_t remains valid.
 *
 * Expected usage:
 *
 *    #include <rcl/rcl.h>
 *
 *    rcl_node_t node = rcl_get_zero_initialized_node();
 *    rcl_node_options_t node_ops = rcl_node_get_default_options();
 *    rcl_ret_t ret = rcl_node_init(&node, "node_name", &node_ops);
 *    // ... error handling
 *    rcl_guard_condition_t guard_condition = rcl_get_zero_initialized_guard_condition();
 *    ret = rcl_guard_condition_init(
 *      &guard_condition, &node, rcl_guard_condition_get_default_options());
 *    // ... error handling, and on shutdown do deinitialization:
 *    ret = rcl_guard_condition_fini(&guard_condition, &node);
 *    // ... error handling for rcl_guard_condition_fini()
 *    ret = rcl_node_fini(&node);
 *    // ... error handling for rcl_deinitialize_node()
 *
 * This function does allocate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \TODO(wjwwood): does this function need a node to be passed to it? (same for fini)
 *
 * \param[inout] guard_condition preallocated guard_condition structure
 * \param[in] node valid rcl node handle
 * \param[in] options the guard_condition's options
 * \return RCL_RET_OK if guard_condition was initialized successfully, or
 *         RCL_RET_ALREADY_INIT if the guard condition is already initialized, or
 *         RCL_RET_INVALID_ARGUMENT if any arugments are invalid, or
 *         RCL_RET_BAD_ALLOC if allocating memory failed, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_guard_condition_init(
  rcl_guard_condition_t * guard_condition,
  const rcl_node_t * node,
  const rcl_guard_condition_options_t options);

/// Finalize a rcl_guard_condition_t.
/* After calling, calls to rcl_guard_condition_trigger() will fail when using
 * this guard condition.
 * However, the given node handle is still valid.
 *
 * This function does free heap memory and can allocate memory on errors.
 * This function is not thread-safe with rcl_guard_condition_trigger().
 * This function is lock-free.
 *
 * \param[inout] guard_condition handle to the guard_condition to be finalized
 * \param[in] node handle to the node used to create the guard_condition
 * \return RCL_RET_OK if guard_condition was finalized successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arugments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_guard_condition_fini(rcl_guard_condition_t * guard_condition, rcl_node_t * node);

/// Return the default options in a rcl_guard_condition_options_t struct.
/* This function does not allocate heap memory.
 * This function is thread-safe.
 * This function is lock-free.
 */
rcl_guard_condition_options_t
rcl_guard_condition_get_default_options();

/// Trigger a rcl guard condition.
/* This function can fail, and therefore return NULL, if the:
 *   - guard condition is NULL
 *   - guard condition is invalid (never called init or called fini)
 *
 * A guard condition can be triggered from any thread.
 *
 * This function does not allocate heap memory, but can on errors.
 * This function is thread-safe with itself, but cannot be called concurrently
 * with rcl_guard_condition_fini() on the same guard condition.
 * This function is lock-free, but the underlying system calls may not be.
 *
 * \param[in] guard_condition handle to the guard_condition to be triggered
 * \return RCL_RET_OK if the guard condition was triggered, or
 *         RCL_RET_INVALID_ARGUMENT if any arugments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_guard_condition_trigger(const rcl_guard_condition_t * guard_condition);

/// Return the rmw guard condition handle.
/* The handle returned is a pointer to the internally held rmw handle.
 * This function can fail, and therefore return NULL, if the:
 *   - guard_condition is NULL
 *   - guard_condition is invalid (never called init, called fini, or invalid node)
 *
 * The returned handle is only valid as long as the given guard_condition is valid.
 *
 * \param[in] guard_condition pointer to the rcl guard_condition
 * \return rmw guard_condition handle if successful, otherwise NULL
 */
rmw_guard_condition_t *
rcl_guard_condition_get_rmw_guard_condition_handle(const rcl_guard_condition_t * guard_condition);

#if __cplusplus
}
#endif

#endif  // RCL__GUARD_CONDITION_H_
