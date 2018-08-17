// Copyright 2017 Open Source Robotics Foundation, Inc.
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
#include <chrono>
#include <thread>

#include "rcl/timer.h"

#include "rcl/rcl.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"

class TestTimerFixture : public ::testing::Test
{
public:
  rcl_node_t * node_ptr;
  void SetUp()
  {
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_publisher_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};

TEST_F(TestTimerFixture, test_two_timers) {
  rcl_ret_t ret;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  rcl_timer_t timer2 = rcl_get_zero_initialized_timer();

  ret = rcl_timer_init(&timer, &clock, RCL_MS_TO_NS(5), nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_timer_init(&timer2, &clock, RCL_MS_TO_NS(20), nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  ret = rcl_wait_set_init(&wait_set, 0, 0, 2, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_wait_set_add_timer(&wait_set, &timer);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_timer(&wait_set, &timer2);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_timer_fini(&timer);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_timer_fini(&timer2);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  ret = rcl_wait(&wait_set, RCL_MS_TO_NS(10));
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  uint8_t nonnull_timers = 0;
  for (uint8_t i = 0; i < wait_set.size_of_timers; i++) {
    if (wait_set.timers[i] != NULL) {
      nonnull_timers++;
    }
  }
  bool is_ready = false;
  ret = rcl_timer_is_ready(&timer, &is_ready);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_TRUE(is_ready);
  ret = rcl_timer_is_ready(&timer2, &is_ready);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_FALSE(is_ready);
  ASSERT_EQ(1, nonnull_timers);

  ret = rcl_clock_fini(&clock);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

TEST_F(TestTimerFixture, test_two_timers_ready_before_timeout) {
  rcl_ret_t ret;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  rcl_timer_t timer2 = rcl_get_zero_initialized_timer();

  ret = rcl_timer_init(&timer, &clock, RCL_MS_TO_NS(5), nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_timer_init(&timer2, &clock, RCL_MS_TO_NS(10), nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  ret = rcl_wait_set_init(&wait_set, 0, 0, 2, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_wait_set_add_timer(&wait_set, &timer);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_timer(&wait_set, &timer2);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_timer_fini(&timer);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_timer_fini(&timer2);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  ret = rcl_wait(&wait_set, RCL_MS_TO_NS(20));
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  uint8_t nonnull_timers = 0;
  for (uint8_t i = 0; i < wait_set.size_of_timers; i++) {
    if (wait_set.timers[i] != NULL) {
      nonnull_timers++;
    }
  }
  bool is_ready = false;
  ret = rcl_timer_is_ready(&timer, &is_ready);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_TRUE(is_ready);
  ret = rcl_timer_is_ready(&timer2, &is_ready);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_FALSE(is_ready);
  ASSERT_EQ(1, nonnull_timers);

  ret = rcl_clock_fini(&clock);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

TEST_F(TestTimerFixture, test_timer_not_ready) {
  rcl_ret_t ret;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();

  ret = rcl_timer_init(&timer, &clock, RCL_MS_TO_NS(5), nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  ret = rcl_wait_set_init(&wait_set, 0, 0, 1, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_wait_set_add_timer(&wait_set, &timer);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_timer_fini(&timer);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  ret = rcl_wait(&wait_set, RCL_MS_TO_NS(1));
  EXPECT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  uint8_t nonnull_timers = 0;
  for (uint8_t i = 0; i < wait_set.size_of_timers; i++) {
    if (wait_set.timers[i] != NULL) {
      nonnull_timers++;
    }
  }
  bool is_ready = false;
  ret = rcl_timer_is_ready(&timer, &is_ready);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_FALSE(is_ready);
  ASSERT_EQ(0, nonnull_timers);

  ret = rcl_clock_fini(&clock);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

TEST_F(TestTimerFixture, test_canceled_timer) {
  rcl_ret_t ret;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();

  ret = rcl_timer_init(&timer, &clock, 500, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_timer_cancel(&timer);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  ret = rcl_wait_set_init(&wait_set, 0, 0, 1, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_wait_set_add_timer(&wait_set, &timer);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_timer_fini(&timer);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  ret = rcl_wait(&wait_set, RCL_MS_TO_NS(1));
  EXPECT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  uint8_t nonnull_timers = 0;
  for (uint8_t i = 0; i < wait_set.size_of_timers; i++) {
    if (wait_set.timers[i] != NULL) {
      nonnull_timers++;
    }
  }
  bool is_ready = false;
  ret = rcl_timer_is_ready(&timer, &is_ready);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_FALSE(is_ready);
  ASSERT_EQ(0, nonnull_timers);

  ret = rcl_clock_fini(&clock);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

TEST_F(TestTimerFixture, test_rostime_time_until_next_call) {
  rcl_ret_t ret;
  const int64_t sec_5 = 5LL * 1000LL * 1000LL * 1000LL;
  int64_t time_until = 0;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_ROS_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string_safe();
  });
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, &clock, sec_5, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_timer_fini(&timer)) << rcl_get_error_string_safe();
  });

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, 1)) << rcl_get_error_string_safe();
  ret = rcl_timer_get_time_until_next_call(&timer, &time_until);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(sec_5 - 1, time_until);

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_5)) << rcl_get_error_string_safe();
  ret = rcl_timer_get_time_until_next_call(&timer, &time_until);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(0, time_until);

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_5 + 1)) <<
    rcl_get_error_string_safe();
  ret = rcl_timer_get_time_until_next_call(&timer, &time_until);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(-1, time_until);
}

TEST_F(TestTimerFixture, test_system_time_to_ros_time) {
  rcl_ret_t ret;
  const int64_t sec_5 = 5LL * 1000LL * 1000LL * 1000LL;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_ROS_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string_safe();
  });

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, &clock, sec_5, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_timer_fini(&timer)) << rcl_get_error_string_safe();
  });

  int64_t time_until_pre = 0;
  ASSERT_EQ(RCL_RET_OK, rcl_timer_get_time_until_next_call(&timer, &time_until_pre)) <<
    rcl_get_error_string_safe();
  ASSERT_LT(0, time_until_pre);
  ASSERT_GT(sec_5, time_until_pre);

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, 1)) << rcl_get_error_string_safe();
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  int64_t time_until = 0;
  ASSERT_EQ(RCL_RET_OK, rcl_timer_get_time_until_next_call(&timer, &time_until)) <<
    rcl_get_error_string_safe();
  // Because of time credit the time until next call should be less than before
  EXPECT_GT(time_until_pre, time_until);
  EXPECT_LT(0, time_until);
}

