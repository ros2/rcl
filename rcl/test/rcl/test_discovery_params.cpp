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
#include "rcl/discovery_params.h"
#include "rmw/discovery_params.h"
#include "rcutils/env.h"

TEST(TestDiscoveryInfo, test_get_peers) {
  rmw_discovery_params_t discovery_params_var = rmw_get_zero_initialized_discovery_params();

  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", ""));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(0u, discovery_params_var.peers_count);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(1u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]) << discovery_params_var.peers[0];

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(1u, discovery_params_var.peers_count);
  EXPECT_STREQ("ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3", discovery_params_var.peers[0]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "192.168.0.1;10.0.0.2"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]);
  EXPECT_STREQ("10.0.0.2", discovery_params_var.peers[1]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(
    rcutils_set_env("ROS_PEERS", "192.168.0.1;ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]);
  EXPECT_STREQ("ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3", discovery_params_var.peers[1]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(
    rcutils_set_env("ROS_PEERS", "ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3;192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3", discovery_params_var.peers[0]);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[1]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "10.1.2.3;192.168.0.0/24"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("10.1.2.3", discovery_params_var.peers[0]);
  EXPECT_STREQ("192.168.0.0/24", discovery_params_var.peers[1]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", ";"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(0u, discovery_params_var.peers_count);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "192.168.0.1;"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(1u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", ";192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(1u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "example.com"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(1u, discovery_params_var.peers_count);
  EXPECT_STREQ("example.com", discovery_params_var.peers[0]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", "example.com;192.168.0.1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("example.com", discovery_params_var.peers[0]);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[1]);
}

TEST(TestDiscoveryInfo, test_get_multicast) {
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", ""));
  rmw_discovery_params_t discovery_params_var = rmw_get_zero_initialized_discovery_params();

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "0"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_DISABLED, discovery_params_var.use_multicast);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_ENABLED, discovery_params_var.use_multicast);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "2"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_DISABLED, discovery_params_var.use_multicast);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "Unexpected"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_DISABLED, discovery_params_var.use_multicast);
}

TEST(TestDiscoveryInfo, test_bad_argument) {
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rcl_get_discovery_params(nullptr));
}

TEST(TestDiscoveryInfo, test_get_both) {
  rmw_discovery_params_t discovery_params_var = rmw_get_zero_initialized_discovery_params();

  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", ""));
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "0"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_DISABLED, discovery_params_var.use_multicast);
  EXPECT_EQ(0u, discovery_params_var.peers_count);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(
    rcutils_set_env("ROS_PEERS", "192.168.0.1;ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "0"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_DISABLED, discovery_params_var.use_multicast);
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]);
  EXPECT_STREQ("ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3", discovery_params_var.peers[1]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(
    rcutils_set_env("ROS_PEERS", "192.168.0.1;ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3"));
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_ENABLED, discovery_params_var.use_multicast);
  EXPECT_EQ(2u, discovery_params_var.peers_count);
  EXPECT_STREQ("192.168.0.1", discovery_params_var.peers[0]);
  EXPECT_STREQ("ceab:78ee:b73a:ec05:0898:0b2c:5ce5:8ed3", discovery_params_var.peers[1]);

  discovery_params_var = rmw_get_zero_initialized_discovery_params();
  ASSERT_TRUE(rcutils_set_env("ROS_PEERS", ""));
  ASSERT_TRUE(rcutils_set_env("ROS_USE_MULTICAST_DISCOVERY", "1"));
  EXPECT_EQ(RCL_RET_OK, rcl_get_discovery_params(&discovery_params_var));
  EXPECT_EQ(RMW_MULTICAST_DISCOVERY_ENABLED, discovery_params_var.use_multicast);
  EXPECT_EQ(0u, discovery_params_var.peers_count);
}
