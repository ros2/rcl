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
#include <string>
#include <utility>
#include <vector>

#include "rcl_action/goal_handle.h"
#include "rcl_action/types.h"

#include "rcl/error_handling.h"

TEST(TestGoalHandle, test_goal_handle_init_fini)
{
  rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();

  // Initialize with a null goal handle
  rcl_ret_t ret = rcl_action_goal_handle_init(nullptr, &goal_info, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with a null goal info
  rcl_action_goal_handle_t goal_handle = rcl_action_get_zero_initialized_goal_handle();
  EXPECT_EQ(goal_handle.impl, nullptr);
  ret = rcl_action_goal_handle_init(&goal_handle, nullptr, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with an invalid allocator
  rcl_allocator_t invalid_allocator = (rcl_allocator_t)rcutils_get_zero_initialized_allocator();
  ret = rcl_action_goal_handle_init(&goal_handle, &goal_info, invalid_allocator);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Initialize with valid goal handle and allocator
  ret = rcl_action_goal_handle_init(&goal_handle, &goal_info, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_NE(goal_handle.impl, nullptr);

  // Try to initialize again
  ret = rcl_action_goal_handle_init(&goal_handle, &goal_info, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_ALREADY_INIT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with null goal handle
  ret = rcl_action_goal_handle_fini(nullptr);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Finalize with valid goal handle
  ret = rcl_action_goal_handle_fini(&goal_handle);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
}

TEST(TestGoalHandle, test_goal_handle_is_valid)
{
  // Check null goal handle
  bool is_valid = rcl_action_goal_handle_is_valid(nullptr);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check uninitialized goal handle
  rcl_action_goal_handle_t goal_handle = rcl_action_get_zero_initialized_goal_handle();
  is_valid = rcl_action_goal_handle_is_valid(&goal_handle);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check valid goal handle
  rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();
  rcl_ret_t ret = rcl_action_goal_handle_init(
    &goal_handle, &goal_info, rcl_get_default_allocator());
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  is_valid = rcl_action_goal_handle_is_valid(&goal_handle);
  EXPECT_TRUE(is_valid) << rcl_get_error_string().str;

  // Finalize
  ret = rcl_action_goal_handle_fini(&goal_handle);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
}

TEST(TestGoalHandle, test_goal_handle_get_info)
{
  // Initialize a goal info message to test
  rcl_action_goal_info_t goal_info_input = rcl_action_get_zero_initialized_goal_info();
  for (int i = 0; i < 16; ++i) {
    goal_info_input.uuid[i] = static_cast<uint8_t>(i);
  }
  goal_info_input.stamp.sec = 123;
  goal_info_input.stamp.nanosec = 456u;

  // Check with null goal handle
  rcl_action_goal_info_t goal_info_output = rcl_action_get_zero_initialized_goal_info();
  rcl_ret_t ret = rcl_action_goal_handle_get_info(nullptr, &goal_info_output);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with invalid goal handle
  rcl_action_goal_handle_t goal_handle = rcl_action_get_zero_initialized_goal_handle();
  ret = rcl_action_goal_handle_get_info(&goal_handle, &goal_info_output);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with null goal info
  ret = rcl_action_goal_handle_init(&goal_handle, &goal_info_input, rcl_get_default_allocator());
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ret = rcl_action_goal_handle_get_info(&goal_handle, nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with valid arguments
  ret = rcl_action_goal_handle_get_info(&goal_handle, &goal_info_output);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(goal_info_input.uuid[i], goal_info_output.uuid[i]);
  }
  EXPECT_EQ(goal_info_input.stamp.sec, goal_info_output.stamp.sec);
  EXPECT_EQ(goal_info_input.stamp.nanosec, goal_info_output.stamp.nanosec);

  // Finalize
  ret = rcl_action_goal_handle_fini(&goal_handle);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
}

TEST(TestGoalHandle, test_goal_handle_update_state_invalid)
{
  // Check with null argument
  rcl_ret_t ret = rcl_action_update_goal_state(nullptr, GOAL_EVENT_EXECUTE);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with invalid goal handle
  rcl_action_goal_handle_t goal_handle = rcl_action_get_zero_initialized_goal_handle();
  ret = rcl_action_update_goal_state(&goal_handle, GOAL_EVENT_NUM_EVENTS);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_HANDLE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  // Check with invalid goal event
  rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();
  ret = rcl_action_goal_handle_init(&goal_handle, &goal_info, rcl_get_default_allocator());
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ret = rcl_action_update_goal_state(&goal_handle, GOAL_EVENT_NUM_EVENTS);
  EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();
}

using EventStatePair = std::pair<rcl_action_goal_event_t, rcl_action_goal_state_t>;
using StateTransitionSequence = std::vector<EventStatePair>;
const std::vector<std::string> event_strs = {
  "EXECUTE", "CANCEL", "SET_SUCCEEDED", "SET_ABORTED", "SET_CANCELED"};

class TestGoalHandleStateTransitionSequence
  : public ::testing::TestWithParam<StateTransitionSequence>
{
public:
  static std::string print_sequence_param_name(
    const testing::TestParamInfo<StateTransitionSequence> & info)
  {
    std::stringstream result;
    for (const EventStatePair & event_state : info.param) {
      result << "_" << event_strs[event_state.first];
    }
    return result.str();
  }

protected:
  rcl_action_goal_handle_t goal_handle;
  StateTransitionSequence test_sequence;

  void expect_state_eq(const rcl_action_goal_state_t expected_state)
  {
    rcl_action_goal_state_t state;
    rcl_ret_t ret = rcl_action_goal_handle_get_status(&this->goal_handle, &state);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    EXPECT_EQ(state, expected_state);
  }

  void SetUp()
  {
    // Initialize goal info
    rcl_action_goal_info_t goal_info = rcl_action_get_zero_initialized_goal_info();

    // Initialize goal handle
    this->goal_handle = rcl_action_get_zero_initialized_goal_handle();
    rcl_ret_t ret = rcl_action_goal_handle_init(
      &this->goal_handle, &goal_info, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    // Get test sequence
    this->test_sequence = GetParam();
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_action_goal_handle_fini(&this->goal_handle);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};

TEST_P(TestGoalHandleStateTransitionSequence, test_goal_handle_state_transitions)
{
  // Goal handle starts in state ACCEPTED
  expect_state_eq(GOAL_STATE_ACCEPTED);

  // Walk through state transitions
  rcl_ret_t ret;
  for (const EventStatePair & event_state : this->test_sequence) {
    ret = rcl_action_update_goal_state(&this->goal_handle, event_state.first);
    const rcl_action_goal_state_t & expected_state = event_state.second;
    if (GOAL_STATE_UNKNOWN == expected_state) {
      EXPECT_EQ(ret, RCL_RET_ACTION_GOAL_EVENT_INVALID);
      continue;
    }
    EXPECT_EQ(ret, RCL_RET_OK);
    expect_state_eq(expected_state);
  }
}

// Test sequence parameters
// Note, each sequence starts in the ACCEPTED state
const StateTransitionSequence valid_state_transition_sequences[] = {
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_SET_CANCELED, GOAL_STATE_CANCELED},
  },
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_SET_SUCCEEDED, GOAL_STATE_SUCCEEDED},
  },
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_SET_ABORTED, GOAL_STATE_ABORTED},
  },
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_SET_SUCCEEDED, GOAL_STATE_SUCCEEDED},
  },
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_SET_ABORTED, GOAL_STATE_ABORTED},
  },
  {
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_SET_CANCELED, GOAL_STATE_CANCELED},
  },
  {
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_SET_ABORTED, GOAL_STATE_ABORTED},
  },
  // This is an odd case, but valid nonetheless
  {
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_SET_SUCCEEDED, GOAL_STATE_SUCCEEDED},
  },
};

