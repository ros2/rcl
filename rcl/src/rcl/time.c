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

// Process default ROS time sources
static rcl_time_source_t * rcl_default_ros_time_source;
static rcl_time_source_t * rcl_default_steady_time_source;
static rcl_time_source_t * rcl_default_system_time_source;

// Internal storage for RCL_ROS_TIME implementation
typedef struct rcl_ros_time_source_storage_t
{
  atomic_uint_least64_t current_time;
  bool active;
  // TODO(tfoote): store subscription here
} rcl_ros_time_source_storage_t;

// Implementation only
rcl_ret_t
rcl_get_steady_time(void * data, rcl_time_point_value_t * current_time)
{
  (void)data;  // unused
  return rcl_steady_time_now(current_time);
}

// Implementation only
rcl_ret_t
rcl_get_system_time(void * data, rcl_time_point_value_t * current_time)
{
  (void)data;  // unused
  return rcl_system_time_now(current_time);
}

// Internal method for zeroing values on init, assumes time_source is valid
void
rcl_init_generic_time_source(rcl_time_source_t * time_source)
{
  time_source->type = RCL_TIME_SOURCE_UNINITIALIZED;
  time_source->pre_update = NULL;
  time_source->post_update = NULL;
  time_source->get_now = NULL;
  time_source->data = NULL;
}

// The function used to get the current ros time.
// This is in the implementation only
rcl_ret_t
rcl_get_ros_time(void * data, rcl_time_point_value_t * current_time)
{
  rcl_ros_time_source_storage_t * t = (rcl_ros_time_source_storage_t *)data;
  if (!t->active) {
    return rcl_get_system_time(data, current_time);
  }
  *current_time = rcl_atomic_load_uint64_t(&(t->current_time));
  return RCL_RET_OK;
}

bool
rcl_time_source_valid(rcl_time_source_t * time_source)
{
  if (time_source == NULL ||
    time_source->type == RCL_TIME_SOURCE_UNINITIALIZED ||
    time_source->get_now == NULL)
  {
    return false;
  }
  return true;
}

