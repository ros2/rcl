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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "rcl/logging.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

TEST(CLASSNAME(TestLogging, RMW_IMPLEMENTATION), test_logging_initialization) {
  EXPECT_EQ(g_rcl_logging_initialized, false);
  rcl_logging_initialize();
  EXPECT_EQ(g_rcl_logging_initialized, true);
  rcl_logging_initialize();
  EXPECT_EQ(g_rcl_logging_initialized, true);
  g_rcl_logging_initialized = false;
  EXPECT_EQ(g_rcl_logging_initialized, false);
}

size_t g_log_calls = 0;

struct LogEvent
{
  rcl_log_location_t * location;
  int level;
  std::string name;
  std::string message;
};
LogEvent g_last_log_event;

TEST(CLASSNAME(TestLogging, RMW_IMPLEMENTATION), test_logging) {
  EXPECT_EQ(g_rcl_logging_initialized, false);
  rcl_logging_initialize();
  EXPECT_EQ(g_rcl_logging_initialized, true);

  auto rcl_logging_console_output_handler = [](
    rcl_log_location_t * location,
    int level, const char * name, const char * format, va_list * args) -> void
    {
      g_log_calls += 1;
      g_last_log_event.location = location;
      g_last_log_event.level = level;
      g_last_log_event.name = name ? name : "";
      char buffer[1024];
      vsnprintf(buffer, sizeof(buffer), format, *args);
      g_last_log_event.message = buffer;
    };

  rcl_logging_output_handler_t original_function = rcl_logging_get_output_handler();
  rcl_logging_set_output_handler(rcl_logging_console_output_handler);

  EXPECT_EQ(rcl_logging_get_severity_threshold(), RCL_LOG_SEVERITY_DEBUG);

  // check all attributes for a debug log message
  rcl_log_location_t location = {"func", "file", 42u};
  g_log_calls = 0;
  rcl_log(&location, RCL_LOG_SEVERITY_DEBUG, "name1", "message %d", 11);
  EXPECT_EQ(g_log_calls, 1u);
  EXPECT_TRUE(g_last_log_event.location != NULL);
  if (g_last_log_event.location) {
    EXPECT_STREQ(g_last_log_event.location->function_name, "func");
    EXPECT_STREQ(g_last_log_event.location->file_name, "file");
    EXPECT_EQ(g_last_log_event.location->line_number, 42u);
  }
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_DEBUG);
  EXPECT_EQ(g_last_log_event.name, "name1");
  EXPECT_EQ(g_last_log_event.message, "message 11");

  // check global severity threshold
  int original_severity_threshold = rcl_logging_get_severity_threshold();
  rcl_logging_set_severity_threshold(RCL_LOG_SEVERITY_INFO);
  EXPECT_EQ(rcl_logging_get_severity_threshold(), RCL_LOG_SEVERITY_INFO);
  rcl_log(NULL, RCL_LOG_SEVERITY_DEBUG, "name2", "message %d", 22);
  EXPECT_EQ(g_log_calls, 1u);

  // check other severity levels
  rcl_log(NULL, RCL_LOG_SEVERITY_INFO, "name3", "message %d", 33);
  EXPECT_EQ(g_log_calls, 2u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_INFO);
  EXPECT_EQ(g_last_log_event.name, "name3");
  EXPECT_EQ(g_last_log_event.message, "message 33");

  rcl_log(NULL, RCL_LOG_SEVERITY_WARN, "", "");
  EXPECT_EQ(g_log_calls, 3u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_WARN);

  rcl_log(NULL, RCL_LOG_SEVERITY_ERROR, "", "");
  EXPECT_EQ(g_log_calls, 4u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_ERROR);

  rcl_log(NULL, RCL_LOG_SEVERITY_FATAL, NULL, "");
  EXPECT_EQ(g_log_calls, 5u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_FATAL);

  // restore original state
  rcl_logging_set_severity_threshold(original_severity_threshold);
  rcl_logging_set_output_handler(original_function);
  g_rcl_logging_initialized = false;
  EXPECT_EQ(g_rcl_logging_initialized, false);
}
