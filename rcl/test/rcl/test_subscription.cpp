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

#include <chrono>
#include <string>
#include <thread>

#include "rcl/subscription.h"

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

class CLASSNAME (TestSubscriptionFixture, RMW_IMPLEMENTATION) : public ::testing::Test
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
    ret = rcl_node_init(this->node_ptr, name, &node_options);
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

void
wait_for_subscription_to_be_ready(
  rcl_subscription_t * subscription,
  size_t max_tries,
  int64_t period_ms,
  bool & success)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 1, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto wait_set_exit = make_scope_exit([&wait_set]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_wait_set_fini(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  size_t iteration = 0;
  do {
    ++iteration;
    ret = rcl_wait_set_clear_subscriptions(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_add_subscription(&wait_set, subscription);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait(&wait_set, RCL_MS_TO_NS(period_ms));
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    for (size_t i = 0; i < wait_set.size_of_subscriptions; ++i) {
      if (wait_set.subscriptions[i] && wait_set.subscriptions[i] == subscription) {
        success = true;
        return;
      }
    }
  } while (iteration < max_tries);
  success = false;
}

/* Basic nominal test of a subscription.
 */
TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_subscription_nominal) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts = ROSIDL_GET_TYPE_SUPPORT(std_msgs, msg, Int64);
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
  const char * topic = "chatter_int64";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto publisher_exit = make_scope_exit([&publisher, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto subscription_exit = make_scope_exit([&subscription, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  EXPECT_EQ(strcmp(rcl_subscription_get_topic_name(&subscription), topic), 0);
  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  {
    std_msgs__msg__Int64 msg;
    std_msgs__msg__Int64__init(&msg);
    msg.data = 42;
    ret = rcl_publish(&publisher, &msg);
    std_msgs__msg__Int64__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, 10, 100, success);
  ASSERT_TRUE(success);
  {
    std_msgs__msg__Int64 msg;
    std_msgs__msg__Int64__init(&msg);
    auto msg_exit = make_scope_exit([&msg]() {
      stop_memory_checking();
      std_msgs__msg__Int64__fini(&msg);
    });
    bool taken = false;
    ret = rcl_take(&subscription, &msg, &taken, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ASSERT_TRUE(taken) << "failed to take a message, even though the subscription was ready";
    ASSERT_EQ(42, msg.data);
  }
}

/* Basic nominal test of a publisher with a string.
 */
TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_subscription_nominal_string) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts = ROSIDL_GET_TYPE_SUPPORT(std_msgs, msg, String);
  const char * topic = "chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto publisher_exit = make_scope_exit([&publisher, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto subscription_exit = make_scope_exit([&subscription, this]() {
    stop_memory_checking();
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });
  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  const char * test_string = "testing";
  {
    std_msgs__msg__String msg;
    std_msgs__msg__String__init(&msg);
    ASSERT_TRUE(rosidl_generator_c__String__assign(&msg.data, test_string));
    ret = rcl_publish(&publisher, &msg);
    // TODO(wjwwood): re-enable this fini when ownership of the string is resolved.
    //                currently with Connext we will spuriously get a double free here.
    // std_msgs__msg__String__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, 10, 100, success);
  ASSERT_TRUE(success);
  {
    std_msgs__msg__String msg;
    std_msgs__msg__String__init(&msg);
    auto msg_exit = make_scope_exit([&msg]() {
      stop_memory_checking();
      std_msgs__msg__String__fini(&msg);
    });
    bool taken = false;
    ret = rcl_take(&subscription, &msg, &taken, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ASSERT_TRUE(taken) << "failed to take a message, even though the subscription was ready";
    ASSERT_EQ(std::string(test_string), std::string(msg.data.data, msg.data.size));
  }
}
