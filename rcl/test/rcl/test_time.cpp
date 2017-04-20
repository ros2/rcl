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

// Tests the rcl_system_time_now() function.
TEST_F(CLASSNAME(TestTimeFixture, RMW_IMPLEMENTATION), test_rcl_system_time_now) {
  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_system_time_now(nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  rcl_time_point_value_t now = 0;
  ret = rcl_system_time_now(&now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_NE(now, 0u);
  // Compare to std::chrono::system_clock time (within a second).
  now = 0;
  ret = rcl_system_time_now(&now);
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = now - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  }
}

// Tests the rcl_steady_time_now() function.
TEST_F(CLASSNAME(TestTimeFixture, RMW_IMPLEMENTATION), test_rcl_steady_time_now) {
  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_steady_time_now(nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  rcl_time_point_value_t now = 0;
  ret = rcl_steady_time_now(&now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_NE(now, 0u);
  // Compare to std::chrono::steady_clock difference of two times (within a second).
  now = 0;
  ret = rcl_steady_time_now(&now);
  std::chrono::steady_clock::time_point now_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // Wait for a little while.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Then take a new timestamp with each and compare.
  rcl_time_point_value_t later;
  ret = rcl_steady_time_now(&later);
  std::chrono::steady_clock::time_point later_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  int64_t steady_diff = later - now;
  int64_t sc_diff =
    std::chrono::duration_cast<std::chrono::nanoseconds>(later_sc - now_sc).count();
  const int k_tolerance_ms = 1;
  EXPECT_LE(llabs(steady_diff - sc_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "steady_clock differs";
}

// Tests the rcl_set_ros_time_override() function.
TEST_F(CLASSNAME(TestTimeFixture, RMW_IMPLEMENTATION), test_rcl_ros_time_set_override) {
  rcl_time_source_t * ros_time_source = rcl_get_default_ros_time_source();
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
  ret = rcl_is_enabled_ros_time_override(ros_time_source, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_is_enabled_ros_time_override(nullptr, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  rcl_time_point_t query_now;
  bool is_enabled;
  ret = rcl_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  ret = rcl_time_point_init(&query_now, ros_time_source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  ret = rcl_time_point_get_now(&query_now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_NE(query_now.nanoseconds, 0u);
  // Compare to std::chrono::system_clock time (within a second).
  ret = rcl_time_point_get_now(&query_now);
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
  ret = rcl_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // set the time point
  ret = rcl_set_ros_time_override(ros_time_source, set_point);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // check still disabled
  ret = rcl_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // get real
  ret = rcl_time_point_get_now(&query_now);
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
  ret = rcl_enable_ros_time_override(ros_time_source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // check enabled
  ret = rcl_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, true);
  // get sim
  ret = rcl_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(query_now.nanoseconds, set_point);
  // disable
  ret = rcl_disable_ros_time_override(ros_time_source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // check disabled
  ret = rcl_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // get real
  ret = rcl_time_point_get_now(&query_now);
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

TEST_F(CLASSNAME(TestTimeFixture, RMW_IMPLEMENTATION), test_rcl_init_for_time_source_and_point) {
  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_ros_time_source_init(nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  // Check for normal operation (not allowed to alloc).
  rcl_time_source_t source;
  ret = rcl_ros_time_source_init(&source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  rcl_time_point_t a_time;
  ret = rcl_time_point_init(&a_time, &source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  rcl_time_point_t default_time;
  ret = rcl_time_point_init(&default_time, nullptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

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
  // ret = rcl_system_time_now(&now);
  // {
  //   std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
  //   auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
  // now_sc.time_since_epoch());
  //   int64_t now_ns_int = now_ns.count();
  //   int64_t now_diff = now.nanoseconds - now_ns_int;
  //   const int k_tolerance_ms = 1000;
  //   EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  // }
  ret = rcl_time_point_fini(&a_time);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  ret = rcl_ros_time_source_fini(&source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), time_source_validation) {
  ASSERT_FALSE(rcl_time_source_valid(NULL));
  rcl_time_source_t uninitialized;
  // Not reliably detectable due to random values.
  // ASSERT_FALSE(rcl_time_source_valid(&uninitialized));
  rcl_ret_t ret;
  ret = rcl_ros_time_source_init(&uninitialized);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), default_time_source_instanciation) {
  rcl_time_source_t * ros_time_source = rcl_get_default_ros_time_source();
  ASSERT_TRUE(rcl_time_source_valid(ros_time_source));
  rcl_time_source_t * steady_time_source = rcl_get_default_steady_time_source();
  ASSERT_TRUE(rcl_time_source_valid(steady_time_source));
  rcl_time_source_t * system_time_source = rcl_get_default_system_time_source();
  ASSERT_TRUE(rcl_time_source_valid(system_time_source));
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), specific_time_source_instantiation) {
  {
    rcl_time_source_t uninitialized_time_source;
    rcl_ret_t ret = rcl_time_source_init(
      RCL_TIME_SOURCE_UNINITIALIZED, &uninitialized_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(uninitialized_time_source.type, RCL_TIME_SOURCE_UNINITIALIZED) <<
      "Expected time source of type RCL_TIME_SOURCE_UNINITIALIZED";
  }
  {
    rcl_time_source_t ros_time_source;
    rcl_ret_t ret = rcl_time_source_init(RCL_ROS_TIME, &ros_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(ros_time_source.type, RCL_ROS_TIME) <<
      "Expected time source of type RCL_ROS_TIME";
    ret = rcl_time_source_fini(&ros_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  }
  {
    rcl_time_source_t system_time_source;
    rcl_ret_t ret = rcl_time_source_init(RCL_SYSTEM_TIME, &system_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(system_time_source.type, RCL_SYSTEM_TIME) <<
      "Expected time source of type RCL_SYSTEM_TIME";
    ret = rcl_time_source_fini(&system_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  }
  {
    rcl_time_source_t steady_time_source;
    rcl_ret_t ret = rcl_time_source_init(RCL_STEADY_TIME, &steady_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
    EXPECT_EQ(steady_time_source.type, RCL_STEADY_TIME) <<
      "Expected time source of type RCL_STEADY_TIME";
    ret = rcl_time_source_fini(&steady_time_source);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  }
}

TEST(CLASSNAME(rcl_time, RMW_IMPLEMENTATION), rcl_time_difference) {
  rcl_ret_t ret;
  rcl_time_point_t a, b;
  ret = rcl_time_point_init(&a, nullptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  ret = rcl_time_point_init(&b, nullptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  a.nanoseconds = 1000;
  b.nanoseconds = 2000;

  rcl_duration_t d;
  ret = rcl_duration_init(&d, nullptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  ret = rcl_difference_times(&a, &b, &d);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_EQ(d.nanoseconds, 1000);
  EXPECT_EQ(d.time_source->type, RCL_ROS_TIME);

  ret = rcl_difference_times(&b, &a, &d);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(d.nanoseconds, -1000);
  EXPECT_EQ(d.time_source->type, RCL_ROS_TIME);

  rcl_time_source_t * system_time_source = rcl_get_default_system_time_source();
  EXPECT_TRUE(system_time_source != nullptr);

  rcl_time_point_t e;
  ret = rcl_time_point_init(&e, system_time_source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
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
  rcl_time_source_t * ros_time_source = rcl_get_default_ros_time_source();
  rcl_time_point_t query_now;
  rcl_ret_t ret;
  rcl_time_point_value_t set_point = 1000000000ull;

  ret = rcl_time_point_init(&query_now, ros_time_source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // set callbacks
  ros_time_source->pre_update = pre_callback;
  ros_time_source->post_update = post_callback;


  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Query it to do something different. no changes expected
  ret = rcl_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Set the time before it's enabled. Should be no callbacks
  ret = rcl_set_ros_time_override(ros_time_source, set_point);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // enable
  ret = rcl_enable_ros_time_override(ros_time_source);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Set the time now that it's enabled, now get callbacks
  ret = rcl_set_ros_time_override(ros_time_source, set_point);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  EXPECT_TRUE(pre_callback_called);
  EXPECT_TRUE(post_callback_called);
}
