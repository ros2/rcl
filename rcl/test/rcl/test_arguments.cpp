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
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(1, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
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
  EXPECT_TRUE(is_valid_arg("rostopic:=/foo/bar"));
  EXPECT_TRUE(is_valid_arg("rosservice:=baz"));
  EXPECT_TRUE(is_valid_arg("rostopic://rostopic:=rosservice"));
  EXPECT_TRUE(is_valid_arg("rostopic:///rosservice:=rostopic"));
  EXPECT_TRUE(is_valid_arg("rostopic:///foo/bar:=baz"));
  EXPECT_TRUE(is_valid_arg("__params:=file_name.yaml"));

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
  EXPECT_FALSE(is_valid_arg("rostopic://:=rosservice"));
  EXPECT_FALSE(is_valid_arg("rostopic::=rosservice"));
  EXPECT_FALSE(is_valid_arg("__param:=file_name.yaml"));

  // Setting logger level
  EXPECT_TRUE(is_valid_arg("__log_level:=UNSET"));
  EXPECT_TRUE(is_valid_arg("__log_level:=DEBUG"));
  EXPECT_TRUE(is_valid_arg("__log_level:=INFO"));
  EXPECT_TRUE(is_valid_arg("__log_level:=WARN"));
  EXPECT_TRUE(is_valid_arg("__log_level:=ERROR"));
  EXPECT_TRUE(is_valid_arg("__log_level:=FATAL"));
  EXPECT_TRUE(is_valid_arg("__log_level:=debug"));
  EXPECT_TRUE(is_valid_arg("__log_level:=Info"));

  EXPECT_FALSE(is_valid_arg("__log:=foo"));
  EXPECT_FALSE(is_valid_arg("__loglevel:=foo"));
  EXPECT_FALSE(is_valid_arg("__log_level:="));
  EXPECT_FALSE(is_valid_arg("__log_level:=foo"));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_no_args) {
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(0, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_null_args) {
  int argc = 1;
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(argc, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_null_args_output) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), NULL);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_remap) {
  const char * argv[] = {"process_name", "/foo/bar:=/fiz/buz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_mix_valid_invalid_rules) {
  const char * argv[] = {"process_name", "/foo/bar:=", "bar:=/fiz/buz", "}bar:=fiz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0, 1, 3);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_copy) {
  const char * argv[] = {"process_name", "/foo/bar:=", "bar:=/fiz/buz", "__ns:=/foo"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;

  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_arguments_t copied_args = rcl_get_zero_initialized_arguments();
  ret = rcl_arguments_copy(&parsed_args, &copied_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  EXPECT_UNPARSED(parsed_args, 0, 1);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));

  EXPECT_UNPARSED(copied_args, 0, 1);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&copied_args));
}

// Similar to the default allocator, but returns NULL when size is zero.
// This is useful for emulating systems where `malloc(0)` return NULL.
// TODO(jacobperron): Consider using this allocate function in other tests
static void *
__return_null_on_zero_allocate(size_t size, void * state)
{
  RCUTILS_UNUSED(state);
  if (size == 0) {
    return NULL;
  }
  return malloc(size);
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_copy_no_args) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  allocator.allocate = __return_null_on_zero_allocate;
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(0, NULL, allocator, &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed(&parsed_args));

  rcl_arguments_t copied_args = rcl_get_zero_initialized_arguments();
  ret = rcl_arguments_copy(&parsed_args, &copied_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed(&copied_args));

  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&copied_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_two_namespace) {
  const char * argv[] = {"process_name", "__ns:=/foo/bar", "__ns:=/fiz/buz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_uninitialized_parsed_args) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args;
  int not_null = 1;
  parsed_args.impl = reinterpret_cast<rcl_arguments_impl_t *>(&not_null);
  ASSERT_EQ(RCL_RET_INVALID_ARGUMENT,
    rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_double_parse) {
  const char * argv[] = {"process_name", "__ns:=/foo/bar", "__ns:=/fiz/buz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  ASSERT_EQ(RCL_RET_OK,
    rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args));
  ASSERT_EQ(RCL_RET_INVALID_ARGUMENT,
    rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args));
  rcl_reset_error();
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}


TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_null) {
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_arguments_fini(NULL));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_impl_null) {
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  parsed_args.impl = NULL;
  EXPECT_EQ(RCL_RET_ERROR, rcl_arguments_fini(&parsed_args));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_fini_twice) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  ASSERT_EQ(RCL_RET_OK, rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  EXPECT_EQ(RCL_RET_ERROR, rcl_arguments_fini(&parsed_args));
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_remove_ros_args) {
  const char * argv[] =
  {"process_name", "-d", "__ns:=/foo/bar", "__ns:=/fiz/buz", "--foo=bar", "--baz"};
  int argc = sizeof(argv) / sizeof(const char *);

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, alloc, &parsed_args);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  int nonros_argc = 0;
  const char ** nonros_argv = NULL;

  ret = rcl_remove_ros_arguments(
    argv,
    &parsed_args,
    alloc,
    &nonros_argc,
    &nonros_argv);

  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(nonros_argc, 4);
  EXPECT_STREQ(nonros_argv[0], "process_name");
  EXPECT_STREQ(nonros_argv[1], "-d");
  EXPECT_STREQ(nonros_argv[2], "--foo=bar");
  EXPECT_STREQ(nonros_argv[3], "--baz");
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));

  if (NULL != nonros_argv) {
    alloc.deallocate(nonros_argv, alloc.state);
  }
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_remove_ros_args_zero) {
  const char * argv[] = {""};
  rcl_ret_t ret;

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();

  int nonros_argc = 0;
  const char ** nonros_argv = NULL;

  ret = rcl_remove_ros_arguments(
    argv,
    &parsed_args,
    alloc,
    &nonros_argc,
    &nonros_argv);

  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;

  if (NULL != nonros_argv) {
    alloc.deallocate(nonros_argv, alloc.state);
  }
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_param_argument_zero) {
  const char * argv[] = {"process_name", "__ns:=/namespace", "random:=arg"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_ret_t ret;

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();

  ret = rcl_parse_arguments(argc, argv, alloc, &parsed_args);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  int parameter_filecount = rcl_arguments_get_param_files_count(&parsed_args);
  EXPECT_EQ(0, parameter_filecount);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_param_argument_single) {
  const char * argv[] = {
    "process_name", "__ns:=/namespace", "random:=arg", "__params:=parameter_filepath"
  };
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_ret_t ret;

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();

  ret = rcl_parse_arguments(argc, argv, alloc, &parsed_args);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  int parameter_filecount = rcl_arguments_get_param_files_count(&parsed_args);
  EXPECT_EQ(1, parameter_filecount);
  char ** parameter_files = NULL;
  ret = rcl_arguments_get_param_files(&parsed_args, alloc, &parameter_files);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_STREQ("parameter_filepath", parameter_files[0]);

  for (int i = 0; i < parameter_filecount; ++i) {
    alloc.deallocate(parameter_files[i], alloc.state);
  }
  alloc.deallocate(parameter_files, alloc.state);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_param_argument_multiple) {
  const char * argv[] = {
    "process_name", "__params:=parameter_filepath1", "__ns:=/namespace",
    "random:=arg", "__params:=parameter_filepath2"
  };
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_ret_t ret;

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();

  ret = rcl_parse_arguments(argc, argv, alloc, &parsed_args);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  int parameter_filecount = rcl_arguments_get_param_files_count(&parsed_args);
  EXPECT_EQ(2, parameter_filecount);
  char ** parameter_files = NULL;
  ret = rcl_arguments_get_param_files(&parsed_args, alloc, &parameter_files);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_STREQ("parameter_filepath1", parameter_files[0]);
  EXPECT_STREQ("parameter_filepath2", parameter_files[1]);
  for (int i = 0; i < parameter_filecount; ++i) {
    alloc.deallocate(parameter_files[i], alloc.state);
  }
  alloc.deallocate(parameter_files, alloc.state);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}
