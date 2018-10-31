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
  EXPECT_NE(goal_handle.impl, nullptr);
  EXPECT_NE(goal_handle.impl->info, nullptr);

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

    // Populate goal info
    // TODO(jacobperron): Do this in a goal_info_init() function and pass to goal handle
    for (int i = 0; i < 16; ++i) {
      this->goal_handle_ptr->impl->info.uuid[i] = static_cast<uint8_t>(i);
    }
    this->goal_handle_ptr->impl->info.stamp.sec = 123;
    this->goal_handle_ptr->impl->info.stamp.nanosec = 456u;
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_action_goal_handle_fini(this->goal_handle_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    delete this->goal_handle_ptr;
  }
};

TEST_F(TestGoalHandleFixture, test_goal_handle_update_state_valid)
{
  // Starting state ACCEPTED
  this->goal_handle_ptr->impl->state = GOAL_STATE_ACCEPTED;
  rcl_ret_t ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_EXECUTE);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_EXECUTING);
  this->goal_handle_ptr->impl->state = GOAL_STATE_ACCEPTED;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_CANCEL);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELING);

  // Starting state EXECUTING
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_CANCEL);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELING);
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_SUCCEEDED);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_SUCCEEDED);
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_CANCELED);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELED);
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_ABORTED);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_ABORTED);

  // Starting state CANCELING
  this->goal_handle_ptr->impl->state = GOAL_STATE_CANCELING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_CANCELED);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELED);
  this->goal_handle_ptr->impl->state = GOAL_STATE_CANCELING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_SUCCEEDED);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_SUCCEEDED);
  this->goal_handle_ptr->impl->state = GOAL_STATE_CANCELING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_ABORTED);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_ABORTED);
}

TEST_F(TestGoalHandleFixture, test_goal_handle_update_state_invalid)
{
  // Check with invalid goal handle
  rcl_ret_t ret = rcl_action_update_goal_state(nullptr, GOAL_EVENT_EXECUTE);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID);

  // Check with invalid goal event
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_NUM);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  // State should not change
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_EXECUTING);

  // Starting state UNKNOWN
  for (int i = 0; i < GOAL_EVENT_NUM; ++i) {
    this->goal_handle_ptr->impl->state = GOAL_STATE_UNKNOWN;
    ret = rcl_action_update_goal_state(this->goal_handle_ptr, i);
    EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
    EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_UNKNOWN);
  }

  // Starting state ACCEPTED
  this->goal_handle_ptr->impl->state = GOAL_STATE_ACCEPTED;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_SUCCEEDED);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_ACCEPTED);
  this->goal_handle_ptr->impl->state = GOAL_STATE_ACCEPTED;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_CANCELED);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_ACCEPTED);
  this->goal_handle_ptr->impl->state = GOAL_STATE_ACCEPTED;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_ABORTED);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_ACCEPTED);

  // Starting state EXECUTING
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_EXECUTE);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_EXECUTING);
  this->goal_handle_ptr->impl->state = GOAL_STATE_EXECUTING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_SET_CANCELED);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_EXECUTING);

  // Starting state CANCELING
  this->goal_handle_ptr->impl->state = GOAL_STATE_CANCELING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_EXECUTE);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELING);
  this->goal_handle_ptr->impl->state = GOAL_STATE_CANCELING;
  ret = rcl_action_update_goal_state(this->goal_handle_ptr, GOAL_EVENT_CANCEL);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
  EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELING);

  // Starting state SUCCEEDED
  for (int i = 0; i < GOAL_EVENT_NUM; ++i) {
    this->goal_handle_ptr->impl->state = GOAL_STATE_SUCCEEDED;
    ret = rcl_action_update_goal_state(this->goal_handle_ptr, i);
    EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
    EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_SUCCEEDED);
  }

  // Starting state CANCELED
  for (int i = 0; i < GOAL_EVENT_NUM; ++i) {
    this->goal_handle_ptr->impl->state = GOAL_STATE_CANCELED;
    ret = rcl_action_update_goal_state(this->goal_handle_ptr, i);
    EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
    EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_CANCELED);
  }

  // Starting state ABORTED
  for (int i = 0; i < GOAL_EVENT_NUM; ++i) {
    this->goal_handle_ptr->impl->state = GOAL_STATE_ABORTED;
    ret = rcl_action_update_goal_state(this->goal_handle_ptr, i);
    EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
    EXPECT_EQ(this->goal_handle_ptr->impl->state, GOAL_STATE_ABORTED);
  }
}

