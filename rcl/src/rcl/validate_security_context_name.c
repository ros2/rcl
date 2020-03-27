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

#include "rcl/validate_security_context_name.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <rcutils/snprintf.h>

#include "rmw/validate_namespace.h"

#include "rcl/error_handling.h"

#include "./common.h"

rcl_ret_t
rcl_validate_security_context_name(
  const char * security_context,
  int * validation_result,
  size_t * invalid_index)
{
  if (!security_context) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  return rmw_validate_namespace_with_size(
    security_context, strlen(security_context), validation_result, invalid_index);
}

rcl_ret_t
rcl_validate_security_context_name_with_size(
  const char * security_context,
  size_t security_context_length,
  int * validation_result,
  size_t * invalid_index)
{
  if (!security_context) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  if (!validation_result) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  int t_validation_result;
  size_t t_invalid_index;
  rmw_ret_t ret = rmw_validate_namespace_with_size(
    security_context, security_context_length, &t_validation_result, &t_invalid_index);
  if (ret != RMW_RET_OK) {
    return rcl_convert_rmw_ret_to_rcl_ret(ret);
  }

  if (t_validation_result != RMW_NAMESPACE_VALID &&
    t_validation_result != RMW_NAMESPACE_INVALID_TOO_LONG)
  {
    switch (t_validation_result) {
      case RMW_NAMESPACE_INVALID_IS_EMPTY_STRING:
        *validation_result = RCL_CONTEXT_NAME_INVALID_IS_EMPTY_STRING;
        break;
      case RMW_NAMESPACE_INVALID_NOT_ABSOLUTE:
        *validation_result = RCL_CONTEXT_NAME_INVALID_NOT_ABSOLUTE;
        break;
      case RMW_NAMESPACE_INVALID_ENDS_WITH_FORWARD_SLASH:
        *validation_result = RCL_CONTEXT_NAME_INVALID_ENDS_WITH_FORWARD_SLASH;
        break;
      case RMW_NAMESPACE_INVALID_CONTAINS_UNALLOWED_CHARACTERS:
        *validation_result = RCL_CONTEXT_NAME_INVALID_CONTAINS_UNALLOWED_CHARACTERS;
        break;
      case RMW_NAMESPACE_INVALID_CONTAINS_REPEATED_FORWARD_SLASH:
        *validation_result = RCL_CONTEXT_NAME_INVALID_CONTAINS_REPEATED_FORWARD_SLASH;
        break;
      case RMW_NAMESPACE_INVALID_NAME_TOKEN_STARTS_WITH_NUMBER:
        *validation_result = RCL_CONTEXT_NAME_INVALID_NAME_TOKEN_STARTS_WITH_NUMBER;
        break;
      default:
        {
          char default_err_msg[256];
          // explicitly not taking return value which is number of bytes written
          int ret = rcutils_snprintf(
            default_err_msg, sizeof(default_err_msg),
            "rcl_validate_security_context_name_with_size(): "
            "unknown rmw_validate_namespace_with_size() result '%d'",
            t_validation_result);
          if (ret < 0) {
            RCL_SET_ERROR_MSG(
              "rcl_validate_security_context_name_with_size(): "
              "rcutils_snprintf() failed");
          } else {
            RCL_SET_ERROR_MSG(default_err_msg);
          }
        }
        return RCL_RET_ERROR;
    }
    if (invalid_index) {
      *invalid_index = t_invalid_index;
    }
    return RCL_RET_OK;
  }

  // security_context might be longer that namespace length, check false positives and correct
  if (t_validation_result == RMW_NAMESPACE_INVALID_TOO_LONG &&
    security_context_length <= RCL_CONTEXT_NAME_MAX_LENGTH)
  {
    *validation_result = RCL_CONTEXT_NAME_VALID;
    return RCL_RET_OK;
  }

  // everything was ok, set result to valid namespace, avoid setting invalid_index, and return
  *validation_result = RCL_CONTEXT_NAME_VALID;
  return RCL_RET_OK;
}

const char *
rcl_security_context_name_validation_result_string(int validation_result)
{
  switch (validation_result) {
    case RCL_CONTEXT_NAME_VALID:
      return NULL;
    case RCL_CONTEXT_NAME_INVALID_IS_EMPTY_STRING:
      return "context name must not be empty";
    case RCL_CONTEXT_NAME_INVALID_NOT_ABSOLUTE:
      return "context name must be absolute, it must lead with a '/'";
    case RCL_CONTEXT_NAME_INVALID_ENDS_WITH_FORWARD_SLASH:
      return "context name must not end with a '/', unless only a '/'";
    case RCL_CONTEXT_NAME_INVALID_CONTAINS_UNALLOWED_CHARACTERS:
      return "context name must not contain characters other than alphanumerics, '_', or '/'";
    case RCL_CONTEXT_NAME_INVALID_CONTAINS_REPEATED_FORWARD_SLASH:
      return "context name must not contain repeated '/'";
    case RCL_CONTEXT_NAME_INVALID_NAME_TOKEN_STARTS_WITH_NUMBER:
      return "context name must not have a token that starts with a number";
    case RCL_CONTEXT_NAME_INVALID_TOO_LONG:
      return "context name should not exceed '"
             RCUTILS_STRINGIFY(RCL_CONTEXT_NAME_MAX_NAME_LENGTH) "'";
    default:
      return "unknown result code for rcl context name validation";
  }
}
