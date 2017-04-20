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

#if __cplusplus
extern "C"
{
#endif

#include "rcl/logging.h"

bool g_rcl_logging_initialized = false;

RclLogFunction g_rcl_logging_output_handler = NULL;

int g_rcl_logging_severity_threshold = 0;

void rcl_logging_initialize()
{
  if (!g_rcl_logging_initialized) {
    g_rcl_logging_initialized = true;
    g_rcl_logging_output_handler = &rcl_logging_console_output_handler;
    g_rcl_logging_severity_threshold = RCL_LOG_SEVERITY_DEBUG;
  }
}

RclLogFunction rcl_logging_get_output_handler()
{
  return g_rcl_logging_output_handler;
}

void rcl_logging_set_output_handler(RclLogFunction function)
{
  g_rcl_logging_output_handler = function;
}

int rcl_logging_get_severity_threshold()
{
  return g_rcl_logging_severity_threshold;
}

void rcl_logging_set_severity_threshold(int severity)
{
  g_rcl_logging_severity_threshold = severity;
}

void rcl_log(
  RclLogLocation * location,
  int severity, const char * name, const char * format, ...)
{
  if (severity < g_rcl_logging_severity_threshold) {
    return;
  }
  RclLogFunction output_handler = g_rcl_logging_output_handler;
  if (output_handler) {
    va_list args;
    va_start(args, format);
    (*output_handler)(location, severity, name, format, &args);
    va_end(args);
  }
}

void rcl_logging_console_output_handler(
  RclLogLocation * location,
  int severity, const char * name, const char * format, va_list * args)
{
  FILE * stream = NULL;
  const char * severity_string = "";
  switch (severity) {
    case RCL_LOG_SEVERITY_DEBUG:
      stream = stdout;
      severity_string = "DEBUG";
      break;
    case RCL_LOG_SEVERITY_INFO:
      stream = stdout;
      severity_string = "INFO";
      break;
    case RCL_LOG_SEVERITY_WARN:
      stream = stderr;
      severity_string = "WARN";
      break;
    case RCL_LOG_SEVERITY_ERROR:
      stream = stderr;
      severity_string = "ERROR";
      break;
    case RCL_LOG_SEVERITY_FATAL:
      stream = stderr;
      severity_string = "FATAL";
      break;
    default:
      fprintf(stderr, "unknown severity level: %d\n", severity);
      return;
  }

  char buffer[1024];
  int written = vsnprintf(buffer, sizeof(buffer), format, *args);
  if (written < 0) {
    fprintf(stderr, "failed to format message: '%s'\n", format);
    return;
  }

  if (!location) {
    fprintf(stream, "[%s] [%s]: %s\n", severity_string, name, buffer);
  } else {
    fprintf(
      stream, "[%s] [%s]: %s (%s() at %s:%zu)\n", severity_string, name, buffer,
      location->function_name, location->file_name, location->line_number);
  }
}

#if __cplusplus
}
#endif
