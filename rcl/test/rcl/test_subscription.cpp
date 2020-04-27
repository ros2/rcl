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
#include "test_msgs/msg/basic_types.h"
#include "test_msgs/msg/strings.h"
#include "rosidl_runtime_c/string_functions.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
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
    const char * name = "test_subscription_node";
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

void
wait_for_subscription_to_be_ready(
  rcl_subscription_t * subscription,
  rcl_context_t * context,
  size_t max_tries,
  int64_t period_ms,
  bool & success)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret =
    rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, 0, context, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
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

/* Test subscription init, fini and is_valid functions
 */
TEST_F(
  CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION),
  test_subscription_init_fini_and_is_valid)
{
  rcl_ret_t ret;

  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  const char * topic = "chatter";
  const char * expected_topic = "/chatter";

  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(strcmp(rcl_subscription_get_topic_name(&subscription), expected_topic), 0);
  ret = rcl_subscription_fini(&subscription, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // Test is_valid for subscription with nullptr
  EXPECT_FALSE(rcl_subscription_is_valid(nullptr));
  rcl_reset_error();

  // Test is_valid for zero initialized subscription
  subscription = rcl_get_zero_initialized_subscription();
  EXPECT_FALSE(rcl_subscription_is_valid(&subscription));
  rcl_reset_error();
}

/* Basic nominal test of a subscription
 */
TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_subscription_nominal) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  const char * topic = "/chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_reset_error();

  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#ifdef RMW_TIMESTAMPS_SUPPORTED
  rcl_time_point_value_t pre_publish_time;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_system_time_now(&pre_publish_time)) << " could not get system time failed";
#endif
  {
    test_msgs__msg__BasicTypes msg;
    test_msgs__msg__BasicTypes__init(&msg);
    msg.int64_value = 42;
    ret = rcl_publish(&publisher, &msg, nullptr);
    test_msgs__msg__BasicTypes__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
  {
    test_msgs__msg__BasicTypes msg;
    test_msgs__msg__BasicTypes__init(&msg);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      test_msgs__msg__BasicTypes__fini(&msg);
    });
    rmw_message_info_t message_info = rmw_get_zero_initialized_message_info();
    ret = rcl_take(&subscription, &msg, &message_info, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ASSERT_EQ(42, msg.int64_value);
  #ifdef RMW_TIMESTAMPS_SUPPORTED
    EXPECT_NE(0u, message_info.source_timestamp);
    EXPECT_TRUE(pre_publish_time <= message_info.source_timestamp) <<
      pre_publish_time << " > " << message_info.source_timestamp;
  #ifdef RMW_RECEIVED_TIMESTAMP_SUPPORTED
    EXPECT_NE(0u, message_info.received_timestamp);
    EXPECT_TRUE(pre_publish_time <= message_info.received_timestamp);
    EXPECT_TRUE(message_info.source_timestamp <= message_info.received_timestamp);
  #else
    EXPECT_EQ(0u, message_info.received_timestamp);
  #endif
  #else
    EXPECT_EQ(0u, message_info.source_timestamp);
    EXPECT_EQ(0u, message_info.received_timestamp);
  #endif
  }
}

/* Basic nominal test of a publisher with a string.
 */
TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_subscription_nominal_string) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Strings);
  const char * topic = "rcl_test_subscription_nominal_string_chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  const char * test_string = "testing";
  {
    test_msgs__msg__Strings msg;
    test_msgs__msg__Strings__init(&msg);
    ASSERT_TRUE(rosidl_runtime_c__String__assign(&msg.string_value, test_string));
    ret = rcl_publish(&publisher, &msg, nullptr);
    test_msgs__msg__Strings__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
  {
    test_msgs__msg__Strings msg;
    test_msgs__msg__Strings__init(&msg);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      test_msgs__msg__Strings__fini(&msg);
    });
    ret = rcl_take(&subscription, &msg, nullptr, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ASSERT_EQ(std::string(test_string), std::string(msg.string_value.data, msg.string_value.size));
  }
}

/* Basic nominal test of a subscription taking a sequence.
 */
