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

#if __cplusplus
extern "C"
{
#endif

#include "rcl/node.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/rcl.h"
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


#define ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "ROS_SECURITY_ROOT_DIRECTORY"
#define ROS_SECURITY_STRATEGY_VAR_NAME "ROS_SECURITY_STRATEGY"
#define ROS_SECURITY_ENABLE_VAR_NAME "ROS_SECURITY_ENABLE"

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  size_t actual_domain_id;
  rmw_node_t * rmw_node_handle;
  uint64_t rcl_instance_id;
  rcl_guard_condition_t * graph_guard_condition;
  const char * logger_name;
} rcl_node_impl_t;


const char * rcl_get_secure_root(const char * node_name)
{
  const char * ros_secure_root_env = NULL;
  if (NULL == node_name) {
    return NULL;
  }
  if (rcutils_get_env(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME, &ros_secure_root_env)) {
    return NULL;
  }
  if (!ros_secure_root_env) {
    return NULL;  // environment variable not defined
  }
  size_t ros_secure_root_size = strlen(ros_secure_root_env);
  if (!ros_secure_root_size) {
    return NULL;  // environment variable was empty
  }
  char * node_secure_root = rcutils_join_path(ros_secure_root_env, node_name);
  if (!rcutils_is_directory(node_secure_root)) {
    free(node_secure_root);
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
  const rcl_node_options_t * options)
{
  size_t domain_id = 0;
  const char * ros_domain_id;
  const rmw_guard_condition_t * rmw_graph_guard_condition = NULL;
  rcl_guard_condition_options_t graph_guard_condition_options =
    rcl_guard_condition_get_default_options();
  rcl_ret_t ret;
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  // Check options and allocator first, so allocator can be used for errors.
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(name, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(namespace_, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, *allocator);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing node '%s' in namespace '%s'", name, namespace_)
  if (node->impl) {
    RCL_SET_ERROR_MSG("node already initialized, or struct memory was unintialized", *allocator);
    return RCL_RET_ALREADY_INIT;
  }
  // Make sure rcl has been initialized.
  if (!rcl_ok()) {
    RCL_SET_ERROR_MSG("rcl_init() has not been called", *allocator);
    return RCL_RET_NOT_INIT;
  }
  // Make sure the node name is valid before allocating memory.
  int validation_result = 0;
  ret = rmw_validate_node_name(name, &validation_result, NULL);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    return ret;
  }
  if (validation_result != RMW_NODE_NAME_VALID) {
    const char * msg = rmw_node_name_validation_result_string(validation_result);
    RCL_SET_ERROR_MSG(msg, *allocator);
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
    // TODO(wjwwood): replace with generic strcat that takes an allocator once available
    // length + 2, because new leading / and terminating \0
    char * temp = (char *)allocator->allocate(namespace_length + 2, allocator->state);
    RCL_CHECK_FOR_NULL_WITH_MSG(
      temp, "allocating memory failed", return RCL_RET_BAD_ALLOC, *allocator);
    temp[0] = '/';
    memcpy(temp + 1, namespace_, strlen(namespace_) + 1);
    local_namespace_ = temp;
    should_free_local_namespace_ = true;
  }
  // Make sure the node namespace is valid.
  validation_result = 0;
  ret = rmw_validate_namespace(local_namespace_, &validation_result, NULL);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), *allocator);
    if (should_free_local_namespace_) {
      allocator->deallocate((char *)local_namespace_, allocator->state);
    }
    return ret;
  }
  if (validation_result != RMW_NAMESPACE_VALID) {
    const char * msg = rmw_namespace_validation_result_string(validation_result);
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING((*allocator), "%s, result: %d", msg, validation_result);

    if (should_free_local_namespace_) {
      allocator->deallocate((char *)local_namespace_, allocator->state);
    }
    return RCL_RET_NODE_INVALID_NAMESPACE;
  }

  // Allocate space for the implementation struct.
  node->impl = (rcl_node_impl_t *)allocator->allocate(sizeof(rcl_node_impl_t), allocator->state);
  if (!node->impl && should_free_local_namespace_) {
    allocator->deallocate((char *)local_namespace_, allocator->state);
  }
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC, *allocator);
  node->impl->rmw_node_handle = NULL;
  node->impl->graph_guard_condition = NULL;
  // Initialize node impl.
  // node options (assume it is trivially copyable)
  node->impl->options = *options;

  // node logger name
  char * node_name_with_ns = rcutils_format_string(*allocator, "%s/%s", local_namespace_, name);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node_name_with_ns,
    "formatting node namespace + name string failed",
    goto fail,
    *allocator
  );
  // remove leading slashes
  while (0 == rcutils_find(node_name_with_ns, '/')) {
    node_name_with_ns = rcutils_strdup(node_name_with_ns + 1, *allocator);
    RCL_CHECK_FOR_NULL_WITH_MSG(
      node_name_with_ns,
      "allocating memory failed",
      goto fail,
      *allocator
    );
  }
  // convert slashes to dot separators
  node->impl->logger_name = rcutils_repl_str(node_name_with_ns, "/", ".", allocator);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->logger_name,
    "allocating memory failed",
    goto fail,
    *allocator
  );

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
        RCL_SET_ERROR_MSG("failed to interpret ROS_DOMAIN_ID as integral number", *allocator);
        goto fail;
      }
      domain_id = (size_t)number;
    }
  } else {
    domain_id = node->impl->options.domain_id;
  }
  // actual domain id
  node->impl->actual_domain_id = domain_id;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Using domain ID of '%zu'", domain_id)

  const char * ros_security_enable = NULL;
  const char * ros_enforce_security = NULL;

  if (rcutils_get_env(ROS_SECURITY_ENABLE_VAR_NAME, &ros_security_enable)) {
    RCL_SET_ERROR_MSG(
      "Environment variable " RCUTILS_STRINGIFY(ROS_SECURITY_ENABLE_VAR_NAME)
      " could not be read", rcl_get_default_allocator());
    if (should_free_local_namespace_) {
      allocator->deallocate((char *)local_namespace_, allocator->state);
    }
    return RCL_RET_ERROR;
  }

  bool use_security = (0 == strcmp(ros_security_enable, "true"));
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Using security: %s", use_security ? "true" : "false")

  if (rcutils_get_env(ROS_SECURITY_STRATEGY_VAR_NAME, &ros_enforce_security)) {
    RCL_SET_ERROR_MSG(
      "Environment variable " RCUTILS_STRINGIFY(ROS_SECURITY_STRATEGY_VAR_NAME)
      " could not be read", rcl_get_default_allocator());
    if (should_free_local_namespace_) {
      allocator->deallocate((char *)local_namespace_, allocator->state);
    }
    return RCL_RET_ERROR;
  }

  rmw_node_security_options_t node_security_options =
    rmw_get_zero_initialized_node_security_options();
  node_security_options.enforce_security = (0 == strcmp(ros_enforce_security, "Enforce")) ?
    RMW_SECURITY_ENFORCEMENT_ENFORCE : RMW_SECURITY_ENFORCEMENT_PERMISSIVE;

  if (!use_security) {
    node_security_options.enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
  } else {  // if use_security
    // File discovery magic here
    const char * node_secure_root = rcl_get_secure_root(name);
    if (node_secure_root) {
      node_security_options.security_root_path = node_secure_root;
    } else {
      if (RMW_SECURITY_ENFORCEMENT_ENFORCE == node_security_options.enforce_security) {
        RCL_SET_ERROR_MSG(
          "SECURITY ERROR: unable to find a folder matching the node name in the "
          RCUTILS_STRINGIFY(ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME)
          " directory while the requested security strategy requires it", *allocator);
        if (should_free_local_namespace_) {
          allocator->deallocate((char *)local_namespace_, allocator->state);
        }
        return RCL_RET_ERROR;
      }
    }
  }
  node->impl->rmw_node_handle = rmw_create_node(
    name, local_namespace_, domain_id, &node_security_options);

  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->rmw_node_handle, rmw_get_error_string_safe(), goto fail, *allocator);
  // free local_namespace_ if necessary
  if (should_free_local_namespace_) {
    allocator->deallocate((char *)local_namespace_, allocator->state);
    should_free_local_namespace_ = false;
  }
  // instance id
  node->impl->rcl_instance_id = rcl_get_instance_id();
  // graph guard condition
  rmw_graph_guard_condition = rmw_node_get_graph_guard_condition(node->impl->rmw_node_handle);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    rmw_graph_guard_condition, rmw_get_error_string_safe(), goto fail, *allocator);

  node->impl->graph_guard_condition = (rcl_guard_condition_t *)allocator->allocate(
    sizeof(rcl_guard_condition_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->graph_guard_condition,
    "allocating memory failed",
    goto fail,
    *allocator
  );
  *node->impl->graph_guard_condition = rcl_get_zero_initialized_guard_condition();
  graph_guard_condition_options.allocator = *allocator;
  ret = rcl_guard_condition_init_from_rmw(
    node->impl->graph_guard_condition,
    rmw_graph_guard_condition,
    graph_guard_condition_options);
  if (ret != RCL_RET_OK) {
    // error message already set
    goto fail;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Node initialized")
  return RCL_RET_OK;
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
          "failed to fini rmw node in error recovery: %s", rmw_get_error_string_safe()
        )
      }
    }
    if (node->impl->graph_guard_condition) {
      ret = rcl_guard_condition_fini(node->impl->graph_guard_condition);
      if (ret != RCL_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
          ROS_PACKAGE_NAME,
          "failed to fini guard condition in error recovery: %s", rcl_get_error_string_safe()
        )
      }
      allocator->deallocate(node->impl->graph_guard_condition, allocator->state);
    }
    allocator->deallocate(node->impl, allocator->state);
  }
  *node = rcl_get_zero_initialized_node();

  if (should_free_local_namespace_) {
    allocator->deallocate((char *)local_namespace_, allocator->state);
    local_namespace_ = NULL;
  }
  return fail_ret;
}

