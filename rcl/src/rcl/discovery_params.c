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

#include "rcutils/allocator.h"
#include "rcutils/env.h"
#include "rcutils/logging_macros.h"
#include "rcutils/split.h"
#include "rcutils/types/string_array.h"

#include "rcl/error_handling.h"
#include "rcl/types.h"

const char * const RCL_STATIC_PEERS_ENV_VAR = "ROS_STATIC_PEERS";
const char * const RCL_AUTOMATIC_DISCOVERY_RANGE_ENV_VAR = "ROS_AUTOMATIC_DISCOVERY_RANGE";

rcl_ret_t
rcl_get_discovery_automatic_range(rmw_discovery_params_t * discovery_params)
{
  const char * ros_automatic_discovery_range_env_val = NULL;
  const char * get_env_error_str = NULL;

  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCL_CHECK_ARGUMENT_FOR_NULL(discovery_params, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(
    RCL_AUTOMATIC_DISCOVERY_RANGE_ENV_VAR,
    &ros_automatic_discovery_range_env_val);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(RCL_MULTICAST_DISCOVERY_RANGE_ENV_VAR) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }
  if (ros_automatic_discovery_range_env_val == NULL) {
    discovery_params->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST;
  } else if (strcmp(ros_automatic_discovery_range_env_val, "1") == 0) {
    discovery_params->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_OFF;
  } else if (strcmp(ros_automatic_discovery_range_env_val, "2") == 0) {
    discovery_params->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST;
  } else if (strcmp(ros_automatic_discovery_range_env_val, "3") == 0) {
    discovery_params->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET;
  } else {
    RCUTILS_LOG_WARN_NAMED(
      ROS_PACKAGE_NAME,
      "Invalid value %s specified for '" RCUTILS_STRINGIFY(
        RCL_MULTICAST_DISCOVERY_RANGE_ENV_VAR) "'; assuming localhost only",
      ros_automatic_discovery_range_env_val);

    discovery_params->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST;
  }

  return RCL_RET_OK;
}

RCL_PUBLIC
rcl_ret_t
rcl_automatic_discovery_range_to_string(
  char * destination,
  size_t size,
  rmw_discovery_params_t * discovery_params)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(discovery_params, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(destination, RCL_RET_INVALID_ARGUMENT);

  switch (discovery_params->automatic_discovery_range) {
    case RMW_AUTOMATIC_DISCOVERY_RANGE_OFF:
      snprintf(
        destination,
        size,
        "RMW_AUTOMATIC_DISCOVERY_RANGE_OFF (%d)",
        discovery_params->automatic_discovery_range);
      break;
    case RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST:
      snprintf(
        destination,
        size,
        "RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST (%d)",
        discovery_params->automatic_discovery_range);
      break;
    case RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET:
      snprintf(
        destination,
        size,
        "RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET (%d)",
        discovery_params->automatic_discovery_range);
      break;
    case RMW_AUTOMATIC_DISCOVERY_RANGE_DEFAULT:
    default:
      snprintf(
        destination,
        size,
        "RMW_AUTOMATIC_DISCOVERY_RANGE_DEFAULT (%d)",
        discovery_params->automatic_discovery_range);
      break;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_get_discovery_static_peers(
  rmw_discovery_params_t * discovery_params,
  rcutils_allocator_t * allocator)
{
  const char * ros_peers_env_val = NULL;
  const char * get_env_error_str = NULL;

  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCL_CHECK_ARGUMENT_FOR_NULL(discovery_params, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(RCL_STATIC_PEERS_ENV_VAR, &ros_peers_env_val);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(RCL_STATIC_PEERS_ENV_VAR) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }

  discovery_params->static_peers_count = 0;
  if (ros_peers_env_val != NULL) {
    rcutils_string_array_t array = rcutils_get_zero_initialized_string_array();
    rcutils_ret_t split_ret = rcutils_split(ros_peers_env_val, ';', *allocator, &array);
    if (RCUTILS_RET_OK != split_ret) {
      RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
      return RCL_RET_ERROR;
    }

    if (array.size > RMW_DISCOVERY_PARAMS_MAX_PEERS) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Too many peers specified in '" RCUTILS_STRINGIFY(
          RCL_STATIC_PEERS_ENV_VAR) "' (maximum of %d)", RMW_DISCOVERY_PARAMS_MAX_PEERS);
      if (RCUTILS_RET_OK != rcutils_string_array_fini(&array)) {
        // Don't do anything here; we are failing anyway
      }
      return RCL_RET_ERROR;
    }

    for (size_t i = 0; i < array.size; ++i) {
      if (strlen(array.data[i]) > (RMW_DISCOVERY_PARAMS_PEER_MAX_LENGTH - 1)) {
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Static peer %s specified to '" RCUTILS_STRINGIFY(
            RCL_MULTICAST_DISCOVERY_RANGE_ENV_VAR) "' is too long (maximum of %d); skipping",
          array.data[i], RMW_DISCOVERY_PARAMS_PEER_MAX_LENGTH - 1);
        continue;
      }
      strncpy(
        discovery_params->static_peers[discovery_params->static_peers_count], array.data[i],
        RMW_DISCOVERY_PARAMS_PEER_MAX_LENGTH);
      discovery_params->static_peers[discovery_params->static_peers_count][
        RMW_DISCOVERY_PARAMS_PEER_MAX_LENGTH - 1] = '\0';
      discovery_params->static_peers_count++;
    }

    if (RCUTILS_RET_OK != rcutils_string_array_fini(&array)) {
      RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
      // We don't fail here because we got the work done, we will just leak memory
    }
  }

  return RCL_RET_OK;
}
