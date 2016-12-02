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

#include <gtest/gtest.h>
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
  rcl_state_unknown.index,
  rcl_state_unconfigured.index,
  rcl_state_inactive.index,
  rcl_state_active.index,
  rcl_state_finalized.index
};
static const std::vector<const char*> primary_names = {
  rcl_state_unknown.label,
  rcl_state_unconfigured.label,
  rcl_state_inactive.label,
  rcl_state_active.label,
  rcl_state_finalized.label
};

static const std::vector<size_t> transition_states = {
  rcl_state_configuring.index,
  rcl_state_cleaningup.index,
  rcl_state_shuttingdown.index,
  rcl_state_activating.index,
  rcl_state_deactivating.index,
  rcl_state_errorprocessing.index
};
static const std::vector<const char*> transition_names = {
  rcl_state_configuring.label,
  rcl_state_cleaningup.label,
  rcl_state_shuttingdown.label,
  rcl_state_activating.label,
  rcl_state_deactivating.label,
  rcl_state_errorprocessing.label
};

TEST_F(TestDefaultStateMachine, zero_init)
{
  rcl_state_machine_t state_machine = rcl_get_zero_initialized_state_machine();
  ASSERT_EQ(rcl_state_machine_is_initialized(&state_machine), false);
  const rcl_transition_map_t * transition_map = &state_machine.transition_map;
  ASSERT_EQ(transition_map->size, 0);
  ASSERT_EQ(transition_map->primary_states, nullptr);
  ASSERT_EQ(transition_map->transition_arrays, nullptr);
}

TEST_F(TestDefaultStateMachine, init)
{
  rcl_state_machine_t state_machine = rcl_get_zero_initialized_state_machine();
  rcl_init_default_state_machine(&state_machine);

  const rcl_transition_map_t * transition_map = &state_machine.transition_map;
  ASSERT_EQ(transition_map->size, primary_names.size());

  for (auto i=0; i < primary_names.size(); ++i)
  {
    ASSERT_EQ(primary_states[i], transition_map->primary_states[i].index);
    ASSERT_STREQ(primary_names[i], transition_map->primary_states[i].label);
  }
}

TEST_F(TestDefaultStateMachine, transitions)
{
  rcl_state_machine_t state_machine = rcl_get_zero_initialized_state_machine();
  rcl_init_default_state_machine(&state_machine);

  // exclude error processing for now
  for (auto i=0; i < transition_states.size()-1; ++i)
  {
    EXPECT_FALSE(nullptr ==
        rcl_get_registered_transition_by_index(&state_machine, transition_states[i]));
    EXPECT_FALSE(nullptr ==
        rcl_get_registered_transition_by_label(&state_machine, transition_names[i]));
  }
}

TEST_F(TestDefaultStateMachine, default_sequence)
{
  rcl_state_machine_t state_machine = rcl_get_zero_initialized_state_machine();
  rcl_init_default_state_machine(&state_machine);

  // testing default transition sequence.
  // This test requires that the transitions are set
  // as depicted in design.ros2.org

  //const rcl_transition_map_t * transition_map = &state_machine.transition_map;

  EXPECT_EQ(state_machine.current_state->index, rcl_state_unconfigured.index);
}
