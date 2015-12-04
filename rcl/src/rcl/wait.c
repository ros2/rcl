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

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/time.h"
#include "rmw/rmw.h"
#include "./common.h"

typedef struct rcl_wait_set_impl_t {
  size_t subscription_index;
  rmw_subscriptions_t rmw_subscriptions;
  size_t guard_condition_index;
  rmw_guard_conditions_t rmw_guard_conditions;
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

static void
__wait_set_clean_up(rcl_wait_set_t * wait_set, rcl_allocator_t allocator)
{
  if (wait_set->subscriptions) {
    rcl_wait_set_resize_subscriptions(wait_set, 0);
  }
  if (wait_set->guard_conditions) {
    rcl_wait_set_resize_guard_conditions(wait_set, 0);
  }
  if (wait_set->impl) {
    allocator.deallocate(wait_set->impl, allocator.state);
    wait_set->impl = NULL;
  }
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
  // Allocate space for the implementation struct.
  wait_set->impl = (rcl_wait_set_impl_t *)allocator.allocate(
    sizeof(rcl_wait_set_impl_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    wait_set->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  wait_set->impl->rmw_subscriptions.subscribers = NULL;
  wait_set->impl->rmw_subscriptions.subscriber_count = 0;
  wait_set->impl->rmw_guard_conditions.guard_conditions = NULL;
  wait_set->impl->rmw_guard_conditions.guard_condition_count = 0;
  // Initialize subscription space.
  rcl_ret_t ret;
  if ((ret = rcl_wait_set_resize_subscriptions(wait_set, number_of_subscriptions)) != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  if ((ret = rcl_wait_set_clear_subscriptions(wait_set)) != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  // Initialize guard condition space.
  ret = rcl_wait_set_resize_guard_conditions(wait_set, number_of_guard_conditions);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  if ((ret = rcl_wait_set_clear_guard_conditions(wait_set)) != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  // Set allocator.
  wait_set->allocator = allocator;
  // Initialize pruned.
  wait_set->pruned = false;
  return RCL_RET_OK;
fail:
  __wait_set_clean_up(wait_set, allocator);
  return fail_ret;
}

rcl_ret_t
rcl_wait_set_fini(rcl_wait_set_t * wait_set)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  if (__wait_set_is_valid(wait_set)) {
    __wait_set_clean_up(wait_set, wait_set->allocator);
  }
  return result;
}

#define SET_ADD(Type, RMWStorage) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_ARGUMENT_FOR_NULL(Type, RCL_RET_INVALID_ARGUMENT); \
  if (!__wait_set_is_valid(wait_set)) { \
    RCL_SET_ERROR_MSG("wait set is invalid"); \
    return RCL_RET_WAIT_SET_INVALID; \
  } \
  if (!(wait_set->impl->Type##_index < wait_set->size_of_##Type##s)) { \
    RCL_SET_ERROR_MSG(#Type "s set is full"); \
    return RCL_RET_WAIT_SET_FULL; \
  } \
  size_t current_index = wait_set->impl->Type##_index++; \
  wait_set->Type##s[current_index] = Type; \
  /* Also place into rmw storage. */ \
  rmw_##Type##_t * rmw_handle = rcl_##Type##_get_rmw_##Type##_handle(Type); \
  RCL_CHECK_FOR_NULL_WITH_MSG( \
    rmw_handle, rcl_get_error_string_safe(), return RCL_RET_ERROR); \
  wait_set->impl->RMWStorage[current_index] = rmw_handle->data; \
  return RCL_RET_OK;

#define SET_CLEAR(Type, RMWStorage, RMWCount) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  if (!__wait_set_is_valid(wait_set)) { \
    RCL_SET_ERROR_MSG("wait set is invalid"); \
    return RCL_RET_WAIT_SET_INVALID; \
  } \
  memset(wait_set->Type##s, 0, sizeof(rcl_##Type##_t *) * wait_set->size_of_##Type##s);  \
  wait_set->impl->Type##_index = 0; \
  /* Also clear the rmw storage. */ \
  memset( \
    wait_set->impl->RMWStorage, \
    0, \
    sizeof(rmw_subscription_t *) * wait_set->impl->RMWCount); \
  return RCL_RET_OK;

#define SET_RESIZE(Type, RMWStorage, RMWCount) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_FOR_NULL_WITH_MSG( \
    wait_set->impl, "wait set is invalid", return RCL_RET_WAIT_SET_INVALID); \
  if (size == wait_set->size_of_##Type##s) { \
    return RCL_RET_OK; \
  } \
  if (size == 0) { \
    if (wait_set->Type##s) { \
      wait_set->allocator.deallocate(wait_set->Type##s, wait_set->allocator.state); \
      wait_set->Type##s = NULL; \
    } \
    /* Also deallocate the rmw storage. */ \
    if (wait_set->impl->RMWStorage) { \
      wait_set->allocator.deallocate(wait_set->impl->RMWStorage, wait_set->allocator.state); \
      wait_set->impl->RMWStorage = NULL; \
    } \
  } \
  else { \
    wait_set->size_of_##Type##s = 0; \
    wait_set->Type##s = (const rcl_##Type##_t **)wait_set->allocator.reallocate( \
      wait_set->Type##s, sizeof(rcl_##Type##_t *) * size, wait_set->allocator.state); \
    RCL_CHECK_FOR_NULL_WITH_MSG( \
      wait_set->Type##s, "allocating memory failed", return RCL_RET_BAD_ALLOC); \
    wait_set->size_of_##Type##s = size; \
    /* Also resize the rmw storage. */ \
    wait_set->impl->RMWCount = 0; \
    wait_set->impl->RMWStorage = (void **)wait_set->allocator.reallocate( \
      wait_set->impl->RMWStorage, sizeof(rcl_##Type##_t *) * size, wait_set->allocator.state); \
    if (!wait_set->impl->RMWStorage) { \
      wait_set->allocator.deallocate(wait_set->Type##s, wait_set->allocator.state); \
      wait_set->size_of_##Type##s = 0; \
      RCL_SET_ERROR_MSG("allocating memory failed"); \
      return RCL_RET_BAD_ALLOC; \
    } \
    wait_set->impl->RMWCount = size; \
  } \
  return RCL_RET_OK;

rcl_ret_t
rcl_wait_set_add_subscription(
  rcl_wait_set_t * wait_set,
  const rcl_subscription_t * subscription)
{
  SET_ADD(subscription, rmw_subscriptions.subscribers)
}

rcl_ret_t
rcl_wait_set_clear_subscriptions(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(
    subscription,
    rmw_subscriptions.subscribers,
    rmw_subscriptions.subscriber_count)
}

rcl_ret_t
rcl_wait_set_resize_subscriptions(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(
    subscription,
    rmw_subscriptions.subscribers,
    rmw_subscriptions.subscriber_count)
}

rcl_ret_t
rcl_wait_set_add_guard_condition(
  rcl_wait_set_t * wait_set,
  const rcl_guard_condition_t * guard_condition)
{
  SET_ADD(guard_condition, rmw_guard_conditions.guard_conditions)
}

rcl_ret_t
rcl_wait_set_clear_guard_conditions(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(
    guard_condition,
    rmw_guard_conditions.guard_conditions,
    rmw_guard_conditions.guard_condition_count)
}

rcl_ret_t
rcl_wait_set_resize_guard_conditions(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(
    guard_condition,
    rmw_guard_conditions.guard_conditions,
    rmw_guard_conditions.guard_condition_count)
}

rcl_ret_t
rcl_wait(rcl_wait_set_t * wait_set, int64_t timeout)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(timeout, RCL_RET_INVALID_ARGUMENT);
  if (!__wait_set_is_valid(wait_set)) {
    RCL_SET_ERROR_MSG("wait set is invalid");
    return RCL_RET_WAIT_SET_INVALID;
  }
  if (wait_set->size_of_subscriptions == 0 && wait_set->size_of_guard_conditions == 0) {
    RCL_SET_ERROR_MSG("wait set is empty");
    return RCL_RET_WAIT_SET_EMPTY;
  }
  // Create dummy sets for currently unsupported wait-ables.
  static rmw_services_t dummy_services = {0, NULL};
  static rmw_clients_t dummy_clients = {0, NULL};
  rmw_time_t rmw_timeout = rcl_time_from_int64_t_nanoseconds(timeout);
  // Wait.
  rmw_ret_t ret = rmw_wait(
    &wait_set->impl->rmw_subscriptions,
    &wait_set->impl->rmw_guard_conditions,
    &dummy_services,
    &dummy_clients,
    &rmw_timeout
  );
  // Check for timeout.
  if (ret == RMW_RET_TIMEOUT) {
    // Assume none were set (because timeout was reached first), and clear all.
    rcl_wait_set_clear_subscriptions(wait_set);
    rcl_wait_set_clear_guard_conditions(wait_set);
    return RCL_RET_TIMEOUT;
  }
  // Check for error.
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  // Set corresponding rcl subscription handles NULL.
  for (size_t i = 0; i < wait_set->size_of_subscriptions; ++i) {
    assert(i < wait_set->impl->rmw_subscriptions.subscriber_count);  // Defensive.
    if (!wait_set->impl->rmw_subscriptions.subscribers[i]) {
      wait_set->subscriptions[i] = NULL;
    }
  }
  // Set corresponding rcl guard_condition handles NULL.
  for (size_t i = 0; i < wait_set->size_of_guard_conditions; ++i) {
    assert(i < wait_set->impl->rmw_guard_conditions.guard_condition_count);  // Defensive.
    if (!wait_set->impl->rmw_guard_conditions.guard_conditions[i]) {
      wait_set->guard_conditions[i] = NULL;
    }
  }
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
