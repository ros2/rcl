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

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

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
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_guard_condition_t
rcl_get_zero_initialized_guard_condition(void);

/// Initialize a rcl guard_condition.
/* After calling this function on a rcl_guard_condition_t, it can be passed to
 * rcl_wait() and then concurrently it can be triggered to wake-up rcl_wait().
 *
 * Expected usage:
 *
 *    #include <rcl/rcl.h>
 *
 *    // ... error handling
 *    rcl_guard_condition_t guard_condition = rcl_get_zero_initialized_guard_condition();
 *    ret = rcl_guard_condition_init(
 *      &guard_condition, rcl_guard_condition_get_default_options());
 *    // ... error handling, and on shutdown do deinitialization:
 *    ret = rcl_guard_condition_fini(&guard_condition);
 *    // ... error handling for rcl_guard_condition_fini()
 *
 * This function does allocate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \param[inout] guard_condition preallocated guard_condition structure
 * \param[in] options the guard_condition's options
 * \return RCL_RET_OK if guard_condition was initialized successfully, or
 *         RCL_RET_ALREADY_INIT if the guard condition is already initialized, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_BAD_ALLOC if allocating memory failed, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_guard_condition_init(
  rcl_guard_condition_t * guard_condition,
  const rcl_guard_condition_options_t options);

/// Same as rcl_guard_condition_init(), but reusing an existing rmw handle.
/* In addition to the documentation for rcl_guard_condition_init(), the
 * rmw_guard_condition parameter must not be null and must point to a valid
 * rmw guard condition.
 *
 * Also the life time of the rcl guard condition is tied to the life time of
 * the rmw guard condition.
 * So if the rmw guard condition is destroyed before the rcl guard condition,
 * the rcl guard condition becomes invalid.
 *
 * Similarly if the resulting rcl guard condition is fini'ed before the rmw
 * guard condition, then the rmw guard condition is no longer valid.
 *
 * \param[inout] guard_condition preallocated guard_condition structure
 * \param[in] rmw_guard_condition existing rmw guard condition to reuse
 * \param[in] options the guard_condition's options
 * \return RCL_RET_OK if guard_condition was initialized successfully, or
 *         RCL_RET_ALREADY_INIT if the guard condition is already initialized, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_BAD_ALLOC if allocating memory failed, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_guard_condition_init_from_rmw(
  rcl_guard_condition_t * guard_condition,
  const rmw_guard_condition_t * rmw_guard_condition,
  const rcl_guard_condition_options_t options);

/// Finalize a rcl_guard_condition_t.
/* After calling, calls to rcl_trigger_guard_condition() will fail when using
 * this guard condition.
 *
 * This function does free heap memory and can allocate memory on errors.
 * This function is not thread-safe with rcl_trigger_guard_condition().
 * This function is lock-free.
 *
 * \param[inout] guard_condition handle to the guard_condition to be finalized
 * \return RCL_RET_OK if guard_condition was finalized successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_guard_condition_fini(rcl_guard_condition_t * guard_condition);

/// Return the default options in a rcl_guard_condition_options_t struct.
/* This function does not allocate heap memory.
 * This function is thread-safe.
 * This function is lock-free.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_guard_condition_options_t
rcl_guard_condition_get_default_options(void);

/// Trigger a rcl guard condition.
/* This function can fail, and return RCL_RET_INVALID_ARGUMENT, if the:
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
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_trigger_guard_condition(rcl_guard_condition_t * guard_condition);

/// Return the rmw guard condition handle.
/* The handle returned is a pointer to the internally held rmw handle.
 * This function can fail, and therefore return NULL, if the:
 *   - guard_condition is NULL
 *   - guard_condition is invalid (never called init, called fini, or invalid node)
 *
 * The returned handle is made invalid if the guard condition is finalized or
 * if rcl_shutdown() is called.
 * The returned handle is not guaranteed to be valid for the life time of the
 * guard condition as it may be finalized and recreated itself.
 * Therefore it is recommended to get the handle from the guard condition using
 * this function each time it is needed and avoid use of the handle
 * concurrently with functions that might change it.
 *
 * \param[in] guard_condition pointer to the rcl guard_condition
 * \return rmw guard_condition handle if successful, otherwise NULL
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rmw_guard_condition_t *
rcl_guard_condition_get_rmw_handle(const rcl_guard_condition_t * guard_condition);

#if __cplusplus
}
#endif

#endif  // RCL__GUARD_CONDITION_H_