rcl_ret_t
rcl_node_fini(rcl_node_t * node)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing node")
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (!node->impl) {
    // Repeat calls to fini or calling fini on a zero initialized node is ok.
    return RCL_RET_OK;
  }
  rcl_allocator_t allocator = node->impl->options.allocator;
  rcl_ret_t result = RCL_RET_OK;
  rmw_ret_t rmw_ret = rmw_destroy_node(node->impl->rmw_node_handle);
  if (rmw_ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), allocator);
    result = RCL_RET_ERROR;
  }
  rcl_ret_t rcl_ret = rcl_guard_condition_fini(node->impl->graph_guard_condition);
  if (rcl_ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe(), allocator);
    result = RCL_RET_ERROR;
  }
  allocator.deallocate(node->impl->graph_guard_condition, allocator.state);
  // assuming that allocate and deallocate are ok since they are checked in init
  allocator.deallocate((char *)node->impl->logger_name, allocator.state);
  allocator.deallocate(node->impl, allocator.state);
  node->impl = NULL;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Node finalized")
  return result;
}

bool
rcl_node_is_valid(const rcl_node_t * node, rcl_allocator_t * error_msg_allocator)
{
  rcl_allocator_t alloc = error_msg_allocator ? *error_msg_allocator : rcl_get_default_allocator();
  RCL_CHECK_ALLOCATOR_WITH_MSG(&alloc, "allocator is invalid", return false);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, false, alloc);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl, "rcl node implementation is invalid", return false, alloc);
  if (node->impl->rcl_instance_id != rcl_get_instance_id()) {
    RCL_SET_ERROR_MSG(
      "rcl node is invalid, rcl instance id does not match", alloc);
    return false;
  }
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl->rmw_node_handle, "rcl node's rmw handle is invalid", return false, alloc);
  return true;
}

