// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__LOGGING_H_
#define RCL__LOGGING_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "rcl/visibility_control.h"

#if __cplusplus
extern "C"
{
#endif

/// The flag if the logging system has been initialized.
RCL_PUBLIC
extern bool g_rcl_logging_initialized;

/// Initialize the logging system.
/**
 * The function is called automatically when using the logging macros.
 */
RCL_PUBLIC
void rcl_logging_initialize();

/// The structure identifying the caller location in the source code.
typedef struct rcl_log_location_t
{
  /// The name of the function containing the log call.
  const char * function_name;
  /// The name of the source file containing the log call.
  const char * file_name;
  /// The line number containing the log call.
  size_t line_number;
} rcl_log_location_t;

/// The severity levels of log message.
enum RCL_LOG_SEVERITY
{
  RCL_LOG_SEVERITY_DEBUG = 0,  ///< The debug log level
  RCL_LOG_SEVERITY_INFO = 1,  ///< The info log level
  RCL_LOG_SEVERITY_WARN = 2,  ///< The warn log level
  RCL_LOG_SEVERITY_ERROR = 3,  ///< The error log level
  RCL_LOG_SEVERITY_FATAL = 4,  ///< The fatal log level
};

/// The function signature to log messages.
/**
 * \param The pointer to the location struct
 * \param The severity level
 * \param The name of the logger
 * \param The format string
 * \param The variable argument list
 */
typedef void (* rcl_logging_output_handler_t)(
  rcl_log_location_t *,  // location
  int,  // severity
  const char *,  // name
  const char *,  // format
  va_list *  // args
);

/// The function pointer of the current output handler.
RCL_PUBLIC
extern rcl_logging_output_handler_t g_rcl_logging_output_handler;

/// Get the current output handler.
/**
 * \return The function pointer of the current output handler.
 */
RCL_PUBLIC
rcl_logging_output_handler_t rcl_logging_get_output_handler();

/// Set the current output handler.
/**
 * \param function The function pointer of the output handler to be used.
 */
RCL_PUBLIC
void rcl_logging_set_output_handler(rcl_logging_output_handler_t function);

/// The global severity threshold before calling the output handler.
/**
 * The global severity threshold is being checked after the conditions when
 * using the various logging macros.
 *
 * \param severity The global severity threshold to be used.
 */
RCL_PUBLIC
extern int g_rcl_logging_severity_threshold;

/// Get the global severity threshold.
/**
 * \return The severity threshold.
 */
RCL_PUBLIC
int rcl_logging_get_severity_threshold();

/// Set the global severity threshold.
/**
 * \param severity The severity threshold to be used.
 */
RCL_PUBLIC
void rcl_logging_set_severity_threshold(int severity);

/// Log a message.
/**
 * \param location The pointer to the location struct
 * \param severity The severity level
 * \param name The name of the logger
 * \param format The format string
 * \param ... The variable arguments
 */
RCL_PUBLIC
void rcl_log(
  rcl_log_location_t * location,
  int severity,
  const char * name,
  const char * format,
  ...);

/// The default output handler outputs log messages to the standard streams.
/**
 * The messages with a severity `DEBUG` and `INFO` are written to `stdout`.
 * The messages with a severity `WARN`, `ERROR`, and `FATAL` are written to
 * `stderr`.
 * For each message the severity and name is prepended and the location
 * information is appended when available.
 *
 * \param location The pointer to the location struct
 * \param severity The severity level
 * \param name The name of the logger
 * \param format The format string
 * \param args The variable argument list
 */
RCL_PUBLIC
void rcl_logging_console_output_handler(
  rcl_log_location_t * location,
  int severity, const char * name, const char * format, va_list * args);

#if __cplusplus
}
#endif

#endif  // RCL__LOGGING_H_
