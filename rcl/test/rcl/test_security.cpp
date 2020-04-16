// Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <algorithm>
#include <cstring>
#include <string>

#include "rcl/security.h"
#include "rcl/error_handling.h"

#include "rcutils/filesystem.h"

#include "rmw/error_handling.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"


#define TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME "/test_security_directory"
#define TEST_ENCLAVE "dummy_enclave"
#define TEST_ENCLAVE_ABSOLUTE "/" TEST_ENCLAVE

#ifndef _WIN32
# define PATH_SEPARATOR "/"
#else
# define PATH_SEPARATOR "\\"
#endif

#define TEST_ENCLAVE_MULTIPLE_TOKENS_ABSOLUTE \
  "/group1" TEST_ENCLAVE_ABSOLUTE
#define TEST_ENCLAVE_MULTIPLE_TOKENS_DIR \
  "group1" PATH_SEPARATOR TEST_ENCLAVE

char g_envstring[512] = {0};

static int putenv_wrapper(const char * env_var)
{
#ifdef _WIN32
  return _putenv(env_var);
#else
  return putenv(const_cast<char *>(env_var));
#endif
}

static int unsetenv_wrapper(const char * var_name)
{
#ifdef _WIN32
  // On windows, putenv("VAR=") deletes VAR from environment
  std::string var(var_name);
  var += "=";
  return _putenv(var.c_str());
#else
  return unsetenv(var_name);
#endif
}

class TestGetSecureRoot : public ::testing::Test
{
protected:
  void SetUp() final
  {
    // Reset rcutil error global state in case a previously
    // running test has failed.
    rcl_reset_error();

    // Always make sure the variable we set is unset at the beginning of a test
    unsetenv_wrapper(ROS_SECURITY_KEYSTORE_VAR_NAME);
    unsetenv_wrapper(ROS_SECURITY_ENCLAVE_OVERRIDE);
    unsetenv_wrapper(ROS_SECURITY_STRATEGY_VAR_NAME);
    unsetenv_wrapper(ROS_SECURITY_ENABLE_VAR_NAME);
    allocator = rcl_get_default_allocator();
    root_path = nullptr;
    secure_root = nullptr;
    base_lookup_dir_fqn = nullptr;
  }

  void TearDown() final
  {
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      allocator.deallocate(root_path, allocator.state);
    });
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      allocator.deallocate(secure_root, allocator.state);
    });
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      allocator.deallocate(base_lookup_dir_fqn, allocator.state);
    });
  }

  void set_base_lookup_dir_fqn(const char * resource_dir, const char * resource_dir_name)
  {
    base_lookup_dir_fqn = rcutils_join_path(
      resource_dir, resource_dir_name, allocator);
    std::string putenv_input = ROS_SECURITY_KEYSTORE_VAR_NAME "=";
    putenv_input += base_lookup_dir_fqn;
    memcpy(
      g_envstring, putenv_input.c_str(),
      std::min(putenv_input.length(), sizeof(g_envstring) - 1));
    putenv_wrapper(g_envstring);
  }

  rcl_allocator_t allocator;
  char * root_path;
  char * secure_root;
  char * base_lookup_dir_fqn;
};

TEST_F(TestGetSecureRoot, failureScenarios) {
  EXPECT_EQ(
    rcl_get_secure_root(TEST_ENCLAVE_ABSOLUTE, &allocator),
    (char *) NULL);
  rcl_reset_error();

  putenv_wrapper(ROS_SECURITY_KEYSTORE_VAR_NAME "=" TEST_RESOURCES_DIRECTORY);

  /* Security directory is set, but there's no matching directory */
  /// Wrong enclave
  EXPECT_EQ(
    rcl_get_secure_root("some_other_enclave", &allocator),
    (char *) NULL);
  rcl_reset_error();
}

TEST_F(TestGetSecureRoot, successScenarios_local_root_enclave) {
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);

  secure_root = rcl_get_secure_root("/", &allocator);
  ASSERT_NE(nullptr, secure_root);
  ASSERT_STREQ(
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME PATH_SEPARATOR "enclaves",
    secure_root);
}

TEST_F(TestGetSecureRoot, successScenarios_local_exactMatch) {
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);

  secure_root = rcl_get_secure_root(TEST_ENCLAVE_ABSOLUTE, &allocator);
  ASSERT_NE(nullptr, secure_root);
  std::string secure_root_str(secure_root);
  ASSERT_STREQ(
    TEST_ENCLAVE,
    secure_root_str.substr(secure_root_str.size() - strlen(TEST_ENCLAVE)).c_str());
}

