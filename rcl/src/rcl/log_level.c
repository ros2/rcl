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

#ifdef __cplusplus
extern "C"
{
#endif

rcl_log_levels_t *
rcl_log_levels_init(const rcl_allocator_t allocator, size_t logger_count)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return NULL);
  rcl_log_levels_t * log_levels = allocator.allocate(sizeof(rcl_log_levels_t), allocator.state);
  if (NULL == log_levels) {
    RCL_SET_ERROR_MSG("Error allocating mem");
    return NULL;
  }

  log_levels->default_logger_level = RCUTILS_LOG_SEVERITY_UNSET;
  log_levels->logger_settings = NULL;
  log_levels->num_logger_settings = 0;
  log_levels->allocator = allocator;

  if (logger_count > 0) {
    log_levels->logger_settings = allocator.allocate(
      sizeof(rcl_logger_setting_t) * logger_count, allocator.state);
    if (NULL == log_levels->logger_settings) {
      RCL_SET_ERROR_MSG("Error allocating mem");
      if (RCL_RET_OK != rcl_log_levels_fini(log_levels)) {
        RCL_SET_ERROR_MSG("Error while finalizing log levels due to another error");
      }
      return NULL;
    }
  }

  return log_levels;
}

rcl_ret_t
rcl_log_levels_copy(const rcl_log_levels_t * log_levels, rcl_log_levels_t * log_levels_out)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels_out, RCL_RET_INVALID_ARGUMENT);

  rcutils_allocator_t allocator = log_levels->allocator;

  log_levels_out->default_logger_level = log_levels->default_logger_level;
  for (size_t i = 0; i < log_levels->num_logger_settings; ++i) {
    log_levels_out->logger_settings[i].name =
      rcutils_strdup(log_levels->logger_settings[i].name, allocator);
    if (NULL == log_levels_out->logger_settings[i].name) {
      if (RCL_RET_OK != rcl_log_levels_fini(log_levels_out)) {
        RCL_SET_ERROR_MSG("Error while finalizing log levels due to another error");
      }
      return RCL_RET_BAD_ALLOC;
    }
    log_levels_out->logger_settings[i].level =
      log_levels->logger_settings[i].level;
    log_levels_out->num_logger_settings++;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_log_levels_fini(rcl_log_levels_t * log_levels)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(log_levels, RCL_RET_INVALID_ARGUMENT);
  rcutils_allocator_t allocator = log_levels->allocator;
  if (log_levels->logger_settings) {
    if (log_levels->logger_settings) {
      for (size_t i = 0; i < log_levels->num_logger_settings; ++i) {
        allocator.deallocate(log_levels->logger_settings[i].name, allocator.state);
      }
      log_levels->num_logger_settings = 0;

      allocator.deallocate(log_levels->logger_settings, allocator.state);
      log_levels->logger_settings = NULL;
    }
  }
  allocator.deallocate(log_levels, allocator.state);
  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
