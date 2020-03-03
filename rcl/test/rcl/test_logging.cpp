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

#include <gtest/gtest.h>

#include "rcl/logging.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestLoggingFixture, RMW_IMPLEMENTATION) : public ::testing::Test {};

#define CONTAINS_LOGGING_ARGUMENTS(argv) \
  rcl_contains_logging_arguments(sizeof(argv) / sizeof(const char *), argv)

TEST_F(CLASSNAME(TestLoggingFixture, RMW_IMPLEMENTATION), contains_logging_arguments) {
  const char * with_logging_arguments[] = {
    "--log-level", "debug", "--log-config-file", "asd.config"
  };
  const char * with_one_logging_argument[] = {"--enable-stdout-logs", "bsd"};
  const char * with_mixed_arguments[] = {
    "asd", "--log-level", "debug", "bsd", "--log-config-file", "asd.config"
  };
  const char * with_other_argument[] = {"--asd"};
  const char * with_other_arguments[] = {"--asd", "bsd"};

  EXPECT_TRUE(CONTAINS_LOGGING_ARGUMENTS(with_logging_arguments));
  EXPECT_TRUE(CONTAINS_LOGGING_ARGUMENTS(with_one_logging_argument));
  EXPECT_TRUE(CONTAINS_LOGGING_ARGUMENTS(with_mixed_arguments));
  EXPECT_FALSE(CONTAINS_LOGGING_ARGUMENTS(with_other_argument));
  EXPECT_FALSE(CONTAINS_LOGGING_ARGUMENTS(with_other_arguments));
}
