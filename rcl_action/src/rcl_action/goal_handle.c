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
//
#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/goal_handle.h"

#include "rcl/rcl.h"
#include "rcl/error_handling.h"

typedef struct rcl_action_goal_handle_impl_t
{
  rcl_action_goal_info_t * info;
  rcl_action_goal_state_t state;
  rcl_allocator_t allocator;
} rcl_action_goal_handle_impl_t;

rcl_action_goal_handle_t
rcl_action_get_zero_initialized_goal_handle(void)
{
  static rcl_action_goal_handle_t null_handle = {0};
  return null_handle;
}

rcl_ret_t
rcl_action_goal_handle_init(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(goal_handle, RCL_RET_ACTION_GOAL_HANDLE_INVALID, allocator);

  // Ensure the goal handle is zero initialized
  if (goal_handle->impl) {
    RCL_SET_ERROR_MSG(
      "goal_handle already initialized, or memory was unintialized", allocator);
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for the goal handle impl
  goal_handle->impl = (rcl_action_goal_handle_impl_t *)allocator.allocate(
    sizeof(rcl_action_goal_handle_impl_t), allocator.state);
  if (!goal_handle->impl) {
    RCL_SET_ERROR_MSG("goal_handle memory allocation failed", allocator);
    return RCL_RET_BAD_ALLOC;
  }
  // TODO(jacobperron): Allocate space for the goal info pointer
  goal_handle->impl->allocator = allocator;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_goal_handle_fini(rcl_action_goal_handle_t * goal_handle)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(
    goal_handle,
    RCL_RET_ACTION_GOAL_HANDLE_INVALID,
    rcl_get_default_allocator());

  // TODO(jacobperron): Replace with `rcl_action_goal_handle_is_valid()`
  if (!goal_handle->impl) {
    return RCL_RET_ACTION_GOAL_HANDLE_INVALID;
  }
  // TODO(jacobperron): Deallocate goal info pointer
  goal_handle->impl->allocator.deallocate(goal_handle->impl, goal_handle->impl->allocator.state);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_update_goal_state(
  rcl_action_goal_handle_t * goal_handle,
  const rcl_action_goal_event_t goal_event)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_goal_handle_get_info(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_action_goal_info_t * goal_info)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_goal_handle_get_status(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_action_goal_state_t * status)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

bool
rcl_action_goal_handle_is_active(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_allocator_t * error_msg_allocator)
{
  // TODO(jacobperron): impl
  return true;
}

bool
rcl_action_goal_handle_is_valid(
  const rcl_action_goal_handle_t * goal_handle,
  rcl_allocator_t * error_msg_allocator)
{
  // TODO(jacobperron): impl
  return true;
}

#ifdef __cplusplus
}
#endif
