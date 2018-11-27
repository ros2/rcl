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
#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

#include "rcl/rcl.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"

#include "rcutils/logging_macros.h"

#include "test_msgs/msg/primitives.h"
#include "test_msgs/srv/primitives.h"

#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestCountFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_node_t * old_node_ptr;
  rcl_node_t * node_ptr;
  rcl_wait_set_t * wait_set_ptr;
  void SetUp()
  {
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->old_node_ptr = new rcl_node_t;
    *this->old_node_ptr = rcl_get_zero_initialized_node();
    const char * old_name = "old_node_name";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->old_node_ptr, old_name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ret = rcl_shutdown();  // after this, the old_node_ptr should be invalid
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_graph_node";
    ret = rcl_node_init(this->node_ptr, name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    this->wait_set_ptr = new rcl_wait_set_t;
    *this->wait_set_ptr = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(this->wait_set_ptr, 0, 1, 0, 0, 0, rcl_get_default_allocator());
  }

  void TearDown()
  {
    rcl_ret_t ret;
    ret = rcl_node_fini(this->old_node_ptr);
    delete this->old_node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_wait_set_fini(this->wait_set_ptr);
    delete this->wait_set_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
};


TEST_F(CLASSNAME(TestCountFixture, RMW_IMPLEMENTATION), test_count_matched_functions) {
  std::string topic_name("/test_count_matched_functions__");
  rcl_ret_t ret;

  rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, Primitives);
  ret = rcl_publisher_init(&pub, this->node_ptr, ts, topic_name.c_str(), &pub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  {
    size_t subscription_count;
    ret = rcl_publisher_get_subscription_count(&pub, &subscription_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(0u, subscription_count);
  }

  rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub, this->node_ptr, ts, topic_name.c_str(), &sub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // This sleep is currently needed to allow opensplice and connext to correctly fire
  // the on_publication_matched/on_subscription_matched functions.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  {
    size_t subscription_count;
    ret = rcl_publisher_get_subscription_count(&pub, &subscription_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(1u, subscription_count);
  }

  {
    size_t publisher_count;
    ret = rcl_subscription_get_publisher_count(&sub, &publisher_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(1u, publisher_count);
  }

  rcl_subscription_t sub2 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub2_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub2, this->node_ptr, ts, topic_name.c_str(), &sub2_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // This sleep is currently needed to allow opensplice and connext to correctly fire
  // the on_publication_matched/on_subscription_matched functions.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  {
    size_t subscription_count;
    ret = rcl_publisher_get_subscription_count(&pub, &subscription_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(2u, subscription_count);
  }

  {
    size_t publisher_count;
    ret = rcl_subscription_get_publisher_count(&sub, &publisher_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(1u, publisher_count);
  }

  {
    size_t publisher_count;
    ret = rcl_subscription_get_publisher_count(&sub2, &publisher_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(1u, publisher_count);
  }

  ret = rcl_publisher_fini(&pub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcl_reset_error();

  // This sleep is currently needed to allow opensplice and connext to correctly fire
  // the on_publication_matched/on_subscription_matched functions.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  {
    size_t publisher_count;
    ret = rcl_subscription_get_publisher_count(&sub, &publisher_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(0u, publisher_count);
  }

  {
    size_t publisher_count;
    ret = rcl_subscription_get_publisher_count(&sub2, &publisher_count);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    rcl_reset_error();
    EXPECT_EQ(0u, publisher_count);
  }
}
