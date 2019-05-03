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

// Disable -Wmissing-field-initializers, as it is overly strict and will be
// relaxed in future versions of GCC (it is not a warning for clang).
// See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=36750#c13
#ifndef _WIN32
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <string>
#include <thread>
#include <vector>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/rcl.h"

#include "rcutils/logging_macros.h"
#include "rcutils/logging.h"

#include "test_msgs/msg/basic_types.h"
#include "test_msgs/srv/basic_types.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

bool is_connext =
  std::string(rmw_get_implementation_identifier()).find("rmw_connext") == 0;

bool is_opensplice =
  std::string(rmw_get_implementation_identifier()).find("rmw_opensplice") == 0;

class CLASSNAME (TestGraphFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_context_t * old_context_ptr;
  rcl_context_t * context_ptr;
  rcl_node_t * old_node_ptr;
  rcl_node_t * node_ptr;
  rcl_wait_set_t * wait_set_ptr;
  const char * test_graph_node_name = "test_graph_node";

  void SetUp()
  {
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });
    this->old_context_ptr = new rcl_context_t;
    *this->old_context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->old_context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->old_node_ptr = new rcl_node_t;
    *this->old_node_ptr = rcl_get_zero_initialized_node();
    const char * old_name = "old_node_name";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->old_node_ptr, old_name, "", this->old_context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(this->old_context_ptr);  // after this, the old_node_ptr should be invalid
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();

    ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_graph_node";
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->wait_set_ptr = new rcl_wait_set_t;
    *this->wait_set_ptr = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(
      this->wait_set_ptr, 0, 1, 0, 0, 0, 0, this->context_ptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret;
    ret = rcl_node_fini(this->old_node_ptr);
    delete this->old_node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_wait_set_fini(this->wait_set_ptr);
    delete this->wait_set_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->context_ptr;
    ret = rcl_context_fini(this->old_context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->old_context_ptr;
  }
};

