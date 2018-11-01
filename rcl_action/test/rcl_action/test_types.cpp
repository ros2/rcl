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

#include "rcl_action/types.h"

TEST(TestActionTypes, test_get_zero_inititalized_goal_info)
{
  rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();
  ASSERT_EQ(sizeof(goal_info.uuid) / sizeof(uint8_t), 16u);
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info.uuid[i], 0u);
  }
  EXPECT_EQ(goal_info.stamp.sec, 0);
  EXPECT_EQ(goal_info.stamp.nanosec, 0u);

  // Modify the first and get another zero initialized goal info struct
  // to confirm they are independent objects
  for (int i = 0; i < 16; ++i) {
    goal_info.uuid[i] = static_cast<uint8_t>(i);
  }
  goal_info.stamp.sec = 1234;
  goal_info.stamp.nanosec = 4567u;
  rcl_action_goal_info_t another_goal_info = rcl_action_get_zero_initialized_goal_info();
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info.uuid[i], i);
    EXPECT_EQ(another_goal_info.uuid[i], 0u);
  }
  EXPECT_EQ(goal_info.stamp.sec, 1234);
  EXPECT_EQ(goal_info.stamp.nanosec, 4567u);
  EXPECT_EQ(another_goal_info.stamp.sec, 0);
  EXPECT_EQ(another_goal_info.stamp.nanosec, 0u);
}

TEST(TestActionTypes, test_get_zero_initialized_goal_status_array)
{
  rcl_action_goal_status_array_t status_array =
    rcl_action_get_zero_initialized_goal_status_array();
  EXPECT_EQ(status_array.status_list.size, 0u);
  EXPECT_EQ(status_array.status_list.data, nullptr);
}

TEST(TestActionTypes, test_get_zero_inititalized_cancel_request)
{
  rcl_action_cancel_request_t cancel_request = rcl_action_get_zero_initialized_cancel_request();
  ASSERT_EQ(sizeof(cancel_request.goal_info.uuid) / sizeof(uint8_t), 16u);
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(cancel_request.goal_info.uuid[i], 0u);
  }
  EXPECT_EQ(cancel_request.goal_info.stamp.sec, 0);
  EXPECT_EQ(cancel_request.goal_info.stamp.nanosec, 0u);
}

TEST(TestActionTypes, test_get_zero_initialized_cancel_response)
{
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();
  EXPECT_EQ(cancel_response.goals_canceling.size, 0u);
  EXPECT_EQ(cancel_response.goals_canceling.data, nullptr);
}

TEST(TestActionTypes, test_init_fini_goal_status_array)
{
  const size_t num_status = 3;
  // Initialize with invalid status array
  rcl_ret_t ret = rcl_action_goal_status_array_init(
    nullptr,
    num_status,
    rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Initialize with invalid allocator
  rcl_allocator_t invalid_allocator = rcl_get_default_allocator();
  invalid_allocator.allocate = nullptr;
  rcl_action_goal_status_array_t status_array =
    rcl_action_get_zero_initialized_goal_status_array();
  ASSERT_EQ(status_array.status_list.size, 0u);
  ret = rcl_action_goal_status_array_init(&status_array, num_status, invalid_allocator);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Initialize with valid arguments
  status_array = rcl_action_get_zero_initialized_goal_status_array();
  ASSERT_EQ(status_array.status_list.size, 0u);
  ret = rcl_action_goal_status_array_init(&status_array, num_status, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(num_status, status_array.status_list.size);
  EXPECT_NE(nullptr, status_array.status_list.data);

  // Finalize with invalid status array
  ret = rcl_action_goal_status_array_fini(nullptr, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Finalize with invalid allocator
  ret = rcl_action_goal_status_array_fini(&status_array, invalid_allocator);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Finalize with valid arguments
  ret = rcl_action_goal_status_array_fini(&status_array, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK);
}

TEST(TestActionTypes, test_init_fini_cancel_response)
{
  const size_t num_goals_canceling = 3;
  // Initialize with invalid cancel response
  rcl_ret_t ret = rcl_action_cancel_response_init(
    nullptr,
    num_goals_canceling,
    rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Initialize with invalid allocator
  rcl_allocator_t invalid_allocator = rcl_get_default_allocator();
  invalid_allocator.allocate = nullptr;
  rcl_action_cancel_response_t cancel_response = rcl_action_get_zero_initialized_cancel_response();
  ASSERT_EQ(cancel_response.goals_canceling.size, 0u);
  ret = rcl_action_cancel_response_init(&cancel_response, num_goals_canceling, invalid_allocator);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Initialize with valid arguments
  cancel_response = rcl_action_get_zero_initialized_cancel_response();
  ASSERT_EQ(cancel_response.goals_canceling.size, 0u);
  ret = rcl_action_cancel_response_init(
    &cancel_response,
    num_goals_canceling,
    rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(num_goals_canceling, cancel_response.goals_canceling.size);
  EXPECT_NE(nullptr, cancel_response.goals_canceling.data);

  // Finalize with invalid cancel response
  ret = rcl_action_cancel_response_fini(nullptr, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Finalize with invalid allocator
  ret = rcl_action_cancel_response_fini(&cancel_response, invalid_allocator);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Finalize with valid arguments
  ret = rcl_action_cancel_response_fini(&cancel_response, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK);
}
