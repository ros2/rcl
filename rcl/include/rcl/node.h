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

#ifndef RCL__NODE_H_
#define RCL__NODE_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/allocator.h"
#include "rcl/types.h"

// Intentional underflow to get max size_t.
#define RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID (size_t)-1

struct rcl_node_impl_t;

/// Handle for a ROS node.
/* This handle can be in one a few stats, which are referred to below:
 * - uninitialized: memory allocated, but not initialized to anything
 * - invalid: memory allocated but not usable
 *   - either zero initialized or shutdown (rcl_fini() or rcl_node_fini())
 * - valid: memory allocated and usable
 */
typedef struct rcl_node_t {
  /// Private implementation pointer.
  struct rcl_node_impl_t * impl;
} rcl_node_t;

typedef struct rcl_node_options_t {
  // bool anonymous_name;
  // rmw_qos_profile_t parameter_qos;
  /// If true, no parameter infrastructure will be setup.
  bool no_parameters;
  /// If set, then this value overrides the ROS_DOMAIN_ID environment variable.
  /* It defaults to RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID, which will cause the
   * node to use the ROS domain ID set in the ROS_DOMAIN_ID environment
   * variable, or on some systems 0 if the environment variable is not set.
   *
   * \TODO(wjwwood): Should we put a limit on the ROS_DOMAIN_ID value, that way
   *                 we can have a safe value for the default
   *                 RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID? (currently max size_t)
   */
  size_t domain_id;
  /// Custom allocator used for incidental allocations, e.g. node name string.
  rcl_allocator_t allocator;
} rcl_node_options_t;

/// Return a rcl_node_t struct with members initialized to NULL.
/* Should be called to get rcl_node_t before passing to rcl_node_init().
 * It's also possible to use calloc() instead of this if the rcl_node is being
 * allocated on the heap.
 */
rcl_node_t
rcl_get_zero_initialized_node();

/// Initialize a ROS node.
/* Calling this on a rcl_node_t makes it a valid node handle until rcl_fini
 * is called or until rcl_node_fini is called on it.
 *
 * After calling the ROS node object can be used to create other middleware
 * primitives like publishers, services, parameters, and etc.
 *
 * The name of the node cannot coincide with another node of the same name.
 * If a node of the same name is already in the domain, it will be shutdown.
 *
 * A node contains infrastructure for ROS parameters, which include advertising
 * publishers and service servers.
 * This function will create those external parameter interfaces even if
 * parameters are not used later.
 *
 * The rcl_node_t given must be allocated and zero initalized.
 * Passing an rcl_node_t which has already had this function called on it, more
 * recently than rcl_node_fini, will fail.
 * An allocated rcl_node_t with uninitialized memory is undefined behavior.
 *
 * Expected usage:
 *
 *    rcl_node_t node = rcl_get_zero_initialized_node();
 *    rcl_node_options_t * node_ops = rcl_node_get_default_options();
 *    rcl_ret_t ret = rcl_node_init(&node, "node_name", node_ops);
 *    // ... error handling and then use the node, but finally deinitialize it:
 *    ret = rcl_node_fini(&node);
 *    // ... error handling for rcl_node_fini()
 *
 * This function is not thread-safe.
 *
 * \pre the node handle must be allocated, zero initialized, and invalid
 * \post the node handle is valid and can be used to in other rcl_* functions
 *
 * \param[inout] node a preallocated node structure
 * \param[in] name the name of the node
 * \param[in] options the node options; pass null to use default options
 * \return RCL_RET_OK if node was initialized successfully, or
 *         RCL_RET_ALREADY_INIT if the node has already be initialized, or
 *         RCL_RET_INVALID_ARGUMENT if any arugments are invalid, or
 *         RCL_RET_BAD_ALLOC if allocating memory failed, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_node_init(rcl_node_t * node, const char * name, const rcl_node_options_t * options);

/// Finalized a rcl_node_t.
/* Shuts down any automatically created infrastructure and deallocates memory.
 * After calling, the rcl_node_t can be safely deallocated.
 *
 * Any middleware primitives created by the user, e.g. publishers, services, etc.,
 * are invalid after deinitialization.
 *
 * This function is not thread-safe.
 *
 * \param[in] node handle to the node to be finalized
 * \return RCL_RET_OK if node was finalized successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arugments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_node_fini(rcl_node_t * node);

/// Return the default node options in a rcl_node_options_t.
rcl_node_options_t
rcl_node_get_default_options();

/// Get the name of the node.
/* This function returns the node's internal name string.
 * This function can fail, and therefore return NULL, if:
 *   - node is NULL
 *   - node has not been initialized (the implementation is invalid)
 *
 * The returned string is only valid as long as the given rcl_node_t is valid.
 * The value of the string may change if the value in the rcl_node_t changes,
 * and therefore copying the string is recommended if this is a concern.
 *
 * \param[in] node pointer to the node
 * \return name string if successful, otherwise NULL
 */