rcl_node_options_t
rcl_node_get_default_options()
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_node_options_t default_options = {
    .domain_id = RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID,
  };
  // Must set the allocator after because it is not a compile time constant.
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

const char *
rcl_node_get_name(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node, NULL)) {
    return NULL;
  }
  return node->impl->rmw_node_handle->name;
}

const char *
rcl_node_get_namespace(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node, NULL)) {
    return NULL;
  }
  return node->impl->rmw_node_handle->namespace_;
}

const rcl_node_options_t *
rcl_node_get_options(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node, NULL)) {
    return NULL;
  }
  return &node->impl->options;
}

rcl_ret_t
rcl_node_get_domain_id(const rcl_node_t * node, size_t * domain_id)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(domain_id, RCL_RET_INVALID_ARGUMENT, node_options->allocator);
  *domain_id = node->impl->actual_domain_id;
  return RCL_RET_OK;
}

rmw_node_t *
rcl_node_get_rmw_handle(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node, NULL)) {
    return NULL;
  }
  return node->impl->rmw_node_handle;
}

uint64_t
rcl_node_get_rcl_instance_id(const rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, 0, rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node->impl, "node implementation is invalid", return 0, rcl_get_default_allocator());
  return node->impl->rcl_instance_id;
}

const struct rcl_guard_condition_t *
rcl_node_get_graph_guard_condition(const rcl_node_t * node)
{
  if (!rcl_node_is_valid(node, NULL)) {
    return NULL;
  }
  return node->impl->graph_guard_condition;
}

#if __cplusplus
}
#endif
