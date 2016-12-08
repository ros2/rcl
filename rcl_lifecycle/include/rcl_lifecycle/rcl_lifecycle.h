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

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include <rcl_lifecycle/visibility_control.h>
#include <rcl_lifecycle/data_types.h>

RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_state_machine_t
rcl_lifecycle_get_zero_initialized_state_machine();

RCL_LIFECYCLE_PUBLIC
rcl_ret_t
rcl_lifecycle_state_machine_init(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_change_state,
  bool default_states);

RCL_LIFECYCLE_PUBLIC
rcl_ret_t
rcl_lifecycle_state_machine_fini(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle);

RCL_LIFECYCLE_PUBLIC
rcl_ret_t
rcl_lifecycle_state_machine_is_initialized(
  const rcl_lifecycle_state_machine_t * state_machine);

RCL_LIFECYCLE_PUBLIC
const rcl_lifecycle_transition_t *
rcl_lifecycle_is_valid_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  unsigned int transition_id);

/// Execute a transition
/*
 * Important note for bool \param success here:
 * This is meant as feedback from the high level
 * callback associated with this transition.
 * In case success is true, the actual goal state
 * of the transition is set as a current state.
 * In case success is false, the error state
 * of the transition is set a a current state.
 */
RCL_LIFECYCLE_PUBLIC
rcl_ret_t
rcl_lifecycle_start_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  unsigned int transition_id, bool success, bool publish_notification);

RCL_LIFECYCLE_PUBLIC
void
rcl_print_state_machine(const rcl_lifecycle_state_machine_t * state_machine);

#if __cplusplus
}
#endif  // extern "C"

#endif  // RCL_LIFECYCLE__RCL_LIFECYCLE_H_
