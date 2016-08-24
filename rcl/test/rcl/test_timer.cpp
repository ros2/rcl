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

#include <gtest/gtest.h>

#include <inttypes.h>

#include <chrono>
#include <thread>

#include "rcl/error_handling.h"
#include "rcl/timer.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

void timer_callback(rcl_timer_t * timer, uint64_t last_call_time)
{
  (void)timer;
  (void)last_call_time;
}

// Test if the function rcl_timer_get_time_until_next_call handles steady time
// overflows correctly and reports ready correctly.
TEST(CLASSNAME(rcl_timer, RMW_IMPLEMENTATION), get_time_until_next_call_overflow_ready_test) {
  // mimic overflow of steady time
  // without changing the steady time function this uses a custom last_call_time
  rcl_time_point_value_t now = 0;
  rcl_ret_t ret = rcl_steady_time_now(&now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // set a timer which should not be ready yet
  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, now, timer_callback, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // fake last call time to simulate steady time overflow
  rcl_time_point_value_t before_overflow = UINT64_MAX;
  ret = _rcl_timer_set_last_call_time(&timer, before_overflow);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // should return a time_until_next_call <= zero
  int64_t time_until_next_call = INT64_MAX;
  ret = rcl_timer_get_time_until_next_call(&timer, &time_until_next_call);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_LE(time_until_next_call, 0);

  ret = rcl_timer_fini(&timer);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
}

// Test if the function rcl_timer_get_time_until_next_call handles steady time
// overflows correctly and reports not ready correctly.
TEST(CLASSNAME(rcl_timer, RMW_IMPLEMENTATION), get_time_until_next_call_overflow_not_ready_test) {
  // mimic overflow of steady time
  // without changing the steady time function this uses a custom last_call_time
  rcl_time_point_value_t now = 0;
  rcl_ret_t ret = rcl_steady_time_now(&now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // check that not ready is reported correctly
  rcl_time_point_value_t not_ready_by = RCL_MS_TO_NS(100);

  // set a timer which should not be ready yet
  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, now + not_ready_by, timer_callback, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // fake last call time to simulate steady time overflow
  rcl_time_point_value_t before_overflow = UINT64_MAX;
  ret = _rcl_timer_set_last_call_time(&timer, before_overflow);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // should return a time_until_next_call around not_ready_by
  int64_t time_until_next_call = INT64_MAX;
  ret = rcl_timer_get_time_until_next_call(&timer, &time_until_next_call);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_GE(time_until_next_call, not_ready_by / 2);
  EXPECT_LE(time_until_next_call, not_ready_by * 2);

  ret = rcl_timer_fini(&timer);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
}
