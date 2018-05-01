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

#ifndef RCL_LIFECYCLE__RCL_LIFECYCLE_H_
#define RCL_LIFECYCLE__RCL_LIFECYCLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "rcl_lifecycle/data_types.h"
#include "rcl_lifecycle/visibility_control.h"

RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_state_t
rcl_lifecycle_get_zero_initialized_state();

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_init(
  rcl_lifecycle_state_t * state,
  unsigned int id,
  const char * label,
  const rcl_allocator_t * allocator);

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_fini(
  rcl_lifecycle_state_t * state,
  const rcl_allocator_t * allocator);

RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_transition_t
rcl_lifecycle_get_zero_initialized_transition();

/// Initialize transition with existing states
/**
 * Note: the transition pointer will take ownership
 * of the start and goal state. When calling
 * rcl_lifecycle_transition_fini(), the two states
 * will be freed.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_transition_init(
  rcl_lifecycle_transition_t * transition,
  unsigned int id,
  const char * label,
  rcl_lifecycle_state_t * start,
  rcl_lifecycle_state_t * goal,
  const rcl_allocator_t * allocator);

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_transition_fini(
  rcl_lifecycle_transition_t * transition,
  const rcl_allocator_t * allocator);

RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_state_machine_t
rcl_lifecycle_get_zero_initialized_state_machine();

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
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
  const rcl_allocator_t * allocator);

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_machine_fini(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle,
  const rcl_allocator_t * allocator);

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_machine_is_initialized(
  const rcl_lifecycle_state_machine_t * state_machine);

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
const rcl_lifecycle_transition_t *
rcl_lifecycle_is_valid_callback_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_lifecycle_transition_key_t key);

/// Execute a transition
/*
 * Important note for \param key here:
 * This is meant as feedback from the high level
 * callback associated with this transition.
 * The key is the index for the valid transitions
 * associated with the current state.
 * This key may either be a valid external stimuli
 * such as "configure" or direct return codes from
 * callbacks such as RCL_LIFECYCLE_RET_OK et. al.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_trigger_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_lifecycle_transition_key_t key, bool publish_notification);

RCL_LIFECYCLE_PUBLIC
void
rcl_print_state_machine(const rcl_lifecycle_state_machine_t * state_machine);

#ifdef __cplusplus
}
#endif  // extern "C"

#endif  // RCL_LIFECYCLE__RCL_LIFECYCLE_H_
