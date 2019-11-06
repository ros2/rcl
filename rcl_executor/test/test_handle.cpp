// Copyright (c) 2019 - for information on the respective copyright owner
// see the NOTICE file and/or the repository https://github.com/micro-ROS/rcl_executor.
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

// #include <std_msgs/msg/int32.h>
// #include <std_msgs/msg/string.h>
// #include <geometry_msgs/msg/twist.h>
#include <gtest/gtest.h>
#include "rcl_executor/handle.h"
#include "osrf_testing_tools_cpp/scope_exit.hpp"

class TestDefaultHandle : public ::testing::Test
{
public:
  void SetUp()
  {
  }

  void TearDown()
  {
  }
};

/*
 * Test suite
 */
TEST_F(TestDefaultHandle, handle_size_zero_init) {
  rcl_ret_t rc;

  rcle_handle_size_t info;
  size_t zero = 0;
  rc = rcle_handle_size_zero_init(&info);
  EXPECT_EQ(rc, RCL_RET_OK);
  EXPECT_EQ(info.number_of_clients, zero);
  EXPECT_EQ(info.number_of_guard_conditions, zero);
  EXPECT_EQ(info.number_of_services, zero);
  EXPECT_EQ(info.number_of_subscriptions, zero);
  EXPECT_EQ(info.number_of_timers, zero);
  EXPECT_EQ(info.number_of_events, zero);

  // test null pointer
  rc = rcle_handle_size_zero_init(NULL);
  EXPECT_EQ(rc, RCL_RET_INVALID_ARGUMENT);
  rcutils_reset_error();
}

TEST_F(TestDefaultHandle, handle_init) {
  rcl_ret_t rc;

  rcle_handle_t handle;
  size_t max_handles = 10;
  rc = rcle_handle_init(&handle, max_handles);
  EXPECT_EQ(rc, RCL_RET_OK);
  EXPECT_EQ(handle.type, NONE);
  EXPECT_EQ(handle.invocation, ON_NEW_DATA);
  // EXPECT_EQ(handle.subscription, NULL);
  // EXPECT_EQ(handle.timer, NULL);
  // EXPECT_EQ(handle.data, NULL);
  // EXPECT_EQ(handle.callback, NULL);
  EXPECT_EQ(handle.index, max_handles);
  EXPECT_EQ(handle.initialized, false);
  EXPECT_EQ(handle.data_available, false);

  // test null pointer
  rc = rcle_handle_init(NULL, max_handles);
  EXPECT_EQ(rc, RCL_RET_INVALID_ARGUMENT);
  rcutils_reset_error();
}

TEST_F(TestDefaultHandle, handle_clear) {
  rcl_ret_t rc;

  rcle_handle_t handle;
  size_t max_handles = 10;  // assumption: max_handles > 1
  rc = rcle_handle_init(&handle, max_handles);

  // setup dummy handle
  handle.initialized = true;
  handle.index = 0;

  rc = rcle_handle_clear(&handle, max_handles - 1);
  EXPECT_EQ(rc, RCL_RET_OK);
  EXPECT_EQ(handle.index, max_handles - 1);
  EXPECT_EQ(handle.initialized, false);

  // test null pointer
  rc = rcle_handle_clear(NULL, max_handles);
  EXPECT_EQ(rc, RCL_RET_INVALID_ARGUMENT);
  rcutils_reset_error();
}

TEST_F(TestDefaultHandle, handle_print) {
  rcl_ret_t rc;

  rcle_handle_t handle;
  size_t max_handles = 10;
  rc = rcle_handle_init(&handle, max_handles);

  rc = rcle_handle_print(&handle);
  EXPECT_EQ(rc, RCL_RET_OK);

  // test null pointer
  rc = rcle_handle_print(NULL);
  EXPECT_EQ(rc, RCL_RET_INVALID_ARGUMENT);
  rcutils_reset_error();
}
