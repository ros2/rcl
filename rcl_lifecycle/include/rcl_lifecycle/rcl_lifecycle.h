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
#include "rcl_lifecycle/default_state_machine.h"
#include "rcl_lifecycle/visibility_control.h"

/// Return a rcl_lifecycle_state_t struct with members set to `NULL` or 0.
/**
 * Should be called to get a null rcl_lifecycle_state_t before passing to
 * rcl_lifecycle_state_init().
 */
RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_state_t
rcl_lifecycle_get_zero_initialized_state();

/// Initialize a rcl_lifecycle_state_init.
/*
 *
 * \param[inout] state pointer to the state struct to be initialized
 * \param[in] id identifier of the state
 * \param[in] label label of the state
 * \param[in] allocator a valid allocator used to initialized the lifecycle state
 * \return `RCL_RET_OK` if state was initialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_init(
  rcl_lifecycle_state_t * state,
  unsigned int id,
  const char * label,
  const rcl_allocator_t * allocator);

/// Finalize a rcl_lifecycle_state_t.
/*
 * \param[inout] state struct to be deinitialized
 * \param[in] allocator a valid allocator used to deinitialized the lifecycle state
 * \return `RCL_RET_OK` if the state was deinitialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_fini(
  rcl_lifecycle_state_t * state,
  const rcl_allocator_t * allocator);


/// Return a rcl_lifecycle_transition_t struct with members set to `NULL` or 0.
/**
 * Should be called to get a null rcl_lifecycle_transition_t before passing to
 * rcl_lifecycle_transition_init().
 */
RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_transition_t
rcl_lifecycle_get_zero_initialized_transition();

/// Initialize transition with existing states
/**
 * Note: the transition pointer will take ownership
 * of the start and goal state. When calling
 * rcl_lifecycle_transition_fini(), the two states
 * will be freed.

 * \param[in] transition to a preallocated, zero-initialized transition structure
 *    to be initialized.
 * \param[in] id identifier of the transition
 * \param[in] label label of the transition
 * \param[in] start the value where the transition is initialized
 * \param[in] goal the objetive og the transition
 * \param[in] allocator a valid allocator used to deinitialized the lifecycle state
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

/// Finalize a rcl_lifecycle_transition_t.
/*
 * \param[inout] transition struct to be deinitialized
 * \param[in] allocator a valid allocator used to deinitialized the transition
 * \return `RCL_RET_OK` if the state was deinitialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_transition_fini(
  rcl_lifecycle_transition_t * transition,
  const rcl_allocator_t * allocator);

/// Return a rcl_lifecycle_state_machine_t struct with members set to `NULL` or 0.
/**
 * Should be called to get a null rcl_lifecycle_state_machine_t before passing to
 * rcl_lifecycle_state_machine_init().
 */
RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_state_machine_t
rcl_lifecycle_get_zero_initialized_state_machine();

/// Initialize state_machine
/*
 * \param[inout] state_machine struct to be initialized
 * \param[in] node_handle valid node handle
 * \param[in] ts_pub_notify pointer to the a notify publisher, it used to publish the transitions
 * \param[in] ts_srv_change_state pointer to the service that allows to trigger state change
 * \param[in] ts_srv_get_state pointer to the service that allos to get the current state
 * \param[in] ts_srv_get_available_states
 * \param[in] ts_srv_get_available_transitions pointer to the service that allos to get the
 *    available transitions
 * \param[in] ts_srv_get_transition_graph
 * \param[in] default_states
 * \param[in] allocator a valid allocator used to deinitialized the transition
   a valid allocator used to deinitialized the state machine
 */
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
  const rosidl_service_type_support_t * ts_srv_get_transition_graph,
  bool default_states,
  const rcl_allocator_t * allocator);

/// Finalize a rcl_lifecycle_state_machine_t.
/*
 * \param[inout] state_machine struct to be deinitialized
 * \param[in] node_handle valid node handle
 * \param[in] allocator a valid allocator used to deinitialized the state machine
 * \return `RCL_RET_OK` if the state was deinitialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_machine_fini(
  rcl_lifecycle_state_machine_t * state_machine,
  rcl_node_t * node_handle,
  const rcl_allocator_t * allocator);

/// Check if a state machine is active using a rcl_lifecycle_state_machine_t.
/*
 * \param[in] state_machine pointer to the state machine struct
 * \return `RCL_RET_OK` if the state was deinitialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_state_machine_is_initialized(
  const rcl_lifecycle_state_machine_t * state_machine);

/// Get a state by id.
/*
 * \param[in] state pointer to the state struct
 * \param[in] id identifier to be find in the valid transitions
 * \return a pointer to the lifecycle transition if the id exists or otherwise it return NULL
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
const rcl_lifecycle_transition_t *
rcl_lifecycle_get_transition_by_id(
  const rcl_lifecycle_state_t * state,
  uint8_t id);

/// Get a state by id.
/*
 * \param[in] state pointer to the state struct
 * \param[in] label label to be find in the valid transitions
 * \return a pointer to the lifecycle transition if the label exists or otherwise it return NULL
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
const rcl_lifecycle_transition_t *
rcl_lifecycle_get_transition_by_label(
  const rcl_lifecycle_state_t * state,
  const char * label);

/// Trigger a state by id.
/*
 * \param[in] state_machine pointer to the state machine struct
 * \param[in] id identifier of the transition to be triggered
 * \param[in] publish_notification if the value is `true` a message will be published
 *    notifying the transition, otherwise no value will be published
 * \return `RCL_RET_OK` if the state was deinitialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_trigger_transition_by_id(
  rcl_lifecycle_state_machine_t * state_machine,
  uint8_t id,
  bool publish_notification);

/// Trigger a state by label.
/*
 * \param[in] state_machine pointer to the state machine struct
 * \param[in] label label of the transition to be triggered
 * \param[in] publish_notification if the value is `true` a message will be published
 *    notifying the transition, otherwise no value will be published
 * \return `RCL_RET_OK` if the state was deinitialized successfully, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_trigger_transition_by_label(
  rcl_lifecycle_state_machine_t * state_machine,
  const char * label,
  bool publish_notification);

/// Print the state machine data
/*
 * \param[in] state_machine the state machine structure to print
 */
RCL_LIFECYCLE_PUBLIC
void
rcl_print_state_machine(const rcl_lifecycle_state_machine_t * state_machine);

#ifdef __cplusplus
}
#endif  // extern "C"

#endif  // RCL_LIFECYCLE__RCL_LIFECYCLE_H_
