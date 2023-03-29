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

#include "rcl/rcl.h"
#include "rcl/discovery_options.h"

#include "rcutils/allocator.h"
#include "rcutils/env.h"

#include "rmw/discovery_options.h"

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
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_get_discovery_static_peers(nullptr, &allocator));

  rmw_discovery_options_t discovery_options_var = rmw_get_zero_initialized_discovery_options();
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_get_discovery_static_peers(&discovery_options_var, nullptr));
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
