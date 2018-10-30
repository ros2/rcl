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

#ifndef RCL_ACTION__GOAL_HANDLE_H_
#define RCL_ACTION__GOAL_HANDLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/goal_state_machine.h"
#include "rcl_action/types.h"
#include "rcl_action/visibility_control.h"

// Forward declare
typedef struct rcl_action_server_t rcl_action_server_t;

/// Internal rcl action goal implementation struct.
struct rcl_action_goal_handle_impl_t;

/// Goal handle for an action.
typedef struct rcl_action_goal_handle_t
{
  struct rcl_action_goal_handle_impl_t * impl;
} rcl_action_goal_handle_t;

/// Return a rcl_action_goal_handle_t struct with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_goal_handle_t before passing to
 * rcl_action_goal_handle_init().
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_action_goal_handle_t
rcl_action_get_zero_initialized_goal_handle(void);

/// Initialize a rcl_action_goal_handle_t.
/**
 * After calling this function on a rcl_action_goal_handle_t, it can be used to update the
 * goals state with rcl_action_update_goal_state().
 * It can also be used to query the state of the goal with
 * rcl_action_goal_handle_get_message() and rcl_action_goal_handle_is_active().
 * Goal information can be accessed with rcl_action_goal_handle_get_message() and
 * rcl_action_goal_handle_get_info().
 *
 * The given rcl_action_server_t must be valid and the resulting rcl_action_goal_handle_t is
 * only valid as long as the given rcl_action_server_t remains valid.
 *
 * Expected usage:
 *
 * ```c
 * #include <rcl/rcl.h>
 * #include <rcl_action/rcl_action.h>
 * #include <rosidl_generator_c/action_type_support_struct.h>
 * #include <example_interfaces/action/fibonacci.h>
 *
 * // ... initialize node
 * const rosidl_action_type_support_t * ts =
 *   ROSIDL_GET_ACTION_TYPE_SUPPORT(example_interfaces, Fibonacci);
 * rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();
 * rcl_action_server_options_t action_server_ops = rcl_action_server_get_default_options();
 * ret = rcl_action_server_init(&action_server, &node, ts, "fibonacci", &action_server_ops);
 * // ... error handling
 * rcl_action_goal_handle_t goal_handle = rcl_action_get_zero_initialized_goal_handle();
 * ret = rcl_action_goal_handle_init(&goal_handle, &action_server);
 * // ... error handling, and on shutdown do finalization:
 * ret = rcl_action_goal_handle_fini(&goal_handle);
 * // ... error handling for rcl_goal_handle_fini()
 * ret = rcl_action_server_fini(&action_server, &node);
 * // ... error handling for rcl_action_server_fini()
 * // ... finalize and error handling for node
 * ```
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[out] goal_handle preallocated, zero-initialized, goal handle structure
 *   to be initialized
 * \param[in] action_server valid rcl action server
 * \param[in] type_support type support object for the action's type
 * \return `RCL_RET_OK` if goal_handle was initialized successfully, or
 * \return `RCL_RET_ACTION_GOAL_HANDLE_INVALID` if the goal handle is invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_init(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_action_server_t * action_server);

/// Finalize a rcl_action_goal_handle_t.
/**
 * After calling, rcl_action_goal_handle_t will no longer be valid and
 * rcl_action_server_t will no longer track the goal associated with the goal handle.
 *
 * After calling, calls to rcl_action_publish_feedback(), rcl_action_publish_status(),
 * rcl_action_update_goal_state(), rcl_action_goal_handle_get_status(),
 * rcl_action_goal_handle_is_active(), rcl_action_goal_handle_get_message(), and
 * rcl_action_goal_handle_get_info() will fail when using this goal handle.
 *
 * However, the given action server is still valid.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] goal_handle struct to be deinitialized
 * \param[in] action_server used to create the goal handle
 * \return `RCL_RET_OK` if the goal handle was deinitialized successfully, or
 * \return `RCL_RET_ACTION_GOAL_HANDLE_INVALID` if the goal handle is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_fini(rcl_action_goal_handle_t * goal_handle);

/// Update a goal state with a rcl_action_goal_handle_t and an event.
/**
 * This is a non-blocking call.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] goal_handle struct containing goal state to transition
 * \param[in] goal_event the event used to transition the goal state
 * \return `RCL_RET_OK` if the goal state was updated successfully, or
 * \return `RCL_RET_ACTION_GOAL_EVENT_INVALID` if the goal event is invalid, or
 * \return `RCL_RET_ACTION_GOAL_HANDLE_INVALID` if the goal handle is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_update_goal_state(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_action_goal_event_t goal_event);

/// Get the ID of a goal using a rcl_action_goal_handle_t.
/**
 * This is a non-blocking call.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] goal_handle struct containing the goal and meta
 * \param[out] goal_info a preallocated struct where the goal info is copied
 * \return `RCL_RET_OK` if the goal ID was accessed successfully, or
 * \return `RCL_RET_ACTION_GOAL_HANDLE_INVALID` if the goal handle is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_get_info(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_action_goal_info_t * goal_info);

/// Get the status of a goal.
/**
 * This is a non-blocking call.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] goal_handle struct containing the goal and metadata
 * \param[out] status a preallocated struct where the goal status is copied
 * \return `RCL_RET_OK` if the goal ID was accessed successfully, or
 * \return `RCL_RET_ACTION_GOAL_HANDLE_INVALID` if the goal handle is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_get_status(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_action_goal_state_t * status);

/// Check if a goal is active using a rcl_action_goal_handle_t.
/**
 * This is a non-blocking call.
 *
 * The allocator needs to either be a valid allocator or `NULL`, in which case
 * the default allocator will be used.
 * The allocator is used when allocation is needed for an error message.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] goal_handle struct containing the goal and metadata
 * \param[in] error_msg_allocator a valid allocator or `NULL`
 * \return `true` if a goal is in one of the following states: ACCEPTED, EXECUTING, or CANCELING, or
 * \return `false` otherwise, also
 * \return `false` if the goal handle pointer is invalid or the allocator is invalid
*/
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
bool
rcl_action_goal_handle_is_active(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_allocator_t * error_msg_allocator);

/// Check if a rcl_action_goal_handle_t is valid.
/**
 * This is a non-blocking call.
 *
 * The allocator needs to either be a valid allocator or `NULL`, in which case
 * the default allocator will be used.
 * The allocator is used when allocation is needed for an error message.
 *
 * A goal handle is invalid if:
 *   - the implementation is `NULL` (rcl_action_goal_handle_init() not called or failed)
 *   - rcl_shutdown() has been called since the goal handle has been initialized
 *   - the goal handle has been finalized with rcl_action_goal_handle_fini()
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] goal_handle struct to evaluate as valid or not
 * \param[in] error_msg_allocator a valid allocator or `NULL`
 * \return `true` if the goal handle is valid, `false` otherwise, also
 * \return `false` if the allocator is invalid
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
bool
rcl_action_goal_handle_is_valid(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_allocator_t * error_msg_allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__GOAL_HANDLE_H_
