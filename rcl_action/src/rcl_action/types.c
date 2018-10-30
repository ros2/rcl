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
  static rcl_action_goal_status_array_t status_array = {{0, 0, 0}};
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
  static rcl_action_cancel_response_t response = {{0, 0, 0}};
  return response;
}

rcl_ret_t
rcl_action_goal_status_array_init(
  rcl_action_goal_status_array_t * status_array,
  const size_t num_status,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(status_array, RCL_RET_INVALID_ARGUMENT, allocator);
  // Ensure status array is zero initialized
  if (status_array->status_list.size > 0) {
    RCL_SET_ERROR_MSG("status_array already inititalized", allocator);
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for status array
  status_array->status_list.data = (rcl_action_goal_status_t *) allocator.zero_allocate(
    num_status, sizeof(rcl_action_goal_status_t), allocator.state);
  if (!status_array->status_list.data) {
    return RCL_RET_BAD_ALLOC;
  }
  status_array->status_list.size = num_status;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_goal_status_array_fini(
  rcl_action_goal_status_array_t * status_array,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(status_array, RCL_RET_INVALID_ARGUMENT, allocator);
  if (!status_array->status_list.data) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  allocator.deallocate(status_array->status_list.data, allocator.state);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_cancel_response_init(
  rcl_action_cancel_response_t * cancel_response,
  const size_t num_goals_canceling,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(cancel_response, RCL_RET_INVALID_ARGUMENT, allocator);
  // Ensure cancel response is zero initialized
  if (cancel_response->goals_canceling.size > 0) {
    RCL_SET_ERROR_MSG("cancel_response already inititalized", allocator);
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for cancel response
  cancel_response->goals_canceling.data = (rcl_action_goal_info_t *) allocator.zero_allocate(
    num_goals_canceling, sizeof(rcl_action_goal_info_t), allocator.state);
  if (!cancel_response->goals_canceling.data) {
    return RCL_RET_BAD_ALLOC;
  }
  cancel_response->goals_canceling.size = num_goals_canceling;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_cancel_response_fini(
  rcl_action_cancel_response_t * cancel_response,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(cancel_response, RCL_RET_INVALID_ARGUMENT, allocator);
  if (!cancel_response->goals_canceling.data) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  allocator.deallocate(cancel_response->goals_canceling.data, allocator.state);
  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
