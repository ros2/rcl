// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "./arguments_impl.h"
#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/logging.h"
#include "rcl/logging_external_interface.h"
#include "rcl/macros.h"
#include "rcutils/logging.h"
#include "rcutils/time.h"

#define RCL_LOGGING_MAX_OUTPUT_FUNCS (4)

static rcutils_logging_output_handler_t
  g_rcl_logging_out_handlers[RCL_LOGGING_MAX_OUTPUT_FUNCS] = {0};

static uint8_t g_rcl_logging_num_out_handlers = 0;

/**
 * An output function that sends to multiple output appenders.
 */
static
void
rcl_logging_multiple_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str);

/**
 * An output function that sends to the external logger library.
 */
static
void
rcl_logging_ext_lib_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str);

/**
 * An output function that sends to the rosout topic.
 */
static
void
rcl_logging_rosout_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str);

rcl_ret_t
rcl_logging_configure(const rcl_arguments_t * global_args)
{
  RCUTILS_LOGGING_AUTOINIT
  int default_level = global_args->impl->log_level;
  const char * config_file = global_args->impl->external_log_config_file;
  bool enable_stdout = !global_args->impl->log_stdout_disabled;
  bool enable_rosout = !global_args->impl->log_rosout_disabled;
  bool enable_ext_lib = !global_args->impl->log_ext_lib_disabled;
  rcl_ret_t status = RCL_RET_OK;
  g_rcl_logging_num_out_handlers = 0;

  if (default_level >= 0) {
    rcutils_logging_set_default_logger_level(default_level);
  }
  if (enable_stdout) {
    g_rcl_logging_out_handlers[g_rcl_logging_num_out_handlers++] =
      rcutils_logging_console_output_handler;
  }
  if (enable_rosout) {
    g_rcl_logging_out_handlers[g_rcl_logging_num_out_handlers++] =
      rcl_logging_rosout_output_handler;
  }
  if (enable_ext_lib) {
    status = rcl_logging_external_initialize(config_file);
    if (RCL_RET_OK == status) {
      rcl_logging_external_set_logger_level(NULL, default_level);
      g_rcl_logging_out_handlers[g_rcl_logging_num_out_handlers++] =
        rcl_logging_ext_lib_output_handler;
    }
  }
  rcutils_logging_set_output_handler(rcl_logging_multiple_output_handler);
  return status;
}


static
void
rcl_logging_multiple_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str)
{
  for (uint8_t i = 0;
    i < g_rcl_logging_num_out_handlers && NULL != g_rcl_logging_out_handlers[i]; ++i)
  {
    g_rcl_logging_out_handlers[i](location, severity, name, timestamp, log_str);
  }
}

static
void
rcl_logging_ext_lib_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str)
{
  RCL_UNUSED(location);
  RCL_UNUSED(timestamp);
  rcl_logging_external_log(severity, name, log_str);
}

static
void
rcl_logging_rosout_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str)
{
  // TODO(nburek): Placeholder for rosout topic feature
  RCL_UNUSED(location);
  RCL_UNUSED(severity);
  RCL_UNUSED(name);
  RCL_UNUSED(timestamp);
  RCL_UNUSED(log_str);
}

#ifdef __cplusplus
}
#endif
