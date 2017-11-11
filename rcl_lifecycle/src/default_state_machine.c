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

#include "lifecycle_msgs/msg/state.h"
#include "lifecycle_msgs/msg/transition.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

#include "rcl_lifecycle/transition_map.h"

#include "default_state_machine.h"  // NOLINT
#include "states.h"  // NOLINT

#if __cplusplus
extern "C"
{
#endif

rcl_lifecycle_transition_key_t * empty_transition_key = NULL;
rcl_lifecycle_transition_t * empty_transition = NULL;
unsigned int empty_transition_size = 0;

// Primary States
rcl_lifecycle_state_t rcl_state_unknown = {
  "unknown", lifecycle_msgs__msg__State__PRIMARY_STATE_UNKNOWN, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_unconfigured = {
  "unconfigured", lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_inactive = {
  "inactive", lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_active = {
  "active", lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_finalized = {
  "finalized", lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED, NULL, NULL, 0
};
// Transition States
rcl_lifecycle_state_t rcl_state_configuring = {
  "configuring", lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_cleaningup = {
  "cleaningup", lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_shuttingdown = {
  "shuttingdown", lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_activating = {
  "activating", lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_deactivating = {
  "deactivating", lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING, NULL, NULL, 0
};
rcl_lifecycle_state_t rcl_state_errorprocessing = {
  "errorprocessing", lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING, NULL, NULL, 0
};

// Transitions
const rcl_lifecycle_transition_t rcl_transition_configure = {
  "configure", lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
  &rcl_state_unconfigured, &rcl_state_configuring
};
const rcl_lifecycle_transition_t rcl_transition_configure_success = {
  "configure_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_CONFIGURE_SUCCESS,
  &rcl_state_configuring, &rcl_state_inactive
};
const rcl_lifecycle_transition_t rcl_transition_configure_failure = {
  "configure_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_CONFIGURE_FAILURE,
  &rcl_state_configuring, &rcl_state_unconfigured
};
const rcl_lifecycle_transition_t rcl_transition_configure_error = {
  "configure_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_ERROR,
  &rcl_state_configuring, &rcl_state_errorprocessing
};

const rcl_lifecycle_transition_t rcl_transition_cleanup = {
  "cleanup", lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
  &rcl_state_inactive, &rcl_state_cleaningup
};
const rcl_lifecycle_transition_t rcl_transition_cleanup_success = {
  "cleanup_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_SUCCESS,
  &rcl_state_cleaningup, &rcl_state_unconfigured
};
const rcl_lifecycle_transition_t rcl_transition_cleanup_failure = {
  "cleanup_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_FAILURE,
  &rcl_state_cleaningup, &rcl_state_inactive
};
const rcl_lifecycle_transition_t rcl_transition_cleanup_error = {
  "cleanup_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_CLEANUP_ERROR,
  &rcl_state_cleaningup, &rcl_state_errorprocessing
};

const rcl_lifecycle_transition_t rcl_transition_activate = {
  "activate", lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
  &rcl_state_inactive, &rcl_state_activating
};
const rcl_lifecycle_transition_t rcl_transition_activate_success = {
  "activate_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_ACTIVATE_SUCCESS,
  &rcl_state_activating, &rcl_state_active
};
const rcl_lifecycle_transition_t rcl_transition_activate_failure = {
  "activate_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_ACTIVATE_FAILURE,
  &rcl_state_activating, &rcl_state_inactive
};
const rcl_lifecycle_transition_t rcl_transition_activate_error = {
  "activate_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_ACTIVATE_ERROR,
  &rcl_state_activating, &rcl_state_errorprocessing
};

const rcl_lifecycle_transition_t rcl_transition_deactivate = {
  "deactivate", lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
  &rcl_state_active, &rcl_state_deactivating
};
const rcl_lifecycle_transition_t rcl_transition_deactivate_success = {
  "deactivate_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_DEACTIVATE_SUCCESS,
  &rcl_state_deactivating, &rcl_state_inactive
};
const rcl_lifecycle_transition_t rcl_transition_deactivate_failure = {
  "deactivate_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_DEACTIVATE_FAILURE,
  &rcl_state_deactivating, &rcl_state_active
};
const rcl_lifecycle_transition_t rcl_transition_deactivate_error = {
  "deactivate_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_DEACTIVATE_ERROR,
  &rcl_state_deactivating, &rcl_state_errorprocessing
};

const rcl_lifecycle_transition_t rcl_transition_unconfigured_shutdown = {
  "unconfigured_shutdown", lifecycle_msgs__msg__Transition__TRANSITION_UNCONFIGURED_SHUTDOWN,
  &rcl_state_unconfigured, &rcl_state_shuttingdown
};
const rcl_lifecycle_transition_t rcl_transition_inactive_shutdown = {
  "inactive_shutdown", lifecycle_msgs__msg__Transition__TRANSITION_INACTIVE_SHUTDOWN,
  &rcl_state_inactive, &rcl_state_shuttingdown
};
const rcl_lifecycle_transition_t rcl_transition_active_shutdown = {
  "active_shutdown", lifecycle_msgs__msg__Transition__TRANSITION_ACTIVE_SHUTDOWN,
  &rcl_state_active, &rcl_state_shuttingdown
};
const rcl_lifecycle_transition_t rcl_transition_shutdown_success = {
  "shutdown_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_SHUTDOWN_SUCCESS,
  &rcl_state_shuttingdown, &rcl_state_finalized
};
const rcl_lifecycle_transition_t rcl_transition_shutdown_failure = {
  "shutdown_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_SHUTDOWN_FAILURE,
  &rcl_state_shuttingdown, &rcl_state_finalized
};
const rcl_lifecycle_transition_t rcl_transition_shutdown_error = {
  "shutdown_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_SHUTDOWN_ERROR,
  &rcl_state_shuttingdown, &rcl_state_errorprocessing
};

const rcl_lifecycle_transition_t rcl_transition_error_success = {
  "errorprocessing_success", lifecycle_msgs__msg__Transition__TRANSITION_ON_ERROR_SUCCESS,
  &rcl_state_errorprocessing, &rcl_state_unconfigured
};
const rcl_lifecycle_transition_t rcl_transition_error_failure = {
  "errorprocessing_failure", lifecycle_msgs__msg__Transition__TRANSITION_ON_ERROR_FAILURE,
  &rcl_state_errorprocessing, &rcl_state_finalized
};
const rcl_lifecycle_transition_t rcl_transition_error_error = {
  "errorprocessing_error", lifecycle_msgs__msg__Transition__TRANSITION_ON_ERROR_ERROR,
  &rcl_state_errorprocessing, &rcl_state_finalized
};

// default implementation as despicted on
// design.ros2.org
rcl_ret_t
rcl_lifecycle_init_default_state_machine(
  rcl_lifecycle_state_machine_t * state_machine, const rcutils_allocator_t * allocator)
{
  rcutils_ret_t ret;
  /*
   * we once register all primary states
   */
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_unknown, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_unconfigured, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_inactive, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_active, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_finalized, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  /*
   * we once register all transition states
   */
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_configuring, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_cleaningup, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_shuttingdown, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_activating, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_deactivating, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_state(
    &state_machine->transition_map, rcl_state_errorprocessing, allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  /*
   * we register all transitions from each primary state
   * it registers the transition and the key on which to trigger
   */
  // transition configure
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure_success,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure_failure,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_configure_error,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  // transition cleanup
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup_success,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup_failure,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_cleanup_error,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  // transition activate
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate_success,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate_failure,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_activate_error,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  // transition deactivate
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate_success,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate_failure,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_deactivate_error,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  // transition shutdown
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_unconfigured_shutdown,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_inactive_shutdown,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_active_shutdown,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown_success,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown_failure,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_shutdown_error,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  // error state
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_error_success,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_error_failure,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }
  ret = rcl_lifecycle_register_transition(
    &state_machine->transition_map,
    rcl_transition_error_error,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    allocator);
  if (ret != RCL_RET_OK) {
    goto fail;
  }

  state_machine->current_state = &rcl_state_unconfigured;

  return RCL_RET_OK;

fail:
  if (rcl_lifecycle_transition_map_fini(&state_machine->transition_map, allocator) != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("could not free lifecycle transition map. Leaking memory!\n",
      rcl_get_default_allocator());
  }
  return RCL_RET_ERROR;
}

#if __cplusplus
}
#endif  // extern "C"
