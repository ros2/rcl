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

#ifndef RCL__EXPAND_AND_REMAP_TOPIC_NAME_H_
#define RCL__EXPAND_AND_REMAP_TOPIC_NAME_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/node.h"
#include "rcl/visibility_control.h"

/// Expand and apply remapping rules in a given name.
/**
 * The input_topic_name, node_name, and node_namespace arguments must all be
 * vaid, null terminated c strings.
 * The output_topic_name will not be assigned a value in the event of an error.
 *
 * The output_topic_name will be null terminated.
 * It is also allocated, so it needs to be deallocated, when it is no longer
 * needed, with the same allocator given to this function.
 * Make sure the `char *` which is passed for the output_topic_name does not
 * point to allocated memory before calling this function, because it will be
 * overwritten and therefore leaked if this function is successful.
 *
 * The input topic name is validated using rcl_validate_topic_name(),
 * but if it fails validation RCL_RET_TOPIC_NAME_INVALID is returned.
 *
 * The input node name is validated using rmw_validate_node_name(),
 * but if it fails validation RCL_RET_NODE_INVALID_NAME is returned.
 *
 * The input node namespace is validated using rmw_validate_namespace(),
 * but if it fails validation RCL_RET_NODE_INVALID_NAMESPACE is returned.
 *
 * /sa rcl_expand_topic_name
 * /sa rcl_remap_topic_name
 * /sa rmw_remap_service_name
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] input_topic_name topic name to be expanded
 * \param[in] node_name name of the node associated with the topic
 * \param[in] node_namespace namespace of the node associated with the topic
 * \param[in] allocator the allocator to be used when creating the output topic
 * \param[in] is_service indicates that a service name should be expanded when `true`.
 *  If not, a topic name is expanded.
 * \param[out] output_topic_name output char * pointer
 * \return `RCL_RET_OK` if the topic name was expanded successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_TOPIC_NAME_INVALID` if the given topic name is invalid, or
 * \return `RCL_RET_NODE_INVALID_NAME` if the name is invalid, or
 * \return `RCL_RET_NODE_INVALID_NAMESPACE` if the namespace_ is invalid, or
 * \return `RCL_RET_UNKNOWN_SUBSTITUTION` for unknown substitutions in name, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_expand_and_remap_name(
  const char * input_topic_name,
  const char * node_name,
  const char * node_namespace,
  rcl_allocator_t allocator,
  bool is_service,
  char ** output_topic_name);

#ifdef __cplusplus
}
#endif

#endif  // RCL__EXPAND_AND_REMAP_TOPIC_NAME_H_
