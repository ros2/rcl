// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rcutils/env.h"

#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/rmw_implementation_identifier_check.h"

TEST(TestRmwCheck, test_rmw_check_id_impl) {
  EXPECT_EQ(RCL_RET_OK, rcl_rmw_implementation_identifier_check());
}

TEST(TestRmwCheck, test_failing_configuration) {
  const char * expected_rmw_impl_env = NULL;
  const char * expected_rmw_id_matches = NULL;

  const char * get_env_var_name = rcutils_get_env(
    RMW_IMPLEMENTATION_ENV_VAR_NAME,
    &expected_rmw_impl_env);

  const char * get_env_id_matches_name = rcutils_get_env(
    RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME,
    &expected_rmw_id_matches);

  // Fail test case, reason: RMW_IMPLEMENTATION_ENV_VAR_NAME set, not matching rmw impl
  EXPECT_TRUE(rcutils_set_env(RMW_IMPLEMENTATION_ENV_VAR_NAME, "some_random_name"));
  EXPECT_TRUE(rcutils_set_env(RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME, ""));
  EXPECT_EQ(RCL_RET_MISMATCHED_RMW_ID, rcl_rmw_implementation_identifier_check());
  EXPECT_TRUE(rcl_error_is_set());
  rcl_reset_error();

  // Fail test case, reason: RMW_IMPLEMENTATION_ENV_VAR_NAME set, not matching rmw impl
  EXPECT_TRUE(rcutils_set_env(RMW_IMPLEMENTATION_ENV_VAR_NAME, ""));
  EXPECT_TRUE(rcutils_set_env(RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME, "some_random_name"));
  EXPECT_EQ(RCL_RET_MISMATCHED_RMW_ID, rcl_rmw_implementation_identifier_check());
  EXPECT_TRUE(rcl_error_is_set());
  rcl_reset_error();

  // Fail test case, reason: env variables not equal
  EXPECT_TRUE(rcutils_set_env(RMW_IMPLEMENTATION_ENV_VAR_NAME, "some_random_name"));
  EXPECT_TRUE(rcutils_set_env(RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME, "diff_random"));
  EXPECT_EQ(RCL_RET_ERROR, rcl_rmw_implementation_identifier_check());
  EXPECT_TRUE(rcl_error_is_set());
  rcl_reset_error();

  // Fail test case, reason: equal env variables do not match rmw impl
  EXPECT_TRUE(rcutils_set_env(RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME, "some_random_name"));
  EXPECT_TRUE(rcutils_set_env(RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME, "some_random_name"));
  EXPECT_EQ(RCL_RET_MISMATCHED_RMW_ID, rcl_rmw_implementation_identifier_check());
  EXPECT_TRUE(rcl_error_is_set());
  rcl_reset_error();

  // Restore env variables set in the test
  EXPECT_TRUE(rcutils_set_env(RMW_IMPLEMENTATION_ENV_VAR_NAME, get_env_var_name));
  EXPECT_TRUE(rcutils_set_env(RCL_ASSERT_RMW_ID_MATCHES_ENV_VAR_NAME, get_env_id_matches_name));
}
