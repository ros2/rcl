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

#include "rcl/rcl.h"
#include "rcl/remap.h"
#include "rcl/error_handling.h"

#include "./arg_macros.hpp"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestRemapFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
  }

  void TearDown()
  {
  }
};

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__ns:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), nodename_prefix_namespace_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv,
    "process_name", "Node1:__ns:=/foo/bar", "Node2:__ns:=/this_one", "Node3:__ns:=/bar/foo");

  {
    char * output = NULL;
    ret = rcl_remap_node_namespace(NULL, true, "Node1", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/foo/bar", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_node_namespace(NULL, true, "Node2", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/this_one", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_node_namespace(NULL, true, "Node3", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/bar/foo", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name");

  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_namespace_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__ns:=/global_args");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name", "__ns:=/local_args");

  char * output = NULL;
  ret = rcl_remap_node_namespace(
    &local_arguments, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/local_args", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__ns:=/foo/bar");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name");

  char * output = NULL;
  ret = rcl_remap_node_namespace(
    &local_arguments, false, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_namespace_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/foobar:=/foo/bar", "__ns:=/namespace",
    "__node:=remap_name");

  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, true, "NodeName", allocator, &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/namespace", output);
  allocator.deallocate(output, allocator.state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/bar/foo:=/foo/bar");

  {
    char * output = NULL;
    ret = rcl_remap_topic_name(
      NULL, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_STREQ("/foo/bar", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_topic_name(
      NULL, true, "/foo/bar", "NodeName", "/", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_EQ(NULL, output);
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), relative_topic_name_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "foo:=bar");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, true, "/ns/foo", "NodeName", "/ns", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_STREQ("/ns/bar", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), nodename_prefix_topic_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv,
    "process_name", "Node1:/foo:=/foo/bar", "Node2:/foo:=/this_one", "Node3:/foo:=/bar/foo");

  {
    char * output = NULL;
    ret = rcl_remap_topic_name(NULL, true, "/foo", "Node1", "/", rcl_get_default_allocator(),
        &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/foo/bar", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_topic_name(NULL, true, "/foo", "Node2", "/", rcl_get_default_allocator(),
        &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/this_one", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_topic_name(NULL, true, "/foo", "Node3", "/", rcl_get_default_allocator(),
        &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/bar/foo", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/bar/foo:=/foo/bar");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    &local_arguments, false, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_topic_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/bar/foo:=/foo/global_args");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/bar/foo:=/foo/local_args");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    &local_arguments, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/local_args", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_topic_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__ns:=/namespace", "__node:=remap_name",
    "/foobar:=/foo/bar");

  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, true, "/foobar", "NodeName", "/", allocator, &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);
  allocator.deallocate(output, allocator.state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_service_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/bar/foo:=/foo/bar");

  {
    char * output = NULL;
    ret = rcl_remap_service_name(
      NULL, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_STREQ("/foo/bar", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_service_name(
      NULL, true, "/foobar", "NodeName", "/", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_EQ(NULL, output);
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), relative_service_name_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "foo:=bar");

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, true, "/ns/foo", "NodeName", "/ns", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_STREQ("/ns/bar", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), nodename_prefix_service_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv,
    "process_name", "Node1:/foo:=/foo/bar", "Node2:/foo:=/this_one", "Node3:/foo:=/bar/foo");

  {
    char * output = NULL;
    ret = rcl_remap_service_name(NULL, true, "/foo", "Node1", "/", rcl_get_default_allocator(),
        &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/foo/bar", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_service_name(NULL, true, "/foo", "Node2", "/", rcl_get_default_allocator(),
        &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/this_one", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_service_name(NULL, true, "/foo", "Node3", "/", rcl_get_default_allocator(),
        &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_STREQ("/bar/foo", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_service_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/bar/foo:=/foo/bar");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name");

  char * output = NULL;
  ret = rcl_remap_service_name(
    &local_arguments, false, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_service_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name");

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_service_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/bar/foo:=/foo/global_args");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name", "/bar/foo:=/foo/local_args");

  char * output = NULL;
  ret = rcl_remap_service_name(
    &local_arguments, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/local_args", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_service_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__ns:=/namespace", "__node:=remap_name",
    "/foobar:=/foo/bar");

  rcl_allocator_t allocator = rcl_get_default_allocator();

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, true, "/foobar", "NodeName", "/", allocator, &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);
  allocator.deallocate(output, allocator.state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_nodename_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__node:=globalname");

  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", allocator, &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("globalname", output);
  allocator.deallocate(output, allocator.state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_nodename_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name");

  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_nodename_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__node:=global_name");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name", "__node:=local_name");

  char * output = NULL;
  ret = rcl_remap_node_name(
    &local_arguments, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("local_name", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_nodename_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__node:=globalname");
  rcl_arguments_t local_arguments;
  SCOPE_LOCAL_ARGS(local_arguments, "process_name");

  char * output = NULL;
  ret = rcl_remap_node_name(
    &local_arguments, false, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), use_first_nodename_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "__node:=firstname", "__node:=secondname");

  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", allocator, &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("firstname", output);
  allocator.deallocate(output, allocator.state);
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_nodename_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  SCOPE_GLOBAL_ARGS(argc, argv, "process_name", "/foobar:=/foo/bar", "__ns:=/namespace",
    "__node:=remap_name");

  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", allocator, &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("remap_name", output);
  allocator.deallocate(output, allocator.state);
}
