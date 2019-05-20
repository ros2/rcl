// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "rcl/expand_topic_name.h"

#include "rcl/error_handling.h"

using namespace std::string_literals;

TEST(test_expand_topic_name, normal) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_string_map_t subs = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcu_ret = rcutils_string_map_init(&subs, 0, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, rcu_ret);
  ret = rcl_get_default_topic_name_substitutions(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);

  // {node}/chatter example
  {
    const char * topic = "{node}/chatter";
    const char * ns = "/my_ns";
    const char * node = "my_node";
    std::string expected = std::string(ns) + "/" + node + "/chatter";
    char * expanded_topic;
    ret = rcl_expand_topic_name(topic, node, ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    EXPECT_STREQ(expected.c_str(), expanded_topic);
    allocator.deallocate(expanded_topic, allocator.state);
  }

  ret = rcutils_string_map_fini(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);
}

TEST(test_expand_topic_name, invalid_arguments) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_string_map_t subs = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcu_ret = rcutils_string_map_init(&subs, 0, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, rcu_ret);
  ret = rcl_get_default_topic_name_substitutions(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);

  const char * topic = "{node}/chatter";
  const char * ns = "/my_ns";
  const char * node = "my_node";
  char * expanded_topic;

  // pass null for topic string
  {
    ret = rcl_expand_topic_name(NULL, node, ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
    rcl_reset_error();
  }

  // pass null for node name
  {
    ret = rcl_expand_topic_name(topic, NULL, ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
    rcl_reset_error();
  }

  // pass null for node namespace
  {
    ret = rcl_expand_topic_name(topic, node, NULL, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
    rcl_reset_error();
  }

  // pass null for subs
  {
    ret = rcl_expand_topic_name(topic, node, ns, NULL, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
    rcl_reset_error();
  }

  // pass null for expanded_topic
  {
    ret = rcl_expand_topic_name(topic, node, ns, &subs, allocator, NULL);
    EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
    rcl_reset_error();
  }

  // pass invalid topic
  {
    ret = rcl_expand_topic_name("white space", node, ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_TOPIC_NAME_INVALID, ret);
    rcl_reset_error();
  }

  // pass invalid node name
  {
    ret = rcl_expand_topic_name(topic, "/invalid_node", ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_NODE_INVALID_NAME, ret);
    rcl_reset_error();
  }

  // pass invalid node namespace
  {
    ret = rcl_expand_topic_name(topic, node, "white space", &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_NODE_INVALID_NAMESPACE, ret);
    rcl_reset_error();
  }

  ret = rcutils_string_map_fini(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);
}

TEST(test_expand_topic_name, various_valid_topics) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_string_map_t subs = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcu_ret = rcutils_string_map_init(&subs, 0, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, rcu_ret);
  ret = rcl_get_default_topic_name_substitutions(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);

  std::vector<std::vector<std::string>> topics_that_should_expand_to = {
    // {"input_topic", "node_name", "/namespace", "expected result"},
    {"/chatter", "my_node", "/my_ns", "/chatter"},
    {"chatter", "my_node", "/my_ns", "/my_ns/chatter"},
    {"{node}/chatter", "my_node", "/my_ns", "/my_ns/my_node/chatter"},
    {"/{node}", "my_node", "/my_ns", "/my_node"},
    {"{node}", "my_node", "/my_ns", "/my_ns/my_node"},
    {"{ns}", "my_node", "/my_ns", "/my_ns"},
    {"{namespace}", "my_node", "/my_ns", "/my_ns"},
    {"{namespace}/{node}/chatter", "my_node", "/my_ns", "/my_ns/my_node/chatter"},

    // this one will produce an invalid topic, but will pass
    // the '//' should be caught by the rmw_validate_full_topic_name() function
    {"/foo/{namespace}", "my_node", "/my_ns", "/foo//my_ns"},

    // examples from the design doc:
    //   http://design.ros2.org/articles/topic_and_service_names.html
    // the node constructor would make the "" namespace into "/" implicitly
    {"ping", "my_node", "/", "/ping"},
    {"ping", "my_node", "/my_ns", "/my_ns/ping"},

    {"/ping", "my_node", "/", "/ping"},
    {"/ping", "my_node", "/my_ns", "/ping"},

    {"~", "my_node", "/", "/my_node"},
    {"~", "my_node", "/my_ns", "/my_ns/my_node"},

    {"~/ping", "my_node", "/", "/my_node/ping"},
    {"~/ping", "my_node", "/my_ns", "/my_ns/my_node/ping"},
  };

  for (const auto & inout : topics_that_should_expand_to) {
    const char * topic = inout.at(0).c_str();
    const char * node = inout.at(1).c_str();
    const char * ns = inout.at(2).c_str();
    std::string expected = inout.at(3);
    char * expanded_topic;
    ret = rcl_expand_topic_name(topic, node, ns, &subs, allocator, &expanded_topic);
    std::stringstream ss;
    ss <<
      "^ While expanding '" << topic <<
      "' with node '" << node <<
      "' and namespace '" << ns << "'";
    EXPECT_EQ(RCL_RET_OK, ret) <<
      ss.str() <<
      ", it failed with '" << ret << "': " << rcl_get_error_string().str;
    EXPECT_STREQ(expected.c_str(), expanded_topic) << ss.str() << " strings did not match.\n";
    allocator.deallocate(expanded_topic, allocator.state);
  }

  ret = rcutils_string_map_fini(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);
}

TEST(test_expand_topic_name, unknown_substitution) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_string_map_t subs = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcu_ret = rcutils_string_map_init(&subs, 0, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, rcu_ret);
  ret = rcl_get_default_topic_name_substitutions(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);

  {
    const char * topic = "{doesnotexist}";
    const char * ns = "/my_ns";
    const char * node = "my_node";
    char * expanded_topic;
    ret = rcl_expand_topic_name(topic, node, ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_UNKNOWN_SUBSTITUTION, ret);
    rcl_reset_error();
    EXPECT_EQ(NULL, expanded_topic);
    allocator.deallocate(expanded_topic, allocator.state);
  }

  ret = rcutils_string_map_fini(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);
}

TEST(test_expand_topic_name, custom_substitution) {
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_string_map_t subs = rcutils_get_zero_initialized_string_map();
  rcutils_ret_t rcu_ret = rcutils_string_map_init(&subs, 0, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, rcu_ret);
  ret = rcl_get_default_topic_name_substitutions(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);

  rcu_ret = rcutils_string_map_set(&subs, "ping", "pong");
  ASSERT_EQ(RCUTILS_RET_OK, rcu_ret);

  {
    const char * topic = "{ping}";
    const char * ns = "/my_ns";
    const char * node = "my_node";
    char * expanded_topic;
    ret = rcl_expand_topic_name(topic, node, ns, &subs, allocator, &expanded_topic);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    EXPECT_STREQ("/my_ns/pong", expanded_topic);
    allocator.deallocate(expanded_topic, allocator.state);
  }

  ret = rcutils_string_map_fini(&subs);
  ASSERT_EQ(RCL_RET_OK, ret);
}
