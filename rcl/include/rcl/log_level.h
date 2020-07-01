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

#ifndef RCL__LOG_LEVEL_H_
#define RCL__LOG_LEVEL_H_

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// A logger item to specify a name and a log level
typedef struct rcl_logger_setting_t
{
  /// name for the logger.
  char * name;
  /// level for the logger.
  rcl_log_severity_t level;
} rcl_logger_setting_t;

/// Hold default logger level and other logger setting.
typedef struct rcl_log_levels_t
{
  /// Default logger level
  rcl_log_severity_t default_logger_level;
  /// Array of logger setting
  struct rcl_logger_setting_t * logger_settings;
  /// Number of logger settings
  size_t num_logger_settings;
  /// Allocator used to allocate objects in this struct
  rcl_allocator_t allocator;
} rcl_log_levels_t;

/// Initialize a log levels structure.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] allocator memory allocator to be used.
 * \param[in] logger_count to allocate the logger setting count of log levels.
 * \return a pointer to log level structure on success or NULL on failure.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_log_levels_t *
rcl_log_levels_init(const rcl_allocator_t allocator, size_t logger_count);

/// Copy one log levels structure into another.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] log_levels The structure to be copied.
 *  Its allocator is used to copy memory into the new structure.
 * \param[out] log_levels_out A zero-initialized log levels structure to be copied into.
 * \return `RCL_RET_OK` if the structure was copied successfully, or
 * \return `RCL_RET_INVALID_ARGUMENT` if log_levels or log_levels_out are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_log_levels_copy(const rcl_log_levels_t * log_levels, rcl_log_levels_t * log_levels_out);

/// Free log levels structure.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] log_levels The structure to be deallocated.
 * \return `RCL_RET_OK` if the memory was successfully freed, or
 * \return `RCL_RET_INVALID_ARGUMENT` if log_levels is invalid.
 */
RCL_PUBLIC
rcl_ret_t
rcl_log_levels_fini(rcl_log_levels_t * log_levels);

#ifdef __cplusplus
}
#endif

#endif  // RCL__LOG_LEVEL_H_