/* Test the rcl_get_topic_names_and_types and rcl_destroy_topic_names_and_types functions.
 *
 * This does not test content of the rcl_names_and_types_t structure.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_get_and_destroy_topic_names_and_types
) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t zero_allocator = static_cast<rcl_allocator_t>(
    rcutils_get_zero_initialized_allocator());
  rcl_names_and_types_t tnat {};
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  // invalid node
  ret = rcl_get_topic_names_and_types(nullptr, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_topic_names_and_types(&zero_node, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_topic_names_and_types(this->old_node_ptr, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid allocator
  ret = rcl_get_topic_names_and_types(this->node_ptr, nullptr, false, &tnat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_topic_names_and_types(this->node_ptr, &zero_allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid topic_names_and_types
  ret = rcl_get_topic_names_and_types(this->node_ptr, &allocator, false, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid argument to rcl_destroy_topic_names_and_types
  ret = rcl_names_and_types_fini(nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // valid calls
  ret = rcl_get_topic_names_and_types(this->node_ptr, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ret = rcl_names_and_types_fini(&tnat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/* Test the rcl_get_service_names_and_types function.
 *
 * This does not test content of the rcl_names_and_types_t structure.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_get_service_names_and_types
) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t zero_allocator = static_cast<rcl_allocator_t>(
    rcutils_get_zero_initialized_allocator());
  rcl_names_and_types_t tnat {};
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  // invalid node
  ret = rcl_get_service_names_and_types(nullptr, &allocator, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types(&zero_node, &allocator, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types(this->old_node_ptr, &allocator, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid allocator
  ret = rcl_get_service_names_and_types(this->node_ptr, nullptr, &tnat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types(this->node_ptr, &zero_allocator, &tnat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid service_names_and_types
  ret = rcl_get_service_names_and_types(this->node_ptr, &allocator, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid argument to rcl_destroy_service_names_and_types
  ret = rcl_names_and_types_fini(nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // valid calls
  ret = rcl_get_service_names_and_types(this->node_ptr, &allocator, &tnat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ret = rcl_names_and_types_fini(&tnat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/* Test the rcl_names_and_types_init function.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_names_and_types_init
) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t zero_allocator = static_cast<rcl_allocator_t>(
    rcutils_get_zero_initialized_allocator());
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  // invalid names and types
  ret = rcl_names_and_types_init(nullptr, 10, &allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid allocator
  ret = rcl_names_and_types_init(&nat, 10, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_names_and_types_init(&nat, 10, &zero_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // zero size
  ret = rcl_names_and_types_init(&nat, 0, &allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, 0u);
  ret = rcl_names_and_types_fini(&nat);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  // non-zero size
  size_t num_names = 10u;
  ret = rcl_names_and_types_init(&nat, num_names, &allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(nat.names.size, num_names);
  for (size_t i = 0; i < num_names; i++) {
    EXPECT_EQ(nat.types[i].size, 0u);
  }
  ret = rcl_names_and_types_fini(&nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/* Test the rcl_get_publisher_names_and_types_by_node function.
 *
 * This does not test content of the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publisher_names_and_types_by_node
) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t zero_allocator = static_cast<rcl_allocator_t>(
    rcutils_get_zero_initialized_allocator());
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * unknown_node_name = "/test_rcl_get_publisher_names_and_types_by_node";
  // const char * unknown_node_ns = "/test/namespace";
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  // invalid node
  ret = rcl_get_publisher_names_and_types_by_node(
    nullptr, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_publisher_names_and_types_by_node(
    &zero_node, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_publisher_names_and_types_by_node(
    this->old_node_ptr, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid allocator
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, nullptr, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &zero_allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid names
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, nullptr, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, this->test_graph_node_name, nullptr, &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // test valid strings with invalid node names
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, "", "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, "_InvalidNodeName", "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // TODO(jacobperron): This succeeds, but should fail due to invalid namespace
  // ret = rcl_get_publisher_names_and_types_by_node(
  //   this->node_ptr, &allocator, false, this->test_graph_node_name, "_!invalidNs", &nat);
  // EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  // rcl_reset_error();
  // invalid names and types
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, this->test_graph_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // unknown node name
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, unknown_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // unknown node namespace
  // TODO(jacobperron): This succeeds, but should fail due to invalid namespace
  // ret = rcl_get_publisher_names_and_types_by_node(
  //   this->node_ptr, &allocator, false, this->test_graph_node_name, unknown_node_ns, &nat);
  // EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  // rcl_reset_error();
  // valid call
  ret = rcl_get_publisher_names_and_types_by_node(
    this->node_ptr, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

/* Test the rcl_get_subscriber_names_and_types_by_node function.
 *
 * This does not test content of the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriber_names_and_types_by_node
) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t zero_allocator = static_cast<rcl_allocator_t>(
    rcutils_get_zero_initialized_allocator());
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * unknown_node_name = "/test_rcl_get_subscriber_names_and_types_by_node";
  // const char * unknown_node_ns = "/test/namespace";
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  // invalid node
  ret = rcl_get_subscriber_names_and_types_by_node(
    nullptr, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_subscriber_names_and_types_by_node(
    &zero_node, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->old_node_ptr, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid allocator
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, nullptr, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &zero_allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid names
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, nullptr, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, this->test_graph_node_name, nullptr, &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // test valid strings with invalid node names
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, "", "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, "_InvalidNodeName", "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // TODO(jacobperron): This succeeds, but should fail due to invalid namespace
  // ret = rcl_get_subscriber_names_and_types_by_node(
  //   this->node_ptr, &allocator, false, this->test_graph_node_name, "_!invalidNs", &nat);
  // EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  // rcl_reset_error();
  // invalid names and types
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, this->test_graph_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // unknown node name
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, unknown_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // unknown node namespace
  // TODO(jacobperron): This succeeds, but should fail due to invalid namespace
  // ret = rcl_get_subscriber_names_and_types_by_node(
  //   this->node_ptr, &allocator, false, this->test_graph_node_name, unknown_node_ns, &nat);
  // EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  // rcl_reset_error();
  // valid call
  ret = rcl_get_subscriber_names_and_types_by_node(
    this->node_ptr, &allocator, false, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

/* Test the rcl_get_service_names_and_types_by_node function.
 *
 * This does not test content of the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_get_service_names_and_types_by_node
) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t zero_allocator = static_cast<rcl_allocator_t>(
    rcutils_get_zero_initialized_allocator());
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * unknown_node_name = "/test_rcl_get_service_names_and_types_by_node";
  // const char * unknown_node_ns = "/test/namespace";
  rcl_names_and_types_t nat = rcl_get_zero_initialized_names_and_types();
  // invalid node
  ret = rcl_get_service_names_and_types_by_node(
    nullptr, &allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types_by_node(
    &zero_node, &allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types_by_node(
    this->old_node_ptr, &allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid allocator
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, nullptr, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &zero_allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid names
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, nullptr, "", &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, this->test_graph_node_name, nullptr, &nat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // test valid strings with invalid node names
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, "", "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, "_InvalidNodeName", "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // TODO(jacobperron): This succeeds, but should fail due to invalid namespace
  // ret = rcl_get_service_names_and_types_by_node(
  //   this->node_ptr, &allocator, false, this->test_graph_node_name, "_!invalidNs", &nat);
  // EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  // rcl_reset_error();
  // invalid names and types
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, this->test_graph_node_name, "", nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // unknown node name
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, unknown_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // unknown node namespace
  // TODO(jacobperron): This succeeds, but should fail due to invalid namespace
  // ret = rcl_get_service_names_and_types_by_node(
  //   this->node_ptr, &allocator, this->test_graph_node_name, unknown_node_ns, &nat);
  // EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  // rcl_reset_error();
  // valid call
  ret = rcl_get_service_names_and_types_by_node(
    this->node_ptr, &allocator, this->test_graph_node_name, "", &nat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

/* Test the rcl_count_publishers function.
 *
 * This does not test content of the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_count_publishers
) {
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * topic_name = "/topic_test_rcl_count_publishers";
  size_t count;
  // invalid node
  ret = rcl_count_publishers(nullptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_count_publishers(&zero_node, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_count_publishers(this->old_node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid topic name
  ret = rcl_count_publishers(this->node_ptr, nullptr, &count);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // TODO(wjwwood): test valid strings with invalid topic names in them
  // invalid count
  ret = rcl_count_publishers(this->node_ptr, topic_name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // valid call
  ret = rcl_count_publishers(this->node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

/* Test the rcl_count_subscribers function.
 *
 * This does not test content of the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_count_subscribers
) {
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * topic_name = "/topic_test_rcl_count_subscribers";
  size_t count;
  // invalid node
  ret = rcl_count_subscribers(nullptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_count_subscribers(&zero_node, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  ret = rcl_count_subscribers(this->old_node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // invalid topic name
  ret = rcl_count_subscribers(this->node_ptr, nullptr, &count);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // TODO(wjwwood): test valid strings with invalid topic names in them
  // invalid count
  ret = rcl_count_subscribers(this->node_ptr, topic_name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // valid call
  ret = rcl_count_subscribers(this->node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

void
check_graph_state(
  const rcl_node_t * node_ptr,
  rcl_wait_set_t * wait_set_ptr,
  const rcl_guard_condition_t * graph_guard_condition,
  std::string & topic_name,
  size_t expected_publisher_count,
  size_t expected_subscriber_count,
  bool expected_in_tnat,
  size_t number_of_tries)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
    "Expecting %zu publishers, %zu subscribers, and that the topic is%s in the graph.",
    expected_publisher_count,
    expected_subscriber_count,
    expected_in_tnat ? "" : " not"
  );
  size_t publisher_count = 0;
  size_t subscriber_count = 0;
  bool is_in_tnat = false;
  rcl_names_and_types_t tnat {};
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  for (size_t i = 0; i < number_of_tries; ++i) {
    ret = rcl_count_publishers(node_ptr, topic_name.c_str(), &publisher_count);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();

    ret = rcl_count_subscribers(node_ptr, topic_name.c_str(), &subscriber_count);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();

    tnat = rcl_get_zero_initialized_names_and_types();
    ret = rcl_get_topic_names_and_types(node_ptr, &allocator, false, &tnat);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    is_in_tnat = false;
    for (size_t i = 0; RCL_RET_OK == ret && i < tnat.names.size; ++i) {
      if (topic_name == std::string(tnat.names.data[i])) {
        ASSERT_FALSE(is_in_tnat) << "duplicates in the tnat";  // Found it more than once!
        is_in_tnat = true;
      }
    }
    if (RCL_RET_OK == ret) {
      ret = rcl_names_and_types_fini(&tnat);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      rcl_reset_error();
    }

    RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME,
      " Try %zu: %zu publishers, %zu subscribers, and that the topic is%s in the graph.",
      i + 1,
      publisher_count,
      subscriber_count,
      is_in_tnat ? "" : " not"
    );
    if (
      expected_publisher_count == publisher_count &&
      expected_subscriber_count == subscriber_count &&
      expected_in_tnat == is_in_tnat)
    {
      RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "  state correct!");
      break;
    }
    // Wait for graph change before trying again.
    if ((i + 1) == number_of_tries) {
      // Don't wait for the graph to change on the last loop because we won't check again.
      continue;
    }
    ret = rcl_wait_set_clear(wait_set_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait_set_add_guard_condition(wait_set_ptr, graph_guard_condition, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(400);
    RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME,
      "  state wrong, waiting up to '%s' nanoseconds for graph changes... ",
      std::to_string(time_to_sleep.count()).c_str());
    ret = rcl_wait(wait_set_ptr, time_to_sleep.count());
    if (ret == RCL_RET_TIMEOUT) {
      RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "timeout");
      continue;
    }
    RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "change occurred");
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  EXPECT_EQ(expected_publisher_count, publisher_count);
  EXPECT_EQ(expected_subscriber_count, subscriber_count);
  if (expected_in_tnat) {
    EXPECT_TRUE(is_in_tnat);
  } else {
    EXPECT_FALSE(is_in_tnat);
  }
}

/**
 * Type define a get topics function.
 */
