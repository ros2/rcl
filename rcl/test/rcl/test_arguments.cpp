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
#include <sstream>

#include "rcl/rcl.h"
#include "rcl/arguments.h"

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
  }

  void TearDown()
  {
  }
};

#define EXPECT_UNPARSED(parsed_args, ...) \
  do { \
    int expect_unparsed[] = {__VA_ARGS__}; \
    int expect_num_unparsed = sizeof(expect_unparsed) / sizeof(int); \
    rcl_allocator_t alloc = rcl_get_default_allocator(); \
    int actual_num_unparsed = rcl_arguments_get_count_unparsed(&parsed_args); \
    int * actual_unparsed = NULL; \
    if (actual_num_unparsed > 0) { \
      rcl_ret_t ret = rcl_arguments_get_unparsed(&parsed_args, alloc, &actual_unparsed); \
      ASSERT_EQ(RCL_RET_OK, ret); \
    } \
    std::stringstream expected; \
    expected << "["; \
    for (int e = 0; e < expect_num_unparsed; ++e) { \
      expected << expect_unparsed[e] << ", "; \
    } \
    expected << "]"; \
    std::stringstream actual; \
    actual << "["; \
    for (int a = 0; a < actual_num_unparsed; ++a) { \
      actual << actual_unparsed[a] << ", "; \
    } \
    actual << "]"; \
    if (NULL != actual_unparsed) { \
      alloc.deallocate(actual_unparsed, alloc.state); \
    } \
    EXPECT_STREQ(expected.str().c_str(), actual.str().c_str()); \
  } while (0)

bool
is_valid_arg(const char * arg)
{
  const char * argv[] = {arg};
  rcl_arguments_t parsed_args;
  rcl_ret_t ret = rcl_parse_arguments(1, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  bool is_valid = 0 == rcl_arguments_get_count_unparsed(&parsed_args);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  return is_valid;
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), check_valid_vs_invalid_args) {
  EXPECT_TRUE(is_valid_arg("__node:=node_name"));
  EXPECT_TRUE(is_valid_arg("old_name:__node:=node_name"));
  EXPECT_TRUE(is_valid_arg("old_name:__node:=nodename123"));
  EXPECT_TRUE(is_valid_arg("__node:=nodename123"));
  EXPECT_TRUE(is_valid_arg("__ns:=/foo/bar"));
  EXPECT_TRUE(is_valid_arg("__ns:=/"));
  EXPECT_TRUE(is_valid_arg("_:=kq"));
  EXPECT_TRUE(is_valid_arg("nodename:__ns:=/foobar"));
  EXPECT_TRUE(is_valid_arg("foo:=bar"));
  EXPECT_TRUE(is_valid_arg("~/foo:=~/bar"));
  EXPECT_TRUE(is_valid_arg("/foo/bar:=bar"));
  EXPECT_TRUE(is_valid_arg("foo:=/bar"));
  EXPECT_TRUE(is_valid_arg("/foo123:=/bar123"));
  EXPECT_TRUE(is_valid_arg("node:/foo123:=/bar123"));

  EXPECT_FALSE(is_valid_arg(":="));
  EXPECT_FALSE(is_valid_arg("foo:="));
  EXPECT_FALSE(is_valid_arg(":=bar"));
  EXPECT_FALSE(is_valid_arg("__ns:="));
  EXPECT_FALSE(is_valid_arg("__node:="));
  EXPECT_FALSE(is_valid_arg("__node:=/foo/bar"));
  EXPECT_FALSE(is_valid_arg("__ns:=foo"));
  EXPECT_FALSE(is_valid_arg(":__node:=nodename"));
  EXPECT_FALSE(is_valid_arg("~:__node:=nodename"));
  EXPECT_FALSE(is_valid_arg("}foo:=/bar"));
  EXPECT_FALSE(is_valid_arg("f oo:=/bar"));
  EXPECT_FALSE(is_valid_arg("foo:=/b ar"));
  EXPECT_FALSE(is_valid_arg("f{oo:=/bar"));
  EXPECT_FALSE(is_valid_arg("foo:=/b}ar"));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_no_args) {
  rcl_arguments_t parsed_args;
  rcl_ret_t ret = rcl_parse_arguments(0, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_null_args) {
  int argc = 1;
  rcl_arguments_t parsed_args;
  rcl_ret_t ret = rcl_parse_arguments(argc, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_null_args_output) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), NULL);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_remap) {
  const char * argv[] = {"process_name", "/foo/bar:=/fiz/buz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_mix_valid_invalid_rules) {
  const char * argv[] = {"process_name", "/foo/bar:=", "bar:=/fiz/buz", "}bar:=fiz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_UNPARSED(parsed_args, 0, 1, 3);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_two_namespace) {
  const char * argv[] = {"process_name", "__ns:=/foo/bar", "__ns:=/fiz/buz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args;
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_null) {
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_arguments_fini(NULL));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_impl_null) {
  rcl_arguments_t parsed_args;
  parsed_args.impl = NULL;
  EXPECT_EQ(RCL_RET_ERROR, rcl_arguments_fini(&parsed_args));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_twice) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args;
  ASSERT_EQ(RCL_RET_OK, rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  EXPECT_EQ(RCL_RET_ERROR, rcl_arguments_fini(&parsed_args));
  rcl_reset_error();
}
