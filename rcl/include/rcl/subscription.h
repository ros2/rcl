// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__SUBSCRIPTION_H_
#define RCL__SUBSCRIPTION_H_

#if __cplusplus
extern "C"
{
#endif

#include "rosidl_generator_c/message_type_support_struct.h"

#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/visibility_control.h"

/// Internal rcl implementation struct.
struct rcl_subscription_impl_t;

/// Handle for a rcl subscription.
typedef struct rcl_subscription_t
{
  struct rcl_subscription_impl_t * impl;
} rcl_subscription_t;

/// Options available for a rcl subscription.
typedef struct rcl_subscription_options_t
{
  /// Middleware quality of service settings for the subscription.
  rmw_qos_profile_t qos;
  /// If true, messages published from within the same node are ignored.
  bool ignore_local_publications;
  /// Custom allocator for the subscription, used for incidental allocations.
  /* For default behavior (malloc/free), see: rcl_get_default_allocator() */
  rcl_allocator_t allocator;
} rcl_subscription_options_t;

/// Return a rcl_subscription_t struct with members set to NULL.
/* Should be called to get a null rcl_subscription_t before passing to
 * rcl_initalize_subscription().
 * It's also possible to use calloc() instead of this if the rcl_subscription_t
 * is being allocated on the heap.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_subscription_t
rcl_get_zero_initialized_subscription(void);

/// Initialize a ROS subscription.
/* After calling this function on a rcl_subscription_t, it can be used to take
 * messages of the given type to the given topic using rcl_take().
 *
 * The given rcl_node_t must be valid and the resulting rcl_subscription_t is
 * only valid as long as the given rcl_node_t remains valid.
 *
 * The rosidl_message_type_support_t is obtained on a per .msg type basis.
 * When the user defines a ROS message, code is generated which provides the
 * required rosidl_message_type_support_t object.
 * This object can be obtained using a language appropriate mechanism.
 * \TODO(wjwwood) probably should talk about this once and link to it instead
 * For C this macro can be used (using std_msgs/String as an example):
 *
 *    #include <rosidl_generator_c/message_type_support.h>
 *    #include <std_msgs/msgs/string.h>
 *    rosidl_message_type_support_t * string_ts =
 *      ROSIDL_GET_MESSAGE_TYPE_SUPPORT(std_msgs, String);
 *
 * For C++ a template function is used:
 *
 *    #include <rosidl_generator_cpp/message_type_support.hpp>
 *    #include <std_msgs/msgs/string.hpp>
 *    rosidl_message_type_support_t * string_ts =
 *      rosidl_generator_cpp::get_message_type_support_handle<std_msgs::msg::String>();
 *
 * The rosidl_message_type_support_t object contains message type specific
 * information used to publish messages.
 *
 * \TODO(wjwwood) update this once we've come up with an official scheme.
 * The topic name must be a non-empty string which follows the topic naming
 * format.
 *
 * The options struct allows the user to set the quality of service settings as
 * well as a custom allocator which is used when (de)initializing the
 * subscription to allocate space for incidental things, e.g. the topic
 * name string.
 *
 * Expected usage (for C messages):
 *
 *    #include <rcl/rcl.h>
 *    #include <rosidl_generator_c/message_type_support.h>
 *    #include <std_msgs/msgs/string.h>
 *
 *    rcl_node_t node = rcl_get_zero_initialized_node();
 *    rcl_node_options_t node_ops = rcl_node_get_default_options();
 *    rcl_ret_t ret = rcl_node_init(&node, "node_name", &node_ops);
 *    // ... error handling
 *    rosidl_message_type_support_t * ts = ROSIDL_GET_MESSAGE_TYPE_SUPPORT(std_msgs, String);
 *    rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
 *    rcl_subscription_options_t subscription_ops = rcl_subscription_get_default_options();
 *    ret = rcl_subscription_init(&subscription, &node, ts, "chatter", &subscription_ops);
 *    // ... error handling, and when finished deinitialization
 *    ret = rcl_subscription_fini(&subscription, &node);
 *    // ... error handling for rcl_subscription_fini()
 *    ret = rcl_node_fini(&node);
 *    // ... error handling for rcl_node_fini()
 *
 * This function is not thread-safe.
 *
 * \param[out] subscription preallocated subscription structure
 * \param[in] node valid rcl node handle
 * \param[in] type_support type support object for the topic's type
 * \param[in] topic_name the name of the topic
 * \param[in] options subscription options, including quality of service settings
 * \return RCL_RET_OK if subscription was initialized successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_BAD_ALLOC if allocating memory failed, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_subscription_init(
  rcl_subscription_t * subscription,
  const rcl_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rcl_subscription_options_t * options);

/// Finalize a rcl_subscription_t.
/* After calling, the node will no longer be subscribed on this topic
 * (assuming this is the only subscription on this topic in this node).
 *
 * After calling, calls to rcl_wait and rcl_take will fail when using this
 * subscription.
 * Additioanlly rcl_wait will be interrupted if currently blocking.
 * However, the given node handle is still valid.
 *
 * This function is not thread-safe.
 *
 * \param[inout] subscription handle to the subscription to be deinitialized
 * \param[in] node handle to the node used to create the subscription
 * \return RCL_RET_OK if subscription was deinitialized successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_subscription_fini(rcl_subscription_t * subscription, rcl_node_t * node);

/// Return the default subscription options in a rcl_subscription_options_t.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_subscription_options_t
rcl_subscription_get_default_options(void);

/// Take a ROS message from a topic using a rcl subscription.
/* It is the job of the caller to ensure that the type of the ros_message
 * argument and the type associate with the subscription, via the type
 * support, match.
 * Passing a different type to rcl_take produces undefined behavior and cannot
 * be checked by this function and therefore no deliberate error will occur.
 *
 * TODO(wjwwood) blocking of take?
 * TODO(wjwwood) pre-, during-, and post-conditions for message ownership?
 * TODO(wjwwood) is rcl_take thread-safe?
 * TODO(wjwwood) Should there be an rcl_message_info_t?
 *
 * The ros_message pointer should point to an already allocated ROS message
 * struct of the correct type, into which the taken ROS message will be copied
 * if one is available.
 * If taken is false after calling, then the ROS message will be unmodified.
 *
 * If allocation is required when taking the message, e.g. if space needs to
 * be allocated for a dynamically sized array in the target message, then the
 * allocator given in the subscription options is used.
 *
 * The rmw message_info struct contains meta information about this particular
 * message instance, like what the GUID of the publisher which published it
 * originally or whether or not the message received from within the same
 * process.
 * The message_info argument should be an already allocated rmw_message_info_t
 * structure.
 * Passing NULL for message_info will result in the argument being ignored.
 *
 * \param[in] subscription the handle to the subscription from which to take
 * \param[inout] ros_message type-erased ptr to a allocated ROS message
 * \param[out] message_info rmw struct which contains meta-data for the message
 * \return RCL_RET_OK if the message was published, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_SUBSCRIPTION_INVALID if the subscription is invalid, or
 *         RCL_RET_BAD_ALLOC if allocating memory failed, or
 *         RCL_RET_SUBSCRIPTION_TAKE_FAILED if take failed but no error
 *         occurred in the middleware, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_take(
  const rcl_subscription_t * subscription,
  void * ros_message,
  rmw_message_info_t * message_info);

/// Get the topic name for the subscription.
/* This function returns the subscription's internal topic name string.
 * This function can fail, and therefore return NULL, if the:
 *   - subscription is NULL
 *   - subscription is invalid (never called init, called fini, or invalid)
 *
 * The returned string is only valid as long as the subscription is valid.
 * The value of the string may change if the topic name changes, and therefore
 * copying the string is recommended if this is a concern.
 *
 * This function is not thread-safe, and copying the result is not thread-safe.
 *
 * \param[in] subscription the pointer to the subscription
 * \return name string if successful, otherwise NULL
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const char *
rcl_subscription_get_topic_name(const rcl_subscription_t * subscription);

/// Return the rcl subscription options.
/* This function returns the subscription's internal options struct.
 * This function can fail, and therefore return NULL, if the:
 *   - subscription is NULL
 *   - subscription is invalid (never called init, called fini, or invalid)
 *
 * The returned struct is only valid as long as the subscription is valid.
 * The values in the struct may change if the subscription's options change,
 * and therefore copying the struct is recommended if this is a concern.
 *
 * This function is not thread-safe, and copying the result is not thread-safe.
 *
 * \param[in] subscription pointer to the subscription
 * \return options struct if successful, otherwise NULL
 */
RCL_PUBLIC
RCL_WARN_UNUSED
const rcl_subscription_options_t *
rcl_subscription_get_options(const rcl_subscription_t * subscription);

/// Return the rmw subscription handle.
/* The handle returned is a pointer to the internally held rmw handle.
 * This function can fail, and therefore return NULL, if the:
 *   - subscription is NULL
 *   - subscription is invalid (never called init, called fini, or invalid)
 *
 * The returned handle is made invalid if the subscription is finalized or if
 * rcl_shutdown() is called.
 * The returned handle is not guaranteed to be valid for the life time of the
 * subscription as it may be finalized and recreated itself.
 * Therefore it is recommended to get the handle from the subscription using
 * this function each time it is needed and avoid use of the handle
 * concurrently with functions that might change it.
 *
 * This function is not thread-safe.
 *
 * \param[in] subscription pointer to the rcl subscription
 * \return rmw subscription handle if successful, otherwise NULL
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rmw_subscription_t *
rcl_subscription_get_rmw_handle(const rcl_subscription_t * subscription);

#if __cplusplus
}
#endif

#endif  // RCL__SUBSCRIPTION_H_