typedef std::function<rcl_ret_t(const rcl_node_t *,
    const char * node_name,
    rcl_names_and_types_t *)> GetTopicsFunc;

/**
 * Expect a certain number of topics on a given subsystem.
 */
void expect_topics_types(
  const rcl_node_t * node,
  const GetTopicsFunc & func,
  size_t num_topics,
  const char * topic_name,
  bool expect,
  bool & is_success)
{
  rcl_ret_t ret;
  rcl_names_and_types_t nat{};
  nat = rcl_get_zero_initialized_names_and_types();
  ret = func(node, topic_name, &nat);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  is_success &= num_topics == nat.names.size;
  if (expect) {
    EXPECT_EQ(num_topics, nat.names.size);
  } else {
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Expected topics %zu, actual topics %zu", num_topics,
      nat.names.size);
  }
  ret = rcl_names_and_types_fini(&nat);
  ASSERT_EQ(RCL_RET_OK, ret);
  rcl_reset_error();
}

/**
 * Expected state of a node.
 */
struct expected_node_state
{
  size_t publishers;
  size_t subscribers;
  size_t services;
};

/**
 * Extend the TestGraphFixture with a multi node fixture for node discovery and node-graph perspective.
 */
class NodeGraphMultiNodeFixture : public CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION)
{
public:
  const char * remote_node_name = "remote_graph_node";
  std::string topic_name = "/test_node_info_functions__";
  rcl_node_t * remote_node_ptr;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  GetTopicsFunc sub_func, pub_func, service_func;
  rcl_context_t * remote_context_ptr;

