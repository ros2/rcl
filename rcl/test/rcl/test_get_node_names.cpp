// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#include <chrono>
#include <thread>

#include "rcutils/types.h"

#include "rcl/graph.h"
#include "rcl/rcl.h"
#include "rmw/rmw.h"

#include "../memory_tools/memory_tools.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

using namespace std::chrono_literals;

class CLASSNAME (TestGetNodeNames, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
    set_on_unexpected_malloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED MALLOC";});
    set_on_unexpected_realloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED REALLOC";});
    set_on_unexpected_free_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED FREE";});
    start_memory_checking();
  }

  void TearDown()
  {
    assert_no_malloc_end();
    assert_no_realloc_end();
    assert_no_free_end();
    stop_memory_checking();
    set_on_unexpected_malloc_callback(nullptr);
    set_on_unexpected_realloc_callback(nullptr);
    set_on_unexpected_free_callback(nullptr);
  }
};

TEST_F(CLASSNAME(TestGetNodeNames, RMW_IMPLEMENTATION), test_rcl_get_node_names) {
  auto ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto node1_ptr = new rcl_node_t;
  *node1_ptr = rcl_get_zero_initialized_node();
  const char * node1_name = "node1";
  rcl_node_options_t node1_options = rcl_node_get_default_options();
  ret = rcl_node_init(node1_ptr, node1_name, "", &node1_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  auto node2_ptr = new rcl_node_t;
  *node2_ptr = rcl_get_zero_initialized_node();
  const char * node2_name = "node2";
  rcl_node_options_t node2_options = rcl_node_get_default_options();
  ret = rcl_node_init(node2_ptr, node2_name, "", &node2_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  std::this_thread::sleep_for(1s);

  rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
  ret = rcl_get_node_names(node1_ptr, node1_options.allocator, &node_names);
  ASSERT_EQ(RCUTILS_RET_OK, ret) << rcl_get_error_string_safe();

  EXPECT_EQ(size_t(2), node_names.size);
  EXPECT_EQ(0, strcmp(node1_name, node_names.data[0]));
  EXPECT_EQ(0, strcmp(node2_name, node_names.data[1]));

  ret = rcutils_string_array_fini(&node_names);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcl_node_fini(node1_ptr);
  delete node1_ptr;
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_node_fini(node2_ptr);
  delete node2_ptr;
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_shutdown();
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}
