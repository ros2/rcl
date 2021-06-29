// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "../src/impl/node_params_descriptors.h"
#include "rcutils/allocator.h"

TEST(TestNodeParamsDescriptors, init_fini) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcl_node_params_descriptors_t node_descriptors = {NULL, NULL, 0u, 128u};
  EXPECT_EQ(RCUTILS_RET_OK, node_params_descriptors_init(&node_descriptors, allocator));
  EXPECT_NE(nullptr, node_descriptors.parameter_names);
  EXPECT_NE(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(128u, node_descriptors.capacity_descriptors);
  rcl_yaml_node_params_descriptors_fini(&node_descriptors, allocator);
  EXPECT_EQ(nullptr, node_descriptors.parameter_names);
  EXPECT_EQ(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(0u, node_descriptors.capacity_descriptors);

  // This function doesn't return anything, so just check it doesn't segfault on the second try
  rcl_yaml_node_params_descriptors_fini(&node_descriptors, allocator);
  rcl_yaml_node_params_descriptors_fini(nullptr, allocator);
}

TEST(TestNodeParamsDescriptors, init_with_capacity_fini) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcl_node_params_descriptors_t node_descriptors = {NULL, NULL, 0u, 1024u};
  EXPECT_EQ(
    RCUTILS_RET_OK, node_params_descriptors_init_with_capacity(
      &node_descriptors, 1024, allocator));
  EXPECT_NE(nullptr, node_descriptors.parameter_names);
  EXPECT_NE(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(1024u, node_descriptors.capacity_descriptors);
  rcl_yaml_node_params_descriptors_fini(&node_descriptors, allocator);
  EXPECT_EQ(nullptr, node_descriptors.parameter_names);
  EXPECT_EQ(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(0u, node_descriptors.capacity_descriptors);

  // This function doesn't return anything, so just check it doesn't segfault on the second try
  rcl_yaml_node_params_descriptors_fini(&node_descriptors, allocator);
  rcl_yaml_node_params_descriptors_fini(nullptr, allocator);
}

TEST(TestNodeParamsDescriptors, reallocate_with_capacity_fini) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcl_node_params_descriptors_t node_descriptors = {NULL, NULL, 0u, 1024u};
  EXPECT_EQ(
    RCUTILS_RET_OK, node_params_descriptors_init_with_capacity(
      &node_descriptors, 1024, allocator));
  EXPECT_NE(nullptr, node_descriptors.parameter_names);
  EXPECT_NE(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(1024u, node_descriptors.capacity_descriptors);
  EXPECT_EQ(RCUTILS_RET_OK, node_params_descriptors_reallocate(&node_descriptors, 2048, allocator));
  EXPECT_NE(nullptr, node_descriptors.parameter_names);
  EXPECT_NE(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(2048u, node_descriptors.capacity_descriptors);
  rcl_yaml_node_params_descriptors_fini(&node_descriptors, allocator);
  EXPECT_EQ(nullptr, node_descriptors.parameter_names);
  EXPECT_EQ(nullptr, node_descriptors.parameter_descriptors);
  EXPECT_EQ(0u, node_descriptors.num_params);
  EXPECT_EQ(0u, node_descriptors.capacity_descriptors);

  // This function doesn't return anything, so just check it doesn't segfault on the second try
  rcl_yaml_node_params_descriptors_fini(&node_descriptors, allocator);
  rcl_yaml_node_params_descriptors_fini(nullptr, allocator);
}
