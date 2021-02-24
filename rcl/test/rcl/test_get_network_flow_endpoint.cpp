// Copyright 2020 Ericsson AB
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

#include "mimick/mimick.h"
#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"
#include "rcl/network_flow.h"
#include "rcl/publisher.h"
#include "rcl/rcl.h"
#include "rcl/subscription.h"
#include "test_msgs/msg/basic_types.h"

#include "./allocator_testing_utils.h"
#include "../mocking_utils/patch.hpp"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestNetworkFlowNode, RMW_IMPLEMENTATION) : public ::testing::Test
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
        EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
      });
      this->context_ptr = new rcl_context_t;
      *this->context_ptr = rcl_get_zero_initialized_context();
      ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    constexpr char name[] = "test_network_flow_node";
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
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(this->context_ptr);
    delete this->context_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};

class CLASSNAME (TestNetworkFlowPublisher, RMW_IMPLEMENTATION)
  : public CLASSNAME(TestNetworkFlowNode, RMW_IMPLEMENTATION)
{
public:
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  const char * topic = "chatter";

  rcl_publisher_t publisher;
  rcl_publisher_t publisher_unique_network_flow;

  rcl_publisher_options_t publisher_options;
  rcl_publisher_options_t publisher_options_unique_network_flow;

  void SetUp() override
  {
    CLASSNAME(TestNetworkFlowNode, RMW_IMPLEMENTATION) ::SetUp();

    publisher = rcl_get_zero_initialized_publisher();
    publisher_options = rcl_publisher_get_default_options();
    rcl_ret_t ret = rcl_publisher_init(
      &publisher, this->node_ptr, ts, topic, &publisher_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    publisher_unique_network_flow = rcl_get_zero_initialized_publisher();
    publisher_options_unique_network_flow = rcl_publisher_get_default_options();
    publisher_options_unique_network_flow.rmw_publisher_options.require_unique_network_flow =
      RMW_UNIQUE_NETWORK_FLOW_STRICTLY_REQUIRED;
    ret = rcl_publisher_init(
      &publisher_unique_network_flow, this->node_ptr, ts, topic,
      &publisher_options_unique_network_flow);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown() override
  {
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_publisher_fini(&publisher_unique_network_flow, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    CLASSNAME(TestNetworkFlowNode, RMW_IMPLEMENTATION) ::TearDown();
  }
};

class CLASSNAME (TestNetworkFlowSubscription, RMW_IMPLEMENTATION)
  : public CLASSNAME(TestNetworkFlowNode, RMW_IMPLEMENTATION)
{
public:
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  const char * topic = "chatter";

  rcl_subscription_t subscription;
  rcl_subscription_t subscription_unique_network_flow;

  rcl_subscription_options_t subscription_options;
  rcl_subscription_options_t subscription_options_unique_network_flow;

  void SetUp() override
  {
    CLASSNAME(TestNetworkFlowNode, RMW_IMPLEMENTATION) ::SetUp();

    subscription = rcl_get_zero_initialized_subscription();
    subscription_options = rcl_subscription_get_default_options();
    rcl_ret_t ret = rcl_subscription_init(
      &subscription, this->node_ptr, ts, topic, &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    subscription_unique_network_flow = rcl_get_zero_initialized_subscription();
    subscription_options_unique_network_flow = rcl_subscription_get_default_options();
    subscription_options_unique_network_flow.rmw_subscription_options.require_unique_network_flow =
      RMW_UNIQUE_NETWORK_FLOW_STRICTLY_REQUIRED;
    ret = rcl_subscription_init(
      &subscription_unique_network_flow, this->node_ptr, ts, topic,
      &subscription_options_unique_network_flow);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown() override
  {
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_subscription_fini(&subscription_unique_network_flow, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    CLASSNAME(TestNetworkFlowNode, RMW_IMPLEMENTATION) ::TearDown();
  }
};

TEST_F(
  CLASSNAME(
    TestNetworkFlowPublisher,
    RMW_IMPLEMENTATION), test_publisher_get_network_flow_errors) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t failing_allocator = get_failing_allocator();
  rcl_network_flow_array_t network_flow_array = rcl_get_zero_initialized_network_flow_array();

  // Invalid publisher
  ret = rcl_publisher_get_network_flow(
    nullptr, &allocator, &network_flow_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();

  // Invalid allocator
  ret = rcl_publisher_get_network_flow(
    &this->publisher, nullptr, &network_flow_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();

  // Invalid network_flow_array
  ret = rcl_publisher_get_network_flow(
    &this->publisher, &allocator, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();

  // Failing allocator
  set_failing_allocator_is_failing(failing_allocator, true);
  ret = rcl_publisher_get_network_flow(
    &this->publisher, &failing_allocator, &network_flow_array);
  EXPECT_EQ(RCL_RET_BAD_ALLOC, ret);
  rcl_reset_error();

  // Non-zero network_flow_array
  network_flow_array.size = 1;
  ret = rcl_publisher_get_network_flow(
    &this->publisher, &allocator, &network_flow_array);
  EXPECT_EQ(RCL_RET_ERROR, ret);
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestNetworkFlowPublisher, RMW_IMPLEMENTATION), test_publisher_get_network_flow) {
  rcl_ret_t ret_1;
  rcl_ret_t ret_2;
  rcl_ret_t ret_3 = false;
  rcl_allocator_t allocator = rcl_get_default_allocator();

  // Get network flow of ordinary publisher
  rcl_network_flow_array_t network_flow_array = rcl_get_zero_initialized_network_flow_array();
  ret_1 = rcl_publisher_get_network_flow(
    &this->publisher, &allocator, &network_flow_array);
  if (ret_1 == RCL_RET_OK || ret_1 == RCL_RET_UNSUPPORTED) {
    ret_3 = true;
  }
  EXPECT_EQ(true, ret_3);
  ret_3 = false;

  // Get network flow of publisher with unique network flow
  rcl_network_flow_array_t network_flow_array_unique =
    rcl_get_zero_initialized_network_flow_array();
  ret_2 = rcl_publisher_get_network_flow(
    &this->publisher_unique_network_flow, &allocator, &network_flow_array_unique);
  if (ret_2 == RCL_RET_OK || ret_2 == RCL_RET_UNSUPPORTED) {
    ret_3 = true;
  }
  EXPECT_EQ(true, ret_3);
  ret_3 = false;

  if (ret_1 == RCL_RET_OK && ret_2 == RCL_RET_OK) {
    // Expect network flows not to be same
    for (size_t i = 0; i < network_flow_array.size; i++) {
      for (size_t j = 0; j < network_flow_array_unique.size; j++) {
        bool strcmp_ret = false;
        if (strcmp(
            network_flow_array.network_flow[i].internet_address,
            network_flow_array_unique.network_flow[j].internet_address) == 0)
        {
          strcmp_ret = true;
        }
        EXPECT_FALSE(
          network_flow_array.network_flow[i].transport_protocol ==
          network_flow_array_unique.network_flow[j].transport_protocol &&
          network_flow_array.network_flow[i].internet_protocol ==
          network_flow_array_unique.network_flow[j].internet_protocol &&
          network_flow_array.network_flow[i].transport_port ==
          network_flow_array_unique.network_flow[j].transport_port &&
          network_flow_array.network_flow[i].flow_label ==
          network_flow_array_unique.network_flow[j].flow_label &&
          strcmp_ret);
      }
    }
  }

  // Release resources
  rcl_network_flow_array_fini(&network_flow_array, &allocator);
  rcl_network_flow_array_fini(&network_flow_array_unique, &allocator);
}

TEST_F(
  CLASSNAME(
    TestNetworkFlowSubscription,
    RMW_IMPLEMENTATION), test_subscription_get_network_flow_errors) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_allocator_t failing_allocator = get_failing_allocator();
  rcl_network_flow_array_t network_flow_array = rcl_get_zero_initialized_network_flow_array();

  // Invalid subscription
  ret = rcl_subscription_get_network_flow(
    nullptr, &allocator, &network_flow_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();

  // Invalid allocator
  ret = rcl_subscription_get_network_flow(
    &this->subscription, nullptr, &network_flow_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();

  // Invalid network_flow_array
  ret = rcl_subscription_get_network_flow(
    &this->subscription, &allocator, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();

  // Failing allocator
  set_failing_allocator_is_failing(failing_allocator, true);
  ret = rcl_subscription_get_network_flow(
    &this->subscription, &failing_allocator, &network_flow_array);
  EXPECT_EQ(RCL_RET_BAD_ALLOC, ret);
  rcl_reset_error();

  // Non-zero network_flow_array
  network_flow_array.size = 1;
  ret = rcl_subscription_get_network_flow(
    &this->subscription, &allocator, &network_flow_array);
  EXPECT_EQ(RCL_RET_ERROR, ret);
  rcl_reset_error();
}

TEST_F(
  CLASSNAME(
    TestNetworkFlowSubscription,
    RMW_IMPLEMENTATION), test_subscription_get_network_flow) {
  rcl_ret_t ret_1;
  rcl_ret_t ret_2;
  rcl_ret_t ret_3 = false;
  rcl_allocator_t allocator = rcl_get_default_allocator();

  // Get network flow of ordinary subscription
  rcl_network_flow_array_t network_flow_array = rcl_get_zero_initialized_network_flow_array();
  ret_1 = rcl_subscription_get_network_flow(
    &this->subscription, &allocator, &network_flow_array);
  if (ret_1 == RCL_RET_OK || ret_1 == RCL_RET_UNSUPPORTED) {
    ret_3 = true;
  }
  EXPECT_EQ(true, ret_3);
  ret_3 = false;

  // Get network flow of subscription with unique network flow
  rcl_network_flow_array_t network_flow_array_unique =
    rcl_get_zero_initialized_network_flow_array();
  ret_2 = rcl_subscription_get_network_flow(
    &this->subscription_unique_network_flow, &allocator, &network_flow_array_unique);
  if (ret_2 == RCL_RET_OK || ret_2 == RCL_RET_UNSUPPORTED) {
    ret_3 = true;
  }
  EXPECT_EQ(true, ret_3);
  ret_3 = false;

  if (ret_1 == RCL_RET_OK && ret_2 == RCL_RET_OK) {
    // Expect network flows not to be same
    for (size_t i = 0; i < network_flow_array.size; i++) {
      for (size_t j = 0; j < network_flow_array_unique.size; j++) {
        bool strcmp_ret = false;
        if (strcmp(
            network_flow_array.network_flow[i].internet_address,
            network_flow_array_unique.network_flow[j].internet_address) == 0)
        {
          strcmp_ret = true;
        }
        EXPECT_FALSE(
          network_flow_array.network_flow[i].transport_protocol ==
          network_flow_array_unique.network_flow[j].transport_protocol &&
          network_flow_array.network_flow[i].internet_protocol ==
          network_flow_array_unique.network_flow[j].internet_protocol &&
          network_flow_array.network_flow[i].transport_port ==
          network_flow_array_unique.network_flow[j].transport_port &&
          network_flow_array.network_flow[i].flow_label ==
          network_flow_array_unique.network_flow[j].flow_label &&
          strcmp_ret);
      }
    }
  }

  // Release resources
  rcl_network_flow_array_fini(&network_flow_array, &allocator);
  rcl_network_flow_array_fini(&network_flow_array_unique, &allocator);
}
