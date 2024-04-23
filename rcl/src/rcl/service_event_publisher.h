// Copyright 2022 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__SERVICE_EVENT_PUBLISHER_H_
#define RCL__SERVICE_EVENT_PUBLISHER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/service_introspection.h"
#include "rcl/time.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#include "rosidl_runtime_c/service_type_support_struct.h"

typedef struct rcl_service_event_publisher_s
{
  /// Handle to publisher for publishing service events
  rcl_publisher_t * publisher;
  /// Name of service introspection topic: <service_name>/<RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX>
  char * service_event_topic_name;
  /// Current state of introspection; off, metadata, or contents
  rcl_service_introspection_state_t introspection_state;
  /// Handle to clock for timestamping service events
  rcl_clock_t * clock;
  /// Publisher options for service event publisher
  rcl_publisher_options_t publisher_options;
  /// Handle to service typesupport
  const rosidl_service_type_support_t * service_type_support;
} rcl_service_event_publisher_t;

/// Return a rcl_service_event_publisher_t struct with members set to `NULL`.
/**
 * Should be called to get a null rcl_service_event_publisher_t before passing to
 * rcl_service_event_publisher_init().
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_service_event_publisher_t
rcl_get_zero_initialized_service_event_publisher(void);

/// Initialize a service event publisher.
/**
 * After calling this function on a rcl_service_event_publisher_t, it can be used to
 * send service introspection messages by calling rcl_send_service_event_message().
 *
 * The given rcl_node_t must be valid and the resulting rcl_service_event_publisher_t is
 * only valid as long as the given rcl_node_t remains valid.
 *
 * Similarly, the given rcl_clock_t must be valid and the resulting rcl_service_event_publisher_t
 * is only valid as long as the given rcl_clock_t remains valid.
 *
 * The passed in service_name should be the fully-qualified, remapped service name.
 * The service event publisher will add a custom suffix as the topic name.
 *
 * The rosidl_service_type_support_t is obtained on a per `.srv` type basis.
 * When the user defines a ROS service, code is generated which provides the
 * required rosidl_service_type_support_t object.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | Maybe [1]
 * Lock-Free          | Maybe [1]
 * <i>[1] rmw implementation defined</i>
 *
 * \param[inout] service_event_publisher preallocated rcl_service_event_publisher_t
 * \param[in] node valid rcl_node_t to use to create the introspection publisher
 * \param[in] clock valid rcl_clock_t to use to generate the introspection timestamps
 * \param[in] publisher_options options to use when creating the introspection publisher
 * \param[in] service_name fully-qualified and remapped service name
 * \param[in] service_type_support type support library associated with this service
 * \return #RCL_RET_OK if the call was successful
 * \return #RCL_RET_INVALID_ARGUMENT if the event publisher, client, or node is invalid,
 * \return #RCL_RET_NODE_INVALID if the given node is invalid, or
 * \return #RCL_RET_BAD_ALLOC if a memory allocation failed, or
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_event_publisher_init(
  rcl_service_event_publisher_t * service_event_publisher,
  const rcl_node_t * node,
  rcl_clock_t * clock,
  const rcl_publisher_options_t publisher_options,
  const char * service_name,
  const rosidl_service_type_support_t * service_type_support);

/// Finalize a rcl_service_event_publisher_t.
/**
 * After calling this function, calls to any of the other functions here
 * (except for rcl_service_event_publisher_init()) will fail.
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
 * \param[inout] service_event_publisher handle to the event publisher to be finalized
 * \param[in] node a valid (not finalized) handle to the node used to create the client
 * \return #RCL_RET_OK if client was finalized successfully, or
 * \return #RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 * \return #RCL_RET_NODE_INVALID if the node is invalid, or
 * \return #RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_event_publisher_fini(
  rcl_service_event_publisher_t * service_event_publisher,
  rcl_node_t * node);

/// Check that the service event publisher is valid.
/**
 * The bool returned is `false` if the service event publisher is invalid.
 * The bool returned is `true` otherwise.
 * In the case where `false` is returned, an error message is set.
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
 * \param[in] service_event_publisher pointer to the service event publisher
 * \return `true` if `service_event_publisher` is valid, otherwise `false`
 */
RCL_PUBLIC
bool
rcl_service_event_publisher_is_valid(const rcl_service_event_publisher_t * service_event_publisher);

/// Send a service event message.
/**
 * It is the job of the caller to ensure that the type of the `ros_request`
 * parameter and the type associated with the event publisher (via the type support)
 * match.
 * Passing a different type to publish produces undefined behavior and cannot
 * be checked by this function and therefore no deliberate error will occur.
 *
 * rcl_send_service_event_message() is a potentially blocking call.
 *
 * The ROS request message given by the `ros_response_request` void pointer is always
 * owned by the calling code, but should remain constant during rcl_send_service_event_message().
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] service_event_publisher pointer to the service event publisher
 * \param[in] event_type introspection event type from service_msgs::msg::ServiceEventInfo
 * \param[in] ros_response_request type-erased pointer to the ROS response request
 * \param[in] sequence_number sequence number of the event
 * \param[in] guid GUID associated with this event
 * \return #RCL_RET_OK if the event was published successfully, or
 * \return #RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 * \return #RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_send_service_event_message(
  const rcl_service_event_publisher_t * service_event_publisher,
  uint8_t event_type,
  const void * ros_response_request,
  int64_t sequence_number,
  const uint8_t guid[16]);

/// Change the operating state of this service event publisher.
/**
 * \param[in] service_event_publisher pointer to the service event publisher
 * \param[in] introspection_state new introspection state
 * \return #RCL_RET_OK if the event was published successfully, or
 * \return #RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
rcl_ret_t
rcl_service_event_publisher_change_state(
  rcl_service_event_publisher_t * service_event_publisher,
  rcl_service_introspection_state_t introspection_state);

#ifdef __cplusplus
}
#endif
#endif  // RCL__SERVICE_EVENT_PUBLISHER_H_
