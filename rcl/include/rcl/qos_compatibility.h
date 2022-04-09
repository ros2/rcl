// Copyright 2022 Open Source Robotics Foundation, Inc.
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

/// @file

#ifndef RCL__QOS_COMPATIBILITY_H_
#define RCL__QOS_COMPATIBILITY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/graph.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

/// Get a subscription QoS profile that is compatible with discovered endpoints.
/**
 * Adapts the given QoS profile to be compatible with the majority of publishers on a given topic,
 * while maintaining the highest level of service possible.
 *
 * See also \ref rmw_qos_profile_get_most_compatible_for_subscription().
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node The node to use to query the graph.
 * \param[in] topic_name Name of the topic to query for endpoints.
 * \param[out] subscription_qos_profile This QoS profile is modified such that is is compatible
 *   with the majority of publishers on the given topic.
 * \return #RCL_RET_OK if there were no errors, or
 * \return #RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 * \return #RCL_RET_ERROR an unexpected error occurred.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_get_compatible_qos_for_topic_subscription(
  const rcl_node_t * node,
  const char * topic_name,
  rmw_qos_profile_t * subscription_qos_profile);

#ifdef __cplusplus
}
#endif

#endif  // RCL__QOS_COMPATIBILITY_H_
