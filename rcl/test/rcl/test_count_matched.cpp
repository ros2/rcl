// Copyright 2018 Open Source Robotics Foundation, Inc.
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
#include <cstddef>
#include <string>
#include <thread>

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#include "rcl/rcl.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"

#include "rcutils/logging_macros.h"

#include "test_msgs/msg/basic_types.h"
#include "test_msgs/srv/basic_types.h"

#include "rcl/error_handling.h"

class TestCountFixture : public ::testing::Test
{
public:
  rcl_node_t * node_ptr;

  void SetUp()
  {
    rcl_ret_t ret;
    rcl_node_options_t node_options = rcl_node_get_default_options();

    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_init_options_fini(&init_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_count_node";
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->wait_set_ptr = new rcl_wait_set_t;
    *this->wait_set_ptr = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(
      this->wait_set_ptr, 0, 1, 0, 0, 0, 0, this->context_ptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    graph_guard_condition = rcl_node_get_graph_guard_condition(this->node_ptr);
  }

  void TearDown()
  {
    rcl_ret_t ret;

    ret = rcl_wait_set_fini(this->wait_set_ptr);
    delete this->wait_set_ptr;
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
  const rcl_guard_condition_t * graph_guard_condition;
  rcl_context_t * context_ptr;
  rcl_wait_set_t * wait_set_ptr;

  void check_state(
    rcl_publisher_t * publisher,
    rcl_subscription_t * subscriber,
    size_t expected_subscriber_count,
    size_t expected_publisher_count,
    size_t number_of_tries)
  {
    size_t subscriber_count = -1;
    size_t publisher_count = -1;

    rcl_ret_t ret;

    for (size_t i = 0; i < number_of_tries; ++i) {
      if (publisher) {
        ret = rcl_publisher_get_subscription_count(publisher, &subscriber_count);
        EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
        rcl_reset_error();
      }

      if (subscriber) {
        ret = rcl_subscription_get_publisher_count(subscriber, &publisher_count);
        EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
        rcl_reset_error();
      }

      if (
        expected_publisher_count == publisher_count &&
        expected_subscriber_count == subscriber_count)
      {
        RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "state correct!");
        break;
      }

      if ((i + 1) == number_of_tries) {
        // Don't wait for the graph to change on the last loop because we won't check again.
        continue;
      }

      ret = rcl_wait_set_clear(wait_set_ptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      ret = rcl_wait_set_add_guard_condition(wait_set_ptr, graph_guard_condition, NULL);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(200);
      RCUTILS_LOG_INFO_NAMED(
        ROS_PACKAGE_NAME,
        "state wrong, waiting up to '%s' nanoseconds for graph changes... ",
        std::to_string(time_to_sleep.count()).c_str());
      ret = rcl_wait(wait_set_ptr, time_to_sleep.count());
      if (ret == RCL_RET_TIMEOUT) {
        RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "timeout");
        continue;
      }
      RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "change occurred");
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    EXPECT_EQ(expected_publisher_count, publisher_count);
    EXPECT_EQ(expected_subscriber_count, subscriber_count);
  }
};

TEST_F(TestCountFixture, test_count_matched_functions)
{
  std::string topic_name("/test_count_matched_functions__");
  rcl_ret_t ret;

  rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t pub_opts = rcl_publisher_get_default_options();
  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  ret = rcl_publisher_init(&pub, this->node_ptr, ts, topic_name.c_str(), &pub_opts);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  check_state(&pub, nullptr, 0, -1, 9);

  rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_opts = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub, this->node_ptr, ts, topic_name.c_str(), &sub_opts);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  check_state(&pub, &sub, 1, 1, 9);

  rcl_subscription_t sub2 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub2_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub2, this->node_ptr, ts, topic_name.c_str(), &sub2_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  check_state(&pub, &sub, 2, 1, 9);
  check_state(&pub, &sub2, 2, 1, 9);

  ret = rcl_publisher_fini(&pub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  check_state(nullptr, &sub, -1, 0, 9);
  check_state(nullptr, &sub2, -1, 0, 9);

  ret = rcl_subscription_fini(&sub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  ret = rcl_subscription_fini(&sub2, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
}

TEST_F(TestCountFixture, test_count_matched_functions_mismatched_qos)
{
  std::string topic_name("/test_count_matched_functions_mismatched_qos__");
  rcl_ret_t ret;

  rcl_publisher_t pub = rcl_get_zero_initialized_publisher();

  rcl_publisher_options_t pub_opts = rcl_publisher_get_default_options();
  pub_opts.qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  pub_opts.qos.depth = 10;
  pub_opts.qos.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
  pub_opts.qos.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
  pub_opts.qos.avoid_ros_namespace_conventions = false;
  pub_opts.allocator = rcl_get_default_allocator();

  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  ret = rcl_publisher_init(&pub, this->node_ptr, ts, topic_name.c_str(), &pub_opts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCL_RET_OK, rcl_publisher_fini(&pub, this->node_ptr)) << rcl_get_error_string().str;
  });

  check_state(&pub, nullptr, 0, -1, 9);

  rcl_subscription_t sub = rcl_get_zero_initialized_subscription();

  rcl_subscription_options_t sub_opts = rcl_subscription_get_default_options();
  sub_opts.qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  sub_opts.qos.depth = 10;
  sub_opts.qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
  sub_opts.qos.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
  sub_opts.qos.avoid_ros_namespace_conventions = false;
  sub_opts.allocator = rcl_get_default_allocator();

  ret = rcl_subscription_init(&sub, this->node_ptr, ts, topic_name.c_str(), &sub_opts);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCL_RET_OK,
      rcl_subscription_fini(&sub, this->node_ptr)) << rcl_get_error_string().str;
  });

  rmw_qos_compatibility_type_t compat;
  rmw_ret_t rmw_ret =
    rmw_qos_profile_check_compatible(pub_opts.qos, sub_opts.qos, &compat, nullptr, 0);
  ASSERT_EQ(rmw_ret, RMW_RET_OK);

  if (compat == RMW_QOS_COMPATIBILITY_OK) {
    check_state(&pub, &sub, 1, 1, 9);
  } else {
    // Expect that no publishers or subscribers should be matched due to qos.
    check_state(&pub, &sub, 0, 0, 9);
  }

  rcl_subscription_t sub2 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub2_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub2, this->node_ptr, ts, topic_name.c_str(), &sub2_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCL_RET_OK,
      rcl_subscription_fini(&sub2, this->node_ptr)) << rcl_get_error_string().str;
  });

  if (compat == RMW_QOS_COMPATIBILITY_OK) {
    check_state(&pub, &sub, 2, 1, 9);
    check_state(&pub, &sub2, 2, 1, 9);
  } else {
    // Even multiple subscribers should not match
    check_state(&pub, &sub, 0, 0, 9);
    check_state(&pub, &sub2, 0, 0, 9);
  }
}
