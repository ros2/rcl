// Copyright 2018-2020 Open Source Robotics Foundation, Inc.
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

#include "rcl/security.h"

#include <stdbool.h>

#include "rcl/error_handling.h"

#include "rcutils/logging_macros.h"
#include "rcutils/filesystem.h"
#include "rcutils/get_env.h"
#include "rcutils/strdup.h"

#include "rmw/security_options.h"

rcl_ret_t
rcl_get_security_options_from_environment(
  const char * name,
  const rcutils_allocator_t * allocator,
  rmw_security_options_t * security_options)
{
  bool use_security = false;
  rcl_ret_t ret = rcl_security_enabled(&use_security);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Using security: %s", use_security ? "true" : "false");

  if (!use_security) {
    security_options->enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
    return RMW_RET_OK;
  }

  ret = rcl_get_enforcement_policy(&security_options->enforce_security);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  // File discovery magic here
  char * secure_root = rcl_get_secure_root(name, allocator);
  if (secure_root) {
    RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "Found security directory: %s", secure_root);
    security_options->security_root_path = secure_root;
  } else {
    if (RMW_SECURITY_ENFORCEMENT_ENFORCE == security_options->enforce_security) {
      return RCL_RET_ERROR;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_security_enabled(bool * use_security)
{
  const char * ros_security_enable = NULL;
  const char * get_env_error_str = NULL;

  RCL_CHECK_ARGUMENT_FOR_NULL(use_security, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(ROS_SECURITY_ENABLE_VAR_NAME, &ros_security_enable);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(ROS_SECURITY_ENABLE_VAR_NAME) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }

  *use_security = (0 == strcmp(ros_security_enable, "true"));
  return RCL_RET_OK;
}

rcl_ret_t
rcl_get_enforcement_policy(rmw_security_enforcement_policy_t * policy)
{
  const char * ros_enforce_security = NULL;
  const char * get_env_error_str = NULL;

  RCL_CHECK_ARGUMENT_FOR_NULL(policy, RCL_RET_INVALID_ARGUMENT);

  get_env_error_str = rcutils_get_env(ROS_SECURITY_STRATEGY_VAR_NAME, &ros_enforce_security);
  if (NULL != get_env_error_str) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting env var '" RCUTILS_STRINGIFY(ROS_SECURITY_STRATEGY_VAR_NAME) "': %s\n",
      get_env_error_str);
    return RCL_RET_ERROR;
  }

  *policy = (0 == strcmp(ros_enforce_security, "Enforce")) ?
    RMW_SECURITY_ENFORCEMENT_ENFORCE : RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
  return RCL_RET_OK;
}

char * exact_match_lookup(
  const char * name,
  const char * ros_secure_keystore_env,
  const rcl_allocator_t * allocator)
{
  // Perform an exact match for the enclave name in directory <root dir>.
  char * secure_root = NULL;
  char * enclaves_dir = NULL;
  enclaves_dir = rcutils_join_path(ros_secure_keystore_env, "enclaves", *allocator);
  // "/" case when root namespace is explicitly passed in
  if (0 == strcmp(name, "/")) {
    secure_root = enclaves_dir;
  } else {
    char * relative_path = NULL;
    // Get native path, ignore the leading forward slash
    // TODO(ros2team): remove the hard-coded length, use the length of the root namespace instead
    relative_path = rcutils_to_native_path(name + 1, *allocator);
    secure_root = rcutils_join_path(enclaves_dir, relative_path, *allocator);
    allocator->deallocate(relative_path, allocator->state);
    allocator->deallocate(enclaves_dir, allocator->state);
  }
  return secure_root;
}

char * rcl_get_secure_root(
  const char * name,
  const rcl_allocator_t * allocator)
{
  bool ros_secure_enclave_override = true;

  // find out if either of the configuration environment variables are set
  const char * env_buf = NULL;
  if (NULL == name) {
    return NULL;
  }
  const char * get_env_error_str = NULL;
  // check if enclave override environment variable is empty
  get_env_error_str = rcutils_get_env(ROS_SECURITY_ENCLAVE_OVERRIDE, &env_buf);
  if (NULL != get_env_error_str) {
    RCUTILS_LOG_ERROR("rcutils_get_env failed: %s\n", get_env_error_str);
    return NULL;
  }
  if (!env_buf) {
    return NULL;
  }
  if (0 == strcmp("", env_buf)) {
    ros_secure_enclave_override = false;
  }
  char * ros_secure_enclave_override_env = rcutils_strdup(env_buf, *allocator);

  // check if keystore environment variable is empty
  get_env_error_str = rcutils_get_env(ROS_SECURITY_KEYSTORE_VAR_NAME, &env_buf);
  if (NULL != get_env_error_str) {
    RCUTILS_LOG_ERROR("rcutils_get_env failed: %s\n", get_env_error_str);
    allocator->deallocate(ros_secure_enclave_override_env, allocator->state);
    return NULL;
  }
  if (!env_buf) {
    allocator->deallocate(ros_secure_enclave_override_env, allocator->state);
    return NULL;
  }
  if (0 == strcmp("", env_buf)) {
    allocator->deallocate(ros_secure_enclave_override_env, allocator->state);
    return NULL;  // environment variable was empty
  }
  char * ros_secure_keystore_env = rcutils_strdup(env_buf, *allocator);

  // given usable environment variables, overwrite with next lookup
  char * secure_root = NULL;
  if (ros_secure_enclave_override) {
    secure_root = exact_match_lookup(
      ros_secure_enclave_override_env,
      ros_secure_keystore_env,
      allocator);
  } else {
    secure_root = exact_match_lookup(
      name,
      ros_secure_keystore_env,
      allocator);
  }

  if (NULL == secure_root || !rcutils_is_directory(secure_root)) {
    // Check secure_root is not NULL before checking directory
    if (NULL == secure_root) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "SECURITY ERROR: unable to find a folder matching the name '%s' in '%s'. ",
        name, ros_secure_keystore_env);
    } else {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "SECURITY ERROR: directory '%s' does not exist.", secure_root);
    }
    allocator->deallocate(ros_secure_enclave_override_env, allocator->state);
    allocator->deallocate(ros_secure_keystore_env, allocator->state);
    allocator->deallocate(secure_root, allocator->state);
    return NULL;
  }
  allocator->deallocate(ros_secure_keystore_env, allocator->state);
  return secure_root;
}
