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

#include "rcl/timer.h"

#include "./common.h"
#include "./stdatomic_helper.h"

typedef struct rcl_timer_impl_t
{
  // The user supplied callback.
  atomic_uintptr_t callback;
  // This is a duration in nanoseconds.
  atomic_uint_least64_t period;
  // This is a time in nanoseconds since an unspecified time.
  atomic_uint_least64_t last_call_time;
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
  uint64_t period,
  const rcl_timer_callback_t callback,
  rcl_allocator_t allocator)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(callback, RCL_RET_INVALID_ARGUMENT);
  if (timer->impl) {
    RCL_SET_ERROR_MSG("timer already initailized, or memory was uninitialized");
    return RCL_RET_ALREADY_INIT;
  }
  rcl_time_point_value_t now_steady;
  rcl_ret_t now_ret = rcl_steady_time_now(&now_steady);
  if (now_ret != RCL_RET_OK) {
    return now_ret;  // rcl error state should already be set.
  }
  rcl_timer_impl_t impl;
  impl.callback = ATOMIC_VAR_INIT((uintptr_t)callback);
  impl.period = ATOMIC_VAR_INIT(period);
  impl.last_call_time = ATOMIC_VAR_INIT(now_steady);
  impl.canceled = ATOMIC_VAR_INIT(false);
  impl.allocator = allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator.allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator.deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  timer->impl = (rcl_timer_impl_t *)allocator.allocate(sizeof(rcl_timer_impl_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
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

rcl_ret_t
rcl_timer_call(rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  if (rcl_atomic_load_bool(&timer->impl->canceled)) {
    RCL_SET_ERROR_MSG("timer is canceled");
    return RCL_RET_TIMER_CANCELED;
  }
  rcl_time_point_value_t now_steady;
  rcl_ret_t now_ret = rcl_steady_time_now(&now_steady);
  if (now_ret != RCL_RET_OK) {
    return now_ret;  // rcl error state should already be set.
  }
  rcl_time_point_value_t previous_ns =
    rcl_atomic_exchange_uint64_t(&timer->impl->last_call_time, now_steady);
  uint64_t since_last_call = now_steady - previous_ns;
  rcl_timer_callback_t typed_callback =
    (rcl_timer_callback_t)rcl_atomic_load_uintptr_t(&timer->impl->callback);
  typed_callback(timer, since_last_call);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_is_ready(const rcl_timer_t * timer, bool * is_ready)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
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
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(time_until_next_call, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  rcl_time_point_value_t now;
  rcl_ret_t ret = rcl_steady_time_now(&now);
  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
  uint64_t period = rcl_atomic_load_uint64_t(&timer->impl->period);
  *time_until_next_call =
    (rcl_atomic_load_uint64_t(&timer->impl->last_call_time) + period) - now;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_get_time_since_last_call(
  const rcl_timer_t * timer,
  rcl_time_point_value_t * time_since_last_call)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(time_since_last_call, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  rcl_time_point_value_t now;
  rcl_ret_t ret = rcl_steady_time_now(&now);
  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
  *time_since_last_call =
    now - rcl_atomic_load_uint64_t(&timer->impl->last_call_time);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_get_period(const rcl_timer_t * timer, uint64_t * period)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(period, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  *period = rcl_atomic_load_uint64_t(&timer->impl->period);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_exchange_period(const rcl_timer_t * timer, uint64_t new_period, uint64_t * old_period)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(old_period, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  *old_period = rcl_atomic_exchange_uint64_t(&timer->impl->period, new_period);
  return RCL_RET_OK;
}

rcl_timer_callback_t
rcl_timer_get_callback(const rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return NULL);
  return (rcl_timer_callback_t)rcl_atomic_load_uintptr_t(&timer->impl->callback);
}

rcl_timer_callback_t
rcl_timer_exchange_callback(rcl_timer_t * timer, const rcl_timer_callback_t new_callback)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, NULL);
  RCL_CHECK_ARGUMENT_FOR_NULL(new_callback, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return NULL);
  return (rcl_timer_callback_t)rcl_atomic_exchange_uintptr_t(
    &timer->impl->callback, (uintptr_t)new_callback);
}

rcl_ret_t
rcl_timer_cancel(rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  rcl_atomic_store(&timer->impl->canceled, true);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_is_canceled(const rcl_timer_t * timer, bool * is_canceled)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_canceled, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  *is_canceled = rcl_atomic_load_bool(&timer->impl->canceled);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_timer_reset(rcl_timer_t * timer)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(timer->impl, "timer is invalid", return RCL_RET_TIMER_INVALID);
  rcl_time_point_value_t now;
  rcl_ret_t now_ret = rcl_steady_time_now(&now);
  if (now_ret != RCL_RET_OK) {
    return now_ret;  // rcl error state should already be set.
  }
  rcl_atomic_store(&timer->impl->last_call_time, now);
  rcl_atomic_store(&timer->impl->canceled, false);
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
