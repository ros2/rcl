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

#include "example_interfaces/srv/add_two_ints.h"
#include "rcl/rcl.h"
#include "rcl/remap.h"
#include "rcl/error_handling.h"
#include "std_msgs/msg/int64.h"

#include "../memory_tools/memory_tools.hpp"
#include "./arg_macros.hpp"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif


class CLASSNAME (TestRemapFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
    stop_memory_checking();
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

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), node_uses_remapped_name) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__node:=new_name");

  // Do remap node name using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/", &default_options));
    EXPECT_STREQ("new_name", rcl_node_get_name(&node));
    EXPECT_STREQ("new_name", rcl_node_get_logger_name(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap node name
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    options.use_global_arguments = false;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/", &options));
    EXPECT_STREQ("original_name", rcl_node_get_name(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "__node:=local_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/", &options));
    EXPECT_STREQ("local_name", rcl_node_get_name(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), node_uses_remapped_namespace) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__ns:=/new_ns");

  // Do remap namespace using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/old_ns", &default_options));
    EXPECT_STREQ("/new_ns", rcl_node_get_namespace(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/old_ns", &options));
    EXPECT_STREQ("/old_ns", rcl_node_get_namespace(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "__ns:=/local_ns");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/old_ns", &options));
    EXPECT_STREQ("/local_ns", rcl_node_get_namespace(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), publisher_uses_remapped_topic) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/foo/bar:=/bar/foo");

  // Do remap topic using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &default_options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
    rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    ret = rcl_publisher_init(&publisher, &node, ts, "/foo/bar", &publisher_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/bar/foo", rcl_publisher_get_topic_name(&publisher));

    EXPECT_EQ(RCL_RET_OK, rcl_publisher_fini(&publisher, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
    rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    ret = rcl_publisher_init(&publisher, &node, ts, "/foo/bar", &publisher_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/bar", rcl_publisher_get_topic_name(&publisher));

    EXPECT_EQ(RCL_RET_OK, rcl_publisher_fini(&publisher, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=/local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/", &options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
    rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    ret = rcl_publisher_init(&publisher, &node, ts, "/foo/bar", &publisher_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/local/remap", rcl_publisher_get_topic_name(&publisher));

    EXPECT_EQ(RCL_RET_OK, rcl_publisher_fini(&publisher, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap a relative topic name that matches
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/foo", &options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
    rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    ret = rcl_publisher_init(&publisher, &node, ts, "bar", &publisher_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/local/remap", rcl_publisher_get_topic_name(&publisher));

    EXPECT_EQ(RCL_RET_OK, rcl_publisher_fini(&publisher, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), subscription_uses_remapped_topic) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/foo/bar:=/bar/foo");

  // Do remap topic using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &default_options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
    rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
    ret = rcl_subscription_init(&subscription, &node, ts, "/foo/bar", &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/bar/foo", rcl_subscription_get_topic_name(&subscription));

    EXPECT_EQ(RCL_RET_OK, rcl_subscription_fini(&subscription, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
    rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
    ret = rcl_subscription_init(&subscription, &node, ts, "/foo/bar", &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/bar", rcl_subscription_get_topic_name(&subscription));

    EXPECT_EQ(RCL_RET_OK, rcl_subscription_fini(&subscription, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=/local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/", &options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
    rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
    ret = rcl_subscription_init(&subscription, &node, ts, "/foo/bar", &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/local/remap", rcl_subscription_get_topic_name(&subscription));

    EXPECT_EQ(RCL_RET_OK, rcl_subscription_fini(&subscription, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap a relative topic name that matches
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/foo", &options));

    const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
    rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
    rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
    ret = rcl_subscription_init(&subscription, &node, ts, "bar", &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/local/remap", rcl_subscription_get_topic_name(&subscription));

    EXPECT_EQ(RCL_RET_OK, rcl_subscription_fini(&subscription, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), client_uses_remapped_service) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/foo/bar:=/bar/foo");

  // Do remap service name using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &default_options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_client_options_t client_options = rcl_client_get_default_options();
    rcl_client_t client = rcl_get_zero_initialized_client();
    ret = rcl_client_init(&client, &node, ts, "/foo/bar", &client_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/bar/foo", rcl_client_get_service_name(&client));

    EXPECT_EQ(RCL_RET_OK, rcl_client_fini(&client, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_client_options_t client_options = rcl_client_get_default_options();
    rcl_client_t client = rcl_get_zero_initialized_client();
    ret = rcl_client_init(&client, &node, ts, "/foo/bar", &client_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/bar", rcl_client_get_service_name(&client));

    EXPECT_EQ(RCL_RET_OK, rcl_client_fini(&client, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=/local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/", &options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_client_options_t client_options = rcl_client_get_default_options();
    rcl_client_t client = rcl_get_zero_initialized_client();
    ret = rcl_client_init(&client, &node, ts, "/foo/bar", &client_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/local/remap", rcl_client_get_service_name(&client));

    EXPECT_EQ(RCL_RET_OK, rcl_client_fini(&client, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap a relative service name that matches
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/foo", &options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_client_options_t client_options = rcl_client_get_default_options();
    rcl_client_t client = rcl_get_zero_initialized_client();
    ret = rcl_client_init(&client, &node, ts, "bar", &client_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/local/remap", rcl_client_get_service_name(&client));

    EXPECT_EQ(RCL_RET_OK, rcl_client_fini(&client, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), service_uses_remapped_service) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/foo/bar:=/bar/foo");

  // Do remap service name using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &default_options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_service_t service = rcl_get_zero_initialized_service();
    ret = rcl_service_init(&service, &node, ts, "/foo/bar", &service_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/bar/foo", rcl_service_get_service_name(&service));

    EXPECT_EQ(RCL_RET_OK, rcl_service_fini(&service, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/old_ns", &options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_service_t service = rcl_get_zero_initialized_service();
    ret = rcl_service_init(&service, &node, ts, "/foo/bar", &service_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/bar", rcl_service_get_service_name(&service));

    EXPECT_EQ(RCL_RET_OK, rcl_service_fini(&service, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=/local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/", &options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_service_t service = rcl_get_zero_initialized_service();
    ret = rcl_service_init(&service, &node, ts, "/foo/bar", &service_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/local/remap", rcl_service_get_service_name(&service));

    EXPECT_EQ(RCL_RET_OK, rcl_service_fini(&service, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap a relative service name that matches
  {
    rcl_arguments_t local_arguments;
    SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/foo/bar:=local/remap");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "node_name", "/foo", &options));

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_service_t service = rcl_get_zero_initialized_service();
    ret = rcl_service_init(&service, &node, ts, "bar", &service_options);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_STREQ("/foo/local/remap", rcl_service_get_service_name(&service));

    EXPECT_EQ(RCL_RET_OK, rcl_service_fini(&service, &node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
}
