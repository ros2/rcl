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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/node.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/arguments.h"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/remap.h"
#include "rcutils/filesystem.h"
#include "rcutils/find.h"
#include "rcutils/format_string.h"
#include "rcutils/get_env.h"
#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"
#include "rcutils/repl_str.h"
#include "rcutils/snprintf.h"
#include "rcutils/strdup.h"
#include "rmw/error_handling.h"
#include "rmw/node_security_options.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "./common.h"
#include "./context_impl.h"

#define ROS_SECURITY_NODE_DIRECTORY_VAR_NAME "ROS_SECURITY_NODE_DIRECTORY"
#define ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "ROS_SECURITY_ROOT_DIRECTORY"
#define ROS_SECURITY_STRATEGY_VAR_NAME "ROS_SECURITY_STRATEGY"
#define ROS_SECURITY_ENABLE_VAR_NAME "ROS_SECURITY_ENABLE"

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  size_t actual_domain_id;
  rmw_node_t * rmw_node_handle;
  rcl_guard_condition_t * graph_guard_condition;
  const char * logger_name;
} rcl_node_impl_t;


/// Return the logger name associated with a node given the validated node name and namespace.
/**
 * E.g. for a node named "c" in namespace "/a/b", the logger name will be
 * "a.b.c", assuming logger name separator of ".".
 *
 * \param[in] node_name validated node name (a single token)
 * \param[in] node_namespace validated, absolute namespace (starting with "/")
 * \param[in] allocator the allocator to use for allocation
 * \returns duplicated string or null if there is an error
 */
const char * rcl_create_node_logger_name(
  const char * node_name,
  const char * node_namespace,
  const rcl_allocator_t * allocator)
{
  // If the namespace is the root namespace ("/"), the logger name is just the node name.
  if (strlen(node_namespace) == 1) {
    return rcutils_strdup(node_name, *allocator);
  }

  // Convert the forward slashes in the namespace to the separator used for logger names.
  // The input namespace has already been expanded and therefore will always be absolute,
  // i.e. it will start with a forward slash, which we want to ignore.
  const char * ns_with_separators = rcutils_repl_str(
    node_namespace + 1,  // Ignore the leading forward slash.
    "/", RCUTILS_LOGGING_SEPARATOR_STRING,
    allocator);
  if (NULL == ns_with_separators) {
    return NULL;
  }

  // Join the namespace and node name to create the logger name.
  char * node_logger_name = rcutils_format_string(
    *allocator, "%s%s%s", ns_with_separators, RCUTILS_LOGGING_SEPARATOR_STRING, node_name);
  allocator->deallocate((char *)ns_with_separators, allocator->state);
  return node_logger_name;
}

/// Return the secure root directory associated with a node given its validated name and namespace.
/**
 * E.g. for a node named "c" in namespace "/a/b", the secure root path will be
 * "a/b/c", where the delimiter "/" is native for target file system (e.g. "\\" for _WIN32).
 * However, this expansion can be overridden by setting the secure node directory environment
 * variable, allowing users to explicitly specify the exact secure root directory to be utilized.
 * Such an override is useful for where the FQN of a node is non-deterministic before runtime,
 * or when testing and using additional tools that may not otherwise not be easily provisioned.
 *
 * \param[in] node_name validated node name (a single token)
 * \param[in] node_namespace validated, absolute namespace (starting with "/")
 * \param[in] allocator the allocator to use for allocation
 * \returns machine specific (absolute) node secure root path or NULL on failure
 */
