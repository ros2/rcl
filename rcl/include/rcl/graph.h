// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__GRAPH_H_
#define RCL__GRAPH_H_

#if __cplusplus
extern "C"
{
#endif

#include <rmw/rmw.h>
#include <rmw/types.h>

#include "rosidl_generator_c/service_type_support.h"

#include "rcl/macros.h"
#include "rcl/client.h"
#include "rcl/node.h"
#include "rcl/visibility_control.h"

typedef rmw_topic_names_and_types_t rcl_topic_names_and_types_t;


/// Return a rcl_topic_names_and_types_t struct with members initialized to NULL.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_topic_names_and_types_t
rcl_get_zero_initialized_topic_names_and_types(void);

/// Return a list of topic names and their types.
/* This function returns a list of topic names in the ROS graph and their types.
 *
 * The node parameter must not be NULL, and must point to a valid node.
 *
 * The topic_names_and_types parameter must be allocated and zero initialized.
 * The topic_names_and_types is the output for this function, and contains
 * allocated memory.
 * Therefore, it should be passed to rcl_destroy_topic_names_and_types() when
 * it is no longer needed.
 * Failing to do so will result in leaked memory.
 *
 * This function does manipulate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[out] topic_names_and_types list of topic names and their types
 * \return RCL_RET_OK if the query was successful, or
 *         RCL_RET_NODE_INVALID if the node is invalid, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_get_topic_names_and_types(
  const rcl_node_t * node,
  rcl_topic_names_and_types_t * topic_names_and_types);

/// Destroy a struct which was previously given to rcl_get_topic_names_and_types.
/* The topic_names_and_types parameter must not be NULL, and must point to an
 * already allocated rcl_topic_names_and_types_t struct that was previously
 * passed to a successful rcl_get_topic_names_and_types() call.
 *
 * This function does manipulate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \param[inout] topic_names_and_types struct to be destroyed
 * \return RCL_RET_OK if successful, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_destroy_topic_names_and_types(
  rcl_topic_names_and_types_t * topic_names_and_types);

/// Return the number of publishers on a given topic.
/* This function returns the number of publishers on a given topic.
 *
 * The node parameter must not be NULL, and must point to a valid node.
 *
 * The topic_name parameter must not be NULL, and must not be an empty string.
 * It should also follow the topic name rules.
 * \TODO(wjwwood): link to the topic name rules.
 *
 * The count parameter must not be NULL, and must point to a valid bool.
 * The count parameter is the output for this function and will be set.
 *
 * This function may manipulate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] topic_name the name of the topic in question
 * \param[out] count number of publishers on the given topic
 * \return RCL_RET_OK if the query was successful, or
 *         RCL_RET_NODE_INVALID if the node is invalid, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_count_publishers(
  const rcl_node_t * node,
  const char * topic_name,
  size_t * count);

/// Return the number of subscriptions on a given topic.
/* This function returns the number of subscriptions on a given topic.
 *
 * The node parameter must not be NULL, and must point to a valid node.
 *
 * The topic_name parameter must not be NULL, and must not be an empty string.
 * It should also follow the topic name rules.
 * \TODO(wjwwood): link to the topic name rules.
 *
 * The count parameter must not be NULL, and must point to a valid bool.
 * The count parameter is the output for this function and will be set.
 *
 * This function may manipulate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] topic_name the name of the topic in question
 * \param[out] count number of subscriptions on the given topic
 * \return RCL_RET_OK if the query was successful, or
 *         RCL_RET_NODE_INVALID if the node is invalid, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_count_subscribers(
  const rcl_node_t * node,
  const char * topic_name,
  size_t * count);

/// Check if a service server is available for the given service client.
/* This function will return true for is_available if there is a service server
 * available for the given client.
 *
 * The node parameter must not be NULL, and must point to a valid node.
 *
 * The client parameter must not be NULL, and must point to a valid client.
 *
 * The given client and node must match, i.e. the client must have been created
 * using the given node.
 *
 * The is_available parameter must not be NULL, and must point a bool variable.
 * The result of the check will be stored in the is_available parameter.
 *
 * This function does manipulate heap memory.
 * This function is not thread-safe.
 * This function is lock-free.
 *
 * \param[in] node the handle to the node being used to query the ROS graph
 * \param[in] client the handle to the service client being queried
 * \param[out] is_available set to true if there is a service server available, else false
 * \return RCL_RET_OK if the check was made successfully (regardless of the service readiness), or
 *         RCL_RET_NODE_INVALID if the node is invalid, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_server_is_available(
  const rcl_node_t * node,
  const rcl_client_t * client,
  bool * is_available);

#if __cplusplus
}
#endif

#endif  // RCL__GRAPH_H_
