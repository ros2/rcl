// Copyright 2014 Open Source Robotics Foundation, Inc.
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

/** \mainpage rcl: Common functionality for other ROS Client Libraries
 *
 * `rcl` consists of functions and structs (pure C) organized into ROS concepts:
 *
 * - Nodes
 *   - rcl/node.h
 * - Publisher
 *   - rcl/publisher.h
 * - Subscription
 *   - rcl/subscription.h
 * - Service Client
 *   - rcl/client.h
 * - Service Server
 *   - rcl/service.h
 * - Timer
 *   - rcl/timer.h
 *
 * There are some functions for working with "Topics" and "Services":
 *
 * - A function to validate a topic or service name (not necessarily fully qualified):
 *   - rcl_validate_topic_name()
 *   - rcl/validate_topic_name.h
 * - A function to expand a topic or service name to a fully qualified name:
 *   - rcl_expand_topic_name()
 *   - rcl/expand_topic_name.h
 *
 * It also has some machinery that is necessary to wait on and act on these concepts:
 *
 * - Initialization and shutdown management (global for now)
 *   - rcl/rcl.h
 * - Wait sets for waiting on messages/service requests and responses/timers to be ready
 *   - rcl/wait.h
 * - Guard conditions for waking up wait sets asynchronously
 *   - rcl/guard_condition.h
 * - Functions for introspecting and getting notified of changes of the ROS graph
 *   - rcl/graph.h
 *
 * Further still there are some useful abstractions and utilities:
 *
 * - Allocator concept, which can used to control allocation in `rcl_*` functions
 *   - rcl/allocator.h
 * - Concept of ROS Time and access to steady and system wall time
 *   - rcl/time.h
 * - Error handling functionality (C style)
 *   - rcl/error_handling.h
 * - Macros
 *   - rcl/macros.h
 * - Return code types
 *   - rcl/types.h
 * - Macros for controlling symbol visibility on the library
 *   - rcl/visibility_control.h
 */

#ifndef RCL__RCL_H_
#define RCL__RCL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"
#include "rcl/types.h"
#include "rcl/wait.h"
#include "rcl/visibility_control.h"

/// Global initialization of rcl.
/**
 * Unless otherwise noted, this must be called before using any rcl functions.
 *
 * This function can only be run once after starting the program, and once
 * after each call to rcl_shutdown().
 * Repeated calls will fail with `RCL_RET_ALREADY_INIT`.
 *
 * This function can be called any time after rcl_shutdown() is called, but it
 * cannot be called from within a callback being executed by an rcl executor.
 * For example, you can call rcl_shutdown() from within a timer callback, but
 * you have to return from the callback, and therefore exit any in-progress
 * call to a spin function, before calling rcl_init() again.
 *
 * The `argc` and `argv` parameters can contain command line arguments for the
 * program.
 * rcl specific arguments will be parsed and removed, but other arguments will
 * be ignored.
 * If `argc` is `0` and `argv` is `NULL` no parameters will be parsed.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | Yes
 * Lock-Free          | Yes [1]
 * <i>[1] if `atomic_is_lock_free()` returns true for `atomic_uint_least64_t`</i>
 *
 * \param[in] argc number of strings in argv
 * \param[in] argv command line arguments; rcl specific arguments are removed
 * \param[in] allocator rcl_allocator_t used in rcl_init() and rcl_shutdown()
 * \return `RCL_RET_OK` if initialization is successful, or
 * \return `RCL_RET_ALREADY_INIT` if rcl_init has already been called, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_init(int argc, char const * const * argv, rcl_allocator_t allocator);

/// Signal global shutdown of rcl.
/**
 * This function does not have to be called on exit, but does have to be called
 * making a repeat call to rcl_init().
 *
 * This function can only be called once after each call to rcl_init().
 * Repeated calls will fail with RCL_RET_NOT_INIT.
 * This function is not thread safe.
 *
 * When this function is called:
 *  - Any rcl objects created since the last call to rcl_init() are invalidated.
 *  - Calls to rcl_ok() will return `false`.
 *  - Any executors waiting for work (within a call to spin) are interrupted.
 *  - No new work (executing callbacks) will be done in executors.
 *  - Currently running work in executors will be finished.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes [1]
 * Uses Atomics       | Yes
 * Lock-Free          | Yes [2]
 * <i>[1] not thread-safe with rcl_init()</i>
 * <i>[2] if `atomic_is_lock_free()` returns true for `atomic_uint_least64_t`</i>
 *
 * \return `RCL_RET_OK` if the shutdown was completed successfully, or
 * \return `RCL_RET_NOT_INIT` if rcl is not currently initialized, or
 * \return `RCL_RET_ERROR` if an unspecified error occur.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_shutdown(void);

/// Returns an uint64_t number that is unique for the latest rcl_init call.
/**
 * If called before rcl_init or after rcl_shutdown then 0 will be returned.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | Yes
 * Lock-Free          | Yes [1]
 * <i>[1] if `atomic_is_lock_free()` returns true for `atomic_uint_least64_t`</i>
 *
 * \return a unique id specific to this rcl instance, or `0` if not initialized.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
uint64_t
rcl_get_instance_id(void);

/// Return `true` if rcl is currently initialized, otherwise `false`.
/**
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | Yes
 * Lock-Free          | Yes [1]
 * <i>[1] if `atomic_is_lock_free()` returns true for `atomic_uint_least64_t`</i>
 */
RCL_PUBLIC
RCL_WARN_UNUSED
bool
rcl_ok(void);

#ifdef __cplusplus
}
#endif

#endif  // RCL__RCL_H_