const char * rcl_get_secure_root(
  const char * node_name,
  const char * node_namespace,
  const rcl_allocator_t * allocator)
{
  bool ros_secure_node_override = true;
  const char * ros_secure_root_env = NULL;
  if (NULL == node_name) {
    return NULL;
  }
  if (rcutils_get_env(ROS_SECURITY_NODE_DIRECTORY_VAR_NAME, &ros_secure_root_env)) {
    return NULL;
  }
  if (!ros_secure_root_env) {
    return NULL;
  }
  size_t ros_secure_root_size = strlen(ros_secure_root_env);
  if (!ros_secure_root_size) {
    // check root directory if node directory environment variable is empty
    if (rcutils_get_env(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME, &ros_secure_root_env)) {
      return NULL;
    }
    if (!ros_secure_root_env) {
      return NULL;
    }
    ros_secure_root_size = strlen(ros_secure_root_env);
    if (!ros_secure_root_size) {
      return NULL;  // environment variable was empty
    } else {
      ros_secure_node_override = false;
    }
  }
  char * node_secure_root = NULL;
  if (ros_secure_node_override) {
    node_secure_root =
      (char *)allocator->allocate(ros_secure_root_size + 1, allocator->state);
    memcpy(node_secure_root, ros_secure_root_env, ros_secure_root_size + 1);
    // TODO(ros2team): This make an assumption on the value and length of the root namespace.
    // This should likely come from another (rcl/rmw?) function for reuse.
    // If the namespace is the root namespace ("/"), the secure root is just the node name.
  } else if (strlen(node_namespace) == 1) {
    node_secure_root = rcutils_join_path(ros_secure_root_env, node_name, *allocator);
  } else {
    char * node_fqn = NULL;
    char * node_root_path = NULL;
    // Combine node namespace with node name
    // TODO(ros2team): remove the hard-coded value of the root namespace.
    node_fqn = rcutils_format_string(*allocator, "%s%s%s", node_namespace, "/", node_name);
    // Get native path, ignore the leading forward slash.
    // TODO(ros2team): remove the hard-coded length, use the length of the root namespace instead.
    node_root_path = rcutils_to_native_path(node_fqn + 1, *allocator);
    node_secure_root = rcutils_join_path(ros_secure_root_env, node_root_path, *allocator);
    allocator->deallocate(node_fqn, allocator->state);
    allocator->deallocate(node_root_path, allocator->state);
  }
  // Check node_secure_root is not NULL before checking directory
  if (NULL == node_secure_root) {
    allocator->deallocate(node_secure_root, allocator->state);
    return NULL;
  } else if (!rcutils_is_directory(node_secure_root)) {
    allocator->deallocate(node_secure_root, allocator->state);
    return NULL;
  }
  return node_secure_root;
}

rcl_node_t
rcl_get_zero_initialized_node()
{
  static rcl_node_t null_node = {0};
  return null_node;
}

