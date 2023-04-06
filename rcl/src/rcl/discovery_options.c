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

#include "rcl/discovery_options.h"

#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/env.h"
#include "rcutils/logging_macros.h"
#include "rcutils/snprintf.h"
#include "rcutils/split.h"
#include "rcutils/types/string_array.h"

#include "rcl/error_handling.h"
#include "rcl/types.h"

#include "rmw/error_handling.h"

#include "./common.h"

static const char * const RCL_STATIC_PEERS_ENV_VAR = "ROS_STATIC_PEERS";
static const char * const RCL_AUTOMATIC_DISCOVERY_RANGE_ENV_VAR = "ROS_AUTOMATIC_DISCOVERY_RANGE";

#define GET_RMW_DISCOVERY_RANGE(x) \
  _GET_DEFAULT_DISCOVERY_RANGE(x)
#define _GET_DEFAULT_DISCOVERY_RANGE(x) \
  RMW_AUTOMATIC_DISCOVERY_RANGE_ ## x

rcl_ret_t
rcl_get_automatic_discovery_range(rmw_discovery_options_t * discovery_options)
{
  const char * ros_automatic_discovery_range_env_val = NULL;
  const char * get_env_error_str = NULL;

  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCL_CHECK_ARGUMENT_FOR_NULL(discovery_options, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(
    RCL_AUTOMATIC_DISCOVERY_RANGE_ENV_VAR,
    &ros_automatic_discovery_range_env_val);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '%s': %s", RCL_AUTOMATIC_DISCOVERY_RANGE_ENV_VAR,
      get_env_error_str);
    return RCL_RET_ERROR;
  }
  if (strcmp(ros_automatic_discovery_range_env_val, "") == 0) {
#ifdef RCL_DEFAULT_DISCOVERY_RANGE
    discovery_options->automatic_discovery_range =
      GET_RMW_DISCOVERY_RANGE(RCL_DEFAULT_DISCOVERY_RANGE);
#else
    discovery_options->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET;
#endif
  } else if (strcmp(ros_automatic_discovery_range_env_val, "OFF") == 0) {
    discovery_options->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_OFF;
  } else if (strcmp(ros_automatic_discovery_range_env_val, "LOCALHOST") == 0) {
    discovery_options->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST;
  } else if (strcmp(ros_automatic_discovery_range_env_val, "SUBNET") == 0) {
    discovery_options->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET;
  } else if (strcmp(ros_automatic_discovery_range_env_val, "SYSTEM_DEFAULT") == 0) {
    discovery_options->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_SYSTEM_DEFAULT;
  } else {
    RCUTILS_LOG_WARN_NAMED(
      ROS_PACKAGE_NAME,
      "Invalid value '%s' specified for '%s', assuming localhost only",
      ros_automatic_discovery_range_env_val,
      RCL_AUTOMATIC_DISCOVERY_RANGE_ENV_VAR);

    discovery_options->automatic_discovery_range = RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST;
  }

  return RCL_RET_OK;
}

RCL_PUBLIC
const char *
rcl_automatic_discovery_range_to_string(rmw_automatic_discovery_range_t automatic_discovery_range)
{
  switch (automatic_discovery_range) {
    case RMW_AUTOMATIC_DISCOVERY_RANGE_NOT_SET:
      return "RMW_AUTOMATIC_DISCOVERY_RANGE_NOT_SET";
    case RMW_AUTOMATIC_DISCOVERY_RANGE_OFF:
      return "RMW_AUTOMATIC_DISCOVERY_RANGE_OFF";
    case RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST:
      return "RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST";
    case RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET:
      return "RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET";
    case RMW_AUTOMATIC_DISCOVERY_RANGE_SYSTEM_DEFAULT:
      return "RMW_AUTOMATIC_DISCOVERY_RANGE_SYSTEM_DEFAULT";
    default:
      return NULL;
  }
}

rcl_ret_t
rcl_get_discovery_static_peers(
  rmw_discovery_options_t * discovery_options,
  rcutils_allocator_t * allocator)
{
  const char * ros_peers_env_val = NULL;
  const char * get_env_error_str = NULL;

  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCL_CHECK_ARGUMENT_FOR_NULL(discovery_options, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(RCL_STATIC_PEERS_ENV_VAR, &ros_peers_env_val);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting environment variable '%s': %s",
      RCL_STATIC_PEERS_ENV_VAR, get_env_error_str);
    return RCL_RET_ERROR;
  }

  // The value of the env var should be at least "", even when not set.
  if (NULL == ros_peers_env_val) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Environment variable value unexpectedly NULL when checking '%s'",
      RCL_STATIC_PEERS_ENV_VAR);
    return RCL_RET_ERROR;
  }

  rcutils_string_array_t array = rcutils_get_zero_initialized_string_array();
  rcutils_ret_t split_ret = rcutils_split(ros_peers_env_val, ';', *allocator, &array);
  if (RCUTILS_RET_OK != split_ret) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
    return RCL_RET_ERROR;
  }

  rmw_ret_t rmw_ret = rmw_discovery_options_init(discovery_options, array.size, allocator);
  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }

  for (size_t i = 0; i < array.size; ++i) {
    if (strlen(array.data[i]) > (RMW_DISCOVERY_OPTIONS_STATIC_PEERS_MAX_LENGTH - 1)) {
      RCUTILS_LOG_WARN_NAMED(
        ROS_PACKAGE_NAME,
        "Static peer %s specified to '%s' is too long (maximum of %d); skipping",
        array.data[i], RCL_STATIC_PEERS_ENV_VAR,
        RMW_DISCOVERY_OPTIONS_STATIC_PEERS_MAX_LENGTH - 1);
      continue;
    }
#ifdef _WIN32
    strncpy_s(
      discovery_options->static_peers[i].peer_address,
      RMW_DISCOVERY_OPTIONS_STATIC_PEERS_MAX_LENGTH,
      array.data[i],
      RMW_DISCOVERY_OPTIONS_STATIC_PEERS_MAX_LENGTH);
#else
    strncpy(
      discovery_options->static_peers[i].peer_address,
      array.data[i],
      RMW_DISCOVERY_OPTIONS_STATIC_PEERS_MAX_LENGTH);
    discovery_options->static_peers[i].peer_address[
      RMW_DISCOVERY_OPTIONS_STATIC_PEERS_MAX_LENGTH - 1] = '\0';
#endif
  }

  if (RCUTILS_RET_OK != rcutils_string_array_fini(&array)) {
    RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
    return RCL_RET_ERROR;
  }

  return RCL_RET_OK;
}
