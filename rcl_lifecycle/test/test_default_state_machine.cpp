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

std::vector<rcl_lifecycle_ret_t> keys =
{
  lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
  lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
  lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
  lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
  lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
  RCL_LIFECYCLE_RET_OK,
  RCL_LIFECYCLE_RET_FAILURE,
  RCL_LIFECYCLE_RET_ERROR
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

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
}

TEST_F(TestDefaultStateMachine, wrong_default_sequence) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  { // supposed to stay unconfigured for all invalid
    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE ||
        *it == lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN) {continue;}

      EXPECT_EQ(
        RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(
        state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
    }
  }

  { // supposed to stay configuring for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == RCL_LIFECYCLE_RET_OK ||
        *it == RCL_LIFECYCLE_RET_FAILURE ||
        *it == RCL_LIFECYCLE_RET_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);
    }
  }

  { // supposed to stay inactive for all invalid
    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP ||
        *it == lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE ||
        *it == lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN) {continue;}

      fprintf(stderr, "applying key %u\n", *it);
      EXPECT_EQ(
        RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    }
  }

  { // supposed to stay activating for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == RCL_LIFECYCLE_RET_OK ||
        *it == RCL_LIFECYCLE_RET_FAILURE ||
        *it == RCL_LIFECYCLE_RET_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);
    }
  }

  { // supposed to stay active for all invalid
    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE ||
        *it == lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN)
      {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
    }
  }

  { // supposed to stay deactivating for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == RCL_LIFECYCLE_RET_OK ||
        *it == RCL_LIFECYCLE_RET_FAILURE ||
        *it == RCL_LIFECYCLE_RET_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);
    }
  }

  { // supposed to stay cleanup for all invalid
    // skip inactive, we tested that already
    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == RCL_LIFECYCLE_RET_OK ||
        *it == RCL_LIFECYCLE_RET_FAILURE ||
        *it == RCL_LIFECYCLE_RET_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
    }
  }

  { // supposed to stay shutting down for all invalid
    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
    // shutdown directly, since we tested already unconfigured
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (*it == RCL_LIFECYCLE_RET_OK ||
        *it == RCL_LIFECYCLE_RET_FAILURE ||
        *it == RCL_LIFECYCLE_RET_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);
    }
  }

  { // supposed to stay finalized for all invalid
    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
    }
  }
}

TEST_F(TestDefaultStateMachine, default_in_a_loop) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  for (auto i = 0; i < 5; ++i) {
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  }

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
}

TEST_F(TestDefaultStateMachine, default_sequence_failure) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_FAILURE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  ///////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_FAILURE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  //////////////////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_FAILURE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

  //////////////////////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_FAILURE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  /////////////////////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_FAILURE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
}

TEST_F(TestDefaultStateMachine, default_sequence_error_resolved) {
  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  rcl_lifecycle_init_default_state_machine(&state_machine);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  ///////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  //////////////////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  //////////////////////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  /////////////////////////////
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
  test_trigger_transition(
    &state_machine,
    RCL_LIFECYCLE_RET_OK,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
}

TEST_F(TestDefaultStateMachine, default_sequence_error_unresolved) {
  {
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_ERROR,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_FAILURE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }

  {
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    rcl_lifecycle_init_default_state_machine(&state_machine);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_OK,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_ERROR,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

    test_trigger_transition(
      &state_machine,
      RCL_LIFECYCLE_RET_ERROR,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
}