  void SetUp() override
  {
    CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION) ::SetUp();
    rcl_ret_t ret;

    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) <<
        rcl_get_error_string().str;
    });

    remote_node_ptr = new rcl_node_t;
    *remote_node_ptr = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();

    this->remote_context_ptr = new rcl_context_t;
    *this->remote_context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->remote_context_ptr);

    ret = rcl_node_init(remote_node_ptr, remote_node_name, "", this->remote_context_ptr,
        &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    sub_func = std::bind(rcl_get_subscriber_names_and_types_by_node,
        std::placeholders::_1,
        &this->allocator,
        false,
        std::placeholders::_2,
        "/",
        std::placeholders::_3);
    pub_func = std::bind(rcl_get_publisher_names_and_types_by_node,
        std::placeholders::_1,
        &this->allocator,
        false,
        std::placeholders::_2,
        "/",
        std::placeholders::_3);
    service_func = std::bind(rcl_get_service_names_and_types_by_node,
        std::placeholders::_1,
        &this->allocator,
        std::placeholders::_2,
        "/",
        std::placeholders::_3);
    WaitForAllNodesAlive();
  }

  void TearDown() override
  {
    rcl_ret_t ret;
    CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION) ::TearDown();
    ret = rcl_node_fini(this->remote_node_ptr);

    delete this->remote_node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Tearing down class");

    ret = rcl_shutdown(this->remote_context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(this->remote_context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->remote_context_ptr;
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
    // wait for all 3 nodes to be discovered: remote_node, old_node, node
    size_t attempts = 0;
    size_t max_attempts = 4;
    while (node_names.size < 3) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      ret = rcl_get_node_names(this->remote_node_ptr, allocator, &node_names, &node_namespaces);
      attempts++;
      ASSERT_LE(attempts, max_attempts) << "Unable to attain all required nodes";
    }
  }

  /**
   * Verify the number of subsystems each node should have.
   *
   * @param node_state expected state of node
   * @param remote_node_state expected state of remote node
   */
  void VerifySubsystemCount(
    const expected_node_state && node_state,
    const expected_node_state && remote_node_state) const
  {
    std::vector<rcl_node_t *> node_vec;
    node_vec.push_back(this->node_ptr);
    node_vec.push_back(this->remote_node_ptr);

    size_t attempts = 20;
    bool is_expect = false;
    rcl_ret_t ret;

    for (size_t i = 0; i < attempts; ++i) {
      if (attempts - 1 == i) {is_expect = true;}
      bool is_success = true;
      // verify each node contains the same node graph.
      for (auto node : node_vec) {
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Checking subscribers from node");
        expect_topics_types(node, sub_func, node_state.subscribers,
          test_graph_node_name, is_expect, is_success);
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Checking services from node");
        expect_topics_types(node, service_func, node_state.services,
          test_graph_node_name, is_expect, is_success);
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Checking publishers from node");
        expect_topics_types(node, pub_func, node_state.publishers,
          test_graph_node_name, is_expect, is_success);

        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Checking subscribers from remote node");
        expect_topics_types(node, sub_func, remote_node_state.subscribers,
          this->remote_node_name, is_expect, is_success);
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Checking publishers from remote node");
        expect_topics_types(node, pub_func, remote_node_state.publishers,
          this->remote_node_name, is_expect, is_success);
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Checking services from remote node");
        expect_topics_types(node, service_func, remote_node_state.services,
          this->remote_node_name, is_expect, is_success);
        if (!is_success) {
          ret = rcl_wait_set_clear(wait_set_ptr);
          ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
          ret =
            rcl_wait_set_add_guard_condition(wait_set_ptr, rcl_node_get_graph_guard_condition(
                node), NULL);
          ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
          std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(400);
          RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
            "  state wrong, waiting up to '%s' nanoseconds for graph changes... ",
            std::to_string(time_to_sleep.count()).c_str());
          ret = rcl_wait(wait_set_ptr, time_to_sleep.count());
          if (ret == RCL_RET_TIMEOUT) {
            RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "timeout");
            continue;
          }
          RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "change occurred");
          ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
          break;
        }
      }
      if (is_success) {
        break;
      }
    }
  }
};

