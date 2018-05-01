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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/types.h"

/// Retrieve the value of the given environment variable if it exists, or "".
/* The returned cstring is only valid until the next time this function is
 * called, because it is a direct pointer to the static storage.
 * The returned value char * variable should never have free() called on it.
 * If the environment variable is not set, an empty string will be returned.
 *
 * Environment variables will be truncated at 2048 characters on Windows.
 *
 * This function does not allocate heap memory, but the system calls might.
 * This function is not thread-safe.
 * This function is not lock-free.
 *
 * \param[in] env_name the name of the environment variable
 * \param[out] env_value pointer to the value cstring, or "" if unset
 * \return RCL_RET_OK if the value is retrieved successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR an unspecified error occur.
 */
rcl_ret_t
rcl_impl_getenv(const char * env_name, const char ** env_value);

/// Convenience function for converting common rmw_ret_t return codes to rcl.
rcl_ret_t
rcl_convert_rmw_ret_to_rcl_ret(rmw_ret_t rmw_ret);

#ifdef __cplusplus
}
#endif

#endif  // RCL__COMMON_H_