rcl_ret_t
rcl_node_init(
  rcl_node_t * node,
  const char * name,
  const char * namespace_,
  rcl_context_t * context,
  const rcl_node_options_t * options)
{
  size_t domain_id = 0;
  const char * ros_domain_id;
  const rmw_guard_condition_t * rmw_graph_guard_condition = NULL;
  rcl_guard_condition_options_t graph_guard_condition_options =
    rcl_guard_condition_get_default_options();
  rcl_ret_t ret;
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  char * remapped_node_name = NULL;

  // Check options and allocator first, so allocator can be used for errors.
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(namespace_, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing node '%s' in namespace '%s'", name, namespace_);
  if (node->impl) {
    RCL_SET_ERROR_MSG("node already initialized, or struct memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  // Make sure rcl has been initialized.
  RCL_CHECK_FOR_NULL_WITH_MSG(
    context, "given context in options is NULL", return RCL_RET_INVALID_ARGUMENT);
  if (!rcl_context_is_valid(context)) {
    RCL_SET_ERROR_MSG(
      "the given context is not valid, "
      "either rcl_init() was not called or rcl_shutdown() was called.");
    return RCL_RET_NOT_INIT;
  }
  // Make sure the node name is valid before allocating memory.
  int validation_result = 0;
  ret = rmw_validate_node_name(name, &validation_result, NULL);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return ret;
  }
  if (validation_result != RMW_NODE_NAME_VALID) {
    const char * msg = rmw_node_name_validation_result_string(validation_result);
    RCL_SET_ERROR_MSG(msg);
    return RCL_RET_NODE_INVALID_NAME;
  }

  // Process the namespace.
  size_t namespace_length = strlen(namespace_);
  const char * local_namespace_ = namespace_;
  bool should_free_local_namespace_ = false;
  // If the namespace is just an empty string, replace with "/"
  if (namespace_length == 0) {
    // Have this special case to avoid a memory allocation when "" is passed.
    local_namespace_ = "/";
  }

  // If the namespace does not start with a /, add one.
  if (namespace_length > 0 && namespace_[0] != '/') {
    local_namespace_ = rcutils_format_string(*allocator, "/%s", namespace_);
    RCL_CHECK_FOR_NULL_WITH_MSG(
      local_namespace_,
      "failed to format node namespace string",
      ret = RCL_RET_BAD_ALLOC; goto cleanup);
    should_free_local_namespace_ = true;
  }
  // Make sure the node namespace is valid.
  validation_result = 0;
  ret = rmw_validate_namespace(local_namespace_, &validation_result, NULL);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    goto cleanup;
  }
  if (validation_result != RMW_NAMESPACE_VALID) {
    const char * msg = rmw_namespace_validation_result_string(validation_result);
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("%s, result: %d", msg, validation_result);

    ret = RCL_RET_NODE_INVALID_NAMESPACE;
    goto cleanup;
  }

  // Allocate space for the implementation struct.
  node->impl = (rcl_node_impl_t *)allocator->allocate(sizeof(rcl_node_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl, "allocating memory failed", ret = RCL_RET_BAD_ALLOC; goto cleanup);
  node->impl->rmw_node_handle = NULL;
  node->impl->graph_guard_condition = NULL;
  node->impl->logger_name = NULL;
  node->impl->options = rcl_node_get_default_options();
  node->context = context;
  // Initialize node impl.
  ret = rcl_node_options_copy(options, &(node->impl->options));
  if (RCL_RET_OK != ret) {
    goto fail;
  }

  // Remap the node name and namespace if remap rules are given
  rcl_arguments_t * global_args = NULL;
  if (node->impl->options.use_global_arguments) {
    global_args = &(node->context->global_arguments);
  }
  ret = rcl_remap_node_name(
    &(node->impl->options.arguments), global_args, name, *allocator,
    &remapped_node_name);
  if (RCL_RET_OK != ret) {
    goto fail;
  } else if (NULL != remapped_node_name) {
    name = remapped_node_name;
  }
  char * remapped_namespace = NULL;
  ret = rcl_remap_node_namespace(
    &(node->impl->options.arguments), global_args, name,
    *allocator, &remapped_namespace);
  if (RCL_RET_OK != ret) {
    goto fail;
  } else if (NULL != remapped_namespace) {
    if (should_free_local_namespace_) {
      allocator->deallocate((char *)local_namespace_, allocator->state);
    }
    should_free_local_namespace_ = true;
    local_namespace_ = remapped_namespace;
  }

  // node logger name
  node->impl->logger_name = rcl_create_node_logger_name(name, local_namespace_, allocator);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->logger_name, "creating logger name failed", goto fail);

  // node rmw_node_handle
  if (node->impl->options.domain_id == RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID) {
    // Find the domain ID set by the environment.
    ret = rcl_impl_getenv("ROS_DOMAIN_ID", &ros_domain_id);
    if (ret != RCL_RET_OK) {
      goto fail;
    }
    if (ros_domain_id) {
      unsigned long number = strtoul(ros_domain_id, NULL, 0);  // NOLINT(runtime/int)
      if (number == ULONG_MAX) {
        RCL_SET_ERROR_MSG("failed to interpret ROS_DOMAIN_ID as integral number");
        goto fail;
      }
      domain_id = (size_t)number;
    }
  } else {
    domain_id = node->impl->options.domain_id;
  }
  // actual domain id
  node->impl->actual_domain_id = domain_id;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Using domain ID of '%zu'", domain_id);

  const char * ros_security_enable = NULL;
  const char * ros_enforce_security = NULL;

  if (rcutils_get_env(ROS_SECURITY_ENABLE_VAR_NAME, &ros_security_enable)) {
    RCL_SET_ERROR_MSG(
      "Environment variable " RCUTILS_STRINGIFY(ROS_SECURITY_ENABLE_VAR_NAME)
      " could not be read");
    ret = RCL_RET_ERROR;
    goto fail;
  }

  bool use_security = (0 == strcmp(ros_security_enable, "true"));
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Using security: %s", use_security ? "true" : "false");

  if (rcutils_get_env(ROS_SECURITY_STRATEGY_VAR_NAME, &ros_enforce_security)) {
    RCL_SET_ERROR_MSG(
      "Environment variable " RCUTILS_STRINGIFY(ROS_SECURITY_STRATEGY_VAR_NAME)
      " could not be read");
    ret = RCL_RET_ERROR;
    goto fail;
  }

  rmw_node_security_options_t node_security_options =
    rmw_get_zero_initialized_node_security_options();
  node_security_options.enforce_security = (0 == strcmp(ros_enforce_security, "Enforce")) ?
    RMW_SECURITY_ENFORCEMENT_ENFORCE : RMW_SECURITY_ENFORCEMENT_PERMISSIVE;

  if (!use_security) {
    node_security_options.enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
  } else {  // if use_security
    // File discovery magic here
    const char * node_secure_root = rcl_get_secure_root(name, local_namespace_, allocator);
    if (node_secure_root) {
      node_security_options.security_root_path = node_secure_root;
    } else {
      if (RMW_SECURITY_ENFORCEMENT_ENFORCE == node_security_options.enforce_security) {
        RCL_SET_ERROR_MSG(
          "SECURITY ERROR: unable to find a folder matching the node name in the "
          RCUTILS_STRINGIFY(ROS_SECURITY_NODE_DIRECTORY_VAR_NAME)
          " or "
          RCUTILS_STRINGIFY(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME)
          " directories while the requested security strategy requires it");
        ret = RCL_RET_ERROR;
        goto cleanup;
      }
    }
  }
  node->impl->rmw_node_handle = rmw_create_node(
    &(node->context->impl->rmw_context),
    name, local_namespace_, domain_id, &node_security_options);

  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->rmw_node_handle, rmw_get_error_string().str, goto fail);
  // graph guard condition
  rmw_graph_guard_condition = rmw_node_get_graph_guard_condition(node->impl->rmw_node_handle);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    rmw_graph_guard_condition, rmw_get_error_string().str, goto fail);

  node->impl->graph_guard_condition = (rcl_guard_condition_t *)allocator->allocate(
    sizeof(rcl_guard_condition_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->graph_guard_condition,
    "allocating memory failed",
    goto fail);
  *node->impl->graph_guard_condition = rcl_get_zero_initialized_guard_condition();
  graph_guard_condition_options.allocator = *allocator;
  ret = rcl_guard_condition_init_from_rmw(
    node->impl->graph_guard_condition,
    rmw_graph_guard_condition,
    context,
    graph_guard_condition_options);
  if (ret != RCL_RET_OK) {
    // error message already set
    goto fail;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Node initialized");
  ret = RCL_RET_OK;
  goto cleanup;
fail:
  if (node->impl) {
    if (node->impl->logger_name) {
      allocator->deallocate((char *)node->impl->logger_name, allocator->state);
    }
    if (node->impl->rmw_node_handle) {
      ret = rmw_destroy_node(node->impl->rmw_node_handle);
      if (ret != RMW_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
          ROS_PACKAGE_NAME,
          "failed to fini rmw node in error recovery: %s", rmw_get_error_string().str
        );
      }
    }
    if (node->impl->graph_guard_condition) {
      ret = rcl_guard_condition_fini(node->impl->graph_guard_condition);
      if (ret != RCL_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
          ROS_PACKAGE_NAME,
          "failed to fini guard condition in error recovery: %s", rcl_get_error_string().str
        );
      }
      allocator->deallocate(node->impl->graph_guard_condition, allocator->state);
    }
    if (NULL != node->impl->options.arguments.impl) {
      ret = rcl_arguments_fini(&(node->impl->options.arguments));
      if (ret != RCL_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
          ROS_PACKAGE_NAME,
          "failed to fini arguments in error recovery: %s", rcl_get_error_string().str
        );
      }
    }
    allocator->deallocate(node->impl, allocator->state);
  }
  *node = rcl_get_zero_initialized_node();

  ret = fail_ret;
  // fall through from fail -> cleanup
