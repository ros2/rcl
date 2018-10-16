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

#include <uuid/uuid.h>


// rcl action specific ret codes in 2XXX
/// Action name does not pass validation return code.
#define RCL_RET_ACTION_NAME_INVALID 2000
/// Action client is invalid return code.
#define RCL_RET_ACTION_CLIENT_INVALID 2100
/// Action client failed to take response return code.
#define RCL_RET_ACTION_CLIENT_TAKE_FAILED 2101
/// Action server is invalid return code.
#define RCL_RET_ACTION_SERVER_INVALID 2200
/// Action server failed to take request return code.
#define RCL_RET_ACTION_SERVER_TAKE_FAILED 2201

/// Action goal ID
typedef struct rcl_action_goal_id_t
{
  /// UUID for a goal
  uuid_t id;
  /// Timestamp. Number of seconds since epoch
  uint64_t timestamp;
} rcl_action_goal_id_t;

/// Action goal status
typedef enum rcl_action_status_t
{
  ACCEPTED,
  EXECUTING,
  CANCELING,
  SUCCEEDED,
  CANCELED,
  ABORTED
} rcl_action_status_t;

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__TYPES_H_
