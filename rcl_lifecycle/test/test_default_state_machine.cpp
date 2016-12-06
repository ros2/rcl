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

static const std::vector<size_t> primary_states = {
  lifecycle_msgs__msg__State__PRIMARY_STATE_UNKNOWN,
  lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
  lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
  lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
  lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED
};
static const std::vector<const char *> primary_names = {
  "unknown",
  "unconfigured",
  "inactive",
  "active",
  "finalized"
};

static const std::vector<size_t> transition_states = {
  lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
  lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
  lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
  lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
  lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
  lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING
};
static const std::vector<const char *> transition_names = {
  "configuring",
  "cleaningup",
  "shuttingdown",
  "activating",
  "deactivating",
  "errorprocessing"
};

/*
 * Helper functions
 */
void
test_successful_state_change(rcl_lifecycle_state_machine_t & state_machine,
  unsigned int expected_current_state_id,
  unsigned int expected_transition_id,
  unsigned int expected_goal_state_id)
{
  EXPECT_EQ(state_machine.current_state->id, expected_current_state_id);
  EXPECT_EQ(rcl_lifecycle_start_transition(&state_machine, expected_transition_id,
    false), RCL_RET_OK);
  EXPECT_EQ(state_machine.current_state->id, expected_transition_id);
  auto cb_success = true;
  EXPECT_EQ(rcl_lifecycle_finish_transition(&state_machine, expected_transition_id,
    cb_success, false), RCL_RET_OK);
  EXPECT_EQ(state_machine.current_state->id, expected_goal_state_id);
}

void
test_successful_state_change_strong(rcl_lifecycle_state_machine_t & state_machine,
  unsigned int expected_current_state_id,
  unsigned int expected_transition_id,
  unsigned int expected_goal_state_id)
{
  ASSERT_EQ(state_machine.current_state->id, expected_current_state_id);
  ASSERT_EQ(rcl_lifecycle_start_transition(&state_machine, expected_transition_id,
    false), RCL_RET_OK);
  ASSERT_EQ(state_machine.current_state->id, expected_transition_id);
  auto cb_success = true;
  ASSERT_EQ(rcl_lifecycle_finish_transition(&state_machine, expected_transition_id,
    cb_success, false), RCL_RET_OK);
  ASSERT_EQ(state_machine.current_state->id, expected_goal_state_id);
}

void
test_unsuccessful_state_change(rcl_lifecycle_state_machine_t & state_machine,
  unsigned int expected_current_state_id,
  unsigned int expected_transition_id,
  unsigned int expected_goal_state_id)
{
  EXPECT_EQ(state_machine.current_state->id, expected_current_state_id);
  EXPECT_EQ(rcl_lifecycle_start_transition(&state_machine, expected_transition_id,
    false), RCL_RET_OK);
  EXPECT_EQ(state_machine.current_state->id, expected_transition_id);
  auto cb_success = false;
  EXPECT_EQ(rcl_lifecycle_finish_transition(&state_machine, expected_transition_id,
    cb_success, false), RCL_RET_OK);
  EXPECT_EQ(state_machine.current_state->id, expected_goal_state_id);
}

/*
 * Test suite
 */
TEST_F(TestDefaultStateMachine, zero_init) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ASSERT_EQ(rcl_lifecycle_state_machine_is_initialized(&state_machine), RCL_RET_ERROR);
  const rcl_lifecycle_transition_map_t * transition_map = &state_machine.transition_map;
  ASSERT_EQ(transition_map->size, 0);
  ASSERT_EQ(transition_map->primary_states, nullptr);
  ASSERT_EQ(transition_map->transition_arrays, nullptr);
}

TEST_F(TestDefaultStateMachine, init) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  const rcl_lifecycle_transition_map_t * transition_map = &state_machine.transition_map;
  ASSERT_EQ(transition_map->size, primary_names.size());

  for (auto i = 0; i < primary_names.size(); ++i) {
    ASSERT_EQ(primary_states[i], transition_map->primary_states[i].id);
    ASSERT_STREQ(primary_names[i], transition_map->primary_states[i].label);
  }
}

