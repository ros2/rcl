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
#include <fstream>
#include <sstream>

#include "rcutils/error_handling.h"
#include "rcutils/filesystem.h"
#include "rcutils/thread_attr.h"

#include "rcl_yaml_param_parser/parser_thread_attr.h"

struct TestParserThreadAttr : testing::Test
{
  void SetUp() override
  {
    rcutils_ret_t ret;
    path = nullptr;

    rcutils_reset_error();
    attrs = rcutils_get_zero_initialized_thread_attrs();
    alloc = rcutils_get_default_allocator();
    ret = rcutils_thread_attrs_init(&attrs, alloc);
    ASSERT_EQ(RCUTILS_RET_OK, ret);
  }

  void prepare_thread_attr_file(char const * filename)
  {
    char buf[1024];
    bool ret = rcutils_get_cwd(buf, sizeof(buf));
    ASSERT_TRUE(ret);

    path = rcutils_join_path(buf, "test/", alloc);
    ASSERT_NE(nullptr, path);
    path = rcutils_join_path(path, filename, alloc);
    ASSERT_NE(nullptr, path);
  }

  void TearDown() override
  {
    rcutils_ret_t ret;
    if (path) {
      alloc.deallocate(path, alloc.state);
    }
    ret = rcutils_thread_attrs_fini(&attrs);
    EXPECT_EQ(RCUTILS_RET_OK, ret);
  }
  rcutils_thread_attrs_t attrs;
  rcutils_allocator_t alloc;
  char * path;
};

static const rcutils_thread_scheduling_policy_t expected_policies[] = {
  RCUTILS_THREAD_SCHEDULING_POLICY_UNKNOWN,
  RCUTILS_THREAD_SCHEDULING_POLICY_FIFO,
  RCUTILS_THREAD_SCHEDULING_POLICY_RR,
  RCUTILS_THREAD_SCHEDULING_POLICY_SPORADIC,
  RCUTILS_THREAD_SCHEDULING_POLICY_OTHER,
  RCUTILS_THREAD_SCHEDULING_POLICY_IDLE,
  RCUTILS_THREAD_SCHEDULING_POLICY_BATCH,
  RCUTILS_THREAD_SCHEDULING_POLICY_DEADLINE,
  RCUTILS_THREAD_SCHEDULING_POLICY_UNKNOWN,
  RCUTILS_THREAD_SCHEDULING_POLICY_FIFO,
};

TEST_F(TestParserThreadAttr, success_file) {
  rcutils_ret_t ret;

  prepare_thread_attr_file("thread_attr_success.yaml");

  ret = rcl_parse_yaml_thread_attrs_file(path, &attrs);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(10, attrs.num_attributes);

  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(attrs.attributes[i].priority, i * 10);
    char buf[32];
    snprintf(buf, sizeof(buf), "attr-%lu", i);
    EXPECT_STREQ(buf, attrs.attributes[i].tag);
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i));
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i + 10));
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i * i));
    EXPECT_EQ(expected_policies[i], attrs.attributes[i].scheduling_policy);
  }
}

TEST_F(TestParserThreadAttr, success_value) {
  rcutils_ret_t ret;

  prepare_thread_attr_file("thread_attr_success.yaml");

  std::ifstream ifs(path);
  std::stringstream ss;
  ss << ifs.rdbuf();

  ret = rcl_parse_yaml_thread_attrs_value(ss.str().c_str(), &attrs);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(10, attrs.num_attributes);

  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(attrs.attributes[i].priority, i * 10);
    char buf[32];
    snprintf(buf, sizeof(buf), "attr-%lu", i);
    EXPECT_STREQ(buf, attrs.attributes[i].tag);
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i));
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i + 10));
    EXPECT_TRUE(rcutils_thread_core_affinity_is_set(&attrs.attributes[i].core_affinity, i * i));
    EXPECT_EQ(expected_policies[i], attrs.attributes[i].scheduling_policy);
  }
}

TEST_F(TestParserThreadAttr, bad_file_path) {
  rcutils_ret_t ret = rcl_parse_yaml_thread_attrs_file("not_exist.yaml", &attrs);

  EXPECT_EQ(RCUTILS_RET_ERROR, ret);
}
