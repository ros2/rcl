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
#include "../src/default_state_machine.hxx"


class TestDefaultStateMachine : public ::testing::Test
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

/*
 * Test suite
 */
TEST_F(TestDefaultStateMachine, zero_init) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ASSERT_EQ(rcl_lifecycle_state_machine_is_initialized(&state_machine), RCL_RET_ERROR);
  const rcl_lifecycle_transition_map_t * transition_map = &state_machine.transition_map;
  ASSERT_EQ(transition_map->states_size, 0);
  ASSERT_EQ(transition_map->states, nullptr);
  ASSERT_EQ(transition_map->transitions_size, 0);
  ASSERT_EQ(transition_map->transitions, nullptr);
}

TEST_F(TestDefaultStateMachine, default_sequence) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);
  ASSERT_EQ(lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED, state_machine.current_state->id);
  EXPECT_EQ(
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    rcl_lifecycle_is_valid_external_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE)->id);
  EXPECT_EQ(lifecycle_msgs__msg__Transition__TRANSITION_UNCONFIGURED_SHUTDOWN, rcl_lifecycle_is_valid_external_transition(&state_machine, lifecycle_msgs__msg__Transition__TRANSITION_UNCONFIGURED_SHUTDOWN)->id);
  EXPECT_EQ(RCL_RET_OK, rcl_lifecycle_trigger_external_transition(&state_machine, lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE, false));
  EXPECT_EQ(lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING, state_machine.current_state->id);
  auto callback = RCL_LIFECYCLE_RET_OK;
  EXPECT_EQ(RCL_RET_OK, rcl_lifecycle_trigger_callback_transition(&state_machine, callback, false));
  EXPECT_EQ(lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE, state_machine.current_state->id);

}

TEST_F(TestDefaultStateMachine, print_state_machine) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);
  //rcl_print_state_machine(&state_machine);
}
