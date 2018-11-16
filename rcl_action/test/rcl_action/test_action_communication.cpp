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

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

// TODO(jacobperron): Add action client to complete tests
class CLASSNAME (TestActionCommunication, RMW_IMPLEMENTATION) : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rcl_ret_t ret = rcl_init(0, nullptr, allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(&this->node, "test_action_communication_node", "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_clock_init(RCL_STEADY_TIME, &this->clock, &allocator);
    const rosidl_action_type_support_t * ts = ROSIDL_GET_ACTION_TYPE_SUPPORT(
      test_msgs, Fibonacci);
    const rcl_action_server_options_t options = rcl_action_server_get_default_options();
    const char * action_name = "test_action_commmunication_name";
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
    EXPECT_EQ(ret, RCL_RET_OK);
    ret = rcl_node_fini(&this->node);
    EXPECT_EQ(ret, RCL_RET_OK);
    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void init_test_uuid0(uint8_t * uuid)
  {
    for (uint8_t i = 0; i < 16; ++i) {
      uuid[i] = i;
    }
  }

  void init_test_uuid1(uint8_t * uuid)
  {
    for (uint8_t i = 0; i < 16; ++i) {
      uuid[i] = 15 - i;
    }
  }

  rcl_action_server_t action_server;
  rcl_node_t node;
  rcl_clock_t clock;
};  // class TestActionCommunication

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_take_goal_request)
{
  test_msgs__action__Fibonacci_Goal_Request goal_request;
  test_msgs__action__Fibonacci_Goal_Request__init(&goal_request);

  // Take request with null action server
  rcl_ret_t ret = rcl_action_take_goal_request(nullptr, &goal_request);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Take request with null message
  ret = rcl_action_take_goal_request(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Take request with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_take_goal_request(&invalid_action_server, &goal_request);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Take with valid arguments
  // TODO(jacobperron): Send a request from a client
  // ret = rcl_action_take_goal_request(&this->action_server, &goal_request);
  // EXPECT_EQ(ret, RCL_RET_OK);

  test_msgs__action__Fibonacci_Goal_Request__fini(&goal_request);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_send_goal_response)
{
  test_msgs__action__Fibonacci_Goal_Response goal_response;
  test_msgs__action__Fibonacci_Goal_Response__init(&goal_response);

  // Send response with null action server
  rcl_ret_t ret = rcl_action_send_goal_response(nullptr, &goal_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Send response with null message
  ret = rcl_action_send_goal_response(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Send response with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_send_goal_response(&invalid_action_server, &goal_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Send with valid arguments
  // TODO(jacobperron): Check with client on receiving end
  ret = rcl_action_send_goal_response(&this->action_server, &goal_response);
  EXPECT_EQ(ret, RCL_RET_OK);

  test_msgs__action__Fibonacci_Goal_Response__fini(&goal_response);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_take_cancel_request)
{
  action_msgs__srv__CancelGoal_Request cancel_request;
  action_msgs__srv__CancelGoal_Request__init(&cancel_request);

  // Take request with null action server
  rcl_ret_t ret = rcl_action_take_cancel_request(nullptr, &cancel_request);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Take request with null message
  ret = rcl_action_take_cancel_request(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Take request with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_take_cancel_request(&invalid_action_server, &cancel_request);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Take with valid arguments
  // TODO(jacobperron): Send a request from a client
  // ret = rcl_action_take_cancel_request(&this->action_server, &cancel_request);
  // EXPECT_EQ(ret, RCL_RET_OK);

  action_msgs__srv__CancelGoal_Request__fini(&cancel_request);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_send_cancel_response)
{
  action_msgs__srv__CancelGoal_Response cancel_response;
  action_msgs__srv__CancelGoal_Response__init(&cancel_response);

  // Send response with null action server
  rcl_ret_t ret = rcl_action_send_cancel_response(nullptr, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Send response with null message
  ret = rcl_action_send_cancel_response(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Send response with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_send_cancel_response(&invalid_action_server, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Send with valid arguments
  // TODO(jacobperron): Check with client on receiving end
  ret = rcl_action_send_cancel_response(&this->action_server, &cancel_response);
  EXPECT_EQ(ret, RCL_RET_OK);

  action_msgs__srv__CancelGoal_Response__fini(&cancel_response);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_take_result_request)
{
  test_msgs__action__Fibonacci_Result_Request result_request;
  test_msgs__action__Fibonacci_Result_Request__init(&result_request);

  // Take request with null action server
  rcl_ret_t ret = rcl_action_take_result_request(nullptr, &result_request);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Take request with null message
  ret = rcl_action_take_result_request(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Take request with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_take_result_request(&invalid_action_server, &result_request);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Take with valid arguments
  // TODO(jacobperron): Send a request from a client
  // ret = rcl_action_take_result_request(&this->action_server, &result_request);
  // EXPECT_EQ(ret, RCL_RET_OK);

  test_msgs__action__Fibonacci_Result_Request__fini(&result_request);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_send_result_response)
{
  test_msgs__action__Fibonacci_Result_Response result_response;
  test_msgs__action__Fibonacci_Result_Response__init(&result_response);

  // Send response with null action server
  rcl_ret_t ret = rcl_action_send_result_response(nullptr, &result_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Send response with null message
  ret = rcl_action_send_result_response(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Send response with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_send_result_response(&invalid_action_server, &result_response);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Send with valid arguments
  // TODO(jacobperron): Check with client on receiving end
  ret = rcl_action_send_result_response(&this->action_server, &result_response);
  EXPECT_EQ(ret, RCL_RET_OK);

  test_msgs__action__Fibonacci_Result_Response__fini(&result_response);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_publish_feedback)
{
  test_msgs__action__Fibonacci_Feedback feedback;
  test_msgs__action__Fibonacci_Feedback__init(&feedback);

  // Publish feedback with null action server
  rcl_ret_t ret = rcl_action_publish_feedback(nullptr, &feedback);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Publish feedback with null message
  ret = rcl_action_publish_feedback(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Publish feedback with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_publish_feedback(&invalid_action_server, &feedback);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Publish feedback with valid arguments
  // TODO(jacobperron): Check with client on receiving end
  ret = rcl_action_publish_feedback(&this->action_server, &feedback);
  EXPECT_EQ(ret, RCL_RET_OK);

  test_msgs__action__Fibonacci_Feedback__fini(&feedback);
}

TEST_F(CLASSNAME(TestActionCommunication, RMW_IMPLEMENTATION), test_publish_status)
{
  rcl_action_goal_status_array_t status_array =
    rcl_action_get_zero_initialized_goal_status_array();
  rcl_ret_t ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Publish status with null action server
  ret = rcl_action_publish_status(nullptr, &status_array.msg);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Publish status with null message
  ret = rcl_action_publish_status(&this->action_server, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);
  rcl_reset_error();

  // Publish status with invalid action server
  rcl_action_server_t invalid_action_server = rcl_action_get_zero_initialized_server();
  ret = rcl_action_publish_status(&invalid_action_server, &status_array.msg);
  EXPECT_EQ(ret, RCL_RET_ACTION_SERVER_INVALID);
  rcl_reset_error();

  // Publish status with valid arguments (but empty array)
  // TODO(jacobperron): Check with client on receiving end
  ret = rcl_action_publish_status(&this->action_server, &status_array.msg);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  // Add a goal before publishing the status array
  ret = rcl_action_goal_status_array_fini(&status_array);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();
  rcl_action_goal_handle_t * goal_handle;
  goal_handle = rcl_action_accept_new_goal(&this->action_server, &goal_info);
  ASSERT_NE(goal_handle, nullptr) << rcl_get_error_string().str;
  ret = rcl_action_get_goal_status_array(&this->action_server, &status_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  // Publish status with valid arguments (one goal in array)
  // TODO(jacobperron): Check with client on receiving end
  ret = rcl_action_publish_status(&this->action_server, &status_array.msg);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  ret = rcl_action_goal_status_array_fini(&status_array);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
}
