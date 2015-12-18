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

#include "rmw/error_handling.h"

/* The error handling in RCL is just an alias to the error handling in RMW. */

typedef rmw_error_state_t rcl_error_state_t;

#define rcl_set_error_state rmw_set_error_state

#define RCL_SET_ERROR_MSG(msg) RMW_SET_ERROR_MSG(msg)

#define rcl_error_is_set rmw_error_is_set

#define rcl_get_error_state rmw_get_error_state

#define rcl_get_error_string rmw_get_error_string

#define rcl_get_error_string_safe rmw_get_error_string_safe

#define rcl_reset_error rmw_reset_error

#endif  // RCL__ERROR_HANDLING_H_
