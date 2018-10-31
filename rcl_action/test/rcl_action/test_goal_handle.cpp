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

#include "rcl_action/goal_handle.h"

#include "rcl/error_handling.h"


TEST(TestGoalHandle, test_goal_handle_init_fini)
{
  // Initialize with null goal handle
  rcl_ret_t ret = rcl_action_goal_handle_init(nullptr, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID);

  // Initialize with an invalid allocator
  rcl_action_goal_handle_t goal_handle = rcl_action_get_zero_initialized_goal_handle();
  EXPECT_EQ(goal_handle.impl, nullptr);
  rcl_allocator_t invalid_allocator = rcl_get_default_allocator();
  invalid_allocator.allocate = nullptr;
  ret = rcl_action_goal_handle_init(&goal_handle, invalid_allocator);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Initialize with valid goal handle and allocator
  ret = rcl_action_goal_handle_init(&goal_handle, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK);

  // Try to initialize again
  ret = rcl_action_goal_handle_init(&goal_handle, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_ALREADY_INIT);

  // Finalize with null goal handle
  ret = rcl_action_goal_handle_fini(nullptr);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID);

  // Finalize with valid goal handle
  ret = rcl_action_goal_handle_fini(&goal_handle);
  EXPECT_EQ(ret, RCL_RET_OK);
}

class TestGoalHandleFixture : public ::testing::Test
{
public:
  rcl_action_goal_handle_t * goal_handle_ptr;

  void SetUp()
  {
    this->goal_handle_ptr = new rcl_action_goal_handle_t;
    *this->goal_handle_ptr = rcl_action_get_zero_initialized_goal_handle();
    rcl_ret_t ret = rcl_action_goal_handle_init(this->goal_handle_ptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_action_goal_handle_fini(this->goal_handle_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    delete this->goal_handle_ptr;
  }
};

TEST(TestGoalHandle, test_goal_handle_update_state_valid)
{
}

TEST(TestGoalHandle, test_goal_handle_update_state_invalid)
{
}
