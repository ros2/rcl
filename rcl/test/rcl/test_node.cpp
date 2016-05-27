// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include <string>

#include "rcl/rcl.h"
#include "rcl/node.h"
#include "rmw/rmw.h"  // For rmw_get_implementation_identifier.

#include "../memory_tools/memory_tools.hpp"
#include "../scope_exit.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestNodeFixture, RMW_IMPLEMENTATION) : public ::testing::Test
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

bool is_opensplice =
  std::string(rmw_get_implementation_identifier()).find("opensplice") != std::string::npos;
#if defined(WIN32)
bool is_windows = true;
#else
bool is_windows = false;
#endif

/* Tests the node accessors, i.e. rcl_node_get_* functions.
 */
TEST_F(CLASSNAME(TestNodeFixture, RMW_IMPLEMENTATION), test_rcl_node_accessors) {
  stop_memory_checking();
  rcl_ret_t ret;
  // Initialize rcl with rcl_init().
  ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret);  // Shutdown later after invalid node.
  // Create an invalid node (rcl_shutdown).
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  const char * name = "node_name";
  rcl_node_options_t default_options = rcl_node_get_default_options();
  default_options.domain_id = 42;  // Set the domain id to something explicit.
  ret = rcl_node_init(&invalid_node, name, &default_options);
  if (is_windows && is_opensplice) {
    // On Windows with OpenSplice, setting the domain id is not expected to work.
    ASSERT_NE(RCL_RET_OK, ret);
    // So retry with the default domain id setting (uses the environment as is).
    default_options.domain_id = rcl_node_get_default_options().domain_id;
    ret = rcl_node_init(&invalid_node, name, &default_options);
    ASSERT_EQ(RCL_RET_OK, ret);
  } else {
    // This is the normal check (not windows and windows if not opensplice)
    ASSERT_EQ(RCL_RET_OK, ret);
  }
  auto rcl_invalid_node_exit = make_scope_exit([&invalid_node]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_node_fini(&invalid_node);
    EXPECT_EQ(RCL_RET_OK, ret);
  });
  ret = rcl_shutdown();  // Shutdown to invalidate the node.
  ASSERT_EQ(RCL_RET_OK, ret);
  ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret);
  auto rcl_shutdown_exit = make_scope_exit([]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_shutdown();
    ASSERT_EQ(RCL_RET_OK, ret);
  });
  // Create a zero init node.
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  // Create a normal node.
  rcl_node_t node = rcl_get_zero_initialized_node();
  ret = rcl_node_init(&node, name, &default_options);
  ASSERT_EQ(RCL_RET_OK, ret);
  auto rcl_node_exit = make_scope_exit([&node]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_node_fini(&node);
    EXPECT_EQ(RCL_RET_OK, ret);
  });
  // Test rcl_node_is_valid().
  bool is_valid;
  is_valid = rcl_node_is_valid(nullptr);
  EXPECT_EQ(false, is_valid);
  rcl_reset_error();
  is_valid = rcl_node_is_valid(&zero_node);
  EXPECT_EQ(false, is_valid);
  rcl_reset_error();
  is_valid = rcl_node_is_valid(&invalid_node);
  EXPECT_EQ(false, is_valid);
  rcl_reset_error();
  is_valid = rcl_node_is_valid(&node);
  EXPECT_EQ(true, is_valid);
  rcl_reset_error();
  // Test rcl_node_get_name().
  const char * actual_node_name;
  actual_node_name = rcl_node_get_name(nullptr);
  EXPECT_EQ(nullptr, actual_node_name);
  rcl_reset_error();
  actual_node_name = rcl_node_get_name(&zero_node);
  EXPECT_EQ(nullptr, actual_node_name);
  rcl_reset_error();
  actual_node_name = rcl_node_get_name(&invalid_node);
  EXPECT_EQ(nullptr, actual_node_name);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  actual_node_name = rcl_node_get_name(&node);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_TRUE(actual_node_name ? true : false);
  if (actual_node_name) {
    EXPECT_EQ(std::string(name), std::string(actual_node_name));
  }
  // Test rcl_node_get_options().
  const rcl_node_options_t * actual_options;
  actual_options = rcl_node_get_options(nullptr);
  EXPECT_EQ(nullptr, actual_options);
  rcl_reset_error();
  actual_options = rcl_node_get_options(&zero_node);
  EXPECT_EQ(nullptr, actual_options);
  rcl_reset_error();
  actual_options = rcl_node_get_options(&invalid_node);
  EXPECT_EQ(nullptr, actual_options);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  actual_options = rcl_node_get_options(&node);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_NE(nullptr, actual_options);
  if (actual_options) {
    EXPECT_EQ(default_options.allocator.allocate, actual_options->allocator.allocate);
    EXPECT_EQ(default_options.domain_id, actual_options->domain_id);
  }
  // Test rcl_node_get_domain_id().
  size_t actual_domain_id;
  ret = rcl_node_get_domain_id(nullptr, &actual_domain_id);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ret = rcl_node_get_domain_id(&zero_node, &actual_domain_id);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
  rcl_reset_error();
  ret = rcl_node_get_domain_id(&invalid_node, &actual_domain_id);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  ret = rcl_node_get_domain_id(&node, &actual_domain_id);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(RCL_RET_OK, ret);
  if (RCL_RET_OK == ret && (!is_windows || !is_opensplice)) {
    // Can only expect the domain id to be 42 if not windows or not opensplice.
    EXPECT_EQ(42u, actual_domain_id);
  }
  // Test rcl_node_get_rmw_handle().
  rmw_node_t * node_handle;
  node_handle = rcl_node_get_rmw_handle(nullptr);
  EXPECT_EQ(nullptr, node_handle);
  rcl_reset_error();
  node_handle = rcl_node_get_rmw_handle(&zero_node);
  EXPECT_EQ(nullptr, node_handle);
  rcl_reset_error();
  node_handle = rcl_node_get_rmw_handle(&invalid_node);
  EXPECT_EQ(nullptr, node_handle);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  node_handle = rcl_node_get_rmw_handle(&node);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_NE(nullptr, node_handle);
  // Test rcl_node_get_rcl_instance_id().
  uint64_t instance_id;
  instance_id = rcl_node_get_rcl_instance_id(nullptr);
  EXPECT_EQ(0u, instance_id);
  rcl_reset_error();
  instance_id = rcl_node_get_rcl_instance_id(&zero_node);
  EXPECT_EQ(0u, instance_id);
  rcl_reset_error();
  instance_id = rcl_node_get_rcl_instance_id(&invalid_node);
  EXPECT_NE(0u, instance_id);
  EXPECT_NE(42u, instance_id);
  rcl_reset_error();
  start_memory_checking();
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  instance_id = rcl_node_get_rcl_instance_id(&node);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_NE(0u, instance_id);
  // Test rcl_node_get_graph_guard_condition
  std::string rmw_id(rmw_get_implementation_identifier());
  if (rmw_id.find("opensplice") != std::string::npos) {
    // Only test with opensplice for now, as connext and fastrtps are not working.
    // TODO(wjwwood): remove this check when more middlewares implement this
    const rcl_guard_condition_t * graph_guard_condition = nullptr;
    graph_guard_condition = rcl_node_get_graph_guard_condition(nullptr);
    EXPECT_EQ(nullptr, graph_guard_condition);
    rcl_reset_error();
    graph_guard_condition = rcl_node_get_graph_guard_condition(&zero_node);
    EXPECT_EQ(nullptr, graph_guard_condition);
    rcl_reset_error();
    graph_guard_condition = rcl_node_get_graph_guard_condition(&invalid_node);
    EXPECT_EQ(nullptr, graph_guard_condition);
    rcl_reset_error();
    start_memory_checking();
    assert_no_malloc_begin();
    assert_no_realloc_begin();
    assert_no_free_begin();
    graph_guard_condition = rcl_node_get_graph_guard_condition(&node);
    assert_no_malloc_end();
    assert_no_realloc_end();
    assert_no_free_end();
    stop_memory_checking();
    EXPECT_NE(nullptr, graph_guard_condition);
  }
}

