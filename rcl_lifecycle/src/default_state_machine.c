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

#include "rcl_lifecycle/transition_map.h"

#include "default_state_machine.h"  // NOLINT
#include "states.h"  // NOLINT

#if __cplusplus
extern "C"
{
#endif

// Primary States
const rcl_lifecycle_state_t rcl_state_unknown =
{"unknown", lifecycle_msgs__msg__State__PRIMARY_STATE_UNKNOWN,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_unconfigured =
{"unconfigured", lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_inactive =
{"inactive", lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_active =
{"active", lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_finalized =
{"finalized", lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED,
 NULL, NULL, 0};
// Transition States
const rcl_lifecycle_state_t rcl_state_configuring =
{"configuring", lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_cleaningup =
{"cleaningup", lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_shuttingdown =
{"shuttingdown", lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_activating =
{"activating", lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_deactivating =
{"deactivating", lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
 NULL, NULL, 0};
const rcl_lifecycle_state_t rcl_state_errorprocessing =
{"errorprocessing", lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
 NULL, NULL, 0};

// Transitions
const rcl_lifecycle_transition_t rcl_transition_configure =
{"configure", lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
 &rcl_state_unconfigured, &rcl_state_configuring};
const rcl_lifecycle_transition_t rcl_transition_configure_success =
{"configure_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_CONFIGURE_SUCCESS,
 &rcl_state_configuring, &rcl_state_inactive};
const rcl_lifecycle_transition_t rcl_transition_configure_failure =
{"configure_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_CONFIGURE_FAILURE,
 &rcl_state_configuring, &rcl_state_unconfigured};
const rcl_lifecycle_transition_t rcl_transition_configure_error =
{"configure_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_ERROR,
 &rcl_state_configuring, &rcl_state_errorprocessing};

const rcl_lifecycle_transition_t rcl_transition_cleanup =
{"cleanup", lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
 &rcl_state_inactive, &rcl_state_cleaningup};
const rcl_lifecycle_transition_t rcl_transition_cleanup_success =
{"cleanup_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_SUCCESS,
 &rcl_state_cleaningup, &rcl_state_unconfigured};
const rcl_lifecycle_transition_t rcl_transition_cleanup_failure =
{"cleanup_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_FAILURE,
 &rcl_state_cleaningup, &rcl_state_inactive};
const rcl_lifecycle_transition_t rcl_transition_cleanup_error =
{"cleanup_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_ERROR,
 &rcl_state_cleaningup, &rcl_state_errorprocessing};

const rcl_lifecycle_transition_t rcl_transition_activate =
{"activate", lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
 &rcl_state_inactive, &rcl_state_activating};
const rcl_lifecycle_transition_t rcl_transition_activate_success =
{"activate_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_ACTIVATE_SUCCESS,
 &rcl_state_activating, &rcl_state_active};
const rcl_lifecycle_transition_t rcl_transition_activate_failure =
{"activate_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_ACTIVATE_FAILURE,
 &rcl_state_activating, &rcl_state_inactive};
const rcl_lifecycle_transition_t rcl_transition_activate_error =
{"activate_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_ACTIVATE_ERROR,
 &rcl_state_activating, &rcl_state_errorprocessing};

const rcl_lifecycle_transition_t rcl_transition_deactivate =
{"deactivate", lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
 &rcl_state_active, &rcl_state_deactivating};
const rcl_lifecycle_transition_t rcl_transition_deactivate_success =
{"deactivate_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_DEACTIVATE_SUCCESS,
 &rcl_state_deactivating, &rcl_state_inactive};
const rcl_lifecycle_transition_t rcl_transition_deactivate_failure =
{"deactivate_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_DEACTIVATE_FAILURE,
 &rcl_state_deactivating, &rcl_state_active};
const rcl_lifecycle_transition_t rcl_transition_deactivate_error =
{"deactivate_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_DEACTIVATE_ERROR,
 &rcl_state_deactivating, &rcl_state_errorprocessing};

const rcl_lifecycle_transition_t rcl_transition_unconfigured_shutdown =
{"unconfigured_shutdown", lifecycle_msgs__msg__Transition__TRANSITION_UNCONFIGURED_SHUTDOWN,
 &rcl_state_unconfigured, &rcl_state_shuttingdown};
const rcl_lifecycle_transition_t rcl_transition_inactive_shutdown =
{"inactive_shutdown", lifecycle_msgs__msg__Transition__TRANSITION_INACTIVE_SHUTDOWN,
 &rcl_state_inactive, &rcl_state_shuttingdown};
const rcl_lifecycle_transition_t rcl_transition_active_shutdown =
{"active_shutdown", lifecycle_msgs__msg__Transition__TRANSITION_ACTIVE_SHUTDOWN,
 &rcl_state_active, &rcl_state_shuttingdown};
const rcl_lifecycle_transition_t rcl_transition_shutdown_success =
{"shutdown_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_SHUTDOWN_SUCCESS,
 &rcl_state_shuttingdown, &rcl_state_finalized};
const rcl_lifecycle_transition_t rcl_transition_shutdown_failure =
{"shutdown_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_SHUTDOWN_FAILURE,
 &rcl_state_shuttingdown, &rcl_state_finalized};
const rcl_lifecycle_transition_t rcl_transition_shutdown_error =
{"shutdown_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_SHUTDOWN_ERROR,
 &rcl_state_shuttingdown, &rcl_state_errorprocessing};

const rcl_lifecycle_transition_t rcl_transition_error_success =
{"errorprocessing_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_ERROR_SUCCESS,
 &rcl_state_errorprocessing, &rcl_state_unconfigured};
const rcl_lifecycle_transition_t rcl_transition_error_failure =
{"errorprocessing_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_ERROR_FAILURE,
 &rcl_state_errorprocessing, &rcl_state_finalized};
const rcl_lifecycle_transition_t rcl_transition_error_error =
{"errorprocessing_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_ERROR_ERROR,
 &rcl_state_errorprocessing, &rcl_state_finalized};
// *INDENT-ON*

// default implementation as despicted on
// design.ros2.org
rcl_ret_t
rcl_lifecycle_init_default_state_machine(rcl_lifecycle_state_machine_t * state_machine)
{
  // Maybe we can directly store only pointers to states
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_unknown);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_unconfigured);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_inactive);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_active);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_finalized);
  // transition states
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_configuring);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_cleaningup);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_shuttingdown);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_activating);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_deactivating);
  rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_errorprocessing);

  // add transitions to map
  // TRANSITION_CONFIGURE
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure_success,
    RCL_LIFECYCLE_RET_OK);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure_failure,
    RCL_LIFECYCLE_RET_FAILURE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure_error,
    RCL_LIFECYCLE_RET_ERROR);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup_success,
    RCL_LIFECYCLE_RET_OK);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup_failure,
    RCL_LIFECYCLE_RET_FAILURE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup_error,
    RCL_LIFECYCLE_RET_ERROR);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate_success,
    RCL_LIFECYCLE_RET_OK);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate_failure,
    RCL_LIFECYCLE_RET_FAILURE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate_error,
    RCL_LIFECYCLE_RET_ERROR);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate_success,
    RCL_LIFECYCLE_RET_OK);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate_failure,
    RCL_LIFECYCLE_RET_FAILURE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate_error,
    RCL_LIFECYCLE_RET_ERROR);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_unconfigured_shutdown,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_inactive_shutdown,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_active_shutdown,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN);

  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown_success,
    RCL_LIFECYCLE_RET_OK);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown_failure,
    RCL_LIFECYCLE_RET_FAILURE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown_error,
    RCL_LIFECYCLE_RET_ERROR);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_error_success,
    RCL_LIFECYCLE_RET_OK);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_error_failure,
    RCL_LIFECYCLE_RET_FAILURE);
  rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_error_error,
    RCL_LIFECYCLE_RET_ERROR);
  state_machine->current_state = &rcl_state_unconfigured;
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif  // extern "C"