INSTANTIATE_TEST_CASE_P(
  TestValidGoalHandleStateTransitions,
  TestGoalHandleStateTransitionSequence,
  ::testing::ValuesIn(valid_state_transition_sequences),
  TestGoalHandleStateTransitionSequence::print_sequence_param_name);

const StateTransitionSequence invalid_state_transition_sequences[] = {
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_EXECUTE, GOAL_STATE_UNKNOWN},
  },
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_CANCEL, GOAL_STATE_CANCELING},
    {GOAL_EVENT_CANCEL, GOAL_STATE_UNKNOWN},
  },
  {
    {GOAL_EVENT_EXECUTE, GOAL_STATE_EXECUTING},
    {GOAL_EVENT_EXECUTE, GOAL_STATE_UNKNOWN},
  },
  {
    {GOAL_EVENT_SET_CANCELED, GOAL_STATE_UNKNOWN},
  },
  {
    {GOAL_EVENT_SET_SUCCEEDED, GOAL_STATE_UNKNOWN},
  },
  {
    {GOAL_EVENT_SET_ABORTED, GOAL_STATE_UNKNOWN},
  },
};

INSTANTIATE_TEST_CASE_P(
  TestInvalidGoalHandleStateTransitions,
  TestGoalHandleStateTransitionSequence,
  ::testing::ValuesIn(invalid_state_transition_sequences),
  TestGoalHandleStateTransitionSequence::print_sequence_param_name);
