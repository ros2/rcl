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

#include <gtest/gtest.h>

#include <inttypes.h>

#include <chrono>
#include <thread>

#include "rcl/error_handling.h"
#include "rcl/time.h"

#include "../memory_tools/memory_tools.hpp"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestTimeFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
    set_on_unexpected_malloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED MALLOC";});
    set_on_unexpected_realloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED REALLOC";});
    set_on_unexpected_free_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED FREE";});
    start_memory_checking();
  }

  void TearDown()
  {
    assert_no_malloc_end();
    assert_no_realloc_end();
    assert_no_free_end();
    stop_memory_checking();
    set_on_unexpected_malloc_callback(nullptr);
    set_on_unexpected_realloc_callback(nullptr);
    set_on_unexpected_free_callback(nullptr);
  }
};

// Tests the rcl_set_ros_time_override() function.
TEST_F(CLASSNAME(TestTimeFixture, RMW_IMPLEMENTATION), test_rcl_ros_time_set_override) {
  rcl_clock_t * ros_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  rcl_ret_t retval = rcl_ros_clock_init(ros_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();

  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_set_ros_time_override(nullptr, 0);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  bool result;
  ret = rcl_is_enabled_ros_time_override(nullptr, &result);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_is_enabled_ros_time_override(ros_clock, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_is_enabled_ros_time_override(nullptr, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  rcl_time_point_t query_now;
  bool is_enabled;
  ret = rcl_is_enabled_ros_time_override(ros_clock, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  ret = rcl_clock_get_now(ros_clock, &query_now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_NE(query_now.nanoseconds, 0u);
  // Compare to std::chrono::system_clock time (within a second).
  ret = rcl_clock_get_now(ros_clock, &query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = query_now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "ros_clock differs";
  }
  // Test ros time specific APIs
  rcl_time_point_value_t set_point = 1000000000ull;
  // Check initialized state
  ret = rcl_is_enabled_ros_time_override(ros_clock, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // set the time point
  ret = rcl_set_ros_time_override(ros_clock, set_point);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // check still disabled
  ret = rcl_is_enabled_ros_time_override(ros_clock, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // get real
  ret = rcl_clock_get_now(ros_clock, &query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = query_now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "ros_clock differs";
  }
  // enable
  ret = rcl_enable_ros_time_override(ros_clock);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // check enabled
  ret = rcl_is_enabled_ros_time_override(ros_clock, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, true);
  // get sim
  ret = rcl_clock_get_now(ros_clock, &query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(query_now.nanoseconds, set_point);
  // disable
  ret = rcl_disable_ros_time_override(ros_clock);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // check disabled
  ret = rcl_is_enabled_ros_time_override(ros_clock, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // get real
  ret = rcl_clock_get_now(ros_clock, &query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = query_now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "ros_clock differs";
  }
}

TEST_F(CLASSNAME(TestTimeFixture, RMW_IMPLEMENTATION), test_rcl_init_for_clock_and_point) {
  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_ros_clock_init(nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  // Check for normal operation (not allowed to alloc).
  rcl_clock_t source;
  ret = rcl_ros_clock_init(&source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  rcl_clock_t * ros_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  rcl_ret_t retval = rcl_ros_clock_init(ros_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();

  // assert_no_malloc_begin();
  // assert_no_free_begin();
  // // Do stuff in here
  // assert_no_malloc_end();
  // assert_no_realloc_end();
  // assert_no_free_end();
  // stop_memory_checking();
  // EXPECT_NE(now.nanoseconds, 0u);
  // // Compare to std::chrono::system_clock time (within a second).
  // now = {0};
  // ret = rcutils_system_time_now(&now);
  // {
  //   std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
  //   auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
  // now_sc.time_since_epoch());
  //   int64_t now_ns_int = now_ns.count();
  //   int64_t now_diff = now.nanoseconds - now_ns_int;
  //   const int k_tolerance_ms = 1000;
  //   EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  // }
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), clock_validation) {
  ASSERT_FALSE(rcl_clock_valid(NULL));
  rcl_clock_t uninitialized;
  // Not reliably detectable due to random values.
  // ASSERT_FALSE(rcl_clock_valid(&uninitialized));
  rcl_ret_t ret;
  ret = rcl_ros_clock_init(&uninitialized);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), default_clock_instanciation) {
  rcl_clock_t * ros_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  rcl_ret_t retval = rcl_ros_clock_init(ros_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();
  ASSERT_TRUE(rcl_clock_valid(ros_clock));

  rcl_clock_t * steady_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  retval = rcl_steady_clock_init(steady_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();
  ASSERT_TRUE(rcl_clock_valid(steady_clock));

  rcl_clock_t * system_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  retval = rcl_system_clock_init(system_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();
  ASSERT_TRUE(rcl_clock_valid(system_clock));
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), specific_clock_instantiation) {
  {
    rcl_clock_t uninitialized_clock;
    rcl_ret_t ret = rcl_clock_init(
      RCL_CLOCK_UNINITIALIZED, &uninitialized_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(uninitialized_clock.type, RCL_CLOCK_UNINITIALIZED) <<
      "Expected time source of type RCL_CLOCK_UNINITIALIZED";
  }
  {
    rcl_clock_t ros_clock;
    rcl_ret_t ret = rcl_clock_init(RCL_ROS_TIME, &ros_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(ros_clock.type, RCL_ROS_TIME) <<
      "Expected time source of type RCL_ROS_TIME";
    ret = rcl_clock_fini(&ros_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  }
  {
    rcl_clock_t system_clock;
    rcl_ret_t ret = rcl_clock_init(RCL_SYSTEM_TIME, &system_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(system_clock.type, RCL_SYSTEM_TIME) <<
      "Expected time source of type RCL_SYSTEM_TIME";
    ret = rcl_clock_fini(&system_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  }
  {
    rcl_clock_t steady_clock;
    rcl_ret_t ret = rcl_clock_init(RCL_STEADY_TIME, &steady_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(steady_clock.type, RCL_STEADY_TIME) <<
      "Expected time source of type RCL_STEADY_TIME";
    ret = rcl_clock_fini(&steady_clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  }
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), rcl_time_difference) {
  rcl_clock_t * ros_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  rcl_ret_t retval = rcl_ros_clock_init(ros_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_TRUE(ros_clock != nullptr);
  EXPECT_TRUE(ros_clock->data != nullptr);
  EXPECT_EQ(ros_clock->type, RCL_ROS_TIME);

  rcl_ret_t ret;
  rcl_time_point_t a, b;

  a.nanoseconds = 1000;
  b.nanoseconds = 2000;
  a.clock_type = RCL_ROS_TIME;
  b.clock_type = RCL_ROS_TIME;

  rcl_duration_t d;
  ret = rcl_duration_init(&d, &(ros_clock->type));
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  ret = rcl_difference_times(&a, &b, &d);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_EQ(d.nanoseconds, 1000);
  EXPECT_EQ(d.clock_type, RCL_ROS_TIME);

  ret = rcl_difference_times(&b, &a, &d);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(d.nanoseconds, -1000);
  EXPECT_EQ(d.clock_type, RCL_ROS_TIME);
}

static bool pre_callback_called = false;
static bool post_callback_called = false;

void pre_callback(void)
{
  pre_callback_called = true;
  EXPECT_FALSE(post_callback_called);
}
void post_callback(void)
{
  EXPECT_TRUE(pre_callback_called);
  post_callback_called = true;
}


TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), rcl_time_update_callbacks) {
  rcl_clock_t * ros_clock =
    reinterpret_cast<rcl_clock_t *>(calloc(1, sizeof(rcl_clock_t)));
  rcl_ret_t retval = rcl_ros_clock_init(ros_clock);
  EXPECT_EQ(retval, RCL_RET_OK) << rcl_get_error_string_safe();
  rcl_time_point_t query_now;
  rcl_ret_t ret;
  rcl_time_point_value_t set_point = 1000000000ull;

  // set callbacks
  ros_clock->pre_update = pre_callback;
  ros_clock->post_update = post_callback;


  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Query it to do something different. no changes expected
  ret = rcl_clock_get_now(ros_clock, &query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Set the time before it's enabled. Should be no callbacks
  ret = rcl_set_ros_time_override(ros_clock, set_point);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // enable
  ret = rcl_enable_ros_time_override(ros_clock);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Set the time now that it's enabled, now get callbacks
  ret = rcl_set_ros_time_override(ros_clock, set_point);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_TRUE(pre_callback_called);
  EXPECT_TRUE(post_callback_called);
}
