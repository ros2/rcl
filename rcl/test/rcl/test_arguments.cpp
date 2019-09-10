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
#include <vector>

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#include "rcl/rcl.h"
#include "rcl/arguments.h"
#include "rcl/error_handling.h"

#include "rcl_yaml_param_parser/parser.h"

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
      ASSERT_TRUE(NULL != actual_unparsed); \
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
    EXPECT_EQ(expected.str(), actual.str()); \
  } while (0)

#define EXPECT_UNPARSED_ROS(parsed_args, ...) \
  do { \
    int expect_unparsed_ros[] = {__VA_ARGS__}; \
    int expect_num_unparsed_ros = sizeof(expect_unparsed_ros) / sizeof(int); \
    rcl_allocator_t alloc = rcl_get_default_allocator(); \
    int actual_num_unparsed_ros = rcl_arguments_get_count_unparsed_ros(&parsed_args); \
    int * actual_unparsed_ros = NULL; \
    if (actual_num_unparsed_ros > 0) { \
      rcl_ret_t ret = rcl_arguments_get_unparsed_ros(&parsed_args, alloc, &actual_unparsed_ros); \
      ASSERT_EQ(RCL_RET_OK, ret); \
      ASSERT_TRUE(NULL != actual_unparsed_ros); \
    } \
    std::stringstream expected; \
    expected << "["; \
    for (int e = 0; e < expect_num_unparsed_ros; ++e) { \
      expected << expect_unparsed_ros[e] << ", "; \
    } \
    expected << "]"; \
    std::stringstream actual; \
    actual << "["; \
    for (int a = 0; a < actual_num_unparsed_ros; ++a) { \
      actual << actual_unparsed_ros[a] << ", "; \
    } \
    actual << "]"; \
    if (NULL != actual_unparsed_ros) { \
      alloc.deallocate(actual_unparsed_ros, alloc.state); \
    } \
    EXPECT_EQ(expected.str(), actual.str()); \
  } while (0)

