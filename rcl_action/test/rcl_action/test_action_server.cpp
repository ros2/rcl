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

#include "test_msgs/action/fibonacci.h"

TEST(TestActionServerInitFini, test_action_server_init_fini)
{
  rcl_ret_t ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_options = rcl_node_get_default_options();
  ret = rcl_node_init(&node, "test_action_server_node", "", &node_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(
    test_msgs, action, Fibonacci);
  const rcl_action_server_options_t options = rcl_action_server_get_default_options();
  const char * action_name = "test_action_server_name";
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();

  // Initialize with a null action server
  ret = rcl_action_server_init(nullptr, &node, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null node
  ret = rcl_action_server_init(&action_server, nullptr, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an invalid node
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  ret = rcl_action_server_init(&action_server, &invalid_node, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null typesupport
  ret = rcl_action_server_init(&action_server, &node, nullptr, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null name
  ret = rcl_action_server_init(&action_server, &node, ts, nullptr, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null options
  ret = rcl_action_server_init(&action_server, &node, ts, action_name, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with valid arguments
  ret = rcl_action_server_init(&action_server, &node, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Try to initialize again
  ret = rcl_action_server_init(&action_server, &node, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_ALREADY_INIT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with null action server
  ret = rcl_action_server_fini(nullptr, &node);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_server_fini(&invalid_action_server, &node);
  // Nothing happens
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

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

class TestActionServer : public ::testing::Test
{
protected:
  void SetUp()
  {
    rcl_ret_t ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(&this->node, "test_action_server_node", "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(
      test_msgs, action, Fibonacci);
    const rcl_action_server_options_t options = rcl_action_server_get_default_options();
    const char * action_name = "test_action_server_name";
    this->action_server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(&this->action_server, &this->node, ts, action_name, &options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    // Finalize
    rcl_ret_t ret = rcl_action_server_fini(&this->action_server, &this->node);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  rcl_action_server_t action_server;
  rcl_node_t node;
};  // class TestActionServer

TEST_F(TestActionServer, test_action_server_is_valid)
{
  // Check with null pointer
  bool is_valid = rcl_action_server_is_valid(nullptr);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with uninitialized action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  is_valid = rcl_action_server_is_valid(&invalid_action_server);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check valid action server
  is_valid = rcl_action_server_is_valid(&this->action_server);
  EXPECT_TRUE(is_valid) << rcl_get_error_string().str;
}

TEST_F(TestActionServer, test_action_server_accept_new_goal)
{
  // Initialize a goal info
  rcl_action_goal_info_t goal_info_in = rcl_action_get_zero_initialized_goal_info();
  for (int i = 0; i < 16; ++i) {
    goal_info_in.uuid[i] = static_cast<uint8_t>(i);
  }
  goal_info_in.stamp.sec = 1234;
  goal_info_in.stamp.nanosec = 4567u;

  // Accept goal with a null action server
  rcl_action_goal_handle_t * goal_handle = rcl_action_accept_new_goal(nullptr, &goal_info_in);
  EXPECT_EQ(goal_handle, nullptr);
  rcl_reset_error();

  // Accept goal with null goal info
  goal_handle = rcl_action_accept_new_goal(&this->action_server, nullptr);
  EXPECT_EQ(goal_handle, nullptr);
  rcl_reset_error();

  // Accept goal with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  goal_handle = rcl_action_accept_new_goal(&invalid_action_server, &goal_info_in);
  EXPECT_EQ(goal_handle, nullptr);
  rcl_reset_error();

  // Accept with valid arguments
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  EXPECT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  rcl_action_goal_info_t goal_info_out = rcl_action_get_zero_initialized_goal_info();
  rcl_ret_t ret = rcl_action_goal_handle_get_info(goal_handle, &goal_info_out);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info_out.uuid[i], goal_info_in.uuid[i]);
  }
  EXPECT_EQ(goal_info_out.stamp.sec, goal_info_in.stamp.sec);
  EXPECT_EQ(goal_info_out.stamp.nanosec, goal_info_in.stamp.nanosec);
  size_t num_goals;
  const rcl_action_goal_handle_t * goal_handle_array = rcl_action_server_get_goal_handles(
    &this->action_server, &num_goals);
  EXPECT_EQ(num_goals, 1u);
  EXPECT_NE(goal_handle_array, nullptr) << rcl_get_error_string().str;

  // Accept with the same goal ID
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  EXPECT_EQ(goal_handle, nullptr);
  rcl_reset_error();

  // Accept a different goal
  goal_info_in = rcl_action_get_zero_initialized_goal_info();
  for (int i = 0; i < 16; ++i) {
    goal_info_in.uuid[i] = static_cast<uint8_t>(15 - i);
  }
  goal_info_in.stamp.sec = 4321;
  goal_info_in.stamp.nanosec = 7654u;
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  EXPECT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  ret = rcl_action_goal_handle_get_info(goal_handle, &goal_info_out);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info_out.uuid[i], goal_info_in.uuid[i]);
  }
  EXPECT_EQ(goal_info_out.stamp.sec, goal_info_in.stamp.sec);
  EXPECT_EQ(goal_info_out.stamp.nanosec, goal_info_in.stamp.nanosec);
  goal_handle_array = rcl_action_server_get_goal_handles(&this->action_server, &num_goals);
  EXPECT_EQ(num_goals, 2u);
  EXPECT_NE(goal_handle_array, nullptr) << rcl_get_error_string().str;
}

TEST_F(TestActionServer, test_action_server_get_goal_status_array)
{
  rcl_action_goal_status_array_t status_array =
    rcl_action_get_zero_initialized_goal_status_array();
  // Get with null action server
  rcl_ret_t ret = rcl_action_get_goal_status_array(nullptr, &status_array);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Get with null status array
  ret = rcl_action_get_goal_status_array(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Get with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_get_goal_status_array(&invalid_action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Get with valid arguments (but not goals being tracked)
  ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_EQ(status_array.msg.status_list.data, nullptr);
  EXPECT_EQ(status_array.msg.status_list.size, 0u);

  // Add a goal before getting the status array
  rcl_action_goal_info_t goal_info_in = rcl_action_get_zero_initialized_goal_info();
  for (int i = 0; i < 16; ++i) {
    goal_info_in.uuid[i] = static_cast<uint8_t>(i);
  }
  rcl_action_goal_handle_t * goal_handle;
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(status_array.msg.status_list.data, nullptr);
  EXPECT_EQ(status_array.msg.status_list.size, 1u);
  rcl_action_goal_info_t * goal_info_out = &status_array.msg.status_list.data[0].goal_info;
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info_out->uuid[i], goal_info_in.uuid[i]);
  }

  // Add nine more goals
  /*
  for (int i = 1; i < 10; ++i) {
    for (int j = 0; j < 16; ++j) {
      goal_info_in.uuid[j] = static_cast<uint8_t>(i + j);
    }
    goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
    ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  }
  ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(status_array.msg.status_list.data, nullptr);
  EXPECT_EQ(status_array.msg.status_list.size, 10u);
  for (int i = 0; i < 10; ++i) {
    goal_info_out = &status_array.msg.status_list.data[0].goal_info;
    for (int j = 0; j < 16; ++j) {
      EXPECT_EQ(goal_info_out->uuid[j], i + j);
    }
  }
  */
}

TEST_F(TestActionServer, test_action_server_get_action_name)
{
  // Get action_name for a null action server
  const char * action_name = rcl_action_server_get_action_name(nullptr);
  EXPECT_EQ(action_name, nullptr);
  rcl_reset_error();

  // Get action_name for an invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  action_name = rcl_action_server_get_action_name(&invalid_action_server);
  EXPECT_EQ(action_name, nullptr);
  rcl_reset_error();

  // Get action_name for a valid action server
  action_name = rcl_action_server_get_action_name(&this->action_server);
  ASSERT_NE(action_name, nullptr) << rcl_get_error_string().str;
  EXPECT_STREQ(action_name, "test_action_server_name");
}

TEST_F(TestActionServer, test_action_server_get_options)
{
  // Get options for a null action server
  const rcl_action_server_options_t * options = rcl_action_server_get_options(nullptr);
  EXPECT_EQ(options, nullptr);
  rcl_reset_error();

  // Get options for an invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  options = rcl_action_server_get_options(&invalid_action_server);
  EXPECT_EQ(options, nullptr);
  rcl_reset_error();

  // Get options for a valid action server
  options = rcl_action_server_get_options(&this->action_server);
  EXPECT_NE(options, nullptr) << rcl_get_error_string().str;
}
