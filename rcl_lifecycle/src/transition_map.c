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

#include "rcl_lifecycle/transition_map.h"

void
rcl_lifecycle_register_state(
  rcl_lifecycle_transition_map_t * transition_map,
  rcl_lifecycle_state_t state)
{
  if (rcl_lifecycle_get_state(transition_map, state.id) != NULL) {
    // primary state is already registered
    fprintf(stderr, "%s:%u, State %u is already registered\n",
      __FILE__, __LINE__, state.id);
    return;
  }

  // add new primary state memory
  ++transition_map->states_size;
  transition_map->states = realloc(
    transition_map->states,
    transition_map->states_size * sizeof(rcl_lifecycle_state_t));

  transition_map->states[transition_map->states_size - 1] = state;
}

void
rcl_lifecycle_register_transition(
  rcl_lifecycle_transition_map_t * transition_map,
  rcl_lifecycle_transition_t transition,
  rcl_lifecycle_ret_t key)
{
  // we add a new transition, so increase the size
  ++transition_map->transitions_size;
  transition_map->transitions = realloc(
    transition_map->transitions,
    transition_map->transitions_size * sizeof(rcl_lifecycle_transition_t));
  // finally set the new transition to the end of the array
  transition_map->transitions[transition_map->transitions_size - 1] = transition;

  // connect transition to state key
  rcl_lifecycle_state_t * state = rcl_lifecycle_get_state(transition_map, transition.start->id);

  ++state->valid_transition_size;
  state->valid_transitions = realloc(
    state->valid_transitions,
    state->valid_transition_size * sizeof(rcl_lifecycle_transition_t));
  state->valid_transition_keys = realloc(
    state->valid_transition_keys,
    state->valid_transition_size * sizeof(rcl_lifecycle_ret_t));

  // assign key
  state->valid_transition_keys[state->valid_transition_size - 1] = key;
  state->valid_transitions[state->valid_transition_size - 1] = transition;
}

rcl_lifecycle_state_t *
rcl_lifecycle_get_state(
  rcl_lifecycle_transition_map_t * transition_map,
  unsigned int state_id)
{
  for (unsigned int i = 0; i < transition_map->states_size; ++i) {
    if (transition_map->states[i].id == state_id) {
      return &transition_map->states[i];
    }
  }
  return NULL;
}

rcl_lifecycle_transition_t *
rcl_lifecycle_get_transitions(
  rcl_lifecycle_transition_map_t * transition_map,
  unsigned int transition_id)
{
  for (unsigned int i = 0; i < transition_map->transitions_size; ++i) {
    if (transition_map->transitions[i].id == transition_id) {
      return &transition_map->transitions[i];
    }
  }
  return NULL;
}

#if __cplusplus
}
#endif