rcl_ret_t
rcl_time_source_init(
  enum rcl_time_source_type_t time_source_type, rcl_time_source_t * time_source
)
{
  switch (time_source_type) {
    case RCL_TIME_SOURCE_UNINITIALIZED:
      RCL_CHECK_ARGUMENT_FOR_NULL(
        time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
      rcl_init_generic_time_source(time_source);
      return RCL_RET_OK;
    case RCL_ROS_TIME:
      return rcl_ros_time_source_init(time_source);
    case RCL_SYSTEM_TIME:
      return rcl_system_time_source_init(time_source);
    case RCL_STEADY_TIME:
      return rcl_steady_time_source_init(time_source);
    default:
      return RCL_RET_INVALID_ARGUMENT;
  }
}

rcl_ret_t
rcl_time_source_fini(rcl_time_source_t * time_source)
{
  switch (time_source->type) {
    case RCL_ROS_TIME:
      return rcl_ros_time_source_fini(time_source);
    case RCL_SYSTEM_TIME:
      return rcl_system_time_source_fini(time_source);
    case RCL_STEADY_TIME:
      return rcl_steady_time_source_fini(time_source);
    case RCL_TIME_SOURCE_UNINITIALIZED:
    // fall through
    default:
      return RCL_RET_INVALID_ARGUMENT;
  }
}

rcl_ret_t
rcl_ros_time_source_init(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_init_generic_time_source(time_source);
  time_source->data = calloc(1, sizeof(rcl_ros_time_source_storage_t));
  time_source->get_now = rcl_get_ros_time;
  time_source->type = RCL_ROS_TIME;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_ros_time_source_fini(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_ROS_TIME) {
    RCL_SET_ERROR_MSG("time_source not of type RCL_ROS_TIME", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  free((rcl_ros_time_source_storage_t *)time_source->data);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_steady_time_source_init(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_init_generic_time_source(time_source);
  time_source->get_now = rcl_get_steady_time;
  time_source->type = RCL_STEADY_TIME;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_steady_time_source_fini(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_STEADY_TIME) {
    RCL_SET_ERROR_MSG("time_source not of type RCL_STEADY_TIME", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_system_time_source_init(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_init_generic_time_source(time_source);
  time_source->get_now = rcl_get_system_time;
  time_source->type = RCL_SYSTEM_TIME;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_system_time_source_fini(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_SYSTEM_TIME) {
    RCL_SET_ERROR_MSG("time_source not of type RCL_SYSTEM_TIME", rcl_get_default_allocator());
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_time_point_init(rcl_time_point_t * time_point, rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_point, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (!time_source) {
    time_point->time_source = rcl_get_default_ros_time_source();
    return RCL_RET_OK;
  }
  time_point->time_source = time_source;

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
rcl_duration_init(rcl_duration_t * duration, rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(duration, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (!time_source) {
    duration->time_source = rcl_get_default_ros_time_source();
    return RCL_RET_OK;
  }
  duration->time_source = time_source;

  return RCL_RET_OK;
}

rcl_ret_t
rcl_duration_fini(rcl_duration_t * duration)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(duration, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  (void)duration;
  return RCL_RET_OK;
}

rcl_time_source_t *
rcl_get_default_ros_time_source(void)
{
  if (!rcl_default_ros_time_source) {
    rcl_default_ros_time_source = (rcl_time_source_t *)calloc(1, sizeof(rcl_time_source_t));
    rcl_ret_t retval = rcl_ros_time_source_init(rcl_default_ros_time_source);
    if (retval != RCL_RET_OK) {
      return NULL;
    }
  }
  return rcl_default_ros_time_source;
}

rcl_time_source_t *
rcl_get_default_steady_time_source(void)
{
  if (!rcl_default_steady_time_source) {
    rcl_default_steady_time_source = (rcl_time_source_t *)calloc(1, sizeof(rcl_time_source_t));
    rcl_ret_t retval = rcl_steady_time_source_init(rcl_default_steady_time_source);
    if (retval != RCL_RET_OK) {
      return NULL;
    }
  }
  return rcl_default_steady_time_source;
}

rcl_time_source_t *
rcl_get_default_system_time_source(void)
{
  if (!rcl_default_system_time_source) {
    rcl_default_system_time_source = (rcl_time_source_t *)calloc(1, sizeof(rcl_time_source_t));
    rcl_ret_t retval = rcl_system_time_source_init(rcl_default_system_time_source);
    if (retval != RCL_RET_OK) {
      return NULL;
    }
  }
  return rcl_default_system_time_source;
}

rcl_ret_t
rcl_set_default_ros_time_source(rcl_time_source_t * process_time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(
    process_time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (rcl_default_ros_time_source) {
    free(rcl_default_ros_time_source);
  }
  rcl_default_ros_time_source = process_time_source;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_difference_times(rcl_time_point_t * start, rcl_time_point_t * finish,
  rcl_duration_t * delta)
{
  if (start->time_source->type != finish->time_source->type) {
    RCL_SET_ERROR_MSG(
      "Cannot difference between time points with time_sources types.",
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
rcl_time_point_get_now(rcl_time_point_t * time_point)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_point, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_point->time_source && time_point->time_source->get_now) {
    return time_point->time_source->get_now(time_point->time_source->data,
             &(time_point->nanoseconds));
  }
  RCL_SET_ERROR_MSG(
    "time_source is not initialized or does not have get_now registered.",
    rcl_get_default_allocator());
  return RCL_RET_ERROR;
}

rcl_ret_t
rcl_enable_ros_time_override(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_ROS_TIME) {
    RCL_SET_ERROR_MSG(
      "Time source is not RCL_ROS_TIME cannot enable override.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  rcl_ros_time_source_storage_t * storage = \
    (rcl_ros_time_source_storage_t *)time_source->data;
  if (!storage) {
    RCL_SET_ERROR_MSG("Storage not initialized, cannot enable.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  storage->active = true;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_disable_ros_time_override(rcl_time_source_t * time_source)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_ROS_TIME) {
    return RCL_RET_ERROR;
  }
  rcl_ros_time_source_storage_t * storage = \
    (rcl_ros_time_source_storage_t *)time_source->data;
  if (!storage) {
    RCL_SET_ERROR_MSG("Storage not initialized, cannot disable.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  storage->active = false;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_is_enabled_ros_time_override(
  rcl_time_source_t * time_source,
  bool * is_enabled)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(is_enabled, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_ROS_TIME) {
    return RCL_RET_ERROR;
  }
  rcl_ros_time_source_storage_t * storage = \
    (rcl_ros_time_source_storage_t *)time_source->data;
  if (!storage) {
    RCL_SET_ERROR_MSG("Storage not initialized, cannot query.", rcl_get_default_allocator())
    return RCL_RET_ERROR;
  }
  *is_enabled = storage->active;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_set_ros_time_override(
  rcl_time_source_t * time_source,
  rcl_time_point_value_t time_value)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(time_source, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (time_source->type != RCL_ROS_TIME) {
    return RCL_RET_ERROR;
  }
  rcl_ros_time_source_storage_t * storage = \
    (rcl_ros_time_source_storage_t *)time_source->data;
  if (storage->active && time_source->pre_update) {
    time_source->pre_update();
  }
  rcl_atomic_store(&(storage->current_time), time_value);
  if (storage->active && time_source->post_update) {
    time_source->post_update();
  }
  return RCL_RET_OK;
}
