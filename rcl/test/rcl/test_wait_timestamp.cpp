// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include <algorithm>  // for std::max
#include <atomic>
#include <chrono>
#include <future>
#include <sstream>
#include <iostream>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/wait.h"
#include "rcl/graph.h"

#include "rcutils/logging_macros.h"

#include "test_msgs/msg/empty.h"
#include "test_msgs/srv/basic_types.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (WaitSetTimestampTestFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_context_t * context_ptr {nullptr};
  const rosidl_message_type_support_t * ts {nullptr};
  rcl_node_t * send_node_ptr {nullptr};
  rcl_node_t * receive_node_ptr {nullptr};
  rcl_publisher_t * pub_ptr {nullptr};
  rcl_subscription_t * sub_ptr {nullptr};
  test_msgs__msg__Empty msg;

  void SetUp()
  {
    rcl_ret_t ret;
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

    // type support for all
    const char * TOPIC = "test_wait_timestamp_pub_sub";
    ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Empty);
    EXPECT_NE(nullptr, ts);

    // create send node
    send_node_ptr = new rcl_node_t;
    *send_node_ptr = rcl_get_zero_initialized_node();
    rcl_node_options_t send_node_options = rcl_node_get_default_options();
    ret = rcl_node_init(send_node_ptr, "talker", "", context_ptr, &send_node_options);

    // receive node
    receive_node_ptr = new rcl_node_t;
    *receive_node_ptr = rcl_get_zero_initialized_node();
    rcl_node_options_t receive_node_options = rcl_node_get_default_options();
    ret = rcl_node_init(receive_node_ptr, "listener", "", this->context_ptr, &receive_node_options);

    // publisher
    rcl_publisher_options_t pub_opt = rcl_publisher_get_default_options();
    pub_ptr = new rcl_publisher_t;
    *pub_ptr = rcl_get_zero_initialized_publisher();
    ret = rcl_publisher_init(pub_ptr, send_node_ptr, ts, TOPIC, &pub_opt);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    rcl_subscription_options_t sub_opt = rcl_subscription_get_default_options();
    sub_ptr = new rcl_subscription_t;
    *sub_ptr = rcl_get_zero_initialized_subscription();
    ret = rcl_subscription_init(sub_ptr, receive_node_ptr, ts, TOPIC, &sub_opt);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    test_msgs__msg__Empty__init(&msg);
  }

  void TearDown()
  {
    test_msgs__msg__Empty__fini(&msg);

    EXPECT_EQ(RCL_RET_OK, rcl_publisher_fini(pub_ptr, send_node_ptr)) <<
      rcl_get_error_string().str;
    delete pub_ptr;
    pub_ptr = nullptr;
    EXPECT_EQ(RCL_RET_OK, rcl_subscription_fini(sub_ptr, receive_node_ptr)) <<
      rcl_get_error_string().str;
    delete sub_ptr;
    sub_ptr = nullptr;
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(this->receive_node_ptr)) << rcl_get_error_string().str;
    delete this->receive_node_ptr;
    this->receive_node_ptr = nullptr;
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(this->send_node_ptr)) << rcl_get_error_string().str;
    delete this->send_node_ptr;
    this->send_node_ptr = nullptr;
    EXPECT_EQ(RCL_RET_OK, rcl_shutdown(this->context_ptr)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_context_fini(this->context_ptr)) << rcl_get_error_string().str;
    delete this->context_ptr;
    this->context_ptr = nullptr;
  }

  void wait_for_communication_ready()
  {
    // TODO(iluetkeb): check events to determine when the connection is there,
    // instead of blocking the test for 1s...
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};

TEST_F(CLASSNAME(WaitSetTimestampTestFixture, RMW_IMPLEMENTATION), test_pub_sub) {
  rcl_ret_t ret = RCL_RET_OK;

  // wait for setup to complete, then send two messages, a little time apart
  wait_for_communication_ready();
  {
    ret = rcl_publish(pub_ptr, &msg, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ret = rcl_publish(pub_ptr, &msg, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  // wait for middleware
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  ret =
    rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, 0, context_ptr, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_wait_set_fini(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  ret = rcl_wait_set_clear(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ret = rcl_wait_set_add_subscription(&wait_set, sub_ptr, NULL);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ret = rcl_wait(&wait_set, RCL_MS_TO_NS(1000));
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // check for presence indicator
  EXPECT_NE(nullptr, wait_set.subscriptions[0]);

  // check for timestamp presence
  EXPECT_NE(0LL, wait_set.subscriptions_timestamps[0]);

  // now take the message to clear it from the queue
  ret = rcl_take(sub_ptr, &msg, nullptr, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(CLASSNAME(WaitSetTimestampTestFixture, RMW_IMPLEMENTATION), test_client_service) {
  rcl_ret_t ret = RCL_RET_OK;
  const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
    test_msgs, srv, BasicTypes);
  const char * topic = "primitives";

  rcl_service_t service = rcl_get_zero_initialized_service();
  rcl_service_options_t service_options = rcl_service_get_default_options();
  ret = rcl_service_init(&service, send_node_ptr, ts, topic, &service_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_service_fini(&service, send_node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // query the graph until the service becomes available
  rcl_names_and_types_t tnat {};
  rcl_allocator_t allocator = rcl_get_default_allocator();
  do {
    ret = rcl_get_service_names_and_types(
      receive_node_ptr, &allocator, &tnat);
    EXPECT_EQ(RCL_RET_OK, ret);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      rcl_ret_t ret = rcl_names_and_types_fini(&tnat);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    });
  } while (tnat.names.size == 0);


  // create the client
  rcl_client_t client = rcl_get_zero_initialized_client();

  // Initialize the client.
  rcl_client_options_t client_options = rcl_client_get_default_options();
  ret = rcl_client_init(&client, send_node_ptr, ts, topic, &client_options);
  EXPECT_EQ(RCL_RET_OK, ret);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_client_fini(&client, receive_node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // send request
  test_msgs__srv__BasicTypes_Request client_request;
  test_msgs__srv__BasicTypes_Request__init(&client_request);
  int64_t sequence_number;
  ret = rcl_send_request(&client, &client_request, &sequence_number);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(sequence_number, 1);

  // wait for it
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  ret =
    rcl_wait_set_init(&wait_set, 0, 0, 0, 0, 1, 0, context_ptr, allocator);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_wait_set_fini(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // first the service
  ret = rcl_wait_set_add_service(&wait_set, &service, NULL);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ret = rcl_wait(&wait_set, RCL_MS_TO_NS(1000));
  EXPECT_EQ(RCL_RET_OK, ret);

  bool request_received = false;
  rcutils_time_point_value_t timestamp = 0;
  for (size_t i = 0; i < wait_set.size_of_services; ++i) {
    if (wait_set.services[i] && wait_set.services[i] == &service) {
      request_received = true;
      timestamp = wait_set.services_timestamps[i];
    }
  }
  EXPECT_EQ(true, request_received);
  EXPECT_NE(0, timestamp);
}