cleanup:
  if (should_free_local_namespace_) {
    allocator->deallocate((char *)local_namespace_, allocator->state);
    local_namespace_ = NULL;
  }
  if (NULL != remapped_node_name) {
    allocator->deallocate(remapped_node_name, allocator->state);
  }
  return ret;
}

rcl_ret_t
rcl_node_fini(rcl_node_t * node)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing node");
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_NODE_INVALID);
  if (!node->impl) {
    // Repeat calls to fini or calling fini on a zero initialized node is ok.
    return RCL_RET_OK;
  }
  rcl_allocator_t allocator = node->impl->options.allocator;
  rcl_ret_t result = RCL_RET_OK;
  rmw_ret_t rmw_ret = rmw_destroy_node(node->impl->rmw_node_handle);
  if (rmw_ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    result = RCL_RET_ERROR;
  }
  rcl_ret_t rcl_ret = rcl_guard_condition_fini(node->impl->graph_guard_condition);
  if (rcl_ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    result = RCL_RET_ERROR;
  }
  allocator.deallocate(node->impl->graph_guard_condition, allocator.state);
  // assuming that allocate and deallocate are ok since they are checked in init
  allocator.deallocate((char *)node->impl->logger_name, allocator.state);
  if (NULL != node->impl->options.arguments.impl) {
    rcl_ret_t ret = rcl_arguments_fini(&(node->impl->options.arguments));
    if (ret != RCL_RET_OK) {
      return ret;
    }
  }
  allocator.deallocate(node->impl, allocator.state);
  node->impl = NULL;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Node finalized");
  return result;
}

