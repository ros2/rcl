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

#ifndef RCL__SECURITY_H_
#define RCL__SECURITY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "rcl/allocator.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"
#include "rmw/security_options.h"

#ifndef ROS_SECURITY_NODE_DIRECTORY_VAR_NAME
  #define ROS_SECURITY_NODE_DIRECTORY_VAR_NAME "ROS_SECURITY_NODE_DIRECTORY"
#endif

#ifndef ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME
  #define ROS_SECURITY_ROOT_DIRECTORY_VAR_NAME "ROS_SECURITY_ROOT_DIRECTORY"
#endif

#ifndef ROS_SECURITY_LOOKUP_TYPE_VAR_NAME
  #define ROS_SECURITY_LOOKUP_TYPE_VAR_NAME "ROS_SECURITY_LOOKUP_TYPE"
#endif

#ifndef ROS_SECURITY_STRATEGY_VAR_NAME
#define ROS_SECURITY_STRATEGY_VAR_NAME "ROS_SECURITY_STRATEGY"
#endif

#ifndef ROS_SECURITY_ENABLE_VAR_NAME
#define ROS_SECURITY_ENABLE_VAR_NAME "ROS_SECURITY_ENABLE"
#endif

/// Init the security options from the values of the environment variables and passed names.
/**
 * Inits the passed security options based on the environment.
 * For more details:
 *  \sa rcl_security_enabled
 *  \sa rcl_get_enforcement_policy
 *  \sa rcl_get_secure_root
 *
 * \param name[in] name used to find the securiy root path.
 * \param namespace_[in] namespace_ used to find the security root path.
 * \param allocator[in] used to do allocations.
 * \param security_options[out] security options that will be configured according to
 *  the environment.
 */
RCL_PUBLIC
rcl_ret_t
rcl_get_security_options_from_environment(
  const char * name,
  const char * namespace_,
  const rcutils_allocator_t * allocator,
  rmw_security_options_t * security_options);

/// Check if security has to be used, according to the environment.
/**
 * If `ROS_SECURITY_ENABLE` environment variable is set to "true", `use_security` will be set to
 * true.
 *
 * \param use_security[out] Must not be NULL.
 * \returns RCL_RET_INVALID_ARGUMENT if an argument is not valid, or
 * \returns RCL_RET_ERROR if an unexpected error happened, or
 * \returns RCL_RET_OK.
 */
RCL_PUBLIC
rcl_ret_t
rcl_security_enabled(bool * use_security);

/// Get security enforcement policy from the environment.
/**
 * Sets `policy` based on the value of `ROS_SECURITY_STRATEGY` environment variable.
 * If `ROS_SECURITY_STRATEGY` is "Enforce", `policy` will be `RMW_SECURITY_ENFORCEMENT_ENFORCE`.
 * If not, `policy` will be `RMW_SECURITY_ENFORCEMENT_PERMISSIVE`.
 *
 * \param policy[out] Must not be NULL.
 * \returns RCL_RET_INVALID_ARGUMENT if an argument is not valid, or
 * \returns RCL_RET_ERROR if an unexpected error happened, or
 * \returns RCL_RET_OK.
 */
RCL_PUBLIC
rcl_ret_t
rcl_get_enforcement_policy(rmw_security_enforcement_policy_t * policy);

/// Return the secure root given a name and namespace.
/**
 * The returned security directory is associated with the node or context depending on the
 * rmw implementation.
 *
 * The value of the environment variable `ROS_SECURITY_ROOT_DIRECTORY` is used as a root.
 * The specific directory to be used, is found from that root using the `name` and `namespace_`
 * passed.
 * E.g. for a node/context named "c" in namespace "/a/b" root "/r", the secure root path will be
 * "/r/a/b/c", where the delimiter "/" is native for target file system (e.g. "\\" for _WIN32).
 *
 * If `ROS_SECURITY_LOOKUP_TYPE_VAR_NAME` is set to `MATCH_PREFIX`, when no exact match is found for
 * the node/context name, a best match would be used instead
 * (by performing longest-prefix matching).
 *
 * Only for rmw implementations that associate a security directory with a node:
 *  However, this expansion can be overridden by setting the secure node directory environment
 *  (`ROS_SECURITY_NODE_DIRECTORY`) variable, allowing users to explicitly specify the exact secure
 *  root directory to be utilized.
 *  Such an override is useful for where the FQN of a node is non-deterministic before runtime,
 *  or when testing and using additional tools that may not otherwise be easily provisioned.
 *
 * \param[in] name validated name (a single token)
 * \param[in] namespace_ validated, absolute namespace (starting with "/")
 * \param[in] allocator the allocator to use for allocation
 * \returns Machine specific (absolute) secure root path or NULL on failure.
 *  Returned pointer must be deallocated by the caller of this function
 */
RCL_PUBLIC
char * rcl_get_secure_root(
  const char * name,
  const char * namespace_,
  const rcl_allocator_t * allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCL__SECURITY_H_
