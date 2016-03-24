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
#include <stdbool.h>
#include <string.h>

#include "./common.h"
#include "./stdatomic_helper.h"
#include "rcl/error_handling.h"
#include "rcl/time.h"
#include "rmw/rmw.h"

typedef struct rcl_wait_set_impl_t
{
  size_t subscription_index;
  rmw_subscriptions_t rmw_subscriptions;
  size_t guard_condition_index;
  rmw_guard_conditions_t rmw_guard_conditions;
  size_t client_index;
  rmw_clients_t rmw_clients;
  size_t service_index;
  rmw_services_t rmw_services;
  rmw_waitset_t * rmw_waitset;
  size_t timer_index;
  rcl_allocator_t allocator;
} rcl_wait_set_impl_t;

rcl_wait_set_t
rcl_get_zero_initialized_wait_set()
{
  static rcl_wait_set_t null_wait_set = {
    .subscriptions = NULL,
    .size_of_subscriptions = 0,
    .guard_conditions = NULL,
    .size_of_guard_conditions = 0,
    .clients = NULL,
    .size_of_clients = 0,
    .services = NULL,
    .size_of_services = 0,
    .timers = NULL,
    .size_of_timers = 0,
    .impl = NULL,
  };
  return null_wait_set;
}

static bool
__wait_set_is_valid(const rcl_wait_set_t * wait_set)
{
  return wait_set && wait_set->impl;
}

static void
__wait_set_clean_up(rcl_wait_set_t * wait_set, rcl_allocator_t allocator)
{
  if (wait_set->subscriptions) {
    rcl_ret_t ret = rcl_wait_set_resize_subscriptions(wait_set, 0);
    (void)ret;  // NO LINT
    assert(ret == RCL_RET_OK);  // Defensive, shouldn't fail with size 0.
  }
  if (wait_set->guard_conditions) {
    rcl_ret_t ret = rcl_wait_set_resize_guard_conditions(wait_set, 0);
    (void)ret;  // NO LINT
    assert(ret == RCL_RET_OK);  // Defensive, shouldn't fail with size 0.
  }
  if (wait_set->timers) {
    rcl_ret_t ret = rcl_wait_set_resize_timers(wait_set, 0);
    (void)ret;  // NO LINT
    assert(ret == RCL_RET_OK);  // Defensive, shouldn't fail with size 0.
  }
  if (wait_set->clients) {
    rcl_ret_t ret = rcl_wait_set_resize_clients(wait_set, 0);
    (void)ret;  // NO LINT
    assert(ret == RCL_RET_OK);  // Defensive, shouldn't fail with size 0.
  }
  if (wait_set->services) {
    rcl_ret_t ret = rcl_wait_set_resize_services(wait_set, 0);
    (void)ret;  // NO LINT
    assert(ret == RCL_RET_OK);  // Defensive, shouldn't fail with size 0.
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
  size_t number_of_timers,
  size_t number_of_clients,
  size_t number_of_services,
  rcl_allocator_t allocator)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  if (__wait_set_is_valid(wait_set)) {
    RCL_SET_ERROR_MSG("wait_set already initialized, or memory was uninitialized.");
    return RCL_RET_ALREADY_INIT;
  }
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
  memset(wait_set->impl, 0, sizeof(rcl_wait_set_impl_t));
  wait_set->impl->rmw_subscriptions.subscribers = NULL;
  wait_set->impl->rmw_subscriptions.subscriber_count = 0;
  wait_set->impl->rmw_guard_conditions.guard_conditions = NULL;
  wait_set->impl->rmw_guard_conditions.guard_condition_count = 0;
  wait_set->impl->rmw_clients.clients = NULL;
  wait_set->impl->rmw_clients.client_count = 0;
  wait_set->impl->rmw_services.services = NULL;
  wait_set->impl->rmw_services.service_count = 0;

  static rmw_guard_conditions_t fixed_guard_conditions;
  fixed_guard_conditions.guard_conditions = NULL;
  fixed_guard_conditions.guard_condition_count = 0;
  wait_set->impl->rmw_waitset = rmw_create_waitset(
    &fixed_guard_conditions,
    2 * number_of_subscriptions + number_of_guard_conditions + number_of_clients +
    number_of_services);
  if (!wait_set->impl->rmw_waitset) {
    goto fail;
  }

  // Set allocator.
  wait_set->impl->allocator = allocator;
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
  // Initialize timer space.
  ret = rcl_wait_set_resize_timers(wait_set, number_of_timers);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  if ((ret = rcl_wait_set_clear_timers(wait_set)) != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  // Initialize client space.
  ret = rcl_wait_set_resize_clients(wait_set, number_of_clients);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  if ((ret = rcl_wait_set_clear_clients(wait_set)) != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  // Initialize service space.
  ret = rcl_wait_set_resize_services(wait_set, number_of_services);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  if ((ret = rcl_wait_set_clear_services(wait_set)) != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }
  return RCL_RET_OK;
fail:
  if (__wait_set_is_valid(wait_set)) {
    rmw_ret_t ret = rmw_destroy_waitset(wait_set->impl->rmw_waitset);
    if (ret != RMW_RET_OK) {
      fail_ret = RCL_RET_WAIT_SET_INVALID;
    }
  }
  __wait_set_clean_up(wait_set, allocator);
  return fail_ret;
}

rcl_ret_t
rcl_wait_set_fini(rcl_wait_set_t * wait_set)
{
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);

  if (__wait_set_is_valid(wait_set)) {
    rmw_ret_t ret = rmw_destroy_waitset(wait_set->impl->rmw_waitset);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
      result = RCL_RET_WAIT_SET_INVALID;
    }
    __wait_set_clean_up(wait_set, wait_set->impl->allocator);
  }
  return result;
}

