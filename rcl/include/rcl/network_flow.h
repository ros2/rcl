// Copyright 2020 Ericsson AB
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


#ifndef RCL__NETWORK_FLOW_H_
#define RCL__NETWORK_FLOW_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <rmw/network_flow.h>
#include <rmw/network_flow_array.h>

#include "rcl/allocator.h"
#include "rcl/arguments.h"
#include "rcl/context.h"
#include "rcl/macros.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

typedef rmw_network_flow_t rcl_network_flow_t;
typedef rmw_network_flow_array_t rcl_network_flow_array_t;
typedef rmw_transport_protocol_t rcl_transport_protocol_t;
typedef rmw_internet_protocol_t rcl_internet_protocol_t;

#define rcl_get_zero_initialized_network_flow_array \
  rmw_get_zero_initialized_network_flow_array
#define rcl_network_flow_array_fini rmw_network_flow_array_fini

#define rcl_network_flow_get_transport_protocol_string \
  rmw_network_flow_get_transport_protocol_string
#define rcl_network_flow_get_internet_protocol_string \
  rmw_network_flow_get_internet_protocol_string

/// Get network flows of a publisher
/**
 * Query the underlying middleware for a given publisher's network flows
 *
 * The `publisher` argument must point to a valid publisher.
 *
 * The `allocator` argument must be a valid allocator.
 *
 * The `network_flow_array` argument must be allocated and zero-initialized.
 * The function returns network flows in the `network_flow_array` argument,
 * using the allocator to allocate memory for the `network_flow_array`
 * argument's internal data structures whenever required. The caller is
 * reponsible for memory deallocation by passing the `network_flow_array`
 * argument to `rcl_network_flow_array_fini` function.
 *
 * \todo TODO(anamud): Are the same network flows returned by successive calls
 * to this function?
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] publisher the publisher instance to inspect
 * \param[in] allocator allocator to be used when allocating space for network_flow_array_t
 * \param[out] network_flow_array the network flows
 * \return `RCL_RET_OK` if successful, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any argument is null, or
 * \return `RCL_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCL_RET_UNSUPPORTED` if not supported, or
 * \return `RCL_RET_ERROR` if an unexpected error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_publisher_get_network_flow(
  const rcl_publisher_t * publisher,
  rcutils_allocator_t * allocator,
  rcl_network_flow_array_t * network_flow_array);

/// Get network flows of a subscription
/**
 * Query the underlying middleware for a given subscription's network flows
 *
 * The `subscription` argument must point to a valid subscription.
 *
 * The `allocator` argument must be a valid allocator.
 *
 * The `network_flow_array` argument must be allocated and zero-initialized.
 * The function returns network flows in the `network_flow_array` argument,
 * using the allocator to allocate memory for the `network_flow_array`
 * argument's internal data structures whenever required. The caller is
 * reponsible for memory deallocation by passing the `network_flow_array`
 * argument to `rcl_network_flow_array_fini` function.
 *
 * \todo TODO(anamud): Are the same network flows returned by successive calls
 * to this function?
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Maybe [1]
 * <i>[1] implementation may need to protect the data structure with a lock</i>
 *
 * \param[in] subscription the subscription instance to inspect
 * \param[in] allocator allocator to be used when allocating space for network_flow_array_t
 * \param[out] network_flow_array the network flows
 * \return `RCL_RET_OK` if successful, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any argument is null, or
 * \return `RCL_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCL_RET_UNSUPPORTED` if not supported, or
 * \return `RCL_RET_ERROR` if an unexpected error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_subscription_get_network_flow(
  const rcl_subscription_t * subscription,
  rcutils_allocator_t * allocator,
  rcl_network_flow_array_t * network_flow_array);

#ifdef __cplusplus
}
#endif

#endif  // RCL__NETWORK_FLOW_H_
