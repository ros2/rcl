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

#include "rcl/rcl.h"

#include "./failing_allocator_functions.hpp"
#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"
#include "rcl/error_handling.h"
#include "rcutils/snprintf.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

using osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_realloc;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_calloc;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_free;

class CLASSNAME (TestRCLFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
    osrf_testing_tools_cpp::memory_tools::initialize();
    on_unexpected_malloc([]() {ADD_FAILURE() << "UNEXPECTED MALLOC";});
    on_unexpected_realloc([]() {ADD_FAILURE() << "UNEXPECTED REALLOC";});
    on_unexpected_free([]() {ADD_FAILURE() << "UNEXPECTED FREE";});
  }

  void TearDown()
  {
    osrf_testing_tools_cpp::memory_tools::uninitialize();
  }
};

struct FakeTestArgv
{
  FakeTestArgv()
  : allocator(rcutils_get_default_allocator()), argc(2)
  {
    this->argv =
      reinterpret_cast<char **>(allocator.allocate(2 * sizeof(char *), allocator.state));
    if (!this->argv) {
      throw std::bad_alloc();
    }
    this->argv[0] = rcutils_format_string(allocator, "%s", "foo");
    if (!this->argv[0]) {
      allocator.deallocate(this->argv, allocator.state);
      throw std::bad_alloc();
    }
    this->argv[1] = rcutils_format_string(allocator, "%s", "bar");
    if (!this->argv[1]) {
      allocator.deallocate(this->argv[0], allocator.state);
      allocator.deallocate(this->argv, allocator.state);
      throw std::bad_alloc();
    }
  }

  ~FakeTestArgv()
  {
    if (this->argv) {
      if (this->argc > 0) {
        size_t unsigned_argc = this->argc;
        for (size_t i = 0; i < unsigned_argc; --i) {
          allocator.deallocate(this->argv[i], allocator.state);
        }
      }
    }
    allocator.deallocate(this->argv, allocator.state);
  }

  rcutils_allocator_t allocator;
  int argc;
  std::string foo;
  std::string bar;
  char ** argv;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

private:
  FakeTestArgv(const FakeTestArgv &) = delete;
};

/* Tests the rcl_init(), rcl_ok(), and rcl_shutdown() functions.
 */
TEST_F(CLASSNAME(TestRCLFixture, RMW_IMPLEMENTATION), test_rcl_init_and_ok_and_shutdown) {
  rcl_ret_t ret;
  // A shutdown before any init has been called should fail.
  ret = rcl_shutdown();
  EXPECT_EQ(RCL_RET_NOT_INIT, ret);
  rcl_reset_error();
  ASSERT_FALSE(rcl_ok());
  // If argc is not 0, but argv is, it should be an invalid argument.
  ret = rcl_init(42, nullptr, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ASSERT_FALSE(rcl_ok());
  // If either the allocate or deallocate function pointers are not set, it should be invalid arg.
  rcl_allocator_t invalid_allocator = rcl_get_default_allocator();
  invalid_allocator.allocate = nullptr;
  ret = rcl_init(0, nullptr, invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ASSERT_FALSE(rcl_ok());
  invalid_allocator.allocate = rcl_get_default_allocator().allocate;
  invalid_allocator.deallocate = nullptr;
  ret = rcl_init(0, nullptr, invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ASSERT_FALSE(rcl_ok());
  // If the malloc call fails (with some valid arguments to copy), it should be a bad alloc.
  {
    FakeTestArgv test_args;
    rcl_allocator_t failing_allocator = rcl_get_default_allocator();
    failing_allocator.allocate = failing_malloc;
    failing_allocator.reallocate = failing_realloc;
    failing_allocator.zero_allocate = failing_calloc;
    ret = rcl_init(test_args.argc, test_args.argv, failing_allocator);
    EXPECT_EQ(RCL_RET_BAD_ALLOC, ret);
    rcl_reset_error();
    ASSERT_FALSE(rcl_ok());
  }
  // If argc is 0 and argv is nullptr and the allocator is valid, it should succeed.
  ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_TRUE(rcl_ok());
  // Then shutdown should work.
  ret = rcl_shutdown();
  EXPECT_EQ(ret, RCL_RET_OK);
  ASSERT_FALSE(rcl_ok());
  // Valid argc/argv values and a valid allocator should succeed.
  {
    FakeTestArgv test_args;
    ret = rcl_init(test_args.argc, test_args.argv, rcl_get_default_allocator());
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_TRUE(rcl_ok());
  }
  // Then shutdown should work.
  ret = rcl_shutdown();
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_FALSE(rcl_ok());
  // A repeat call to shutdown should not work.
  ret = rcl_shutdown();
  EXPECT_EQ(RCL_RET_NOT_INIT, ret);
  rcl_reset_error();
  ASSERT_FALSE(rcl_ok());
  // Repeat, but valid, calls to rcl_init() should fail.
  {
    FakeTestArgv test_args;
    ret = rcl_init(test_args.argc, test_args.argv, rcl_get_default_allocator());
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_TRUE(rcl_ok());
    ret = rcl_init(test_args.argc, test_args.argv, rcl_get_default_allocator());
    EXPECT_EQ(RCL_RET_ALREADY_INIT, ret);
    rcl_reset_error();
    ASSERT_TRUE(rcl_ok());
  }
  // But shutdown should still work.
  ret = rcl_shutdown();
  EXPECT_EQ(ret, RCL_RET_OK);
  ASSERT_FALSE(rcl_ok());
}

/* Tests the rcl_get_instance_id() and rcl_ok() functions.
 */
TEST_F(CLASSNAME(TestRCLFixture, RMW_IMPLEMENTATION), test_rcl_get_instance_id_and_ok) {
  rcl_ret_t ret;
  // Instance id should be 0 before rcl_init().
  EXPECT_EQ(0u, rcl_get_instance_id());
  ASSERT_FALSE(rcl_ok());
  // It should still return 0 after an invalid init.
  ret = rcl_init(1, nullptr, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  EXPECT_EQ(0u, rcl_get_instance_id());
  ASSERT_FALSE(rcl_ok());
  // A non-zero instance id should be returned after a valid init.
  {
    FakeTestArgv test_args;
    ret = rcl_init(test_args.argc, test_args.argv, rcl_get_default_allocator());
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_TRUE(rcl_ok());
  }
  // And it should be allocation free.
  uint64_t first_instance_id;
  EXPECT_NO_MEMORY_OPERATIONS({
    first_instance_id = rcl_get_instance_id();
  });
  EXPECT_NE(0u, first_instance_id);
  EXPECT_EQ(first_instance_id, rcl_get_instance_id());  // Repeat calls should return the same.
  EXPECT_EQ(true, rcl_ok());
  // Calling after a shutdown should return 0.
  ret = rcl_shutdown();
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(0u, rcl_get_instance_id());
  ASSERT_FALSE(rcl_ok());
  // It should return a different value after another valid init.
  {
    FakeTestArgv test_args;
    ret = rcl_init(test_args.argc, test_args.argv, rcl_get_default_allocator());
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_TRUE(rcl_ok());
  }
  EXPECT_NE(0u, rcl_get_instance_id());
  EXPECT_NE(first_instance_id, rcl_get_instance_id());
  ASSERT_TRUE(rcl_ok());
  // Shutting down a second time should result in 0 again.
  ret = rcl_shutdown();
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(0u, rcl_get_instance_id());
  ASSERT_FALSE(rcl_ok());
}
