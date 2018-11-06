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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/goal_state_machine.h"

typedef rcl_action_goal_state_t
(* rcl_action_goal_event_handler)(rcl_action_goal_state_t, rcl_action_goal_event_t);

// Transition event handlers
rcl_action_goal_state_t
_execute_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_ACCEPTED != state || GOAL_EVENT_EXECUTE != event) {
    return GOAL_STATE_UNKNOWN;
  }
  return GOAL_STATE_EXECUTING;
}

rcl_action_goal_state_t
_cancel_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if ((GOAL_STATE_ACCEPTED != state && GOAL_STATE_EXECUTING != state) ||
    GOAL_EVENT_CANCEL != event)
  {
    return GOAL_STATE_UNKNOWN;
  }
  return GOAL_STATE_CANCELING;
}

rcl_action_goal_state_t
_set_succeeded_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if ((GOAL_STATE_EXECUTING != state && GOAL_STATE_CANCELING != state) ||
    GOAL_EVENT_SET_SUCCEEDED != event)
  {
    return GOAL_STATE_UNKNOWN;
  }
  return GOAL_STATE_SUCCEEDED;
}

rcl_action_goal_state_t
_set_aborted_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if ((GOAL_STATE_EXECUTING != state && GOAL_STATE_CANCELING != state) ||
    GOAL_EVENT_SET_ABORTED != event)
  {
    return GOAL_STATE_UNKNOWN;
  }
  return GOAL_STATE_ABORTED;
}

rcl_action_goal_state_t
_set_canceled_event_handler(rcl_action_goal_state_t state, rcl_action_goal_event_t event)
{
  if (GOAL_STATE_CANCELING != state || GOAL_EVENT_SET_CANCELED != event) {
    return GOAL_STATE_UNKNOWN;
  }
  return GOAL_STATE_CANCELED;
}

// Transition map
static rcl_action_goal_event_handler
  _goal_state_transition_map[GOAL_STATE_NUM_STATES][GOAL_EVENT_NUM_EVENTS] = {
  [GOAL_STATE_ACCEPTED] = {
    [GOAL_EVENT_EXECUTE] = _execute_event_handler,
    [GOAL_EVENT_CANCEL] = _cancel_event_handler,
  },
  [GOAL_STATE_EXECUTING] = {
    [GOAL_EVENT_CANCEL] = _cancel_event_handler,
    [GOAL_EVENT_SET_SUCCEEDED] = _set_succeeded_event_handler,
    [GOAL_EVENT_SET_ABORTED] = _set_aborted_event_handler,
  },
  [GOAL_STATE_CANCELING] = {
    [GOAL_EVENT_SET_SUCCEEDED] = _set_succeeded_event_handler,
    [GOAL_EVENT_SET_ABORTED] = _set_aborted_event_handler,
    [GOAL_EVENT_SET_CANCELED] = _set_canceled_event_handler,
  },
};

rcl_action_goal_state_t
rcl_action_transition_goal_state(
  const rcl_action_goal_state_t state,
  const rcl_action_goal_event_t event)
{
  // event < 0 is always false since it is an unsigned enum
  if (state < 0 ||
    state >= GOAL_STATE_NUM_STATES ||
    event >= GOAL_EVENT_NUM_EVENTS)
  {
    return GOAL_STATE_UNKNOWN;
  }
  rcl_action_goal_event_handler handler = _goal_state_transition_map[state][event];
  if (NULL == handler) {
    return GOAL_STATE_UNKNOWN;
  }
  return handler(state, event);
}

#ifdef __cplusplus
}
#endif
