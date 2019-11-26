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

#ifndef RCL_EXECUTOR__LET_EXECUTOR_H_
#define RCL_EXECUTOR__LET_EXECUTOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdarg.h>

#include <rcl/error_handling.h>
#include <rcutils/logging_macros.h>

#include "rcl_executor/handle.h"

/*! \file rcl_executor.h
    \brief RCL-Executor provides real-time scheduling policies.
*/

/// Container for executor
typedef struct
{
  /// Context (to get information if ROS is up-and-running)
  rcl_context_t * context;
  /// Container for dynamic array for DDS-handles
  rcle_handle_t * handles;
  /// Maximum size of array 'handles'
  size_t max_handles;
  /// Index to the next free element in array handles
  size_t index;
  /// Container to memory allocator for array handles
  const rcl_allocator_t * allocator;
  /// Wait set (is initialized only in the first call of the rcle_let_executor_spin_some function)
  rcl_wait_set_t wait_set;
  /// wait_set initialized
  bool wait_set_initialized;
  /// Statistics objects about total number of subscriptions, timers, clients, services, etc.
  rcle_handle_size_t info;
  /// timeout in nanoseconds for rcl_wait() used in rcle_let_executor_spin_once(). Default 100ms
  uint64_t timeout_ns;
} rcle_let_executor_t;


/**
 *  Return a rcle_let_executor_t struct with pointer members initialized to `NULL`
 *  and member variables to 0.
 */
rcle_let_executor_t
rcle_let_executor_get_zero_initialized_executor(void);
/**
 *  Initializes an executor.
 *  It creates a dynamic array with size \p number_of_handles using the
 *  \p allocator. It assignes the scheduler to \p sched.
 *
 *
 *  * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] e preallocated rcle_let_executor_t
 * \param[in] context RCL context
 * \param[in] number_of_handles size of the handle array
 * \param[in] shed scheduling policy
 * \param[in] allocator allocator for allocating memory
 * \return `RCL_RET_OK` if the executor was initialized successfully
 * \return `RCL_RET_INVALID_ARGUMENT` if any null pointer as argument
 * \return `RCL_RET_ERROR` in case of failure
 */
rcl_ret_t
rcle_let_executor_init(
  rcle_let_executor_t * e,
  rcl_context_t * context,
  const size_t number_of_handles,
  const rcl_allocator_t * allocator);

/**
 *  Set timeout in nanoseconds for rcl_wait (called during {@link rcle_let_executor_spin_once()}).
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param [inout] executor pointer to initialized executor
 * \param [in] timeout_ns  timeout in nanoseconds for the rcl_wait (DDS middleware)
 * \return `RCL_RET_OK` if timeout was set successfully
 * \return `RCL_RET_INVALID_ARGUMENT` if \p executor is a null pointer
 * \return `RCL_RET_ERROR` in an error occured
 */
rcl_ret_t
rcle_let_executor_set_timeout(
  rcle_let_executor_t * executor,
  const uint64_t timeout_ns);


/**
 *  Cleans up executor.
 *  Deallocates dynamic memory of {@link rcle_let_executor_t.handles} and
 *  resets all other values of {@link rcle_let_executor_t}.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param [inout] executor pointer to initialized executor
 * \return `RCL_RET_OK` if reset operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if \p executor is a null pointer
 * \return `RCL_RET_INVALID_ARGUMENT` if \p executor.handles is a null pointer
 * \return `RCL_RET_ERROR` in an error occured (aka executor was not initialized)
 */
rcl_ret_t
rcle_let_executor_fini(rcle_let_executor_t * executor);

