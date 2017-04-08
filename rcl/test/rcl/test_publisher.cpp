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
#include "std_msgs/msg/int64.h"
#include "std_msgs/msg/string.h"
#include "rosidl_generator_c/string_functions.h"

#include "../memory_tools/memory_tools.hpp"
#include "../scope_exit.hpp"
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
  rcl_node_t * node_ptr;
  void SetUp()
  {
    stop_memory_checking();
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "node_name";
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

/* Basic nominal test of a publisher.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publisher_nominal) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
  // TODO(wjwwood): Change this back to just chatter when this OpenSplice problem is resolved:
  //  ========================================================================================
  //  Report      : WARNING
  //  Date        : Wed Feb 10 18:17:03 PST 2016
  //  Description : Create Topic "chatter" failed: typename <std_msgs::msg::dds_::Int64_>
  //                differs exiting definition <std_msgs::msg::dds_::String_>.
  //  Node        : farl
  //  Process     : test_subscription__rmw_opensplice_cpp <23524>
  //  Thread      : main thread 7fff7342d000
  //  Internals   : V6.4.140407OSS///v_topicNew/v_topic.c/448/21/1455157023.781423000
  const char * topic_name = "chatter_int64";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto publisher_exit = make_scope_exit([&publisher, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(&publisher), topic_name), 0);
  std_msgs__msg__Int64 msg;
  std_msgs__msg__Int64__init(&msg);
  msg.data = 42;
  ret = rcl_publish(&publisher, &msg);
  std_msgs__msg__Int64__fini(&msg);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

/* Basic nominal test of a publisher with a string.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publisher_nominal_string) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  const char * topic_name = "chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto publisher_exit = make_scope_exit([&publisher, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  std_msgs__msg__String msg;
  std_msgs__msg__String__init(&msg);
  ASSERT_TRUE(rosidl_generator_c__String__assign(&msg.data, "testing"));
  ret = rcl_publish(&publisher, &msg);
  // TODO(wjwwood): re-enable this fini when ownership of the string is resolved.
  //                currently with Connext we will spuriously get a double free here.
  // std_msgs__msg__String__fini(&msg);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

/* Testing the publisher init and fini functions.
 */
TEST_F(CLASSNAME(TestPublisherFixture, RMW_IMPLEMENTATION), test_publisher_init_fini) {
  stop_memory_checking();
  rcl_ret_t ret;
  // Setup valid inputs.
  rcl_publisher_t publisher;
  const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64);
  const char * topic_name = "chatter";
  rcl_publisher_options_t default_publisher_options = rcl_publisher_get_default_options();

  // Try passing null for publisher in init.
  ret = rcl_publisher_init(nullptr, this->node_ptr, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing null for a node pointer in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, nullptr, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing an invalid (uninitialized) node in init.
  publisher = rcl_get_zero_initialized_publisher();
  rcl_node_t invalid_node = rcl_get_zero_initialized_node();
  ret = rcl_publisher_init(&publisher, &invalid_node, ts, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing null for the type support in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, nullptr, topic_name, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing null for the topic name in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, nullptr, &default_publisher_options);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing null for the options in init.
  publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing options with an invalid allocate in allocator with init.
  publisher = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t publisher_options_with_invalid_allocator;
  publisher_options_with_invalid_allocator = rcl_publisher_get_default_options();
  publisher_options_with_invalid_allocator.allocator.allocate = nullptr;
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, ts, topic_name, &publisher_options_with_invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // Try passing options with an invalid deallocate in allocator with init.
  publisher = rcl_get_zero_initialized_publisher();
  publisher_options_with_invalid_allocator = rcl_publisher_get_default_options();
  publisher_options_with_invalid_allocator.allocator.deallocate = nullptr;
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, ts, topic_name, &publisher_options_with_invalid_allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();

  // An allocator with an invalid realloc will probably work (so we will not test it).

  // Try passing options with a failing allocator with init.
  publisher = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t publisher_options_with_failing_allocator;
  publisher_options_with_failing_allocator = rcl_publisher_get_default_options();
  publisher_options_with_failing_allocator.allocator.allocate = failing_malloc;
  publisher_options_with_failing_allocator.allocator.deallocate = failing_free;
  publisher_options_with_failing_allocator.allocator.reallocate = failing_realloc;
  ret = rcl_publisher_init(
    &publisher, this->node_ptr, ts, topic_name, &publisher_options_with_failing_allocator);
  EXPECT_EQ(RCL_RET_BAD_ALLOC, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}
