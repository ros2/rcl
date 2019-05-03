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
#include "rcl/logging_rosout.h"
#include "rcl/macros.h"
#include "rcutils/logging.h"
#include "rcutils/time.h"

#define RCL_LOGGING_MAX_OUTPUT_FUNCS (4)

static rcutils_logging_output_handler_t
  g_rcl_logging_out_handlers[RCL_LOGGING_MAX_OUTPUT_FUNCS] = {0};

static uint8_t g_rcl_logging_num_out_handlers = 0;
static rcl_allocator_t g_logging_allocator;
static bool g_rcl_logging_stdout_enabled = false;
static bool g_rcl_logging_rosout_enabled = false;
static bool g_rcl_logging_ext_lib_enabled = false;

/**
 * An output function that sends to multiple output appenders.
 */
static
void
rcl_logging_multiple_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, va_list * args);

/**
 * An output function that sends to the external logger library.
 */
static
void
rcl_logging_ext_lib_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, va_list * args);

rcl_ret_t
rcl_logging_configure(const rcl_arguments_t * global_args, const rcl_allocator_t * allocator)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(global_args, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);
  RCUTILS_LOGGING_AUTOINIT
    g_logging_allocator = *allocator;
  int default_level = global_args->impl->log_level;
  const char * config_file = global_args->impl->external_log_config_file;
  g_rcl_logging_stdout_enabled = !global_args->impl->log_stdout_disabled;
  g_rcl_logging_rosout_enabled = !global_args->impl->log_rosout_disabled;
  g_rcl_logging_ext_lib_enabled = !global_args->impl->log_ext_lib_disabled;
  rcl_ret_t status = RCL_RET_OK;
  g_rcl_logging_num_out_handlers = 0;

  if (default_level >= 0) {
    rcutils_logging_set_default_logger_level(default_level);
  }
  if (g_rcl_logging_stdout_enabled) {
    g_rcl_logging_out_handlers[g_rcl_logging_num_out_handlers++] =
      rcutils_logging_console_output_handler;
  }
  if (g_rcl_logging_rosout_enabled) {
    status = rcl_logging_rosout_init(allocator);
    if (RCL_RET_OK == status) {
      g_rcl_logging_out_handlers[g_rcl_logging_num_out_handlers++] =
        rcl_logging_rosout_output_handler;
    }
  }
  if (g_rcl_logging_ext_lib_enabled) {
    status = rcl_logging_external_initialize(config_file, g_logging_allocator);
    if (RCL_RET_OK == status) {
      rcl_logging_external_set_logger_level(NULL, default_level);
      g_rcl_logging_out_handlers[g_rcl_logging_num_out_handlers++] =
        rcl_logging_ext_lib_output_handler;
    }
  }
  rcutils_logging_set_output_handler(rcl_logging_multiple_output_handler);
  return status;
}

rcl_ret_t rcl_logging_fini()
{
  rcl_ret_t status = RCL_RET_OK;
  rcutils_logging_set_output_handler(rcutils_logging_console_output_handler);

  if (g_rcl_logging_rosout_enabled) {
    status = rcl_logging_rosout_fini();
  }
  if (RCL_RET_OK == status && g_rcl_logging_ext_lib_enabled) {
    status = rcl_logging_external_shutdown();
  }

  return status;
}

static
void
rcl_logging_multiple_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, va_list * args)
{
  for (uint8_t i = 0;
    i < g_rcl_logging_num_out_handlers && NULL != g_rcl_logging_out_handlers[i]; ++i)
  {
    g_rcl_logging_out_handlers[i](location, severity, name, timestamp, format, args);
  }
}

static
void
rcl_logging_ext_lib_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, va_list * args)
{
  rcl_ret_t status;
  char msg_buf[1024] = "";
  rcutils_char_array_t msg_array = {
    .buffer = msg_buf,
    .owns_buffer = false,
    .buffer_length = 0u,
    .buffer_capacity = sizeof(msg_buf),
    .allocator = g_logging_allocator
  };

  char output_buf[1024] = "";
  rcutils_char_array_t output_array = {
    .buffer = output_buf,
    .owns_buffer = false,
    .buffer_length = 0u,
    .buffer_capacity = sizeof(output_buf),
    .allocator = g_logging_allocator
  };

  va_list args_clone;
  va_copy(args_clone, *args);
  status = rcutils_char_array_vsprintf(&msg_array, format, args_clone);
  va_end(args_clone);

  if (RCL_RET_OK == status) {
    status = rcutils_logging_format_message(
      location, severity, name, timestamp, msg_array.buffer, &output_array);
    if (RCL_RET_OK != status) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("failed to format log message: ");
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
      rcl_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    }
  } else {
    RCUTILS_SAFE_FWRITE_TO_STDERR("failed to format user log message: ");
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
    rcl_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
  }
  rcl_logging_external_log(severity, name, output_array.buffer);
  status = rcutils_char_array_fini(&msg_array);
  if (RCL_RET_OK != status) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("failed to finalize char array: ");
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
    rcl_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
  }
  status = rcutils_char_array_fini(&output_array);
  if (RCL_RET_OK != status) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("failed to finalize char array: ");
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
    rcl_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
  }
}

#ifdef __cplusplus
}
#endif
