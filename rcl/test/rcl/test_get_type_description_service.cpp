// Copyright 2023 Open Source Robotics Foundation, Inc.
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
#include <cstring>
#include <thread>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/service.h"
#include "rcl/rcl.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcutils/types/string_array.h"
#include "rosidl_runtime_c/string_functions.h"
#include "type_description_interfaces/srv/get_type_description.h"

#include "node_impl.h"  // NOLINT
#include "wait_for_entity_helpers.hpp"

constexpr char GET_TYPE_DESCRIPTION_SRV_TYPE_NAME[] =
  "type_description_interfaces/srv/GetTypeDescription";

static bool string_in_array(rcutils_string_array_t * array, const char * pattern)
{
  for (size_t i = 0; i < array->size; ++i) {
    if (strcmp(array->data[i], pattern) == 0) {
      return true;
    }
  }
  return false;
}

static bool service_exists(
  const rcl_node_t * node_ptr, const char * service_name,
  const char * service_type, std::chrono::milliseconds timeout)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();

  // Wait for a maximum of timeout milliseconds for the service to show up
  auto start_time = std::chrono::system_clock::now();
  while (std::chrono::system_clock::now() - start_time < timeout) {
    rcl_names_and_types_t srv_names_and_types = rcl_get_zero_initialized_names_and_types();

    if (
      RCL_RET_OK != rcl_get_service_names_and_types(
        node_ptr,
        &allocator, &srv_names_and_types))
    {
      return false;
    }

    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      EXPECT_EQ(RCL_RET_OK, rcl_names_and_types_fini(&srv_names_and_types));
    });

    if (srv_names_and_types.names.size >= 1) {
      if (string_in_array(&(srv_names_and_types.names), service_name)) {
        for (size_t i = 0; i < srv_names_and_types.names.size; ++i) {
          if (string_in_array(&(srv_names_and_types.types[i]), service_type)) {
            return true;
          }
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return false;
}

static bool service_not_exists(
  const rcl_node_t * node_ptr, const char * service_name,
  const char * service_type, std::chrono::milliseconds timeout)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();

  // Wait for a maximum of timeout milliseconds for the service to go away
  // Note that this is not just the negation of service_exists; we actually
  // want to wait until the service goes away
  auto start_time = std::chrono::system_clock::now();
  while (std::chrono::system_clock::now() - start_time < timeout) {
    rcl_names_and_types_t srv_names_and_types = rcl_get_zero_initialized_names_and_types();

    if (
      RCL_RET_OK != rcl_get_service_names_and_types(
        node_ptr,
        &allocator, &srv_names_and_types))
    {
      return false;
    }

    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      EXPECT_EQ(RCL_RET_OK, rcl_names_and_types_fini(&srv_names_and_types));
    });

    if (srv_names_and_types.names.size == 0) {
      return true;
    }

    if (!string_in_array(&(srv_names_and_types.names), service_name)) {
      return true;
    }

    // If we make it here, the service name was in the array.  If the *type* isn't, then there is
    // another service with this name, which we can ignore.
    for (size_t i = 0; i < srv_names_and_types.names.size; ++i) {
      if (string_in_array(&(srv_names_and_types.types[i]), service_type)) {
        // Both the name and type were here, so the service currently exists.  Continue waiting
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return false;
}

class TestGetTypeDescSrvFixture : public ::testing::Test
{
public:
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
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_service_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    const char * node_fqn = rcl_node_get_fully_qualified_name(this->node_ptr);
    snprintf(
      get_type_description_service_name, sizeof(get_type_description_service_name),
      "%s/get_type_description", node_fqn);
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

protected:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;
  char get_type_description_service_name[256];
};


/* Test init and fini functions. */
TEST_F(TestGetTypeDescSrvFixture, test_service_init_and_fini_functions) {
  rcl_service_t service = rcl_get_zero_initialized_service();

  // Service does not initially exist
  EXPECT_TRUE(
    service_not_exists(
      this->node_ptr, this->get_type_description_service_name,
      GET_TYPE_DESCRIPTION_SRV_TYPE_NAME, std::chrono::seconds(5)));

  // Once the type descrition service is init, then it appear in the graph
  EXPECT_EQ(RCL_RET_OK, rcl_node_type_description_service_init(&service, this->node_ptr));
  EXPECT_TRUE(
    service_exists(
      this->node_ptr, this->get_type_description_service_name,
      GET_TYPE_DESCRIPTION_SRV_TYPE_NAME, std::chrono::seconds(5)));

  // Once the type description service is fini, then it no longer appears in the graph
  EXPECT_EQ(RCL_RET_OK, rcl_service_fini(&service, this->node_ptr));
  EXPECT_TRUE(
    service_not_exists(
      this->node_ptr, this->get_type_description_service_name,
      GET_TYPE_DESCRIPTION_SRV_TYPE_NAME, std::chrono::seconds(5)));

  // Repeatedly destroying the service should not cause faults.
  EXPECT_EQ(RCL_RET_OK, rcl_service_fini(&service, this->node_ptr));
}

/* Basic nominal test of the ~/get_type_description service. */
TEST_F(TestGetTypeDescSrvFixture, test_service_nominal) {
  rcl_ret_t ret;
  const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
    type_description_interfaces, srv, GetTypeDescription);

  // Create type description service.
  auto service = rcl_get_zero_initialized_service();
  EXPECT_EQ(RCL_RET_OK, rcl_node_type_description_service_init(&service, this->node_ptr));

  // Create client.
  rcl_client_t client = rcl_get_zero_initialized_client();
  rcl_client_options_t client_options = rcl_client_get_default_options();
  ret = rcl_client_init(
    &client, this->node_ptr, ts, this->get_type_description_service_name,
    &client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_client_fini(&client, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_service_fini(&service, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  ASSERT_TRUE(wait_for_server_to_be_available(this->node_ptr, &client, 10, 1000));

  // Initialize a request.
  type_description_interfaces__srv__GetTypeDescription_Request client_request;
  type_description_interfaces__srv__GetTypeDescription_Request__init(&client_request);

  // Fill the request. We use the GetTypeDescription hash because we know that that type
  // is registered.
  const rosidl_type_hash_t * type_hash = ts->get_type_hash_func(ts);
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * type_hash_str;
  EXPECT_EQ(RCUTILS_RET_OK, rosidl_stringify_type_hash(type_hash, allocator, &type_hash_str));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(type_hash_str, allocator.state);
  });
  rosidl_runtime_c__String__assign(&client_request.type_hash, type_hash_str);
  rosidl_runtime_c__String__assign(&client_request.type_name, GET_TYPE_DESCRIPTION_SRV_TYPE_NAME);
  client_request.include_type_sources = false;

  // Send the request.
  int64_t sequence_number;
  ret = rcl_send_request(&client, &client_request, &sequence_number);
  type_description_interfaces__srv__GetTypeDescription_Request__fini(&client_request);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // This scope simulates handling request in a different context
  {
    auto * service_ptr = &service;
    ASSERT_TRUE(wait_for_service_to_be_ready(service_ptr, context_ptr, 10, 100));

    type_description_interfaces__srv__GetTypeDescription_Response service_response;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      type_description_interfaces__srv__GetTypeDescription_Response__fini(&service_response);
    });

    type_description_interfaces__srv__GetTypeDescription_Request service_request;
    type_description_interfaces__srv__GetTypeDescription_Request__init(&service_request);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      type_description_interfaces__srv__GetTypeDescription_Request__fini(&service_request);
    });
    rmw_service_info_t header;
    ret = rcl_take_request_with_info(service_ptr, &header, &service_request);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    rcl_node_type_description_service_handle_request(
      node_ptr,
      &header.request_id,
      &service_request,
      &service_response);

    ret = rcl_send_response(service_ptr, &header.request_id, &service_response);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  ASSERT_TRUE(wait_for_client_to_be_ready(&client, context_ptr, 10, 100));
  // Initialize the response owned by the client and take the response.
  type_description_interfaces__srv__GetTypeDescription_Response client_response;
  type_description_interfaces__srv__GetTypeDescription_Response__init(&client_response);

  // Retrieve the response.
  rmw_service_info_t header;
  ret = rcl_take_response_with_info(&client, &header, &client_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(client_response.successful, true);
  EXPECT_EQ(sequence_number, header.request_id.sequence_number);

  type_description_interfaces__srv__GetTypeDescription_Response__fini(&client_response);
}

