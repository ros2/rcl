// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include "rcl_action/action_client.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "test_msgs/action/fibonacci.h"

class TestActionClientBaseFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rcl_ret_t ret;
    {
      rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
      ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
        EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
      });
      this->context = rcl_get_zero_initialized_context();
      ret = rcl_init(0, nullptr, &init_options, &this->context);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    this->node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_options = rcl_node_get_default_options();
    const char * node_name = "test_action_client_node";
    ret = rcl_node_init(&this->node, node_name, "", &this->context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown() override
  {
    rcl_ret_t ret = rcl_node_fini(&this->node);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(&this->context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  rcl_context_t context;
  rcl_node_t node;
};


TEST_F(TestActionClientBaseFixture, test_action_client_init_fini) {
  rcl_ret_t ret = RCL_RET_OK;
  rcl_action_client_t invalid_action_client =
    rcl_action_get_zero_initialized_client();
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  const char * action_name = "test_action_client_name";
  const rosidl_action_type_support_t * action_typesupport =
    ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
  const rcl_action_client_options_t action_client_options =
    rcl_action_client_get_default_options();
  rcl_action_client_options_t invalid_action_client_options =
    rcl_action_client_get_default_options();
  invalid_action_client_options.allocator =
    (rcl_allocator_t)rcutils_get_zero_initialized_allocator();
  rcl_action_client_t action_client = rcl_action_get_zero_initialized_client();

  ret = rcl_action_client_init(
    nullptr, &this->node, action_typesupport,
    action_name, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, nullptr, action_typesupport,
    action_name, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &invalid_node, action_typesupport,
    action_name, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &this->node, nullptr,
    action_name, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &this->node, action_typesupport,
    nullptr, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &this->node,
    action_typesupport, action_name,
    nullptr);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &this->node,
    action_typesupport, action_name,
    &invalid_action_client_options);
  EXPECT_EQ(ret, RCL_RET_INVALID_ARGUMENT) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &this->node, action_typesupport,
    action_name, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_init(
    &action_client, &this->node, action_typesupport,
    action_name, &action_client_options);
  EXPECT_EQ(ret, RCL_RET_ALREADY_INIT) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_fini(nullptr, &this->node);
  EXPECT_EQ(ret, RCL_RET_ACTION_CLIENT_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_fini(&invalid_action_client, &this->node);
  EXPECT_EQ(ret, RCL_RET_ACTION_CLIENT_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_fini(&action_client, nullptr);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_fini(&action_client, &invalid_node);
  EXPECT_EQ(ret, RCL_RET_NODE_INVALID) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_action_client_fini(&action_client, &this->node);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  rcl_reset_error();
}

class TestActionClientFixture : public TestActionClientBaseFixture
{
protected:
  void SetUp() override
  {
    TestActionClientBaseFixture::SetUp();
    this->action_client = rcl_action_get_zero_initialized_client();
    const rosidl_action_type_support_t * action_typesupport =
      ROSIDL_GET_ACTION_TYPE_SUPPORT(test_msgs, Fibonacci);
    this->action_client_options = rcl_action_client_get_default_options();
    rcl_ret_t ret = rcl_action_client_init(
      &this->action_client, &this->node, action_typesupport,
      this->action_name, &this->action_client_options);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    this->invalid_action_client = rcl_action_get_zero_initialized_client();
  }

  void TearDown() override
  {
    rcl_ret_t ret = rcl_action_client_fini(&this->action_client, &this->node);
    ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
    TestActionClientBaseFixture::TearDown();
  }

  const char * const action_name = "test_action_client_name";
  rcl_action_client_options_t action_client_options;
  rcl_action_client_t invalid_action_client;
  rcl_action_client_t action_client;
};

TEST_F(TestActionClientFixture, test_action_client_is_valid) {
  bool is_valid = rcl_action_client_is_valid(nullptr);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  is_valid = rcl_action_client_is_valid(&this->invalid_action_client);
  EXPECT_FALSE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();

  is_valid = rcl_action_client_is_valid(&this->action_client);
  EXPECT_TRUE(is_valid) << rcl_get_error_string().str;
  rcl_reset_error();
}

TEST_F(TestActionClientFixture, test_action_client_get_action_name) {
  const char * name = rcl_action_client_get_action_name(nullptr);
  EXPECT_EQ(name, nullptr) << rcl_get_error_string().str;
  rcl_reset_error();

  name = rcl_action_client_get_action_name(&this->invalid_action_client);
  EXPECT_EQ(name, nullptr) << rcl_get_error_string().str;
  rcl_reset_error();

  name = rcl_action_client_get_action_name(&this->action_client);
  ASSERT_NE(name, nullptr) << rcl_get_error_string().str;
  EXPECT_STREQ(name, this->action_name);
}

TEST_F(TestActionClientFixture, test_action_client_get_options) {
  const rcl_action_client_options_t * options =
    rcl_action_client_get_options(nullptr);
  EXPECT_EQ(options, nullptr) << rcl_get_error_string().str;
  rcl_reset_error();

  options = rcl_action_client_get_options(&this->invalid_action_client);
  EXPECT_EQ(options, nullptr) << rcl_get_error_string().str;
  rcl_reset_error();

  options = rcl_action_client_get_options(&this->action_client);
  ASSERT_NE(options, nullptr) << rcl_get_error_string().str;
}
