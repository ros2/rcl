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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_lifecycle/rcl_lifecycle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/rcl.h"
#include "rcl/error_handling.h"

#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"

#include "rcl_lifecycle/default_state_machine.h"
#include "rcl_lifecycle/transition_map.h"

#include "./com_interface.h"

rcl_lifecycle_state_t
rcl_lifecycle_get_zero_initialized_state()
{
  rcl_lifecycle_state_t state;
  state.id = 0;
  state.label = NULL;
  return state;
}

rcl_ret_t
rcl_lifecycle_state_init(
  rcl_lifecycle_state_t * state,
  unsigned int id,
  const char * label,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't initialize state, no allocator given\n");
    return RCL_RET_ERROR;
  }
  if (!state) {
    RCL_SET_ERROR_MSG("state pointer is null\n");
    return RCL_RET_ERROR;
  }

  state->id = id;
  state->label = rcutils_strndup(label, strlen(label), *allocator);
  if (!state->label) {
    RCL_SET_ERROR_MSG("failed to duplicate label for rcl_lifecycle_state_t\n");
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_state_fini(
  rcl_lifecycle_state_t * state,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't free state, no allocator given\n");
    return RCL_RET_ERROR;
  }
  // it is already NULL
  if (!state) {
    return RCL_RET_OK;
  }

  if (state->label) {
    allocator->deallocate((char *)state->label, allocator->state);
    state->label = NULL;
  }

  return RCL_RET_OK;
}

rcl_lifecycle_transition_t
rcl_lifecycle_get_zero_initialized_transition()
{
  rcl_lifecycle_transition_t transition;
  transition.id = 0;
  transition.label = NULL;
  transition.start = NULL;
  transition.goal = NULL;
  return transition;
}

rcl_ret_t
rcl_lifecycle_transition_init(
  rcl_lifecycle_transition_t * transition,
  unsigned int id,
  const char * label,
  rcl_lifecycle_state_t * start,
  rcl_lifecycle_state_t * goal,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't initialize transition, no allocator given\n");
    return RCL_RET_ERROR;
  }

  if (!transition) {
    RCL_SET_ERROR_MSG("transition pointer is null\n");
    return RCL_RET_OK;
  }

  transition->start = start;
  transition->goal = goal;

  transition->id = id;
  transition->label = rcutils_strndup(label, strlen(label), *allocator);
  if (!transition->label) {
    RCL_SET_ERROR_MSG("failed to duplicate label for rcl_lifecycle_transition_t\n");
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_transition_fini(
  rcl_lifecycle_transition_t * transition,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't initialize transition, no allocator given\n");
    return RCL_RET_ERROR;
  }
  // it is already NULL
  if (!transition) {
    return RCL_RET_OK;
  }

  rcl_ret_t ret = RCL_RET_OK;

  if (rcl_lifecycle_state_fini(transition->start, allocator) != RCL_RET_OK) {
    ret = RCL_RET_ERROR;
  }
  allocator->deallocate(transition->start, allocator->state);
  transition->start = NULL;

  if (rcl_lifecycle_state_fini(transition->goal, allocator) != RCL_RET_OK) {
    ret = RCL_RET_ERROR;
  }
  allocator->deallocate(transition->goal, allocator->state);
  transition->goal = NULL;

  allocator->deallocate((char *)transition->label, allocator->state);
  transition->label = NULL;

  return ret;
}

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
  const rosidl_service_type_support_t * ts_srv_get_transition_graph,
  bool default_states,
  const rcl_allocator_t * allocator)
{
  if (!allocator) {
    RCL_SET_ERROR_MSG("can't initialize state machine, no allocator given\n");
    return RCL_RET_ERROR;
  }

  rcl_ret_t ret = rcl_lifecycle_com_interface_init(
    &state_machine->com_interface, node_handle,
    ts_pub_notify,
    ts_srv_change_state, ts_srv_get_state,
    ts_srv_get_available_states, ts_srv_get_available_transitions, ts_srv_get_transition_graph);
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
    RCL_SET_ERROR_MSG("can't free state machine, no allocator given\n");
    return RCL_RET_ERROR;
  }

  rcl_ret_t fcn_ret = RCL_RET_OK;

  if (rcl_lifecycle_com_interface_fini(&state_machine->com_interface, node_handle) != RCL_RET_OK) {
    rcl_error_string_t error_string = rcl_get_error_string();
    rcutils_reset_error();
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "could not free lifecycle com interface. Leaking memory!\n%s", error_string.str);
    fcn_ret = RCL_RET_ERROR;
  }

  if (rcl_lifecycle_transition_map_fini(
      &state_machine->transition_map, allocator) != RCL_RET_OK)
  {
    rcl_error_string_t error_string = rcl_get_error_string();
    rcutils_reset_error();
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "could not free lifecycle transition map. Leaking memory!\n%s", error_string.str);
    fcn_ret = RCL_RET_ERROR;
  }

  return fcn_ret;
}

