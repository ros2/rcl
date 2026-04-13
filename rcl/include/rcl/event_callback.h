// Copyright 2021 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__EVENT_CALLBACK_H_
#define RCL__EVENT_CALLBACK_H_

#include "rmw/event_callback_type.h"
#include "rcl/visibility_control.h"
#include "rcl/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef rmw_event_callback_t rcl_event_callback_t;

typedef struct rcl_event_callback_with_data_s
{
  rcl_event_callback_t callback;
  const void * user_data;
} rcl_event_callback_with_data_t;

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_event_callback_with_data_t
rcl_get_zero_initialized_event_callback_with_data(void);

#ifdef __cplusplus
}
#endif

#endif  // RCL__EVENT_CALLBACK_H_