TEST_F(TestDefaultStateMachine, transitions) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  // exclude error processing for now
  for (auto i = 0; i < transition_states.size() - 1; ++i) {
    EXPECT_FALSE(nullptr ==
      rcl_lifecycle_get_registered_transition(&state_machine, transition_states[i]));
  }
}

TEST_F(TestDefaultStateMachine, default_sequence) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  {  // configuring  (unconfigured to inactive)
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
  }

  {  // activating  (inactive to active)
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
  }

  {  // deactivating  (active to inactive)
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
  }

  {  // cleaningup  (inactive to unconfigured)
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  }

  {  // shutdown  (unconfigured to finalized)
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
}

TEST_F(TestDefaultStateMachine, wrong_default_sequence) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  { // supposed to stay unconfigured for all invalid
    for (auto it = transition_states.begin(); it != transition_states.end(); ++it) {
      if (*it == lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING ||
        *it == lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP ||
        *it == lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN) {continue;}

      EXPECT_EQ(rcl_lifecycle_start_transition(&state_machine, *it, false), RCL_RET_ERROR);
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
    }
  }

  { // supposed to stay inactive for all invalid
    test_successful_state_change_strong(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    for (auto it = transition_states.begin(); it != transition_states.end(); ++it) {
      if (*it == lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP ||
        *it == lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING ||
        *it == lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN) {continue;}

      EXPECT_EQ(rcl_lifecycle_start_transition(&state_machine, *it, false), RCL_RET_ERROR);
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    }
  }

  { // supposed to stay inactive for all invalid
    test_successful_state_change_strong(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

    for (auto it = transition_states.begin(); it != transition_states.end(); ++it) {
      if (*it == lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING ||
        *it == lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN) {continue;}

      EXPECT_EQ(rcl_lifecycle_start_transition(&state_machine, *it, false), RCL_RET_ERROR);
      EXPECT_EQ(state_machine.current_state->id, lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
    }
  }

  { // supposed to stay finalized for all invalid
    test_successful_state_change_strong(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

    for (auto it = transition_states.begin(); it != transition_states.end(); ++it) {
      EXPECT_EQ(rcl_lifecycle_start_transition(&state_machine, *it, false), RCL_RET_ERROR);
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
    }
  }
}

TEST_F(TestDefaultStateMachine, default_sequence_loop) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  // testing default transition sequence.
  // This test requires that the transitions are set
  // as depicted in design.ros2.org
  size_t n = 5;
  for (auto i = 0; i < n; ++i) {
    { // configuring  (unconfigured to inactive)
      test_successful_state_change_strong(state_machine,
        lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
        lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
        lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    }

    { // activating  (inactive to active)
      test_successful_state_change_strong(state_machine,
        lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
        lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
        lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
    }

    { // deactivating  (active to inactive)
      test_successful_state_change_strong(state_machine,
        lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
        lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
        lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    }

    { // cleaningup  (inactive to unconfigured)
      test_successful_state_change_strong(state_machine,
        lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
        lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
        lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
    }
  }
  {  // shutdown  (unconfigured to finalized)
    test_successful_state_change_strong(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
}

TEST_F(TestDefaultStateMachine, default_sequence_shutdown) {
  {  // unconfigured to shutdown
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
  {  // inactive to shutdown
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }

  {  // active to shutdown
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
}

TEST_F(TestDefaultStateMachine, default_sequence_error_resolved) {
  {  // configuring to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = true;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  }

  {  // cleaningup to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = true;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  }

  {  // activating to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = true;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  }

  {  // deactivating to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = true;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  }
}

TEST_F(TestDefaultStateMachine, default_sequence_error_unresolved) {
  {  // configuring to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = false;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id, lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }

  {  // cleaningup to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = false;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id, lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }

  {  // activating to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = false;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id, lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }

  {  // deactivating to error
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_successful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
    test_unsuccessful_state_change(state_machine,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
    bool error_resolved = false;
    EXPECT_EQ(rcl_lifecycle_state_machine_resolve_error(&state_machine, error_resolved),
      RCL_RET_OK);
    EXPECT_EQ(state_machine.current_state->id, lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
}
