// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rcl/log_level.h"

#include "rcl/error_handling.h"
#include "rcutils/allocator.h"
#include "rcutils/strdup.h"

rcl_log_levels_t *
rcl_log_levels_init(const rcl_allocator_t * allocator, size_t logger_count)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return NULL);
  rcl_log_levels_t * log_levels = allocator->allocate(sizeof(rcl_log_levels_t), allocator->state);
  if (NULL == log_levels) {
    RCL_SET_ERROR_MSG("Error allocating memory");
    return NULL;
  }

  log_levels->default_logger_level = RCUTILS_LOG_SEVERITY_UNSET;
  log_levels->logger_settings = NULL;
  log_levels->num_logger_settings = 0;
  log_levels->capacity_logger_settings = logger_count;
  log_levels->allocator = *allocator;

  if (logger_count > 0) {
    log_levels->logger_settings = allocator->allocate(
      sizeof(rcl_logger_setting_t) * logger_count, allocator->state);
    if (NULL == log_levels->logger_settings) {
      RCL_SET_ERROR_MSG("Error allocating memory");
      if (RCL_RET_OK != rcl_log_levels_fini(log_levels)) {
        RCL_SET_ERROR_MSG("Error while finalizing log levels due to another error");
      }
      return NULL;
    }
  }

  return log_levels;
}

rcl_ret_t
rcl_log_levels_copy(const rcl_log_levels_t * src, rcl_log_levels_t * dst)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(src, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(dst, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t allocator = src->allocator;

  dst->default_logger_level = src->default_logger_level;
  for (size_t i = 0; i < src->num_logger_settings; ++i) {
    dst->logger_settings[i].name =
      rcutils_strdup(src->logger_settings[i].name, allocator);
    if (NULL == dst->logger_settings[i].name) {
      return RCL_RET_BAD_ALLOC;
    }
    dst->logger_settings[i].level =
      src->logger_settings[i].level;
    dst->num_logger_settings++;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_log_levels_fini(rcl_log_levels_t * log_levels)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t allocator = log_levels->allocator;
  if (log_levels->logger_settings) {
    for (size_t i = 0; i < log_levels->num_logger_settings; ++i) {
      allocator.deallocate((void *)log_levels->logger_settings[i].name, allocator.state);
    }
    log_levels->num_logger_settings = 0;

    allocator.deallocate(log_levels->logger_settings, allocator.state);
    log_levels->logger_settings = NULL;
  }
  allocator.deallocate(log_levels, allocator.state);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_log_levels_shrink_to_size(rcl_log_levels_t * log_levels)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t allocator = log_levels->allocator;
  if (0U == log_levels->num_logger_settings) {
    allocator.deallocate(log_levels->logger_settings, allocator.state);
    log_levels->logger_settings = NULL;
  } else if (log_levels->num_logger_settings < log_levels->capacity_logger_settings) {
    log_levels->logger_settings = rcutils_reallocf(
      log_levels->logger_settings,
      sizeof(rcl_logger_setting_t) * log_levels->num_logger_settings, &allocator);
    if (NULL == log_levels->logger_settings) {
      return RCL_RET_BAD_ALLOC;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_log_levels_add_logger_setting(
  rcl_log_levels_t * log_levels, const char * logger_name, rcl_log_severity_t log_level)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels->logger_settings, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(logger_name, RCL_RET_INVALID_ARGUMENT);
  if (log_levels->num_logger_settings >= log_levels->capacity_logger_settings) {
    RCL_SET_ERROR_MSG("No capacity to store a logger setting");
    return RCL_RET_ERROR;
  }

  rcl_logger_setting_t * logger_setting =
    &log_levels->logger_settings[log_levels->num_logger_settings];
  logger_setting->name = logger_name;
  logger_setting->level = (rcl_log_severity_t)log_level;
  log_levels->num_logger_settings += 1;
  return RCL_RET_OK;
}
