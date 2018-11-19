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

#include "rcl/publisher.h"

#include "rcl/rcl.h"
#include "test_msgs/msg/primitives.h"
#include "rosidl_generator_c/string_functions.h"

#include "./failing_allocator_functions.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestPublisherFixture, RMW_IMPLEMENTATION) : public ::testing::Test
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
      OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
        EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
      });
      this->context_ptr = new rcl_context_t;
      *this->context_ptr = rcl_get_zero_initialized_context();
      ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_publisher_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(this->context_ptr);
    delete this->context_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};

/* Basic nominal test of a publisher.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publisher_nominal) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  const char * topic_name = "chatter";
  const char * expected_topic_name = "/chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(&publisher), expected_topic_name), 0);
  test_msgs__msg__Primitives msg;
  test_msgs__msg__Primitives__init(&msg);
  msg.int64_value = 42;
  ret = rcl_publish(&publisher, &msg);
  test_msgs__msg__Primitives__fini(&msg);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/* Basic nominal test of a publisher with a string.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publisher_nominal_string) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  const char * topic_name = "chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  test_msgs__msg__Primitives msg;
  test_msgs__msg__Primitives__init(&msg);
  ASSERT_TRUE(rosidl_generator_c__String__assign(&msg.string_value, "testing"));
  ret = rcl_publish(&publisher, &msg);
  test_msgs__msg__Primitives__fini(&msg);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/* Test two publishers using different message types with the same basename.
 *
 * Regression test for https://github.com/ros2/rmw_connext/issues/234, where rmw_connext_cpp could
 * not support publishers on topics with the same basename (but different namespaces) using
 * different message types, because at the time partitions were used for implementing namespaces.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publishers_different_types) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts_int =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  const char * topic_name = "basename";
  const char * expected_topic_name = "/basename";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts_int, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(&publisher), expected_topic_name), 0);

  rcl_publisher_t publisher_in_namespace = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts_string = ROSIDL_GET_MSG_TYPE_SUPPORT(
    test_msgs, msg, Primitives);
  topic_name = "namespace/basename";
  expected_topic_name = "/namespace/basename";
  ret = rcl_publisher_init(
    &publisher_in_namespace, this->node_ptr, ts_string, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher_in_namespace, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(&publisher_in_namespace), expected_topic_name), 0);

  test_msgs__msg__Primitives msg_int;
  test_msgs__msg__Primitives__init(&msg_int);
  msg_int.int64_value = 42;
  ret = rcl_publish(&publisher, &msg_int);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  test_msgs__msg__Primitives__fini(&msg_int);

  test_msgs__msg__Primitives msg_string;
  test_msgs__msg__Primitives__init(&msg_string);
  ASSERT_TRUE(rosidl_generator_c__String__assign(&msg_string.string_value, "testing"));
  ret = rcl_publish(&publisher_in_namespace, &msg_string);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

/* Testing the publisher init and fini functions.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publisher_init_fini) {
  rcl_ret_t ret;
  // Setup valid inputs.
  rcl_publisher_t publisher;
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  const char * topic_name = "chatter";
  rcl_publisher_options_t default_publisher_options = rcl_publisher_get_default_options();

  // Check if null publisher is valid
  EXPECT_FALSE(rcl_publisher_is_valid(nullptr));
  rcl_reset_error();

  // Check if zero initialized node is valid
  publisher = rcl_get_zero_initialized_publisher();
  EXPECT_FALSE(rcl_publisher_is_valid(&publisher));
  rcl_reset_error();

  // Check that valid publisher is valid
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_TRUE(rcl_publisher_is_valid(&publisher));
  rcl_reset_error();

  // Try passing null for publisher in init.
  ret = rcl_publisher_init(nullptr, this->node_ptr, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing null for a node pointer in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, nullptr, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing an invalid (uninitialized) node in init.
  publisher = rcl_get_zero_initialized_publisher();
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  ret = rcl_publisher_init(&publisher, &invalid_node, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing null for the type support in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, nullptr, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing null for the topic name in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, nullptr, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing null for the options in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing options with an invalid allocate in allocator with init.
  publisher = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t publisher_options_with_invalid_allocator;
  publisher_options_with_invalid_allocator = rcl_publisher_get_default_options();
  publisher_options_with_invalid_allocator.allocator.allocate = nullptr;
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, ts, topic_name, &publisher_options_with_invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // Try passing options with an invalid deallocate in allocator with init.
  publisher = rcl_get_zero_initialized_publisher();
  publisher_options_with_invalid_allocator = rcl_publisher_get_default_options();
  publisher_options_with_invalid_allocator.allocator.deallocate = nullptr;
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, ts, topic_name, &publisher_options_with_invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // An allocator with an invalid realloc will probably work (so we will not test it).

  // Try passing options with a failing allocator with init.
  publisher = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t publisher_options_with_failing_allocator;
  publisher_options_with_failing_allocator = rcl_publisher_get_default_options();
  publisher_options_with_failing_allocator.allocator.allocate = failing_malloc;
  publisher_options_with_failing_allocator.allocator.reallocate = failing_realloc;
  publisher_options_with_failing_allocator.allocator.zero_allocate = failing_calloc;
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, ts, topic_name, &publisher_options_with_failing_allocator);
  EXPECT_EQ(RCL_RET_BAD_ALLOC, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}