TEST_F(
  CLASSNAME(
    TestSubscriptionFixture,
    RMW_IMPLEMENTATION), test_subscription_nominal_string_sequence) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Strings);
  const char * topic = "rcl_test_subscription_nominal_string_sequence_chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  const char * test_string = "testing";
  {
    test_msgs__msg__Strings msg;
    test_msgs__msg__Strings__init(&msg);
    ASSERT_TRUE(rosidl_runtime_c__String__assign(&msg.string_value, test_string));
    ret = rcl_publish(&publisher, &msg, nullptr);
    ret = rcl_publish(&publisher, &msg, nullptr);
    ret = rcl_publish(&publisher, &msg, nullptr);
    test_msgs__msg__Strings__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
  auto allocator = rcutils_get_default_allocator();
  {
    size_t size = 1;
    rmw_message_info_sequence_t message_infos;
    rmw_message_info_sequence_init(&message_infos, size, &allocator);

    rmw_message_sequence_t messages;
    rmw_message_sequence_init(&messages, size, &allocator);

    auto seq = test_msgs__msg__Strings__Sequence__create(size);

    for (size_t ii = 0; ii < size; ++ii) {
      messages.data[ii] = &seq->data[ii];
    }

    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      rmw_message_info_sequence_fini(&message_infos);
      rmw_message_sequence_fini(&messages);
      test_msgs__msg__Strings__Sequence__destroy(seq);
    });

    // Attempt to take more than capacity allows.
    ret = rcl_take_sequence(&subscription, 5, &messages, &message_infos, nullptr);
    ASSERT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;

    ASSERT_EQ(0u, messages.size);
    ASSERT_EQ(0u, message_infos.size);
  }

  {
    size_t size = 5;
    rmw_message_info_sequence_t message_infos;
    rmw_message_info_sequence_init(&message_infos, size, &allocator);

    rmw_message_sequence_t messages;
    rmw_message_sequence_init(&messages, size, &allocator);

    auto seq = test_msgs__msg__Strings__Sequence__create(size);

    for (size_t ii = 0; ii < size; ++ii) {
      messages.data[ii] = &seq->data[ii];
    }

    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      rmw_message_info_sequence_fini(&message_infos);
      rmw_message_sequence_fini(&messages);
      test_msgs__msg__Strings__Sequence__destroy(seq);
    });

    ret = rcl_take_sequence(&subscription, 5, &messages, &message_infos, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ASSERT_EQ(3u, messages.size);
    ASSERT_EQ(3u, message_infos.size);
  }

  {
    test_msgs__msg__Strings msg;
    test_msgs__msg__Strings__init(&msg);
    ASSERT_TRUE(rosidl_runtime_c__String__assign(&msg.string_value, test_string));
    ret = rcl_publish(&publisher, &msg, nullptr);
    ret = rcl_publish(&publisher, &msg, nullptr);
    ret = rcl_publish(&publisher, &msg, nullptr);
    ret = rcl_publish(&publisher, &msg, nullptr);
    ret = rcl_publish(&publisher, &msg, nullptr);
    test_msgs__msg__Strings__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  // Give a brief moment for publications to go through.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // Take fewer messages than are available in the subscription
  {
    size_t size = 3;
    rmw_message_info_sequence_t message_infos;
    rmw_message_info_sequence_init(&message_infos, size, &allocator);

    rmw_message_sequence_t messages;
    rmw_message_sequence_init(&messages, size, &allocator);

    auto seq = test_msgs__msg__Strings__Sequence__create(size);

    for (size_t ii = 0; ii < size; ++ii) {
      messages.data[ii] = &seq->data[ii];
    }

    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      rmw_message_info_sequence_fini(&message_infos);
      rmw_message_sequence_fini(&messages);
      test_msgs__msg__Strings__Sequence__destroy(seq);
    });

    ret = rcl_take_sequence(&subscription, 3, &messages, &message_infos, nullptr);

    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ASSERT_EQ(3u, messages.size);
    ASSERT_EQ(3u, message_infos.size);

    ASSERT_EQ(
      std::string(test_string),
      std::string(seq->data[0].string_value.data, seq->data[0].string_value.size));
  }
}

