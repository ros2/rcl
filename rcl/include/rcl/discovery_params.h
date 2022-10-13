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

#ifndef RCL__DISCOVERY_PARAMS_H_
#define RCL__DISCOVERY_PARAMS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/types.h"
#include "rcl/visibility_control.h"

#include "rcutils/allocator.h"

#include "rmw/discovery_params.h"

/// Determine how the user wishes to discover other ROS nodes automatically.
/**
 * Checks an environment variable to determine how far automatic discovery should be allowed to
 * propagate: not at all, the local machine only, or however far the automatic discovery mechanism
 * used by the RMW implementation can propagate on the network (e.g. for multicast-based discovery,
 * this will be the local subnet).
 *
 * \param[out] discovery_params Must not be NULL.
 * \return #RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
 * \return #RCL_RET_ERROR if an unexpected error happened, or
 * \return #RCL_RET_OK.
 */
RCL_PUBLIC
rcl_ret_t
rcl_get_discovery_automatic_range(rmw_discovery_params_t * discovery_params);

/// Convert the automatic discovery range value to a string for easy printing.
/**
 * The string buffer passed to this function should be at least 40 bytes. If it is less (as
 * indicated by the size parameter) the stringified enumeration value will be truncated.
 *
 * \param[in] destination The string buffer to print into. Must not be NULL.
 * \param[in] size Must be the size of the destination buffer, including space for a NULL byte.
 * \param[in] discovery_params Must not be NULL.
 * \return #RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
 * \return #RCL_RET_ERROR if an unexpected error happened, or
 * \return #RCL_RET_OK.
 */
RCL_PUBLIC
rcl_ret_t
rcl_automatic_discovery_range_to_string(
  char * destination,
  size_t size,
  rmw_discovery_params_t * discovery_params);

/// Determine how the user wishes to discover other ROS nodes via statically-configured peers.
/**
 * Checks an environment variable to determine the hosts that the user wants to communicate with,
 * in addition to localhost.
 * TODO(gbiggs): Where should the IPs be validated?
 *
 * \param[out] discovery_params Must not be NULL.
 * \return #RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
 * \return #RCL_RET_ERROR if an unexpected error happened, or
 * \return #RCL_RET_OK.
 */
RCL_PUBLIC
rcl_ret_t
rcl_get_discovery_static_peers(
  rmw_discovery_params_t * discovery_params,
  rcutils_allocator_t * allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL__DISCOVERY_PARAMS_H_
