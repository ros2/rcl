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
rcl_lifecycle_register_primary_state(rcl_lifecycle_transition_map_t * m,
  rcl_lifecycle_state_t primary_state)
{
  if (rcl_lifecycle_get_primary_state(m, primary_state.id) != NULL) {
    // primary state is already registered
    fprintf(stderr, "%s:%u, Primary state %u is already registered\n",
      __FILE__, __LINE__, primary_state.id);
    return;
  }

  // add new primary state memory
  ++m->size;
  if (m->size == 1) {
    m->primary_states = malloc(m->size * sizeof(rcl_lifecycle_state_t));
    m->transition_arrays = malloc(m->size * sizeof(rcl_lifecycle_transition_array_t));
  } else {
    m->primary_states = realloc(
      m->primary_states, m->size * sizeof(rcl_lifecycle_state_t));
    m->transition_arrays = realloc(
      m->transition_arrays, m->size * sizeof(rcl_lifecycle_transition_array_t));
  }
  m->primary_states[m->size - 1] = primary_state;
  m->transition_arrays[m->size - 1].transitions = NULL;
  m->transition_arrays[m->size - 1].size = 0;  // initialize array to size 0
}

void
rcl_lifecycle_register_transition_by_state(rcl_lifecycle_transition_map_t * m,
  const rcl_lifecycle_state_t * start, const rcl_lifecycle_state_t * goal,
  rcl_lifecycle_state_transition_t transition)
{
  transition.start = start;
  transition.goal = goal;

  // TODO(karsten1987): check whether we can improve that
  rcl_lifecycle_transition_array_t * transition_array = rcl_lifecycle_get_transitions(
    m, transition.start->id);
  if (!transition_array) {
    fprintf(stderr, "%s:%u, Unable to find transition array registered for start id %u",
      __FILE__, __LINE__, transition.start->id);
    return;
  }

  // we add a new transition, so increase the size
  ++transition_array->size;
  if (transition_array->size == 1) {
    transition_array->transitions = malloc(
      transition_array->size * sizeof(rcl_lifecycle_state_transition_t));
  } else {
    transition_array->transitions = realloc(
      transition_array->transitions,
      transition_array->size * sizeof(rcl_lifecycle_state_transition_t));
  }
  // finally set the new transition to the end of the array
  transition_array->transitions[transition_array->size - 1] = transition;
}

void
rcl_lifecycle_register_transition(rcl_lifecycle_transition_map_t * m,
  unsigned int start_id, unsigned int goal_id, rcl_lifecycle_state_transition_t transition)
{
  // the idea here is to add this transition based on the
  // start label and classify them.
  const rcl_lifecycle_state_t * start_state = rcl_lifecycle_get_primary_state(m, start_id);
  if (start_state == NULL) {
    // return false here?
    return;
  }
  const rcl_lifecycle_state_t * goal_state = rcl_lifecycle_get_primary_state(m, goal_id);
  if (goal_state == NULL) {
    // return false here?
    return;
  }
  rcl_lifecycle_register_transition_by_state(m, start_state, goal_state, transition);
}

rcl_lifecycle_state_t *
rcl_lifecycle_get_primary_state(rcl_lifecycle_transition_map_t * m,
  unsigned int id)
{
  for (unsigned int i = 0; i < m->size; ++i) {
    if (m->primary_states[i].id == id) {
      return &m->primary_states[i];
    }
  }
  return NULL;
}

rcl_lifecycle_transition_array_t *
rcl_lifecycle_get_transitions(rcl_lifecycle_transition_map_t * m,
  unsigned int id)
{
  for (unsigned int i = 0; i < m->size; ++i) {
    if (m->primary_states[i].id == id) {
      return &m->transition_arrays[i];
    }
  }
  return NULL;
}

#if __cplusplus
}
#endif
