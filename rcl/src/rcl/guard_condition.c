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

#include "rcl/guard_condition.h"

#include "./common.h"
#include "rcl/rcl.h"
#include "rmw/rmw.h"

typedef struct rcl_guard_condition_impl_t
{
  rmw_guard_condition_t * rmw_handle;
  rcl_guard_condition_options_t options;
} rcl_guard_condition_impl_t;

rcl_guard_condition_t
rcl_get_zero_initialized_guard_condition()
{
  static rcl_guard_condition_t null_guard_condition = {0};
  return null_guard_condition;
}

rcl_ret_t
__rcl_guard_condition_init_from_rmw_impl(
  rcl_guard_condition_t * guard_condition,
  const rmw_guard_condition_t * rmw_guard_condition,
  const rcl_guard_condition_options_t options)
{
  // This function will create an rmw_guard_condition if the parameter is null.

  // Perform argument validation.
  RCL_CHECK_ARGUMENT_FOR_NULL(guard_condition, RCL_RET_INVALID_ARGUMENT);
  const rcl_allocator_t * allocator = &options.allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Ensure the guard_condition handle is zero initialized.
  if (guard_condition->impl) {
    RCL_SET_ERROR_MSG("guard_condition already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for the guard condition impl.
  guard_condition->impl = (rcl_guard_condition_impl_t *)allocator->allocate(
    sizeof(rcl_guard_condition_impl_t), allocator->state);
  if (!guard_condition->impl) {
    RCL_SET_ERROR_MSG("allocating memory failed");
    return RCL_RET_BAD_ALLOC;
  }
  // Create the rmw guard condition.
  if (rmw_guard_condition) {
    // If given, just assign (cast away const).
    guard_condition->impl->rmw_handle = (rmw_guard_condition_t *)rmw_guard_condition;
  } else {
    // Otherwise create one.
    guard_condition->impl->rmw_handle = rmw_create_guard_condition();
    if (!guard_condition->impl->rmw_handle) {
      // Deallocate impl and exit.
      allocator->deallocate(guard_condition->impl, allocator->state);
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
      return RCL_RET_ERROR;
    }
  }
  // Copy options into impl.
  guard_condition->impl->options = options;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_guard_condition_init(
  rcl_guard_condition_t * guard_condition,
  const rcl_guard_condition_options_t options)
{
  // NULL indicates "create a new rmw guard condition".
  return __rcl_guard_condition_init_from_rmw_impl(guard_condition, NULL, options);
}

rcl_ret_t
rcl_guard_condition_init_from_rmw(
  rcl_guard_condition_t * guard_condition,
  const rmw_guard_condition_t * rmw_guard_condition,
  const rcl_guard_condition_options_t options)
{
  return __rcl_guard_condition_init_from_rmw_impl(guard_condition, rmw_guard_condition, options);
}

rcl_ret_t
rcl_guard_condition_fini(rcl_guard_condition_t * guard_condition)
{
  // Perform argument validation.
  RCL_CHECK_ARGUMENT_FOR_NULL(guard_condition, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t result = RCL_RET_OK;
  if (guard_condition->impl) {
    if (guard_condition->impl->rmw_handle) {
      if (rmw_destroy_guard_condition(guard_condition->impl->rmw_handle) != RMW_RET_OK) {
        RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
        result = RCL_RET_ERROR;
      }
    }
    rcl_allocator_t allocator = guard_condition->impl->options.allocator;
    if (allocator.deallocate) {
      allocator.deallocate(guard_condition->impl, allocator.state);
    } else {
      RCL_SET_ERROR_MSG("deallocate not set");
      result = RCL_RET_ERROR;
    }
  }
  return result;
}

rcl_guard_condition_options_t
rcl_guard_condition_get_default_options()
{
  static rcl_guard_condition_options_t default_options;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

rcl_ret_t
rcl_trigger_guard_condition(rcl_guard_condition_t * guard_condition)
{
  // Perform argument validation.
  RCL_CHECK_ARGUMENT_FOR_NULL(guard_condition, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    guard_condition->impl,
    "guard condition implementation is invalid",
    return RCL_RET_INVALID_ARGUMENT);
  // Trigger the guard condition.
  if (rmw_trigger_guard_condition(guard_condition->impl->rmw_handle) != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rmw_guard_condition_t *
rcl_guard_condition_get_rmw_handle(const rcl_guard_condition_t * guard_condition)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(guard_condition, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    guard_condition->impl, "guard condition implementation is invalid", return NULL);
  return guard_condition->impl->rmw_handle;
}

#if __cplusplus
}
#endif
