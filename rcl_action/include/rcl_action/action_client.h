// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#ifndef RCL_ACTION__ACTION_CLIENT_H_
#define RCL_ACTION__ACTION_CLIENT_H_

#ifdef __cplusplus
extern "C"
{
#endif

// TODO(jacobperron): replace type support typedef with one defined in rosdl_generator_c
// #include "rosidl_generator_c/action_type_support_struct.h"
typedef struct rosidl_action_type_support_t
{
  // TODO(jacobperron): What mock elements go here?
} rosidl_action_type_support_t;

#include "rcl_action/types.h"
#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/visibility_control.h"


/// Internal action client implementation struct.
struct rcl_action_client_impl_t;

/// Structure which encapsulates a ROS action client.
typedef struct rcl_action_client_t
{
  struct rcl_action_client_impl_t * impl;
} rcl_action_client_t;

/// Options available for a rcl_action_client_t.
typedef struct rcl_action_client_options_t
{
  /// Middleware quality of service settings for the action client.
  // TODO(jacoberron): Add multiple QoS settings for services and topics
  rmw_qos_profile_t qos;
  /// Custom allocator for the action client, used for incidental allocations.
  /** For default behavior (malloc/free), see: rcl_get_default_allocator() */
  rcl_allocator_t allocator;
} rcl_action_client_options_t;

/// Return a rcl_action_client_t struct with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_client_t before passing to
 * rcl_action_client_init().
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_client_t
rcl_action_get_zero_initialized_client(void);

/// Initialize a rcl_action_client_t.
/**
 * After calling this function on a rcl_action_client_t, it can be used to send
 * goals of the given type to the given topic using rcl_action_send_goal_request().
 * If a goal request is sent to a (possibly remote) server and if the server
 * sends a response, the client can access the response with
 * rcl_take_goal_response() once the response is available.
 *
 * After a goal request has been accepted, the rcl_action_client_t associated with the
 * goal can perform the following operations:
 *
 * - Send a request for the result with rcl_action_send_result_request().
 * If the server sends a response when the goal terminates, the result can be accessed
 * with rcl_action_take_result_response(), once the response is available.
 * - Send a cancel request for the goal with rcl_action_send_cancel_request().
 * If the server sends a response to a cancel request, the client can access the
 * response with rcl_action_take_cancel_response() once the response is available.
 * - Take feedback about the goal with rcl_action_take_feedback().
 *
 * A rcl_action_client_t can be used to access the current status of all accepted goals
 * communicated by the action server with rcl_action_take_status().
 *
 * The given rcl_node_t must be valid and the resulting rcl_action_client_t is
 * only valid as long as the given rcl_node_t remains valid.
 *
 * The rosidl_action_type_support_t is obtained on a per .action type basis.
 * When the user defines a ROS action, code is generated which provides the
 * required rosidl_action_type_support_t object.
 * This object can be obtained using a language appropriate mechanism.
 *
 * \todo TODO(jacobperron) write these instructions once and link to it instead
 *
 * For C, a macro can be used (for example `example_interfaces/Fibonacci`):
 *
 * ```c
 * #include <rosidl_generator_c/action_type_support_struct.h>
 * #include <example_interfaces/action/fibonacci.h>
 * const rosidl_action_type_support_t * ts =
 *   ROSIDL_GET_ACTION_TYPE_SUPPORT(example_interfaces, Fibonacci);
 * ```
 *
 * For C++, a template function is used:
 *
 * ```cpp
 * #include <rosidl_generator_cpp/action_type_support.hpp>
 * #include <example_interfaces/action/fibonacci.h>
 * using rosidl_typesupport_cpp::get_action_type_support_handle;
 * const rosidl_action_type_support_t * ts =
 *   get_action_type_support_handle<example_interfaces::action::Fibonacci>();
 * ```
 *
 * The rosidl_action_type_support_t object contains action type specific
 * information used to send or take goals, results, and feedback.
 *
 * The topic name must be a c string that follows the topic and service name
 * format rules for unexpanded names, also known as non-fully qualified names:
 *
 * \see rcl_expand_topic_name
 *
 * The options struct allows the user to set the quality of service settings as
 * well as a custom allocator that is used when initializing/finalizing the
 * client to allocate space for incidentals, e.g. the action client name string.
 *
 * Expected usage (for C action clients):
 *
 * ```c
 * #include <rcl/rcl.h>
 * #include <rcl_action/action_client.h>
 * #include <rosidl_generator_c/action_type_support_struct.h>
 * #include <example_interfaces/action/fibonacci.h>
 *
 * rcl_node_t node = rcl_get_zero_initialized_node();
 * rcl_node_options_t node_ops = rcl_node_get_default_options();
 * rcl_ret_t ret = rcl_node_init(&node, "node_name", "/my_namespace", &node_ops);
 * // ... error handling
 * const rosidl_action_type_support_t * ts =
 *   ROSIDL_GET_ACTION_TYPE_SUPPORT(example_interfaces, Fibonacci);
 * rcl_action_client_t action_client = rcl_action_get_zero_initialized_client();
 * rcl_action_client_options_t action_client_ops = rcl_action_client_get_default_options();
 * ret = rcl_action_client_init(&action_client, &node, ts, "fibonacci", &action_client_ops);
 * // ... error handling, and on shutdown do finalization:
 * ret = rcl_action_client_fini(&action_client, &node);
 * // ... error handling for rcl_action_client_fini()
 * ret = rcl_node_fini(&node);
 * // ... error handling for rcl_node_fini()
 * ```
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[out] action_client a preallocated, zero-initialized action client structure
 *   to be initialized
 * \param[in] node valid rcl node handle
 * \param[in] type_support type support object for the action's type
 * \param[in] action_name the name of the action
 * \param[in] options action_client options, including quality of service settings
 * \return `RCL_RET_OK` if action_client was initialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_NAME_INVALID` if the given action name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_client_init(
  rcl_action_client_t * action_client,
  const rcl_node_t * node,
  const rosidl_action_type_support_t * type_support,
  const char * action_name,
  const rcl_action_client_options_t * options);

/// Finalize a rcl_action_client_t.
/**
 * After calling, the node will no longer listen for goals for this action client
 * (assuming this is the only action client of this type in this node).
 *
 * After calling, calls to rcl_wait(), rcl_action_send_goal_request(),
 * rcl_action_take_goal_response(), rcl_action_send_cancel_request(),
 * rcl_action_take_cancel_response(), rcl_action_send_result_request(),
 * rcl_action_take_result_response(), rcl_action_take_feedback(), and
 * rcl_action_take_status(), will fail when using this action client.
 *
 * Additionally, rcl_wait() will be interrupted if currently blocking.
 *
 * The given node handle is still valid.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] action_client handle to the action_client to be deinitialized
 * \param[in] node handle to the node used to create the action client
 * \return `RCL_RET_OK` if the action client was deinitialized successfully, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_client_fini(rcl_action_client_t * action_client, rcl_node_t * node);

/// Return the default action client options in a rcl_action_client_options_t.
/**
 * The defaults are:
 *
 * - qos = TODO(jacobperron): RFC where to define default? and what should it be?
 * - allocator = rcl_get_default_allocator()
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_client_options_t
rcl_action_client_get_default_options(void);

/// Send a ROS goal using a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * It is the job of the caller to ensure that the type of the `ros_goal`
 * parameter and the type associate with the client (via the type support)
 * match.
 * Passing a different type produces undefined behavior and cannot be checked
 * by this function and therefore no deliberate error will occur.
 *
 * It is the job of the caller to ensure that the `goal_info` is populated
 * with a valid goal ID.
 *
 * \todo TODO(jacobperron): Reference utility functions for generating new goal info/IDs
 *
 * The ROS goal message given by the `ros_goal` void pointer is always
 * owned by the calling code, but should remain constant during execution of this
 * function. i.e. Before and after calling rcl_action_send_goal_request() the
 * `ros_goal` message can change, but it cannot be changed during the call to
 * rcl_action_send_goal_request().
 * The same `ros_goal` can be passed to multiple calls of this function
 * simultaneously, even if the action clients differ.
 * The `ros_goal` is unmodified by this function.
 *
 * This function is thread safe so long as access to both the rcl_action_client_t
 * and the `ros_goal` are synchronized.
 * That means that calling rcl_action_send_goal_request() from multiple threads is allowed,
 * but calling rcl_action_send_goal_request() at the same time as non-thread safe action
 * client functions is not, e.g. calling rcl_action_send_goal_request() and
 * rcl_action_client_fini() concurrently is not allowed.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes [1]
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] for unique pairs of clients and goals, see above for more</i>
 *
 * \param[in] action_client handle to the client that will make the goal request
 * \param[in] goal_info pointer to a struct containing meta-data about the goal
 * \param[in] ros_goal pointer to the ROS goal message
 * \return `RCL_RET_OK` if the request was sent successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the client is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_goal_request(
  const rcl_action_client_t * action_client,
  const rcl_action_goal_info_t * goal_info,
  void * ros_goal);

/// Take a response for a goal request from an action server using a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * The caller must provide a pointer to an allocated struct for the `goal_info`.
 * If the goal is accepted, this function will populate `goal_info` with the goal ID
 * and the time that the server accepted the goal.
 *
 * This function will set `accepted` to `true` if the server accepted the goal.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client handle to the client that will take the goal response
 * \param[inout] goal_info pointer to the goal info
 * \param[out] accepted set to `true` if the goal was accepted by the server, `false` otherwise
 * \return `RCL_RET_ACTION_GOAL_ACCEPTED` if the response was taken successfully and
 *   the goal was accepted, or
 * \return `RCL_RET_ACTION_GOAL_REJECTED` if the response was taken successfully and
 *   the goal was rejected, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_ACTION_CLIENT_TAKE_FAILED` if take failed but no error occurred
 *         in the middleware, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_goal_response(
  const rcl_action_client_t * action_client,
  rcl_action_goal_info_t * goal_info);

/// Take a ROS feedback message for an active goal associated with a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * It is the job of the caller to ensure that the type of the `ros_feedback` parameter
 * and the type associate with the action client, via the type support, match.
 * Passing a different type to this function produces undefined behavior and cannot
 * be checked by this function and therefore no deliberate error will occur.
 *
 * TODO(jacobperron) blocking of take?
 * TODO(jacobperron) pre-, during-, and post-conditions for message ownership?
 * TODO(jacobperron) is rcl_take thread-safe?
 *
 * `goal_info` should point to preallocated struct.
 * If feedback is successfully taken, meta-data about the goal that the feedback
 * is associated with will be copied into the `goal_info struct.
 *
 * `ros_feedback` should point to a preallocated ROS message struct of the
 * correct type.
 * If feedback is successfully taken, the feedback message is copied to into the
 * `ros_feedback` struct.
 *
 * If allocation is required when taking the feedback, e.g. if space needs to
 * be allocated for a dynamically sized array in the target message, then the
 * allocator given in the action client options is used.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Maybe [1]
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] only if required when filling the feedback message, avoided for fixed sizes</i>
 *
 * \param[in] action_client handle to the client that will take the goal response
 * \param[out] goal_info pointer to a struct for meta-data about the goal associated
 *   with taken feedback
 * \param[out] ros_feedback pointer to the ROS feedback message.
 * \return `RCL_RET_OK` if the response was taken successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_CLIENT_TAKE_FAILED` if take failed but no error occurred
 *         in the middleware, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_feedback(
  const rcl_action_client_t * action_client,
  rcl_action_goal_info_t * goal_header,
  void * ros_feedback);

/// Take a ROS status message using a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * The caller is responsible for allocating the `status_array` struct with a
 * zero-initialization (the internal array should not be allocated).
 * If there is a successful take, then `status_array` is populated
 * with the allocator given in the action client options.
 * It is the callers responsibility to deallocate the `status_array` struct using
 * the allocator given in the action client options.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client handle to the client that will take status message
 * \param[out] status_array pointer to a struct containing an array of goal statuses.
 * \return `RCL_RET_OK` if the response was taken successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_CLIENT_TAKE_FAILED` if take failed but no error occurred
 *         in the middleware, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_status(
  const rcl_action_client_t * action_client,
  rcl_action_status_array_t * status_array);

/// Send a request for the result of a completed goal associated with a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * It is the job of the caller to ensure that the `goal_info` is populated
 * with a valid goal ID.
 * The rcl_action_client_t will be used to make a request for the result associated with
 * the `goal_info`.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client handle to the client that will send the result request
 * \param[in] goal_info pointer to a struct containing meta-data about the goal
 * \return `RCL_RET_OK` if the request was sent successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_result_request(
  const rcl_action_client_t * action_client,
  const rcl_action_goal_info_t * goal_info);

/// Take a ROS result message for a completed goal associated with a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * It is the job of the caller to ensure that the type of the `ros_result`
 * parameter and the type associate with the client (via the type support)
 * match.
 * Passing a different type produces undefined behavior and cannot be checked
 * by this function and therefore no deliberate error will occur.
 *
 * TODO(jacobperron) blocking of take?
 * TODO(jacobperron) pre-, during-, and post-conditions for message ownership?
 * TODO(jacobperron) is this thread-safe?
 *
 * `goal_info` should point to a preallocated struct.
 * If a result is successfully taken, meta-data about the goal that the result
 * is associated with will be copied into the `goal_info struct.
 *
 * `terminal_state` should point to a preallocated struct.
 * If a result is successfully taken, it is set to the goals terminal state.
 *
 * `ros_result` should point to a preallocated ROS message struct of the
 * correct type.
 * If a result is successfully taken, the result message is copied to into the
 * `ros_result` struct.
 *
 * If allocation is required when taking the result, e.g. if space needs to
 * be allocated for a dynamically sized array in the target message, then the
 * allocator given in the action client options is used.
 *
 * `terminal_state` is the resultant state of the goal.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Maybe [1]
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] only if required when filling the feedback message, avoided for fixed sizes</i>
 *
 * \param[in] action_client handle to the client that will take the result response
 * \param[out] goal_info preallocated struct where metadata about the goal associated
 *   with a taken result is copied
 * \param[out] terminal_state preallocated variable that is set to SUCCEEDED, ABORTED, or CANCELED
 * \param[out] ros_result preallocated struct where the ROS result message is copied.
 * \return `RCL_RET_OK` if the response was taken successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_CLIENT_TAKE_FAILED` if take failed but no error occurred
 *         in the middleware, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_result_response(
  const rcl_action_client_t * action_client,
  rcl_action_goal_info_t * goal_info,
  rcl_action_goal_state_t * terminal_state,
  void * ros_result);

/// Send a cancel request for a goal using a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * It is the job of the caller to ensure that the `goal_info` is populated
 * with a goal ID and a timestamp.
 * The following cancel policy applies based on the goal ID and the timestamp:
 *
 * - If the goal ID is zero and timestamp is zero, cancel all goals.
 * - If the goal ID is zero and timestamp is not zero, cancel all goals accepted
 *   at or before the timestamp.
 * - If the goal ID is not zero and timestamp is zero, cancel the goal with the
 *   given ID regardless of the time it was accepted.
 * - If the goal ID is not zero and timestamp is not zero, cancel the goal with the
 *   given ID and all goals accepted at or before the timestamp.
 *
 * The rcl_action_client_t will be used to make the cancel request associated with
 * the `goal_info`.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client handle to the client that will make the cancel request
 * \param[in] goal_info pointer to a struct for metadata about the goal(s) to cancel
 * \return `RCL_RET_OK` if the response was taken successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_cancel_request(
  const rcl_action_client_t * action_client,
  const rcl_action_goal_info_t * goal_info);

/// Take a cancel response using a rcl_action_client_t.
/**
 * This is a non-blocking call.
 *
 * The caller is responsible for allocating the `cancel_response` struct with a
 * zero-initialization (the internal array should not be allocated).
 * If there is a successful response is taken, then `cancel_response` is populated
 * with the allocator given in the action client options.
 * It is the callers responsibility to deallocate the `cancel_response` struct using
 * the allocator given in the action client options.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client handle to the client that will take the cancel response
 * \param[out] cancel_response a zero-initialized struct where the cancel response is copied.
 * \return `RCL_RET_OK` if the response was taken successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_CLIENT_TAKE_FAILED` if take failed but no error occurred
 *         in the middleware, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_cancel_response(
  const rcl_action_client_t * action_client,
  rcl_action_cancel_response_t * cancel_response);

/// Get the name of the action for an action client.
/**
 * This function returns the action client's internal topic name string.
 * This function can fail, and therefore return `NULL`, if the:
 *   - action client is `NULL`
 *   - action client is invalid (never called init, called fini, or invalid)
 *
 * The returned string is only valid as long as the action client is valid.
 * The value of the string may change if the topic name changes, and therefore
 * copying the string is recommended if this is a concern.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client the pointer to the action client
 * \return name string if successful, otherwise `NULL`
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const char *
rcl_action_client_get_action_name(const rcl_action_client_t * action_client);

/// Return the rcl action client options.
/**
 * This function returns the action client's internal options struct.
 * This function can fail, and therefore return `NULL`, if the:
 *   - action client is `NULL`
 *   - action client is invalid (never called init, called fini, or invalid)
 *
 * The returned struct is only valid as long as the action client is valid.
 * The values in the struct may change if the action client's options change,
 * and therefore copying the struct is recommended if this is a concern.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client pointer to the action client
 * \return options struct if successful, otherwise `NULL`
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const rcl_action_client_options_t *
rcl_action_client_get_options(const rcl_action_client_t * action_client);

/// Check that the action client is valid.
/**
 * The bool returned is `false` if `action_client` is invalid.
 * The bool returned is `true` otherwise.
 * In the case where `false` is to be returned, an error message is set.
 * This function cannot fail.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_client pointer to the rcl action client
 * \param[in] error_msg_allocator a valid allocator or `NULL`
 * \return `true` if `action_client` is valid, otherwise `false`
 */
RCL_PUBLIC
bool
rcl_action_client_is_valid(
  const rcl_action_client_t * action_client,
  rcl_allocator_t * error_msg_allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__ACTION_CLIENT_H_
