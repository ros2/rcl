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

#ifndef RCL_ACTION__GOAL_STATE_MACHINE_H_
#define RCL_ACTION__GOAL_STATE_MACHINE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/visibility_control.h"


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
} rcl_action_status_t;

/// Transition events
typedef enum rcl_action_goal_event_t
{
  GOAL_EVENT_EXECUTE = 0,
  GOAL_EVENT_CANCEL,
  GOAL_EVENT_SET_SUCCEEDED,
  GOAL_EVENT_SET_ABORTED,
  GOAL_EVENT_SET_CANCELED,
  GOAL_EVENT_NUM_EVENTS
} rcl_action_goal_event_t;

typedef rcl_action_goal_state_t
(* rcl_action_goal_event_handler)(rcl_action_goal_state_t, rcl_action_goal_event_t);

// Transition event handlers
rcl_action_goal_state_t
_execute_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_ACCEPTED != state || GOAL_EVENT_EXECUTE != event) {
    return GOAL_STATE_INVALID;
  }
  return GOAL_STATE_EXECUTING;
}

rcl_action_goal_state_t
_cancel_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_ACCEPTED != state ||
    GOAL_STATE_EXECUTING != state ||
    GOAL_EVENT_CANCEL != event)
  {
    return GOAL_STATE_INVALID;
  }
  return GOAL_STATE_CANCELING;
}

rcl_action_goal_state_t
_set_succeeded_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_EXECUTING != state ||
    GOAL_STATE_CANCELING != state ||
    GOAL_EVENT_SET_SUCCEEDED != event)
  {
    return GOAL_STATE_INVALID;
  }
  return GOAL_STATE_SUCCEEDED;
}

rcl_action_goal_state_t
_set_aborted_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_EXECUTING != state ||
    GOAL_STATE_CANCELING != state ||
    GOAL_EVENT_SET_ABORTED != event)
  {
    return GOAL_STATE_INVALID;
  }
  return GOAL_STATE_ABORTED;
}

rcl_action_goal_state_t
_set_canceled_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_CANCELING != state || GOAL_EVENT_SET_CANCELED != event) {
    return GOAL_STATE_INVALID;
  }
  return GOAL_STATE_CANCELED;
}

// Transition map
rcl_action_goal_event_handler _goal_state_transition_map[NUM_EVENTS][NUM_STATES] = {
  [GOAL_STATE_ACCEPTED] = {
    [GOAL_EVENT_EXECUTE] = _execute_event_handler,
    [GOAL_EVENT_CANCEL] = _cancel_event_handler,
  },
  [GOAL_STATE_EXECUTING] = {
    [GOAL_EVENT_CANCEL] = _cancel_event_handler,
    [GOAL_EVENT_SET_SUCCEEDED] = _set_succeded_event_handler,
    [GOAL_EVENT_SET_ABORTED] = _set_aborted_event_handler,
  },
  [GOAl_STATE_CANCELING] = {
    [GOAL_EVENT_SET_SUCCEEDED] = _set_succeeded_event_handler,
    [GOAL_EVENT_SET_ABORTED] = _set_aborted_event_handler,
    [GOAL_EVENT_SET_CANCELED] = _set_canceled_event_handler,
  },
};

/// Transition a goal from one state to the next.
/**
 * \todo TODO(jacobperron): Document
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_goal_state_t
rcl_action_transition_goal_state(
  const rcl_action_goal_state_t state,
  const rcl_action_goal_event_t event)
{
  rcl_action_goal_event_handler handler = _goal_state_transition_map[event][state];
  if (NULL == handler) {
    return GOAL_STATE_INVALID;
  }
  return handler(state, event);
}

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__GOAL_STATE_MACHINE_H_
