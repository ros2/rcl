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

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

#include "rcl/rcl.h"
#include "rcl/error_handling.h"
#include "rcl/wait.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif


#define TOLERANCE 100000  // clock error tolerance in nanoseconds

TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), test_resize_to_zero) {
  // Initialize a waitset with a subscription and then resize it to zero.
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_wait_set_resize_subscriptions(&wait_set, 0);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  EXPECT_EQ(wait_set.size_of_subscriptions, 0);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

// Some test cases for the waitset
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), finite_timeout) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 0, 1, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  int64_t timeout = 1000000;  // nanoseconds
  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  // Check time
  int64_t diff = (after_sc - before_sc).count();
  EXPECT_LE(diff, timeout + TOLERANCE);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

// give a negative timeout
// check that a timer overrides a negative timeout
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), negative_timeout) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 1, 1, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  // Add a dummy guard condition to avoid an error
  rcl_guard_condition_t guard_cond = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_cond, rcl_guard_condition_get_default_options());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_guard_condition(&wait_set, &guard_cond);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();


  // TODO Make sure timer assumption fits with original rcl timer assumptions,
  // maybe by duplication test separately.
  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  rcl_timer_callback_t callback;
  ret = rcl_timer_init(&timer, 1000000, callback, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_timer(&wait_set, &timer);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  int64_t timeout = -1;
  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  // We expect a timeout here (timer value reached)
  ASSERT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  // Check time
  int64_t diff = (after_sc - before_sc).count();
  EXPECT_LE(diff, 1000000 + TOLERANCE);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_timer_fini(&timer);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

// give zero timeout
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), zero_timeout) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 1, 1, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  // Add a dummy guard condition to avoid an error
  rcl_guard_condition_t guard_cond = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_cond, rcl_guard_condition_get_default_options());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_guard_condition(&wait_set, &guard_cond);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();


  // Time spent during wait should be negligible.
  int64_t timeout = 0;
  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  // We expect a timeout here (timer value reached)
  ASSERT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  int64_t diff = (after_sc - before_sc).count();
  EXPECT_LE(diff, TOLERANCE);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

// Check the interaction of a guard condition and a negative timeout by
// triggering a guard condition in a separate thread
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), guard_condition) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 1, 0, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_guard_condition_t guard_cond = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_cond, rcl_guard_condition_get_default_options());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_guard_condition(&wait_set, &guard_cond);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  std::promise<rcl_ret_t> p;

  int64_t timeout = -1;

  std::chrono::nanoseconds trigger_diff;
  std::thread trigger_thread([&p, &guard_cond, &trigger_diff]() {
      std::chrono::steady_clock::time_point before_trigger = std::chrono::steady_clock::now();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      rcl_ret_t ret = rcl_trigger_guard_condition(&guard_cond);
      std::chrono::steady_clock::time_point after_trigger = std::chrono::steady_clock::now();
      trigger_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
        after_trigger - before_trigger);
      p.set_value(ret);
    }
  );
  auto f = p.get_future();

  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  int64_t diff = (after_sc - before_sc).count();
  EXPECT_EQ(RCL_RET_OK, f.get());

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  trigger_thread.join();
  EXPECT_LE(std::abs(diff - trigger_diff.count()), TOLERANCE);
  ret = rcl_guard_condition_fini(&guard_cond);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}
