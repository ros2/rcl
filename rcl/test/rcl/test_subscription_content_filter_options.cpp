// Copyright 2021 Open Source Robotics Foundation, Inc.
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

TEST(TestSubscriptionContentFilterOptions, subscription_options_failure) {
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();

  const char * filter_expression1 = "filter=1";
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_options_set_content_filter_options(
      nullptr, 0, nullptr, nullptr)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_options_set_content_filter_options(
      filter_expression1, 0, nullptr, nullptr)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_options_set_content_filter_options(
      filter_expression1, 1, nullptr, &subscription_options)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_options_fini(
      nullptr)
  );
  rcl_reset_error();
}

TEST(TestSubscriptionContentFilterOptions, subscription_options_success)
{
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();

  const char * filter_expression1 = "filter=1";

  {
    EXPECT_EQ(
      RCL_RET_OK,
      rcl_subscription_options_set_content_filter_options(
        filter_expression1, 0, nullptr, &subscription_options)
    );

    rmw_subscription_content_filter_options_t * content_filter_options =
      subscription_options.rmw_subscription_options.content_filter_options;
    ASSERT_NE(nullptr, content_filter_options);
    EXPECT_STREQ(filter_expression1, content_filter_options->filter_expression);
    EXPECT_EQ(0u, content_filter_options->expression_parameters.size);
    EXPECT_EQ(nullptr, content_filter_options->expression_parameters.data);
  }

  const char * filter_expression2 = "(filter1=%0 OR filter1=%1) AND filter2=%2";
  const char * expression_parameters2[] = {
    "1", "2", "3",
  };
  size_t expression_parameters_count2 = sizeof(expression_parameters2) / sizeof(char *);

  {
    EXPECT_EQ(
      RCL_RET_OK,
      rcl_subscription_options_set_content_filter_options(
        filter_expression2, expression_parameters_count2,
        expression_parameters2, &subscription_options)
    );

    rmw_subscription_content_filter_options_t * content_filter_options =
      subscription_options.rmw_subscription_options.content_filter_options;
    ASSERT_NE(nullptr, content_filter_options);
    EXPECT_STREQ(filter_expression2, content_filter_options->filter_expression);
    ASSERT_EQ(
      expression_parameters_count2,
      content_filter_options->expression_parameters.size);
    for (size_t i = 0; i < expression_parameters_count2; ++i) {
      EXPECT_STREQ(
        content_filter_options->expression_parameters.data[i],
        expression_parameters2[i]);
    }
  }

  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_options_fini(&subscription_options)
  );
}

TEST(TestSubscriptionContentFilterOptions, content_filter_options_failure) {
  rcl_subscription_content_filter_options_t content_filter_options =
    rcl_get_zero_initialized_subscription_content_filter_options();

  const char * filter_expression1 = "filter=1";
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_init(
      nullptr, 0, nullptr, nullptr)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_init(
      filter_expression1, 0, nullptr, nullptr)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_init(
      filter_expression1, 1, nullptr, &content_filter_options)
  );
  rcl_reset_error();

  // set
  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_set(
      nullptr, 0, nullptr, nullptr)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_set(
      filter_expression1, 0, nullptr, nullptr)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_set(
      filter_expression1, 1, nullptr, &content_filter_options)
  );
  rcl_reset_error();

  EXPECT_EQ(
    RCL_RET_INVALID_ARGUMENT,
    rcl_subscription_content_filter_options_fini(
      nullptr)
  );
  rcl_reset_error();
}

TEST(TestSubscriptionContentFilterOptions, content_filter_options_success)
{
  rmw_subscription_content_filter_options_t * content_filter_options;
  const char * filter_expression1 = "filter=1";
  const char * filter_expression1_update = "filter=2";

  rcl_subscription_content_filter_options_t subscription_content_filter_options =
    rcl_get_zero_initialized_subscription_content_filter_options();
  {
    // init with filter_expression1
    EXPECT_EQ(
      RCL_RET_OK,
      rcl_subscription_content_filter_options_init(
        filter_expression1, 0, nullptr, &subscription_content_filter_options)
    );

    content_filter_options =
      &subscription_content_filter_options.rmw_subscription_content_filter_options;
    EXPECT_STREQ(filter_expression1, content_filter_options->filter_expression);
    EXPECT_EQ(0u, content_filter_options->expression_parameters.size);
    EXPECT_EQ(nullptr, content_filter_options->expression_parameters.data);

    // set with filter_expression1_update
    EXPECT_EQ(
      RCL_RET_OK,
      rcl_subscription_content_filter_options_set(
        filter_expression1_update, 0, nullptr, &subscription_content_filter_options)
    );

    content_filter_options =
      &subscription_content_filter_options.rmw_subscription_content_filter_options;
    EXPECT_STREQ(filter_expression1_update, content_filter_options->filter_expression);
    EXPECT_EQ(0u, content_filter_options->expression_parameters.size);
    EXPECT_EQ(nullptr, content_filter_options->expression_parameters.data);
  }

  const char * filter_expression2 = "(filter1=%0 OR filter1=%1) AND filter2=%2";
  const char * expression_parameters2[] = {
    "1", "2", "3",
  };
  size_t expression_parameters_count2 = sizeof(expression_parameters2) / sizeof(char *);

  const char * filter_expression2_update = "(filter1=%0 AND filter1=%1) OR filter2=%2";
  const char * expression_parameters2_update[] = {
    "11", "22", "33",
  };
  size_t expression_parameters_count2_update = sizeof(expression_parameters2) / sizeof(char *);

  rcl_subscription_content_filter_options_t subscription_content_filter_options2 =
    rcl_get_zero_initialized_subscription_content_filter_options();
  {
    // init with filter_expression2 and expression_parameters2
    EXPECT_EQ(
      RCL_RET_OK,
      rcl_subscription_content_filter_options_init(
        filter_expression2, expression_parameters_count2,
        expression_parameters2, &subscription_content_filter_options2)
    );

    content_filter_options =
      &subscription_content_filter_options2.rmw_subscription_content_filter_options;
    ASSERT_NE(nullptr, content_filter_options);
    EXPECT_STREQ(filter_expression2, content_filter_options->filter_expression);
    ASSERT_EQ(
      expression_parameters_count2,
      content_filter_options->expression_parameters.size);
    for (size_t i = 0; i < expression_parameters_count2; ++i) {
      EXPECT_STREQ(
        content_filter_options->expression_parameters.data[i],
        expression_parameters2[i]);
    }

    // set with filter_expression2_update and expression_parameters2_update
    EXPECT_EQ(
      RCL_RET_OK,
      rcl_subscription_content_filter_options_set(
        filter_expression2_update, expression_parameters_count2_update,
        expression_parameters2_update, &subscription_content_filter_options2)
    );

    content_filter_options =
      &subscription_content_filter_options2.rmw_subscription_content_filter_options;
    ASSERT_NE(nullptr, content_filter_options);
    EXPECT_STREQ(filter_expression2_update, content_filter_options->filter_expression);
    ASSERT_EQ(
      expression_parameters_count2_update,
      content_filter_options->expression_parameters.size);
    for (size_t i = 0; i < expression_parameters_count2_update; ++i) {
      EXPECT_STREQ(
        content_filter_options->expression_parameters.data[i],
        expression_parameters2_update[i]);
    }
  }

  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_content_filter_options_fini(
      &subscription_content_filter_options)
  );
  EXPECT_EQ(
    RCL_RET_OK,
    rcl_subscription_content_filter_options_fini(
      &subscription_content_filter_options2)
  );
}
