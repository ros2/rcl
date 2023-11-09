// Copyright 2022 Open Source Robotics Foundation, Inc.
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
#include <rosidl_runtime_c/service_type_support_struct.h>
#include <service_msgs/msg/detail/service_event_info__struct.h>

#include <cstdint>
#include <cstring>

#include "../mocking_utils/patch.hpp"
#include "./service_event_publisher.h"
#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/allocator.h"
#include "rcl/client.h"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/service.h"
#include "rcl/service_introspection.h"
#include "rcl/subscription.h"
#include "rcl/time.h"
#include "rcl/types.h"
#include "rmw/rmw.h"
#include "service_msgs/msg/service_event_info.h"
#include "test_msgs/srv/basic_types.h"
#include "wait_for_entity_helpers.hpp"

class TestServiceEventPublisherFixture : public ::testing::Test
{
public:
  void SetUp() override
  {
    rcl_ret_t ret;
    allocator = rcl_get_default_allocator();

    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });

    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    constexpr char name[] = "test_service_event_publisher_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->clock_ptr = new rcl_clock_t;
    ret = rcl_clock_init(RCL_STEADY_TIME, clock_ptr, &allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown() override
  {
    rcl_ret_t ret;

    ret = rcl_clock_fini(this->clock_ptr);
    delete this->clock_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_context_fini(this->context_ptr);
    delete this->context_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

protected:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;
  rcl_clock_t * clock_ptr;
  rcl_allocator_t allocator;
  const rosidl_service_type_support_t * srv_ts =
    ROSIDL_GET_SRV_TYPE_SUPPORT(test_msgs, srv, BasicTypes);
};

/* Basic nominal test of service introspection features covering init, fini, and sending a message
 */
TEST_F(TestServiceEventPublisherFixture, test_service_event_publisher_nominal)
{
  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();
  rcl_ret_t ret;

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, this->node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_service_event_publisher_change_state(
    &service_event_publisher, RCL_SERVICE_INTROSPECTION_METADATA);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  test_msgs__srv__BasicTypes_Request client_request;
  test_msgs__srv__BasicTypes_Request__init(&client_request);

  client_request.bool_value = false;
  client_request.uint8_value = 1;
  client_request.uint32_value = 2;
  int64_t sequence_number = 1;
  uint8_t guid[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

  ret = rcl_send_service_event_message(
    &service_event_publisher, service_msgs__msg__ServiceEventInfo__REQUEST_SENT, &client_request,
    sequence_number, guid);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_service_event_publisher_fini(&service_event_publisher, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(TestServiceEventPublisherFixture, test_service_event_publisher_init_and_fini)
{
  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();
  rcl_clock_t clock;
  rcl_ret_t ret;

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, nullptr, &clock, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, this->node_ptr, nullptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_service_event_publisher_fini(&service_event_publisher, nullptr);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "123test_service_event_publisher<>h", srv_ts);
  EXPECT_EQ(RCL_RET_TOPIC_NAME_INVALID, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();

  {
    auto mock = mocking_utils::patch_to_fail(
      "lib:rcl", rmw_create_publisher, "patch rmw_create_publisher to fail", nullptr);
    ret = rcl_service_event_publisher_init(
      &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
      "test_service_event_publisher", srv_ts);
    EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
    rcutils_reset_error();
  }
}

/* Test sending service introspection message via service_event_publisher.h
 */
TEST_F(TestServiceEventPublisherFixture, test_service_event_publisher_send_message_nominal)
{
  uint8_t guid[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  auto sub_opts = rcl_subscription_get_default_options();
  std::string topic = "test_service_event_publisher";
  std::string service_event_topic = topic + RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX;
  rcl_ret_t ret;

  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    topic.c_str(), srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  ret = rcl_service_event_publisher_change_state(
    &service_event_publisher, RCL_SERVICE_INTROSPECTION_CONTENTS);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(
    &subscription, node_ptr, srv_ts->event_typesupport, service_event_topic.c_str(), &sub_opts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  ASSERT_TRUE(wait_for_established_subscription(service_event_publisher.publisher, 10, 100));

  test_msgs__srv__BasicTypes_Request test_req;
  memset(&test_req, 0, sizeof(test_msgs__srv__BasicTypes_Request));
  test_msgs__srv__BasicTypes_Request__init(&test_req);
  test_req.bool_value = true;
  test_req.uint16_value = 42;
  test_req.uint32_value = 123;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Request__fini(&test_req);});

  ret = rcl_send_service_event_message(
    &service_event_publisher, service_msgs__msg__ServiceEventInfo__REQUEST_RECEIVED, &test_req, 1,
    guid);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ASSERT_TRUE(wait_for_subscription_to_be_ready(&subscription, context_ptr, 10, 100));

  rmw_message_info_t message_info = rmw_get_zero_initialized_message_info();
  test_msgs__srv__BasicTypes_Event event_msg;
  test_msgs__srv__BasicTypes_Event__init(&event_msg);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Event__fini(&event_msg);});
  ret = rcl_take(&subscription, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(1, event_msg.info.sequence_number);
  ASSERT_EQ(0, memcmp(guid, event_msg.info.client_gid, sizeof(guid)));
  ASSERT_EQ(0U, event_msg.response.size);
  ASSERT_EQ(1U, event_msg.request.size);
  ASSERT_EQ(test_req.bool_value, event_msg.request.data[0].bool_value);
  ASSERT_EQ(test_req.uint16_value, event_msg.request.data[0].uint16_value);
  ASSERT_EQ(test_req.uint32_value, event_msg.request.data[0].uint32_value);
}

TEST_F(TestServiceEventPublisherFixture, test_service_event_publisher_send_message_return_codes)
{
  rcl_ret_t ret;

  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();

  uint8_t guid[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  char topic[] = "test_service_event_publisher";

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    topic, srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_service_event_publisher_change_state(
    &service_event_publisher, RCL_SERVICE_INTROSPECTION_METADATA);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_send_service_event_message(nullptr, 0, nullptr, 0, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  test_msgs__srv__BasicTypes_Request test_req;
  test_msgs__srv__BasicTypes_Request__init(&test_req);
  test_req.bool_value = true;
  test_req.uint16_value = 42;
  test_req.uint32_value = 123;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Request__fini(&test_req);});

  ret = rcl_send_service_event_message(
    &service_event_publisher, service_msgs__msg__ServiceEventInfo__REQUEST_SENT, &test_req, 0,
    nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret = rcl_send_service_event_message(
    &service_event_publisher, service_msgs__msg__ServiceEventInfo__RESPONSE_RECEIVED, &test_req, 0,
    guid);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_send_service_event_message(&service_event_publisher, 5, &test_req, 0, guid);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(TestServiceEventPublisherFixture, test_service_event_publisher_utils)
{
  rcl_ret_t ret;

  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  EXPECT_TRUE(rcl_service_event_publisher_is_valid(&service_event_publisher));

  rcl_publisher_fini(service_event_publisher.publisher, node_ptr);
  EXPECT_TRUE(rcl_service_event_publisher_is_valid(&service_event_publisher));

  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  service_event_publisher.clock = nullptr;
  EXPECT_FALSE(rcl_service_event_publisher_is_valid(&service_event_publisher));

  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  service_event_publisher.clock = clock_ptr;
  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(
  TestServiceEventPublisherFixture,
  test_service_event_publisher_enable_and_disable_return_codes)
{
  rcl_ret_t ret;

  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();

  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // ok to enable twice
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_service_event_publisher_change_state(
      &service_event_publisher, RCL_SERVICE_INTROSPECTION_METADATA));
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_service_event_publisher_change_state(
      &service_event_publisher, RCL_SERVICE_INTROSPECTION_METADATA));

  EXPECT_EQ(
    RCL_RET_OK,
    rcl_service_event_publisher_change_state(
      &service_event_publisher, RCL_SERVICE_INTROSPECTION_OFF));
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_service_event_publisher_change_state(
      &service_event_publisher, RCL_SERVICE_INTROSPECTION_OFF));

  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

class TestServiceEventPublisherWithServicesAndClientsFixture : public ::testing::Test
{
public:
  void SetUp()
  {
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });

    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_service_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->clock_ptr = new rcl_clock_t;
    ret = rcl_clock_init(RCL_STEADY_TIME, clock_ptr, &allocator);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    std::string srv_name = "test_service_introspection_service";
    std::string service_event_topic = srv_name + RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX;

    this->service_ptr = new rcl_service_t;
    *this->service_ptr = rcl_get_zero_initialized_service();
    rcl_service_options_t service_options = rcl_service_get_default_options();
    ret = rcl_service_init(
      this->service_ptr, this->node_ptr, srv_ts, srv_name.c_str(), &service_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_service_configure_service_introspection(
      this->service_ptr, this->node_ptr, clock_ptr, srv_ts, rcl_publisher_get_default_options(),
      RCL_SERVICE_INTROSPECTION_CONTENTS);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->client_ptr = new rcl_client_t;
    *this->client_ptr = rcl_get_zero_initialized_client();
    rcl_client_options_t client_options = rcl_client_get_default_options();
    ret = rcl_client_init(
      this->client_ptr, this->node_ptr, srv_ts, srv_name.c_str(), &client_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_client_configure_service_introspection(
      this->client_ptr, this->node_ptr, clock_ptr, srv_ts, rcl_publisher_get_default_options(),
      RCL_SERVICE_INTROSPECTION_CONTENTS);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->subscription_ptr = new rcl_subscription_t;
    *this->subscription_ptr = rcl_get_zero_initialized_subscription();
    rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
    ret = rcl_subscription_init(
      this->subscription_ptr, this->node_ptr, srv_ts->event_typesupport,
      service_event_topic.c_str(), &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ASSERT_TRUE(wait_for_established_publisher(this->subscription_ptr, 10, 100));

    ASSERT_TRUE(wait_for_server_to_be_available(this->node_ptr, this->client_ptr, 10, 1000));
  }

  void TearDown()
  {
    rcl_ret_t ret;

    ret = rcl_subscription_fini(this->subscription_ptr, this->node_ptr);
    delete this->subscription_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_client_fini(this->client_ptr, this->node_ptr);
    delete this->client_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_service_fini(this->service_ptr, this->node_ptr);
    delete this->service_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_clock_fini(this->clock_ptr);
    delete this->clock_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_context_fini(this->context_ptr);
    delete this->context_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

protected:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;
  rcl_service_t * service_ptr;
  rcl_client_t * client_ptr;
  rcl_clock_t * clock_ptr;
  rcl_subscription_t * subscription_ptr;
  const rosidl_service_type_support_t * srv_ts =
    ROSIDL_GET_SRV_TYPE_SUPPORT(test_msgs, srv, BasicTypes);
};

/* Whole test of service event publisher with service, client, and subscription
 */
TEST_F(
  TestServiceEventPublisherWithServicesAndClientsFixture,
  test_service_event_publisher_with_subscriber)
{
  rcl_ret_t ret;
  rmw_message_info_t message_info = rmw_get_zero_initialized_message_info();
  test_msgs__srv__BasicTypes_Event event_msg;
  test_msgs__srv__BasicTypes_Event__init(&event_msg);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Event__fini(&event_msg);});

  test_msgs__srv__BasicTypes_Request client_request;
  memset(&client_request, 0, sizeof(test_msgs__srv__BasicTypes_Request));
  test_msgs__srv__BasicTypes_Request__init(&client_request);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Request__fini(&client_request);});
  client_request.bool_value = false;
  client_request.uint8_value = 1;
  client_request.uint32_value = 2;

  int64_t sequence_number;
  ret = rcl_send_request(this->client_ptr, &client_request, &sequence_number);
  EXPECT_NE(sequence_number, 0);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ASSERT_TRUE(wait_for_service_to_be_ready(this->service_ptr, this->context_ptr, 10, 100));

  // expect a REQUEST_SENT event
  ASSERT_TRUE(
    wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100));
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(service_msgs__msg__ServiceEventInfo__REQUEST_SENT, event_msg.info.event_type);

  test_msgs__srv__BasicTypes_Response service_response;
  memset(&service_response, 0, sizeof(test_msgs__srv__BasicTypes_Response));
  test_msgs__srv__BasicTypes_Response__init(&service_response);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {test_msgs__srv__BasicTypes_Response__fini(&service_response);});

  test_msgs__srv__BasicTypes_Request service_request;
  memset(&service_request, 0, sizeof(test_msgs__srv__BasicTypes_Request));
  test_msgs__srv__BasicTypes_Request__init(&service_request);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {test_msgs__srv__BasicTypes_Request__fini(&service_request);});

  rmw_service_info_t header;
  ret = rcl_take_request(
    this->service_ptr, &(header.request_id), &service_request);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(2U, service_request.uint32_value);

  // expect a REQUEST_RECEIVED event
  ASSERT_TRUE(
    wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100));
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(service_msgs__msg__ServiceEventInfo__REQUEST_RECEIVED, event_msg.info.event_type);

  service_response.uint32_value = 2;
  service_response.uint8_value = 3;
  ret = rcl_send_response(this->service_ptr, &header.request_id, &service_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // expect a RESPONSE_SEND event
  ASSERT_TRUE(
    wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100));
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(service_msgs__msg__ServiceEventInfo__RESPONSE_SENT, event_msg.info.event_type);

  test_msgs__srv__BasicTypes_Response client_response;
  memset(&client_response, 0, sizeof(test_msgs__srv__BasicTypes_Response));
  test_msgs__srv__BasicTypes_Response__init(&client_response);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {test_msgs__srv__BasicTypes_Response__fini(&client_response);});

  ASSERT_TRUE(
    wait_for_client_to_be_ready(this->client_ptr, this->context_ptr, 10, 100));
  ret = rcl_take_response(this->client_ptr, &(header.request_id), &client_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // expect a RESPONSE_RECEIVED event
  ASSERT_TRUE(
    wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100));
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(service_msgs__msg__ServiceEventInfo__RESPONSE_RECEIVED, event_msg.info.event_type);
  ASSERT_EQ(1U, event_msg.response.size);
  ASSERT_EQ(2U, event_msg.response.data[0].uint32_value);
}

