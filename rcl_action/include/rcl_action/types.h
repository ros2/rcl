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

#ifndef RCL_ACTION__TYPES_H_
#define RCL_ACTION__TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/visibility_control.h"

#include "action_msgs/msg/goal_info.h"
#include "action_msgs/msg/goal_status.h"
#include "action_msgs/msg/goal_status_array.h"
#include "action_msgs/srv/cancel_goal.h"

#include "rcl/macros.h"
#include "rcl/types.h"


// rcl action specific ret codes in 2XXX
/// Action name does not pass validation return code.
#define RCL_RET_ACTION_NAME_INVALID 2000
/// Action goal accepted return code.
#define RCL_RET_ACTION_GOAL_ACCEPTED 2100
/// Action goal rejected return code.
#define RCL_RET_ACTION_GOAL_REJECTED 2101
/// Action client is invalid return code.
#define RCL_RET_ACTION_CLIENT_INVALID 2102
/// Action client failed to take response return code.
#define RCL_RET_ACTION_CLIENT_TAKE_FAILED 2103
/// Action server is invalid return code.
#define RCL_RET_ACTION_SERVER_INVALID 2200
/// Action server failed to take request return code.
#define RCL_RET_ACTION_SERVER_TAKE_FAILED 2201
/// Action goal handle invalid return code.
#define RCL_RET_ACTION_GOAL_HANDLE_INVALID 2300
/// Action invalid event return code.
#define RCL_RET_ACTION_GOAL_EVENT_INVALID 2301

// Forward declare
typedef struct rcl_action_server_t rcl_action_server_t;

// Typedef generated messages for convenience
typedef action_msgs__msg__GoalInfo rcl_action_goal_info_t;
typedef action_msgs__msg__GoalStatusArray rcl_action_goal_status_array_t;
typedef action_msgs__srv__CancelGoal_Request rcl_action_cancel_request_t;
typedef action_msgs__srv__CancelGoal_Response rcl_action_cancel_response_t;

/// Goal states
// TODO(jacobperron): Let states be defined by action_msgs/msg/goal_status.h
// Ideally, we could use an enum type directly from the message when the feature
// is available. Issue: https://github.com/ros2/rosidl/issues/260
typedef int8_t rcl_action_goal_state_t;
#define GOAL_STATE_UNKNOWN action_msgs__msg__GoalStatus__STATUS_UNKNOWN
#define GOAL_STATE_ACCEPTED action_msgs__msg__GoalStatus__STATUS_ACCEPTED
#define GOAL_STATE_EXECUTING action_msgs__msg__GoalStatus__STATUS_EXECUTING
#define GOAL_STATE_CANCELING action_msgs__msg__GoalStatus__STATUS_CANCELING
#define GOAL_STATE_SUCCEEDED action_msgs__msg__GoalStatus__STATUS_SUCCEEDED
#define GOAL_STATE_CANCELED action_msgs__msg__GoalStatus__STATUS_CANCELED
#define GOAL_STATE_ABORTED action_msgs__msg__GoalStatus__STATUS_ABORTED
#define GOAL_STATE_NUM_STATES 6  // not counting `UNKNOWN`

/// Goal state transition events
typedef enum rcl_action_goal_event_t
{
  GOAL_EVENT_EXECUTE = 0,
  GOAL_EVENT_CANCEL,
  GOAL_EVENT_SET_SUCCEEDED,
  GOAL_EVENT_SET_ABORTED,
  GOAL_EVENT_SET_CANCELED,
  GOAL_EVENT_NUM_EVENTS
} rcl_action_goal_event_t;

/// Return a rcl_action_goal_info_t with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_goal_info_t before passing to
 * rcl_action_goal_info_init().
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_action_goal_info_t
rcl_action_get_zero_initialized_goal_info(void);

