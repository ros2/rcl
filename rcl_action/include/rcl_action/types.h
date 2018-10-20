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

#include "action_msgs/msg/goal_info.h"
#include "action_msgs/msg/goal_status.h"
#include "action_msgs/msg/goal_status_array.h"


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

// Typedef generated messages for convenience
typedef action_msgs__msg__GoalInfo rcl_action_goal_info_t;
typedef action_msgs__msg__GoalStatusArray rcl_action_goal_status_array_t;
typedef action_msgs__srv__Cancel__Request rcl_action_cancel_request_t;
typedef action_msgs__srv__Cancel__Response rcl_action_cancel_response_t;

/// Goal states
// TODO(jacobperron): Let states be defined by action_msgs/msg/goal_status.h
// Ideally, we could use an enum type directly from the message when the feature
// is available. Issue: https://github.com/ros2/rosidl/issues/260
int8_t GOAL_STATE_UNKNOWN = action_msgs__msg__GoalStatus__STATUS_UNKNOWN;
int8_t GOAL_STATE_ACCEPTED = action_msgs__msg__GoalStatus__STATUS_ACCEPTED;
int8_t GOAL_STATE_EXECUTING = action_msgs__msg__GoalStatus__STATUS_EXECUTING;
int8_t GOAL_STATE_CANCELING = action_msgs__msg__GoalStatus__STATUS_CANCELING;
int8_t GOAL_STATE_SUCCEEDED = action_msgs__msg__GoalStatus__STATUS_SUCCEEDED;
int8_t GOAL_STATE_CANCELED = action_msgs__msg__GoalStatus__STATUS_CANCELED;
int8_t GOAL_STATE_ABORTED = action_msgs__msg__GoalStatus__STATUS_ABORTED;
int8_t GOAL_STATE_NUM_STATES = 6;  // not counting `UNKNOWN`

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

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__TYPES_H_
