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

#include <chrono>
#include <thread>

#include "rcl_action/action_server.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

#include "test_msgs/action/fibonacci.h"

TEST(TestActionServerInitFini, test_action_server_init_fini)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_ret_t ret;
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ret = rcl_init_options_init(&init_options, allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_context_t context = rcl_get_zero_initialized_context();
  ret = rcl_init(0, nullptr, &init_options, &context);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_options = rcl_node_get_default_options();
  ret = rcl_node_init(&node, "test_action_server_node", "", &context, &node_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_clock_t clock;
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
  const rcl_action_server_options_t options = rcl_action_server_get_default_options();
  const char * action_name = "test_action_server_name";
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();

  // Initialize with a null action server
  ret = rcl_action_server_init(nullptr, &node, &clock, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null node
  ret = rcl_action_server_init(&action_server, nullptr, &clock, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an invalid node
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  ret = rcl_action_server_init(&action_server, &invalid_node, &clock, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null clock
  ret = rcl_action_server_init(&action_server, &node, nullptr, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an invalid clock
  rcl_clock_t invalid_clock;
  invalid_clock.get_now = nullptr;
  ret = rcl_action_server_init(&action_server, &node, &invalid_clock, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null typesupport
  ret = rcl_action_server_init(&action_server, &node, &clock, nullptr, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null name
  ret = rcl_action_server_init(&action_server, &node, &clock, ts, nullptr, &options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an empty name
  const char * empty_action_name = "";
  ret = rcl_action_server_init(&action_server, &node, &clock, ts, empty_action_name, &options);
  EXPECT_EQ(ret, RCL_RET_ACTION_NAME_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an invalid name
  const char * invalid_action_name = "42";
  ret = rcl_action_server_init(&action_server, &node, &clock, ts, invalid_action_name, &options);
  EXPECT_EQ(ret, RCL_RET_ACTION_NAME_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null options
  ret = rcl_action_server_init(&action_server, &node, &clock, ts, action_name, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with valid arguments
  ret = rcl_action_server_init(&action_server, &node, &clock, ts, action_name, &options);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Try to initialize again
  ret = rcl_action_server_init(&action_server, &node, &clock, ts, action_name, &options);
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

  // Finalize clock
  ret = rcl_clock_fini(&clock);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Finalize node
  ret = rcl_node_fini(&node);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_shutdown(&context);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

class TestActionServer : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    context = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, &context);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(&this->node, "test_action_server_node", "", &context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_clock_init(RCL_ROS_TIME, &this->clock, &allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(
      test_msgs, Fibonacci);
    const rcl_action_server_options_t options = rcl_action_server_get_default_options();
    const char * action_name = "test_action_server_name";
    this->action_server = rcl_action_get_zero_initialized_server();
    ret = rcl_action_server_init(
      &this->action_server, &this->node, &this->clock, ts, action_name, &options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  }

  void TearDown() override
  {
    // Finalize
    rcl_ret_t ret = rcl_action_server_fini(&this->action_server, &this->node);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    ret = rcl_clock_fini(&this->clock);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    ret = rcl_node_fini(&this->node);
    EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    ret = rcl_shutdown(&context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void init_test_uuid0(uint8_t * uuid)
  {
    for (uint8_t i = 0; i < UUID_SIZE; ++i) {
      uuid[i] = i;
    }
  }

  void init_test_uuid1(uint8_t * uuid)
  {
    for (uint8_t i = 0; i < UUID_SIZE; ++i) {
      uuid[i] = 15 - i;
    }
  }

  rcl_action_server_t action_server;
  rcl_context_t context;
  rcl_node_t node;
  rcl_clock_t clock;
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

TEST_F(TestActionServer, test_action_accept_new_goal)
{
  // Initialize a goal info
  rcl_action_goal_info_t goal_info_in = rcl_action_get_zero_initialized_goal_info();
  init_test_uuid0(goal_info_in.uuid);

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
  EXPECT_TRUE(uuidcmp(goal_info_out.uuid, goal_info_in.uuid));
  size_t num_goals = 0u;
  rcl_action_goal_handle_t ** goal_handle_array = {nullptr};
  ret = rcl_action_server_get_goal_handles(&this->action_server, &goal_handle_array, &num_goals);
  ASSERT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(num_goals, 1u);
  EXPECT_NE(goal_handle_array, nullptr) << rcl_get_error_string().str;
  EXPECT_NE(goal_handle_array[0], nullptr) << rcl_get_error_string().str;

  // Accept with the same goal ID
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  EXPECT_EQ(goal_handle, nullptr);
  rcl_reset_error();

  // Accept a different goal
  goal_info_in = rcl_action_get_zero_initialized_goal_info();
  init_test_uuid1(goal_info_in.uuid);
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  EXPECT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  ret = rcl_action_goal_handle_get_info(goal_handle, &goal_info_out);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_TRUE(uuidcmp(goal_info_out.uuid, goal_info_in.uuid));
  ret = rcl_action_server_get_goal_handles(&this->action_server, &goal_handle_array, &num_goals);
  ASSERT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(num_goals, 2u);
  EXPECT_NE(goal_handle_array, nullptr) << rcl_get_error_string().str;
  EXPECT_NE(goal_handle_array[0], nullptr) << rcl_get_error_string().str;
  EXPECT_NE(goal_handle_array[1], nullptr) << rcl_get_error_string().str;
}

TEST_F(TestActionServer, test_action_clear_expired_goals)
{
  const size_t capacity = 1u;
  rcl_action_goal_info_t expired_goals[1u];
  size_t num_expired = 1u;
  // Clear expired goals with null action server
  rcl_ret_t ret = rcl_action_expire_goals(nullptr, expired_goals, capacity, &num_expired);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Clear with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_expire_goals(&invalid_action_server, expired_goals, capacity, &num_expired);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Clear with valid arguments
  ret = rcl_action_expire_goals(&this->action_server, expired_goals, capacity, &num_expired);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_EQ(num_expired, 0u);

  // Clear with valid arguments (optional num_expired)
  ret = rcl_action_expire_goals(&this->action_server, nullptr, 0u, nullptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Test with goals that actually expire
  // Set ROS time
  ASSERT_EQ(RCL_RET_OK, rcl_enable_ros_time_override(&this->clock));
  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&this->clock, RCUTILS_S_TO_NS(1)));
  // Accept a goal to create a new handle
  rcl_action_goal_info_t goal_info_in = rcl_action_get_zero_initialized_goal_info();
  init_test_uuid1(goal_info_in.uuid);
  rcl_action_goal_handle_t * goal_handle =
    rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  // Transition executing to aborted
  ASSERT_EQ(RCL_RET_OK, rcl_action_update_goal_state(goal_handle, GOAL_EVENT_EXECUTE));
  ASSERT_EQ(RCL_RET_OK, rcl_action_update_goal_state(goal_handle, GOAL_EVENT_SET_ABORTED));
  // Set time to something far in the future
  ASSERT_EQ(RCL_RET_OK, rcl_set_ros_time_override(&this->clock, RCUTILS_S_TO_NS(99999)));
  // Clear with valid arguments
  ret = rcl_action_expire_goals(&this->action_server, expired_goals, capacity, &num_expired);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_EQ(num_expired, 1u);
  EXPECT_TRUE(uuidcmp(expired_goals[0].uuid, goal_info_in.uuid));
}

TEST_F(TestActionServer, test_action_process_cancel_request)
{
  rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();

  // Process cancel request with null action server
  rcl_ret_t ret = rcl_action_process_cancel_request(nullptr, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Process cancel request with null request message
  ret = rcl_action_process_cancel_request(&this->action_server, nullptr, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Process cancel request with null response message
  ret = rcl_action_process_cancel_request(&this->action_server, &cancel_request, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Process cancel request with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_process_cancel_request(
    &invalid_action_server, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Process cancel request with valid arguments (but no goals to cancel)
  ret = rcl_action_process_cancel_request(
    &this->action_server, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_EQ(cancel_response.msg.goals_canceling.data, nullptr);
  EXPECT_EQ(cancel_response.msg.goals_canceling.size, 0u);
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
  ret = rcl_action_goal_status_array_fini(&status_array);
  ASSERT_EQ(ret, RCL_RET_OK);

  // Add a goal before getting the status array
  rcl_action_goal_info_t goal_info_in = rcl_action_get_zero_initialized_goal_info();
  init_test_uuid0(goal_info_in.uuid);
  rcl_action_goal_handle_t * goal_handle;
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
  ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(status_array.msg.status_list.data, nullptr);
  EXPECT_EQ(status_array.msg.status_list.size, 1u);
  rcl_action_goal_info_t * goal_info_out = &status_array.msg.status_list.data[0].goal_info;
  EXPECT_TRUE(uuidcmp(goal_info_out->uuid, goal_info_in.uuid));
  ret = rcl_action_goal_status_array_fini(&status_array);
  ASSERT_EQ(ret, RCL_RET_OK);

  // Add nine more goals
  for (int i = 1; i < 10; ++i) {
    for (int j = 0; j < UUID_SIZE; ++j) {
      goal_info_in.uuid[j] = static_cast<uint8_t>(i + j);
    }
    goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
    ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  }
  ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(status_array.msg.status_list.data, nullptr);
  ASSERT_EQ(status_array.msg.status_list.size, 10u);
  for (int i = 0; i < 10; ++i) {
    goal_info_out = &status_array.msg.status_list.data[i].goal_info;
    for (int j = 0; j < UUID_SIZE; ++j) {
      EXPECT_EQ(goal_info_out->uuid[j], i + j);
    }
  }
  ret = rcl_action_goal_status_array_fini(&status_array);
  ASSERT_EQ(ret, RCL_RET_OK);
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

class TestActionServerCancelPolicy : public TestActionServer
{
protected:
  void SetUp() override
  {
    TestActionServer::SetUp();
    // Add several goals
    rcl_action_goal_info_t goal_info_in = rcl_action_get_zero_initialized_goal_info();
    rcl_action_goal_handle_t * goal_handle;
    rcl_ret_t ret;
    for (int i = 0; i < NUM_GOALS; ++i) {
      for (int j = 0; j < UUID_SIZE; ++j) {
        goal_info_in.uuid[j] = static_cast<uint8_t>(i + j);
      }
      goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info_in);
      ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
      goal_infos_out[i] = rcl_action_get_zero_initialized_goal_info();
      ret = rcl_action_goal_handle_get_info(goal_handle, &goal_infos_out[i]);
      ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
      // Sleep so goals have different acceptance times
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  }

  void TearDown() override
  {
    TestActionServer::TearDown();
  }

  static const int NUM_GOALS = 10;
  rcl_action_goal_info_t goal_infos_out[NUM_GOALS];
};

TEST_F(TestActionServerCancelPolicy, test_action_process_cancel_request_all_goals)
{
  // Request to cancel all goals
  rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
  cancel_request.goal_info.stamp.sec = 0;
  cancel_request.goal_info.stamp.nanosec = 0u;
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();
  rcl_ret_t ret = rcl_action_process_cancel_request(
    &this->action_server, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(cancel_response.msg.goals_canceling.data, nullptr);
  ASSERT_EQ(cancel_response.msg.goals_canceling.size, (size_t)NUM_GOALS);
  rcl_action_goal_info_t * goal_info_out;
  for (int i = 0; i < NUM_GOALS; ++i) {
    goal_info_out = &cancel_response.msg.goals_canceling.data[i];
    for (int j = 0; j < UUID_SIZE; ++j) {
      EXPECT_EQ(goal_info_out->uuid[j], static_cast<uint8_t>(i + j));
    }
  }
}

TEST_F(TestActionServerCancelPolicy, test_action_process_cancel_request_single_goal)
{
  // Request to cancel a specific goal
  rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
  init_test_uuid0(cancel_request.goal_info.uuid);
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();
  rcl_ret_t ret = rcl_action_process_cancel_request(
    &this->action_server, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(cancel_response.msg.goals_canceling.data, nullptr);
  ASSERT_EQ(cancel_response.msg.goals_canceling.size, 1u);
  rcl_action_goal_info_t * goal_info = &cancel_response.msg.goals_canceling.data[0];
  EXPECT_TRUE(uuidcmp(goal_info->uuid, cancel_request.goal_info.uuid));
}

TEST_F(TestActionServerCancelPolicy, test_action_process_cancel_request_by_time)
{
  // Request to cancel all goals at and before a specific time
  const size_t time_index = 7;
  rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
  cancel_request.goal_info = this->goal_infos_out[time_index];
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();
  rcl_ret_t ret = rcl_action_process_cancel_request(
    &this->action_server, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(cancel_response.msg.goals_canceling.data, nullptr);
  ASSERT_EQ(cancel_response.msg.goals_canceling.size, time_index + 1);  // goals at indices [0, 7]
  rcl_action_goal_info_t * goal_info_out;
  for (size_t i = 0; i < cancel_response.msg.goals_canceling.size; ++i) {
    goal_info_out = &cancel_response.msg.goals_canceling.data[i];
    for (size_t j = 0; j < UUID_SIZE; ++j) {
      EXPECT_EQ(goal_info_out->uuid[j], static_cast<uint8_t>(i + j));
    }
  }
}

TEST_F(TestActionServerCancelPolicy, test_action_process_cancel_request_by_time_and_id)
{
  // Request to cancel a specific goal by ID and all goals at and before a specific time
  const size_t goal_index = 9;
  const size_t time_index = 2;
  rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
  cancel_request.goal_info = this->goal_infos_out[time_index];
  for (int i = 0; i < UUID_SIZE; ++i) {
    cancel_request.goal_info.uuid[i] = static_cast<uint8_t>(i + goal_index);
  }
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();
  rcl_ret_t ret = rcl_action_process_cancel_request(
    &this->action_server, &cancel_request, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(cancel_response.msg.goals_canceling.data, nullptr);
  const size_t num_goals_canceling = cancel_response.msg.goals_canceling.size;
  ASSERT_EQ(num_goals_canceling, time_index + 2);  // goals at indices [0, 2] and 8
  rcl_action_goal_info_t * goal_info_out;
  for (size_t i = 0; i < num_goals_canceling - 1; ++i) {
    goal_info_out = &cancel_response.msg.goals_canceling.data[i];
    for (size_t j = 0; j < UUID_SIZE; ++j) {
      EXPECT_EQ(goal_info_out->uuid[j], static_cast<uint8_t>(i + j));
    }
  }
  goal_info_out = &cancel_response.msg.goals_canceling.data[num_goals_canceling - 1];
  EXPECT_TRUE(uuidcmp(goal_info_out->uuid, cancel_request.goal_info.uuid));
}
