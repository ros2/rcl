// Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <memory>
#include <string>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/rcl.h"

#include "rmw/topic_info_array.h"
#include "rmw/error_handling.h"

#include "test_msgs/msg/strings.h"
#include "rosidl_generator_c/string_functions.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestInfoByTopicFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_context_t * old_context_ptr;
  rcl_context_t * context_ptr;
  rcl_node_t * old_node_ptr;
  rcl_node_t * node_ptr;
  const char * test_graph_node_name = "test_graph_node";
  std::unique_ptr<rmw_topic_info_array_t> topic_info_array;
  const char * const topic_name = "valid_topic_name";
  bool is_fastrtps;
  void SetUp()
  {
    is_fastrtps = (std::string(rmw_get_implementation_identifier()).find("rmw_fastrtps") == 0);
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });
    this->old_context_ptr = new rcl_context_t;
    *this->old_context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->old_context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->old_node_ptr = new rcl_node_t;
    *this->old_node_ptr = rcl_get_zero_initialized_node();
    const char * old_name = "old_node_name";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->old_node_ptr, old_name, "", this->old_context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(this->old_context_ptr);  // after this, the old_node_ptr should be invalid
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();

    ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_graph_node";
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->topic_info_array = std::make_unique<rmw_topic_info_array_t>();
  }

  void TearDown()
  {
    rcl_ret_t ret;
    ret = rcl_node_fini(this->old_node_ptr);
    delete this->old_node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->context_ptr;
    ret = rcl_context_fini(this->old_context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->old_context_ptr;
  }

  void assert_qos_equality(rmw_qos_profile_t qos_profile1, rmw_qos_profile_t qos_profile2)
  {
    EXPECT_EQ(qos_profile1.deadline.sec, qos_profile2.deadline.sec);
    EXPECT_EQ(qos_profile1.deadline.nsec, qos_profile2.deadline.nsec);
    EXPECT_EQ(qos_profile1.lifespan.sec, qos_profile2.lifespan.sec);
    EXPECT_EQ(qos_profile1.lifespan.nsec, qos_profile2.lifespan.nsec);
    EXPECT_EQ(qos_profile1.reliability, qos_profile2.reliability);
    EXPECT_EQ(qos_profile1.liveliness, qos_profile2.liveliness);
    EXPECT_EQ(qos_profile1.liveliness_lease_duration.sec,
      qos_profile2.liveliness_lease_duration.sec);
    EXPECT_EQ(qos_profile1.liveliness_lease_duration.nsec,
      qos_profile2.liveliness_lease_duration.nsec);
    EXPECT_EQ(qos_profile1.durability, qos_profile2.durability);
  }
};

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_info_by_topic_null_node)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_publishers_info_by_topic(nullptr,
      &allocator, this->topic_name, false, this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriptions_info_by_topic_null_node)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_subscriptions_info_by_topic(nullptr,
      &allocator, this->topic_name, false, this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_info_by_topic_invalid_node)
{
  // this->old_node_ptr is a pointer to an invalid node.
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_publishers_info_by_topic(this->old_node_ptr,
      &allocator, this->topic_name, false, this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriptions_info_by_topic_invalid_node)
{
  // this->old_node_ptr is a pointer to an invalid node.
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_subscriptions_info_by_topic(this->old_node_ptr,
      &allocator, this->topic_name, false, this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_info_by_topic_null_allocator)
{
  const auto ret = rcl_get_publishers_info_by_topic(this->node_ptr, nullptr, this->topic_name,
      false,
      this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriptions_info_by_topic_null_allocator)
{
  const auto ret = rcl_get_subscriptions_info_by_topic(this->node_ptr, nullptr, this->topic_name,
      false,
      this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_info_by_topic_null_topic)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_publishers_info_by_topic(this->node_ptr,
      &allocator, nullptr, false, this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriptions_info_by_topic_null_topic)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_subscriptions_info_by_topic(this->node_ptr,
      &allocator, nullptr, false, this->topic_info_array.get());
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_info_by_topic_null_participants)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_publishers_info_by_topic(this->node_ptr,
      &allocator, this->topic_name, false, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriptions_info_by_topic_null_participants)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_subscriptions_info_by_topic(this->node_ptr,
      &allocator, this->topic_name, false, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_info_by_topic_invalid_participants)
{
  // temp_info_array is invalid because it is expected to be zero initialized
  // and the info_array variable inside it is expected to be null.
  const auto & temp_info_array = this->topic_info_array.get();
  temp_info_array->info_array =
    reinterpret_cast<rmw_topic_info_t *>(calloc(1, sizeof(rmw_topic_info_t)));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    free(temp_info_array->info_array);
  });
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_publishers_info_by_topic(this->node_ptr,
      &allocator, this->topic_name, false, temp_info_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

/*
 * This does not test content of the response.
 * It only tests if the return code is the one expected.
 */
TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_subscriptions_info_by_topic_invalid_participants)
{
  // temp_info_array is invalid because it is expected to be zero initialized
  // and the info_array variable inside it is expected to be null.
  const auto & temp_info_array = this->topic_info_array.get();
  temp_info_array->info_array =
    reinterpret_cast<rmw_topic_info_t *>(calloc(1, sizeof(rmw_topic_info_t)));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    free(temp_info_array->info_array);
  });
  rcl_allocator_t allocator = rcl_get_default_allocator();
  const auto ret = rcl_get_subscriptions_info_by_topic(this->node_ptr,
      &allocator, this->topic_name, false, temp_info_array);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
}

