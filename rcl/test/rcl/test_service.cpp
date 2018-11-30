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

#include "rcl/service.h"

#include "rcl/rcl.h"

#include "test_msgs/srv/primitives.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestServiceFixture, RMW_IMPLEMENTATION) : public ::testing::Test
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
    const char * name = "test_service_node";
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
wait_for_service_to_be_ready(
  rcl_service_t * service,
  size_t max_tries,
  int64_t period_ms,
  bool & success)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 0, 0, 0, 1, rcl_get_default_allocator());
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
    ret = rcl_wait_set_add_service(&wait_set, service, NULL);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait(&wait_set, RCL_MS_TO_NS(period_ms));
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    for (size_t i = 0; i < wait_set.size_of_services; ++i) {
      if (wait_set.services[i] && wait_set.services[i] == service) {
        success = true;
        return;
      }
    }
  } while (iteration < max_tries);
  success = false;
}

/* Basic nominal test of a service.
 */
TEST_F(CLASSNAME(TestServiceFixture, RMW_IMPLEMENTATION), test_service_nominal) {
  rcl_ret_t ret;
  const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
    test_msgs, srv, Primitives);
  const char * topic = "primitives";
  const char * expected_topic = "/primitives";

  rcl_service_t service = rcl_get_zero_initialized_service();
  rcl_service_options_t service_options = rcl_service_get_default_options();
  ret = rcl_service_init(&service, this->node_ptr, ts, topic, &service_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // Check if null service is valid
  EXPECT_FALSE(rcl_service_is_valid(nullptr));
  rcl_reset_error();

  // Check if zero initialized client is valid
  service = rcl_get_zero_initialized_service();
  EXPECT_FALSE(rcl_service_is_valid(&service));
  rcl_reset_error();

  // Check that a valid service is valid
  service = rcl_get_zero_initialized_service();
  ret = rcl_service_init(&service, this->node_ptr, ts, topic, &service_options);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_TRUE(rcl_service_is_valid(&service));
  rcl_reset_error();

  // Check that the service name matches what we assigned.
  EXPECT_EQ(strcmp(rcl_service_get_service_name(&service), expected_topic), 0);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_service_fini(&service, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  rcl_client_t client = rcl_get_zero_initialized_client();
  rcl_client_options_t client_options = rcl_client_get_default_options();
  ret = rcl_client_init(&client, this->node_ptr, ts, topic, &client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_client_fini(&client, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // TODO(wjwwood): add logic to wait for the connection to be established
  //                use count_services busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Initialize a request.
  test_msgs__srv__Primitives_Request client_request;
  test_msgs__srv__Primitives_Request__init(&client_request);
  client_request.uint8_value = 1;
  client_request.uint32_value = 2;
  int64_t sequence_number;
  ret = rcl_send_request(&client, &client_request, &sequence_number);
  EXPECT_EQ(sequence_number, 1);
  test_msgs__srv__Primitives_Request__fini(&client_request);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  bool success;
  wait_for_service_to_be_ready(&service, 10, 100, success);
  ASSERT_TRUE(success);

  // This scope simulates the service responding in a different context so that we can
  // test take_request/send_response in a single-threaded, deterministic execution.
  {
    // Initialize a response.
    test_msgs__srv__Primitives_Response service_response;
    test_msgs__srv__Primitives_Response__init(&service_response);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      test_msgs__srv__Primitives_Response__fini(&service_response);
    });

    // Initialize a separate instance of the request and take the pending request.
    test_msgs__srv__Primitives_Request service_request;
    test_msgs__srv__Primitives_Request__init(&service_request);
    rmw_request_id_t header;
    ret = rcl_take_request(&service, &header, &service_request);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    EXPECT_EQ(1, service_request.uint8_value);
    EXPECT_EQ(2UL, service_request.uint32_value);
    // Simulate a response callback by summing the request and send the response..
    service_response.uint64_value = service_request.uint8_value + service_request.uint32_value;
    ret = rcl_send_response(&service, &header, &service_response);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  wait_for_service_to_be_ready(&service, 10, 100, success);

  // Initialize the response owned by the client and take the response.
  test_msgs__srv__Primitives_Response client_response;
  test_msgs__srv__Primitives_Response__init(&client_response);

  rmw_request_id_t header;
  ret = rcl_take_response(&client, &header, &client_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(client_response.uint64_value, 3ULL);
  EXPECT_EQ(header.sequence_number, 1);
}
