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

#include "rcl/qos.h"
#include "rmw/types.h"

// Converts to string and back to the policy value, check that it's the same
#define TEST_QOS_POLICY_VALUE_STRINGIFY(kind, value) \
  do { \
    EXPECT_EQ( \
      value, rcl_qos_ ## kind ## _policy_from_str(rcl_qos_ ## kind ## _policy_to_str(value))); \
  } while (0)

// Check unrecognized string as input of from_str, check UNKNOWN as input of to_str.
#define TEST_QOS_POLICY_STRINGIFY_CORNER_CASES(kind, kind_upper) \
  do { \
    EXPECT_EQ(NULL, rcl_qos_ ## kind ## _policy_to_str(RMW_QOS_POLICY_ ## kind_upper ## _UNKNOWN)); \
    EXPECT_EQ( \
      RMW_QOS_POLICY_ ## kind_upper ## _UNKNOWN, \
      rcl_qos_ ## kind ## _policy_from_str("this could never be a stringified policy value")); \
  } while (0)

TEST(test_qos_policy_stringify, test_normal_use) {
  TEST_QOS_POLICY_VALUE_STRINGIFY(durability, RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT);
  TEST_QOS_POLICY_VALUE_STRINGIFY(durability, RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
  TEST_QOS_POLICY_VALUE_STRINGIFY(durability, RMW_QOS_POLICY_DURABILITY_VOLATILE);
  TEST_QOS_POLICY_VALUE_STRINGIFY(history, RMW_QOS_POLICY_HISTORY_KEEP_LAST);
  TEST_QOS_POLICY_VALUE_STRINGIFY(history, RMW_QOS_POLICY_HISTORY_KEEP_ALL);
  TEST_QOS_POLICY_VALUE_STRINGIFY(history, RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT);
  TEST_QOS_POLICY_VALUE_STRINGIFY(liveliness, RMW_QOS_POLICY_LIVELINESS_AUTOMATIC);
  TEST_QOS_POLICY_VALUE_STRINGIFY(liveliness, RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC);
  TEST_QOS_POLICY_VALUE_STRINGIFY(liveliness, RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT);
  TEST_QOS_POLICY_VALUE_STRINGIFY(reliability, RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
  TEST_QOS_POLICY_VALUE_STRINGIFY(reliability, RMW_QOS_POLICY_RELIABILITY_RELIABLE);
  TEST_QOS_POLICY_VALUE_STRINGIFY(reliability, RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT);

  TEST_QOS_POLICY_STRINGIFY_CORNER_CASES(durability, DURABILITY);
  TEST_QOS_POLICY_STRINGIFY_CORNER_CASES(history, HISTORY);
  TEST_QOS_POLICY_STRINGIFY_CORNER_CASES(liveliness, LIVELINESS);
  TEST_QOS_POLICY_STRINGIFY_CORNER_CASES(reliability, RELIABILITY);
}
