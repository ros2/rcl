// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lifecycle_msgs/msg/state.h>
#include <lifecycle_msgs/msg/transition.h>

#include "states.hxx"
#include "rcl_lifecycle/transition_map.h"

#include "default_state_machine.hxx"

#if __cplusplus
extern "C"
{
#endif

// *INDENT-OFF*
// Primary States
const rcl_lifecycle_state_t rcl_state_unknown
  = {"unknown", lifecycle_msgs__msg__State__PRIMARY_STATE_UNKNOWN};
const rcl_lifecycle_state_t rcl_state_unconfigured
  = {"unconfigured", lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED};
const rcl_lifecycle_state_t rcl_state_inactive
  = {"inactive", lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE};
const rcl_lifecycle_state_t rcl_state_active
  = {"active", lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE};
const rcl_lifecycle_state_t rcl_state_finalized
  = {"finalized", lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED};
// Transition States
const rcl_lifecycle_state_t rcl_state_configuring
  = {"configuring", lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING};
const rcl_lifecycle_state_t rcl_state_cleaningup
  = {"cleaningup", lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP};
const rcl_lifecycle_state_t rcl_state_shuttingdown
  = {"shuttingdown", lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN};
const rcl_lifecycle_state_t rcl_state_activating
  = {"activating", lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING};
const rcl_lifecycle_state_t rcl_state_deactivating
  = {"deactivating", lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING};
const rcl_lifecycle_state_t rcl_state_errorprocessing
  = {"errorprocessing", lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING};
// Transitions
const rcl_lifecycle_transition_t rcl_transition_configure
  = {"configure", lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
     NULL, NULL, NULL};
const rcl_lifecycle_transition_t rcl_transition_cleanup
  = {"cleanup", lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    NULL, NULL, NULL};
const rcl_lifecycle_transition_t rcl_transition_shutdown
  = {"shutdown", lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
  NULL, NULL, NULL};
const rcl_lifecycle_transition_t rcl_transition_activate
  = {"activate", lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
  NULL, NULL, NULL};
const rcl_lifecycle_transition_t rcl_transition_deactivate
  = {"deactivate", lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
  NULL, NULL, NULL};
// *INDENT-ON*

// default implementation as despicted on
// design.ros2.org
rcl_ret_t
rcl_lifecycle_init_default_state_machine(rcl_lifecycle_state_machine_t * state_machine)
{
  // Maybe we can directly store only pointers to states
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_unknown);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_unconfigured);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_inactive);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_active);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_finalized);
  // transition states
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_configuring);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_cleaningup);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_shuttingdown);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_activating);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_deactivating);
  rcl_lifecycle_register_state(&state_machine->transition_map, rcl_state_errorprocessing);

  // add transitions to map
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure,
    &rcl_state_unconfigured,
    &rcl_state_configuring,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure,
    &rcl_state_configuring,
    &rcl_state_inactive,
    &rcl_state_errorprocessing);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup,
    &rcl_state_inactive,
    &rcl_state_cleaningup,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup,
    &rcl_state_cleaningup,
    &rcl_state_unconfigured,
    &rcl_state_errorprocessing);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown,
    &rcl_state_unconfigured,
    &rcl_state_shuttingdown,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown,
    &rcl_state_inactive,
    &rcl_state_shuttingdown,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown,
    &rcl_state_active,
    &rcl_state_shuttingdown,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown,
    &rcl_state_shuttingdown,
    &rcl_state_finalized,
    &rcl_state_errorprocessing);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate,
    &rcl_state_inactive,
    &rcl_state_activating,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate,
    &rcl_state_activating,
    &rcl_state_active,
    &rcl_state_errorprocessing);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate,
    &rcl_state_active,
    &rcl_state_deactivating,
    &rcl_state_errorprocessing);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate,
    &rcl_state_deactivating,
    &rcl_state_inactive,
    &rcl_state_errorprocessing);

  // two more transitions from errorprocessing to finalized or shtudown
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown,
    &rcl_state_errorprocessing,
    &rcl_state_finalized,
    &rcl_state_finalized);  // shutdown in case of failure
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup,
    &rcl_state_errorprocessing,
    &rcl_state_unconfigured,
    &rcl_state_finalized);  // shutdown in case of failure

  state_machine->current_state = &rcl_state_unconfigured;
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif  // extern "C"
