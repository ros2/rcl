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

#include "rcl/security_directory.h"

#include "rcl/error_handling.h"
#include "rcutils/filesystem.h"
#include "rcutils/get_env.h"
#include "rcutils/format_string.h"

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wembedded-directive"
#endif
#include "tinydir/tinydir.h"
#ifdef __clang__
# pragma clang diagnostic pop
#endif

/**
 * A security lookup function takes in the node's name, namespace, a security root directory and an allocator;
 *  It returns the relevant information required to load the security credentials,
 *   which is currently a path to a directory on the filesystem containing DDS Security permission files.
 */
typedef char * (* security_lookup_fn_t) (
  const char * node_name,
  const char * node_namespace,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator
);

char * exact_match_lookup(
  const char * node_name,
  const char * node_namespace,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator
);

char * prefix_match_lookup(
  const char * node_name,
  const char * node_namespace,
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

/// Return the directory whose name most closely matches node_name (longest-prefix match),
/// scanning under base_dir.
/**
 * By using a prefix match, a node named e.g. "my_node_123" will be able to load and use the
 * directory "my_node" if no better match exists.
 * \param[in] base_dir
 * \param[in] node_name
 * \param[out] matched_name must be a valid memory address allocated with at least
 * _TINYDIR_FILENAME_MAX characters.
 * \return true if a match was found
 */
static bool get_best_matching_directory(
  const char * base_dir,
  const char * node_name,
  char * matched_name)
{
  size_t max_match_length = 0;
  tinydir_dir dir;
  if (NULL == base_dir || NULL == node_name || NULL == matched_name) {
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
        strncmp(file.name, node_name,
        matched_name_length) && matched_name_length > max_match_length)
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
  const char * node_name,
  const char * node_namespace,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator)
{
  // Perform an exact match for the node's name in directory <root dir>/<namespace>.
  char * node_secure_root = NULL;
  // "/" case when root namespace is explicitly passed in
  if (1 == strlen(node_namespace)) {
    node_secure_root = rcutils_join_path(ros_secure_root_env, node_name, *allocator);
  } else {
    char * node_fqn = NULL;
    char * node_root_path = NULL;
    // Combine node namespace with node name
    // TODO(ros2team): remove the hard-coded value of the root namespace
    node_fqn = rcutils_format_string(*allocator, "%s%s%s", node_namespace, "/", node_name);
    // Get native path, ignore the leading forward slash
    // TODO(ros2team): remove the hard-coded length, use the length of the root namespace instead
    node_root_path = rcutils_to_native_path(node_fqn + 1, *allocator);
    node_secure_root = rcutils_join_path(ros_secure_root_env, node_root_path, *allocator);
    allocator->deallocate(node_fqn, allocator->state);
    allocator->deallocate(node_root_path, allocator->state);
  }
  return node_secure_root;
}

char * prefix_match_lookup(
  const char * node_name,
  const char * node_namespace,
  const char * ros_secure_root_env,
  const rcl_allocator_t * allocator)
{
  // Perform longest prefix match for the node's name in directory <root dir>/<namespace>.
  char * node_secure_root = NULL;
  char matched_dir[_TINYDIR_FILENAME_MAX] = {0};
  char * base_lookup_dir = NULL;
  if (strlen(node_namespace) == 1) {
    base_lookup_dir = (char *) ros_secure_root_env;
  } else {
    // TODO(ros2team): remove the hard-coded length, use the length of the root namespace instead.
    base_lookup_dir = rcutils_join_path(ros_secure_root_env, node_namespace + 1, *allocator);
  }
  if (get_best_matching_directory(base_lookup_dir, node_name, matched_dir)) {
    node_secure_root = rcutils_join_path(base_lookup_dir, matched_dir, *allocator);
  }
  if (base_lookup_dir != ros_secure_root_env && NULL != base_lookup_dir) {
    allocator->deallocate(base_lookup_dir, allocator->state);
  }
  return node_secure_root;
}

char * rcl_get_secure_root(
  const char * node_name,
  const char * node_namespace,
  const rcl_allocator_t * allocator)
{
  bool ros_secure_node_override = true;

  // find out if either of the configuration environment variables are set
  const char * env_buf = NULL;
  if (NULL == node_name) {
    return NULL;
  }
  if (rcutils_get_env(ROS_SECURITY_NODE_DIRECTORY_VAR_NAME, &env_buf)) {
    return NULL;
  }
  if (!env_buf) {
    return NULL;
  }
  size_t ros_secure_root_size = strlen(env_buf);
  if (!ros_secure_root_size) {
    // check root directory if node directory environment variable is empty
    if (rcutils_get_env(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME, &env_buf)) {
      return NULL;
    }
    if (!env_buf) {
      return NULL;
    }
    ros_secure_root_size = strlen(env_buf);
    if (!ros_secure_root_size) {
      return NULL;  // environment variable was empty
    } else {
      ros_secure_node_override = false;
    }
  }

  // found a usable environment variable, copy into our memory before overwriting with next lookup
  char * ros_secure_root_env =
    (char *)allocator->allocate(ros_secure_root_size + 1, allocator->state);
  memcpy(ros_secure_root_env, env_buf, ros_secure_root_size + 1);
  // TODO(ros2team): This make an assumption on the value and length of the root namespace.
  // This should likely come from another (rcl/rmw?) function for reuse.
  // If the namespace is the root namespace ("/"), the secure root is just the node name.

  char * lookup_strategy = NULL;
  char * node_secure_root = NULL;
  if (ros_secure_node_override) {
    node_secure_root = (char *)allocator->allocate(ros_secure_root_size + 1, allocator->state);
    memcpy(node_secure_root, ros_secure_root_env, ros_secure_root_size + 1);
    lookup_strategy = g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_NODE_OVERRIDE];

  } else {
    // Check which lookup method to use and invoke the relevant function.
    const char * ros_security_lookup_type = NULL;
    if (rcutils_get_env(ROS_SECURITY_LOOKUP_TYPE_VAR_NAME, &ros_security_lookup_type)) {
      allocator->deallocate(ros_secure_root_env, allocator->state);
      return NULL;
    }
    if (0 == strcmp(ros_security_lookup_type,
      g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_MATCH_PREFIX]))
    {
      node_secure_root = g_security_lookup_fns[ROS_SECURITY_LOOKUP_MATCH_PREFIX]
          (node_name, node_namespace, ros_secure_root_env, allocator);
      lookup_strategy = g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_MATCH_PREFIX];
    } else { /* Default is MATCH_EXACT */
      node_secure_root = g_security_lookup_fns[ROS_SECURITY_LOOKUP_MATCH_EXACT]
          (node_name, node_namespace, ros_secure_root_env, allocator);
      lookup_strategy = g_security_lookup_type_strings[ROS_SECURITY_LOOKUP_MATCH_EXACT];
    }
  }

  if (NULL == node_secure_root || !rcutils_is_directory(node_secure_root)) {
    // Check node_secure_root is not NULL before checking directory
    if (NULL == node_secure_root) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "SECURITY ERROR: unable to find a folder matching the node name in %s%s."
        "Lookup strategy: %s",
        ros_secure_root_env, node_namespace, lookup_strategy);
    } else {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "SECURITY ERROR: directory %s does not exist. Lookup strategy: %s",
        node_secure_root, lookup_strategy);
    }
    allocator->deallocate(ros_secure_root_env, allocator->state);
    allocator->deallocate(node_secure_root, allocator->state);
    return NULL;
  }
  allocator->deallocate(ros_secure_root_env, allocator->state);
  return node_secure_root;
}
