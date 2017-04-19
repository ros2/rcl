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

#include <string>

#include "rcl/rcl.h"
#include "rcl/guard_condition.h"

#include "../memory_tools/memory_tools.hpp"
#include "../scope_exit.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestGuardConditionFixture, RMW_IMPLEMENTATION) : public ::testing::Test
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

/* Tests the guard condition accessors, i.e. rcl_guard_condition_get_* functions.
 */
TEST_F(
  CLASSNAME(TestGuardConditionFixture, RMW_IMPLEMENTATION), test_rcl_guard_condition_accessors) {
  stop_memory_checking();
  rcl_ret_t ret;

  // Initialize rcl with rcl_init().
  ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret);
  // Setup automatic rcl_shutdown()
  auto rcl_shutdown_exit = make_scope_exit([]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_shutdown();
    ASSERT_EQ(RCL_RET_OK, ret);
  });

  // Create a zero initialized guard_condition (but not initialized).
  rcl_guard_condition_t zero_guard_condition = rcl_get_zero_initialized_guard_condition();

  // Create a normal guard_condition.
  rcl_guard_condition_options_t default_options = rcl_guard_condition_get_default_options();
  rcl_guard_condition_t guard_condition = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_condition, default_options);
  ASSERT_EQ(RCL_RET_OK, ret);
  // Setup automatic finalization of guard condition.
  auto rcl_guard_condition_exit = make_scope_exit([&guard_condition]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_guard_condition_fini(&guard_condition);
    EXPECT_EQ(RCL_RET_OK, ret);
  });

  // Test rcl_guard_condition_get_options().
  const rcl_guard_condition_options_t * actual_options;
  actual_options = rcl_guard_condition_get_options(nullptr);
  EXPECT_EQ(nullptr, actual_options);
  rcl_reset_error();
  actual_options = rcl_guard_condition_get_options(&zero_guard_condition);
  EXPECT_EQ(nullptr, actual_options);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  actual_options = rcl_guard_condition_get_options(&guard_condition);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_NE(nullptr, actual_options);
  if (actual_options) {
    EXPECT_EQ(default_options.allocator.allocate, actual_options->allocator.allocate);
  }
  // Test rcl_guard_condition_get_rmw_handle().
  rmw_guard_condition_t * gc_handle;
  gc_handle = rcl_guard_condition_get_rmw_handle(nullptr);
  EXPECT_EQ(nullptr, gc_handle);
  rcl_reset_error();
  gc_handle = rcl_guard_condition_get_rmw_handle(&zero_guard_condition);
  EXPECT_EQ(nullptr, gc_handle);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  gc_handle = rcl_guard_condition_get_rmw_handle(&guard_condition);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_NE(nullptr, gc_handle);
}

/* Tests the guard condition life cycle, including rcl_guard_condition_init/fini().
 */
TEST_F(
  CLASSNAME(TestGuardConditionFixture, RMW_IMPLEMENTATION), test_rcl_guard_condition_life_cycle) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_guard_condition_t guard_condition = rcl_get_zero_initialized_guard_condition();
  rcl_guard_condition_options_t default_options = rcl_guard_condition_get_default_options();
  // Trying to init before rcl_init() should fail.
  ret = rcl_guard_condition_init(&guard_condition, default_options);
  ASSERT_EQ(RCL_RET_NOT_INIT, ret) << "Expected RCL_RET_NOT_INIT";
  ASSERT_TRUE(rcl_error_is_set());
  rcl_reset_error();
  // Initialize rcl with rcl_init().
  ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret);
  auto rcl_shutdown_exit = make_scope_exit([]() {
    rcl_ret_t ret = rcl_shutdown();
    ASSERT_EQ(RCL_RET_OK, ret);
  });
  // Try invalid arguments.
  ret = rcl_guard_condition_init(nullptr, default_options);
  ASSERT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << "Expected RCL_RET_INVALID_ARGUMENT";
  ASSERT_TRUE(rcl_error_is_set());
  rcl_reset_error();
  // Try with invalid allocator.
  rcl_guard_condition_options_t options_with_invalid_allocator =
    rcl_guard_condition_get_default_options();
  options_with_invalid_allocator.allocator.allocate = nullptr;
  options_with_invalid_allocator.allocator.deallocate = nullptr;
  options_with_invalid_allocator.allocator.reallocate = nullptr;
  ret = rcl_guard_condition_init(&guard_condition, options_with_invalid_allocator);
  ASSERT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << "Expected RCL_RET_INVALID_ARGUMENT";
  ASSERT_TRUE(rcl_error_is_set());
  rcl_reset_error();
  // Try with failing allocator.
  rcl_guard_condition_options_t options_with_failing_allocator =
    rcl_guard_condition_get_default_options();
  options_with_failing_allocator.allocator.allocate = failing_malloc;
  options_with_failing_allocator.allocator.deallocate = failing_free;
  options_with_failing_allocator.allocator.reallocate = failing_realloc;
  ret = rcl_guard_condition_init(&guard_condition, options_with_failing_allocator);
  ASSERT_EQ(RCL_RET_BAD_ALLOC, ret) << "Expected RCL_RET_BAD_ALLOC";
  // The error will not be set because the allocator will not work.
  // It should, however, print a message to the screen and get the bad alloc ret code.
  // ASSERT_TRUE(rcl_error_is_set());
  // rcl_reset_error();

  // Try fini with invalid arguments.
  ret = rcl_guard_condition_fini(nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << "Expected RCL_RET_INVALID_ARGUMENT";
  ASSERT_TRUE(rcl_error_is_set());
  rcl_reset_error();
  // Try fini with an uninitialized guard_condition.
  ret = rcl_guard_condition_fini(&guard_condition);
  EXPECT_EQ(RCL_RET_OK, ret);
  // Try a normal init and fini.
  ret = rcl_guard_condition_init(&guard_condition, default_options);
  EXPECT_EQ(RCL_RET_OK, ret);
  ret = rcl_guard_condition_fini(&guard_condition);
  EXPECT_EQ(RCL_RET_OK, ret);
  // Try repeated init and fini calls.
  ret = rcl_guard_condition_init(&guard_condition, default_options);
  EXPECT_EQ(RCL_RET_OK, ret);
  ret = rcl_guard_condition_init(&guard_condition, default_options);
  EXPECT_EQ(RCL_RET_ALREADY_INIT, ret) << "Expected RCL_RET_ALREADY_INIT";
  ASSERT_TRUE(rcl_error_is_set());
  rcl_reset_error();
  ret = rcl_guard_condition_fini(&guard_condition);
  EXPECT_EQ(RCL_RET_OK, ret);
  ret = rcl_guard_condition_fini(&guard_condition);
  EXPECT_EQ(RCL_RET_OK, ret);
}
