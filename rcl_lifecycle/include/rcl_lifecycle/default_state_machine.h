// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#ifndef RCL_LIFECYCLE__DEFAULT_STATE_MACHINE_H_
#define RCL_LIFECYCLE__DEFAULT_STATE_MACHINE_H_

#include "rcl/macros.h"
#include "rcl/types.h"

#include "lifecycle_msgs/msg/transition.h"

#include "rcl_lifecycle/data_types.h"
#include "rcl_lifecycle/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern rcl_lifecycle_transition_key_t default_transition_key_configure;
extern rcl_lifecycle_transition_key_t default_transition_key_cleanup;
extern rcl_lifecycle_transition_key_t default_transition_key_activate;
extern rcl_lifecycle_transition_key_t default_transition_key_deactivate;
extern rcl_lifecycle_transition_key_t default_transition_key_shutdown;

extern rcl_lifecycle_transition_key_t default_transition_key_callback_success;
extern rcl_lifecycle_transition_key_t default_transition_key_callback_failure;
extern rcl_lifecycle_transition_key_t default_transition_key_callback_error;

RCL_LIFECYCLE_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lifecycle_init_default_state_machine(
  rcl_lifecycle_state_machine_t * state_machine, const rcl_allocator_t * allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL_LIFECYCLE__DEFAULT_STATE_MACHINE_H_
