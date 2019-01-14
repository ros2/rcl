// Copyright 2019 Open Source Robotics Foundation, Inc.
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
#include "std_msgs/msg/u_int32__type_support.h"
#include "std_msgs/msg/u_int32.hpp"
#include "std_msgs/msg/u_int32_multi_array__type_support.h"
#include "std_msgs/msg/u_int32_multi_array.hpp"
#include "std_msgs/msg/string__type_support.h"
#include "std_msgs/msg/string.hpp"


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
  std::shared_ptr<void> msg1;
  std::shared_ptr<void> msg2;
  const rosidl_message_type_support_t * ts;
  std::string messageDescription;
};

std::ostream & operator<<(std::ostream & os, const TestMemoryParams & pr)
{
  return os << "TestMemoryParams : " <<
         "[ QoS : [ history : " << pr.qos_profile.history <<
         " - QoS.depth : " << pr.qos_profile.depth <<
         " - QoS.reliability : " << pr.qos_profile.reliability <<
         " - QoS.durability : " << pr.qos_profile.durability <<
         "] - [ MsgDescription : [ " << pr.messageDescription <<" ] \n";
}

std::shared_ptr<std_msgs::msg::UInt32> getMsgWUInt32Value(uint32_t val)
{
  std::shared_ptr<std_msgs::msg::UInt32> msg(
    new std_msgs::msg::UInt32, [](std_msgs::msg::UInt32 * ptr)
    {
      delete (ptr);
    }
  );

  msg->set__data(val);

  return msg;
}

std::shared_ptr<std_msgs::msg::UInt32MultiArray> getMsgWUInt32ArraySize(uint32_t val)
{
  std::vector<uint32_t> vec(val, val);
  std::shared_ptr<std_msgs::msg::UInt32MultiArray> msg(
    new std_msgs::msg::UInt32MultiArray(),
    [](std_msgs::msg::UInt32MultiArray * ptr)
    {
      delete (ptr);
    }
  );

  msg->set__data(vec);

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

  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();

  const char * topic_name = "chatter";
  const char * expected_topic_name = "/chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  TestMemoryParams param = GetParam();
  publisher_options.qos = param.qos_profile;
  rcl_ret_t ret =
    rcl_publisher_init(&publisher, this->node_ptr, param.ts, topic_name, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(&publisher), expected_topic_name), 0);

  EXPECT_NO_MEMORY_OPERATIONS({
    ret = rcl_publish(&publisher, param.msg1.get());
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

  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();

  const char * topic = "chatter";
  const char * expected_topic = "/chatter";
  TestMemoryParams param = GetParam();
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  publisher_options.qos = param.qos_profile;
  ret = rcl_publisher_init(&publisher, this->node_ptr, param.ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  subscription_options.qos = param.qos_profile;
  ret = rcl_subscription_init(&subscription, this->node_ptr, param.ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_subscription_get_topic_name(&subscription), expected_topic), 0);


  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  {
    ret = rcl_publish(&publisher, param.msg1.get());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, 10, 100, success);
  ASSERT_TRUE(success);
  {
    EXPECT_NO_MEMORY_OPERATIONS({
      ret = rcl_take(&subscription, param.msg2.get(), nullptr);
    });
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
}


std::vector<TestMemoryParams> getTestMemoryParams()
{

  const rosidl_message_type_support_t * ts_uint32 =
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt32);
  const rosidl_message_type_support_t * ts_uint32_multi_array =
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt32MultiArray);

  return {
     {rmw_qos_profile_sensor_data, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  // 0
     {rmw_qos_profile_sensor_data, getMsgWUInt32ArraySize(2), getMsgWUInt32ArraySize(2), ts_uint32_multi_array, "ts_uint32_multi_array(5)"} ,   // 1
     {rmw_qos_profile_sensor_data, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"},  // 2
     {rmw_qos_profile_parameters, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  // 3
     {rmw_qos_profile_parameters, getMsgWUInt32ArraySize(5), getMsgWUInt32ArraySize(5), ts_uint32_multi_array, "ts_uint32_multi_array(5)"},  // 4
     {rmw_qos_profile_parameters, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"},  // 5
     {rmw_qos_profile_default, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  // 6
     {rmw_qos_profile_default, getMsgWUInt32ArraySize(5), getMsgWUInt32ArraySize(5), ts_uint32_multi_array, "ts_uint32_multi_array(5)"},  // 7
     {rmw_qos_profile_default, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"},  // 8
     {rmw_qos_profile_services_default, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  // 9
     {rmw_qos_profile_services_default, getMsgWUInt32ArraySize(5), getMsgWUInt32ArraySize(5), ts_uint32_multi_array, "ts_uint32_multi_array(5)"},  // 10
     {rmw_qos_profile_services_default, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"},  // 11
     {rmw_qos_profile_parameter_events, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  // 12
     {rmw_qos_profile_parameter_events, getMsgWUInt32ArraySize(5), getMsgWUInt32ArraySize(5), ts_uint32_multi_array, "ts_uint32_multi_array(5)"},  // 13
     {rmw_qos_profile_parameter_events, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"},  // 14
     {rmw_qos_profile_system_default, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  // 15
     {rmw_qos_profile_system_default, getMsgWUInt32ArraySize(5), getMsgWUInt32ArraySize(5), ts_uint32_multi_array, "ts_uint32_multi_array(5)"},  // 16
     {rmw_qos_profile_system_default, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"},  // 17
     {{
       RMW_QOS_POLICY_HISTORY_KEEP_LAST,
       1000,
       RMW_QOS_POLICY_RELIABILITY_RELIABLE,
       RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
       false
     }, getMsgWUInt32Value(42), getMsgWUInt32Value(42), ts_uint32, "ts_uint32(32)"},  //  18
     {{
       RMW_QOS_POLICY_HISTORY_KEEP_LAST,
       1000,
       RMW_QOS_POLICY_RELIABILITY_RELIABLE,
       RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
       false
     }, getMsgWUInt32ArraySize(5), getMsgWUInt32ArraySize(5), ts_uint32_multi_array, "ts_uint32_multi_array(5)"},  //  19
     {{
       RMW_QOS_POLICY_HISTORY_KEEP_LAST,
       1000,
       RMW_QOS_POLICY_RELIABILITY_RELIABLE,
       RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
       false
     }, getMsgWUInt32ArraySize(1000), getMsgWUInt32ArraySize(1000), ts_uint32_multi_array, "ts_uint32_multi_array(1000)"}  //  20
  };
}

//using TestMemoryFixtureForUInt64 = TestMemoryFixture<UInt64>

INSTANTIATE_TEST_CASE_P_RMW(QOSGroup, TestMemoryFixture,
  ::testing::ValuesIn(getTestMemoryParams()));
