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

#include "rcl/time.h"

#include <stdbool.h>
#include <stdlib.h>

#include "./common.h"
#include "./stdatomic_helper.h"
#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcutils/time.h"


// Internal storage for RCL_ROS_TIME implementation
typedef struct rcl_ros_clock_storage_t
{
  atomic_uint_least64_t current_time;
  bool active;
  // TODO(tfoote): store subscription here
} rcl_ros_clock_storage_t;

// Implementation only
rcl_ret_t
rcl_get_steady_time(void * data, rcl_time_point_value_t * current_time)
{
  (void)data;  // unused
  return rcutils_steady_time_now(current_time);
}

// Implementation only
rcl_ret_t
rcl_get_system_time(void * data, rcl_time_point_value_t * current_time)
{
  (void)data;  // unused
  return rcutils_system_time_now(current_time);
}

// Internal method for zeroing values on init, assumes clock is valid
void
rcl_init_generic_clock(rcl_clock_t * clock)
{
  clock->type = RCL_CLOCK_UNINITIALIZED;
  clock->pre_update = NULL;
  clock->post_update = NULL;
  clock->get_now = NULL;
  clock->data = NULL;
}

// The function used to get the current ros time.
// This is in the implementation only
rcl_ret_t
rcl_get_ros_time(void * data, rcl_time_point_value_t * current_time)
{
  rcl_ros_clock_storage_t * t = (rcl_ros_clock_storage_t *)data;
  if (!t->active) {
    return rcl_get_system_time(data, current_time);
  }
  *current_time = rcl_atomic_load_uint64_t(&(t->current_time));
  return RCL_RET_OK;
}

bool
rcl_clock_valid(rcl_clock_t * clock)
{
  if (clock == NULL ||
    clock->type == RCL_CLOCK_UNINITIALIZED ||
    clock->get_now == NULL)
  {
    return false;
  }
  return true;
}

