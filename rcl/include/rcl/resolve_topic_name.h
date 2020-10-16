// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__RESOLVE_TOPIC_NAME_H_
#define RCL__RESOLVE_TOPIC_NAME_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "rcl/allocator.h"
#include "rcl/arguments.h"
#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/node_options.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

/// Expand a given topic name into a fully-qualified topic name and apply remapping rules.
/**
 * \pre The input_topic_name, node_name, and node_namespace arguments must all be
 * valid, null terminated c strings.
 * \post The output_topic_name will not be assigned a value in the event of an error.
 * \post If assigned, the output_topic_name will be null terminated.
 *  Its memory must be freed by the user.
 *
 * This function uses the substitutions provided by rcl_get_default_topic_name_substitutions(),
 * and the additional ones explained in \ref expand_topic_name().
 *
 * The remapping rules are provided by `local_args` and `global_args`,
 * see \ref rcl_remap_topic_name().
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] local_arguments Command line arguments to be used before global arguments, or
 *   if NULL or zero-initialized then only global arguments are used.
 * \param[in] global_arguments Command line arguments to use if no local rules matched, or
 *   `NULL` or zero-initialized to ignore global arguments.
 * \param[in] input_topic_name Topic name to be expanded and remapped.
 * \param[in] node_name Name of the node associated with the topic.
 * \param[in] node_namespace Namespace of the node associated with the topic.
 * \param[in] allocator The allocator to be used when creating the output topic.
 * \param[in] only_expand if `true`, remmapping rules aren't applied.
 * \param[out] output_topic_name Output char * pointer.
 * \return `RCL_RET_OK` if the topic name was expanded successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any of input_topic_name, node_name, node_namespace
 *  or output_topic_name are NULL, or
 * \return `RCL_RET_INVALID_ARGUMENT` if both local_args and global_args are NULL, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_TOPIC_NAME_INVALID` if the given topic name is invalid
 *  (see \ref rcl_validate_topic_name()), or
 * \return `RCL_RET_NODE_INVALID_NAME` if the given node name is invalid
 *  (see \ref rmw_validate_node_name()), or
 * \return `RCL_RET_NODE_INVALID_NAMESPACE` if the given node namespace is invalid
 *  (see \ref rmw_validate_namespace()), or
 * \return `RCL_RET_UNKNOWN_SUBSTITUTION` for unknown substitutions in name, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_resolve_topic_name(
  const rcl_arguments_t * local_args,
  const rcl_arguments_t * global_args,
  const char * input_topic_name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  bool only_expand,
  char ** output_topic_name);

/// Expand a given topic name into a fully-qualified topic name and apply remapping rules.
/**
 * See \ref rcl_resolve_topic_name_with_node() for more info.
 * This overload takes a node pointer instead of
 * `local_args`, `global_args`, `node_name` and `node_namespace`.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node Node object. Its name, namespace, local/global command line arguments are used.
 * \param[in] input_topic_name Topic name to be expanded and remapped.
 * \param[in] allocator The allocator to be used when creating the output topic.
 * \param[in] only_expand if `true`, remmapping rules aren't applied.
 * \param[out] output_topic_name Output char * pointer.
 * \return `RCL_RET_OK` if the topic name was expanded successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any of input_topic_name, node_name, node_namespace
 *  or output_topic_name are NULL, or
 * \return `RCL_RET_INVALID_ARGUMENT` if both local_args and global_args are NULL, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_TOPIC_NAME_INVALID` if the given topic name is invalid
 *  (see \ref rcl_validate_topic_name()), or
 * \return `RCL_RET_NODE_INVALID_NAME` if the given node name is invalid
 *  (see \ref rmw_validate_node_name()), or
 * \return `RCL_RET_NODE_INVALID_NAMESPACE` if the given node namespace is invalid
 *  (see \ref rmw_validate_namespace()), or
 * \return `RCL_RET_UNKNOWN_SUBSTITUTION` for unknown substitutions in name, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_resolve_topic_name_with_node(
  const rcl_node_t * node,
  const char * input_topic_name,
  rcl_allocator_t allocator,
  bool only_expand,
  char ** output_topic_name);

#ifdef __cplusplus
}
#endif

#endif  // RCL__RESOLVE_TOPIC_NAME_H_
