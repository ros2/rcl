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

#include "rcl/wait.h"

#include <stdbool.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rmw/rmw.h"
#include "./common.h"

typedef struct rcl_wait_set_impl_t {
  void * place_holder;  // To prevent size differences between C and C++.
} rcl_wait_set_impl_t;

rcl_wait_set_t
rcl_get_zero_initialized_wait_set()
{
  static rcl_wait_set_t null_wait_set = {0};
  return null_wait_set;
}

static bool
__wait_set_is_valid(rcl_wait_set_t * wait_set)
{
  return wait_set && wait_set->impl;
}

rcl_ret_t
rcl_wait_set_init(
  rcl_wait_set_t * wait_set,
  size_t number_of_subscriptions,
  size_t number_of_guard_conditions,
  rcl_allocator_t allocator)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  if (__wait_set_is_valid(wait_set)) {
    RCL_SET_ERROR_MSG("wait_set already initialized, or memory was uninitialized.");
    return RCL_RET_ALREADY_INIT;
  }
  RCL_CHECK_FOR_NULL_WITH_MSG(
    number_of_subscriptions,
    "number_of_subscriptions cannot be 0", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    number_of_guard_conditions,
    "number_of_guard_conditions cannot be 0", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator.allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator.deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator.reallocate, "reallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Right now, the implementation struct is unused, but checked for NULL, so set it.
  wait_set->impl = (rcl_wait_set_impl_t *)0xDEADBEEF;
  // Initialize subscription space.
  rcl_wait_set_resize_subscriptions(wait_set, number_of_subscriptions);
  rcl_wait_set_clear_subscriptions(wait_set);
  // Initialize guard condition space.
  rcl_wait_set_resize_guard_conditions(wait_set, number_of_guard_conditions);
  rcl_wait_set_clear_guard_conditions(wait_set);
  // Set allocator.
  wait_set->allocator = allocator;
  // Initialize pruned.
  wait_set->pruned = false;
  return RCL_RET_OK;
fail:
  if (wait_set->subscriptions) {
    allocator.deallocate(wait_set->subscriptions, allocator.state);
  }
  if (wait_set->guard_conditions) {
    allocator.deallocate(wait_set->guard_conditions, allocator.state);
  }
  return fail_ret;
}

rcl_ret_t
rcl_wait_set_fini(rcl_wait_set_t * wait_set)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  if (__wait_set_is_valid(wait_set)) {
    if (wait_set->subscriptions) {
      wait_set->allocator.deallocate(wait_set->subscriptions, wait_set->allocator.state);
    }
    if (wait_set->guard_conditions) {
      wait_set->allocator.deallocate(wait_set->guard_conditions, wait_set->allocator.state);
    }
    if (wait_set->impl) {
      wait_set->impl = NULL;
    }
  }
  return result;
}

#define SET_ADD(Type) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_ARGUMENT_FOR_NULL(Type, RCL_RET_INVALID_ARGUMENT); \
  if (!__wait_set_is_valid(wait_set)) { \
    RCL_SET_ERROR_MSG("wait set is invalid"); \
    return RCL_RET_WAIT_SET_INVALID; \
  } \
  if (!(wait_set->__current_##Type##_offset < wait_set->size_of_##Type##s)) { \
    RCL_SET_ERROR_MSG(#Type "s set is full"); \
    return RCL_RET_WAIT_SET_FULL; \
  } \
  wait_set->Type##s[wait_set->__current_##Type##_offset++] = Type; \
  return RCL_RET_OK;

#define SET_CLEAR(Type) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  if (!__wait_set_is_valid(wait_set)) { \
    RCL_SET_ERROR_MSG("wait set is invalid"); \
    return RCL_RET_WAIT_SET_INVALID; \
  } \
  memset(wait_set->Type##s, 0, sizeof(rcl_##Type##_t *) * wait_set->size_of_##Type##s);  \
  wait_set->__current_##Type##_offset = 0; \
  return RCL_RET_OK;

#define SET_RESIZE(Type) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  if (size == wait_set->size_of_##Type##s) { \
    return RCL_RET_OK; \
  } \
  if (size == 0) { \
    if (wait_set->Type##s) { \
      wait_set->allocator.deallocate(wait_set->Type##s, wait_set->allocator.state); \
    } \
    wait_set->Type##s = NULL; \
    return RCL_RET_OK; \
  } \
  wait_set->size_of_##Type##s = 0; \
  wait_set->Type##s = (const rcl_##Type##_t **)wait_set->allocator.reallocate( \
    wait_set->Type##s, sizeof(rcl_##Type##_t *) * size, wait_set->allocator.state); \
  RCL_CHECK_FOR_NULL_WITH_MSG( \
    wait_set->Type##s, "allocating memory failed", return RCL_RET_BAD_ALLOC); \
  wait_set->size_of_##Type##s = size; \
  return RCL_RET_OK;

rcl_ret_t
rcl_wait_set_add_subscription(
  rcl_wait_set_t * wait_set,
  const rcl_subscription_t * subscription)
{
  SET_ADD(subscription)
}

rcl_ret_t
rcl_wait_set_clear_subscriptions(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(subscription)
}

rcl_ret_t
rcl_wait_set_resize_subscriptions(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(subscription)
}

rcl_ret_t
rcl_wait_set_add_guard_condition(
  rcl_wait_set_t * wait_set,
  const rcl_guard_condition_t * guard_condition)
{
  SET_ADD(guard_condition)
}

rcl_ret_t
rcl_wait_set_clear_guard_conditions(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(guard_condition)
}

rcl_ret_t
rcl_wait_set_resize_guard_conditions(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(guard_condition)
}

rcl_ret_t
rcl_wait(rcl_wait_set_t * wait_set, const rcl_time_t * timeout)
{
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
