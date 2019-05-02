// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <algorithm>
#include "rcl/security_directory.h"
#include "rcutils/filesystem.h"

#define ROOT_NAMESPACE "/"
#define TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME "test_security_directory"
#define TEST_NODE_NAME "dummy_node"
#define TEST_NODE_NAMESPACE ROOT_NAMESPACE TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME

char g_envstring[512] = {0};

static int putenv_wrapper(const char * env_var)
{
#ifdef _WIN32
  return _putenv(env_var);
#else
  return putenv(reinterpret_cast<char *>(const_cast<char *>(env_var)));
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
  void SetUp() override
  {
    // Always make sure the variable we set is unset at the beginning of a test
    unsetenv_wrapper(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME);
    unsetenv_wrapper(ROS_SECURITY_NODE_DIRECTORY_VAR_NAME);
    unsetenv_wrapper(ROS_SECURITY_LOOKUP_TYPE_VAR_NAME);
  }
};

TEST_F(TestGetSecureRoot, failureScenarios) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  ASSERT_EQ(rcl_get_secure_root(TEST_NODE_NAME, TEST_NODE_NAMESPACE, &allocator),
    (char *) NULL);

  putenv_wrapper(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "=" TEST_RESOURCES_DIRECTORY);

  /* Security directory is set, but there's no matching directory */
  /// Wrong namespace
  ASSERT_EQ(rcl_get_secure_root(TEST_NODE_NAME, "/some_other_namespace", &allocator),
    (char *) NULL);
  /// Wrong node name
  ASSERT_EQ(rcl_get_secure_root("not_" TEST_NODE_NAME, TEST_NODE_NAMESPACE, &allocator),
    (char *) NULL);
}

TEST_F(TestGetSecureRoot, successScenarios) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  putenv_wrapper(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "=" TEST_RESOURCES_DIRECTORY);
  /* --------------------------
   * Namespace  : Custom (local)
   * Match type : Exact
   * --------------------------
   * Root: ${CMAKE_BINARY_DIR}/tests/resources
   * Namespace: /test_security_directory
   * Node: dummy_node
   */
  std::string secure_root = rcl_get_secure_root(TEST_NODE_NAME, TEST_NODE_NAMESPACE, &allocator);
  ASSERT_STREQ(TEST_NODE_NAME,
    secure_root.substr(secure_root.size() - (sizeof(TEST_NODE_NAME) - 1)).c_str());

  /* --------------------------
   * Namespace  : Custom (local)
   * Match type : Prefix
   * --------------------------
   * Root: ${CMAKE_BINARY_DIR}/tests/resources
   * Namespace: /test_security_directory
   * Node: dummy_node_and_some_suffix_added */
  ASSERT_STRNE(rcl_get_secure_root(TEST_NODE_NAME "_and_some_suffix_added", TEST_NODE_NAMESPACE,
    &allocator),
    secure_root.c_str());
  putenv_wrapper(ROS_SECURITY_LOOKUP_TYPE_VAR_NAME "=MATCH_PREFIX");
  ASSERT_STREQ(rcl_get_secure_root(TEST_NODE_NAME "_and_some_suffix_added", TEST_NODE_NAMESPACE,
    &allocator),
    secure_root.c_str());

  /* Include the namespace as part of the root security directory and test root namespace */
  char * base_lookup_dir_fqn = rcutils_join_path(TEST_RESOURCES_DIRECTORY,
      TEST_SECURITY_DIRECTORY_RESOURCES_DIR_NAME, allocator);
  std::string putenv_input = ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "=";
  putenv_input += base_lookup_dir_fqn;
  memcpy(g_envstring, putenv_input.c_str(),
    std::min(putenv_input.length(), sizeof(g_envstring) - 1));
  putenv_wrapper(g_envstring);
  /* --------------------------
   * Namespace  : Root
   * Match type : Exact
   * --------------------------
   * Root: ${CMAKE_BINARY_DIR}/tests/resources/test_security_directory
   * Namespace: /
   * Node: dummy_node */
  ASSERT_STREQ(rcl_get_secure_root(TEST_NODE_NAME, ROOT_NAMESPACE,
    &allocator),
    secure_root.c_str());
  putenv_wrapper(ROS_SECURITY_LOOKUP_TYPE_VAR_NAME "=MATCH_EXACT");
  ASSERT_STREQ(rcl_get_secure_root(TEST_NODE_NAME, ROOT_NAMESPACE,
    &allocator),
    secure_root.c_str());

  /* --------------------------
   * Namespace  : Root
   * Match type : Prefix
   * --------------------------
   * Root dir: ${CMAKE_BINARY_DIR}/tests/resources/test_security_directory
   * Namespace: /
   * Node: dummy_node_and_some_suffix_added */
  ASSERT_STRNE(rcl_get_secure_root(TEST_NODE_NAME "_and_some_suffix_added", ROOT_NAMESPACE,
    &allocator),
    secure_root.c_str());
  putenv_wrapper(ROS_SECURITY_LOOKUP_TYPE_VAR_NAME "=MATCH_PREFIX");
  ASSERT_STREQ(rcl_get_secure_root(TEST_NODE_NAME "_and_some_suffix_added", ROOT_NAMESPACE,
    &allocator),
    secure_root.c_str());
}

TEST_F(TestGetSecureRoot, nodeSecurityDirectoryOverride) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  /* Specify a valid directory */
  putenv_wrapper(ROS_SECURITY_NODE_DIRECTORY_VAR_NAME "=" TEST_RESOURCES_DIRECTORY);
  ASSERT_STREQ(rcl_get_secure_root("name shouldn't matter", "namespace shouldn't matter",
    &allocator), TEST_RESOURCES_DIRECTORY);

  /* Setting root dir has no effect */
  putenv_wrapper(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "=" TEST_RESOURCES_DIRECTORY);
  ASSERT_STREQ(rcl_get_secure_root("name shouldn't matter", "namespace shouldn't matter",
    &allocator), TEST_RESOURCES_DIRECTORY);

  /* The override provided should exist. Providing correct node/namespace/root dir won't help
   * if the node override is invalid. */
  putenv_wrapper(
    ROS_SECURITY_NODE_DIRECTORY_VAR_NAME
    "=TheresN_oWayThi_sDirectory_Exists_hence_this_would_fail");
  ASSERT_EQ(rcl_get_secure_root(TEST_NODE_NAME, TEST_NODE_NAMESPACE, &allocator),
    (char *) NULL);
}
