// Copyright 2023 eSOL Co.,Ltd.
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

#include <stddef.h>

#include "rcl/thread_attr.h"
#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/macros.h"
#include "rcl_yaml_param_parser/parser_thread_attr.h"
#include "rcl_yaml_param_parser/types.h"
#include "rcutils/env.h"
#include "rcutils/strdup.h"

const char * const RCL_THREAD_ATTRS_FILE_ENV_VAR = "ROS_THREAD_ATTRS_FILE";
const char * const RCL_THREAD_ATTRS_VALUE_ENV_VAR = "ROS_THREAD_ATTRS_VALUE";

rcl_ret_t
rcl_get_default_thread_attrs_from_value(
  rcl_thread_attrs_t * thread_attrs,
  rcl_allocator_t allocator)
{
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);

  RCL_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR(&allocator, return RCL_RET_INVALID_ARGUMENT);

  const char * ros_thread_attrs_value = NULL;
  const char * get_env_error_str = NULL;

  get_env_error_str = rcutils_get_env(RCL_THREAD_ATTRS_VALUE_ENV_VAR, &ros_thread_attrs_value);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(RCL_THREAD_ATTRS_VALUE_ENV_VAR) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }
  if (ros_thread_attrs_value && strcmp(ros_thread_attrs_value, "") != 0) {
    rcl_ret_t ret = rcl_parse_yaml_thread_attrs_value(ros_thread_attrs_value, thread_attrs);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_get_default_thread_attrs_from_file(
  rcl_thread_attrs_t * thread_attrs,
  rcl_allocator_t allocator)
{
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_ERROR);
  RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(RCL_RET_BAD_ALLOC);

  RCL_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR(&allocator, return RCL_RET_INVALID_ARGUMENT);

  const char * ros_thread_attrs_file = NULL;
  const char * get_env_error_str = NULL;

  get_env_error_str =
    rcutils_get_env(RCL_THREAD_ATTRS_FILE_ENV_VAR, &ros_thread_attrs_file);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(RCL_THREAD_ATTRS_FILE_ENV_VAR) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }
  if (ros_thread_attrs_file && strcmp(ros_thread_attrs_file, "") != 0) {
    rcutils_ret_t ret = rcl_parse_yaml_thread_attrs_file(ros_thread_attrs_file, thread_attrs);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }
  }
  return RCL_RET_OK;
}
