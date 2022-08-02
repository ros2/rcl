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

#include "rcl/discovery_params.h"

#include <stdlib.h>
#include <string.h>

#include "rcutils/env.h"

#include "rcl/error_handling.h"
#include "rcl/types.h"

const char * const RCL_PEERS_ENV_VAR = "ROS_PEERS";
const char * const RCL_MULTICAST_DISCOVERY_ENV_VAR = "ROS_USE_MULTICAST_DISCOVERY";

rcl_ret_t
rcl_get_discovery_params(rmw_discovery_params_t * discovery_params)
{
  const char * ros_peers_env_val = NULL;
  const char * ros_multicast_discovery_env_val = NULL;
  const char * get_env_error_str = NULL;

  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCL_CHECK_ARGUMENT_FOR_NULL(discovery_params, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(RCL_PEERS_ENV_VAR, &ros_peers_env_val);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(RCL_PEERS_ENV_VAR) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }
  discovery_params->peers_count = 0;
  char * state = NULL;
  char * token = strtok_r(ros_peers_env_val, ";", &state);
  while (NULL != token && discovery_params->peers_count < 32) {
    strncpy(discovery_params->peers[discovery_params->peers_count], token, 256);
    discovery_params->peers[discovery_params->peers_count++][255] = '\0';
    token = strtok_r(NULL, ";", &state);
  }

  get_env_error_str = rcutils_get_env(
    RCL_MULTICAST_DISCOVERY_ENV_VAR,
    &ros_multicast_discovery_env_val);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(RCL_MULTICAST_DISCOVERY_ENV_VAR) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }
  discovery_params->use_multicast = (ros_multicast_discovery_env_val != NULL &&
    strcmp(
      ros_multicast_discovery_env_val,
      "1") == 0) ? RMW_MULTICAST_DISCOVERY_ENABLED : RMW_MULTICAST_DISCOVERY_DISABLED;
  return RCL_RET_OK;
}
