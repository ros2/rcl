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
  //TODO(jacobperron): What mock elements go here?
} rosidl_action_type_support_t;

#include "rcl_action/types.h"
#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/visibility_control.h"


/// Internal rcl_action implementation struct.
struct rcl_action_client_impl_t;

/// Structure which encapsulates a ROS Action Client.
typedef struct rcl_action_client_t
{
  struct rcl_action_client_impl_t * impl;
} rcl_action_client_t;

/// Options available for a rcl action client.
typedef struct rcl_action_client_options_t
{
  /// Middleware quality of service settings for the action client.
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

/// Initialize a rcl action_client.
/**
 * After calling this function on a rcl_action_client_t, it can be used to send
 * goals of the given type to the given topic using rcl_client_send_goal_request()
 * and send cancel requests with rcl_action_send_cancel_request().
 * If a goal request is received by a (possibly remote) server and if the server
 * sends a response, the client can access the response through rcl_take_goal_response()
 * once the response is available to the client.
 * Likewise, an available response for a cancel request can be accessed with
 * rcl_take_cancel_response().
 *
 * The action client can also be used to take feedback and status messages for an accepted
 * goal. with rcl_action_take_feedback() and rcl_action_take_status().
 *
 * It can also take a result for a finished goal or a cancel request using
 * rcl_action_take_result() and rcl_action_take_cancel_response() respectively.
 *
 * The given rcl_node_t must be valid and the resulting rcl_action_client_t is
 * only valid as long as the given rcl_node_t remains valid.
 *
 * The rosidl_action_type_support_t is obtained on a per .action type basis.
 * When the user defines a ROS action, code is generated which provides the
 * required rosidl_action_type_support_t object.
 * This object can be obtained using a language appropriate mechanism.
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
 * #include <rcl_action/rcl_action.h>
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
 * \param[out] action_client preallocated action client structure
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
 * After calling, the node will no longer listen for goals for this action client.
 * (assuming this is the only action client of this type in this node).
 *
 * After calling, calls to rcl_wait(), rcl_action_send_goal_request(),
 * rcl_action_send_cancel_request(), rcl_action_take_feedback(), rcl_action_take_status(),
 * rcl_action_take_result(), and rcl_action_take_cancel_result() will fail when using
 * this action client.
 * Additionally, rcl_wait() will be interrupted if currently blocking.
 * However, the given node handle is still valid.
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
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
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
 * - qos = TODO(jacobperron): where to define default? and what should it be?
 * - allocator = rcl_get_default_allocator()
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_client_options_t
rcl_action_client_get_default_options(void);

/// Send a ROS goal using a rcl action client.
/**
 * It is the job of the caller to ensure that the type of the `ros_request`
 * parameter and the type associate with the client (via the type support)
 * match.
 * Passing a different type produces undefined behavior and cannot be checked
 * by this function and therefore no deliberate error will occur.
 *
 * rcl_action_send_goal_request() is an non-blocking call.
 *
 * The ROS request message given by the `ros_request` void pointer is always
 * owned by the calling code, but should remain constant during `send_goal_request`.
 *
 * This function is thread safe so long as access to both the action client and the
 * `ros_request` is synchronized.
 * That means that calling rcl_action_send_goal_request() from multiple threads is allowed,
 * but calling rcl_action_send_goal_request() at the same time as non-thread safe action
 * client functions is not, e.g. calling rcl_action_send_goal_request() and
 * rcl_action_client_fini() concurrently is not allowed.
 * Before and after calling rcl_action_send_goal_request() the message can change,
 * but it cannot be changed during the call to rcl_action_send_goal_request().
 * However, the same `ros_request` can be passed to multiple calls of
 * rcl_action_send_goal_request() simultaneously, even if the action clients differ.
 * The `ros_request` is unmodified by rcl_action_send_goal_request().
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes [1]
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] for unique pairs of clients and requests, see above for more</i>
 *
 * \param[in] action_client handle to the client which will make the response
 * \param[in] ros_request type-erased pointer to the ROS request message
 * \param[out] request_header pointer to a struct containing meta-data about the goal
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
  rcl_action_goal_id_t * request_header,
  void * ros_request);

/// Take a response for a goal request from an action server using an action client.
/**
 * It is the job of the caller to ensure that the type of the `ros_response`
 * parameter and the type associate with the action client (via the type support)
 * match.
 * Passing a different type to rcl_action_take_goal_response() produces undefined
 * behavior and cannot be checked by this function and therefore no deliberate
 * error will occur.
 *
 * The `response_header` is a action goal struct for meta-information about the response
 * being taken (e.g. the goal ID).
 * The caller must provide a pointer to an allocated struct.
 * This function will populate the struct's fields.
 *
 * `ros_response` should point to an already allocated ROS response message
 * struct of the correct type, into which the response from the action server will be
 * copied.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Maybe [1]
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] only if required when filling the message, avoided for fixed sizes</i>
 *
 * \param[in] action_client handle to the client that will take the goal response
 * \param[inout] response_header pointer to the response header
 * \param[inout] ros_response type-erased pointer to the ROS goal response message
 * \return `RCL_RET_OK` if the response was taken successfully, or
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
  rcl_action_goal_id_t * response_header,
  void * ros_response);

/// Take a ROS feedback message for an active goal.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_feedback(
  const rcl_action_client_t * action_client,
  rcl_action_goal_id_t * feedback_header,
  void * ros_message);

/// Take a ROS status message for an active goal.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_status(
  const rcl_action_client_t * action_client,
  rcl_action_goal_id_t * status_header,
  void * ros_message);

/// Send a cancel request for a goal using a rcl action client.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_cancel_request(
  const rcl_action_client_t * action_client,
  rcl_action_goal_id_t * request_header);

/// Take a cancel response using a rcl action client.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_cancel_response(
  const rcl_action_client_t * action_client,
  void * ros_message);

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
