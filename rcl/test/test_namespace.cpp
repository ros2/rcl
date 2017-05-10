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

#include "rcl/client.h"

#include "rcl/rcl.h"
#include "rcl/graph.h"

#include "example_interfaces/srv/add_two_ints.h"
#include "rosidl_generator_c/string_functions.h"

#include "./memory_tools/memory_tools.hpp"
#include "./scope_exit.hpp"
#include "rcl/error_handling.h"

using namespace std::chrono_literals;

class TestNamespaceFixture : public ::testing::Test
{
public:
  rcl_node_t * node_ptr;
  void SetUp()
  {
    stop_memory_checking();
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "rcl_test_namespace_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
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
    rcl_ret_t ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};

/* Basic nominal test of a client.
 */
TEST_F(TestNamespaceFixture, test_client_server) {
  stop_memory_checking();

  rcl_ret_t ret;
  auto ts = ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, AddTwoInts);
  const char * service_name = "/my/namespace/test_namespace_client_server";
  const char * unmatched_client_name = "/your/namespace/test_namespace_client_server";
  const char * matched_client_name = "/my/namespace/test_namespace_client_server";
  auto timeout = 10;

  rcl_service_t service = rcl_get_zero_initialized_service();
  rcl_service_options_t service_options = rcl_service_get_default_options();
  ret = rcl_service_init(&service, this->node_ptr, ts, service_name, &service_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto service_exit = make_scope_exit([&service, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_service_fini(&service, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });

  rcl_client_t unmatched_client = rcl_get_zero_initialized_client();
  rcl_client_options_t unmatched_client_options = rcl_client_get_default_options();
  ret = rcl_client_init(
    &unmatched_client, this->node_ptr, ts, unmatched_client_name, &unmatched_client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto unmatched_client_exit = make_scope_exit([&unmatched_client, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_client_fini(&unmatched_client, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });

  bool is_available = false;
  for (auto i = 0; i < timeout; ++i) {
    ret = rcl_service_server_is_available(this->node_ptr, &unmatched_client, &is_available);
    if (is_available) {
      // this should not happen
      break;
    }
    std::this_thread::sleep_for(1s);
  }
  ASSERT_FALSE(is_available);

  rcl_client_t matched_client = rcl_get_zero_initialized_client();
  rcl_client_options_t matched_client_options = rcl_client_get_default_options();
  ret = rcl_client_init(
    &matched_client, this->node_ptr, ts, matched_client_name, &matched_client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto matched_client_exit = make_scope_exit([&matched_client, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_client_fini(&matched_client, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });

  is_available = false;
  for (auto i = 0; i < timeout; ++i) {
    ret = rcl_service_server_is_available(this->node_ptr, &matched_client, &is_available);
    if (is_available) {
      break;
    }
    std::this_thread::sleep_for(1s);
  }
  ASSERT_TRUE(is_available);
}