/* Test calling ~/get_type_description service with invalid hash. */
TEST_F(TestGetTypeDescSrvFixture, test_service_invalid_hash) {
  rcl_ret_t ret;
  const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
    type_description_interfaces, srv, GetTypeDescription);

  // Create type description service.
  auto service = rcl_get_zero_initialized_service();
  EXPECT_EQ(RCL_RET_OK, rcl_node_type_description_service_init(&service, this->node_ptr));

  // Create client.
  rcl_client_t client = rcl_get_zero_initialized_client();
  rcl_client_options_t client_options = rcl_client_get_default_options();
  ret = rcl_client_init(
    &client, this->node_ptr, ts, this->get_type_description_service_name,
    &client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_ret_t ret = rcl_client_fini(&client, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_service_fini(&service, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  ASSERT_TRUE(wait_for_server_to_be_available(this->node_ptr, &client, 10, 1000));

  // Initialize a request.
  type_description_interfaces__srv__GetTypeDescription_Request client_request;
  type_description_interfaces__srv__GetTypeDescription_Request__init(&client_request);

  // Fill the request.
  rosidl_runtime_c__String__assign(&client_request.type_hash, "foo");
  rosidl_runtime_c__String__assign(&client_request.type_name, "bar");
  client_request.include_type_sources = false;

  // Send the request.
  int64_t sequence_number;
  ret = rcl_send_request(&client, &client_request, &sequence_number);
  type_description_interfaces__srv__GetTypeDescription_Request__fini(&client_request);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // This scope simulates handling request in a different context
  {
    auto * service_ptr = &service;
    ASSERT_TRUE(wait_for_service_to_be_ready(service_ptr, context_ptr, 10, 100));

    type_description_interfaces__srv__GetTypeDescription_Response service_response;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      type_description_interfaces__srv__GetTypeDescription_Response__fini(&service_response);
    });

    type_description_interfaces__srv__GetTypeDescription_Request service_request;
    type_description_interfaces__srv__GetTypeDescription_Request__init(&service_request);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      type_description_interfaces__srv__GetTypeDescription_Request__fini(&service_request);
    });
    rmw_service_info_t header;
    ret = rcl_take_request_with_info(service_ptr, &header, &service_request);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    rcl_node_type_description_service_handle_request(
      node_ptr,
      &header.request_id,
      &service_request,
      &service_response);

    ret = rcl_send_response(service_ptr, &header.request_id, &service_response);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  ASSERT_TRUE(wait_for_client_to_be_ready(&client, context_ptr, 10, 100));
  // Initialize the response owned by the client and take the response.
  type_description_interfaces__srv__GetTypeDescription_Response client_response;
  type_description_interfaces__srv__GetTypeDescription_Response__init(&client_response);

  // Retrieve the response.
  rmw_service_info_t header;
  ret = rcl_take_response_with_info(&client, &header, &client_response);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_EQ(client_response.successful, false);
  EXPECT_GT(strlen(client_response.failure_reason.data), 0);
  EXPECT_EQ(sequence_number, header.request_id.sequence_number);

  type_description_interfaces__srv__GetTypeDescription_Response__fini(&client_response);
}
