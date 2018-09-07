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
#include <vector>

#include "lifecycle_msgs/msg/state.h"
#include "lifecycle_msgs/msg/transition.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

#include "rcutils/logging_macros.h"

#include "rcl_lifecycle/rcl_lifecycle.h"
#include "rcl_lifecycle/default_state_machine.h"

class TestDefaultStateMachine : public ::testing::Test
{
protected:
  rcl_node_t * node_ptr;
  const rcl_allocator_t * allocator;

  void SetUp()
  {
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_state_machine_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    const rcl_node_options_t * node_ops = rcl_node_get_options(this->node_ptr);
    this->allocator = &node_ops->allocator;
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
  uint8_t key_id,
  unsigned int expected_current_state,
  unsigned int expected_goal_state)
{
  rcl_lifecycle_transition_key_t key = {key_id, ""};

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
  rcl_reset_error();
  const rcl_lifecycle_transition_map_t * transition_map = &state_machine.transition_map;
  ASSERT_EQ(transition_map->states_size, 0u);
  ASSERT_EQ(transition_map->states, nullptr);
  ASSERT_EQ(transition_map->transitions_size, 0u);
  ASSERT_EQ(transition_map->transitions, nullptr);

  auto ret = rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

TEST_F(TestDefaultStateMachine, default_sequence) {
  rcl_ret_t ret;

  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

  EXPECT_EQ(RCL_RET_OK,
    rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator));
}

TEST_F(TestDefaultStateMachine, wrong_default_sequence) {
  rcl_ret_t ret;

  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  std::vector<rcl_lifecycle_transition_key_t> keys =
  {
    default_transition_key_configure,
    default_transition_key_cleanup,
    default_transition_key_activate,
    default_transition_key_deactivate,
    default_transition_key_shutdown,
    default_transition_key_callback_success,
    default_transition_key_callback_failure,
    default_transition_key_callback_error,
  };

  { // supposed to stay unconfigured for all invalid
    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
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
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);
    }
  }

  { // supposed to stay inactive for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN) {continue;}

      RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "applying key %u", it->id)
      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
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
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);
    }
  }

  { // supposed to stay active for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN)
      {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
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
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);
    }
  }

  { // supposed to stay cleanup for all invalid
    // skip inactive, we tested that already
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
    }
  }

  { // supposed to stay shutting down for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
    // shutdown directly, since we tested already unconfigured
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      if (it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE ||
        it->id == lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR) {continue;}

      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);
    }
  }

  { // supposed to stay finalized for all invalid
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_trigger_transition(&state_machine, *it, false));
      rcl_reset_error();
      EXPECT_EQ(state_machine.current_state->id,
        lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
    }
  }

  EXPECT_EQ(RCL_RET_OK,
    rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator));
}

TEST_F(TestDefaultStateMachine, default_in_a_loop) {
  rcl_ret_t ret;

  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  for (auto i = 0; i < 5; ++i) {
    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

  EXPECT_EQ(RCL_RET_OK,
    rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator));
}

TEST_F(TestDefaultStateMachine, default_sequence_failure) {
  rcl_ret_t ret;

  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_SHUTDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

  EXPECT_EQ(RCL_RET_OK,
    rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator));
}

TEST_F(TestDefaultStateMachine, default_sequence_error_resolved) {
  rcl_ret_t ret;

  rcl_lifecycle_state_machine_t state_machine = rcl_lifecycle_get_zero_initialized_state_machine();
  ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_DEACTIVATE,
    lifecycle_msgs__msg__State__PRIMARY_STATE_ACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_DEACTIVATING,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CLEANUP,
    lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_CLEANINGUP,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
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
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
    lifecycle_msgs__msg__State__TRANSITION_STATE_SHUTTINGDOWN,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);
  test_trigger_transition(
    &state_machine,
    lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
    lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
    lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED);

  EXPECT_EQ(RCL_RET_OK,
    rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator));
}

TEST_F(TestDefaultStateMachine, default_sequence_error_unresolved) {
  rcl_ret_t ret;

  {
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_FAILURE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);

    EXPECT_EQ(RCL_RET_OK,
      rcl_lifecycle_state_machine_fini(&state_machine, this->node_ptr, this->allocator));
  }

  {
    rcl_lifecycle_state_machine_t state_machine =
      rcl_lifecycle_get_zero_initialized_state_machine();
    ret = rcl_lifecycle_init_default_state_machine(&state_machine, this->allocator);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CONFIGURE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_UNCONFIGURED,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_SUCCESS,
      lifecycle_msgs__msg__State__TRANSITION_STATE_CONFIGURING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_ACTIVATE,
      lifecycle_msgs__msg__State__PRIMARY_STATE_INACTIVE,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ACTIVATING,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING);

    test_trigger_transition(
      &state_machine,
      lifecycle_msgs__msg__Transition__TRANSITION_CALLBACK_ERROR,
      lifecycle_msgs__msg__State__TRANSITION_STATE_ERRORPROCESSING,
      lifecycle_msgs__msg__State__PRIMARY_STATE_FINALIZED);
  }
}