rcl_ret_t
rcl_lifecycle_state_machine_is_initialized(const rcl_lifecycle_state_machine_t * state_machine)
{
  if (!state_machine->com_interface.srv_get_state.impl) {
    RCL_SET_ERROR_MSG("get_state service is null");
    return RCL_RET_ERROR;
  }
  if (!state_machine->com_interface.srv_change_state.impl) {
    RCL_SET_ERROR_MSG("change_state service is null");
    return RCL_RET_ERROR;
  }
  if (rcl_lifecycle_transition_map_is_initialized(&state_machine->transition_map) != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("transition map is null");
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

const rcl_lifecycle_transition_t *
rcl_lifecycle_get_transition_by_id(
  const rcl_lifecycle_state_t * state,
  uint8_t id)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(state, "state pointer is null", return NULL);

  for (unsigned int i = 0; i < state->valid_transition_size; ++i) {
    if (state->valid_transitions[i].id == id) {
      return &state->valid_transitions[i];
    }
  }

  RCUTILS_LOG_WARN_NAMED(
    ROS_PACKAGE_NAME,
    "No transition matching %d found for current state %s",
    id, state->label);

  return NULL;
}

const rcl_lifecycle_transition_t *
rcl_lifecycle_get_transition_by_label(
  const rcl_lifecycle_state_t * state,
  const char * label)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(state, "state pointer is null", return NULL);

  for (unsigned int i = 0; i < state->valid_transition_size; ++i) {
    if (strcmp(state->valid_transitions[i].label, label) == 0) {
      return &state->valid_transitions[i];
    }
  }

  RCUTILS_LOG_WARN_NAMED(
    ROS_PACKAGE_NAME,
    "No transition matching %s found for current state %s",
    label, state->label);

  return NULL;
}

rcl_ret_t
_trigger_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  const rcl_lifecycle_transition_t * transition,
  bool publish_notification)
{
  // If we have a faulty transition pointer
  if (!transition) {
    RCL_SET_ERROR_MSG("Transition is not registered.");
    return RCL_RET_ERROR;
  }

  if (!transition->goal) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME, "No valid goal is set");
    return RCL_RET_ERROR;
  }
  state_machine->current_state = transition->goal;

  if (publish_notification) {
    rcl_ret_t ret = rcl_lifecycle_com_interface_publish_notification(
      &state_machine->com_interface, transition->start, state_machine->current_state);
    if (ret != RCL_RET_OK) {
      rcl_error_string_t error_string = rcl_get_error_string();
      rcutils_reset_error();
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Could not publish transition: %s", error_string.str);
      return RCL_RET_ERROR;
    }
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_lifecycle_trigger_transition_by_id(
  rcl_lifecycle_state_machine_t * state_machine,
  uint8_t id,
  bool publish_notification)
{
  if (!state_machine) {
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "state machine pointer is null");
    return RCL_RET_ERROR;
  }

  const rcl_lifecycle_transition_t * transition =
    rcl_lifecycle_get_transition_by_id(state_machine->current_state, id);

  return _trigger_transition(state_machine, transition, publish_notification);
}

rcl_ret_t
rcl_lifecycle_trigger_transition_by_label(
  rcl_lifecycle_state_machine_t * state_machine,
  const char * label,
  bool publish_notification)
{
  if (!state_machine) {
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "state machine pointer is null");
    return RCL_RET_ERROR;
  }

  const rcl_lifecycle_transition_t * transition =
    rcl_lifecycle_get_transition_by_label(state_machine->current_state, label);

  return _trigger_transition(state_machine, transition, publish_notification);
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
    );
    for (size_t j = 0; j < map->states[i].valid_transition_size; ++j) {
      RCUTILS_LOG_INFO_NAMED(
        ROS_PACKAGE_NAME,
        "\tNode %s: Transition: %s",
        map->states[i].label,
        map->states[i].valid_transitions[j].label);
    }
  }
}
#ifdef __cplusplus
}
#endif  // extern "C"
