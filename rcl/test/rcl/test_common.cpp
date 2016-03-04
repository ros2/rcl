// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include <string>

#include "../../src/rcl/common.h"
#include "../../src/rcl/common.c"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

/* Tests the default allocator.
 *
 * Expected environment variables must be set by the calling code:
 *
 *   - EMPTY_TEST=
 *   - NORMAL_TEST=foo
 *
 * These are set in the call to `ament_add_gtest()` in the `CMakeLists.txt`.
 */
TEST(CLASSNAME(TestCommon, RMW_IMPLEMENTATION), test_getenv) {
  const char * env;
  rcl_ret_t ret;
  ret = rcl_impl_getenv("NORMAL_TEST", NULL);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ret = rcl_impl_getenv(NULL, &env);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
  ret = rcl_impl_getenv("SHOULD_NOT_EXIST_TEST", &env);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ("", std::string(env)) << std::string(env);
  rcl_reset_error();
  ret = rcl_impl_getenv("NORMAL_TEST", &env);
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_NE(nullptr, env);
  EXPECT_EQ("foo", std::string(env));
  ret = rcl_impl_getenv("EMPTY_TEST", &env);
  EXPECT_EQ(RCL_RET_OK, ret);
  ASSERT_NE(nullptr, env);
  EXPECT_EQ("", std::string(env));
}