TEST_F(NodeGraphMultiNodeFixture, test_node_info_subscriptions)
{
  rcl_ret_t ret;
  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  // Create two subscribers.
  rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub, this->node_ptr, ts, this->topic_name.c_str(), &sub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  rcl_subscription_t sub2 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_ops2 = rcl_subscription_get_default_options();
  ret =
    rcl_subscription_init(&sub2, this->remote_node_ptr, ts, this->topic_name.c_str(), &sub_ops2);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  VerifySubsystemCount(expected_node_state{1, 1, 0}, expected_node_state{1, 1, 0});

  // Destroy the node's subscriber
  ret = rcl_subscription_fini(&sub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  VerifySubsystemCount(expected_node_state{1, 0, 0}, expected_node_state{1, 1, 0});

  // Destroy the remote node's subdscriber
  ret = rcl_subscription_fini(&sub2, this->remote_node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  VerifySubsystemCount(expected_node_state{1, 0, 0}, expected_node_state{1, 0, 0});
}

TEST_F(NodeGraphMultiNodeFixture, test_node_info_publishers)
{
  rcl_ret_t ret;
  // Now create a publisher on "topic_name" and check that it is seen.
  rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  ret = rcl_publisher_init(&pub, this->node_ptr, ts, this->topic_name.c_str(), &pub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  VerifySubsystemCount(expected_node_state{2, 0, 0}, expected_node_state{1, 0, 0});

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Destroyed publisher");
  // Destroy the publisher.
  ret = rcl_publisher_fini(&pub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  VerifySubsystemCount(expected_node_state{1, 0, 0}, expected_node_state{1, 0, 0});
}

TEST_F(NodeGraphMultiNodeFixture, test_node_info_services)
{
  rcl_ret_t ret;
  const char * service_name = "test_service";
  rcl_service_t service = rcl_get_zero_initialized_service();
  rcl_service_options_t service_options = rcl_service_get_default_options();
  auto ts1 = ROSIDL_GET_SRV_TYPE_SUPPORT(test_msgs, srv, BasicTypes);
  ret = rcl_service_init(&service, this->node_ptr, ts1, service_name, &service_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  VerifySubsystemCount(expected_node_state{1, 0, 1}, expected_node_state{1, 0, 0});

  // Destroy service.
  ret = rcl_service_fini(&service, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  VerifySubsystemCount(expected_node_state{1, 0, 0}, expected_node_state{1, 0, 0});
}

/*
 * Test graph queries with a hand crafted graph.
 */
TEST_F(CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION), test_graph_query_functions)
{
  std::string topic_name("/test_graph_query_functions__");
  std::chrono::nanoseconds now = std::chrono::system_clock::now().time_since_epoch();
  topic_name += std::to_string(now.count());
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Using topic name: %s", topic_name.c_str());
  rcl_ret_t ret;
  const rcl_guard_condition_t * graph_guard_condition =
    rcl_node_get_graph_guard_condition(this->node_ptr);
  // First assert the "topic_name" is not in use.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    0,    // expected publishers on topic
    0,    // expected subscribers on topic
    false,    // topic expected in graph
    9);    // number of retries
  // Now create a publisher on "topic_name" and check that it is seen.
  rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  ret = rcl_publisher_init(&pub, this->node_ptr, ts, topic_name.c_str(), &pub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Check the graph.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    1,  // expected publishers on topic
    0,  // expected subscribers on topic
    true,  // topic expected in graph
    9);  // number of retries
  // Now create a subscriber.
  rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub, this->node_ptr, ts, topic_name.c_str(), &sub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Check the graph again.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    1,  // expected publishers on topic
    1,  // expected subscribers on topic
    true,  // topic expected in graph
    9);  // number of retries
  // Destroy the publisher.
  ret = rcl_publisher_fini(&pub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Check the graph again.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    0,  // expected publishers on topic
    1,  // expected subscribers on topic
    true,  // topic expected in graph
    9);  // number of retries
  // Destroy the subscriber.
  ret = rcl_subscription_fini(&sub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  // Check the graph again.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    0,  // expected publishers on topic
    0,  // expected subscribers on topic
    false,  // topic expected in graph
    9);  // number of retries
}

/* Test the graph guard condition notices topic changes.
 *
 * Note: this test could be impacted by other communications on the same ROS Domain.
 */
TEST_F(CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION), test_graph_guard_condition_topics) {
  rcl_ret_t ret;
  // Create a thread to sleep for a time, then create a publisher, sleep more, then a subscriber,
  // sleep more, destroy the subscriber, sleep more, and then destroy the publisher.
  std::promise<bool> topic_changes_promise;
  std::thread topic_thread(
    [this, &topic_changes_promise]() {
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      // create the publisher
      rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
      rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
      rcl_ret_t ret = rcl_publisher_init(
        &pub, this->node_ptr, ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes),
        "/chatter_test_graph_guard_condition_topics", &pub_ops);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      // create the subscription
      rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
      rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
      ret = rcl_subscription_init(
        &sub, this->node_ptr, ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes),
        "/chatter_test_graph_guard_condition_topics", &sub_ops);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      // destroy the subscription
      ret = rcl_subscription_fini(&sub, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      // destroy the publication
      ret = rcl_publisher_fini(&pub, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      // notify that the thread is done
      topic_changes_promise.set_value(true);
    });
  // Wait for the graph state to change, expecting it to do so at least 4 times,
  // once for each change in the topics thread.
  const rcl_guard_condition_t * graph_guard_condition =
    rcl_node_get_graph_guard_condition(this->node_ptr);
  ASSERT_NE(nullptr, graph_guard_condition) << rcl_get_error_string().str;
  std::shared_future<bool> future = topic_changes_promise.get_future();
  size_t graph_changes_count = 0;
  // while the topic thread is not done, wait and count the graph changes
  while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    ret = rcl_wait_set_clear(this->wait_set_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait_set_add_guard_condition(this->wait_set_ptr, graph_guard_condition, NULL);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(400);
    RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME,
      "waiting up to '%s' nanoseconds for graph changes",
      std::to_string(time_to_sleep.count()).c_str());
    ret = rcl_wait(this->wait_set_ptr, time_to_sleep.count());
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    graph_changes_count++;
  }
  topic_thread.join();
  // expect at least 4 changes
  ASSERT_GE(graph_changes_count, 4ul);
}

/* Test the rcl_service_server_is_available function.
 */
TEST_F(CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION), test_rcl_service_server_is_available) {
  rcl_ret_t ret;
  // First create a client which will be used to call the function.
  rcl_client_t client = rcl_get_zero_initialized_client();
  auto ts = ROSIDL_GET_SRV_TYPE_SUPPORT(test_msgs, srv, BasicTypes);
  const char * service_name = "/service_test_rcl_service_server_is_available";
  rcl_client_options_t client_options = rcl_client_get_default_options();
  ret = rcl_client_init(&client, this->node_ptr, ts, service_name, &client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_client_fini(&client, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  // Check, knowing there is no service server (created by us at least).
  bool is_available;
  ret = rcl_service_server_is_available(this->node_ptr, &client, &is_available);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_FALSE(is_available);
  // Setup function to wait for service state to change using graph guard condition.
  const rcl_guard_condition_t * graph_guard_condition =
    rcl_node_get_graph_guard_condition(this->node_ptr);
  ASSERT_NE(nullptr, graph_guard_condition) << rcl_get_error_string().str;
  auto wait_for_service_state_to_change = [this, &graph_guard_condition, &client](
    bool expected_state,
    bool & is_available)
    {
      is_available = false;
      auto start = std::chrono::steady_clock::now();
      auto end = start + std::chrono::seconds(10);
      while (std::chrono::steady_clock::now() < end) {
        // We wait multiple times in case other graph changes are occurring simultaneously.
        std::chrono::nanoseconds time_left = end - start;
        std::chrono::nanoseconds time_to_sleep = time_left;
        std::chrono::seconds min_sleep(1);
        if (time_to_sleep > min_sleep) {
          time_to_sleep = min_sleep;
        }
        rcl_ret_t ret = rcl_wait_set_clear(this->wait_set_ptr);
        ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
        ret = rcl_wait_set_add_guard_condition(this->wait_set_ptr, graph_guard_condition, NULL);
        ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
        RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME,
          "waiting up to '%s' nanoseconds for graph changes",
          std::to_string(time_to_sleep.count()).c_str());
        ret = rcl_wait(this->wait_set_ptr, time_to_sleep.count());
        if (ret == RCL_RET_TIMEOUT) {
          if (!is_connext) {
            // TODO(wjwwood):
            //   Connext has a race condition which can cause the graph guard
            //   condition to wake up due to the necessary topics going away,
            //   but afterwards rcl_service_server_is_available() still does
            //   not reflect that the service is "no longer available".
            //   The result is that some tests are flaky unless you not only
            //   check right after a graph change but again in the future where
            //   rcl_service_server_is_available() eventually reports the
            //   service is no longer there. This condition can be removed and
            //   we can always continue when we get RCL_RET_TIMEOUT once that
            //   is fixed.
            continue;
          }
        } else {
          ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
        }
        ret = rcl_service_server_is_available(this->node_ptr, &client, &is_available);
        ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
        if (is_available == expected_state) {
          break;
        }
      }
    };
  {
    // Create the service server.
    rcl_service_t service = rcl_get_zero_initialized_service();
    rcl_service_options_t service_options = rcl_service_get_default_options();
    ret = rcl_service_init(&service, this->node_ptr, ts, service_name, &service_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      rcl_ret_t ret = rcl_service_fini(&service, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    });
    // Wait for and then assert that it is available.
    wait_for_service_state_to_change(true, is_available);
    ASSERT_TRUE(is_available);
  }
  // Assert the state goes back to "not available" after the service is removed.
  wait_for_service_state_to_change(false, is_available);
  ASSERT_FALSE(is_available);
}
