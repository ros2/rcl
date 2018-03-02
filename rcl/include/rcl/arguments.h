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

/// Structure which encapsulates the output of parsing command line arguments
typedef struct rcl_arguments_t
{
  /// Private implementation pointer.
  struct rcl_arguments_impl_t * impl;
} rcl_arguments_t;

// TODO(sloretz) Add API to get non-ROS arguments from rcl_arguments_t

/// Return a rcl_node_t struct with members initialized to `NULL`.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_arguments_t
rcl_get_zero_initialized_arguments(void);

/// Parse command line arguments into a structure usable by code.
/**
 * Arguments are parsed without modification.
 *
 * The first argument is assumed to be the name of the process, and is ignored.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] argc the number of arguments in argv.
 * \param[in] argv the values of the arguments.
 * \param[in] allocator a valid allocator
 * \param[out] args_output a structure that will contain the result of parsing.
 * \return `RCL_RET_OK` if the topic name was remapped or no rules matched
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


/// Deallocate a structure holding parsed command line arguments
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] args the structure to be deallocated
 * \param[in] allocator a valid allocator
 * \return `RCL_RET_OK` if the memory was successfully freed
 * \return `RCL_RET_INVALID_ARGUMENT` if any function arguments are invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_arguments_fini(
  rcl_arguments_t * args,
  rcl_allocator_t allocator);


#if __cplusplus
}
#endif

#endif  // RCL__ARGUMENTS_H_
