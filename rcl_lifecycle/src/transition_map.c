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
  ++transition_map->size;
  if (transition_map->size == 1) {
    transition_map->states = malloc(
      transition_map->size * sizeof(rcl_lifecycle_state_t));
    transition_map->transition_arrays = malloc(
      transition_map->size * sizeof(rcl_lifecycle_transition_array_t));
  } else {
    transition_map->states = realloc(
      transition_map->states,
      transition_map->size * sizeof(rcl_lifecycle_state_t));
    transition_map->transition_arrays = realloc(
      transition_map->transition_arrays,
      transition_map->size * sizeof(rcl_lifecycle_transition_array_t));
  }
  transition_map->states[transition_map->size - 1] = state;
  // zero initialize the associated transition array
  transition_map->transition_arrays[transition_map->size - 1].transitions = NULL;
  transition_map->transition_arrays[transition_map->size - 1].size = 0;
}

void
rcl_lifecycle_register_transition(
  rcl_lifecycle_transition_map_t * transition_map,
  rcl_lifecycle_transition_t transition,
  const rcl_lifecycle_state_t * start,
  const rcl_lifecycle_state_t * goal,
  const rcl_lifecycle_state_t * failure,
  const rcl_lifecycle_state_t * error)
{
  transition.start = start;
  transition.goal = goal;
  transition.failure = failure;
  transition.error = error;

  // TODO(karsten1987): check whether we can improve for not checking duplicates
  rcl_lifecycle_transition_array_t * transition_array = rcl_lifecycle_get_transitions(
    transition_map, transition.start->id);
  if (!transition_array) {
    fprintf(stderr, "%s:%u, Unable to find transition array registered for start id %u",
      __FILE__, __LINE__, transition.start->id);
    return;
  }

  // we add a new transition, so increase the size
  ++transition_array->size;
  if (transition_array->size == 1) {
    transition_array->transitions = malloc(
      transition_array->size * sizeof(rcl_lifecycle_transition_t));
  } else {
    transition_array->transitions = realloc(
      transition_array->transitions,
      transition_array->size * sizeof(rcl_lifecycle_transition_t));
  }
  // finally set the new transition to the end of the array
  transition_array->transitions[transition_array->size - 1] = transition;
}

const rcl_lifecycle_state_t *
rcl_lifecycle_get_state(
  rcl_lifecycle_transition_map_t * transition_map,
  unsigned int state_id)
{
  for (unsigned int i = 0; i < transition_map->size; ++i) {
    if (transition_map->states[i].id == state_id) {
      return &transition_map->states[i];
    }
  }
  return NULL;
}

rcl_lifecycle_transition_array_t *
rcl_lifecycle_get_transitions(
  rcl_lifecycle_transition_map_t * transition_map,
  unsigned int transition_id)
{
  for (unsigned int i = 0; i < transition_map->size; ++i) {
    if (transition_map->states[i].id == transition_id) {
      return &transition_map->transition_arrays[i];
    }
  }
  return NULL;
}

#if __cplusplus
}
#endif
