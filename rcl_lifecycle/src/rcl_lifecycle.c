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

#if __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/rcl.h"
#include "rcl/error_handling.h"

#include "rcutils/logging_macros.h"

#include "rcl_lifecycle/rcl_lifecycle.h"
#include "rcl_lifecycle/transition_map.h"

#include "com_interface.h"  // NOLINT
#include "default_state_machine.h"  // NOLINT
#include "states.h"  // NOLINT

// get zero initialized state machine here
rcl_lifecycle_state_machine_t
rcl_lifecycle_get_zero_initialized_state_machine()
{
  rcl_lifecycle_state_machine_t state_machine;
  state_machine.current_state = NULL;
  state_machine.transition_map = rcl_lifecycle_get_zero_initialized_transition_map();
  state_machine.com_interface = rcl_lifecycle_get_zero_initialized_com_interface();
  return state_machine;
}

rcl_ret_t
rcl_lifecycle_state_machine_init(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_change_state,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_get_available_states,
  const rosidl_service_type_support_t * ts_srv_get_available_transitions,
  bool default_states,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't initialize state machine, no allocator given\n",
      rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }

  rcl_ret_t ret = rcl_lifecycle_com_interface_init(
    &state_machine->com_interface, node_handle,
    ts_pub_notify,
    ts_srv_change_state, ts_srv_get_state,
    ts_srv_get_available_states, ts_srv_get_available_transitions,
    allocator);
  if (ret != RCL_RET_OK) {
    return RCL_RET_ERROR;
  }

  if (default_states) {
    rcl_ret_t ret =
      rcl_lifecycle_init_default_state_machine(state_machine, allocator);
    if (ret != RCL_RET_OK) {
      // init default state machine might have allocated memory,
      // so we have to call fini
      if (rcl_lifecycle_state_machine_fini(state_machine, node_handle, allocator) != RCL_RET_OK) {
        // error already set
        return RCL_RET_ERROR;
      }
    }
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_state_machine_fini(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't free state machine, no allocator given\n",
      rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }

  rcl_ret_t fcn_ret = RCL_RET_OK;

  if (rcl_lifecycle_com_interface_fini(&state_machine->com_interface, node_handle) != RCL_RET_OK) {
    RCL_SET_ERROR_MSG(
      "could not free lifecycle com interface. Leaking memory!\n", rcl_get_default_allocator());
    fcn_ret = RCL_RET_ERROR;
  }

  if (rcl_lifecycle_transition_map_fini(
      &state_machine->transition_map, allocator) != RCL_RET_OK)
  {
    RCL_SET_ERROR_MSG(
      "could not free lifecycle transition map. Leaking memory!\n", rcl_get_default_allocator());
    fcn_ret = RCL_RET_ERROR;
  }

  return fcn_ret;
}

rcl_ret_t
rcl_lifecycle_state_machine_is_initialized(const rcl_lifecycle_state_machine_t * state_machine)
{
  if (!state_machine->com_interface.srv_get_state.impl) {
    RCL_SET_ERROR_MSG("get_state service is null", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  if (!state_machine->com_interface.srv_change_state.impl) {
    RCL_SET_ERROR_MSG("change_state service is null", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  if (rcl_lifecycle_transition_map_is_initialized(&state_machine->transition_map) != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("transition map is null", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

const rcl_lifecycle_transition_t *
rcl_lifecycle_is_valid_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_lifecycle_transition_key_t key)
{
  unsigned int current_id = state_machine->current_state->id;
  const rcl_lifecycle_state_t * current_state = rcl_lifecycle_get_state(
    &state_machine->transition_map, current_id);

  for (unsigned int i = 0; i < current_state->valid_transition_size; ++i) {
    if (current_state->valid_transition_keys[i] == key) {
      return &current_state->valid_transitions[i];
    }
  }
  RCUTILS_LOG_WARN_NAMED(
    ROS_PACKAGE_NAME,
    "No callback transition matching %d found for current state %s",
    key, state_machine->current_state->label)
  return NULL;
}

rcl_ret_t
rcl_lifecycle_trigger_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_lifecycle_transition_key_t key, bool publish_notification)
{
  const rcl_lifecycle_transition_t * transition =
    rcl_lifecycle_is_valid_transition(state_machine, key);

  // If we have a faulty transition pointer
  if (!transition) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME,
      "No transition found for node %s with key %d",
      state_machine->current_state->label, key)
    RCL_SET_ERROR_MSG("Transition is not registered.", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }

  if (!transition->goal) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME, "No valid goal is set")
  }
  state_machine->current_state = transition->goal;
  if (publish_notification) {
    rcl_ret_t ret = rcl_lifecycle_com_interface_publish_notification(
      &state_machine->com_interface, transition->start, state_machine->current_state);
    if (ret != RCL_RET_OK) {
      RCL_SET_ERROR_MSG("Could not publish transition", rcl_get_default_allocator());
      return RCL_RET_ERROR;
    }
  }

  return RCL_RET_OK;
}


void
rcl_print_state_machine(const rcl_lifecycle_state_machine_t * state_machine)
{
  const rcl_lifecycle_transition_map_t * map = &state_machine->transition_map;
  for (size_t i = 0; i < map->states_size; ++i) {
    RCUTILS_LOG_INFO_NAMED(
      ROS_PACKAGE_NAME,
      "Primary State: %s(%u)\n# of valid transitions: %u",
      map->states[i].label, map->states[i].id,
      map->states[i].valid_transition_size
    )
    for (size_t j = 0; j < map->states[i].valid_transition_size; ++j) {
      RCUTILS_LOG_INFO_NAMED(
        ROS_PACKAGE_NAME,
        "\tNode %s: Key %d: Transition: %s",
        map->states[i].label,
        map->states[i].valid_transition_keys[j],
        map->states[i].valid_transitions[j].label)
    }
  }
}
#if __cplusplus
}
#endif  // extern "C"
