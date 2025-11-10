// Copyright 2025 Minju Lee (이민주).
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

#ifndef _WIN32
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <string>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/rcl.h"

#include "rmw/service_endpoint_info_array.h"
#include "rmw/error_handling.h"
#include "wait_for_entity_helpers.hpp"

#include "test_msgs/srv/basic_types.h"
#include "rosidl_runtime_c/string_functions.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"

class TestInfoByServiceFixture : public ::testing::Test
{
public:
  rcl_context_t old_context;
  rcl_context_t context;
  rcl_node_t old_node;
  rcl_node_t node;
  const char * test_graph_node_name = "test_graph_node";
  rmw_service_endpoint_info_array_t service_endpoint_info_array;
  const char * const service_name = "valid_service_name";

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
    this->old_context = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, &this->old_context);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->old_node = rcl_get_zero_initialized_node();
    const char * old_name = "old_node_name";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(&this->old_node, old_name, "", &this->old_context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(&this->old_context);   // after this, the old_node should be invalid
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->context = rcl_get_zero_initialized_context();

    ret = rcl_init(0, nullptr, &init_options, &this->context);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node = rcl_get_zero_initialized_node();
    const char * name = "test_graph_node";
    ret = rcl_node_init(&this->node, name, "", &this->context, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret;
    ret = rcl_node_fini(&this->old_node);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(&this->node);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(&this->context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(&this->context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    // old_context was supposed to have been shutdown already during SetUp()
    if (rcl_context_is_valid(&this->old_context)) {
      ret = rcl_shutdown(&this->old_context);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    ret = rcl_context_fini(&this->old_context);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void assert_qos_equality(
    rmw_qos_profile_t qos_profile1, rmw_qos_profile_t qos_profile2)
  {
    EXPECT_EQ(qos_profile1.deadline.sec, qos_profile2.deadline.sec);
    EXPECT_EQ(qos_profile1.deadline.nsec, qos_profile2.deadline.nsec);

    EXPECT_EQ(qos_profile1.reliability, qos_profile2.reliability);
    EXPECT_EQ(qos_profile1.liveliness, qos_profile2.liveliness);
    EXPECT_EQ(
      qos_profile1.liveliness_lease_duration.sec,
      qos_profile2.liveliness_lease_duration.sec);
    EXPECT_EQ(
      qos_profile1.liveliness_lease_duration.nsec,
      qos_profile2.liveliness_lease_duration.nsec);
    EXPECT_EQ(qos_profile1.durability, qos_profile2.durability);
  }
};

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_info_by_service_null_node)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_clients_info_by_service(
    nullptr, &allocator, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_servers_info_by_service_null_node)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_servers_info_by_service(
    nullptr, &allocator, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_info_by_service_invalid_node)
{
  // this->old_node is an invalid node.
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_clients_info_by_service(
    &this->old_node, &allocator, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_servers_info_by_service_invalid_node)
{
  // this->old_node is an invalid node.
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_servers_info_by_service(
    &this->old_node, &allocator, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_info_by_service_null_allocator)
{
  const auto ret = rcl_get_clients_info_by_service(
    &this->node, nullptr, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_servers_info_by_service_null_allocator)
{
  const auto ret = rcl_get_servers_info_by_service(
    &this->node, nullptr, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_info_by_service_null_service)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_clients_info_by_service(
    &this->node, &allocator, nullptr, false, &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_servers_info_by_service_null_service)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_servers_info_by_service(
    &this->node, &allocator, nullptr, false, &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_info_by_service_null_participants)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_clients_info_by_service(
    &this->node, &allocator, this->service_name, false, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_servers_info_by_service_null_participants)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_servers_info_by_service(
    &this->node, &allocator, this->service_name, false, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_info_by_service_invalid_participants)
{
  // service_endpoint_info_array is invalid because it is expected to be zero initialized
  // and the info_array variable inside it is expected to be null.
  this->service_endpoint_info_array.info_array = new rmw_service_endpoint_info_t();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    delete this->service_endpoint_info_array.info_array;
  });
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_clients_info_by_service(
    &this->node, &allocator, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_ERROR, ret);
  rcl_reset_error();
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(TestInfoByServiceFixture, test_rcl_get_servers_info_by_service_invalid_participants)
{
  // service_endpoint_info_array is invalid because it is expected to be zero initialized
  // and the info_array variable inside it is expected to be null.
  this->service_endpoint_info_array.info_array = new rmw_service_endpoint_info_t();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    delete this->service_endpoint_info_array.info_array;
  });
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_servers_info_by_service(
    &this->node, &allocator, this->service_name, false,
    &this->service_endpoint_info_array);
  EXPECT_EQ(RCL_RET_ERROR, ret);
  rcl_reset_error();
}

TEST_F(TestInfoByServiceFixture, test_rcl_get_clients_server_info_by_service)
{
  rmw_qos_profile_t default_qos_profile = rmw_qos_profile_system_default;
  default_qos_profile.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  default_qos_profile.depth = 0;
  default_qos_profile.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
  default_qos_profile.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
  default_qos_profile.lifespan = {10, 0};
  default_qos_profile.deadline = {11, 0};
  default_qos_profile.liveliness_lease_duration = {20, 0};
  default_qos_profile.liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;

  rcl_ret_t ret;
  const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
    test_msgs, srv, BasicTypes);
  rcl_allocator_t allocator = rcl_get_default_allocator();

  rcl_client_t client = rcl_get_zero_initialized_client();
  rcl_client_options_t client_options = rcl_client_get_default_options();
  client_options.qos = default_qos_profile;
  ret = rcl_client_init(
    &client,
    &this->node,
    ts,
    this->service_name,
    &client_options
  );
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  rcl_service_t server = rcl_get_zero_initialized_service();
  rcl_service_options_t server_options = rcl_service_get_default_options();
  server_options.qos = default_qos_profile;
  ret = rcl_service_init(
    &server,
    &this->node,
    ts,
    this->service_name,
    &server_options
  );
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  const std::string fqdn = std::string("/") + this->service_name;
  // Wait until GraphCache clients are updated
  bool success = false;
  ret = rcl_wait_for_clients(
    &this->node, &allocator, fqdn.c_str(), 1u, RCUTILS_S_TO_NS(1), &success);
  ASSERT_EQ(ret, RCL_RET_OK);
  ASSERT_TRUE(success);
  // Get clients info by service
  rmw_service_endpoint_info_array_t client_endpoint_info_array =
    rmw_get_zero_initialized_service_endpoint_info_array();
  ret = rcl_get_clients_info_by_service(
    &this->node, &allocator, fqdn.c_str(), false,
    &client_endpoint_info_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ASSERT_EQ(client_endpoint_info_array.size, 1u) << "Expected one client";
  rmw_service_endpoint_info_t client_endpoint_info = client_endpoint_info_array.info_array[0];
  EXPECT_STREQ(client_endpoint_info.node_name, this->test_graph_node_name);
  EXPECT_STREQ(client_endpoint_info.node_namespace, "/");
  EXPECT_STREQ(client_endpoint_info.service_type, "test_msgs/srv/BasicTypes");
  EXPECT_EQ(client_endpoint_info.endpoint_type, RMW_ENDPOINT_CLIENT);
  char * type_hash_c_str;
  rosidl_stringify_type_hash(ts->get_type_hash_func(ts), allocator, &type_hash_c_str);
  char * type_hash_c_str_by_client;
  rosidl_stringify_type_hash(
    &client_endpoint_info.service_type_hash, allocator, &type_hash_c_str_by_client);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(allocator.deallocate(type_hash_c_str, &allocator.state));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    allocator.deallocate(type_hash_c_str_by_client, &allocator.state));
  EXPECT_STREQ(type_hash_c_str, type_hash_c_str_by_client);
  EXPECT_TRUE(client_endpoint_info.endpoint_count == 1 || client_endpoint_info.endpoint_count == 2);
  for(size_t i = 0; i < client_endpoint_info.endpoint_count; i++) {
    assert_qos_equality(client_endpoint_info.qos_profiles[i], default_qos_profile);
  }

  // Wait until GraphCache servers are updated
  success = false;
  ret = rcl_wait_for_servers(
    &this->node, &allocator, fqdn.c_str(), 1u, RCUTILS_S_TO_NS(1), &success);
  ASSERT_EQ(ret, RCL_RET_OK);
  ASSERT_TRUE(success);
  // Get subscribers info by service
  rmw_service_endpoint_info_array_t server_endpoint_info_array =
    rmw_get_zero_initialized_service_endpoint_info_array();
  ret = rcl_get_servers_info_by_service(
    &this->node, &allocator, fqdn.c_str(), false,
    &server_endpoint_info_array);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ASSERT_EQ(server_endpoint_info_array.size, 1u) << "Expected one server";
  rmw_service_endpoint_info_t server_endpoint_info =
    server_endpoint_info_array.info_array[0];
  EXPECT_STREQ(server_endpoint_info.node_name, this->test_graph_node_name);
  EXPECT_STREQ(server_endpoint_info.node_namespace, "/");
  EXPECT_STREQ(server_endpoint_info.service_type, "test_msgs/srv/BasicTypes");
  EXPECT_EQ(server_endpoint_info.endpoint_type, RMW_ENDPOINT_SERVER);

  char * type_hash_c_str_by_server;
  rosidl_stringify_type_hash(
    &server_endpoint_info.service_type_hash, allocator, &type_hash_c_str_by_server);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    allocator.deallocate(type_hash_c_str_by_server, &allocator.state));
  EXPECT_STREQ(type_hash_c_str, type_hash_c_str_by_server);
  for(size_t i = 0; i < server_endpoint_info.endpoint_count; i++) {
    assert_qos_equality(server_endpoint_info.qos_profiles[i], default_qos_profile);
  }

  // clean up
  rmw_ret_t rmw_ret =
    rmw_service_endpoint_info_array_fini(&client_endpoint_info_array, &allocator);
  EXPECT_EQ(rmw_ret, RMW_RET_OK) << rmw_get_error_string().str;
  rmw_ret = rmw_service_endpoint_info_array_fini(&server_endpoint_info_array, &allocator);
  EXPECT_EQ(rmw_ret, RMW_RET_OK) << rmw_get_error_string().str;
  ret = rcl_client_fini(&client, &this->node);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ret = rcl_service_fini(&server, &this->node);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
}
