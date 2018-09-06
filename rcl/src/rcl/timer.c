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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/timer.h"

#include <inttypes.h>

#include "./stdatomic_helper.h"
#include "rcl/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/time.h"

typedef struct rcl_timer_impl_t
{
  // The clock providing time.
  rcl_clock_t * clock;
  // The user supplied callback.
  atomic_uintptr_t callback;
  // This is a duration in nanoseconds.
  atomic_uint_least64_t period;
  // This is a time in nanoseconds since an unspecified time.
  atomic_uint_least64_t last_call_time;
  // This is a time in nanoseconds since an unspecified time.
  atomic_uint_least64_t next_call_time;
  // A flag which indicates if the timer is canceled.
  atomic_bool canceled;
  // The user supplied allocator.
  rcl_allocator_t allocator;
} rcl_timer_impl_t;

rcl_timer_t
rcl_get_zero_initialized_timer()
{
  static rcl_timer_t null_timer = {0};
  return null_timer;
}

rcl_ret_t
rcl_timer_init(
  rcl_timer_t * timer,
  rcl_clock_t * clock,
  int64_t period,
  const rcl_timer_callback_t callback,
  rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, allocator);
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Initializing timer with period: %" PRIu64 "ns", period)
  if (timer->impl) {
    RCL_SET_ERROR_MSG("timer already initailized, or memory was uninitialized", allocator);
    return RCL_RET_ALREADY_INIT;
  }
  rcl_time_point_value_t now;
  rcl_ret_t now_ret = rcl_clock_get_now(clock, &now);
  if (now_ret != RCL_RET_OK) {
    return now_ret;  // rcl error state should already be set.
  }
  rcl_timer_impl_t impl;
  impl.clock = clock;
  atomic_init(&impl.callback, (uintptr_t)callback);
  atomic_init(&impl.period, period);
  atomic_init(&impl.last_call_time, now);
  atomic_init(&impl.next_call_time, now + period);
  atomic_init(&impl.canceled, false);
  impl.allocator = allocator;
  timer->impl = (rcl_timer_impl_t *)allocator.allocate(sizeof(rcl_timer_impl_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC, allocator);
  *timer->impl = impl;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_fini(rcl_timer_t * timer)
{
  if (!timer || !timer->impl) {
    return RCL_RET_OK;
  }
  // Will return either RCL_RET_OK or RCL_RET_ERROR since the timer is valid.
  rcl_ret_t result = rcl_timer_cancel(timer);
  rcl_allocator_t allocator = timer->impl->allocator;
  allocator.deallocate(timer->impl, allocator.state);
  return result;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_timer_clock(rcl_timer_t * timer, rcl_clock_t ** clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(timer->impl, RCL_RET_TIMER_INVALID, rcl_get_default_allocator());
  *clock = timer->impl->clock;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_call(rcl_timer_t * timer)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Calling timer")
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  if (rcl_atomic_load_bool(&timer->impl->canceled)) {
    RCL_SET_ERROR_MSG("timer is canceled", *allocator);
    return RCL_RET_TIMER_CANCELED;
  }
  rcl_time_point_value_t now;
  rcl_ret_t now_ret = rcl_clock_get_now(timer->impl->clock, &now);
  if (now_ret != RCL_RET_OK) {
    return now_ret;  // rcl error state should already be set.
  }
  if (now < 0) {
    RCL_SET_ERROR_MSG("clock now returned negative time point value", *allocator);
    return RCL_RET_ERROR;
  }
  uint64_t unow = (uint64_t)now;
  rcl_time_point_value_t previous_ns =
    rcl_atomic_exchange_uint64_t(&timer->impl->last_call_time, unow);
  rcl_timer_callback_t typed_callback =
    (rcl_timer_callback_t)rcl_atomic_load_uintptr_t(&timer->impl->callback);

  uint64_t next_call_time = rcl_atomic_load_uint64_t(&timer->impl->next_call_time);
  uint64_t period = rcl_atomic_load_uint64_t(&timer->impl->period);
  // always move the next call time by exactly period forward
  // don't use now as the base to avoid extending each cycle by the time
  // between the timer being ready and the callback being triggered
  next_call_time += period;
  // in case the timer has missed at least once cycle
  if (next_call_time < unow) {
    if (0 == period) {
      // a timer with a period of zero is considered always ready
      next_call_time = unow;
    } else {
      // move the next call time forward by as many periods as necessary
      uint64_t now_ahead = unow - next_call_time;
      // rounding up without overflow
      uint64_t periods_ahead = 1 + (now_ahead - 1) / period;
      next_call_time += periods_ahead * period;
    }
  }
  rcl_atomic_store(&timer->impl->next_call_time, next_call_time);

  if (typed_callback != NULL) {
    int64_t since_last_call = now - previous_ns;
    typed_callback(timer, since_last_call);
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_is_ready(const rcl_timer_t * timer, bool * is_ready)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(is_ready, RCL_RET_INVALID_ARGUMENT, *allocator);
  int64_t time_until_next_call;
  rcl_ret_t ret = rcl_timer_get_time_until_next_call(timer, &time_until_next_call);
  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
  *is_ready = (time_until_next_call <= 0) && !rcl_atomic_load_bool(&timer->impl->canceled);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_get_time_until_next_call(const rcl_timer_t * timer, int64_t * time_until_next_call)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(time_until_next_call, RCL_RET_INVALID_ARGUMENT, *allocator);
  rcl_time_point_value_t now;
  rcl_ret_t ret = rcutils_steady_time_now(&now);
  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
  *time_until_next_call =
    rcl_atomic_load_uint64_t(&timer->impl->next_call_time) - now;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_get_time_since_last_call(
  const rcl_timer_t * timer,
  rcl_time_point_value_t * time_since_last_call)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(time_since_last_call, RCL_RET_INVALID_ARGUMENT, *allocator);
  rcl_time_point_value_t now;
  rcl_ret_t ret = rcutils_steady_time_now(&now);
  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
  *time_since_last_call =
    now - rcl_atomic_load_uint64_t(&timer->impl->last_call_time);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_get_period(const rcl_timer_t * timer, int64_t * period)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(period, RCL_RET_INVALID_ARGUMENT, *allocator);
  *period = rcl_atomic_load_uint64_t(&timer->impl->period);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_exchange_period(const rcl_timer_t * timer, int64_t new_period, int64_t * old_period)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(old_period, RCL_RET_INVALID_ARGUMENT, *allocator);
  *old_period = rcl_atomic_exchange_uint64_t(&timer->impl->period, new_period);
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Updated timer period from '%" PRIu64 "ns' to '%" PRIu64 "ns'",
    *old_period, new_period)
  return RCL_RET_OK;
}

rcl_timer_callback_t
rcl_timer_get_callback(const rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, NULL, rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer->impl, "timer is invalid", return NULL, rcl_get_default_allocator());
  return (rcl_timer_callback_t)rcl_atomic_load_uintptr_t(&timer->impl->callback);
}

rcl_timer_callback_t
rcl_timer_exchange_callback(rcl_timer_t * timer, const rcl_timer_callback_t new_callback)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Updating timer callback")
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, NULL, rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer->impl, "timer is invalid", return NULL, rcl_get_default_allocator());
  return (rcl_timer_callback_t)rcl_atomic_exchange_uintptr_t(
    &timer->impl->callback, (uintptr_t)new_callback);
}

