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

#ifndef RCL_ACTION__ACTION_SERVER_H_
#define RCL_ACTION__ACTION_SERVER_H_

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

#include "rcl_action/goal_handle.h"
#include "rcl_action/types.h"
#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/visibility_control.h"


/// Internal rcl_action implementation struct.
struct rcl_action_server_impl_t;

/// Structure which encapsulates a ROS Action Server.
typedef struct rcl_action_server_t
{
  struct rcl_action_server_impl_t * impl;
} rcl_action_server_t;

/// Options available for a rcl action server.
typedef struct rcl_action_server_options_t
{
  /// Middleware quality of service settings for the action server.
  rmw_qos_profile_t qos;
  /// Custom allocator for the action server, used for incidental allocations.
  /** For default behavior (malloc/free), see: rcl_get_default_allocator() */
  rcl_allocator_t allocator;
  // TODO(jacobperron): Consider a server 'policy' defining things like queue length
  //                    cancel policy, and result timeout policy
} rcl_action_server_options_t;

/// Return a rcl_action_server_t struct with members set to `NULL`.
/**
 * Should be called to get a null rcl_action_server_t before passing to
 * rcl_action_server_init().
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_server_t
rcl_action_get_zero_initialized_server(void);

/// Initialize a rcl action_server.
/**
 * After calling this function on a rcl_action_server_t, it can be used to take
 * goals of the given type to the given topic using rcl_action_take_goal_request()
 * and take cancel requests with rcl_aciton_take_cancel_request().
 * It can also send a result for a request using rcl_action_send_result() or
 * rcl_action_send_cancel_response().
 *
 * After accepting a goal with rcl_action_take_goal_request(), the rcl_action_server_t can
 * be used to send feedback with rcl_action_publish_feedback() and send status
 * messages with rcl_action_publish_status().
 *
 * The given rcl_node_t must be valid and the resulting rcl_action_server_t is
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
 * client to allocate space for incidentals, e.g. the action server name string.
 *
 * Expected usage (for C action servers):
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
 * rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();
 * rcl_action_server_options_t action_server_ops = rcl_action_server_get_default_options();
 * ret = rcl_action_server_init(&action_server, &node, ts, "fibonacci", &action_server_ops);
 * // ... error handling, and on shutdown do finalization:
 * ret = rcl_action_server_fini(&action_server, &node);
 * // ... error handling for rcl_action_server_fini()
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
 * \param[out] action_server preallocated action server structure
 * \param[in] node valid rcl node handle
 * \param[in] type_support type support object for the action's type
 * \param[in] action_name the name of the action
 * \param[in] options action_server options, including quality of service settings
 * \return `RCL_RET_OK` if action_server was initialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_NAME_INVALID` if the given action name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_server_init(
  rcl_action_server_t * action_server,
  const rcl_node_t * node,
  const rosidl_action_type_support_t * type_support,
  const char * action_name,
  const rcl_action_server_options_t * options);

/// Finalize a rcl_action_server_t.
/**
 * After calling, the node will no longer listen for goals for this action server.
 * (assuming this is the only action server of this type in this node).
 *
 * After calling, calls to rcl_wait(), rcl_action_take_goal_request(),
 * rcl_action_take_cancel_request(), rcl_action_publish_feedback(),
 * rcl_action_publish_status(), rcl_action_send_result(), and
 * rcl_action_send_cancel_response() will fail when using this action server.
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
 * \param[inout] action_server handle to the action_server to be deinitialized
 * \param[in] node handle to the node used to create the action server
 * \return `RCL_RET_OK` if the action server was deinitialized successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_NODE_INVALID` if the node is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_server_fini(rcl_action_server_t * action_server, rcl_node_t * node);

/// Return the default action server options in a rcl_action_server_options_t.
/**
 * The defaults are:
 *
 * - qos = TODO(jacobperron): where to define default? and what should it be?
 * - allocator = rcl_get_default_allocator()
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_action_server_options_t
rcl_action_server_get_default_options(void);

/// Take a pending ROS goal using a rcl action server.
/**
 * It is the job of the caller to ensure that the type of the ros_goal
 * argument and the type associate with the action server, via the type
 * support, match.
 * Passing a different type produces undefined behavior and cannot
 * be checked by this function and therefore no deliberate error will occur.
 *
 * \todo TODO(jacobperron) blocking of take?
 * \todo TODO(jacobperron) pre-, during-, and post-conditions for message ownership?
 * \todo TODO(jacobperron) is rcl_action_take_goal_request thread-safe?
 *
 * The ros_goal pointer should point to an already allocated ROS goal message
 * struct of the correct type, into which the taken ROS goal will be copied
 * if one is available.
 * If taken is false after calling, then the ROS goal will be unmodified.
 *
 * If allocation is required when taking the request, e.g. if space needs to
 * be allocated for a dynamically sized array in the target message, then the
 * allocator given in the action server options is used.
 *
 * request_header is a pointer to a pre-allocated action goal ID struct
 * containing meta-information about the request (e.g. goal ID and timestamp).
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Maybe [1]
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] only if required when filling the request, avoided for fixed sizes</i>
 *
 * \param[in] action_server the handle to the action server from which to take
 * \param[inout] request_header pointer to struct containing meta-data about the goal
 * \param[inout] ros_request type-erased ptr to an allocated ROS request message
 * \return `RCL_RET_OK` if the request was taken, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ACTION_SERVER_TAKE_FAILED` if take failed but no error occurred
 *         in the middleware, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_goal_request(
  const rcl_action_server_t * action_server,
  rcl_action_goal_header_t * request_header,
  void * ros_request);

/// Send a response for a goal request to an action client using an action server.
/**
 * It is the job of the caller to ensure that the type of the `ros_response`
 * parameter and the type associate with the action server (via the type support)
 * match.
 * Passing a different type to rcl_action_send_goal_response() produces undefined
 * behavior and cannot be checked by this function and therefore no deliberate
 * error will occur.
 *
 * rcl_action_send_goal_response() is a non-blocking call.
 *
 * The ROS response message given by the `ros_response` void pointer is always
 * owned by the calling code, but should remain constant during
 * rcl_action_send_goal_response().
 *
 * This function is thread safe so long as access to both the action server and the
 * `ros_response` is synchronized.
 * That means that calling rcl_aciton_send_goal_response() from multiple threads is
 * allowed, but calling rcl_action_send_goal_response() at the same time as non-thread safe
 * action server functions is not, e.g. calling rcl_action_send_goal_response() and
 * rcl_action_server_fini() concurrently is not allowed.
 * Before calling rcl_action_send_goal_response() the message can change and after calling
 * rcl_action_send_goal_response() the message can change, but it cannot be changed during
 * the rcl_action_send_goal_response() call.
 * The same `ros_response`, however, can be passed to multiple calls of
 * rcl_action_send_goal_response() simultaneously, even if the action servers differ.
 * The `ros_response` is unmodified by rcl_action_send_goal_response().
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes [1]
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] for unique pairs of action servers and responses, see above for more</i>
 *
 * \param[in] action_server handle to the action server that will make the goal response
 * \param[inout] response_header ptr to the struct holding metadata about the goal ID
 * \param[in] ros_response type-erased pointer to the ROS response message
 * \return `RCL_RET_OK` if the response was sent successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_goal_response(
  const rcl_action_server_t * action_server,
  const rcl_action_goal_header_t * response_header,
  const bool accepted,
  rcl_action_goal_handle_t * goal_handle);

/// Publish a ROS feedback message for an active goal.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_publish_feedback(
  const rcl_action_server_t * action_server,
  rcl_action_goal_handle_t * goal_handle,
  void * ros_message);

/// Publish a ROS status message for an active goal.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_publish_status(
  const rcl_action_server_t * action_server,
  rcl_action_goal_handle_t * goal_handle,
  void * ros_message);

/// Take a pending result request using a rcl action server.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_result_request(
  const rcl_action_server_t * action_server,
  rcl_action_goal_header_t * request);

/// Send a result response using a rcl action server.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_result_response(
  const rcl_action_server_t * action_server,
  const rcl_action_goal_handle_t * goal_handle,
  const rcl_action_goal_state_t terminal_state,
  void * ros_message);

/// Take a pending cancel request using a rcl action server.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_take_cancel_request(
  const rcl_action_server_t * action_server,
  rcl_action_goal_header_t * request);

/// Send a cancel response using a rcl action server.
/**
 * \todo TODO(jacobperron): Document.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_action_send_cancel_response(
  const rcl_action_server_t * action_server,
  uint32_t num_goals_canceling
  rcl_action_goal_handle_t ** goal_handles);

/// Get the name of the action for an action server.
/**
 * This function returns the action server's internal topic name string.
 * This function can fail, and therefore return `NULL`, if the:
 *   - action server is `NULL`
 *   - action server is invalid (never called init, called fini, or invalid)
 *
 * The returned string is only valid as long as the action server is valid.
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
 * \param[in] action_server the pointer to the action server
 * \return name string if successful, otherwise `NULL`
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const char *
rcl_action_server_get_action_name(const rcl_action_server_t * action_server);

/// Return the rcl action server options.
/**
 * This function returns the action server's internal options struct.
 * This function can fail, and therefore return `NULL`, if the:
 *   - action server is `NULL`
 *   - action server is invalid (never called init, called fini, or invalid)
 *
 * The returned struct is only valid as long as the action server is valid.
 * The values in the struct may change if the action server's options change,
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
 * \param[in] action_server pointer to the action server
 * \return options struct if successful, otherwise `NULL`
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const rcl_action_server_options_t *
rcl_action_server_get_options(const rcl_action_server_t * action_server);

/// Return the goal handles for all active or terminated goals.
/**
 * A pointer to the internally held array of goal handle structs is returned
 * along with the number of items in the array.
 * Goals that have terminated, successfully responded to a client with a
 * result, and have expired (timed out) are not present in the array.
 *
 * This function can fail, and therefore return `NULL`, if the:
 *   - action server is `NULL`
 *   - action server is invalid (never called init, called fini, or invalid)
 *
 * The returned handle is made invalid if the action server is finalized or if
 * rcl_shutdown() is called.
 * The returned handle is not guaranteed to be valid for the life time of the
 * action server as it may be finalized and recreated itself.
 * Therefore, it is recommended to get the handle from the action server using
 * this function each time it is needed and avoid use of the handle
 * concurrently with functions that might change it.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] action_server pointer to the rcl action server
 * \param[out] num_goals is set to the number of goals in the returned array if successful,
 *   not set otherwise.
 * \return pointer to an array goal handles if successful, otherwise `NULL`
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const rcl_action_goal_handle_t *
rcl_action_server_get_goal_handles(
  const rcl_action_server_t * action_server,
  int32_t & num_goals);

/// Check that the action server is valid.
/**
 * The bool returned is `false` if `action_server` is invalid.
 * The bool returned is `true` otherwise.
 * In the case where `false` is to be returned, an
 * error message is set.
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
 * \param[in] action_server pointer to the rcl action server
 * \param[in] error_msg_allocator a valid allocator or `NULL`
 * \return `true` if `action_server` is valid, otherwise `false`
 */
RCL_PUBLIC
bool
rcl_action_server_is_valid(
  const rcl_action_server_t * action_server,
  rcl_allocator_t * error_msg_allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__ACTION_SERVER_H_