TEST_F(TestTimerFixture, test_ros_time_to_system_time) {
  rcl_ret_t ret;
  const int64_t sec_5 = 5LL * 1000LL * 1000LL * 1000LL;
  const int64_t sec_1 = 1000LL * 1000LL * 1000LL;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_ROS_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string_safe();
  });

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, 1)) << rcl_get_error_string_safe();
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, &clock, sec_5, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_timer_fini(&timer)) << rcl_get_error_string_safe();
  });

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_1)) << rcl_get_error_string_safe();
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  int64_t time_until_pre = 0;
  ASSERT_EQ(RCL_RET_OK, rcl_timer_get_time_until_next_call(&timer, &time_until_pre)) <<
    rcl_get_error_string_safe();
  ASSERT_EQ(sec_5 - (sec_1 - 1), time_until_pre);

  ASSERT_EQ(RCL_RET_OK, rcl_disable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  int64_t time_until = 0;
  ASSERT_EQ(RCL_RET_OK, rcl_timer_get_time_until_next_call(&timer, &time_until)) <<
    rcl_get_error_string_safe();
  // Because of time credit the time until next call should be less than before
  EXPECT_GT(time_until_pre, time_until);
  EXPECT_LT(0, time_until);
}

TEST_F(TestTimerFixture, test_ros_time_backwards_jump) {
  rcl_ret_t ret;
  const int64_t sec_5 = 5LL * 1000LL * 1000LL * 1000LL;
  const int64_t sec_3 = 3LL * 1000LL * 1000LL * 1000LL;
  const int64_t sec_2 = 2LL * 1000LL * 1000LL * 1000LL;
  const int64_t sec_1 = 1000LL * 1000LL * 1000LL;

  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ret = rcl_clock_init(RCL_ROS_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string_safe();
  });

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_2)) << rcl_get_error_string_safe();
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, &clock, sec_5, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_timer_fini(&timer)) << rcl_get_error_string_safe();
  });

  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_3)) << rcl_get_error_string_safe();
  {
    // Moved forward a little bit, timer should be closer to being ready
    int64_t time_until = 0;
    ASSERT_EQ(RCL_RET_OK, rcl_timer_get_time_until_next_call(&timer, &time_until)) <<
      rcl_get_error_string_safe();
    EXPECT_EQ(sec_5 - (sec_3 - sec_2), time_until);
  }
  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_1)) << rcl_get_error_string_safe();
  {
    // Jumped back before timer was created, so last_call_time should be 1 period
    int64_t time_until = 0;
    ASSERT_EQ(RCL_RET_OK, rcl_timer_get_time_until_next_call(&timer, &time_until)) <<
      rcl_get_error_string_safe();
    EXPECT_EQ(sec_5, time_until);
  }
}

TEST_F(TestTimerFixture, test_ros_time_wakes_wait) {
  const int64_t sec_5 = 5LL * 1000LL * 1000LL * 1000LL;
  const int64_t sec_1 = 1000LL * 1000LL * 1000LL;
  const int64_t sec_1_5 = 1500LL * 1000LL * 1000LL;

  rcl_ret_t ret;
  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ASSERT_EQ(RCL_RET_OK, rcl_clock_init(RCL_ROS_TIME, &clock, &allocator)) <<
    rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string_safe();
  });
  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_1)) << rcl_get_error_string_safe();
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&clock)) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, &clock, sec_1, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_timer_fini(&timer)) << rcl_get_error_string_safe();
  });

  bool timer_was_ready = false;

  std::thread wait_thr([&](void) {
      rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
      ret = rcl_wait_set_init(&wait_set, 0, 0, 1, 0, 0, rcl_get_default_allocator());
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

      ASSERT_EQ(RCL_RET_OK, rcl_wait_set_add_timer(&wait_set, &timer)) <<
        rcl_get_error_string_safe();
      // *INDENT-OFF* (Uncrustify wants strange un-indentation here)
      OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
        EXPECT_EQ(RCL_RET_OK, rcl_wait_set_fini(&wait_set)) <<
          rcl_get_error_string_safe();
      });
      // *INDENT-ON*

      ret = rcl_wait(&wait_set, sec_5);
      // set some flag indicating wait was exited
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      if (wait_set.timers[0] != NULL) {
        timer_was_ready = true;
      }
    });

  // Timer not exceeded, should not wake
  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_1_5)) <<
    rcl_get_error_string_safe();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_FALSE(timer_was_ready);

  // Timer exceeded, should wake
  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&clock, sec_5)) << rcl_get_error_string_safe();
  auto start = std::chrono::steady_clock::now();
  wait_thr.join();
  auto finish = std::chrono::steady_clock::now();
  EXPECT_TRUE(timer_was_ready);
  EXPECT_LT(finish - start, std::chrono::milliseconds(100));
}