/* Tests the node life cycle, including rcl_node_init() and rcl_node_fini().
 */
TEST_F(CLASSNAME(TestNodeFixture, RMW_IMPLEMENTATION), test_rcl_node_life_cycle) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_node_t node = rcl_get_zero_initialized_node();
  const char * name = "node_name";
  rcl_node_options_t default_options = rcl_node_get_default_options();
  // Trying to init before rcl_init() should fail.
  ret = rcl_node_init(&node, name, &default_options);
  ASSERT_EQ(RCL_RET_NOT_INIT, ret) << "Expected RCL_RET_NOT_INIT";
  rcl_reset_error();
  // Initialize rcl with rcl_init().
  ret = rcl_init(0, nullptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret);
  auto rcl_shutdown_exit = make_scope_exit([]() {
    rcl_ret_t ret = rcl_shutdown();
    ASSERT_EQ(RCL_RET_OK, ret);
  });
  // Try invalid arguments.
  ret = rcl_node_init(nullptr, name, &default_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ret = rcl_node_init(&node, nullptr, &default_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ret = rcl_node_init(&node, name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  // Try with invalid allocator.
  rcl_node_options_t options_with_invalid_allocator = rcl_node_get_default_options();
  options_with_invalid_allocator.allocator.allocate = nullptr;
  options_with_invalid_allocator.allocator.deallocate = nullptr;
  options_with_invalid_allocator.allocator.reallocate = nullptr;
  ret = rcl_node_init(&node, name, &options_with_invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << "Expected RCL_RET_INVALID_ARGUMENT";
  rcl_reset_error();
  // Try with failing allocator.
  rcl_node_options_t options_with_failing_allocator = rcl_node_get_default_options();
  options_with_failing_allocator.allocator.allocate = failing_malloc;
  options_with_failing_allocator.allocator.deallocate = failing_free;
  options_with_failing_allocator.allocator.reallocate = failing_realloc;
  ret = rcl_node_init(&node, name, &options_with_failing_allocator);
  EXPECT_EQ(RCL_RET_BAD_ALLOC, ret) << "Expected RCL_RET_BAD_ALLOC";
  rcl_reset_error();
  // Try fini with invalid arguments.
  ret = rcl_node_fini(nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << "Expected RCL_RET_INVALID_ARGUMENT";
  rcl_reset_error();
  // Try fini with an uninitialized node.
  ret = rcl_node_fini(&node);
  EXPECT_EQ(RCL_RET_OK, ret);
  // Try a normal init and fini.
  ret = rcl_node_init(&node, name, &default_options);
  EXPECT_EQ(RCL_RET_OK, ret);
  ret = rcl_node_fini(&node);
  EXPECT_EQ(RCL_RET_OK, ret);
  // Try repeated init and fini calls.
  ret = rcl_node_init(&node, name, &default_options);
  EXPECT_EQ(RCL_RET_OK, ret);
  ret = rcl_node_init(&node, name, &default_options);
  EXPECT_EQ(RCL_RET_ALREADY_INIT, ret) << "Expected RCL_RET_ALREADY_INIT";
  ret = rcl_node_fini(&node);
  EXPECT_EQ(RCL_RET_OK, ret);
  ret = rcl_node_fini(&node);
  EXPECT_EQ(RCL_RET_OK, ret);
  // Try with a specific domain id.
  rcl_node_options_t options_with_custom_domain_id = rcl_node_get_default_options();
  options_with_custom_domain_id.domain_id = 42;
  ret = rcl_node_init(&node, name, &options_with_custom_domain_id);
  if (is_windows && is_opensplice) {
    // A custom domain id is not expected to work on Windows with Opensplice.
    EXPECT_NE(RCL_RET_OK, ret);
  } else {
    // This is the normal check.
    EXPECT_EQ(RCL_RET_OK, ret);
    ret = rcl_node_fini(&node);
    EXPECT_EQ(RCL_RET_OK, ret);
  }
}