/// Initialize a rcl_action_goal_info_t.
/**
 * After calling this function on a rcl_action_goal_info_t, it can be populated
 * and used to accept goal requests with rcl_action_accept_new_goal().
 *
 * The given rcl_action_server_t must be valid and the resulting
 * rcl_action_goal_info_t is only valid as long as the given rcl_action_server_t
 * remains valid.
 *
 * Expected usage (for C action servers):
 *
 * ```c
 * #include <rcl_action/rcl_action.h>
 *
 * // ... init action server
 * rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();
 * ret = rcl_action_goal_info_init(&goal_info, &action_server);
 * // ... error handling, and when done with the goal info message, finalize
 * ret = rcl_action_goal_info_fini(&goal_info, &action_server);
 * // ... error handling
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
 * \param[out] goal_info a preallocated, zero-initialized, goal info message
 *   to be initialized.
 * \param[in] action_server a valid action server handle
 * \return `RCL_RET_OK` if goal info was initialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_info_init(
  rcl_action_goal_info_t * goal_info,
  const rcl_action_server_t * action_server);

/// Finalize a rcl_action_goal_info_t.
/**
 * After calling, the goal info message will no longer be valid.
 * However, the given action server handle is still valid.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] goal_info the goal info message to be deinitialized
 * \param[in] action_server handle to the action sever used to create the goal info message
 * \return `RCL_RET_OK` if the goal info message was deinitialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_goal_info_fini(
  rcl_action_goal_info_t * goal_info,
  const rcl_action_server_t * action_server);

/// Return a rcl_action_goal_status_array_t with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_goal_status_array_t before passing to
 * rcl_action_server_get_goal_status_array().
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_action_goal_status_array_t
rcl_action_get_zero_initialized_goal_status_array(void);

/// Return a rcl_action_cancel_request_t with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_cancel_request_t before passing to
 *
 * rcl_action_cancel_request_init().
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_action_cancel_request_t
rcl_action_get_zero_initialized_cancel_request(void);

/// Initialize a rcl_action_cancel_request_t.
/**
 * After calling this function on a rcl_action_cancel_request_t, it can be populated
 * and used to process cancel requests with an action server using
 * rcl_action_process_cancel_request().
 *
 * The given rcl_action_server_t must be valid and the resulting
 * rcl_action_cancel_request_t is only valid as long as the given rcl_action_server_t
 * remains valid.
 *
 * Expected usage (for C action servers):
 *
 * ```c
 * #include <rcl/rcl.h>
 * #include <rcl_action/rcl_action.h>
 *
 * // ... init action server
 * rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
 * ret = rcl_action_cancel_request_init(&cancel_request, &action_server);
 * // ... error handling, and when done processing request, finalize
 * ret = rcl_action_cancel_request_fini(&cancel_request, &action_server);
 * // ... error handling
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
 * \param[out] cancel_request a preallocated, zero-initialized, cancel request message
 *   to be initialized.
 * \param[in] action_server a valid action server handle
 * \return `RCL_RET_OK` if cancel request was initialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_cancel_request_init(
  rcl_action_cancel_request_t * cancel_request,
  const rcl_action_server_t * action_server);

/// Finalize a rcl_action_cancel_request_t.
/**
 * After calling, the cancel request message will no longer be valid.
 * However, the given action server handle is still valid.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] cancel_request the cancel request message to be deinitialized
 * \param[in] action_server handle to the action sever used to create the cancel request
 * \return `RCL_RET_OK` if the cancel request was deinitialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_cancel_request_fini(
  rcl_action_cancel_request_t * cancel_request,
  const rcl_action_server_t * action_server);

/// Return a rcl_action_cancel_response_t with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_cancel_response_t before passing to
 * rcl_action_cancel_response_init().
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_action_cancel_response_t
rcl_action_get_zero_initialized_cancel_response(void);

/// Initialize a rcl_action_cancel_response_t.
/**
 * After calling this function on a rcl_action_cancel_response_t, it can be populated
 * and used to process cancel requests with an action server using
 * rcl_action_process_cancel_request().
 *
 * The given rcl_action_server_t must be valid and the resulting
 * rcl_action_cancel_response_t is only valid as long as the given rcl_action_server_t
 * remains valid.
 *
 * Expected usage (for C action servers):
 *
 * ```c
 * #include <rcl/rcl.h>
 * #include <rcl_action/rcl_action.h>
 *
 * // ... init action server
 * rcl_action_cancel_response_t cancel_response =
 *   rcl_action_get_zero_initialized_cancel_response();
 * ret = rcl_action_cancel_response_init(&cancel_response, &action_server);
 * // ... error handling, and when done processing response, finalize
 * ret = rcl_action_cancel_response_fini(&cancel_response, &action_server);
 * // ... error handling
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
 * \param[out] cancel_response a preallocated, zero-initialized, cancel response message
 *   to be initialized.
 * \param[in] action_server a valid action server handle
 * \return `RCL_RET_OK` if cancel response was initialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_cancel_response_init(
  rcl_action_cancel_response_t * cancel_response,
  const rcl_action_server_t * action_server);

/// Finalize a rcl_action_cancel_response_t.
/**
 * After calling, the cancel response message will no longer be valid.
 * However, the given action server handle is still valid.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] cancel_response the cancel response message to be deinitialized
 * \param[in] action_server handle to the action sever used to create the cancel response
 * \return `RCL_RET_OK` if the cancel response was deinitialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_cancel_response_fini(
  rcl_action_cancel_response_t * cancel_response,
  rcl_action_server_t * action_server);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__TYPES_H_
