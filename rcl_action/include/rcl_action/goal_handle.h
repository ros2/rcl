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
#include "rcl/visibility_control.h"

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
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_goal_handle_t
rcl_action_get_zero_initialized_goal_handle(void);

/// Initialize a rcl action goal handle.
/**
 * After calling this function on a rcl_action_goal_handle_t, it can be used to update the
 * goals state with rcl_action_update_goal_state().
 * It can also be used to query the state of the goal with rcl_action_goal_handle_get_status(),
 * rcl_action_goal_handle_is_active(), and TODO
 * Goal information can be accessed with rcl_action_goal_handle_get_message() and
 * rcl_action_goal_handle_get_id().
 *
 *
 * The given rcl_action_server_t must be valid and the resulting rcl_action_goal_handle_t is
 * only valid as long as the given rcl_action_server_t remains valid.
 *
 * The rosidl_action_type_support_t is obtained on a per .action type basis.
 * When the user defines a ROS action, code is generated which provides the
 * required rosidl_action_type_support_t object.
 * This object can be obtained using a language appropriate mechanism.
 * \todo TODO(jacobperron) write these instructions once and link to it instead
 *
 * For C, a macro can be used (for example `example_interfaces/Fibonacci`):
 *
 * ```c
 * #include <rosidl_generator_c/action_type_support_struct.h>
 * #include <example_interfaces/action/fibonacci.h>
 * const rosidl_action_type_support_t * ts =
 *   ROSIDL_GET_ACTION_TYPE_SUPPORT(example_interfaces, Fibonacci);
 * ```
 *
 * For C++, a template function is used:
 *
 * ```cpp
 * #include <rosidl_generator_cpp/action_type_support.hpp>
 * #include <example_interfaces/action/fibonacci.h>
 * using rosidl_typesupport_cpp::get_action_type_support_handle;
 * const rosidl_action_type_support_t * ts =
 *   get_action_type_support_handle<example_interfaces::action::Fibonacci>();
 * ```
 *
 * The rosidl_action_type_support_t object contains action type specific
 * information used to send or take goals, results, and feedback.
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
 * ret = rcl_action_goal_handle_init(&goal_handle, &action_server, ts);
 * // ... error handling, and on shutdown do finalization:
 * ret = rcl_action_goal_handle_fini(&goal_handle, &action_server);
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
 * \param[out] goal_handle preallocated goal handle structure
 * \param[in] action_server valid rcl action server
 * \param[in] type_support type support object for the action's type
 * \return `RCL_RET_OK` if goal_handle was initialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_init(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_action_server_t * action_server,
  const rosidl_action_type_support_t * type_support);

/// Finalize a rcl action goal handle.
/**
 * After calling, goal_handle will no longer be valid and action_server will no longer
 * track the goal associated with goal_handle.
 *
 * After calling, calls to rcl_action_publish_feedback(), rcl_action_publish_status(),
 * rcl_action_update_goal_state(), rcl_action_goal_handle_get_status(),
 * rcl_action_goal_handle_is_active(), TODO, rcl_action_goal_handle_get_message(), and
 * rcl_action_goal_handle_get_id() will fail when using this goal_handle.
 *
 * However, the given action_server is still valid.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] goal_handle struct to be deinitialized
 * \param[in] action_server used to create the goal handle
 * \return `RCL_RET_OK` if the goal handle was deinitialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_fini(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_action_server_t * action_server);

/// Update the state of a goal.
/**
 * TODO: docs
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_update_goal_state(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_action_goal_state state,
  rcl_allocator_t * error_msg_allocator);

/// Get the ID of a goal.
/**
 * TODO: docs
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_get_id()

/// Get the request message associated with a goal.
/**
 * TODO: docs
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_get_message()

/// Get the status of a goal.
/**
 * TODO: docs
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_get_status()

/// Check if the goal is active.
/**
 * A goal is active if it is in one of the following states: ACCEPTED, EXECUTING, or CANCELING.
 * TODO: docs
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_is_active(),

/// Check if a goal handle is valid.
/**
 * TODO: docs
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_handle_is_valid(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_allocator_t * error_msg_allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__GOAL_HANDLE_H_
