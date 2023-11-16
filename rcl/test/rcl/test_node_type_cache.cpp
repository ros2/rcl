// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#include "rcl/node_type_cache.h"
#include "rcl/rcl.h"
#include "rmw/rmw.h"

#include "test_msgs/msg/basic_types.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"
#include "rcl/node.h"
#include "rcutils/env.h"

class TestNodeTypeCacheFixture : public ::testing::Test
{
public:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;
  void SetUp()
  {
    rcl_ret_t ret;
    {
      rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
      ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
      {
        EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options))
          << rcl_get_error_string().str;
      });
      this->context_ptr = new rcl_context_t;
      *this->context_ptr = rcl_get_zero_initialized_context();
      ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    constexpr char name[] = "test_type_cache_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(
      this->node_ptr, name, "", this->context_ptr,
      &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(this->context_ptr);
    delete this->context_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};

TEST_F(TestNodeTypeCacheFixture, test_type_cache_invalid_args) {
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  rcl_type_info_t type_info;

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_node_type_cache_register_type(
      NULL, ts->get_type_hash_func(ts),
      ts->get_type_description_func(ts), ts->get_type_description_sources_func(ts)));
  rcl_reset_error();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_node_type_cache_register_type(
      this->node_ptr, NULL,
      ts->get_type_description_func(ts), ts->get_type_description_sources_func(ts)));
  rcl_reset_error();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT, rcl_node_type_cache_register_type(
      this->node_ptr, ts->get_type_hash_func(ts), NULL, ts->get_type_description_sources_func(ts)));
  rcl_reset_error();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT, rcl_node_type_cache_register_type(
      this->node_ptr, ts->get_type_hash_func(ts), ts->get_type_description_func(ts), NULL));
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_node_type_cache_unregister_type(NULL, ts->get_type_hash_func(ts)));
  rcl_reset_error();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_node_type_cache_unregister_type(this->node_ptr, NULL));
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_node_type_cache_get_type_info(NULL, ts->get_type_hash_func(ts), &type_info));
  rcl_reset_error();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT, rcl_node_type_cache_get_type_info(
      this->node_ptr, NULL, &type_info));
  rcl_reset_error();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT, rcl_node_type_cache_get_type_info(
      this->node_ptr, ts->get_type_hash_func(ts), NULL));
  rcl_reset_error();
}

TEST_F(TestNodeTypeCacheFixture, test_type_registration_count) {
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  rcl_type_info_t type_info;

  // Register once
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_node_type_cache_register_type(
      this->node_ptr, ts->get_type_hash_func(ts),
      ts->get_type_description_func(ts), ts->get_type_description_sources_func(ts)));
  EXPECT_EQ(
    RCL_RET_OK, rcl_node_type_cache_get_type_info(
      this->node_ptr, ts->get_type_hash_func(ts), &type_info));

  // Unregister once and confirm that it got removed from the type cache
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_node_type_cache_unregister_type(this->node_ptr, ts->get_type_hash_func(ts)));
  EXPECT_EQ(
    RCL_RET_ERROR, rcl_node_type_cache_get_type_info(
      this->node_ptr, ts->get_type_hash_func(ts), &type_info));
  rcl_reset_error();

  // Register twice and unregister once. Type info should still be available
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_node_type_cache_register_type(
      this->node_ptr, ts->get_type_hash_func(ts),
      ts->get_type_description_func(ts), ts->get_type_description_sources_func(ts)));
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_node_type_cache_register_type(
      this->node_ptr, ts->get_type_hash_func(ts),
      ts->get_type_description_func(ts), ts->get_type_description_sources_func(ts)));
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_node_type_cache_unregister_type(this->node_ptr, ts->get_type_hash_func(ts)));
  EXPECT_EQ(
    RCL_RET_OK, rcl_node_type_cache_get_type_info(
      this->node_ptr, ts->get_type_hash_func(ts), &type_info));
}

TEST_F(TestNodeTypeCacheFixture, test_invalid_unregistration) {
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  EXPECT_EQ(
    RCL_RET_ERROR,
    rcl_node_type_cache_unregister_type(this->node_ptr, ts->get_type_hash_func(ts)));
  rcl_reset_error();
}
