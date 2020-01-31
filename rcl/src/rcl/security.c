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

#include "rcl/security.h"

#include <stdbool.h>

#include "rcl/error_handling.h"

#include "rcutils/logging_macros.h"
#include "rcutils/filesystem.h"
#include "rcutils/format_string.h"
#include "rcutils/get_env.h"
#include "rcutils/strdup.h"

#include "rmw/security.h"
#include "rmw/security_options.h"

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wembedded-directive"
#endif
#include "tinydir/tinydir.h"
#ifdef __clang__
# pragma clang diagnostic pop
#endif

rcl_ret_t
rcl_get_security_options_from_environment(
  const char * name,
  const char * namespace_,
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

  if (!rmw_use_node_name_in_security_directory_lookup()) {
    ret = rcl_get_enforcement_policy(&security_options->enforce_security);
    if (RCL_RET_OK != ret) {
      return ret;
    }

    if (!use_security) {
      security_options->enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
    } else {  // if use_security
      // File discovery magic here
      char * secure_root = rcl_get_secure_root(
        name,
        namespace_,
        allocator);
      if (secure_root) {
        RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "Found security directory: %s", secure_root);
        security_options->security_root_path = secure_root;
      } else {
        if (RMW_SECURITY_ENFORCEMENT_ENFORCE == security_options->enforce_security) {
          return RCL_RET_ERROR;
        }
      }
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_security_enabled(bool * use_security)
{
  const char * ros_security_enable = NULL;
  const char * get_env_error_str = NULL;

  if (!use_security) {
    return RCL_RET_INVALID_ARGUMENT;
  }

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

  if (!policy) {
    return RCL_RET_INVALID_ARGUMENT;
  }

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

/**
 * A security lookup function takes in the node/context's name, namespace, a security root directory and an allocator;
 *  It returns the relevant information required to load the security credentials,
 *   which is currently a path to a directory on the filesystem containing DDS Security permission files.
 */
typedef char * (* security_lookup_fn_t) (
  const char * name,
  const char * namespace_,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator
);

char * exact_match_lookup(
  const char * name,
  const char * namespace_,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator
);

char * prefix_match_lookup(
  const char * name,
  const char * namespace_,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator
);

security_lookup_fn_t g_security_lookup_fns[] = {
  NULL,
  exact_match_lookup,
  prefix_match_lookup,
};

typedef enum ros_security_lookup_type_e
{
  ROS_SECURITY_LOOKUP_NODE_OVERRIDE = 0,
  ROS_SECURITY_LOOKUP_MATCH_EXACT = 1,
  ROS_SECURITY_LOOKUP_MATCH_PREFIX = 2,
} ros_security_lookup_type_t;

char * g_security_lookup_type_strings[] = {
  "NODE_OVERRIDE",
  "MATCH_EXACT",
  "MATCH_PREFIX"
};

/// Return the directory whose name most closely matches name (longest-prefix match),
/// scanning under base_dir.
/**
 * By using a prefix match, a name "my_name_123" will be able to load and use the
 * directory "my_name" if no better match exists.
 * \param[in] base_dir
 * \param[in] name
 * \param[out] matched_name must be a valid memory address allocated with at least
 * _TINYDIR_FILENAME_MAX characters.
 * \return true if a match was found
 */
static bool get_best_matching_directory(
  const char * base_dir,
  const char * name,
  char * matched_name)
{
  size_t max_match_length = 0;
  tinydir_dir dir;
  if (NULL == base_dir || NULL == name || NULL == matched_name) {
    return false;
  }
  if (-1 == tinydir_open(&dir, base_dir)) {
    return false;
  }
  while (dir.has_next) {
    tinydir_file file;
    if (-1 == tinydir_readfile(&dir, &file)) {
      goto cleanup;
    }
    if (file.is_dir) {
      size_t matched_name_length = strnlen(file.name, sizeof(file.name) - 1);
      if (0 ==
        strncmp(
          file.name, name, matched_name_length) &&
          matched_name_length > max_match_length)
      {
        max_match_length = matched_name_length;
        memcpy(matched_name, file.name, max_match_length);
      }
    }
    if (-1 == tinydir_next(&dir)) {
      goto cleanup;
    }
  }
cleanup:
  tinydir_close(&dir);
  return max_match_length > 0;
}

char * exact_match_lookup(
  const char * name,
  const char * namespace_,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator)
{
  // Perform an exact match for the node/context's name in directory <root dir>/<namespace>.
  char * secure_root = NULL;
  // "/" case when root namespace is explicitly passed in
  if (1 == strlen(namespace_)) {
    secure_root = rcutils_join_path(ros_secure_root_env, name, *allocator);
  } else {
    char * fqn = NULL;
    char * root_path = NULL;
    // Combine namespace with name
    // TODO(ros2team): remove the hard-coded value of the root namespace
    fqn = rcutils_format_string(*allocator, "%s%s%s", namespace_, "/", name);
    // Get native path, ignore the leading forward slash
    // TODO(ros2team): remove the hard-coded length, use the length of the root namespace instead
    root_path = rcutils_to_native_path(fqn + 1, *allocator);
    secure_root = rcutils_join_path(ros_secure_root_env, root_path, *allocator);
    allocator->deallocate(fqn, allocator->state);
    allocator->deallocate(root_path, allocator->state);
  }
  return secure_root;
}

char * prefix_match_lookup(
  const char * name,
  const char * namespace_,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator)
{
  // Perform longest prefix match for the node/context's name in directory <root dir>/<namespace>.
  char * secure_root = NULL;
  char matched_dir[_TINYDIR_FILENAME_MAX] = {0};
  char * base_lookup_dir = NULL;
  if (strlen(namespace_) == 1) {
    base_lookup_dir = (char *) ros_secure_root_env;
  } else {
    // TODO(ros2team): remove the hard-coded length, use the length of the root namespace instead.
    base_lookup_dir = rcutils_join_path(ros_secure_root_env, namespace_ + 1, *allocator);
  }
  if (get_best_matching_directory(base_lookup_dir, name, matched_dir)) {
    secure_root = rcutils_join_path(base_lookup_dir, matched_dir, *allocator);
  }
  if (base_lookup_dir != ros_secure_root_env && NULL != base_lookup_dir) {
    allocator->deallocate(base_lookup_dir, allocator->state);
  }
  return secure_root;
}

char * rcl_get_secure_root(
  const char * name,
  const char * namespace_,
  const rcl_allocator_t * allocator)
{
  bool ros_secure_node_override = true;
  bool use_node_name_in_lookup = rmw_use_node_name_in_security_directory_lookup();

  // find out if either of the configuration environment variables are set
  const char * env_buf = NULL;
  if (NULL == name) {
    return NULL;
  }
  if (use_node_name_in_lookup) {
    if (rcutils_get_env(ROS_SECURITY_NODE_DIRECTORY_VAR_NAME, &env_buf)) {
      return NULL;
    }
    if (!env_buf) {
      return NULL;
    }
  }
  if (!use_node_name_in_lookup || 0 == strcmp("", env_buf)) {
    // check root directory if node directory environment variable is empty
    if (rcutils_get_env(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME, &env_buf)) {
      return NULL;
    }
    if (!env_buf) {
      return NULL;
    }
    if (0 == strcmp("", env_buf)) {
      return NULL;  // environment variable was empty
    } else {
      ros_secure_node_override = false;
    }
  }

  // found a usable environment variable, copy into our memory before overwriting with next lookup
  char * ros_secure_root_env = rcutils_strdup(env_buf, *allocator);

  char * lookup_strategy = NULL;
  char * secure_root = NULL;
  if (ros_secure_node_override) {
    secure_root = rcutils_strdup(ros_secure_root_env, *allocator);
    lookup_strategy = g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_NODE_OVERRIDE];
  } else {
    // Check which lookup method to use and invoke the relevant function.
    const char * ros_security_lookup_type = NULL;
    if (rcutils_get_env(ROS_SECURITY_LOOKUP_TYPE_VAR_NAME, &ros_security_lookup_type)) {
      allocator->deallocate(ros_secure_root_env, allocator->state);
      return NULL;
    }
    if (
      0 == strcmp(
        ros_security_lookup_type,
        g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_MATCH_PREFIX]))
    {
      secure_root = g_security_lookup_fns[ROS_SECURITY_LOOKUP_MATCH_PREFIX]
          (name, namespace_, ros_secure_root_env, allocator);
      lookup_strategy = g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_MATCH_PREFIX];
    } else { /* Default is MATCH_EXACT */
      secure_root = g_security_lookup_fns[ROS_SECURITY_LOOKUP_MATCH_EXACT]
          (name, namespace_, ros_secure_root_env, allocator);
      lookup_strategy = g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_MATCH_EXACT];
    }
  }

  if (NULL == secure_root || !rcutils_is_directory(secure_root)) {
    // Check secure_root is not NULL before checking directory
    if (NULL == secure_root) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "SECURITY ERROR: unable to find a folder matching the name '%s' in '%s%s'. "
        "Lookup strategy: %s",
        name, ros_secure_root_env, namespace_, lookup_strategy);
    } else {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "SECURITY ERROR: directory '%s' does not exist. Lookup strategy: %s",
        secure_root, lookup_strategy);
    }
    allocator->deallocate(ros_secure_root_env, allocator->state);
    allocator->deallocate(secure_root, allocator->state);
    return NULL;
  }
  allocator->deallocate(ros_secure_root_env, allocator->state);
  return secure_root;
}
