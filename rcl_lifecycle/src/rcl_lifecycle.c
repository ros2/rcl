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

// get zero initialized state machine here
rcl_state_machine_t
rcl_get_zero_initialized_state_machine()
{
  rcl_state_machine_t state_machine;
  state_machine.transition_map.size = 0;
  state_machine.transition_map.primary_states = NULL;
  state_machine.transition_map.transition_arrays = NULL;
  state_machine.com_interface = rcl_get_zero_initialized_com_interface();
  return state_machine;
}

rcl_ret_t
rcl_state_machine_init(rcl_state_machine_t * state_machine, rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_change_state,
  bool default_states)
{
  if (rcl_com_interface_init(&state_machine->com_interface, node_handle,
    ts_pub_notify, ts_srv_get_state, ts_srv_change_state) != RCL_RET_OK)
  {
    fprintf(stderr, "%s:%u, Failed to initialize com interface\n",
      __FILE__, __LINE__);
    return RCL_RET_ERROR;
  }

  if (default_states) {
    rcl_init_default_state_machine(state_machine);
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_state_machine_fini(rcl_state_machine_t * state_machine,
  rcl_node_t * node_handle)
{
  rcl_ret_t fcn_ret = RCL_RET_OK;

  fcn_ret = rcl_com_interface_fini(&state_machine->com_interface, node_handle);

  rcl_transition_map_t * transition_map = &state_machine->transition_map;
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
rcl_state_machine_resolve_error(rcl_state_machine_t * state_machine,
  bool resolved)
{
  if (state_machine->current_state->index != rcl_state_errorprocessing.index) {
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
rcl_state_machine_is_initialized(const rcl_state_machine_t * state_machine)
{
  if (&state_machine->transition_map == NULL) {
    RCL_SET_ERROR_MSG("transition map is null");
    return RCL_RET_ERROR;
  }
  if (&state_machine->com_interface.srv_get_state.impl == NULL) {
    RCL_SET_ERROR_MSG("get_state service is null");
    return RCL_RET_ERROR;
  }
  if (&state_machine->com_interface.srv_change_state.impl == NULL) {
    RCL_SET_ERROR_MSG("change_state service is null");
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

const rcl_state_transition_t *
rcl_is_valid_transition_by_index(rcl_state_machine_t * state_machine,
  unsigned int transition_index)
{
  unsigned int current_index = state_machine->current_state->index;
  const rcl_transition_array_t * valid_transitions = rcl_get_transitions_by_index(
    &state_machine->transition_map, current_index);
  if (valid_transitions == NULL) {
    fprintf(stderr, "%s:%u, No transitions registered  for current state %s\n",
      __FILE__, __LINE__, state_machine->current_state->label);
    return NULL;
  }
  for (unsigned int i = 0; i < valid_transitions->size; ++i) {
    if (valid_transitions->transitions[i].transition_state.index == transition_index) {
      return &valid_transitions->transitions[i];
    }
  }
  fprintf(stderr, "%s:%u, No transition matching %u found for current state %s\n",
    __FILE__, __LINE__, transition_index, state_machine->current_state->label);
  return NULL;
}

const rcl_state_transition_t *
rcl_is_valid_transition_by_label(rcl_state_machine_t * state_machine,
  const char * transition_label)
{
  unsigned int current_index = state_machine->current_state->index;
  const rcl_transition_array_t * valid_transitions = rcl_get_transitions_by_index(
    &state_machine->transition_map, current_index);
  for (unsigned int i = 0; i < valid_transitions->size; ++i) {
    if (valid_transitions->transitions[i].transition_state.label == transition_label) {
      return &valid_transitions->transitions[i];
    }
  }
  return NULL;
}

const rcl_state_transition_t *
rcl_get_registered_transition_by_index(rcl_state_machine_t * state_machine,
  unsigned int transition_state_index)
{
  // extensive search approach
  // TODO(karsten1987) can be improved by having a separate array
  // for "registered transition"
  const rcl_transition_map_t * map = &state_machine->transition_map;
  for (unsigned int i = 0; i < map->size; ++i) {
    for (unsigned int j = 0; j < map->transition_arrays[i].size; ++j) {
      if (map->transition_arrays[i].transitions[j].transition_state.index ==
        transition_state_index)
      {
        return &map->transition_arrays[i].transitions[j];
      }
    }
  }
  return NULL;
}

const rcl_state_transition_t *
rcl_get_registered_transition_by_label(rcl_state_machine_t * state_machine,
  const char * transition_state_label)
{
  // extensive search approach
  // TODO(karsten1987) can be improved by having a separate array
  // for "registered transition"
  const rcl_transition_map_t * map = &state_machine->transition_map;
  for (unsigned int i = 0; i < map->size; ++i) {
    for (unsigned int j = 0; j < map->transition_arrays[i].size; ++j) {
      if (strcmp(map->transition_arrays[i].transitions[j].transition_state.label,
        transition_state_label) == 0)
      {
        return &map->transition_arrays[i].transitions[j];
      }
    }
  }
  return NULL;
}

void
rcl_register_callback(rcl_state_machine_t * state_machine,
  unsigned int state_index, unsigned int transition_index, bool (* fcn)(void))
{
  rcl_transition_array_t * transitions = rcl_get_transitions_by_index(
    &state_machine->transition_map, state_index);
  for (unsigned int i = 0; i < transitions->size; ++i) {
    if (transitions->transitions[i].transition_state.index == transition_index) {
      transitions->transitions[i].callback = fcn;
    }
  }
}

// maybe change directly the current state here,
// no need to that all the time inside high level language
bool
rcl_start_transition_by_index(rcl_state_machine_t * state_machine,
  unsigned int transition_index, bool publish_notification)
{
  const rcl_state_transition_t * transition =
    rcl_is_valid_transition_by_index(state_machine, transition_index);

  // If we have a faulty transition pointer
  if (transition == NULL) {
    fprintf(stderr, "%s:%d, Could not find registered transition %u\n",
      __FILE__, __LINE__, transition_index);
    return false;
  }

  // If we have a transition which is semantically not correct
  // we may have to set the current state to something intermediate
  // or simply ignore it
  if (transition->start != state_machine->current_state) {
    fprintf(stderr, "%s:%d, Wrong transition index %s. State machine is in primary state %s\n",
      __FILE__, __LINE__, transition->start->label, state_machine->current_state->label);
    return false;
  }
  // Apply a transition state
  state_machine->current_state = &transition->transition_state;

  if (publish_notification == true) {
    rcl_com_interface_publish_notification(&state_machine->com_interface,
      transition->start, &transition->transition_state);
  }
  return true;
}

bool
rcl_finish_transition_by_index(rcl_state_machine_t * state_machine,
  unsigned int transition_index, bool success, bool publish_notification)
{
  const rcl_state_transition_t * transition =
    rcl_get_registered_transition_by_index(state_machine, transition_index);

  // If we have a faulty transition pointer
  if (transition == NULL) {
    fprintf(stderr, "%s:%d, Could not find registered transition %u\n",
      __FILE__, __LINE__, transition_index);
    return false;
  }

  // If we have a transition which is semantically not correct
  // we may have to set the current state to something intermediate
  // or simply ignore it
  // TODO(Karsten1987): pointer comparison fails here
  if (transition->transition_state.index != state_machine->current_state->index) {
    fprintf(stderr, "%s:%d, Wrong transition state %s. State machine is in primary state %s\n",
      __FILE__, __LINE__, transition->transition_state.label, state_machine->current_state->label);
    return false;
  }

  // high level transition(callback) was executed correctly
  if (success == true) {
    state_machine->current_state = transition->goal;

    if (publish_notification == true) {
      rcl_com_interface_publish_notification(&state_machine->com_interface,
        &transition->transition_state, transition->goal);
    }
    return true;
  }

  // TODO(karsten1987): Clarify where to handle the on_error callback
  state_machine->current_state = transition->error;

  if (publish_notification == true) {
    rcl_com_interface_publish_notification(&state_machine->com_interface,
      &transition->transition_state, transition->error);
  }

  return true;
}

#if __cplusplus
}
#endif  // extern "C"
