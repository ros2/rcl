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

#ifndef RCL__REMAP_H_
#define RCL__REMAP_H_

#include "rcl/allocator.h"
#include "rcl/arguments.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#if __cplusplus
extern "C"
{
#endif

/// Remaps a topic name based on given rules
/**
 * The supplied topic name must have already been expanded to a fully qualified name.
 * \sa rcl_expand_topic_name()
 *
 * If the node has been given arguments then the remap rules from those will be checked first.
 * If no rules matched, then global remap rules will be checked if the node has not also been
 * instructed to ignore global arguments.
 *
 * Remap rules are checked in the order they were given.
 * Processing stops when a remap rule has been matched or there are no more rules.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (if a remap rule matched)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] local_arguments arguments to be used before global arguments
 * \param[in] use_global_arguments if false then global arguments aren't used at all
 * \param[in] node_name the name of the node whose namespace is being remapped
 * \param[in] input_name a fully qualified name to be remapped
 * \param[in] allocator a valid allocator to use
 * \param[out] output_name either an allocated string with the remapped name,
 *             or `NULL` if no remap rules matched the name
 * \return `RCL_RET_OK` if the topic name was remapped or no rules matched
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_TOPIC_NAME_INVALID` if the given topic name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_remap_topic_name(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  char ** output_name);

/// Remaps a service name based on given rules
/**
 * The supplied service name must have already been expanded to a fully qualified name.
 * \sa rcl_expand_topic_name()
 *
 * If the node has been given arguments then the remap rules from those will be checked first.
 * If no rules matched, then global remap rules will be checked if the node has not also been
 * instructed to ignore global arguments.
 *
 * Remap rules are checked in the order they were given.
 * Processing stops when a remap rule has been matched or there are no more rules.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (if a remap rule matched)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] local_arguments arguments to be used before global arguments
 * \param[in] use_global_arguments if false then global arguments aren't used at all
 * \param[in] node_name the name of the node whose namespace is being remapped
 * \param[in] input_name a fully qualified name to be remapped
 * \param[in] allocator a valid allocator to use
 * \param[out] output_name either an allocated string with the remapped name,
 *             or `NULL` if no remap rules matched the name
 * \return `RCL_RET_OK` if the topic name was remapped or no rules matched
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_TOPIC_NAME_INVALID` if the given topic name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_remap_service_name(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  char ** output_name);

/// Remaps a node name based on given rules
/**
 * If the node has been given arguments its remap rules will be checked first.
 * If no rules matched, then global remap rules will be checked if the node has not also been
 * instructed to ignore global arguments.
 *
 * Node name remap rules are checked in the order they were given.
 * Processing stops when a rule has been matched or there are no more rules.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (if a remap rule matched)
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] local_arguments arguments to be used before global arguments
 * \param[in] use_global_arguments if false then global arguments aren't used at all
 * \param[in] node_name the name of the node whose namespace is being remapped
 * \param[in] allocator a valid allocator to use
 * \param[out] output_name either an allocated string with the remapped name,
 *             or `NULL` if no remap rules matched the name
 * \return `RCL_RET_OK` if the node name was remapped or no rules matched
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_NODE_INVALID_NAME` if the name is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_remap_node_name(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_name);


/// Remaps a namespace based on given rules
/**
 * If local_arguments is given then its remap rules will be checked first.
 * If no rules matched, then global remap rules will be checked if not instructed to ignore them.
 *
 * Namespace remap rules are checked in the order they were given.
 * Processing stops when a rule has been matched or there are no more rules.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes (if a remap rule matched)
 * Thread-Safe        | No (yes if use_global_arguments is false)
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] local_arguments arguments to be used before global arguments
 * \param[in] use_global_arguments if false then global arguments aren't used at all
 * \param[in] node_name the name of the node whose namespace is being remapped
 * \param[in] allocator a valid allocator to be used
 * \param[out] output_namespace either an allocated string with the remapped namespace
 *             or `NULL` if no remap rules matched the name
 * \return `RCL_RET_OK` if the node name was remapped or no rules matched
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_NODE_INVALID_NAMESPACE` if the namespace_ is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_remap_node_namespace(
  rcl_arguments_t * local_arguments,
  bool use_global_arguments,
  const char * node_name,
  rcl_allocator_t allocator,
  char ** output_namespace);


#if __cplusplus
}
#endif

#endif  // RCL__REMAP_H_
