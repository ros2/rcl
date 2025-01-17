// Copyright 2025 cellumation GmbH
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

#ifndef RCL__TIMER_IMPL_H_
#define RCL__TIMER_IMPL_H_

#include <inttypes.h>

#include "rcl/timer.h"
#include "rcutils/stdatomic_helper.h"

struct rcl_timer_impl_s
{
  // The clock providing time.
  rcl_clock_t * clock;
  // The associated context.
  rcl_context_t * context;
  // A guard condition used to wake the associated wait set, either when
  // ROSTime causes the timer to expire or when the timer is reset.
  rcl_guard_condition_t guard_condition;
  // The user supplied callback.
  atomic_uintptr_t callback;
  // optionally user supplied data which will be passed into the callback
  atomic_uintptr_t callback_data;

  // This is a duration in nanoseconds, which is initialized as int64_t
  // to be used for internal time calculation.
  atomic_int_least64_t period;
  // This is a time in nanoseconds since an unspecified time.
  atomic_int_least64_t last_call_time;
  // This is a time in nanoseconds since an unspecified time.
  atomic_int_least64_t next_call_time;
  // Credit for time elapsed before ROS time is activated or deactivated.
  atomic_int_least64_t time_credit;
  // A flag which indicates if the timer is canceled.
  atomic_bool canceled;
  // The user supplied allocator.
  rcl_allocator_t allocator;
  // The user supplied on reset callback data.
  rcl_timer_on_reset_callback_data_t reset_callback_data;
  bool in_use_by_waitset;
};
#endif  // RCL__TIMER_IMPL_H_
