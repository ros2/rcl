// Copyright 2018 Open Source Robotics Foundation, Inc.
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
#include "rcl/arguments.h"

#include "../memory_tools/memory_tools.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif


class CLASSNAME (TestArgumentsFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
    stop_memory_checking();
    rcl_ret_t ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
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
    rcl_ret_t ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_only_proc_name) {
  int argc = 1;
  const char * argv[] = {"process_name"};
  rcl_arguments_t parsed_args;
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_no_args) {
  rcl_arguments_t parsed_args;
  rcl_ret_t ret = rcl_parse_arguments(0, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_null_args) {
  int argc = 1;
  rcl_arguments_t parsed_args;
  rcl_ret_t ret = rcl_parse_arguments(argc, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_null_args_output) {
  int argc = 1;
  const char * argv[] = {"process_name"};
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), NULL);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_remap) {
  int argc = 2;
  const char * argv[] = {"process_name", "/foo/bar:=/fiz/buz"};
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_invalid_remap) {
  int argc = 5;
  const char * argv[] = {"process_name", "/foo/bar:=", ":=/fiz/buz", ":=", "/fiz=/buz"};
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_namespace) {
  int argc = 2;
  const char * argv[] = {"process_name", "__ns:=/foo/bar"};
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_two_namespace) {
  int argc = 3;
  const char * argv[] = {"process_name", "__ns:=/foo/bar", "__ns:=/fiz/buz"};
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_null) {
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_arguments_fini(NULL, rcl_get_default_allocator()));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_impl_null) {
  rcl_arguments_t parsed_args;
  parsed_args.impl = NULL;
  EXPECT_EQ(RCL_RET_ERROR, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_twice) {
  int argc = 1;
  const char * argv[] = {"process_name"};
  rcl_arguments_t parsed_args;
  ASSERT_EQ(RCL_RET_OK, rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
  EXPECT_EQ(RCL_RET_ERROR, rcl_arguments_fini(&parsed_args, rcl_get_default_allocator()));
  rcl_reset_error();
}
