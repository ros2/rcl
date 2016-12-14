// Copyright 2016 Open Source Robotics Foundation, Inc.
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

// testing default transition sequence.
// This test requires that the transitions are set
// as depicted in design.ros2.org

#include <gtest/gtest.h>

#include <lifecycle_msgs/msg/state.h>
#include <lifecycle_msgs/msg/transition.h>

#include <vector>

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

#include "rcl_lifecycle/rcl_lifecycle.h"
#include "../src/default_state_machine.h"

class TestMultipleInstances : public ::testing::Test
{
protected:
  rcl_node_t * node_ptr;
  void SetUp()
  {
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "node_name";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};

void
test_trigger_transition(
  rcl_lifecycle_state_machine_t * state_machine,
  int key,
  unsigned int expected_current_state,
  unsigned int expected_goal_state)
{
  EXPECT_EQ(
    expected_current_state, state_machine->current_state->id);
  EXPECT_EQ(
    RCL_RET_OK, rcl_lifecycle_trigger_transition(
      state_machine, key, false));
  EXPECT_EQ(
    expected_goal_state, state_machine->current_state->id);
}

TEST_F(TestMultipleInstances, default_sequence_error_unresolved) {
  rcl_lifecycle_state_machine_t state_machine1 =
    rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine1);

  rcl_lifecycle_state_machine_t state_machine2 =
    rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine2);

  rcl_lifecycle_state_machine_t state_machine3 =
    rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine3);

  test_trigger_transition(
    &state_machine1,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  EXPECT_EQ(
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING, state_machine1.current_state->id);
  EXPECT_EQ(
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED, state_machine2.current_state->id);
  EXPECT_EQ(
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED, state_machine3.current_state->id);
}
