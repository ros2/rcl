// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RCL_ACTION__GRAPH_H_
#define RCL_ACTION__GRAPH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/graph.h"
#include "rcl/node.h"

#include "rcl_action/visibility_control.h"

/// Get a list of action names and types for action clients associated with a node.
/**
 * The `node` parameter must point to a valid node.
 *
 * The `action_names_and_types` parameter must be allocated and zero initialized.
 * This function allocates memory for the returned list of names and types and so it is the
 * callers responsibility to pass `action_names_and_types` to rcl_names_and_types_fini()
 * when it is no longer needed.
 * Failing to do so will result in leaked memory.
 *
 * The returned names are not automatically remapped by this function.
 * Attempting to create action clients or action servers with names returned by this function may
 * not result in the desired action name depending on the remap rules in use.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] allocator allocator for allocating space for strings
 * \param[in] node_name the node name of the actions to return
 * \param[in] node_namespace the node namespace of the actions to return
 * \param[out] action_names_and_types list of action names and their types
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_get_client_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * action_names_and_types);

/// Get a list of action names and types for action servers associated with a node.
/**
 * This function returns a list of action names and types for action servers associated with
 * the provided node name.
 *
 * The `node` parameter must point to a valid node.
 *
 * The `action_names_and_types` parameter must be allocated and zero initialized.
 * This function allocates memory for the returned list of names and types and so it is the
 * callers responsibility to pass `action_names_and_types` to rcl_names_and_types_fini()
 * when it is no longer needed.
 * Failing to do so will result in leaked memory.
 *
 * The returned names are not automatically remapped by this function.
 * Attempting to create action clients or action servers with names returned by this function may
 * not result in the desired action name depending on the remap rules in use.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] allocator allocator for allocating space for strings
 * \param[in] node_name the node name of the actions to return
 * \param[in] node_namespace the node namespace of the actions to return
 * \param[out] action_names_and_types list of action names and their types
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_get_server_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * action_names_and_types);

/// Return a list of action names and their types.
/**
 * This function returns a list of action names and types in the ROS graph.
 *
 * The `node` parameter must point to a valid node.
 *
 * The `action_names_and_types` parameter must be allocated and zero initialized.
 * This function allocates memory for the returned list of names and types and so it is the
 * callers responsibility to pass `action_names_and_types` to rcl_names_and_types_fini()
 * when it is no longer needed.
 * Failing to do so will result in leaked memory.
 *
 * The returned names are not automatically remapped by this function.
 * Attempting to create action clients or action servers with names returned by this function may
 * not result in the desired action name depending on the remap rules in use.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] allocator allocator for allocating space for strings
 * \param[out] action_names_and_types list of action names and types
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_get_names_and_types(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  rcl_names_and_types_t * action_names_and_types);

/// Return the number of action clients for a given action name.
/**
 * The `node` parameter must point to a valid node.
 *
 * The `action_name` parameter must not be `NULL`.
 *
 * The `count` parameter must not be `NULL`.
 * The `count` parameter is the output for this function and will be set.
 *
 * This function counts the number of action client instances for the given
 * action name across all nodes known in the ROS graph.
 * Each action client is counted individually, so a node with two action
 * clients on the same action contributes a count of 2.
 *
 * The `action_name` parameter should be a fully qualified action name, since
 * it is not expanded or automatically remapped by this function.
 * If there is a client created with action name `foo` and remap rule `foo:=bar`
 * then calling this with `action_name` set to `bar` will return a count of 1,
 * and with `action_name` set to `foo` will return a count of 0.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] action_name the name of the action in question
 * \param[out] count number of action clients for the given action
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_count_clients(
  const rcl_node_t * node,
  const char * action_name,
  size_t * count);

/// Return the number of action servers for a given action name.
/**
 * The `node` parameter must point to a valid node.
 *
 * The `action_name` parameter must not be `NULL`.
 *
 * The `count` parameter must not be `NULL`.
 * The `count` parameter is the output for this function and will be set.
 *
 * This function counts the number of action server instances for the given
 * action name across all nodes known in the ROS graph.
 * Each action server is counted individually, so a node with two action
 * servers on the same action contributes a count of 2.
 *
 * The `action_name` parameter should be a fully qualified action name, since
 * it is not expanded or automatically remapped by this function.
 * If there is a server created with action name `foo` and remap rule `foo:=bar`
 * then calling this with `action_name` set to `bar` will return a count of 1,
 * and with `action_name` set to `foo` will return a count of 0.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] action_name the name of the action in question
 * \param[out] count number of action servers for the given action
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_count_servers(
  const rcl_node_t * node,
  const char * action_name,
  size_t * count);

/// Endpoint information of an action client or an action server.
/**
 * An action is built on top of three services and two topics:
 *
 * - the goal service (`<action_name>/_action/send_goal`)
 * - the cancel service (`<action_name>/_action/cancel_goal`)
 * - the result service (`<action_name>/_action/get_result`)
 * - the feedback topic (`<action_name>/_action/feedback`)
 * - the status topic (`<action_name>/_action/status`)
 *
 * This structure aggregates the endpoint information of all the underlying
 * entities of one action client or one action server.
 * The goal service endpoint is used as the canonical identity of an action
 * client or an action server, so `goal_service_info` is always populated.
 * The remaining endpoint information is correlated to the goal service
 * endpoint by the node name and node namespace.
 * If an underlying entity has not been discovered (yet), or if the
 * correlation is not possible, the corresponding member is left zero
 * initialized (i.e. its `node_name` is `NULL`).
 *
 * The `service_type` and `topic_type` fields hold the types of the
 * underlying entities (e.g. `test_msgs/action/Fibonacci_SendGoal`), not the
 * action type.
 * The action type can be derived by trimming the `_SendGoal` suffix from the
 * goal service type.
 */