/* Basic nominal test of a subscription with take_serialize msg
 */
TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_subscription_serialized) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  rcutils_allocator_t allocator = rcl_get_default_allocator();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Strings);
  const char * topic = "/chatterSer";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  rcl_serialized_message_t serialized_msg = rmw_get_zero_initialized_serialized_message();
  auto initial_capacity_ser = 0u;
  ASSERT_EQ(
    RCL_RET_OK, rmw_serialized_message_init(
      &serialized_msg, initial_capacity_ser, &allocator)) << rcl_get_error_string().str;
  const char * test_string = "testing";
  test_msgs__msg__Strings msg;
  test_msgs__msg__Strings__init(&msg);
  ASSERT_TRUE(rosidl_runtime_c__String__assign(&msg.string_value, test_string));
  ASSERT_STREQ(msg.string_value.data, test_string);
  ret = rmw_serialize(&msg, ts, &serialized_msg);
  ASSERT_EQ(RMW_RET_OK, ret);

  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_reset_error();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  {
    ret = rcl_publish_serialized_message(&publisher, &serialized_msg, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
  {
    rcl_serialized_message_t serialized_msg_rcv = rmw_get_zero_initialized_serialized_message();
    initial_capacity_ser = 0u;
    ASSERT_EQ(
      RCL_RET_OK, rmw_serialized_message_init(
        &serialized_msg_rcv, initial_capacity_ser, &allocator)) << rcl_get_error_string().str;
    ret = rcl_take_serialized_message(&subscription, &serialized_msg_rcv, nullptr, nullptr);
    ASSERT_EQ(RMW_RET_OK, ret);

    test_msgs__msg__Strings msg_rcv;
    test_msgs__msg__Strings__init(&msg_rcv);
    ret = rmw_deserialize(&serialized_msg_rcv, ts, &msg_rcv);
    ASSERT_EQ(RMW_RET_OK, ret);
    ASSERT_EQ(
      std::string(test_string), std::string(msg_rcv.string_value.data, msg_rcv.string_value.size));
  }
}

/* Basic test for subscription loan functions
 */
TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_subscription_loaned) {
  rcl_ret_t ret;
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Strings);
  const char * topic = "rcl_loan";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  const char * test_string = "testing";
  {
    test_msgs__msg__Strings msg;
    test_msgs__msg__Strings__init(&msg);
    ASSERT_TRUE(rosidl_runtime_c__String__assign(&msg.string_value, test_string));
    ret = rcl_publish(&publisher, &msg, nullptr);
    test_msgs__msg__Strings__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
  {
    test_msgs__msg__Strings msg;
    test_msgs__msg__Strings * msg_loaned;
    test_msgs__msg__Strings__init(&msg);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      test_msgs__msg__Strings__fini(&msg);
    });

    // Only if rmw supports the functionality
    if (rcl_subscription_can_loan_messages(&subscription)) {
      ret = rcl_take_loaned_message(
        &subscription, reinterpret_cast<void **>(&msg_loaned), nullptr, nullptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      ASSERT_EQ(
        std::string(test_string),
        std::string(msg_loaned->string_value.data, msg_loaned->string_value.size));
    } else {
      ret = rcl_take(&subscription, &msg, nullptr, nullptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      ASSERT_EQ(
        std::string(test_string), std::string(msg.string_value.data, msg.string_value.size));
    }
  }
}

TEST_F(CLASSNAME(TestSubscriptionFixture, RMW_IMPLEMENTATION), test_get_options) {
  rcl_ret_t ret;
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Strings);
  const char * topic = "test_get_options";
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  const rcl_subscription_options_t * get_sub_options = rcl_subscription_get_options(&subscription);
  ASSERT_EQ(subscription_options.qos.history, get_sub_options->qos.history);
  ASSERT_EQ(subscription_options.qos.depth, get_sub_options->qos.depth);
  ASSERT_EQ(subscription_options.qos.durability, get_sub_options->qos.durability);

  ASSERT_EQ(NULL, rcl_subscription_get_options(nullptr));
}