/* Integration level test with disabling service events
 */
TEST_F(
  TestServiceEventPublisherWithServicesAndClientsFixture,
  test_service_event_publisher_with_subscriber_disable_service_events)
{
  rcl_ret_t ret;
  rmw_message_info_t message_info = rmw_get_zero_initialized_message_info();
  test_msgs__srv__BasicTypes_Event event_msg;
  test_msgs__srv__BasicTypes_Event__init(&event_msg);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Event__fini(&event_msg);});

  ret = rcl_service_configure_service_introspection(
    this->service_ptr, this->node_ptr, this->clock_ptr, srv_ts, rcl_publisher_get_default_options(),
    RCL_SERVICE_INTROSPECTION_OFF);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  test_msgs__srv__BasicTypes_Request client_request;
  memset(&client_request, 0, sizeof(test_msgs__srv__BasicTypes_Request));
  test_msgs__srv__BasicTypes_Request__init(&client_request);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({test_msgs__srv__BasicTypes_Request__fini(&client_request);});
  client_request.bool_value = false;
  client_request.uint8_value = 1;
  client_request.uint32_value = 2;

  int64_t sequence_number;
  ret = rcl_send_request(this->client_ptr, &client_request, &sequence_number);
  EXPECT_NE(sequence_number, 0);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ASSERT_TRUE(wait_for_service_to_be_ready(this->service_ptr, this->context_ptr, 10, 100));

  // expect a REQUEST_SENT event
  ASSERT_TRUE(
    wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100));
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(service_msgs__msg__ServiceEventInfo__REQUEST_SENT, event_msg.info.event_type);

  test_msgs__srv__BasicTypes_Response service_response;
  memset(&service_response, 0, sizeof(test_msgs__srv__BasicTypes_Response));
  test_msgs__srv__BasicTypes_Response__init(&service_response);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {test_msgs__srv__BasicTypes_Response__fini(&service_response);});

  test_msgs__srv__BasicTypes_Request service_request;
  memset(&service_request, 0, sizeof(test_msgs__srv__BasicTypes_Request));
  test_msgs__srv__BasicTypes_Request__init(&service_request);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {test_msgs__srv__BasicTypes_Request__fini(&service_request);});

  rmw_service_info_t header;
  ret = rcl_take_request(
    this->service_ptr, &(header.request_id), &service_request);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(2U, service_request.uint32_value);

  // expect take to fail since no introspection message should be published
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_SUBSCRIPTION_TAKE_FAILED, ret) << rcl_get_error_string().str;

  service_response.uint32_value = 2;
  service_response.uint8_value = 3;
  ret = rcl_send_response(this->service_ptr, &header.request_id, &service_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // expect take to fail since no introspection message should be published
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_SUBSCRIPTION_TAKE_FAILED, ret) << rcl_get_error_string().str;

  test_msgs__srv__BasicTypes_Response client_response;
  memset(&client_response, 0, sizeof(test_msgs__srv__BasicTypes_Response));
  test_msgs__srv__BasicTypes_Response__init(&client_response);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {test_msgs__srv__BasicTypes_Response__fini(&client_response);});

  ASSERT_TRUE(
    wait_for_client_to_be_ready(this->client_ptr, this->context_ptr, 10, 100));
  ret = rcl_take_response(this->client_ptr, &(header.request_id), &client_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // expect a RESPONSE_RECEIVED event
  ASSERT_TRUE(
    wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100));
  ret = rcl_take(this->subscription_ptr, &event_msg, &message_info, nullptr);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  ASSERT_EQ(service_msgs__msg__ServiceEventInfo__RESPONSE_RECEIVED, event_msg.info.event_type);
  ASSERT_EQ(2U, event_msg.response.data[0].uint32_value);
}
