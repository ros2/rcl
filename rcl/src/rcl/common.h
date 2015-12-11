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

#ifndef RCL__COMMON_H_
#define RCL__COMMON_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/error_handling.h"
#include "rcl/types.h"

#define RCL_CHECK_ARGUMENT_FOR_NULL(argument, error_return_type) \
  RCL_CHECK_FOR_NULL_WITH_MSG(argument, #argument " argument is null", return error_return_type)

#define RCL_CHECK_FOR_NULL_WITH_MSG(value, msg, error_statement) if (!value) { \
    RCL_SET_ERROR_MSG(msg); \
    error_statement; \
}

// This value put in env_value is only valid until the next call to rcl_impl_getenv (on Windows).
rcl_ret_t
rcl_impl_getenv(const char * env_name, char ** env_value);

#if __cplusplus
}
#endif

#endif  // RCL__COMMON_H_
