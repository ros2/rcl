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
#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/types.h"

#include "rcl/error_handling.h"

rcl_action_goal_info_t
rcl_action_get_zero_initialized_goal_info(void)
{
  static rcl_action_goal_info_t goal_info = {{0}, {0, 0}};
  return goal_info;
}

rcl_action_goal_status_array_t
rcl_action_get_zero_initialized_goal_status_array(void)
{
  static rcl_action_goal_status_array_t status_array = {{{0, 0, 0}}, {0, 0, 0, 0, 0}};
  return status_array;
}

rcl_action_cancel_request_t
rcl_action_get_zero_initialized_cancel_request(void)
{
  static rcl_action_cancel_request_t request = {{{0}, {0, 0}}};
  return request;
}

rcl_action_cancel_response_t
rcl_action_get_zero_initialized_cancel_response(void)
{
  static rcl_action_cancel_response_t response = {{{0, 0, 0}}, {0, 0, 0, 0, 0}};
  return response;
}

rcl_ret_t
rcl_action_goal_status_array_init(
  rcl_action_goal_status_array_t * status_array,
  const size_t num_status,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(status_array, RCL_RET_INVALID_ARGUMENT);
  // Size of array to allocate must be greater than 0
  if (0 == num_status) {
    RCL_SET_ERROR_MSG("num_status must be greater than zero");
    return RCL_RET_INVALID_ARGUMENT;
  }
  // Ensure status array is zero initialized
  if (status_array->msg.status_list.size > 0) {
    RCL_SET_ERROR_MSG("status_array already inititalized");
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for status array
  status_array->msg.status_list.data = (rcl_action_goal_status_t *) allocator.zero_allocate(
    num_status, sizeof(rcl_action_goal_status_t), allocator.state);
  if (!status_array->msg.status_list.data) {
    return RCL_RET_BAD_ALLOC;
  }
  status_array->msg.status_list.size = num_status;
  status_array->allocator = allocator;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_goal_status_array_fini(rcl_action_goal_status_array_t * status_array)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(status_array, RCL_RET_INVALID_ARGUMENT);
  if (!status_array->msg.status_list.data) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  status_array->allocator.deallocate(
    status_array->msg.status_list.data, status_array->allocator.state);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_cancel_response_init(
  rcl_action_cancel_response_t * cancel_response,
  const size_t num_goals_canceling,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(cancel_response, RCL_RET_INVALID_ARGUMENT);
  // Size of array to allocate must be greater than 0
  if (0 == num_goals_canceling) {
    RCL_SET_ERROR_MSG("num_goals_canceling must be greater than zero");
    return RCL_RET_INVALID_ARGUMENT;
  }
  // Ensure cancel response is zero initialized
  if (cancel_response->msg.goals_canceling.size > 0) {
    RCL_SET_ERROR_MSG("cancel_response already inititalized");
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for cancel response
  cancel_response->msg.goals_canceling.data = (rcl_action_goal_info_t *) allocator.zero_allocate(
    num_goals_canceling, sizeof(rcl_action_goal_info_t), allocator.state);
  if (!cancel_response->msg.goals_canceling.data) {
    return RCL_RET_BAD_ALLOC;
  }
  cancel_response->msg.goals_canceling.size = num_goals_canceling;
  cancel_response->allocator = allocator;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_cancel_response_fini(rcl_action_cancel_response_t * cancel_response)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(cancel_response, RCL_RET_INVALID_ARGUMENT);
  if (!cancel_response->msg.goals_canceling.data) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  cancel_response->allocator.deallocate(
    cancel_response->msg.goals_canceling.data, cancel_response->allocator.state);
  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
