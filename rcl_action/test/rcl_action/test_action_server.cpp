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

#include "rcl_action/action_server.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

TEST(TestActionServer, test_action_server_init_fini)
{
  rcl_ret_t ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_options = rcl_node_get_default_options();
  ret = rcl_node_init(&node, "test_action_server_node", "", &node_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  // TODO(jacobperron): Replace when ready
  // const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(
  //   test_msgs, Fibonacci);
  const rosidl_action_type_support_t ts = {0};
  const rcl_action_server_options_t options = rcl_action_server_get_default_options();
  const char * action_name = "test_action_server_name";
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();

  // Initialize with a null action server
  ret = rcl_action_server_init(nullptr, &node, &ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null node
  ret = rcl_action_server_init(&action_server, nullptr, &ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an invalid node
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  ret = rcl_action_server_init(&action_server, &invalid_node, &ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null typesupport
  ret = rcl_action_server_init(&action_server, &node, nullptr, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null name
  ret = rcl_action_server_init(&action_server, &node, &ts, nullptr, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null options
  ret = rcl_action_server_init(&action_server, &node, &ts, action_name, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with valid arguments
  ret = rcl_action_server_init(&action_server, &node, &ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Try to initialize again
  ret = rcl_action_server_init(&action_server, &node, &ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_ALREADY_INIT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with null action server
  ret = rcl_action_server_fini(nullptr, &node);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_server_fini(&invalid_action_server, &node);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with null node
  ret = rcl_action_server_fini(&action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with valid arguments
  ret = rcl_action_server_fini(&action_server, &node);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Finalize node
  ret = rcl_node_fini(&node);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_shutdown();
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST(TestActionServer, test_action_server_is_valid)
{
  rcl_ret_t ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_options = rcl_node_get_default_options();
  ret = rcl_node_init(&node, "test_action_server_node", "", &node_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  // TODO(jacobperron): Replace when ready
  // const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(
  //   test_msgs, Fibonacci);
  const rosidl_action_type_support_t ts = {0};
  const rcl_action_server_options_t options = rcl_action_server_get_default_options();
  const char * action_name = "test_action_server_name";

  // Check with null pointer
  bool is_valid = rcl_action_server_is_valid(nullptr);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with uninitialized action server
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();
  is_valid = rcl_action_server_is_valid(&action_server);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check valid action server
  ret = rcl_action_server_init(&action_server, &node, &ts, action_name, &options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  is_valid = rcl_action_server_is_valid(&action_server);
  EXPECT_TRUE(is_valid) << rcl_get_error_string().str;

  // Finalize
  ret = rcl_action_server_fini(&action_server, &node);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ret = rcl_shutdown();
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}
