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

#ifdef RMW_IMPLEMENTATION
#define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
#define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
#define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestServiceEventPublisherFixture, RMW_IMPLEMENTATION) : public ::testing::Test
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
TEST_F(
  CLASSNAME(TestServiceEventPublisherFixture, RMW_IMPLEMENTATION),
  test_service_event_publisher_nominal)
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

TEST_F(
  CLASSNAME(TestServiceEventPublisherFixture, RMW_IMPLEMENTATION),
  test_service_event_publisher_init_and_fini)
{
  rcl_service_event_publisher_t service_event_publisher =
    rcl_get_zero_initialized_service_event_publisher();
  rcl_clock_t clock;
  rcl_ret_t ret;

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_init with uninitialized clock\n");
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, nullptr, &clock, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_init with null clock\n");
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, this->node_ptr, nullptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_init with valid clock\n");
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_fini with null node\n");
  ret = rcl_service_event_publisher_fini(&service_event_publisher, nullptr);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_fini valid\n");
  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_init with invalid topic name\n");
  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "123test_service_event_publisher<>h", srv_ts);
  EXPECT_EQ(RCL_RET_TOPIC_NAME_INVALID, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_init with valid topic name\n");
  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_fini with valid topic name\n");
  ret = rcl_service_event_publisher_fini(&service_event_publisher, node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  service_event_publisher = rcl_get_zero_initialized_service_event_publisher();

  fprintf(stderr, "CHRIS: Before rcl_service_event_publisher_init patch to fail\n");
  auto mock = mocking_utils::patch_to_fail(
    "lib:rcl", rcl_publisher_init, "patch rcl_publisher_init to fail", RCL_RET_ERROR);
  ret = rcl_service_event_publisher_init(
    &service_event_publisher, node_ptr, clock_ptr, rcl_publisher_get_default_options(),
    "test_service_event_publisher", srv_ts);
  EXPECT_EQ(RCL_RET_ERROR, ret) << rcl_get_error_string().str;
  rcutils_reset_error();
}
