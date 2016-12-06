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
const rcl_lifecycle_state_t rcl_state_unknown          = {"unknown", lifecycle_msgs__msg__State__PRIMARY_STATE_UNKNOWN};
const rcl_lifecycle_state_t rcl_state_unconfigured     = {"unconfigured", lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED};
const rcl_lifecycle_state_t rcl_state_inactive         = {"inactive", lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE};
const rcl_lifecycle_state_t rcl_state_active           = {"active", lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE};
const rcl_lifecycle_state_t rcl_state_finalized        = {"finalized", lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED};
// Transition States
const rcl_lifecycle_state_t rcl_state_configuring      = {"configuring", lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING};
const rcl_lifecycle_state_t rcl_state_cleaningup       = {"cleaningup", lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP};
const rcl_lifecycle_state_t rcl_state_shuttingdown     = {"shuttingdown", lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN};
const rcl_lifecycle_state_t rcl_state_activating       = {"activating", lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING};
const rcl_lifecycle_state_t rcl_state_deactivating     = {"deactivating", lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING};
const rcl_lifecycle_state_t rcl_state_errorprocessing  = {"errorprocessing", lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING};
// *INDENT-ON*

rcl_lifecycle_state_t
rcl_lifecycle_create_state(unsigned int id, char * label)
{
  rcl_lifecycle_state_t ret_state = {.id = id, .label = label};
  return ret_state;
}

rcl_lifecycle_state_transition_t
rcl_lifecycle_create_state_transition(unsigned int id, const char * label)
{
  rcl_lifecycle_state_transition_t ret_transition = {{.id = id, .label = label},
                                           NULL, NULL, NULL, &rcl_state_errorprocessing};
  return ret_transition;
}

// default implementation as despicted on
// design.ros2.org
rcl_ret_t
rcl_lifecycle_init_default_state_machine(rcl_lifecycle_state_machine_t * state_machine)
{
  // Maybe we can directly store only pointers to states
  rcl_lifecycle_register_primary_state(&state_machine->transition_map, rcl_state_unknown);
  rcl_lifecycle_register_primary_state(&state_machine->transition_map, rcl_state_unconfigured);
  rcl_lifecycle_register_primary_state(&state_machine->transition_map, rcl_state_inactive);
  rcl_lifecycle_register_primary_state(&state_machine->transition_map, rcl_state_active);
  rcl_lifecycle_register_primary_state(&state_machine->transition_map, rcl_state_finalized);

  rcl_lifecycle_state_transition_t rcl_transition_configuring = rcl_lifecycle_create_state_transition(
    rcl_state_configuring.id, rcl_state_configuring.label);
  rcl_lifecycle_state_transition_t rcl_transition_shuttingdown = rcl_lifecycle_create_state_transition(
    rcl_state_shuttingdown.id, rcl_state_shuttingdown.label);
  rcl_lifecycle_state_transition_t rcl_transition_cleaningup = rcl_lifecycle_create_state_transition(
    rcl_state_cleaningup.id, rcl_state_cleaningup.label);
  rcl_lifecycle_state_transition_t rcl_transition_activating = rcl_lifecycle_create_state_transition(
    rcl_state_activating.id, rcl_state_activating.label);
  rcl_lifecycle_state_transition_t rcl_transition_deactivating = rcl_lifecycle_create_state_transition(
    rcl_state_deactivating.id, rcl_state_deactivating.label);

  // add transitions to map
  // TODO(karsten1987): Add global transition for unknown/resetting
  // TODO(karsten1987): Pointer comparison fails here because of copy!
  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_unconfigured, &rcl_state_inactive, rcl_transition_configuring);
  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_unconfigured, &rcl_state_finalized, rcl_transition_shuttingdown);

  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_inactive, &rcl_state_unconfigured, rcl_transition_cleaningup);
  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_inactive, &rcl_state_finalized, rcl_transition_shuttingdown);
  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_inactive, &rcl_state_active, rcl_transition_activating);

  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_active, &rcl_state_inactive, rcl_transition_deactivating);
  rcl_lifecycle_register_transition_by_state(&state_machine->transition_map,
    &rcl_state_active, &rcl_state_finalized, rcl_transition_shuttingdown);

  // set to first entry
  // state_machine->current_state = &state_machine->transition_map.primary_states[0];
  state_machine->current_state = &rcl_state_unconfigured;
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif  // extern "C"
