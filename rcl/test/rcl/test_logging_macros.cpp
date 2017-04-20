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

#include <gmock/gmock.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "rcl/logging_macros.h"
#include "rcl/time.h"

using ::testing::EndsWith;

size_t g_log_calls = 0;

struct LogEvent
{
  RclLogLocation * location;
  int level;
  std::string name;
  std::string message;
};
LogEvent g_last_log_event;

class TestLoggingMacros : public ::testing::Test
{
public:
  RclLogFunction previous_output_handler;
  void SetUp()
  {
    g_log_calls = 0;
    EXPECT_EQ(g_rcl_logging_initialized, false);
    rcl_logging_initialize();
    EXPECT_EQ(g_rcl_logging_initialized, true);

    auto rcl_logging_console_output_handler = [](
      RclLogLocation * location,
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

    this->previous_output_handler = rcl_logging_get_output_handler();
    rcl_logging_set_output_handler(rcl_logging_console_output_handler);
  }

  void TearDown()
  {
    rcl_logging_set_output_handler(this->previous_output_handler);
    g_rcl_logging_initialized = false;
    EXPECT_EQ(g_rcl_logging_initialized, false);
  }
};

TEST_F(TestLoggingMacros, test_logging_named) {
  for (int i : {1, 2, 3}) {
    RCL_LOG_DEBUG_NAMED("name", "message %d", i);
  }
  EXPECT_EQ(g_log_calls, 3u);
  EXPECT_TRUE(g_last_log_event.location != NULL);
  if (g_last_log_event.location) {
    EXPECT_STREQ(g_last_log_event.location->function_name, "TestBody");
    EXPECT_THAT(g_last_log_event.location->file_name, EndsWith("test_logging_macros.cpp"));
    EXPECT_EQ(g_last_log_event.location->line_number, 76u);
  }
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_DEBUG);
  EXPECT_EQ(g_last_log_event.name, "name");
  EXPECT_EQ(g_last_log_event.message, "message 3");
}

TEST_F(TestLoggingMacros, test_logging_once) {
  for (int i : {1, 2, 3}) {
    RCL_LOG_INFO_ONCE("message %d", i);
  }
  EXPECT_EQ(g_log_calls, 1u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_INFO);
  EXPECT_EQ(g_last_log_event.name, "");
  EXPECT_EQ(g_last_log_event.message, "message 1");
}

TEST_F(TestLoggingMacros, test_logging_expression) {
  for (int i : {1, 2, 3, 4, 5, 6}) {
    RCL_LOG_INFO_EXPRESSION(i % 3, "message %d", i);
  }
  EXPECT_EQ(g_log_calls, 4u);
  EXPECT_EQ(g_last_log_event.message, "message 5");
}

int g_counter = 0;

bool mod3()
{
  return (g_counter % 3) != 0;
}

TEST_F(TestLoggingMacros, test_logging_function) {
  for (int i : {1, 2, 3, 4, 5, 6}) {
    g_counter = i;
    RCL_LOG_INFO_FUNCTION(&mod3, "message %d", i);
  }
  EXPECT_EQ(g_log_calls, 4u);
  EXPECT_EQ(g_last_log_event.message, "message 5");
}

TEST_F(TestLoggingMacros, test_logging_skipfirst) {
  for (uint32_t i : {1, 2, 3, 4, 5}) {
    RCL_LOG_WARN_SKIPFIRST("message %u", i);
    EXPECT_EQ(g_log_calls, i - 1);
  }
}

TEST_F(TestLoggingMacros, test_logging_throttle) {
  for (int i : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}) {
    RCL_LOG_ERROR_THROTTLE(RCL_STEADY_TIME, 30 /* ms */, "throttled message %d", i)
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20ms);
  }
  EXPECT_EQ(g_log_calls, 5u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_ERROR);
  EXPECT_EQ(g_last_log_event.name, "");
  EXPECT_EQ(g_last_log_event.message, "throttled message 8");
}

TEST_F(TestLoggingMacros, test_logging_skipfirst_throttle) {
  for (int i : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}) {
    RCL_LOG_FATAL_SKIPFIRST_THROTTLE(RCL_STEADY_TIME, 30 /* ms */, "throttled message %d", i)
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20ms);
  }
  EXPECT_EQ(g_log_calls, 4u);
  EXPECT_EQ(g_last_log_event.level, RCL_LOG_SEVERITY_FATAL);
  EXPECT_EQ(g_last_log_event.name, "");
  EXPECT_EQ(g_last_log_event.message, "throttled message 8");
}