rcl_ret_t
rcl_wait_set_get_allocator(const rcl_wait_set_t * wait_set, rcl_allocator_t * allocator)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);
  if (!__wait_set_is_valid(wait_set)) {
    RCL_SET_ERROR_MSG("wait set is invalid");
    return RCL_RET_WAIT_SET_INVALID;
  }
  *allocator = wait_set->impl->allocator;
  return RCL_RET_OK;
}

#define SET_ADD(Type) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_ARGUMENT_FOR_NULL(Type, RCL_RET_INVALID_ARGUMENT); \
  if (!__wait_set_is_valid(wait_set)) { \
    RCL_SET_ERROR_MSG("wait set is invalid"); \
    return RCL_RET_WAIT_SET_INVALID; \
  } \
  if (!(wait_set->impl->Type ## _index < wait_set->size_of_ ## Type ## s)) { \
    RCL_SET_ERROR_MSG(#Type "s set is full"); \
    return RCL_RET_WAIT_SET_FULL; \
  } \
  size_t current_index = wait_set->impl->Type ## _index++; \
  wait_set->Type ## s[current_index] = Type;

#define SET_ADD_RMW(Type, RMWStorage) \
  /* Also place into rmw storage. */ \
  rmw_ ## Type ## _t * rmw_handle = rcl_ ## Type ## _get_rmw_handle(Type); \
  RCL_CHECK_FOR_NULL_WITH_MSG( \
    rmw_handle, rcl_get_error_string_safe(), return RCL_RET_ERROR); \
  wait_set->impl->RMWStorage[current_index] = rmw_handle->data;

#define SET_CLEAR(Type) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  if (!__wait_set_is_valid(wait_set)) { \
    RCL_SET_ERROR_MSG("wait set is invalid"); \
    return RCL_RET_WAIT_SET_INVALID; \
  } \
  memset( \
    (void *)wait_set->Type ## s, \
    0, \
    sizeof(rcl_ ## Type ## _t *) * wait_set->size_of_ ## Type ## s); \
  wait_set->impl->Type ## _index = 0; \

#define SET_CLEAR_RMW(Type, RMWStorage, RMWCount) \
  /* Also clear the rmw storage. */ \
  memset( \
    wait_set->impl->RMWStorage, \
    0, \
    sizeof(rmw_ ## Type ## _t *) * wait_set->impl->RMWCount);

#define SET_RESIZE(Type, ExtraDealloc, ExtraRealloc) \
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_FOR_NULL_WITH_MSG( \
    wait_set->impl, "wait set is invalid", return RCL_RET_WAIT_SET_INVALID); \
  if (size == wait_set->size_of_ ## Type ## s) { \
    return RCL_RET_OK; \
  } \
  rcl_allocator_t allocator = wait_set->impl->allocator; \
  if (size == 0) { \
    if (wait_set->Type ## s) { \
      allocator.deallocate((void *)wait_set->Type ## s, allocator.state); \
      wait_set->Type ## s = NULL; \
    } \
    ExtraDealloc \
  } else { \
    wait_set->size_of_ ## Type ## s = 0; \
    wait_set->Type ## s = (const rcl_ ## Type ## _t * *)allocator.reallocate( \
      (void *)wait_set->Type ## s, sizeof(rcl_ ## Type ## _t *) * size, allocator.state); \
    RCL_CHECK_FOR_NULL_WITH_MSG( \
      wait_set->Type ## s, "allocating memory failed", return RCL_RET_BAD_ALLOC); \
    wait_set->size_of_ ## Type ## s = size; \
    ExtraRealloc \
  } \
  return RCL_RET_OK;

#define SET_RESIZE_RMW_DEALLOC(RMWStorage, RMWCount) \
  /* Also deallocate the rmw storage. */ \
  if (wait_set->impl->RMWStorage) { \
    allocator.deallocate((void *)wait_set->impl->RMWStorage, allocator.state); \
    wait_set->impl->RMWStorage = NULL; \
  }

#define SET_RESIZE_RMW_REALLOC(Type, RMWStorage, RMWCount) \
  /* Also resize the rmw storage. */ \
  wait_set->impl->RMWCount = 0; \
  wait_set->impl->RMWStorage = (void **)allocator.reallocate( \
    wait_set->impl->RMWStorage, sizeof(rcl_ ## Type ## _t *) * size, allocator.state); \
  if (!wait_set->impl->RMWStorage) { \
    allocator.deallocate((void *)wait_set->Type ## s, allocator.state); \
    wait_set->size_of_ ## Type ## s = 0; \
    RCL_SET_ERROR_MSG("allocating memory failed"); \
    return RCL_RET_BAD_ALLOC; \
  } \
  wait_set->impl->RMWCount = size;

rcl_ret_t
rcl_wait_set_add_subscription(
  rcl_wait_set_t * wait_set,
  const rcl_subscription_t * subscription)
{
  SET_ADD(subscription)
  SET_ADD_RMW(subscription, rmw_subscriptions.subscribers)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_clear_subscriptions(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(subscription)
  SET_CLEAR_RMW(
    subscription,
    rmw_subscriptions.subscribers,
    rmw_subscriptions.subscriber_count)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_resize_subscriptions(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(
    subscription,
    SET_RESIZE_RMW_DEALLOC(
      rmw_subscriptions.subscribers, rmw_subscriptions.subscriber_count),
    SET_RESIZE_RMW_REALLOC(
      subscription, rmw_subscriptions.subscribers, rmw_subscriptions.subscriber_count)
  )
}

rcl_ret_t
rcl_wait_set_add_guard_condition(
  rcl_wait_set_t * wait_set,
  const rcl_guard_condition_t * guard_condition)
{
  SET_ADD(guard_condition)
  SET_ADD_RMW(guard_condition, rmw_guard_conditions.guard_conditions)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_clear_guard_conditions(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(guard_condition)
  SET_CLEAR_RMW(
    guard_condition,
    rmw_guard_conditions.guard_conditions,
    rmw_guard_conditions.guard_condition_count)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_resize_guard_conditions(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(
    guard_condition,
    SET_RESIZE_RMW_DEALLOC(
      rmw_guard_conditions.guard_conditions, rmw_guard_conditions.guard_condition_count),
    SET_RESIZE_RMW_REALLOC(
      guard_condition,
      rmw_guard_conditions.guard_conditions,
      rmw_guard_conditions.guard_condition_count)
  )
}

rcl_ret_t
rcl_wait_set_add_timer(
  rcl_wait_set_t * wait_set,
  const rcl_timer_t * timer)
{
  SET_ADD(timer)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_clear_timers(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(timer)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_resize_timers(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(timer,;,;)  // NOLINT
}

rcl_ret_t
rcl_wait_set_add_client(
  rcl_wait_set_t * wait_set,
  const rcl_client_t * client)
{
  SET_ADD(client)
  SET_ADD_RMW(client, rmw_clients.clients)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_clear_clients(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(client)
  SET_CLEAR_RMW(
    clients,
    rmw_clients.clients,
    rmw_clients.client_count)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_resize_clients(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(client,
    SET_RESIZE_RMW_DEALLOC(
      rmw_clients.clients, rmw_clients.client_count),
    SET_RESIZE_RMW_REALLOC(
      client, rmw_clients.clients, rmw_clients.client_count)
  )
}

rcl_ret_t
rcl_wait_set_add_service(
  rcl_wait_set_t * wait_set,
  const rcl_service_t * service)
{
  SET_ADD(service)
  SET_ADD_RMW(service, rmw_services.services)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_clear_services(rcl_wait_set_t * wait_set)
{
  SET_CLEAR(service)
  SET_CLEAR_RMW(
    services,
    rmw_services.services,
    rmw_services.service_count)
  return RCL_RET_OK;
}

rcl_ret_t
rcl_wait_set_resize_services(rcl_wait_set_t * wait_set, size_t size)
{
  SET_RESIZE(service,
    SET_RESIZE_RMW_DEALLOC(
      rmw_services.services, rmw_services.service_count),
    SET_RESIZE_RMW_REALLOC(
      service, rmw_services.services, rmw_services.service_count)
  )
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
  if (wait_set->size_of_subscriptions == 0 && wait_set->size_of_guard_conditions == 0 &&
    wait_set->size_of_clients == 0 && wait_set->size_of_services == 0)
  {
    RCL_SET_ERROR_MSG("wait set is empty");
    return RCL_RET_WAIT_SET_EMPTY;
  }
  // Calculate the timeout argument.
  rmw_time_t * timeout_argument;
  rmw_time_t temporary_timeout_storage;
  if (timeout < 0) {
    // Pass NULL to wait to indicate block indefinitely.
    timeout_argument = NULL;
  }
  if (timeout > 0) {
    // Determine the nearest timeout (given or a timer).
    uint64_t min_timeout = timeout;
    // If min_timeout is > 0, then compare it to the time until each timer.
    // Take the lowest and use that for the wait timeout.
    if (min_timeout > 0) {
      size_t i;
      for (i = 0; i < wait_set->size_of_timers; ++i) {
        if (!wait_set->timers[i]) {
          continue;  // Skip NULL timers.
        }
        int64_t timer_timeout;
        rcl_ret_t ret = rcl_timer_get_time_until_next_call(wait_set->timers[i], &timer_timeout);
        if (ret != RCL_RET_OK) {
          return ret;  // The rcl error state should already be set.
        }
        if ((uint64_t)timer_timeout < min_timeout) {
          min_timeout = timer_timeout;
        }
      }
    }
    // Set that in the temporary storage and point to that for the argument.
    temporary_timeout_storage.sec = RCL_NS_TO_S(min_timeout);
    temporary_timeout_storage.nsec = min_timeout % 1000000000;
    timeout_argument = &temporary_timeout_storage;
  }
  if (timeout == 0) {
    // Then it is non-blocking, so set the temporary storage to 0, 0 and pass it.
    temporary_timeout_storage.sec = 0;
    temporary_timeout_storage.nsec = 0;
    timeout_argument = &temporary_timeout_storage;
  }
  // Wait.
  rmw_ret_t ret = rmw_wait(
    &wait_set->impl->rmw_subscriptions,
    &wait_set->impl->rmw_guard_conditions,
    &wait_set->impl->rmw_services,
    &wait_set->impl->rmw_clients,
    wait_set->impl->rmw_waitset,
    timeout_argument);
  // Check for timeout.
  if (ret == RMW_RET_TIMEOUT) {
    // Assume none were set (because timeout was reached first), and clear all.
    rcl_ret_t rcl_ret;
    // This next line prevents "assigned but never used" warnings in Release mode.
    (void)rcl_ret;  // NO LINT
    rcl_ret = rcl_wait_set_clear_subscriptions(wait_set);
    assert(rcl_ret == RCL_RET_OK);  // Defensive, shouldn't fail with valid wait_set.
    rcl_ret = rcl_wait_set_clear_guard_conditions(wait_set);
    assert(rcl_ret == RCL_RET_OK);  // Defensive, shouldn't fail with valid wait_set.
    rcl_ret = rcl_wait_set_clear_services(wait_set);
    assert(rcl_ret == RCL_RET_OK);  // Defensive, shouldn't fail with valid wait_set.
    rcl_ret = rcl_wait_set_clear_clients(wait_set);
    assert(rcl_ret == RCL_RET_OK);  // Defensive, shouldn't fail with valid wait_set.
    return RCL_RET_TIMEOUT;
  }
  // Check for error.
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  // Check for ready timers next, and set not ready timers to NULL.
  size_t i;
  for (i = 0; i < wait_set->size_of_timers; ++i) {
    bool is_ready = false;
    rcl_ret_t ret = rcl_timer_is_ready(wait_set->timers[i], &is_ready);
    if (ret != RCL_RET_OK) {
      return ret;  // The rcl error state should already be set.
    }
    if (!is_ready) {
      wait_set->timers[i] = NULL;
    }
  }
  // Set corresponding rcl subscription handles NULL.
  for (i = 0; i < wait_set->size_of_subscriptions; ++i) {
    assert(i < wait_set->impl->rmw_subscriptions.subscriber_count);  // Defensive.
    if (!wait_set->impl->rmw_subscriptions.subscribers[i]) {
      wait_set->subscriptions[i] = NULL;
    }
  }
  // Set corresponding rcl guard_condition handles NULL.
  for (i = 0; i < wait_set->size_of_guard_conditions; ++i) {
    assert(i < wait_set->impl->rmw_guard_conditions.guard_condition_count);  // Defensive.
    if (!wait_set->impl->rmw_guard_conditions.guard_conditions[i]) {
      wait_set->guard_conditions[i] = NULL;
    }
  }
  // Set corresponding rcl client handles NULL.
  for (i = 0; i < wait_set->size_of_clients; ++i) {
    assert(i < wait_set->impl->rmw_clients.client_count);  // Defensive.
    if (!wait_set->impl->rmw_clients.clients[i]) {
      wait_set->clients[i] = NULL;
    }
  }
  // Set corresponding rcl service handles NULL.
  for (i = 0; i < wait_set->size_of_services; ++i) {
    assert(i < wait_set->impl->rmw_services.service_count);  // Defensive.
    if (!wait_set->impl->rmw_services.services[i]) {
      wait_set->services[i] = NULL;
    }
  }
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