rcl_ret_t
rcl_timer_cancel(rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID, rcl_get_default_allocator());
  rcl_atomic_store(&timer->impl->canceled, true);
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Timer canceled")
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_is_canceled(const rcl_timer_t * timer, bool * is_canceled)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  const rcl_allocator_t * allocator = rcl_timer_get_allocator(timer);
  if (!allocator) {
    return RCL_RET_TIMER_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(is_canceled, RCL_RET_INVALID_ARGUMENT, *allocator);
  *is_canceled = rcl_atomic_load_bool(&timer->impl->canceled);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_reset(rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID, rcl_get_default_allocator());
  rcl_time_point_value_t now;
  rcl_ret_t now_ret = rcutils_steady_time_now(&now);
  if (now_ret != RCL_RET_OK) {
    return now_ret;  // rcl error state should already be set.
  }
  int64_t period = rcl_atomic_load_uint64_t(&timer->impl->period);
  rcl_atomic_store(&timer->impl->next_call_time, now + period);
  rcl_atomic_store(&timer->impl->canceled, false);
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Timer successfully reset")
  return RCL_RET_OK;
}

const rcl_allocator_t *
rcl_timer_get_allocator(const rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, NULL, rcl_get_default_allocator());
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer->impl, "timer is invalid", return NULL, rcl_get_default_allocator());
  return &timer->impl->allocator;
}

#ifdef __cplusplus
}
#endif
