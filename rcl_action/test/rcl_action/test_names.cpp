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

#include "rcl_action/names.h"

#include "rcl/allocator.h"
#include "rcl/types.h"


TEST(TestActionNames, test_action_goal_service_name)
{
  rcl_ret_t ret;
  const char * const action_name = "test_it";
  rcl_allocator_t default_allocator = rcl_get_default_allocator();

  char * action_goal_service_name;
  ret = rcl_action_get_goal_service_name(
    action_name, default_allocator, &action_goal_service_name);
  ASSERT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("test_it/_action/send_goal", action_goal_service_name);
  default_allocator.deallocate(action_goal_service_name, default_allocator.state);
}

TEST(TestActionNames, test_action_cancel_service_name)
{
  rcl_ret_t ret;
  const char * const action_name = "test_it";
  rcl_allocator_t default_allocator = rcl_get_default_allocator();

  char * action_cancel_service_name;
  ret = rcl_action_get_cancel_service_name(
    action_name, default_allocator, &action_cancel_service_name);
  ASSERT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("test_it/_action/cancel_goal", action_cancel_service_name);
  default_allocator.deallocate(action_cancel_service_name, default_allocator.state);
}

TEST(TestActionNames, test_action_result_service_name)
{
  rcl_ret_t ret;
  const char * const action_name = "test_it";
  rcl_allocator_t default_allocator = rcl_get_default_allocator();

  char * action_result_service_name;
  ret = rcl_action_get_result_service_name(
    action_name, default_allocator, &action_result_service_name);
  ASSERT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("test_it/_action/get_result", action_result_service_name);
  default_allocator.deallocate(action_result_service_name, default_allocator.state);
}

TEST(TestActionNames, test_action_feedback_topic_name)
{
  rcl_ret_t ret;
  const char * const action_name = "test_it";
  rcl_allocator_t default_allocator = rcl_get_default_allocator();

  char * action_feedback_topic_name;
  ret = rcl_action_get_feedback_topic_name(
    action_name, default_allocator, &action_feedback_topic_name);
  ASSERT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("test_it/_action/feedback", action_feedback_topic_name);
  default_allocator.deallocate(action_feedback_topic_name, default_allocator.state);
}

TEST(TestActionNames, test_action_status_topic_name)
{
  rcl_ret_t ret;
  const char * const action_name = "test_it";
  rcl_allocator_t default_allocator = rcl_get_default_allocator();

  char * action_status_topic_name;
  ret = rcl_action_get_status_topic_name(
    action_name, default_allocator, &action_status_topic_name);
  ASSERT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("test_it/_action/status", action_status_topic_name);
  default_allocator.deallocate(action_status_topic_name, default_allocator.state);
}