rcl_ret_t
rcl_clock_init(
  enum rcl_clock_type_t clock_type, rcl_clock_t * clock
)
{
  switch (clock_type) {
    case RCL_CLOCK_UNINITIALIZED:
      RCL_CHECK_ARGUMENT_FOR_NULL(
        clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
      rcl_init_generic_clock(clock);
      return RCL_RET_OK;
    case RCL_ROS_TIME:
      return rcl_ros_clock_init(clock);
    case RCL_SYSTEM_TIME:
      return rcl_system_clock_init(clock);
    case RCL_STEADY_TIME:
      return rcl_steady_clock_init(clock);
    default:
      return RCL_RET_INVALID_ARGUMENT;
  }
}

rcl_ret_t
rcl_clock_fini(rcl_clock_t * clock)
{
  switch (clock->type) {
    case RCL_ROS_TIME:
      return rcl_ros_clock_fini(clock);
    case RCL_SYSTEM_TIME:
      return rcl_system_clock_fini(clock);
    case RCL_STEADY_TIME:
      return rcl_steady_clock_fini(clock);
    case RCL_CLOCK_UNINITIALIZED:
    // fall through
    default:
      return RCL_RET_INVALID_ARGUMENT;
  }
}

rcl_ret_t
rcl_ros_clock_init(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_init_generic_clock(clock);
  clock->data = calloc(1, sizeof(rcl_ros_clock_storage_t));
  clock->get_now = rcl_get_ros_time;
  clock->type = RCL_ROS_TIME;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_ros_clock_fini(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_ROS_TIME) {
    RCL_SET_ERROR_MSG("clock not of type RCL_ROS_TIME", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  free((rcl_ros_clock_storage_t *)clock->data);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_steady_clock_init(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_init_generic_clock(clock);
  clock->get_now = rcl_get_steady_time;
  clock->type = RCL_STEADY_TIME;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_steady_clock_fini(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_STEADY_TIME) {
    RCL_SET_ERROR_MSG("clock not of type RCL_STEADY_TIME", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_system_clock_init(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_init_generic_clock(clock);
  clock->get_now = rcl_get_system_time;
  clock->type = RCL_SYSTEM_TIME;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_system_clock_fini(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_SYSTEM_TIME) {
    RCL_SET_ERROR_MSG("clock not of type RCL_SYSTEM_TIME", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_time_point_init(rcl_time_point_t * time_point, rcl_clock_type_t * clock_type)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_point, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(clock_type, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  time_point->clock_type = *clock_type;

  return RCL_RET_OK;
}

rcl_ret_t
rcl_time_point_fini(rcl_time_point_t * time_point)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_point, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  (void)time_point;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_duration_init(rcl_duration_t * duration, rcl_clock_type_t * clock_type)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(duration, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(clock_type, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  duration->clock_type = *clock_type;

  return RCL_RET_OK;
}

rcl_ret_t
rcl_duration_fini(rcl_duration_t * duration)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(duration, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  (void)duration;
  return RCL_RET_OK;
}


rcl_ret_t
rcl_difference_times(
  rcl_time_point_t * start, rcl_time_point_t * finish, rcl_duration_t * delta)
{
  if (start->clock_type != finish->clock_type) {
    RCL_SET_ERROR_MSG(
      "Cannot difference between time points with clocks types.",
      rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  if (finish->nanoseconds < start->nanoseconds) {
    rcl_time_point_value_t intermediate = start->nanoseconds - finish->nanoseconds;
    delta->nanoseconds = -1 * (int) intermediate;
  }
  delta->nanoseconds = (int)(finish->nanoseconds - start->nanoseconds);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_time_point_get_now(rcl_clock_t * clock, rcl_time_point_t * time_point)
{
  // TODO(tfoote) switch to use external time source
  RCL_CHECK_ARGUMENT_FOR_NULL(time_point, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_point->clock_type && clock->get_now) {
    return clock->get_now(clock->data,
             &(time_point->nanoseconds));
  }
  RCL_SET_ERROR_MSG(
    "clock is not initialized or does not have get_now registered.",
    rcl_get_default_allocator());
  return RCL_RET_ERROR;
}

rcl_ret_t
rcl_enable_ros_time_override(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_ROS_TIME) {
    RCL_SET_ERROR_MSG(
      "Time source is not RCL_ROS_TIME cannot enable override.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  rcl_ros_clock_storage_t * storage = \
    (rcl_ros_clock_storage_t *)clock->data;
  if (!storage) {
    RCL_SET_ERROR_MSG("Storage not initialized, cannot enable.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  storage->active = true;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_disable_ros_time_override(rcl_clock_t * clock)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_ROS_TIME) {
    return RCL_RET_ERROR;
  }
  rcl_ros_clock_storage_t * storage = \
    (rcl_ros_clock_storage_t *)clock->data;
  if (!storage) {
    RCL_SET_ERROR_MSG("Storage not initialized, cannot disable.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  storage->active = false;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_is_enabled_ros_time_override(
  rcl_clock_t * clock,
  bool * is_enabled)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(is_enabled, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_ROS_TIME) {
    return RCL_RET_ERROR;
  }
  rcl_ros_clock_storage_t * storage = \
    (rcl_ros_clock_storage_t *)clock->data;
  if (!storage) {
    RCL_SET_ERROR_MSG("Storage not initialized, cannot query.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  *is_enabled = storage->active;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_set_ros_time_override(
  rcl_clock_t * clock,
  rcl_time_point_value_t time_value)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (clock->type != RCL_ROS_TIME) {
    return RCL_RET_ERROR;
  }
  rcl_ros_clock_storage_t * storage = \
    (rcl_ros_clock_storage_t *)clock->data;
  if (storage->active && clock->pre_update) {
    clock->pre_update();
  }
  rcl_atomic_store(&(storage->current_time), time_value);
  if (storage->active && clock->post_update) {
    clock->post_update();
  }
  return RCL_RET_OK;
}
