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

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#include "rcl/discovery_options.h"
#include "rcl/rcl.h"

#include "rcutils/allocator.h"
#include "rcutils/env.h"

#include "rmw/discovery_options.h"

#include "../src/rcl/context_impl.h"
#include "../src/rcl/init_options_impl.h"

void check_discovery(
  rmw_automatic_discovery_range_t discovery_range,
  size_t static_peer_count)
{
  rcl_ret_t ret;
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options));
  });
  rcl_context_t context = rcl_get_zero_initialized_context();
  ret = rcl_init(0, nullptr, &init_options, &context);
  ASSERT_EQ(RCL_RET_OK, ret);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    ASSERT_EQ(RCL_RET_OK, rcl_shutdown(&context));
    ASSERT_EQ(RCL_RET_OK, rcl_context_fini(&context));
  });
  rmw_discovery_options_t * discovery_options =
    &context.impl->init_options.impl->rmw_init_options.discovery_options;
  EXPECT_EQ(discovery_range, discovery_options->automatic_discovery_range);
  EXPECT_EQ(static_peer_count, discovery_options->static_peers_count);
}

TEST(TestDiscoveryInfo, test_get_peers) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rmw_discovery_options_t discovery_options_var = rmw_get_zero_initialized_discovery_options();

  // Retrieve peers if peer list is empty
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", ""));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(0u, discovery_options_var.static_peers_count);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list has one IPv4 peer
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(1u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list has one IPv6 peer
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(1u, discovery_options_var.static_peers_count);
  EXPECT_STREQ(
    "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3",
    discovery_options_var.static_peers[0].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list has two IPv4 peers
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;10.0.0.2"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ("10.0.0.2", discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list has one IPv6 peer and one IPv4 peer (order reversed)
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(
    rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ(
    "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3",
    discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list has one IPv6 peer and one IPv4 peer
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(
    rcutils_set_env("ROS_STATIC_PEERS", "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3;192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ(
    "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3",
    discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list has one two IPv4 peers with subnet mask
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "10.1.2.3;192.168.0.0/24"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("10.1.2.3", discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ("192.168.0.0/24", discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peers if peer list is empty
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", ";"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(0u, discovery_options_var.static_peers_count);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peer with trailing ;
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(1u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peer with starting ;
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", ";192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(1u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peer with FQDN
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "example.com"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(1u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("example.com", discovery_options_var.static_peers[0].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  // Retrieve peer with FQDN and IPv4
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "example.com;192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("example.com", discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));
}

TEST(TestDiscoveryInfo, test_get_automatic_discovery_range) {
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", ""));
  rmw_discovery_options_t discovery_options_var = rmw_get_zero_initialized_discovery_options();

  // Set unexpected discovery range. Should default to LOCALHOST
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "0"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(
    RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST,
    discovery_options_var.automatic_discovery_range);

  // Set discovery range to OFF
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "OFF"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(RMW_AUTOMATIC_DISCOVERY_RANGE_OFF, discovery_options_var.automatic_discovery_range);

  // Set discovery range to LOCALHOST
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "LOCALHOST"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(
    RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST,
    discovery_options_var.automatic_discovery_range);

  // Set discovery range to SUBNET
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "SUBNET"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET, discovery_options_var.automatic_discovery_range);

  // Set unexpected discovery range. Should default to LOCALHOST
  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "Unexpected"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(
    RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST,
    discovery_options_var.automatic_discovery_range);
}

TEST(TestDiscoveryInfo, test_bad_argument) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_get_automatic_discovery_range(nullptr));
  rcl_reset_error();
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_get_discovery_static_peers(nullptr, &allocator));
  rcl_reset_error();

  rmw_discovery_options_t discovery_options_var = rmw_get_zero_initialized_discovery_options();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_get_discovery_static_peers(&discovery_options_var, nullptr));
  rcl_reset_error();
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));
}

// Since the two functions operate on the same variable instance, make sure they don't interfere
TEST(TestDiscoveryInfo, test_get_both) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rmw_discovery_options_t discovery_options_var = rmw_get_zero_initialized_discovery_options();

  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", ""));
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "0"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(
    RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST,
    discovery_options_var.automatic_discovery_range);
  EXPECT_EQ(0u, discovery_options_var.static_peers_count);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(
    rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "LOCALHOST"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(
    RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST,
    discovery_options_var.automatic_discovery_range);
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ(
    "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3",
    discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(
    rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "SUBNET"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET, discovery_options_var.automatic_discovery_range);
  EXPECT_EQ(2u, discovery_options_var.static_peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_options_var.static_peers[0].peer_address);
  EXPECT_STREQ(
    "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3",
    discovery_options_var.static_peers[1].peer_address);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));

  discovery_options_var = rmw_get_zero_initialized_discovery_options();
  ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", ""));
  ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "SUBNET"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_automatic_discovery_range(&discovery_options_var));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_static_peers(&discovery_options_var, &allocator));
  EXPECT_EQ(RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET, discovery_options_var.automatic_discovery_range);
  EXPECT_EQ(0u, discovery_options_var.static_peers_count);
  EXPECT_EQ(RCL_RET_OK, rmw_discovery_options_fini(&discovery_options_var));
}

// localhost_only is deprecated but still honored to prevail discovery_options.
// see https://github.com/ros2/ros2_documentation/pull/3519#discussion_r1186541935
// TODO(fujitatomoya): remove localhost_only completely after deprecation period.
TEST(TestDiscoveryInfo, test_with_localhost_only) {
  {
    // No environment variable set (default subnet, no specific peers)
    check_discovery(RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET, 0);
  }

  {
    // only ROS_AUTOMATIC_DISCOVERY_RANGE and ROS_STATIC_PEERS set
    ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "LOCALHOST"));
    ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "127.0.0.1;localhost.com"));
    check_discovery(RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST, 2);
  }

  {
    // Only ROS_LOCALHOST_ONLY is enabled
    ASSERT_TRUE(rcutils_set_env("ROS_LOCALHOST_ONLY", "1"));
    check_discovery(RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST, 0);
  }

  {
    // ROS_LOCALHOST_ONLY is enabled and prevails over SUBNET.
    ASSERT_TRUE(rcutils_set_env("ROS_LOCALHOST_ONLY", "1"));
    ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "SUBNET"));
    ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;remote.com"));
    check_discovery(RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST, 0);
  }

  {
    // ROS_LOCALHOST_ONLY is enabled and prevails over OFF.
    ASSERT_TRUE(rcutils_set_env("ROS_LOCALHOST_ONLY", "1"));
    ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "OFF"));
    check_discovery(RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST, 0);
  }

  {
    // ROS_LOCALHOST_ONLY is disabled, falls down to use discovery option.
    ASSERT_TRUE(rcutils_set_env("ROS_LOCALHOST_ONLY", "0"));
    ASSERT_TRUE(rcutils_set_env("ROS_AUTOMATIC_DISCOVERY_RANGE", "SUBNET"));
    ASSERT_TRUE(rcutils_set_env("ROS_STATIC_PEERS", "192.168.0.1;remote.com"));
    check_discovery(RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET, 2);
  }
}