bool
rcl_node_is_valid_except_context(const rcl_node_t * node)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(node, "rcl node pointer is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(node->impl, "rcl node implementation is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->rmw_node_handle, "rcl node's rmw handle is invalid", return false);
  return true;
}

bool
rcl_node_is_valid(const rcl_node_t * node)
{
  bool result = rcl_node_is_valid_except_context(node);
  if (!result) {
    return result;
  }
  if (!rcl_context_is_valid(node->context)) {
    RCL_SET_ERROR_MSG("rcl node's context is invalid");
    return false;
  }
  return true;
}

rcl_node_options_t
rcl_node_get_default_options()
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_node_options_t default_options = {
    .domain_id = RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID,
    .use_global_arguments = true,
  };
  // Must set the allocator after because it is not a compile time constant.
  default_options.allocator = rcl_get_default_allocator();
  default_options.arguments = rcl_get_zero_initialized_arguments();
  return default_options;
}

rcl_ret_t
rcl_node_options_copy(
  const rcl_node_options_t * options,
  rcl_node_options_t * options_out)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options_out, RCL_RET_INVALID_ARGUMENT);
  if (options_out == options) {
    RCL_SET_ERROR_MSG("Attempted to copy options into itself");
    return RCL_RET_INVALID_ARGUMENT;
  }
  options_out->domain_id = options->domain_id;
  options_out->allocator = options->allocator;
  options_out->use_global_arguments = options->use_global_arguments;
  if (NULL != options->arguments.impl) {
    rcl_ret_t ret = rcl_arguments_copy(&(options->arguments), &(options_out->arguments));
    return ret;
  }
  return RCL_RET_OK;
}

const char *
rcl_node_get_name(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return NULL;  // error already set
  }
  return node->impl->rmw_node_handle->name;
}

const char *
rcl_node_get_namespace(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return NULL;  // error already set
  }
  return node->impl->rmw_node_handle->namespace_;
}

const rcl_node_options_t *
rcl_node_get_options(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return NULL;  // error already set
  }
  return &node->impl->options;
}

rcl_ret_t
rcl_node_get_domain_id(const rcl_node_t * node, size_t * domain_id)
{
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(domain_id, RCL_RET_INVALID_ARGUMENT);
  *domain_id = node->impl->actual_domain_id;
  return RCL_RET_OK;
}

rmw_node_t *
rcl_node_get_rmw_handle(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return NULL;  // error already set
  }
  return node->impl->rmw_node_handle;
}

uint64_t
rcl_node_get_rcl_instance_id(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return 0;  // error already set
  }
  return rcl_context_get_instance_id(node->context);
}

const struct rcl_guard_condition_t *
rcl_node_get_graph_guard_condition(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return NULL;  // error already set
  }
  return node->impl->graph_guard_condition;
}

const char *
rcl_node_get_logger_name(const rcl_node_t * node)
{
  if (!rcl_node_is_valid_except_context(node)) {
    return NULL;  // error already set
  }
  return node->impl->logger_name;
}

#ifdef __cplusplus
}
#endif
