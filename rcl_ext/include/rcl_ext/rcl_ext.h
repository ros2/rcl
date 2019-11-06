// Copyright (c) 2018 - for information on the respective copyright owner
// see the NOTICE file and/or the repository https://github.com/micro-ROS/rcl_executor.
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

#ifndef RCL__RCL_EXT_H_
#define RCL__RCL_EXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdarg.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rcutils/logging_macros.h>


/*! \file rcl_ext.h
    \brief rcl_ext provides a thin interface to the Ros Client Library to create objects in a one-liner.
*/

/// The init-object simplifies calls to rcl.
typedef struct {
  rcl_init_options_t init_options;
  rcl_context_t context;
  rcl_allocator_t * allocator;
  rcl_clock_t clock;
} rcl_ext_init_t;

/**
 *  Initialises RCL and creates an rcl-init object.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (in RCL)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] init_obj a preallocated rcl_ext_init_t
 * \param[in] argc number of args of main
 * \param[in] argv array of arguments of main
 * \param[in] allocator allocator for allocating memory
 * \return `RCL_RET_OK` if RCL was initialized successfully
 * \return `RCL_RET_INVALID_ARGUMENT` if any null pointer as argument
 * \return `RCL_RET_ERROR` in case of failure
 */
rcl_ret_t
rcl_ext_init(
    rcl_ext_init_t * init_obj,
    int argc, 
    char const * const * argv, 
    rcl_allocator_t * allocator);

/**
 *  De-allocates the rcl-init object.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (in RCL)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] init_obj a preallocated rcl_ext_init_t
 * \return `RCL_RET_OK` if operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if any null pointer as argument
 * \return `RCL_RET_ERROR` in case of failure
 */
rcl_ret_t
rcl_ext_init_fini(rcl_ext_init_t * init_obj);

/**
 *  Creates an RCL node.
 * 
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (in this function and in RCL)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] name the name of the node
 * \param[in] namespace the namespace of the node
 * \param[in] init_obj the rcl_ext_init_t object 
 * \return rcl_node_t if successful
 * \return NULL if an error occurred
 */
rcl_node_t *
rcl_ext_create_node(
    const char * name, 
    const char * namespace_, 
    rcl_ext_init_t * init_obj);

/**
 *  Deallocates memory of node.
 *  Result is a NULL pointer to `node`.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (de-allocates memory)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] init_obj the rcl_ext_init_t object
 * \param[inout] node the node to be de-allocated
 * \return `RCL_RET_OK` if successful
 * \return `RCL_RET_INVALID_ARGUMENT` if an argument is a null pointer
 * \return `RCL_RET_ERROR` in case of failure
 */
rcl_ret_t
rcl_ext_node_fini(
  rcl_ext_init_t * init_obj, 
  rcl_node_t * node);

/**
 *  Creates an rcl publisher.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (in this function and RCL)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node the rcl node
 * \param[in] allocator the allocator to be used for memory allocation
 * \param[in] type_support the message data type
 * \param[in] topic_name the name of published topic
 * \return `rcl_publisher_t` if successful
 * \return `NULL` if an error occurred
 */
rcl_publisher_t *
rcl_ext_create_publisher(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name);

/**
 *  Deallocates memory of rcl-publisher.
 *  Result is a NULL pointer to `publisher`.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (de-allocates memory)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] init_obj the rcl_ext_init_t object
 * \param[inout] publisher the rcl publisher to be de-allocated
 * \param[in] node the handle to the node used to create the publisher
 * \return `RCL_RET_OK` if successful
 * \return `RCL_RET_INVALID_ARGUMENT` if an argument is a null pointer
 * \return `RCL_RET_ERROR` in case of failure
 */
rcl_ret_t
rcl_ext_publisher_fini(
  rcl_ext_init_t * init_obj, 
  rcl_publisher_t * publisher, 
  rcl_node_t * node);

/**
 *  Creates an rcl subscription.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (in this function and RCL)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node the rcl node
 * \param[in] allocator the allocator to be used for memory allocation
 * \param[in] type_support the message data type
 * \param[in] topic_name the name of subscribed topic
 * \return `rcl_subscription_t` if successful
 * \return `NULL` if an error occurred
 */
rcl_subscription_t *
rcl_ext_create_subscription(
  rcl_node_t * node,
  rcl_allocator_t * allocator,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name);

/**
 *  Deallocates memory of rcl-subscription.
 *  Result is a NULL pointer to `subscription`.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (de-allocates memory)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] init_obj the rcl_ext_init_t object
 * \param[inout] subscription the rcl subscription to be de-allocated
 * \param[in] node the handle to the node used to create the subscriber
 * \return `RCL_RET_OK` if successful
 * \return `RCL_RET_INVALID_ARGUMENT` if an argument is a null pointer
 * \return `RCL_RET_ERROR` in case of a failure
 */
rcl_ret_t
rcl_ext_subscription_fini(
  rcl_ext_init_t * init_obj, 
  rcl_subscription_t * subscription, 
  rcl_node_t * node);

/**
 *  Creates an rcl timer.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (in this function and RCL)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] init_obj the rcl_ext_init_t object
 * \param[in] timeout_ns the time out in nanoseconds of the timer
 * \param[in] callback the callback of the timer
 * \return `rcl_timer_t` if successful
 * \return `NULL` if an error occurred
 */
rcl_timer_t *
rcl_ext_create_timer(
  rcl_ext_init_t * init_obj,
  const uint64_t timeout_ns,
  const rcl_timer_callback_t callback);

/**
 *  Deallocates memory of an rcl-timer.
 *  Result is a NULL pointer to `timer`.
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (de-allocates memory)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] init_obj the rcl_ext_init_t object
 * \param[inout] timer the timer to be de-allocated
 * \return `RCL_RET_OK` if successful
 * \return `RCL_RET_INVALID_ARGUMENT` if an argument is a null pointer
 * \return `RCL_RET_ERROR` in case of a failure
 */
rcl_ret_t
rcl_ext_timer_fini(
  rcl_ext_init_t * init_obj, 
  rcl_timer_t * timer);

/**
 * macro to print errors
 */
#ifndef PRINT_RCL_ERROR
#define PRINT_RCL_ERROR(rcl_ext, rcl) \
  do { \
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, \
      "[" #rcl_ext "] error in " #rcl ": %s\n", rcutils_get_error_string().str); \
    rcl_reset_error(); \
  } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // RCL__RCL_EXT_H_
