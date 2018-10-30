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

#include "rcl/error_handling.h"

#include "rcl_lifecycle/rcl_lifecycle.h"
#include "rcl_lifecycle/transition_map.h"

class TestTransitionMap : public ::testing::Test
{
protected:
  void SetUp()
  {
  }

  void TearDown()
  {
  }
};

TEST_F(TestTransitionMap, zero_initialized) {
  rcl_lifecycle_transition_map_t transition_map =
    rcl_lifecycle_get_zero_initialized_transition_map();

  EXPECT_EQ(RCL_RET_ERROR, rcl_lifecycle_transition_map_is_initialized(&transition_map));

  rcl_allocator_t allocator = rcl_get_default_allocator();
  EXPECT_EQ(RCL_RET_OK, rcl_lifecycle_transition_map_fini(&transition_map, &allocator));
}

TEST_F(TestTransitionMap, initialized) {
  rcl_lifecycle_transition_map_t transition_map =
    rcl_lifecycle_get_zero_initialized_transition_map();

  rcl_allocator_t allocator = rcl_get_default_allocator();

  rcl_lifecycle_state_t state0 = {"my_state_0", 0, NULL, 0};
  rcl_ret_t ret = rcl_lifecycle_register_state(&transition_map, state0, &allocator);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(RCL_RET_OK, rcl_lifecycle_transition_map_is_initialized(&transition_map));

  ret = rcl_lifecycle_register_state(&transition_map, state0, &allocator);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;

  rcl_lifecycle_state_t state1 = {"my_state_1", 1, NULL, 0};
  ret = rcl_lifecycle_register_state(&transition_map, state1, &allocator);

  rcl_lifecycle_state_t * start_state =
    rcl_lifecycle_get_state(&transition_map, state0.id);
  rcl_lifecycle_state_t * goal_state =
    rcl_lifecycle_get_state(&transition_map, state1.id);
  EXPECT_EQ(0u, start_state->id);
  EXPECT_EQ(1u, goal_state->id);

  rcl_lifecycle_transition_t transition01 = {"from0to1", 0,
    start_state, goal_state};
  ret = rcl_lifecycle_register_transition(
    &transition_map, transition01, &allocator);
  EXPECT_EQ(RCL_RET_OK, ret);

  rcl_lifecycle_transition_t transition10 = {"from1to0", 1,
    goal_state, start_state};
  ret = rcl_lifecycle_register_transition(
    &transition_map, transition10, &allocator);
  EXPECT_EQ(RCL_RET_OK, ret);

  const rcl_lifecycle_transition_t * trans =
    rcl_lifecycle_get_transition_by_id(start_state, 0);
  EXPECT_EQ(0u, trans->id);
  trans = rcl_lifecycle_get_transition_by_label(start_state, "from0to1");
  EXPECT_EQ(0u, trans->id);
  trans = rcl_lifecycle_get_transition_by_id(goal_state, 1);
  EXPECT_EQ(1u, trans->id);
  trans = rcl_lifecycle_get_transition_by_label(goal_state, "from1to0");
  EXPECT_EQ(1u, trans->id);

  EXPECT_EQ(RCL_RET_OK, rcl_lifecycle_transition_map_fini(&transition_map, &allocator));
}
