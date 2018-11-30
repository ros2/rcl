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
#include <iostream>
#include <sstream>
#include <thread>

#include "rcutils/types.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/graph.h"
#include "rcl/rcl.h"
#include "rmw/rmw.h"

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
  {}

  void TearDown()
  {}
};

TEST_F(CLASSNAME(TestGetNodeNames, RMW_IMPLEMENTATION), test_rcl_get_node_names) {
  rcl_ret_t ret;
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
  });
  rcl_context_t context = rcl_get_zero_initialized_context();
  ret = rcl_init(0, nullptr, &init_options, &context);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_shutdown(&context)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_context_fini(&context)) << rcl_get_error_string().str;
  });
  auto node1_ptr = new rcl_node_t;
  *node1_ptr = rcl_get_zero_initialized_node();
  const char * node1_name = "node1";
  const char * node1_namespace = "/";
  rcl_node_options_t node1_options = rcl_node_get_default_options();
  ret = rcl_node_init(node1_ptr, node1_name, node1_namespace, &context, &node1_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  auto node2_ptr = new rcl_node_t;
  *node2_ptr = rcl_get_zero_initialized_node();
  const char * node2_name = "node2";
  const char * node2_namespace = "/";
  rcl_node_options_t node2_options = rcl_node_get_default_options();
  ret = rcl_node_init(node2_ptr, node2_name, node2_namespace, &context, &node2_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  auto node3_ptr = new rcl_node_t;
  *node3_ptr = rcl_get_zero_initialized_node();
  const char * node3_name = "node3";
  const char * node3_namespace = "/ns";
  rcl_node_options_t node3_options = rcl_node_get_default_options();
  ret = rcl_node_init(node3_ptr, node3_name, node3_namespace, &context, &node3_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  auto node4_ptr = new rcl_node_t;
  *node4_ptr = rcl_get_zero_initialized_node();
  const char * node4_name = "node2";
  const char * node4_namespace = "/ns/ns";
  rcl_node_options_t node4_options = rcl_node_get_default_options();
  ret = rcl_node_init(node4_ptr, node4_name, node4_namespace, &context, &node4_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  std::this_thread::sleep_for(1s);

  rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
  ret = rcl_get_node_names(node1_ptr, node1_options.allocator, &node_names, &node_namespaces);
  ASSERT_EQ(RCUTILS_RET_OK, ret) << rcl_get_error_string().str;

  std::stringstream ss;
  ss << "[test_rcl_get_node_names]: Found node names:" << std::endl;
  for (size_t i = 0; i < node_names.size; ++i) {
    ss << node_names.data[i] << std::endl;
  }
  EXPECT_EQ(size_t(4), node_names.size) << ss.str();
  EXPECT_EQ(0, strcmp(node1_name, node_names.data[0]));
  EXPECT_EQ(0, strcmp(node2_name, node_names.data[1]));
  EXPECT_EQ(0, strcmp(node3_name, node_names.data[2]));
  EXPECT_EQ(0, strcmp(node4_name, node_names.data[3]));

  EXPECT_EQ(size_t(4), node_namespaces.size) << ss.str();
  EXPECT_EQ(0, strcmp(node1_namespace, node_namespaces.data[0]));
  EXPECT_EQ(0, strcmp(node2_namespace, node_namespaces.data[1]));
  EXPECT_EQ(0, strcmp(node3_namespace, node_namespaces.data[2]));
  EXPECT_EQ(0, strcmp(node4_namespace, node_namespaces.data[3]));

  ret = rcutils_string_array_fini(&node_names);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_string_array_fini(&node_namespaces);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcl_node_fini(node1_ptr);
  delete node1_ptr;
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_node_fini(node2_ptr);
  delete node2_ptr;
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_node_fini(node3_ptr);
  delete node3_ptr;
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_node_fini(node4_ptr);
  delete node4_ptr;
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}
