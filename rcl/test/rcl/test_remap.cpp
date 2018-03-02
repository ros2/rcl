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

#include "../memory_tools/memory_tools.hpp"

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
    stop_memory_checking();
    set_on_unexpected_malloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED MALLOC";});
    set_on_unexpected_realloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED REALLOC";});
    set_on_unexpected_free_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED FREE";});
    start_memory_checking();
  }

  void TearDown()
  {
    assert_no_malloc_end();
    assert_no_realloc_end();
    assert_no_free_end();
    stop_memory_checking();
    set_on_unexpected_malloc_callback(nullptr);
    set_on_unexpected_realloc_callback(nullptr);
    set_on_unexpected_free_callback(nullptr);
    rcl_ret_t ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};


/// \brief Helper to get around non-const args passed to rcl_init()
char ** copy_args(int argc, const char ** args)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  char ** copy = static_cast<char **>(allocator.allocate(sizeof(char *) * argc, allocator.state));
  for (int i = 0; i < argc; ++i) {
    size_t len = strlen(args[i]);
    // +1 for terminating \0
    copy[i] = static_cast<char *>(allocator.allocate(sizeof(char) * len + 1, allocator.state));
    strncpy(copy[i], args[i], len);
    copy[i][len] = '\0';
  }
  return copy;
}

/// \brief destroy args allocated by copy_args
void destroy_args(int argc, char ** args)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  for (int i = 0; i < argc; ++i) {
    allocator.deallocate(args[i], allocator.state);
  }
  allocator.deallocate(args, allocator.state);
}

#define INIT_GLOBAL_ARGS(...) \
  do { \
    const char * const_argv[] = {__VA_ARGS__}; \
    argc = (sizeof(const_argv) / sizeof(const char *)); \
    argv = copy_args(argc, const_argv); \
    ret = rcl_init(argc, argv, rcl_get_default_allocator()); \
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe(); \
  } while (false)

#define CLEANUP_GLOBAL_ARGS() \
  do { \
    destroy_args(argc, argv); \
  } while (false)

#define INIT_LOCAL_ARGS(...) \
  do { \
    const char * local_argv[] = {__VA_ARGS__}; \
    unsigned int local_argc = (sizeof(local_argv) / sizeof(const char *)); \
    ret = rcl_parse_arguments( \
      local_argc, local_argv, rcl_get_default_allocator(), &local_arguments); \
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe(); \
  } while (false)

#define CLEANUP_LOCAL_ARGS() \
  do { \
    ASSERT_EQ(RCL_RET_OK, rcl_arguments_fini(&local_arguments, rcl_get_default_allocator())); \
  } while (false)


TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), nodename_prefix_namespace_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS(
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

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name");

  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_namespace_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/global_args");
  rcl_arguments_t local_arguments;
  INIT_LOCAL_ARGS("process_name", "__ns:=/local_args");

  char * output = NULL;
  ret = rcl_remap_node_namespace(
    &local_arguments, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/local_args", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_LOCAL_ARGS();
  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, false, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_namespace_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/foobar:=/foo/bar", "__ns:=/namespace", "__node:=remap_name");

  char * output = NULL;
  ret = rcl_remap_node_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/namespace", output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/bar");

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

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), relative_topic_name_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "foo:=bar");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, true, "/ns/foo", "NodeName", "/ns", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_STREQ("/ns/bar", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), nodename_prefix_topic_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS(
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

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, false, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_topic_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/global_args");
  rcl_arguments_t local_arguments;
  INIT_LOCAL_ARGS("process_name", "/bar/foo:=/foo/local_args");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    &local_arguments, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/local_args", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_LOCAL_ARGS();
  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_topic_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/namespace", "__node:=remap_name", "/foobar:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_topic_name(
    NULL, true, "/foobar", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_service_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/bar");

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

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), relative_service_name_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "foo:=bar");

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, true, "/ns/foo", "NodeName", "/ns", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_STREQ("/ns/bar", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), nodename_prefix_service_remap) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS(
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

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_service_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, false, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_service_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name");

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_service_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/global_args");
  rcl_arguments_t local_arguments;
  INIT_LOCAL_ARGS("process_name", "/bar/foo:=/foo/local_args");

  char * output = NULL;
  ret = rcl_remap_service_name(
    &local_arguments, true, "/bar/foo", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/local_args", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_LOCAL_ARGS();
  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_service_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/namespace", "__node:=remap_name", "/foobar:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_service_name(
    NULL, true, "/foobar", "NodeName", "/", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_nodename_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__node:=globalname");

  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("globalname", output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_nodename_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name");

  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), local_nodename_replacement_before_global) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__node:=global_name");
  rcl_arguments_t local_arguments;
  INIT_LOCAL_ARGS("process_name", "__node:=local_name");

  char * output = NULL;
  ret = rcl_remap_node_name(
    &local_arguments, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("local_name", output);
  rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);

  CLEANUP_LOCAL_ARGS();
  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_nodename_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__node:=globalname");

  char * output = NULL;
  ret = rcl_remap_node_name(NULL, false, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), use_first_nodename_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__node:=firstname", "__node:=secondname");

  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("firstname", output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), other_rules_before_nodename_rule) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/foobar:=/foo/bar", "__ns:=/namespace", "__node:=remap_name");

  char * output = NULL;
  ret = rcl_remap_node_name(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("remap_name", output);

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), node_uses_remapped_name) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__node:=new_name");

  // Do remap node name using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/", &default_options));
    EXPECT_STREQ("new_name", rcl_node_get_name(&node));
    EXPECT_STREQ("new_name", rcl_node_get_logger_name(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Ignoring global args, don't remap node name
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/", &options));
    EXPECT_STREQ("original_name", rcl_node_get_name(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    INIT_LOCAL_ARGS("process_name", "__node:=local_name");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/", &options));
    EXPECT_STREQ("local_name", rcl_node_get_name(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
    CLEANUP_LOCAL_ARGS();
  }

  CLEANUP_GLOBAL_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), node_uses_remapped_namespace) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/new_ns");

  // Do remap namespace using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t default_options = rcl_node_get_default_options();
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/old_ns", &default_options));
    EXPECT_STREQ("/new_ns", rcl_node_get_namespace(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Do remap namespace using global rule
  {
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.use_global_arguments = false;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/old_ns", &options));
    EXPECT_STREQ("/old_ns", rcl_node_get_namespace(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
  }
  // Remap using local args before global args
  {
    rcl_arguments_t local_arguments;
    INIT_LOCAL_ARGS("process_name", "__ns:=/local_ns");
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_node_options_t options = rcl_node_get_default_options();
    options.arguments = local_arguments;
    ASSERT_EQ(RCL_RET_OK, rcl_node_init(&node, "original_name", "/old_ns", &options));
    EXPECT_STREQ("/local_ns", rcl_node_get_namespace(&node));
    EXPECT_EQ(RCL_RET_OK, rcl_node_fini(&node));
    CLEANUP_LOCAL_ARGS();
  }

  CLEANUP_GLOBAL_ARGS();
}
