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

#ifndef RCL__ARGUMENTS_H_
#define RCL__ARGUMENTS_H_

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#if __cplusplus
extern "C"
{
#endif

struct rcl_arguments_impl_t;

/// Hold output of parsing command line arguments.
typedef struct rcl_arguments_t
{
  /// Private implementation pointer.
  struct rcl_arguments_impl_t * impl;
} rcl_arguments_t;

/// Return a rcl_node_t struct with members initialized to `NULL`.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_arguments_t
rcl_get_zero_initialized_arguments(void);

/// Parse command line arguments into a structure usable by code.
/**
 * Arguments are parsed without modification.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] argc The number of arguments in argv.
 * \param[in] argv Whe values of the arguments.
 * \param[in] allocator A valid allocator.
 * \param[out] args_output A structure that will contain the result of parsing.
 * \return `RCL_RET_OK` if the arguments were parsed successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parse_arguments(
  int argc,
  const char ** argv,
  rcl_allocator_t allocator,
  rcl_arguments_t * args_output);

/// Return the number of arguments that were not successfully parsed.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] args An arguments structure that has been parsed.
 * \return number of unparsed arguments, or
 * \return -1 if args is `NULL` or zero initialized.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
int
rcl_arguments_get_count_unparsed(
  rcl_arguments_t * args);

/// Return a list of indexes that weren't successfully parsed.
/**
 * Some arguments may not have been successfully parsed, or were not intended as ROS arguments.
 * This function populates an array of indexes to these arguments in the original argv array.
 * Since the first argument is always assumed to be a process name, the list will always contain
 * the index 0.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] args An arguments structure that has been parsed.
 * \param[in] allocator A valid allocator.
 * \param[out] output_unparsed_indices An allocated array of indices into the original argv array.
 *   This array must be deallocated by the caller using the given allocator.
 *   If there are no unparsed args then the output will be set to NULL.
 * \return `RCL_RET_OK` if everything goes correctly, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_arguments_get_unparsed(
  rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int ** output_unparsed_indices);

/// Reclaim resources held inside rcl_arguments_t structure.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] args The structure to be deallocated.
 * \param[in] allocator A valid allocator.
 * \return `RCL_RET_OK` if the memory was successfully freed, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_arguments_fini(
  rcl_arguments_t * args);

/// Get a global instance of command line arguments.
/**
 * \sa rcl_init(int, char **, rcl_allocator_t)
 * \sa rcl_shutdown()
 * This returns parsed command line arguments that were passed to `rcl_init()`.
 * The value returned by this function is undefined before `rcl_init()` is called and after
 * `rcl_shutdown()` is called.
 * The return value must not be finalized.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return a global instance of parsed command line arguments.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_arguments_t *
rcl_get_global_arguments();

#if __cplusplus
}
#endif

#endif  // RCL__ARGUMENTS_H_