TEST_F(TestGoalHandleFixture, test_goal_handle_get_info)
{
  // Check with invalid goal handle
  rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();
  rcl_ret_t ret = rcl_action_goal_handle_get_info(nullptr, &goal_info);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID);

  // Check with invalid goal info
  ret = rcl_action_goal_handle_get_info(this->goal_handle_ptr, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Check with valid arguments
  ret = rcl_action_goal_handle_get_info(this->goal_handle_ptr, &goal_info);
  EXPECT_EQ(ret, RCL_RET_OK);
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info.uuid[i], static_cast<uint8_t>(i));
  }
  EXPECT_EQ(goal_info.stamp.sec, 123);
  EXPECT_EQ(goal_info.stamp.nanosec, 456u);
}

TEST_F(TestGoalHandleFixture, test_goal_handle_get_status)
{
  // Check with invalid goal handle
  rcl_action_goal_state_t state = 42;
  rcl_ret_t ret = rcl_action_goal_handle_get_status(nullptr, &state);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID);
  // Confirm state did not change
  EXPECT_EQ(state, 42);

  // Check with invalid state
  ret = rcl_action_goal_handle_get_status(this->goal_handle_ptr, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT);

  // Check setting and getting some states
  this->goal_handle_ptr->info.state = GOAL_STATE_ACCEPTED;
  ret = rcl_action_goal_handle_get_status(this->goal_handle_ptr, &state);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(state, GOAL_STATE_ACCEPTED);
  this->goal_handle_ptr->info.state = GOAL_STATE_ABORTED;
  ret = rcl_action_goal_handle_get_status(this->goal_handle_ptr, &state);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(state, GOAL_STATE_ABORTED);
  this->goal_handle_ptr->info.state = GOAL_STATE_CANCELING;
  ret = rcl_action_goal_handle_get_status(this->goal_handle_ptr, &state);
  EXPECT_EQ(ret, RCL_RET_OK);
  EXPECT_EQ(state, GOAL_STATE_CANCELING);
}

TEST_F(TestGoalHandleFixture, test_goal_handle_is_active)
{
  // Check with invalid goal handle
  bool is_active = rcl_action_goal_handle_is_active(nullptr, NULL);
  EXPECT_FALSE(is_active) << rcl_get_error_string_safe();

  // Check active states
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_ACCEPT;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_TRUE(is_active) << rcl_get_error_string_safe();
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_EXECUTING;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_TRUE(is_active) << rcl_get_error_string_safe();
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_CANCELING;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_TRUE(is_active) << rcl_get_error_string_safe();

  // Check inactive states
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_SUCCEEDED;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_FALSE(is_active) << rcl_get_error_string_safe();
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_CANCELED;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_FALSE(is_active) << rcl_get_error_string_safe();
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_ABORTED;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_FALSE(is_active) << rcl_get_error_string_safe();
  this->goal_handle_ptr->impl->info.state = GOAL_STATE_UNKNOWN;
  is_active = rcl_action_goal_handle_is_active(this->goal_handle_ptr, NULL);
  EXPECT_FALSE(is_active) << rcl_get_error_string_safe();
}

TEST_F(TestGoalHandleFixture, test_goal_handle_is_valid)
{
  // Check null goal handle
  bool is_valid = rcl_action_goal_handle_is_valid(nullptr, NULL);
  EXPECT_FALSE(is_valid) << rcl_get_error_string_safe();

  // Check valid goal handle
  is_valid = rcl_action_goal_handle_is_valid(this->goal_handle, NULL);
  EXPECT_TRUE(is_valid) << rcl_get_error_string_safe();

  // Check null goal info member
  this->goal_handle->imple.info = NULL;
  is_valid = rcl_action_goal_handle_is_valid(this->goal_handle, NULL);
  EXPECT_FALSE(is_valid) << rcl_get_error_string_safe();
}