TEST_F(CLASSNAME(TestInfoByTopicFixture, RMW_IMPLEMENTATION),
  test_rcl_get_publishers_subscription_info_by_topic)
{
  // This is implemented only in fastrtps currently.
  if (!is_fastrtps) {
    GTEST_SKIP();
  }
  rmw_qos_profile_t default_qos_profile;
  default_qos_profile.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  default_qos_profile.depth = 0;
  default_qos_profile.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
  default_qos_profile.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
  default_qos_profile.lifespan = {10, 0};
  default_qos_profile.deadline = {11, 0};
  default_qos_profile.liveliness_lease_duration = {20, 0};
  default_qos_profile.liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;

  rcl_ret_t ret;
  const rosidl_message_type_support_t * ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Strings);
  rcl_allocator_t allocator = rcl_get_default_allocator();

  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  publisher_options.qos = default_qos_profile;
  ret = rcl_publisher_init(
    &publisher,
    this->node_ptr,
    ts,
    this->topic_name,
    &publisher_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;

  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  subscription_options.qos = default_qos_profile;
  ret = rcl_subscription_init(
    &subscription,
    this->node_ptr,
    ts,
    this->topic_name,
    &subscription_options);
  ASSERT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  std::string fqdn = this->topic_name;
  fqdn = "/" + fqdn;
  // Get publishers info by topic
  rmw_topic_info_array_t topic_info_array_pub = rmw_get_zero_initialized_topic_info_array();
  ret = rcl_get_publishers_info_by_topic(this->node_ptr,
      &allocator, fqdn.c_str(), false, &topic_info_array_pub);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_EQ(topic_info_array_pub.count, 1u) << "Expected one publisher";
  rmw_topic_info_t topic_info_pub = topic_info_array_pub.info_array[0];
  EXPECT_STREQ(topic_info_pub.node_name, this->test_graph_node_name);
  EXPECT_STREQ(topic_info_pub.node_namespace, "/");
  EXPECT_STREQ(topic_info_pub.topic_type, "test_msgs::msg::dds_::Strings_");
  assert_qos_equality(topic_info_pub.qos_profile, default_qos_profile);

  rmw_topic_info_array_t topic_info_array_sub = rmw_get_zero_initialized_topic_info_array();
  ret = rcl_get_subscriptions_info_by_topic(this->node_ptr,
      &allocator, fqdn.c_str(), false, &topic_info_array_sub);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  EXPECT_EQ(topic_info_array_sub.count, 1u) << "Expected one subscription";
  rmw_topic_info_t topic_info_sub = topic_info_array_sub.info_array[0];
  EXPECT_STREQ(topic_info_sub.node_name, this->test_graph_node_name);
  EXPECT_STREQ(topic_info_sub.node_namespace, "/");
  EXPECT_STREQ(topic_info_sub.topic_type, "test_msgs::msg::dds_::Strings_");
  assert_qos_equality(topic_info_sub.qos_profile, default_qos_profile);

  // clean up
  rmw_ret_t rmw_ret = rmw_topic_info_array_fini(&topic_info_array_pub, &allocator);
  EXPECT_EQ(rmw_ret, RMW_RET_OK) << rmw_get_error_string().str;
  rmw_ret = rmw_topic_info_array_fini(&topic_info_array_sub, &allocator);
  EXPECT_EQ(rmw_ret, RMW_RET_OK) << rmw_get_error_string().str;
  ret = rcl_subscription_fini(&subscription, this->node_ptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
  ret = rcl_publisher_fini(&publisher, this->node_ptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string().str;
}
