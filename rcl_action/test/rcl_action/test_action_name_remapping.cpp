// Copyright 2024 Open Source Robotics Foundation, Inc.
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
#include <rcl/error_handling.h>
#include <rcl_action/rcl_action.h>
#include "test_msgs/action/fibonacci.h"
#include "arg_macros.hpp"

class TestActionNameRemappingFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }

  void TearDown() override
  {
  }
};

TEST_F(TestActionNameRemappingFixture, test_action_client_name_remapping) {
  // Check that remapping works with global args passed to rcl_init
  int argc;
  char ** argv;

  SCOPE_GLOBAL_ARGS(
    argc, argv,
    "process_name",
    "--ros-args",
    "-r", "__node:=new_name",
    "-r", "__ns:=/new_ns",
    "-r", "/foo/bar:=/bar/foo");

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t default_options = rcl_node_get_default_options();
  ASSERT_EQ(
    RCL_RET_OK,
    rcl_node_init(&node, "original_name", "/original_ns", &context, &default_options));

  // Absolute name that should be unchanged
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "/absolute_name", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/absolute_name",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  // Absolute name that should change
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "/foo/bar", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/bar/foo",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  // Namespace relative names should work
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "ns_relative_name", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/new_ns/ns_relative_name",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  // Node relative names should work
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "~/node_relative_name", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/new_ns/new_name/node_relative_name",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node)) << rcl_get_error_string().str;
}

TEST_F(TestActionNameRemappingFixture, test_action_client_name_remapping_local_rules) {
  // Check that remapping works with local args passed to rcl_node_init
  int argc;
  char ** argv;

  SCOPE_GLOBAL_ARGS(
    argc, argv,
    "process_name",
    "--ros-args",
    "-r", "__node:=global_name",
    "-r", "__ns:=/global_ns",
    "-r", "/foo/bar:=/bar/foo");

  rcl_arguments_t local_arguments;
  SCOPE_ARGS(
    local_arguments,
    "process_name",
    "--ros-args",
    "-r", "__node:=local_name",
    "-r", "__ns:=/local_ns",
    "-r", "/foo/bar:=/bar/local");

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t options = rcl_node_get_default_options();
  options.arguments = local_arguments;
  ASSERT_EQ(
    RCL_RET_OK,
    rcl_node_init(&node, "original_name", "/original_ns", &context, &options));

  // Absolute name that should be unchanged
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "/absolute_name", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/absolute_name",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  // Absolute name that should change
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "/foo/bar", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/bar/local",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  // Namespace relative names should work
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "ns_relative_name", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/local_ns/ns_relative_name",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  // Node relative names should work
  {
    const rcl_action_client_options_t action_client_options =
      rcl_action_client_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_client_t client = rcl_action_get_zero_initialized_client();
    rcl_ret_t ret = rcl_action_client_init(
      &client, &node, action_typesupport, "~/node_relative_name", &action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ("/local_ns/local_name/node_relative_name",
      rcl_action_client_get_action_name(&client)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&client, &node)) << rcl_get_error_string().str;
  }

  EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node)) << rcl_get_error_string().str;
}

TEST_F(TestActionNameRemappingFixture, test_action_server_name_remapping) {
  // Check that remapping works with global args passed to rcl_init
  int argc;
  char ** argv;

  SCOPE_GLOBAL_ARGS(
    argc, argv,
    "process_name",
    "--ros-args",
    "-r", "__node:=new_name",
    "-r", "__ns:=/new_ns",
    "-r", "/foo/bar:=/bar/foo");

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t default_options = rcl_node_get_default_options();
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_clock_t clock;
  rcl_ret_t ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(
    RCL_RET_OK,
    rcl_node_init(&node, "original_name", "/original_ns", &context, &default_options)
  );

  // Absolute name that should be unchanged
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "/absolute_name", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/absolute_name",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }

  // Absolute name that should change
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "/foo/bar", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/bar/foo",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }

  // Namespace relative names should work
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "ns_relative_name", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/new_ns/ns_relative_name",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }

  // Node relative names should work
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "~/node_relative_name", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/new_ns/new_name/node_relative_name",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }
  EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node)) << rcl_get_error_string().str;
}

TEST_F(TestActionNameRemappingFixture, test_action_server_name_remapping_local_rules) {
  // Check that remapping works with local args passed to rcl_init
  int argc;
  char ** argv;

  SCOPE_GLOBAL_ARGS(
    argc, argv,
    "process_name",
    "--ros-args",
    "-r", "__node:=global_name",
    "-r", "__ns:=/global_ns",
    "-r", "/foo/bar:=/bar/global");

  rcl_arguments_t local_arguments;
  SCOPE_ARGS(
    local_arguments,
    "process_name",
    "--ros-args",
    "-r", "__node:=local_name",
    "-r", "__ns:=/local_ns",
    "-r", "/foo/bar:=/bar/local");

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t options = rcl_node_get_default_options();
  options.arguments = local_arguments;

  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_clock_t clock;
  rcl_ret_t ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(
    RCL_RET_OK,
    rcl_node_init(&node, "original_name", "/original_ns", &context, &options)
  );

  // Absolute name that should be unchanged
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "/absolute_name", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/absolute_name",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }

  // Absolute name that should change
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "/foo/bar", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/bar/local",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }

  // Namespace relative names should work
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "ns_relative_name", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/local_ns/ns_relative_name",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }

  // Node relative names should work
  {
    const rcl_action_server_options_t action_server_options =
      rcl_action_server_get_default_options();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    rcl_action_server_t server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &server, &node, &clock, action_typesupport, "~/node_relative_name", &action_server_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_STREQ(
      "/local_ns/local_name/node_relative_name",
      rcl_action_server_get_action_name(&server)
    ) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&server, &node)) << rcl_get_error_string().str;
  }
  EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node)) << rcl_get_error_string().str;
}
