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

#define MAX_NUM_LOGGER_SETTING_ENTRIES 256U

rcl_log_level_t *
rcl_log_level_init(const rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return NULL);
  rcl_log_level_t * log_level = allocator.allocate(sizeof(rcl_log_level_t), allocator.state);
  if (NULL == log_level) {
    RCL_SET_ERROR_MSG("Error allocating mem");
    return NULL;
  }

  log_level->logger_settings = allocator.allocate(
    sizeof(rcl_logger_setting_t) * MAX_NUM_LOGGER_SETTING_ENTRIES, allocator.state);
  if (NULL == log_level->logger_settings) {
    RCL_SET_ERROR_MSG("Error allocating mem");
    rcl_log_level_fini(log_level);
    return NULL;
  }

  log_level->default_logger_level = RCUTILS_LOG_SEVERITY_UNSET;
  log_level->num_logger_settings = 0;
  log_level->allocator = allocator;
  return log_level;
}

rcl_log_level_t * rcl_log_level_copy(
  const rcl_log_level_t * log_level)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(log_level, NULL);

  rcutils_allocator_t allocator = log_level->allocator;
  rcl_log_level_t * out_log_level = rcl_log_level_init(allocator);

  if (NULL == out_log_level) {
    RCL_SET_ERROR_MSG("Error allocating mem");
    return NULL;
  }

  out_log_level->default_logger_level = log_level->default_logger_level;
  for (size_t i = 0; i < log_level->num_logger_settings; ++i) {
    out_log_level->logger_settings[i].name =
      rcutils_strdup(log_level->logger_settings[i].name, allocator);
    if (NULL == out_log_level->logger_settings[i].name) {
      RCL_SET_ERROR_MSG("Error allocating mem");
      goto fail;
    }
    out_log_level->logger_settings[i].level =
      log_level->logger_settings[i].level;
    out_log_level->num_logger_settings++;
  }

  return out_log_level;

fail:
  rcl_log_level_fini(out_log_level);
  return NULL;
}

void
rcl_log_level_fini(
  rcl_log_level_t * log_level)
{
  rcutils_allocator_t allocator = log_level->allocator;
  if (log_level->logger_settings) {
    for (size_t i = 0; i < log_level->num_logger_settings; ++i) {
      allocator.deallocate(log_level->logger_settings[i].name, allocator.state);
    }
    log_level->num_logger_settings = 0;

    allocator.deallocate(log_level->logger_settings, allocator.state);
    log_level->logger_settings = NULL;
  }

  allocator.deallocate(log_level, allocator.state);
}

#ifdef __cplusplus
}
#endif
