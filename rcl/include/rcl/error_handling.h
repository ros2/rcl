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

#ifndef RCL__ERROR_HANDLING_H_
#define RCL__ERROR_HANDLING_H_

#include "rcutils/error_handling.h"

/// The error handling in RCL is just an alias to the error handling in rcutils.
/**
 * Allocators given to functions in rcl are passed along to the error handling
 * on a "best effort" basis.
 * In some situations, like when NULL is passed for the allocator or something
 * else that contains it, the allocator is not available to be passed to the
 * RCL_SET_ERROR_MSG macro.
 * In these cases, the default allocator rcl_get_default_allocator() is used.
 * Since these are considered fatal errors, as opposed to errors that might
 * occur during normal runtime, is should be okay to use the default allocator.
 */

typedef rcutils_error_state_t rcl_error_state_t;

#define rcl_error_state_copy rcutils_error_state_copy

#define rcl_error_state_fini rcutils_error_state_fini

#define rcl_set_error_state rcutils_set_error_state

#define RCL_SET_ERROR_MSG(msg, allocator) RCUTILS_SET_ERROR_MSG(msg, allocator)

#define rcl_error_is_set rcutils_error_is_set

#define rcl_get_error_state rcutils_get_error_state

#define rcl_get_error_string rcutils_get_error_string

#define rcl_get_error_string_safe rcutils_get_error_string_safe

#define rcl_reset_error rcutils_reset_error

#endif  // RCL__ERROR_HANDLING_H_
