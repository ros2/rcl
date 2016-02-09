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

#include "../memory_tools.hpp"

class TestTimeFixture : public ::testing::Test
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

/* Tests the rcl_system_time_point_now() function.
 */
TEST_F(TestTimeFixture, test_rcl_system_time_point_now) {
  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_system_time_point_now(nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  rcl_system_time_point_t now = {0};
  ret = rcl_system_time_point_now(&now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_NE(now.nanoseconds, 0u);
  // Compare to std::chrono::system_clock time (within a second).
  now = {0};
  ret = rcl_system_time_point_now(&now);
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  }
}

/* Tests the rcl_steady_time_point_now() function.
 */
TEST_F(TestTimeFixture, test_rcl_steady_time_point_now) {
  assert_no_realloc_begin();
  rcl_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcl_steady_time_point_now(nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string_safe();
  rcl_reset_error();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  rcl_steady_time_point_t now = {0};
  ret = rcl_steady_time_point_now(&now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_NE(now.nanoseconds, 0u);
  // Compare to std::chrono::steady_clock difference of two times (within a second).
  now = {0};
  ret = rcl_steady_time_point_now(&now);
  std::chrono::steady_clock::time_point now_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  // Wait for a little while.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Then take a new timestamp with each and compare.
  rcl_steady_time_point_t later;
  ret = rcl_steady_time_point_now(&later);
  std::chrono::steady_clock::time_point later_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  int64_t steady_diff = later.nanoseconds - now.nanoseconds;
  int64_t sc_diff =
    std::chrono::duration_cast<std::chrono::nanoseconds>(later_sc - now_sc).count();
  const int k_tolerance_ms = 1;
  EXPECT_LE(llabs(steady_diff - sc_diff), RCL_MS_TO_NS(k_tolerance_ms)) << "steady_clock differs";
}