bool
are_known_ros_args(std::vector<const char *> argv)
{
  const int argc = static_cast<int>(argv.size());
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(
    argc, argv.data(), rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  bool is_valid = (
    0 == rcl_arguments_get_count_unparsed(&parsed_args) &&
    0 == rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  return is_valid;
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), check_known_vs_unknown_args) {
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "__node:=node_name"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "old_name:__node:=node_name"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "old_name:__node:=nodename123"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "__node:=nodename123"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "__ns:=/foo/bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "__ns:=/"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "_:=kq"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "nodename:__ns:=/foobar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "foo:=bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "~/foo:=~/bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "/foo/bar:=bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "foo:=/bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "/foo123:=/bar123"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "node:/foo123:=/bar123"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "rostopic:=/foo/bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "rosservice:=baz"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "rostopic://rostopic:=rosservice"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "rostopic:///rosservice:=rostopic"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-r", "rostopic:///foo/bar:=baz"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-p", "foo:=bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-p", "~/foo:=~/bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-p", "/foo/bar:=bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-p", "foo:=/bar"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "-p", "/foo123:=/bar123"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--params-file", "file_name.yaml"}));

  EXPECT_FALSE(are_known_ros_args({"--ros-args", "--custom-ros-arg"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "__node:=node_name"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "old_name:__node:=node_name"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "/foo/bar:=bar"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "foo:=/bar"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "file_name.yaml"}));

  // Setting config logging file
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-config-file", "file.config"}));

  // Setting logger level
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "UNSET"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "DEBUG"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "INFO"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "WARN"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "ERROR"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "FATAL"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "debug"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--log-level", "Info"}));

  EXPECT_FALSE(are_known_ros_args({"--ros-args", "--log", "foo"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "--loglevel", "foo"}));

  // Disabling logging
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--enable-rosout-logs"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--disable-rosout-logs"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--enable-stdout-logs"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--disable-stdout-logs"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--enable-external-lib-logs"}));
  EXPECT_TRUE(are_known_ros_args({"--ros-args", "--disable-external-lib-logs"}));

  EXPECT_FALSE(are_known_ros_args({"--ros-args", "stdout-logs"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "external-lib-logs"}));
  EXPECT_FALSE(are_known_ros_args({"--ros-args", "external-lib-logs"}));
}

bool
are_valid_ros_args(std::vector<const char *> argv)
{
  const int argc = static_cast<int>(argv.size());
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(
    argc, argv.data(), rcl_get_default_allocator(), &parsed_args);
  if (RCL_RET_OK != ret) {
    EXPECT_EQ(ret, RCL_RET_INVALID_ROS_ARGS) << rcl_get_error_string().str;
    rcl_reset_error();
    return false;
  }
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  return true;
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), check_valid_vs_invalid_args) {
  EXPECT_TRUE(are_valid_ros_args({
    "--ros-args", "-p", "foo:=bar", "-r", "__node:=node_name",
    "--params-file", "file_name.yaml", "--log-level", "INFO",
    "--log-config-file", "file.config"
  }));

  // ROS args unknown to rcl are not (necessarily) invalid
  EXPECT_TRUE(are_valid_ros_args({"--ros-args", "--custom-ros-arg"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--remap"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", ":="}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "foo:="}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", ":=bar"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-p"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--params-file"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-p", ":="}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-p", "foo:="}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-p", ":=bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "__ns:="}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "__node:="}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "__node:=/foo/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "__ns:=foo"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", ":__node:=nodename"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "~:__node:=nodename"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "}foo:=/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "f oo:=/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "foo:=/b ar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "f{oo:=/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "foo:=/b}ar"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "rostopic://:=rosservice"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-r", "rostopic::=rosservice"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-p", "}foo:=/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--param", "}foo:=/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "-p", "f oo:=/bar"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--param", "f oo:=/bar"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--params-file"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--log-config-file"}));

  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--log-level"}));
  EXPECT_FALSE(are_valid_ros_args({"--ros-args", "--log-level", "foo"}));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_no_args) {
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(0, NULL, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed(&parsed_args));
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
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

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_no_ros_args) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_zero_ros_args) {
  const char * argv[] = {"process_name", "--ros-args"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_zero_ros_args_w_trailing_dashes) {
  const char * argv[] = {"process_name", "--ros-args", "--"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_remap) {
  const char * argv[] = {
    "process_name", "--ros-args", "-r", "/foo/bar:=/fiz/buz", "--remap", "foo:=/baz"
  };
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_remap_two_ros_args) {
  const char * argv[] = {"process_name", "--ros-args", "--ros-args", "-r", "/foo/bar:=/fiz/buz"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_remap_w_trailing_dashes) {
  const char * argv[] = {"process_name", "--ros-args", "-r", "/foo/bar:=/fiz/buz", "--"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_one_remap_w_two_trailing_dashes) {
  const char * argv[] = {"process_name", "--ros-args", "-r", "/foo/bar:=/fiz/buz", "--", "--"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0, 5);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_mix_valid_invalid_rules) {
  const char * argv[] = {
    "process_name", "--ros-args", "/foo/bar:=", "-r", "bar:=/fiz/buz", "}bar:=fiz", "--", "arg"
  };
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0, 7);
  EXPECT_UNPARSED_ROS(parsed_args, 2, 5);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_copy) {
  const char * argv[] = {
    "process_name", "--ros-args", "/foo/bar:=", "-r", "bar:=/fiz/buz", "-r", "__ns:=/foo", "--",
    "arg"
  };
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;

  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_arguments_t copied_args = rcl_get_zero_initialized_arguments();
  ret = rcl_arguments_copy(&parsed_args, &copied_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  EXPECT_UNPARSED(parsed_args, 0, 8);
  EXPECT_UNPARSED_ROS(parsed_args, 2);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));

  EXPECT_UNPARSED(copied_args, 0, 8);
  EXPECT_UNPARSED_ROS(copied_args, 2);
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&copied_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_copy_no_ros_args) {
  const char * argv[] = {"process_name", "--ros-args", "--", "arg", "--ros-args"};
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;

  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_arguments_t copied_args = rcl_get_zero_initialized_arguments();
  ret = rcl_arguments_copy(&parsed_args, &copied_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  EXPECT_UNPARSED(parsed_args, 0, 3);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));

  EXPECT_UNPARSED(copied_args, 0, 3);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&copied_args));
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
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));

  rcl_arguments_t copied_args = rcl_get_zero_initialized_arguments();
  ret = rcl_arguments_copy(&parsed_args, &copied_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed(&copied_args));
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&copied_args));

  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&copied_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_two_namespace) {
  const char * argv[] = {
    "process_name", "--ros-args", "-r", "__ns:=/foo/bar", "-r", "__ns:=/fiz/buz"
  };
  int argc = sizeof(argv) / sizeof(const char *);
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();
  rcl_ret_t ret;
  ret = rcl_parse_arguments(argc, argv, rcl_get_default_allocator(), &parsed_args);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_UNPARSED(parsed_args, 0);
  EXPECT_EQ(0, rcl_arguments_get_count_unparsed_ros(&parsed_args));
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
  const char * argv[] = {
    "process_name", "--ros-args", "-r", "__ns:=/foo/bar", "-r", "__ns:=/fiz/buz"
  };
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
  const char * argv[] = {
    "process_name", "-d", "--ros-args", "-r", "__ns:=/foo/bar", "-r", "__ns:=/fiz/buz", "--",
    "--foo=bar", "--baz", "--ros-args", "--ros-args", "-p", "bar:=baz", "--", "--", "arg"
  };
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
  ASSERT_EQ(nonros_argc, 6);
  EXPECT_STREQ(nonros_argv[0], "process_name");
  EXPECT_STREQ(nonros_argv[1], "-d");
  EXPECT_STREQ(nonros_argv[2], "--foo=bar");
  EXPECT_STREQ(nonros_argv[3], "--baz");
  EXPECT_STREQ(nonros_argv[4], "--");
  EXPECT_STREQ(nonros_argv[5], "arg");
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
  const char * argv[] = {"process_name", "--ros-args", "-r", "__ns:=/namespace", "random:=arg"};
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
    "process_name", "--ros-args", "-r", "__ns:=/namespace", "random:=arg",
    "--params-file", "parameter_filepath"
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
    "process_name", "--ros-args", "--params-file", "parameter_filepath1",
    "-r", "__ns:=/namespace", "random:=arg", "--params-file", "parameter_filepath2"
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

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_no_param_overrides) {
  const char * argv[] = {"process_name"};
  int argc = sizeof(argv) / sizeof(const char *);

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();

  rcl_ret_t ret = rcl_parse_arguments(argc, argv, alloc, &parsed_args);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_arguments_get_param_overrides(&parsed_args, NULL);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  rcl_params_t * params = NULL;
  ret = rcl_arguments_get_param_overrides(NULL, &params);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  rcl_params_t preallocated_params;
  params = &preallocated_params;
  ret = rcl_arguments_get_param_overrides(&parsed_args, &params);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  params = NULL;
  ret = rcl_arguments_get_param_overrides(&parsed_args, &params);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(0U, params->num_nodes);
  rcl_yaml_node_struct_fini(params);

  EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
}

