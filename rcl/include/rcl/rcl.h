/* Copyright 2014 Open Source Robotics Foundation, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RCL_RCL_RCL_H_
#define RCL_RCL_RCL_H_

#include <stdbool.h>  // For bool
#include <stddef.h>  // For size_t

// For rosidl_message_type_support_t
#include <rosidl_generator_c/message_type_support.h>

#include "types.h"  // For rcl_*_t types

/// Global initialization for rcl; should be called once per process.
rcl_ret_t
rcl_init(int argc, char **argv);

/// Creates a rcl_node_t; used to implement a ROS Node.
rcl_node_t *
rcl_create_node(const char * name);

/// Destroys a rcl_node_t.
rcl_ret_t
rcl_destroy_node(rcl_node_t * node);

/// Creates a rcl_publisher_t; used to implement a ROS Publisher.
rcl_publisher_t *
rcl_create_publisher(const rcl_node_t * node,
                     const rosidl_message_type_support_t * type_support,
                     const char * topic_name,
                     size_t queue_size);

/// Destroys a rcl_publisher_t.
rcl_ret_t
rcl_destroy_publisher(rcl_publisher_t * publisher);

/// Publishes a ROS message to a ROS Topic using a rcl_publisher_t.
rcl_ret_t
rcl_publish(const rcl_publisher_t * publisher, const void * ros_message);

/// Creates a rcl_subscription_t; used to implement a ROS Subscription.
rcl_subscription_t *
rcl_create_subscription(const rcl_node_t * node,
                        const rosidl_message_type_support_t * type_support,
                        const char * topic_name,
                        size_t queue_size);

/// Destroys a rcl_subscription_t.
rcl_ret_t
rcl_destroy_subscription(rcl_subscription_t * subscription);

/// Takes a ROS message from a given rcl_subscription_t if one is available.
rcl_ret_t
rcl_take(const rcl_subscription_t * subscriber, void * ros_message);

/// Creates a rcl_guard_condition_t; is used by the implementation to
/// asynchronously interrupt rmw_wait.
rcl_guard_condition_t *
rcl_create_guard_condition();

/// Destroys a rcl_guard_condition_t.
rcl_ret_t
rcl_destroy_guard_condition(rcl_guard_condition_t * guard_condition);

/// Triggers the condition, which will interrupt rmw_wait asynchronously.
rcl_ret_t
rcl_trigger_guard_condition(const rcl_guard_condition_t * guard_condition);

/// Creates a rcl_callback_group_t, which provides hints about how to execute
/// groups of callbacks to an executor.
rcl_callback_group_t *
rcl_create_callback_group(rcl_node_t * node);

/// Destroys a rcl_callback_group_t.
rcl_ret_t
rcl_destroy_callback_group(rcl_callback_group_t * callback_group);

/// Creates a rcl_subscription_info_t, which associates rcl_subscription_t's
/// to a callback group.
rcl_subscription_info_t *
rcl_create_subscription_info(rcl_subscription_t * const subscription,
                             rcl_callback_group_t * const callback_group);

/// Destroys a rcl_subscription_info_t.
rcl_ret_t
rcl_destroy_subscription_info(rcl_subscription_info_t * subscription_info);

/// Creates a rcl_timer_info_t, which associates a ROS Timer's guard condition
/// with a callback group.
rcl_timer_info_t *
rcl_create_timer_info(rcl_guard_condition_t * const guard_condition,
                      rcl_callback_group_t * const callback_group);

/// Destroys a rcl_timer_info_t.
rcl_ret_t
rcl_destroy_timer_info(rcl_timer_info_t * timer_info);

/// Creates a rcl_executor_helper_t, which is a collection of callable items.
rcl_executor_helper_t *
rcl_create_executor_helper();

/// Destroys a rcl_executor_helper_t.
rcl_ret_t
rcl_destroy_executor_helper(rcl_executor_helper_t * executor_helper);

/// Add a rcl_subscription_info_t to the collection of callable items.
rcl_ret_t
rcl_add_subscription_info(rcl_executor_helper_t * executor_helper,
                          rcl_subscription_info_t * subscription_info);

/// Removes a rcl_subscription_t from the collection of callable items.
rcl_ret_t
rcl_remove_subscription_info(rcl_executor_helper_t * executor_helper,
                             rcl_subscription_info_t * subscription_info);

/// Add a rcl_timer_info_t to the collection of callable items.
rcl_ret_t
rcl_add_timer_info(rcl_executor_helper_t * executor_helper,
                   rcl_timer_info_t * timer_info);

/// Removes a rcl_subscription_t from the collection of callable items.
rcl_ret_t
rcl_remove_timer_info(rcl_executor_helper_t * executor_helper,
                      rcl_timer_info_t * timer_info);

/// Finds the next ready to be called item if one exists; optionally blocking.
rcl_ret_t
rcl_get_next_any_executable(rcl_executor_helper_t * executor_helper,
                            rcl_any_executable_t * any_executable,
                            bool non_blocking);

#endif  /* RCL_RCL_RCL_H_ */