/**
 *  Adds a subscription to an executor.
 * * An error is returned, if {@link rcle_let_executor_t.handles} array is full.
 * * The total number_of_subscriptions field of {@link rcle_let_executor_t.info}
 *   is incremented by one.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param [inout] executor pointer to initialized executor
 * \param [in] subscription pointer to an allocated subscription
 * \param [in] msg pointer to an allocated message
 * \param [in] callback    function pointer to a callback
 * \param [in] invocation  invocation type for the callback (ALWAYS or only ON_NEW_DATA)
 * \return `RCL_RET_OK` if add-operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if any parameter is a null pointer
 * \return `RCL_RET_ERROR` if any other error occured
 */
rcl_ret_t
rcle_let_executor_add_subscription(
  rcle_let_executor_t * executor,
  rcl_subscription_t * subscription,
  void * msg,
  rcle_callback_t callback,
  rcle_invocation_t invocation);

/**
 *  Adds a timer to an executor.
 * * An error is returned, if {@link rcle_let_executor_t.handles} array is full.
 * * The total number_of_timers field of {@link rcle_let_executor_t.info} is
 *   incremented by one.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param [inout] executor pointer to initialized executor
 * \param [in] timer pointer to an allocated timer
 * \return `RCL_RET_OK` if add-operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if any parameter is a null pointer
 * \return `RCL_RET_ERROR` if any other error occured
 */
rcl_ret_t
rcle_let_executor_add_timer(
  rcle_let_executor_t * executor,
  rcl_timer_t * timer);

/**
 *  The spin-some function checks one-time for new data from the DDS-queue.
 * * the timeout is defined in {@link rcle_let_executor_t.timeout_ns} and can
 *   be set by calling {@link rcle_let_executor_set_timeout()} function (default value is 100ms)
 *
 * The static-LET executor performs the following actions:
 * * initializes the wait_set with all handle of the array executor->handles
 * * waits for new data from DDS queue with rcl_wait() with timeout executor->timeout_ns
 * * takes all ready handles from the wait_set with rcl_take()
 * * processes all handles in the order, how they were added to the executor with the respective add-functions
 *   by calling respective callback (thus implementing first-read, process, semantic of LET)
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 *
 * \param [inout] executor pointer to initialized executor
 * \param[in] timeout_ns  timeout in millisonds
 * \return `RCL_RET_OK` if spin_once operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if any parameter is a null pointer
 * \return `RCL_RET_TIMEOUT` if rcl_wait() returned timeout (aka no data is avaiable during until the timeout)
 * \return `RCL_RET_ERROR` if any other error occured
 */
rcl_ret_t
rcle_let_executor_spin_some(
  rcle_let_executor_t * executor,
  const uint64_t timeout_ns);

/**
 *  The spin function checks for new data at DDS queue as long as ros context is available.
 *  It calls {@link rcle_let_executor_spin_some()} as long as rcl_is_context_is_valid() returns true.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 *
 * \param [inout] executor pointer to initialized executor
 * \return `RCL_RET_OK` if spin operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if executor is a null pointer
 * \return `RCL_RET_ERROR` if any other error occured
 */
rcl_ret_t
rcle_let_executor_spin(rcle_let_executor_t * executor);


/**
 *  The spin_period function checks for new data at DDS queue as long as ros context is available.
 *  It is called every period nanoseconds.
 *  It calls {@link rcle_let_executor_spin_some()} as long as rcl_is_context_is_valid() returns true.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 *
 * \param [inout] executor pointer to initialized executor
 * \param [in] period in nanoseconds
 * \return `RCL_RET_OK` if spin operation was successful
 * \return `RCL_RET_INVALID_ARGUMENT` if executor is a null pointer
 * \return `RCL_RET_ERROR` if any other error occured
 */
rcl_ret_t
rcle_let_executor_spin_period(rcle_let_executor_t * executor, const uint64_t period);

/**
 * macro to print errors
 */

#define PRINT_RCL_ERROR(rclc, rcl) \
  do { \
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, \
      "[" #rclc "] error in " #rcl ": %s\n", rcutils_get_error_string().str); \
    rcl_reset_error(); \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif  // RCL_EXECUTOR__LET_EXECUTOR_H_
