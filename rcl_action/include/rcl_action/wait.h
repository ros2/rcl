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

#ifndef RCL_ACTION__WAIT_H_
#define RCL_ACTION__WAIT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/action_client.h"
#include "rcl_action/action_server.h"
#include "rcl/wait.h"


/// Add an action client to a wait set.
/**
 * This function will add the underlying service clients and subscriber to the wait set.
 *
 * This function behaves similar to adding subscriptions to the wait set, but will add
 * four elements:
 *
 * - Three service clients
 * - One subscriber
 *
 * \see rcl_wait_set_add_subscription
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] wait_set struct where action client service client and subscription
 *   are to be stored
 * \param[in] action_client the action client to be added to the wait set
 * \return `RCL_RET_OK` if added successfully, or
 * \return `RCL_RET_WAIT_SET_INVALID` if the wait set is zero initialized, or
 * \return `RCL_RET_WAIT_SET_FULL` if the subscription set is full, or
 * \return `RCL_RET_ACTION_CLIENT_INVALID` if the action client is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
*/
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_wait_set_add_action_client(
  rcl_wait_set_t * wait_set,
  const rcl_action_client_t * action_client);

/// Add an action server to a wait set.
/**
 * This function will add the underlying services to the wait set.
 *
 * This function behaves similar to adding services to the wait set, but will add
 * three services.
 *
 * \see rcl_wait_set_add_service
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] wait_set struct where action server services are to be stored
 * \param[in] action_server the action server to be added to the wait set
 * \return `RCL_RET_OK` if added successfully, or
 * \return `RCL_RET_WAIT_SET_INVALID` if the wait set is zero initialized, or
 * \return `RCL_RET_WAIT_SET_FULL` if the subscription set is full, or
 * \return `RCL_RET_ACTION_SERVER_INVALID` if the action server is invalid, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
*/
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_wait_set_add_action_server(
  rcl_wait_set_t * wait_set,
  const rcl_action_server_t * action_server);

#ifdef __cplusplus
}
#endif

#endif  // RCL_ACTION__WAIT_H_