const char *
rcl_node_get_name(const rcl_node_t * node);

/// Return the rcl node options.
/* This function returns the node's internal options struct.
 * This function can fail, and therefore return NULL, if:
 *   - node is NULL
 *   - node has not been initialized (the implementation is invalid)
 *
 * The returned struct is only valid as long as the given rcl_node_t is valid.
 * The values in the struct may change if the options of the rcl_node_t changes,
 * and therefore copying the struct is recommended if this is a concern.
 *
 * \param[in] node pointer to the node
 * \return options struct if successful, otherwise NULL
 */
const rcl_node_options_t *
rcl_node_get_options(const rcl_node_t * node);

/// Return the ROS domain ID that the node is using.
/* This function returns the ROS domain ID that the node is in.
 *
 * This function should be used to determine what domain_id was used rather
 * than checking the domin_id field in the node options, because if
 * RCL_NODE_OPTIONS_DEFAULT_DOMAIN_ID is used when creating the node then
 * it is not changed after creation, but this function will return the actual
 * domain_id used.
 *
 * The domain_id field must point to an allocated size_t object to which the
 * ROS domain ID will be written.
 *
 * \param[in] node the handle to the node being queried
 * \return RCL_RET_OK if node the domain ID was retrieved successfully, or
 *         RCL_RET_NODE_INVALID if the node is invalid, or
 *         RCL_RET_INVALID_ARGUMENT if any arugments are invalid, or
 *         RCL_RET_ERROR if an unspecified error occurs.
 */
rcl_ret_t
rcl_node_get_domain_id(const rcl_node_t * node, size_t * domain_id);

/// Return the rmw node handle.
/* The handle returned is a pointer to the internally held rmw handle.
 * This function can fail, and therefore return NULL, if:
 *   - node is NULL
 *   - node has not been initialized (the implementation is invalid)
 *
 * The returned handle is only valid as long as the given node is valid.
 *
 * \TODO(wjwwood) should the return value of this be const?
 *
 * \param[in] node pointer to the rcl node
 * \return rmw node handle if successful, otherwise NULL
 */
rmw_node_t *
rcl_node_get_rmw_node_handle(const rcl_node_t * node);

/// Return the associated rcl instance id.
/* This id is stored when rcl_node_init is called and can be compared with the
 * value returned by rcl_get_instance_id() to check if this node was created in
 * the current rcl context (since the latest call to rcl_init().
 *
 * This function can fail, and therefore return 0, if:
 *   - node is NULL
 *   - node has not been initialized (the implementation is invalid)
 *
 * This function will succeed, however, even if rcl_fini has been called since
 * the node was created.
 *
 * \param[in] node pointer to the rcl node
 * \return rcl instance id captured at node creation or 0 if there was an error
 */
uint64_t
rcl_node_get_rcl_instance_id(const rcl_node_t * node);

#if __cplusplus
}
#endif

#endif  // RCL__NODE_H_
