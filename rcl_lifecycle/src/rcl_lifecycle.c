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

#include "rcl_lifecycle/rcl_lifecycle.h"
#include "rcl_lifecycle/transition_map.h"

#include "com_interface.hxx"
#include "default_state_machine.hxx"
#include "states.hxx"

// get zero initialized state machine here
rcl_lifecycle_state_machine_t
rcl_lifecycle_get_zero_initialized_state_machine()
{
  rcl_lifecycle_state_machine_t state_machine;
  state_machine.transition_map.size = 0;
  state_machine.transition_map.primary_states = NULL;
  state_machine.transition_map.transition_arrays = NULL;
  state_machine.com_interface = rcl_lifecycle_get_zero_initialized_com_interface();
  return state_machine;
}

rcl_ret_t
rcl_lifecycle_state_machine_init(rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_change_state,
  bool default_states)
{
  if (rcl_lifecycle_com_interface_init(&state_machine->com_interface, node_handle,
    ts_pub_notify, ts_srv_get_state, ts_srv_change_state) != RCL_RET_OK)
  {
    return RCL_RET_ERROR;
  }

  if (default_states) {
    rcl_lifecycle_init_default_state_machine(state_machine);
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_state_machine_fini(rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle)
{
  rcl_ret_t fcn_ret = RCL_RET_OK;

  fcn_ret = rcl_lifecycle_com_interface_fini(&state_machine->com_interface, node_handle);

  rcl_lifecycle_transition_map_t * transition_map = &state_machine->transition_map;
  // free the primary states array
  free(transition_map->primary_states);
  transition_map->primary_states = NULL;
  for (unsigned int i = 0; i < transition_map->size; ++i) {
    // free each transition array associated to a primary state
    free(transition_map->transition_arrays[i].transitions);
    transition_map->transition_arrays[i].transitions = NULL;
  }
  // free the top level transition array
  free(transition_map->transition_arrays);
  transition_map->transition_arrays = NULL;

  return fcn_ret;
}

rcl_ret_t
rcl_lifecycle_state_machine_resolve_error(rcl_lifecycle_state_machine_t * state_machine,
  bool resolved)
{
  if (state_machine->current_state->id != rcl_state_errorprocessing.id) {
    RCL_SET_ERROR_MSG("Can't resolve error. State machine is not in errorneous state.");
    return RCL_RET_ERROR;
  }
  if (resolved) {
    state_machine->current_state = &rcl_state_unconfigured;
  } else {
    state_machine->current_state = &rcl_state_finalized;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_state_machine_is_initialized(const rcl_lifecycle_state_machine_t * state_machine)
{
  if (state_machine->transition_map.size == 0) {
    RCL_SET_ERROR_MSG("transition map is empty");
    return RCL_RET_ERROR;
  }
  if (state_machine->com_interface.srv_get_state.impl == NULL) {
    RCL_SET_ERROR_MSG("get_state service is null");
    return RCL_RET_ERROR;
  }
  if (state_machine->com_interface.srv_change_state.impl == NULL) {
    RCL_SET_ERROR_MSG("change_state service is null");
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

const rcl_lifecycle_state_transition_t *
rcl_lifecycle_is_valid_transition(rcl_lifecycle_state_machine_t * state_machine,
  unsigned int transition_id)
{
  unsigned int current_id = state_machine->current_state->id;
  const rcl_lifecycle_transition_array_t * valid_transitions = rcl_lifecycle_get_transitions(
    &state_machine->transition_map, current_id);
  if (valid_transitions == NULL) {
    fprintf(stderr, "%s:%u, No transitions registered  for current state %s\n",
      __FILE__, __LINE__, state_machine->current_state->label);
    return NULL;
  }
  for (unsigned int i = 0; i < valid_transitions->size; ++i) {
    if (valid_transitions->transitions[i].transition_state.id == transition_id) {
      return &valid_transitions->transitions[i];
    }
  }
  fprintf(stderr, "%s:%u, No transition matching %u found for current state %s\n",
    __FILE__, __LINE__, transition_id, state_machine->current_state->label);
  return NULL;
}

const rcl_lifecycle_state_transition_t *
rcl_lifecycle_get_registered_transition(rcl_lifecycle_state_machine_t * state_machine,
  unsigned int transition_state_id)
{
  // extensive search approach
  // TODO(karsten1987) can be improved by having a separate array
  // for "registered transition"
  const rcl_lifecycle_transition_map_t * map = &state_machine->transition_map;
  for (unsigned int i = 0; i < map->size; ++i) {
    for (unsigned int j = 0; j < map->transition_arrays[i].size; ++j) {
      if (map->transition_arrays[i].transitions[j].transition_state.id ==
        transition_state_id)
      {
        return &map->transition_arrays[i].transitions[j];
      }
    }
  }
  return NULL;
}

rcl_ret_t
rcl_lifecycle_start_transition(rcl_lifecycle_state_machine_t * state_machine,
  unsigned int transition_id, bool publish_notification)
{
  const rcl_lifecycle_state_transition_t * transition =
    rcl_lifecycle_is_valid_transition(state_machine, transition_id);

  // If we have a faulty transition pointer
  if (transition == NULL) {
    RCL_SET_ERROR_MSG("Transition is not registered.");
    return RCL_RET_ERROR;
  }

  // If we have a transition which is semantically not correct
  // we may have to set the current state to something intermediate
  // or simply ignore it
  if (transition->start != state_machine->current_state) {
    RCL_SET_ERROR_MSG("Transition not allowed for current state.");
    fprintf(stderr, "%s:%d, Wrong transition id %s. State machine is in primary state %s\n",
      __FILE__, __LINE__, transition->start->label, state_machine->current_state->label);
    return RCL_RET_ERROR;
  }
  // Apply a transition state
  state_machine->current_state = &transition->transition_state;

  if (publish_notification == true) {
    rcl_lifecycle_com_interface_publish_notification(&state_machine->com_interface,
      transition->start, &transition->transition_state);
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_finish_transition(rcl_lifecycle_state_machine_t * state_machine,
  unsigned int transition_id, bool success, bool publish_notification)
{
  const rcl_lifecycle_state_transition_t * transition =
    rcl_lifecycle_get_registered_transition(state_machine, transition_id);

  // If we have a faulty transition pointer
  if (transition == NULL) {
    RCL_SET_ERROR_MSG("Transition is not registered.");
    return RCL_RET_ERROR;
  }

  // TODO(Karsten1987): pointer comparison fails here
  if (transition->transition_state.id != state_machine->current_state->id) {
    RCL_SET_ERROR_MSG("Transition not allowed for current state.");
    fprintf(stderr, "%s:%d, Wrong transition state %s. State machine is in primary state %s\n",
      __FILE__, __LINE__, transition->transition_state.label, state_machine->current_state->label);
    return RCL_RET_ERROR;
  }

  // high level transition(callback) was executed correctly
  if (success == true) {
    state_machine->current_state = transition->goal;

    if (publish_notification == true) {
      rcl_lifecycle_com_interface_publish_notification(&state_machine->com_interface,
        &transition->transition_state, transition->goal);
    }
    return RCL_RET_OK;
  }

  // State machine stays in error state.
  // The only way to resolve the error is by explicitly
  // calling the high level on_error callback and then
  // call 'rcl_lifecycle_state_machine_resolve_error' passing the
  // result of the high level callback
  state_machine->current_state = transition->error;

  if (publish_notification == true) {
    rcl_lifecycle_com_interface_publish_notification(&state_machine->com_interface,
      &transition->transition_state, transition->error);
  }

  return RCL_RET_OK;
}

void
rcl_print_state_machine(const rcl_lifecycle_state_machine_t * state_machine)
{
  const rcl_lifecycle_transition_map_t * map = &state_machine->transition_map;
  for (size_t i=0; i<map->size; ++i)
  {
    printf("Primary State: %s(%u)\n", map->primary_states[i].label, map->primary_states[i].id);
    printf("Attached transitions: %u\n", map->transition_arrays[i].size);
    for (size_t j=0; j<map->transition_arrays[i].size; ++j)
    {
      printf("\tTransition State: %s(%u)\n", map->transition_arrays[i].transitions[j].transition_state.label,
          map->transition_arrays[i].transitions[j].transition_state.id);
    }
  }
}
#if __cplusplus
}
#endif  // extern "C"
