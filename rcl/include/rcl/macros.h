// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__MACROS_H_
#define RCL__MACROS_H_

#include "rcutils/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Ignored return values of functions with this macro will emit a warning.
#define RCL_WARN_UNUSED RCUTILS_WARN_UNUSED

#define RCL_UNUSED(x) RCUTILS_UNUSED(x)

#define RCL_RET_FROM_RCUTIL_RET(rcl_ret_var, rcutils_expr) \
  { \
    rcutils_ret_t rcutils_ret = rcutils_expr; \
    if (RCUTILS_RET_OK != rcutils_ret) { \
      if (rcutils_error_is_set()) { \
        RCL_SET_ERROR_MSG(rcutils_get_error_string().str); \
      } else { \
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("rcutils_ret_t code: %i", rcutils_ret); \
      } \
    } \
    switch (rcutils_ret) { \
      case RCUTILS_RET_OK: \
        rcl_ret_var = RCL_RET_OK; \
        break; \
      case RCUTILS_RET_ERROR: \
        rcl_ret_var = RCL_RET_ERROR; \
        break; \
      case RCUTILS_RET_BAD_ALLOC: \
        rcl_ret_var = RCL_RET_BAD_ALLOC; \
        break; \
      case RCUTILS_RET_INVALID_ARGUMENT: \
        rcl_ret_var = RCL_RET_INVALID_ARGUMENT; \
        break; \
      case RCUTILS_RET_NOT_INITIALIZED: \
        rcl_ret_var = RCL_RET_NOT_INIT; \
        break; \
      default: \
        rcl_ret_var = RCUTILS_RET_ERROR; \
    } \
  }

#ifdef __cplusplus
}
#endif

#endif  // RCL__MACROS_H_
