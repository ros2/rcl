// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include <chrono>
#include <string>
#include <thread>

#include "rcl/subscription.h"

#include "rcl/rcl.h"

#include "rcl_interfaces/msg/log.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"
#include "rcutils/logging_macros.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestLoggingRosoutFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;
  rcl_subscription_t * subscription_ptr;
  void SetUp(int argc, const char* argv[], bool enable_node_option_rosout)
  {
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });
    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();

    ret = rcl_init(argc, argv, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    // create node
    rcl_node_options_t node_options = rcl_node_get_default_options();
    if (!enable_node_option_rosout) {
      node_options.enable_rosout = enable_node_option_rosout;
    }
    const char * name = "test_rcl_node_logging_rosout";
    const char * namespace_ = "/ns";
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    ret = rcl_node_init(this->node_ptr, name, namespace_, this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    // create rosout subscription
    const rosidl_message_type_support_t * ts =
      ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, Log);
    const char * topic = "rosout";
    this->subscription_ptr = new rcl_subscription_t;
    *this->subscription_ptr = rcl_get_zero_initialized_subscription();
    rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
    ret = rcl_subscription_init(this->subscription_ptr, this->node_ptr, ts, topic, &subscription_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret = rcl_subscription_fini(this->subscription_ptr, this->node_ptr);
    delete this->subscription_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown(this->context_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_context_fini(this->context_ptr);
    delete this->context_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};

void
wait_for_subscription_to_be_ready(
  rcl_subscription_t * subscription,
  rcl_context_t * context,
  size_t max_tries,
  int64_t period_ms,
  bool & success)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret =
    rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, 0, context, rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_wait_set_fini(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  size_t iteration = 0;
  do {
    ++iteration;
    ret = rcl_wait_set_clear(&wait_set);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait_set_add_subscription(&wait_set, subscription, NULL);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_wait(&wait_set, RCL_MS_TO_NS(period_ms));
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    for (size_t i = 0; i < wait_set.size_of_subscriptions; ++i) {
      if (wait_set.subscriptions[i] && wait_set.subscriptions[i] == subscription) {
        success = true;
        return;
      }
    }
  } while (iteration < max_tries);
  success = false;
}

/* Basic nominal test of having rosout logging globally enabled and locally enabled in a node
 */
TEST_F(CLASSNAME(TestLoggingRosoutFixture, RMW_IMPLEMENTATION), test_enable_global_rosout_enable_nodeoption) {
  SetUp(0, nullptr, true);

  // log
  RCUTILS_LOG_INFO_NAMED(rcl_node_get_logger_name(this->node_ptr), "SOMETHING");

  bool success;
  wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
}


/* Basic nominal test of having rosout logging globally enabled and locally disabled in a node
 */
TEST_F(CLASSNAME(TestLoggingRosoutFixture, RMW_IMPLEMENTATION), test_enable_global_rosout_disable_nodeoption) {
  SetUp(0, nullptr, false);

  // log
  RCUTILS_LOG_INFO_NAMED(rcl_node_get_logger_name(this->node_ptr), "SOMETHING");

  bool success;
  wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100, success);
  ASSERT_FALSE(success);
}

/* Basic nominal test of having rosout logging globally disabled and locally enabled in a node
 */
TEST_F(CLASSNAME(TestLoggingRosoutFixture, RMW_IMPLEMENTATION), test_disable_global_rosout_enable_nodeoption) {
  const char* argv[] = {"--ros-args", "--disable-rosout-logs"};
  SetUp(2, argv, true);

  // log
  RCUTILS_LOG_INFO_NAMED(rcl_node_get_logger_name(this->node_ptr), "SOMETHING");

  bool success;
  wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100, success);
  ASSERT_FALSE(success);
}

/* Basic nominal test of having rosout logging globally disabled and locally disabled in a node
 */
TEST_F(CLASSNAME(TestLoggingRosoutFixture, RMW_IMPLEMENTATION), test_disable_global_rosout_disable_nodeoption) {
  const char* argv[] = {"--ros-args", "--disable-rosout-logs"};
  SetUp(2, argv, false);

  // log
  RCUTILS_LOG_INFO_NAMED(rcl_node_get_logger_name(this->node_ptr), "SOMETHING");

  bool success;
  wait_for_subscription_to_be_ready(this->subscription_ptr, this->context_ptr, 10, 100, success);
  ASSERT_FALSE(success);
}
