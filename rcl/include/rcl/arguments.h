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

#ifdef __cplusplus
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

#define RCL_LOG_LEVEL_ARG_RULE "__log_level:="
#define RCL_PARAM_FILE_ARG_RULE "__params:="

/// Return a rcl_node_t struct with members initialized to `NULL`.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_arguments_t
rcl_get_zero_initialized_arguments(void);

/// Parse command line arguments into a structure usable by code.
/**
 * If an argument does not appear to be a valid ROS argument then it is skipped
 * and parsing continues with the next argument in `argv`.
 *
 * \sa rcl_get_zero_initialized_arguments()
 * \sa rcl_arguments_get_count_unparsed()
 * \sa rcl_arguments_get_unparsed()
 *
 * Successfully parsed remap rules are stored in the order they were given in `argv`.
 * If given arguments `{"__ns:=/foo", "__ns:=/bar"}` then the namespace used by nodes in this
 * process will be `/foo` and not `/bar`.
 *
 * The default log level will be parsed as `__log_level:=level`, where `level` is a name
 * representing one of the log levels in the `RCUTILS_LOG_SEVERITY` enum, e.g. `info`, `debug`,
 * `warn`, not case sensitive.
 * If multiple of these rules are found, the last one parsed will be used.
 *
 * \sa rcl_remap_topic_name()
 * \sa rcl_remap_service_name()
 * \sa rcl_remap_node_name()
 * \sa rcl_remap_node_namespace()
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
 * \param[in] argv The values of the arguments.
 * \param[in] allocator A valid allocator.
 * \param[out] args_output A structure that will contain the result of parsing.
 *   Must be zero initialized before use.
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
  const char * const argv[],
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
  const rcl_arguments_t * args);

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
  const rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int ** output_unparsed_indices);

/// Return the number of parameter yaml files given in the arguments.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] args An arguments structure that has been parsed.
 * \return number of yaml files, or
 * \return -1 if args is `NULL` or zero initialized.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
int
rcl_arguments_get_param_files_count(
  const rcl_arguments_t * args);


/// Return a list of yaml parameter file paths specified on the command line.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] arguments An arguments structure that has been parsed.
 * \param[in] allocator A valid allocator.
 * \param[out] parameter_files An allocated array of paramter file names.
 *   This array must be deallocated by the caller using the given allocator.
 *   The output is NULL if there were no paramter files.
 * \return `RCL_RET_OK` if everything goes correctly, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_arguments_get_param_files(
  const rcl_arguments_t * arguments,
  rcl_allocator_t allocator,
  char *** parameter_files);

/// Return a list of arguments with ROS-specific arguments removed.
/**
 * Some arguments may not have been intended as ROS arguments.
 * This function populates an array of the aruments in a new argv array.
 * Since the first argument is always assumed to be a process name, the list
 * will always contain the first value from the argument vector.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] argv The argument vector
 * \param[in] args An arguments structure that has been parsed.
 * \param[in] allocator A valid allocator.
 * \param[out] nonros_argc The count of arguments that aren't ROS-specific
 * \param[out] nonros_argv An allocated array of arguments that aren't ROS-specific
 *   This array must be deallocated by the caller using the given allocator.
 *   If there are no non-ROS args, then the output will be set to NULL.
 * \return `RCL_RET_OK` if everything goes correctly, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_remove_ros_arguments(
  char const * const argv[],
  const rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int * nonros_argc,
  const char ** nonros_argv[]);

/// Copy one arguments structure into another.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] args The structure to be copied.
 *  Its allocator is used to copy memory into the new structure.
 * \param[out] args_out A zero-initialized arguments structure to be copied into.
 * \return `RCL_RET_OK` if the structure was copied successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_arguments_copy(
  const rcl_arguments_t * args,
  rcl_arguments_t * args_out);

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
 * \return `RCL_RET_OK` if the memory was successfully freed, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_arguments_fini(
  rcl_arguments_t * args);

#ifdef __cplusplus
}
#endif

#endif  // RCL__ARGUMENTS_H_