TEST_F(CLASSNAME(TestArgumentsFixture, RMW_IMPLEMENTATION), test_two_param_overrides) {
  const char * argv[] = {
    "process_name", "--ros-args",
    "--param", "string_param:=test_string",
    "-p", "some_node:int_param:=4"
  };
  int argc = sizeof(argv) / sizeof(const char *);

  rcl_allocator_t alloc = rcl_get_default_allocator();
  rcl_arguments_t parsed_args = rcl_get_zero_initialized_arguments();

  rcl_ret_t ret = rcl_parse_arguments(argc, argv, alloc, &parsed_args);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_arguments_fini(&parsed_args));
  });

  rcl_params_t * params = NULL;
  ret = rcl_arguments_get_param_overrides(&parsed_args, &params);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_yaml_node_struct_fini(params);
  });
  EXPECT_EQ(2U, params->num_nodes);

  rcl_variant_t * param_value =
    rcl_yaml_node_struct_get("/**", "string_param", params);
  ASSERT_TRUE(NULL != param_value);
  ASSERT_TRUE(NULL != param_value->string_value);
  EXPECT_STREQ("test_string", param_value->string_value);

  param_value = rcl_yaml_node_struct_get("some_node", "int_param", params);
  ASSERT_TRUE(NULL != param_value);
  ASSERT_TRUE(NULL != param_value->integer_value);
  EXPECT_EQ(4, *(param_value->integer_value));
}
