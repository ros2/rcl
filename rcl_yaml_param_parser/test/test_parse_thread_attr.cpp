// Copyright 2023 eSOL Co.,Ltd.
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

#include <yaml.h>
#include <iostream>
#include <string>
#include <sstream>

#include "rcutils/thread_attr.h"
#include "rcutils/error_handling.h"

#include "../src/impl/parse_thread_attr.h"

struct TestParseThreadAttrs : testing::Test
{
  void SetUp() override
  {
    rcutils_reset_error();

    rcutils_ret_t ret;
    rcutils_allocator_t allocator;
    attrs = rcutils_get_zero_initialized_thread_attrs();
    allocator = rcutils_get_default_allocator();
    ret = rcutils_thread_attrs_init(&attrs, allocator);
    ASSERT_EQ(RCUTILS_RET_OK, ret);

    int parser_ret = yaml_parser_initialize(&parser);
    ASSERT_NE(0, parser_ret);
  }
  void TearDown() override
  {
    yaml_parser_delete(&parser);
    rcutils_ret_t ret = rcutils_thread_attrs_fini(&attrs);
    ASSERT_EQ(RCUTILS_RET_OK, ret);
  }

  void prepare_yaml_parser(char const * yaml_value)
  {
    yaml_parser_set_input_string(&parser, (const unsigned char *)yaml_value, strlen(yaml_value));
  }

  yaml_parser_t parser;
  rcutils_thread_attrs_t attrs;
};

struct TestParseThreadAttr : TestParseThreadAttrs
{
  void prepare_yaml_parser(char const * yaml_value)
  {
    TestParseThreadAttrs::prepare_yaml_parser(yaml_value);

    yaml_event_t event;
    int ret;
    ret = yaml_parser_parse(&parser, &event);
    ASSERT_NE(0, ret);
    ret = yaml_parser_parse(&parser, &event);
    ASSERT_NE(0, ret);
    ret = yaml_parser_parse(&parser, &event);
    ASSERT_NE(0, ret);
  }
};

TEST_F(TestParseThreadAttr, success) {
  rcutils_ret_t ret;
  yaml_event_t event;

  prepare_yaml_parser(
    "{ priority: 10, name: thread-1, core_affinity: [1], scheduling_policy: FIFO }");

  ret = parse_thread_attr(&parser, &attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);

  EXPECT_EQ(1, attrs.num_attributes);
  EXPECT_EQ(10, attrs.attributes[0].priority);
  EXPECT_STREQ("thread-1", attrs.attributes[0].name);
  EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[0].core_affinity, 1));
  EXPECT_EQ(RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, attrs.attributes[0].scheduling_policy);

  int parser_ret;
  parser_ret = yaml_parser_parse(&parser, &event);
  ASSERT_NE(0, parser_ret);
  EXPECT_EQ(YAML_DOCUMENT_END_EVENT, event.type);
  parser_ret = yaml_parser_parse(&parser, &event);
  ASSERT_NE(0, parser_ret);
  EXPECT_EQ(YAML_STREAM_END_EVENT, event.type);
}

TEST_F(TestParseThreadAttr, unknown_key) {
  rcutils_ret_t ret;

  prepare_yaml_parser(
    "{ priority: 10, name: thread-1, core_affinity: [1], unknown_key: FIFO }");

  ret = parse_thread_attr(&parser, &attrs);
  EXPECT_EQ(RCUTILS_RET_ERROR, ret);
}

TEST_F(TestParseThreadAttr, all_valid_keys_with_unknown_key) {
  rcutils_ret_t ret;

  prepare_yaml_parser(
    "{ priority: 10, name: thread-1, core_affinity: [1], "
    "scheduling_policy: FIFO, unknown_key: RR }");

  ret = parse_thread_attr(&parser, &attrs);
  EXPECT_EQ(RCUTILS_RET_ERROR, ret);
}

TEST_F(TestParseThreadAttr, missing_key_value) {
  rcutils_ret_t ret;
  prepare_yaml_parser(
    "{ priority: 10, name: thread-1 }");

  ret = parse_thread_attr(&parser, &attrs);
  EXPECT_EQ(RCUTILS_RET_ERROR, ret);
}

TEST_F(TestParseThreadAttrs, success) {
  rcutils_ret_t ret;

  std::stringstream ss;
  ss << "[";
  for (std::size_t i = 0; i < 100; ++i) {
    ss << "{ priority: " << i * 10;
    ss << ", name: thread-" << i;
    ss << ", core_affinity: [" << i << "]";
    ss << ", scheduling_policy: FIFO },";
  }
  ss << "]";

  std::string buf = ss.str();
  prepare_yaml_parser(buf.c_str());

  ret = parse_thread_attr_events(&parser, &attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_EQ(100, attrs.num_attributes);

  for (std::size_t i = 0; i < 100; ++i) {
    EXPECT_EQ(i * 10, attrs.attributes[i].priority);
    ss.str("");
    ss << "thread-" << i;
    buf = ss.str();
    EXPECT_STREQ(buf.c_str(), attrs.attributes[i].name);
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i));
    EXPECT_EQ(RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, attrs.attributes[i].scheduling_policy);
  }
}

TEST_F(TestParseThreadAttr, affinity_multiple_core) {
  rcutils_ret_t ret;
  prepare_yaml_parser(
    "{ priority: 10, name: thread-1, core_affinity: [1,2,3], scheduling_policy: FIFO }");

  ret = parse_thread_attr(&parser, &attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_FALSE(rcutils_thread_core_affinity_is_set(&attrs.attributes[0].core_affinity, 0));
  EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[0].core_affinity, 1));
  EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[0].core_affinity, 2));
  EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[0].core_affinity, 3));
  EXPECT_FALSE(rcutils_thread_core_affinity_is_set(&attrs.attributes[0].core_affinity, 4));
}
