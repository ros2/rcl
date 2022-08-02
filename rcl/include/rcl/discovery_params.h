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
#include "rmw/discovery_params.h"

extern const char * const RCL_PEERS_ENV_VAR;
extern const char * const RCL_MULTICAST_DISCOVERY_ENV_VAR;

/// Determine how the user wishes to discover other ROS nodes.
/**
 * Checks environment variables to determine the hosts that the user wants to communicate with,
 * in addition to localhost, and whether to use multicast discovery or not.
 * TODO(gbiggs): How does "multicast discovery" work for RMW implementations that use a middleware
 * without multicast discovery?
 * TODO(gbiggs): Where should the IPs be validated?
 *
 * \param[out] discovery_params Must not be NULL.
 * \return #RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
 * \return #RCL_RET_ERROR if an unexpected error happened, or
 * \return #RCL_RET_OK.
 */
RCL_PUBLIC
rcl_ret_t
rcl_get_discovery_params(rmw_discovery_params_t * discovery_params);

#ifdef __cplusplus
}
#endif

#endif  // RCL__DISCOVERY_PARAMS_H_
