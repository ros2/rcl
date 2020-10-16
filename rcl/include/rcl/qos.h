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

#ifndef RCL__QOS_H_
#define RCL__QOS_H_

#include "rcl/visibility_control.h"
#include "rmw/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Return a string representing the policy value.
/**
 * Returns `NULL` when `value` is `RMW_QOS_POLICY_*_UNKNOWN` or an undefined enum value.
 *
 * The stringified version of the policy value can be obtained doing the follwing conversion:
 * RMW_QOS_POLICY_<POLICY_KIND>_<POLICY_VALUE> -> lower_case(<POLICY_VALUE>)
 *
 * For example, the strigified version of `RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC` is
 * "manual_by_topic" and `RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT` is `best_effort`.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] value qos policy value to be stringified.
 * \return a null terminated string representing the policy value, or
 * \return `NULL` if value is `RMW_QOS_POLICY_*_UNKNOWN` or an undefined enum value.
 */
RCL_PUBLIC
const char *
rcl_qos_durability_policy_to_str(enum rmw_qos_durability_policy_t value);

/// Return a string representing the policy value.
/**
 * See \ref rcl_qos_durability_policy_to_str() for more details.
 */
RCL_PUBLIC
const char *
rcl_qos_history_policy_to_str(enum rmw_qos_history_policy_t value);

/// Return a string representing the policy value.
/**
 * See \ref rcl_qos_durability_policy_to_str() for more details.
 */
RCL_PUBLIC
const char *
rcl_qos_liveliness_policy_to_str(enum rmw_qos_liveliness_policy_t value);

/// Return a string representing the policy value.
/**
 * See \ref rcl_qos_durability_policy_to_str() for more details.
 */
RCL_PUBLIC
const char *
rcl_qos_reliability_policy_to_str(enum rmw_qos_reliability_policy_t value);

/// Return a enum value based on the provided string.
/**
 * Returns the enum value based on the provided string, or
 * `RMW_QOS_POLICY_*_UNKNOWN` when the provided string is unexpected.
 *
 * How policy values are stringified is explained in \ref rcl_qos_durability_policy_to_str.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] str string identifying a qos policy value.
 * \return the policy value represented by the string, or
 * \return `RMW_QOS_POLICY_*_UNKNOWN` if the string doesn't represent any value.
 */
RCL_PUBLIC
enum rmw_qos_durability_policy_t
rcl_qos_durability_policy_from_str(const char * str);

/// Return a enum value based on the provided string.
/**
 * See \ref rcl_qos_durability_policy_from_str() for more details.
 */
RCL_PUBLIC
enum rmw_qos_history_policy_t
rcl_qos_history_policy_from_str(const char * str);

/// Return a enum value based on the provided string.
/**
 * See \ref rcl_qos_durability_policy_from_str() for more details.
 */
RCL_PUBLIC
enum rmw_qos_liveliness_policy_t
rcl_qos_liveliness_policy_from_str(const char * str);


/// Return a enum value based on the provided string.
/**
 * See \ref rcl_qos_durability_policy_from_str() for more details.
 */
RCL_PUBLIC
enum rmw_qos_reliability_policy_t
rcl_qos_reliability_policy_from_str(const char * str);

#ifdef __cplusplus
}
#endif

#endif  // RCL__QOS_H_
