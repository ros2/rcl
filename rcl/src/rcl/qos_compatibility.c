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

#include "rcl/qos_compatibility.h"

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/types.h"

#include "rmw/qos_profiles.h"

rcl_ret_t
rcl_get_compatible_qos_for_topic_subscription(
  const rcl_node_t * node,
  const char * topic_name,
  rmw_qos_profile_t * subscription_qos_profile)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription_qos_profile, RCL_RET_INVALID_ARGUMENT);

  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  const rcl_allocator_t * allocator = &node_options->allocator;

  rcl_topic_endpoint_info_array_t publishers_info =
    rcl_get_zero_initialized_topic_endpoint_info_array();
  rcl_ret_t ret = rcl_get_publishers_info_by_topic(
    node,
    // TODO(jacobperron): Update rcl_get_publishers_info_by_topic API to take an rcl_allocator_t 
    (rcutils_allocator_t *)allocator,
    topic_name,
    false,
    &publishers_info);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  const size_t number_of_publishers = publishers_info.size;
  if (0u == number_of_publishers) {
    return RCL_RET_OK;
  }

  // Copy QoS profiles to array
  rmw_qos_profile_t * publisher_qos_profiles = (rmw_qos_profile_t *)allocator->allocate(
    number_of_publishers * sizeof(rmw_qos_profile_t),
    allocator->state);
  for (size_t i = 0u; i < number_of_publishers; ++i) {
    publisher_qos_profiles[i] = publishers_info.info_array[i].qos_profile;
  }

  rmw_ret_t rmw_ret = rmw_qos_profile_get_most_compatible_for_subscription(
    publisher_qos_profiles,
    number_of_publishers,
    subscription_qos_profile,
    NULL);

  allocator->deallocate(publisher_qos_profiles, allocator->state);

  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG("unexpected error getting compatible QoS profile for subscription");
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}
