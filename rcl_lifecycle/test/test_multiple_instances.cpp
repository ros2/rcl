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

/*
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

static const std::vector<unsigned int> state_ids = {
  lifecycle_msgs__msg__State__PRIMARY_STATE_UNKNOWN,
  lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
  lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
  lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
  lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED,
  lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
  lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
  lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
  lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
  lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
  lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING
};
static const std::vector<const char *> state_names = {
  "unknown",
  "unconfigured",
  "inactive",
  "active",
  "finalized",
  "configuring",
  "cleaningup",
  "shuttingdown",
  "activating",
  "deactivating",
  "errorprocessing"
};

static const std::vector<unsigned int> transition_ids = {
  lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
  lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
  lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
  lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
  lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE
};
static const std::vector<const char *> transition_names = {
  "configure",
  "cleanup",
  "shutdown",
  "activate",
  "deactivate"
};

void
test_successful_state_change(
  rcl_lifecycle_state_machine_t & state_machine,
  unsigned int transition_id,
  unsigned int expected_current_state_id,
  unsigned int expected_goal_state_id)
{
  EXPECT_EQ(expected_current_state_id, state_machine.current_state->id);
  auto cb_success = RCL_LIFECYCLE_RET_OK;
  EXPECT_EQ(RCL_RET_OK, rcl_lifecycle_trigger_transition(&state_machine, transition_id, cb_success,
    false));
  EXPECT_EQ(expected_goal_state_id, state_machine.current_state->id);
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

  test_successful_state_change(state_machine1,
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
*/