TEST_F(TestGetSecureRoot, successScenarios_local_exactMatch_multipleTokensName) {
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);

  secure_root = rcl_get_secure_root(
    TEST_ENCLAVE_MULTIPLE_TOKENS_ABSOLUTE, &allocator);
  ASSERT_NE(nullptr, secure_root);
  std::string secure_root_str(secure_root);
  ASSERT_STREQ(
    TEST_ENCLAVE,
    secure_root_str.substr(secure_root_str.size() - strlen(TEST_ENCLAVE)).c_str());
}

TEST_F(TestGetSecureRoot, nodeSecurityEnclaveOverride_validEnclave) {
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);

  /* Specify a valid enclave */
  putenv_wrapper(ROS_SECURITY_ENCLAVE_OVERRIDE "=" TEST_ENCLAVE_ABSOLUTE);
  root_path = rcl_get_secure_root(
    "name shouldn't matter", &allocator);
  ASSERT_STREQ(
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME
    PATH_SEPARATOR "enclaves" PATH_SEPARATOR TEST_ENCLAVE,
    root_path);
}

TEST_F(TestGetSecureRoot, nodeSecurityEnclaveOverride_invalidEnclave) {
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);

  /* The override provided should exist. Providing correct node/namespace/root dir won't help
   * if the node override is invalid. */
  putenv_wrapper(
    ROS_SECURITY_ENCLAVE_OVERRIDE
    "=TheresN_oWayThi_sEnclave_Exists_hence_this_should_fail");
  EXPECT_EQ(
    rcl_get_secure_root(TEST_ENCLAVE_ABSOLUTE, &allocator),
    (char *) NULL);
}

TEST_F(TestGetSecureRoot, test_get_security_options) {
  /* The override provided should exist. Providing correct enclave name/root dir
   * won't help if the node override is invalid. */
  rmw_security_options_t options = rmw_get_zero_initialized_security_options();
  putenv_wrapper(ROS_SECURITY_ENABLE_VAR_NAME "=false");
  rcl_ret_t ret = rcl_get_security_options_from_environment(
    "doesn't matter at all", &allocator, &options);
  ASSERT_EQ(RMW_RET_OK, ret) << rmw_get_error_string().str;
  EXPECT_EQ(RMW_SECURITY_ENFORCEMENT_PERMISSIVE, options.enforce_security);
  EXPECT_EQ(NULL, options.security_root_path);

  putenv_wrapper(ROS_SECURITY_ENABLE_VAR_NAME "=true");
  putenv_wrapper(ROS_SECURITY_STRATEGY_VAR_NAME "=Enforce");
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);

  putenv_wrapper(
    ROS_SECURITY_ENCLAVE_OVERRIDE "=" TEST_ENCLAVE_MULTIPLE_TOKENS_ABSOLUTE);
  ret = rcl_get_security_options_from_environment(
    "doesn't matter at all", &allocator, &options);
  ASSERT_EQ(RMW_RET_OK, ret) << rmw_get_error_string().str;
  EXPECT_EQ(RMW_SECURITY_ENFORCEMENT_ENFORCE, options.enforce_security);
  EXPECT_STREQ(
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME
    PATH_SEPARATOR "enclaves" PATH_SEPARATOR TEST_ENCLAVE_MULTIPLE_TOKENS_DIR,
    options.security_root_path);
  EXPECT_EQ(RMW_RET_OK, rmw_security_options_fini(&options, &allocator));

  options = rmw_get_zero_initialized_security_options();
  unsetenv_wrapper(ROS_SECURITY_ENCLAVE_OVERRIDE);
  putenv_wrapper(
    ROS_SECURITY_KEYSTORE_VAR_NAME "="
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME);
  ret = rcl_get_security_options_from_environment(
    TEST_ENCLAVE_ABSOLUTE, &allocator, &options);
  ASSERT_EQ(RMW_RET_OK, ret) << rmw_get_error_string().str;
  EXPECT_EQ(RMW_SECURITY_ENFORCEMENT_ENFORCE, options.enforce_security);
  EXPECT_STREQ(
    TEST_RESOURCES_DIRECTORY TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME
    PATH_SEPARATOR "enclaves" PATH_SEPARATOR TEST_ENCLAVE,
    options.security_root_path);
}
