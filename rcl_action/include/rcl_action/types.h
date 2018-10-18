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

/// Goal states
typedef enum rcl_action_goal_state_t
{
  GOAL_STATE_ACCEPTED = 0,
  GOAL_STATE_EXECUTING,
  GOAL_STATE_CANCELING,
  GOAL_STATE_SUCCEEDED,
  GOAL_STATE_CANCELED,
  GOAL_STATE_ABORTED,
  GOAL_STATE_NUM_STATES,
  GOAL_STATE_INVALID
} rcl_action_goal_state_t;

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

/// Information for a goal
/**
 * Contains metadata for a goal such as the goal ID.
 */
typedef struct rcl_action_goal_info_t
{
  /// UUID for a goal
  uint8_t[16] id;
  /// Time when the goal was created or accepted, depending on the use case.
  /**
   * Number of seconds since epoch.
   */
  uint64_t timestamp;
} rcl_action_goal_info_t;

/// Encapsulates a cancel response
typedef struct rcl_action_cancel_response_t
{
  /// Number of goals that transitioned to CANCELING
  /**
   * This is also the number of elements in the `goals_canceling` array.
   */
  uint32_t num_canceling;
  /// Array of goal info for those goals that transitioned to CANCELING
  rcl_action_goal_info_t * goals_canceling;
} rcl_action_cancel_response_t;

/// Status message
typedef struct rcl_action_status_t
{
  /// Goal metadata
  rcl_action_goal_info_t goal_info;
  /// Status of the goal
  rcl_action_goal_state_t status;
} rcl_action_status_t;

/// Status array message
typedef struct rcl_action_status_array_t
{
  /// Number of elements in the array
  uint32_t num_status;
  /// The array of statuses
  rcl_action_status_t * statuses;
} rcl_action_status_array_t;

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__TYPES_H_
