// Copyright 2026 Open Source Robotics Foundation, Inc.
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

#include "rcl/error_handling.h"
#include "rcl/subscription.h"


TEST(TestSubscriptionOptionsBufferBackends, set_null_options) {
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_options_set_acceptable_buffer_backends("cpu", nullptr)
  );
  rcl_reset_error();
}

TEST(TestSubscriptionOptionsBufferBackends, set_and_verify) {
  rcl_subscription_options_t options = rcl_subscription_get_default_options();

  // Default should be NULL
  EXPECT_EQ(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);

  // Set a single backend
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends("cuda", &options)
  );
  ASSERT_NE(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);
  EXPECT_STREQ(options.rmw_subscription_options.acceptable_buffer_backends, "cuda");

  // Overwrite with a comma-separated list
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends("cuda,demo", &options)
  );
  ASSERT_NE(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);
  EXPECT_STREQ(options.rmw_subscription_options.acceptable_buffer_backends, "cuda,demo");

  // Set to "any"
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends("any", &options)
  );
  ASSERT_NE(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);
  EXPECT_STREQ(options.rmw_subscription_options.acceptable_buffer_backends, "any");

  EXPECT_EQ(RCL_RET_OK, rcl_subscription_options_fini(&options));
  EXPECT_EQ(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);
}

TEST(TestSubscriptionOptionsBufferBackends, set_null_clears) {
  rcl_subscription_options_t options = rcl_subscription_get_default_options();

  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends("demo", &options)
  );
  ASSERT_NE(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);

  // Setting NULL should clear
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends(NULL, &options)
  );
  EXPECT_EQ(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);

  EXPECT_EQ(RCL_RET_OK, rcl_subscription_options_fini(&options));
}

TEST(TestSubscriptionOptionsBufferBackends, set_empty_string_clears) {
  rcl_subscription_options_t options = rcl_subscription_get_default_options();

  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends("demo", &options)
  );
  ASSERT_NE(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);

  // Setting empty string should clear
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_set_acceptable_buffer_backends("", &options)
  );
  EXPECT_EQ(options.rmw_subscription_options.acceptable_buffer_backends, nullptr);

  EXPECT_EQ(RCL_RET_OK, rcl_subscription_options_fini(&options));
}

TEST(TestSubscriptionOptionsBufferBackends, fini_with_no_backends_set) {
  rcl_subscription_options_t options = rcl_subscription_get_default_options();

  // fini on default options (NULL backends) should succeed
  EXPECT_EQ(RCL_RET_OK, rcl_subscription_options_fini(&options));
}
