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

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "rcl/publisher.h"

#include "rcl/rcl.h"
#include "test_msgs/msg/primitives.h"
#include "rosidl_generator_c/string_functions.h"

#include "./failing_allocator_functions.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "osrf_testing_tools_cpp/memory_tools/gtest_quickstart.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

/*
* These macros are needed to make RMW test macros to work with GTEST macros
*/
#define TEST_FIXTURE_P_RMW(test_fixture_name) CLASSNAME(test_fixture_name, \
    RMW_IMPLEMENTATION)
#define APPLY(macro, ...) macro(__VA_ARGS__)
#define TEST_P_RMW(test_case_name, test_name) APPLY(TEST_P, \
    CLASSNAME(test_case_name, RMW_IMPLEMENTATION), test_name)
#define INSTANTIATE_TEST_CASE_P_RMW(instance_name, test_case_name, ...) APPLY( \
    INSTANTIATE_TEST_CASE_P, instance_name, CLASSNAME(test_case_name, \
    RMW_IMPLEMENTATION), __VA_ARGS__)

struct TestMemoryParams
{
  rmw_qos_profile_t qos_profile;
  std::shared_ptr<test_msgs__msg__Primitives> msg;
};

std::ostream & operator<<(std::ostream & os, const TestMemoryParams & pr)
{
  return os << "TestMemoryParams : "
              << "[ QoS : [ history : " << pr.qos_profile.history
              << " - QoS.depth : "  << pr.qos_profile.depth
              << " - QoS.reliability : "  << pr.qos_profile.reliability
              << " - QoS.durability : "  << pr.qos_profile.durability
              << "] - [ Msg : [ int64_value : " << pr.msg->int64_value
              << " - string.value.size : " << pr.msg->string_value.size
            << " ] ]" << '\n';
}

std::shared_ptr<test_msgs__msg__Primitives> getMessageWithStringLength(int length)
{
  std::shared_ptr<test_msgs__msg__Primitives> msg(
    new test_msgs__msg__Primitives, [](test_msgs__msg__Primitives * ptr)
    {
      test_msgs__msg__Primitives__fini(ptr);
      delete (ptr);
    }
  );

  test_msgs__msg__Primitives__init(msg.get());
  std::string test_str(length, 'x');
  rosidl_generator_c__String__assign(&msg->string_value, test_str.c_str());

  return msg;
}

std::shared_ptr<test_msgs__msg__Primitives> getMessageWithInt64Value(int64_t val)
{
  std::shared_ptr<test_msgs__msg__Primitives> msg(
    new test_msgs__msg__Primitives, [](test_msgs__msg__Primitives * ptr)
    {
      test_msgs__msg__Primitives__fini(ptr);
      delete (ptr);
    }
  );

  test_msgs__msg__Primitives__init(msg.get());
  msg->int64_value = val;

  return msg;
}

