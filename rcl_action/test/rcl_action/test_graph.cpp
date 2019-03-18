// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include "rcl_action/action_client.h"
#include "rcl_action/action_server.h"
#include "rcl_action/graph.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#include "test_msgs/action/fibonacci.h"

class TestActionGraphFixture : public ::testing::Test
{
public:
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_context_t old_context;
  rcl_context_t context;
  rcl_node_t old_node;
  rcl_node_t node;
  rcl_wait_set_t wait_set;
  const char * test_graph_node_name = "test_action_graph_node";
  const char * test_graph_old_node_name = "test_action_graph_old_node_name";

  void SetUp()
  {
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, this->allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });
    this->old_context = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, &this->old_context);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->old_node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(
      &this->old_node, this->test_graph_old_node_name, "", &this->old_context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(&this->old_context);  // after this, the old_node should be invalid
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->context = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, &this->context);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node = rcl_get_zero_initialized_node();
    ret = rcl_node_init(&this->node, test_graph_node_name, "", &this->context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->wait_set = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(
      &this->wait_set, 0, 1, 0, 0, 0, &this->context, this->allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret;
    ret = rcl_node_fini(&this->old_node);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_wait_set_fini(&this->wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(&this->node);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(&this->context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(&this->context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(&this->old_context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};

TEST_F(TestActionGraphFixture, test_action_get_client_names_and_types_by_node) {
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();

  // Invalid node
  ret = rcl_action_get_client_names_and_types_by_node(
    nullptr, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_client_names_and_types_by_node(
    &zero_node, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->old_node, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid allocator
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, nullptr, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid node name
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, &this->allocator, "_test_this_Isnot_a_valid_name", "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, &this->allocator, this->test_graph_old_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid names and types
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, &this->allocator, this->test_graph_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Valid call
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(TestActionGraphFixture, test_action_get_server_names_and_types_by_node) {
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();

  // Invalid node
  ret = rcl_action_get_server_names_and_types_by_node(
    nullptr, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_server_names_and_types_by_node(
    &zero_node, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->old_node, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid allocator
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, nullptr, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid node name
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, &this->allocator, "_test_this_Isnot_a_valid_name", "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, &this->allocator, this->test_graph_old_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid names and types
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, &this->allocator, this->test_graph_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Valid call
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, &this->allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(TestActionGraphFixture, test_action_get_names_and_types) {
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();

  // Invalid node
  ret = rcl_action_get_names_and_types(nullptr, &this->allocator, &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_names_and_types(&zero_node, &this->allocator, &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_action_get_names_and_types(&this->old_node, &this->allocator, &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid allocator
  ret = rcl_action_get_names_and_types(&this->node, nullptr, &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Invalid names and types
  ret = rcl_action_get_names_and_types(&this->node, &this->allocator, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Valid call
  ret = rcl_action_get_names_and_types(&this->node, &this->allocator, &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/**
 * Extend the TestActionGraphFixture with a multi-node fixture for node discovery and node-graph
 * perspective.
 */
// TODO(jacobperron): public CLASSNAME(TestActionGraphFixture, RMW_IMPLEMENTATION)
class TestActionGraphMultiNodeFixture : public TestActionGraphFixture
{
public:
  const char * remote_node_name = "remote_graph_node";
  const char * action_name = "/test_action_info_functions__";
  rcl_node_t remote_node;
  rcl_context_t remote_context;

  void SetUp() override
  {
    // CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION) ::SetUp();
    TestActionGraphFixture::SetUp();

    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) <<
        rcl_get_error_string().str;
    });

    this->remote_node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();

    this->remote_context = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, &this->remote_context);

    ret = rcl_node_init(
      &this->remote_node, this->remote_node_name, "", &this->remote_context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    WaitForAllNodesAlive();
  }

  void TearDown() override
  {
    // CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION) ::TearDown();
    TestActionGraphFixture::TearDown();

    rcl_ret_t ret;
    ret = rcl_node_fini(&this->remote_node);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(&this->remote_context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(&this->remote_context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void WaitForAllNodesAlive()
  {
    rcl_ret_t ret;
    rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
    rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      ret = rcutils_string_array_fini(&node_names);
      ASSERT_EQ(RCUTILS_RET_OK, ret);
      ret = rcutils_string_array_fini(&node_namespaces);
      ASSERT_EQ(RCUTILS_RET_OK, ret);
    });
    // Wait for all 3 nodes to be discovered: remote_node, old_node, node
    size_t attempts = 0u;
    size_t max_attempts = 4u;
    while (node_names.size < 3u) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      ret = rcl_get_node_names(&this->remote_node, allocator, &node_names, &node_namespaces);
      attempts++;
      ASSERT_LE(attempts, max_attempts) << "Unable to attain all required nodes";
    }
  }
};

// Note, this test could be affected by other communication on the same ROS domain
TEST_F(TestActionGraphMultiNodeFixture, test_action_get_names_and_types) {
  rcl_ret_t ret;
  // Create an action client
  rcl_action_client_t action_client = rcl_action_get_zero_initialized_client();
  const rosidl_action_type_support_t * action_typesupport =
    ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
  const char * client_action_name = "/test_action_get_names_and_types_client_action_name";
  rcl_action_client_options_t action_client_options = rcl_action_client_get_default_options();
  ret = rcl_action_client_init(
    &action_client,
    &this->remote_node,
    action_typesupport,
    client_action_name,
    &action_client_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&action_client, &this->remote_node)) <<
      rcl_get_error_string().str;
  });
  // Check that there is exactly one action name
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  ret = rcl_action_get_names_and_types(&this->node, &this->allocator, &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 1u);
  EXPECT_STREQ(nat.names.data[0], client_action_name);
  EXPECT_EQ(nat.types[0].size, 1u);
  EXPECT_STREQ(nat.types[0].data[0], "test_msgs/Fibonacci");

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // Create an action server
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();
  rcl_clock_t clock;
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &this->allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string().str;
  });
  const char * server_action_name = "/test_action_get_names_and_types_server_action_name";
  rcl_action_server_options_t action_server_options = rcl_action_server_get_default_options();
  ret = rcl_action_server_init(
    &action_server,
    &this->remote_node,
    &clock,
    action_typesupport,
    server_action_name,
    &action_server_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&action_server, &this->remote_node)) <<
      rcl_get_error_string().str;
  });

  ret = rcl_action_get_names_and_types(&this->node, &this->allocator, &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 2u);
  EXPECT_STREQ(nat.names.data[0], client_action_name);
  EXPECT_STREQ(nat.names.data[1], server_action_name);
  EXPECT_EQ(nat.types[0].size, 1u);
  EXPECT_STREQ(nat.types[0].data[0], "test_msgs/Fibonacci");
  EXPECT_EQ(nat.types[1].size, 1u);
  EXPECT_STREQ(nat.types[1].data[0], "test_msgs/Fibonacci");

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

// Note, this test could be affected by other communication on the same ROS domain
TEST_F(TestActionGraphMultiNodeFixture, test_action_get_server_names_and_types_by_node) {
  rcl_ret_t ret;
  // Create an action client
  rcl_action_client_t action_client = rcl_action_get_zero_initialized_client();
  const rosidl_action_type_support_t * action_typesupport =
    ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
  rcl_action_client_options_t action_client_options = rcl_action_client_get_default_options();
  ret = rcl_action_client_init(
    &action_client,
    &this->remote_node,
    action_typesupport,
    this->action_name,
    &action_client_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&action_client, &this->remote_node)) <<
      rcl_get_error_string().str;
  });
  // Check that there are no action servers
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, &this->allocator, this->remote_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 0u);

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // Create an action server
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();
  rcl_clock_t clock;
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &this->allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string().str;
  });
  rcl_action_server_options_t action_server_options = rcl_action_server_get_default_options();
  ret = rcl_action_server_init(
    &action_server,
    &this->remote_node,
    &clock,
    action_typesupport,
    this->action_name,
    &action_server_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&action_server, &this->remote_node)) <<
      rcl_get_error_string().str;
  });

  // Wait for server to be ready
  bool is_available = false;
  do {
    ret = rcl_action_server_is_available(&this->remote_node, &action_client, &is_available);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  while (!is_available);

  ret = rcl_action_get_server_names_and_types_by_node(
    &this->node, &this->allocator, this->remote_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 1u);
  EXPECT_STREQ(nat.names.data[0], this->action_name);
  EXPECT_EQ(nat.types[0].size, 1u);
  EXPECT_STREQ(nat.types[0].data[0], "test_msgs/Fibonacci");

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

// Note, this test could be affected by other communication on the same ROS domain
TEST_F(TestActionGraphMultiNodeFixture, test_action_get_client_names_and_types_by_node) {
  rcl_ret_t ret;
  const rosidl_action_type_support_t * action_typesupport =
    ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
  // Create an action server
  rcl_action_server_t action_server = rcl_action_get_zero_initialized_server();
  rcl_clock_t clock;
  ret = rcl_clock_init(RCL_STEADY_TIME, &clock, &this->allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_clock_fini(&clock)) << rcl_get_error_string().str;
  });
  rcl_action_server_options_t action_server_options = rcl_action_server_get_default_options();
  ret = rcl_action_server_init(
    &action_server,
    &this->remote_node,
    &clock,
    action_typesupport,
    this->action_name,
    &action_server_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_action_server_fini(&action_server, &this->remote_node)) <<
      rcl_get_error_string().str;
  });

  // Check that there are no action clients
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, &this->allocator, this->remote_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 0u);

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // Create an action client
  rcl_action_client_t action_client = rcl_action_get_zero_initialized_client();
  rcl_action_client_options_t action_client_options = rcl_action_client_get_default_options();
  ret = rcl_action_client_init(
    &action_client,
    &this->remote_node,
    action_typesupport,
    this->action_name,
    &action_client_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    EXPECT_EQ(RCL_RET_OK, rcl_action_client_fini(&action_client, &this->remote_node)) <<
      rcl_get_error_string().str;
  });

  // Wait for server to be ready
  bool is_available = false;
  do {
    ret = rcl_action_server_is_available(&this->remote_node, &action_client, &is_available);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  while (!is_available);

  ret = rcl_action_get_client_names_and_types_by_node(
    &this->node, &this->allocator, this->remote_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 1u);
  EXPECT_STREQ(nat.names.data[0], this->action_name);
  EXPECT_EQ(nat.types[0].size, 1u);
  EXPECT_STREQ(nat.types[0].data[0], "test_msgs/Fibonacci");

  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}
