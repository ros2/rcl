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
    copy[i] = static_cast<char *>(allocator.allocate(sizeof(char) * len, allocator.state));
    strncpy(copy[i], args[i], len);
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

#define CLEANUP_ARGS() \
  do { \
    destroy_args(argc, argv); \
  } while (false)



TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_STREQ("/foo/bar", output);

  CLEANUP_ARGS();
}


TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name");

  char * output = NULL;
  ret = rcl_remap_namespace(NULL, true, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_ARGS();
}


TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_namespace_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "__ns:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_namespace(NULL, false, "NodeName", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), global_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/bar");

  {
    char * output = NULL;
    ret = rcl_remap_name(NULL, true, "NodeName", "/bar/foo", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    ASSERT_STREQ("/foo/bar", output);
    rcl_get_default_allocator().deallocate(output, rcl_get_default_allocator().state);
  }
  {
    char * output = NULL;
    ret = rcl_remap_name(NULL, true, "NodeName", "/foobar", rcl_get_default_allocator(), &output);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_EQ(NULL, output);
  }

  CLEANUP_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_use_global_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name", "/bar/foo:=/foo/bar");

  char * output = NULL;
  ret = rcl_remap_name(NULL, false, "NodeName", "/bar/foo", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_ARGS();
}

TEST_F(CLASSNAME(TestRemapFixture, RMW_IMPLEMENTATION), no_topic_name_replacement) {
  unsigned int argc;
  char ** argv;
  rcl_ret_t ret;
  INIT_GLOBAL_ARGS("process_name");

  char * output = NULL;
  ret = rcl_remap_name(NULL, true, "NodeName", "/bar/foo", rcl_get_default_allocator(), &output);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(NULL, output);

  CLEANUP_ARGS();
}