class TEST_FIXTURE_P_RMW (TestMemoryFixture)
  : public ::testing::TestWithParam<TestMemoryParams>
{
public:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;
  void SetUp() override
  {
    rcl_ret_t ret;
    {
      rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
      ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
        EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) <<
          rcl_get_error_string().str;
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

  void TearDown() override
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

/*
  Memory test of publisher
*/
TEST_P_RMW(TestMemoryFixture, test_memory_publisher) {
  osrf_testing_tools_cpp::memory_tools::ScopedQuickstartGtest scoped_quickstart_gtest(true);

  auto common = [](auto & service) {service.print_backtrace();};

  osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(common);
  osrf_testing_tools_cpp::memory_tools::on_unexpected_free(common);

  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  const char * topic_name = "chatter";
  const char * expected_topic_name = "/chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  TestMemoryParams param = GetParam();
  publisher_options.qos = param.qos_profile;
  rcl_ret_t ret =
    rcl_publisher_init(&publisher, this->node_ptr, ts, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(&publisher), expected_topic_name), 0);

  EXPECT_NO_MEMORY_OPERATIONS({
    ret = rcl_publish(&publisher, param.msg.get());
  });

  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}



void
wait_for_subscription_to_be_ready(
  rcl_subscription_t * subscription,
  size_t max_tries,
  int64_t period_ms,
  bool & success)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_wait_set_fini(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  size_t iteration = 0;
  do {
    ++iteration;
    ret = rcl_wait_set_clear(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait_set_add_subscription(&wait_set, subscription, NULL);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait(&wait_set, RCL_MS_TO_NS(period_ms));
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    for (size_t i = 0; i < wait_set.size_of_subscriptions; ++i) {
      if (wait_set.subscriptions[i] && wait_set.subscriptions[i] == subscription) {
        success = true;
        return;
      }
    }
  } while (iteration < max_tries);
  success = false;
}


/*

 Basic nominal test of a subscription.

 */
TEST_P_RMW(TestMemoryFixture, test_memory_subscription) {
  osrf_testing_tools_cpp::memory_tools::ScopedQuickstartGtest scoped_quickstart_gtest(true);
  auto common = [](auto & service) {service.print_backtrace();};
  osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(common);
  osrf_testing_tools_cpp::memory_tools::on_unexpected_free(common);

  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  const char * topic = "chatter";
  const char * expected_topic = "/chatter";
  TestMemoryParams param = GetParam();
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  publisher_options.qos = param.qos_profile;
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  subscription_options.qos = param.qos_profile;
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_subscription_get_topic_name(&subscription), expected_topic), 0);

  // Test is_valid for subscription with nullptr
  EXPECT_FALSE(rcl_subscription_is_valid(nullptr));
  rcl_reset_error();

  // Test is_valid for zero initialized subscription
  subscription = rcl_get_zero_initialized_subscription();
  EXPECT_FALSE(rcl_subscription_is_valid(&subscription));
  rcl_reset_error();

  // Check that valid subscriber is valid
  subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription));
  rcl_reset_error();

  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  {
    ret = rcl_publish(&publisher, param.msg.get());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, 10, 100, success);
  ASSERT_TRUE(success);
  {
    test_msgs__msg__Primitives msg;
    test_msgs__msg__Primitives__init(&msg);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      test_msgs__msg__Primitives__fini(&msg);
    });
    EXPECT_NO_MEMORY_OPERATIONS({
      ret = rcl_take(&subscription, &msg, nullptr);
    });
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
}


std::vector<TestMemoryParams> getTestMemoryParams()
{
  return {
           {rmw_qos_profile_sensor_data, getMessageWithInt64Value(42)},   // 0
           {rmw_qos_profile_sensor_data, getMessageWithStringLength(5)},   // 1
           {rmw_qos_profile_sensor_data, getMessageWithStringLength(100000)},  // 2
           {rmw_qos_profile_parameters, getMessageWithInt64Value(42)},  // 3
           {rmw_qos_profile_parameters, getMessageWithStringLength(5)},  // 4
           {rmw_qos_profile_parameters, getMessageWithStringLength(100000)},  // 5
           {rmw_qos_profile_default, getMessageWithInt64Value(42)},  // 6
           {rmw_qos_profile_default, getMessageWithStringLength(5)},  // 7
           {rmw_qos_profile_default, getMessageWithStringLength(100000)},  // 8
           {rmw_qos_profile_services_default, getMessageWithInt64Value(42)},  // 9
           {rmw_qos_profile_services_default, getMessageWithStringLength(5)},  // 10
           {rmw_qos_profile_services_default, getMessageWithStringLength(100000)},  // 11
           {rmw_qos_profile_parameter_events, getMessageWithInt64Value(42)},  // 12
           {rmw_qos_profile_parameter_events, getMessageWithStringLength(5)},  // 13
           {rmw_qos_profile_parameter_events, getMessageWithStringLength(100000)},  // 14
           {rmw_qos_profile_system_default, getMessageWithInt64Value(42)},  // 15
           {rmw_qos_profile_system_default, getMessageWithStringLength(5)},  // 16
           {rmw_qos_profile_system_default, getMessageWithStringLength(100000)},  // 17
           {{
             RMW_QOS_POLICY_HISTORY_KEEP_LAST,
             1000,
             RMW_QOS_POLICY_RELIABILITY_RELIABLE,
             RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
             false
            }, getMessageWithStringLength(5) }  //  18
  };
}

INSTANTIATE_TEST_CASE_P_RMW(QOSGroup, TestMemoryFixture,
  ::testing::ValuesIn(getTestMemoryParams()));