typedef struct rcl_action_endpoint_info_s
{
  /// Endpoint information of the goal service of the action client or server.
  rcl_service_endpoint_info_t goal_service_info;
  /// Endpoint information of the cancel service of the action client or server.
  rcl_service_endpoint_info_t cancel_service_info;
  /// Endpoint information of the result service of the action client or server.
  rcl_service_endpoint_info_t result_service_info;
  /// Endpoint information of the feedback topic of the action client or server.
  rcl_topic_endpoint_info_t feedback_topic_info;
  /// Endpoint information of the status topic of the action client or server.
  rcl_topic_endpoint_info_t status_topic_info;
} rcl_action_endpoint_info_t;

/// Array of rcl_action_endpoint_info_t.
typedef struct rcl_action_endpoint_info_array_s
{
  /// Size of the array.
  size_t size;
  /// Contiguous storage of the array.
  rcl_action_endpoint_info_t * info_array;
} rcl_action_endpoint_info_array_t;

/// Return a rcl_action_endpoint_info_t with members set to `NULL` or zero.
RCL_ACTION_PUBLIC
rcl_action_endpoint_info_t
rcl_action_get_zero_initialized_endpoint_info(void);

/// Return a rcl_action_endpoint_info_array_t with members set to `NULL` or zero.
RCL_ACTION_PUBLIC
rcl_action_endpoint_info_array_t
rcl_action_get_zero_initialized_endpoint_info_array(void);

/// Finalize a rcl_action_endpoint_info_array_t.
/**
 * The info_array struct has its members deallocated and reset to `NULL` or
 * zero using the given allocator.
 *
 * \param[inout] info_array object to be finalized
 * \param[in] allocator the allocator used to allocate the array and its contents
 * \return `RCL_RET_OK` if successful, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_endpoint_info_array_fini(
  rcl_action_endpoint_info_array_t * info_array,
  rcutils_allocator_t * allocator);

/// Return a list of endpoint information for each action client of a given action.
/**
 * The `node` parameter must point to a valid node.
 *
 * The `action_name` parameter must not be `NULL` and must be a fully
 * qualified action name.
 * The action name is not automatically remapped by this function.
 *
 * Each entry of the returned list aggregates the endpoint information of all
 * the underlying entities of one action client, i.e. the clients of the
 * goal, cancel, and result services and the subscriptions on the feedback
 * and status topics.
 * See rcl_action_endpoint_info_t for the correlation semantics.
 *
 * The `clients_info` parameter must be allocated and zero initialized with
 * rcl_action_get_zero_initialized_endpoint_info_array().
 * This function allocates memory for the returned list of endpoint information
 * and so it is the callers responsibility to pass `clients_info` to
 * rcl_action_endpoint_info_array_fini() when it is no longer needed.
 * Failing to do so will result in leaked memory.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] allocator allocator for allocating space for the returned information
 * \param[in] action_name the fully qualified name of the action in question
 * \param[out] clients_info list of action client endpoint information
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_NAME_INVALID` if the action name is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_get_clients_info_by_action(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_endpoint_info_array_t * clients_info);

/// Return a list of endpoint information for each action server of a given action.
/**
 * The `node` parameter must point to a valid node.
 *
 * The `action_name` parameter must not be `NULL` and must be a fully
 * qualified action name.
 * The action name is not automatically remapped by this function.
 *
 * Each entry of the returned list aggregates the endpoint information of all
 * the underlying entities of one action server, i.e. the servers of the
 * goal, cancel, and result services and the publishers on the feedback and
 * status topics.
 * See rcl_action_endpoint_info_t for the correlation semantics.
 *
 * The `servers_info` parameter must be allocated and zero initialized with
 * rcl_action_get_zero_initialized_endpoint_info_array().
 * This function allocates memory for the returned list of endpoint information
 * and so it is the callers responsibility to pass `servers_info` to
 * rcl_action_endpoint_info_array_fini() when it is no longer needed.
 * Failing to do so will result in leaked memory.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] allocator allocator for allocating space for the returned information
 * \param[in] action_name the fully qualified name of the action in question
 * \param[out] servers_info list of action server endpoint information
 * \return `RCL_RET_OK` if the query was successful, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_NAME_INVALID` if the action name is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_ACTION_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_get_servers_info_by_action(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_endpoint_info_array_t * servers_info);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__GRAPH_H_
