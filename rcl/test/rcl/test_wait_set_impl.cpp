// Copyright 2016 Open Source Robotics Foundation, Inc.
// Copyright 2020 Robert Bosch GmbH
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

#include <algorithm>  // for std::max
#include <atomic>
#include <chrono>
#include <future>
#include <sstream>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/wait.h"

#include "rcutils/logging_macros.h"

#include "../../src/rcl/wait_set_impl.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

#define TOLERANCE RCL_MS_TO_NS(6)

class CLASSNAME (WaitSetImplTestFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_context_t * context_ptr;
  void SetUp()
  {
    rcl_ret_t ret;
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
    });
    this->context_ptr = new rcl_context_t;
    *this->context_ptr = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    EXPECT_EQ(RCL_RET_OK, rcl_shutdown(this->context_ptr)) << rcl_get_error_string().str;
    EXPECT_EQ(RCL_RET_OK, rcl_context_fini(this->context_ptr)) << rcl_get_error_string().str;
    delete this->context_ptr;
  }
};

TEST_F(CLASSNAME(WaitSetImplTestFixture, RMW_IMPLEMENTATION), test_resize_to_zero) {
  // Initialize a wait set with a subscription and then resize it to zero.
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret =
    rcl_wait_set_init(&wait_set, 1, 1, 1, 1, 1, 0, context_ptr, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  ret = rcl_wait_set_resize(&wait_set, 0u, 0u, 0u, 0u, 0u, 0u);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  EXPECT_EQ(wait_set.size_of_subscriptions, 0ull);
  EXPECT_EQ(wait_set.size_of_guard_conditions, 0ull);
  EXPECT_EQ(wait_set.size_of_clients, 0ull);
  EXPECT_EQ(wait_set.size_of_services, 0ull);
  EXPECT_EQ(wait_set.size_of_timers, 0ull);

  // check that arrays are gone
  EXPECT_EQ(wait_set.subscriptions, nullptr);
  EXPECT_EQ(wait_set.guard_conditions, nullptr);
  EXPECT_EQ(wait_set.timers, nullptr);
  EXPECT_EQ(wait_set.clients, nullptr);
  EXPECT_EQ(wait_set.services, nullptr);
  EXPECT_EQ(wait_set.events, nullptr);

  // check that the timestamp arrays are gone as well
  EXPECT_EQ(wait_set.subscriptions_timestamps, nullptr);
  EXPECT_EQ(wait_set.timers_timestamps, nullptr);
  EXPECT_EQ(wait_set.clients_timestamps, nullptr);
  EXPECT_EQ(wait_set.services_timestamps, nullptr);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(CLASSNAME(WaitSetImplTestFixture, RMW_IMPLEMENTATION), test_init) {
  // zero-initialized wait set should have zero size ;-)
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  EXPECT_EQ(wait_set.size_of_subscriptions, 0ull);
  EXPECT_EQ(wait_set.size_of_guard_conditions, 0ull);
  EXPECT_EQ(wait_set.size_of_clients, 0ull);
  EXPECT_EQ(wait_set.size_of_services, 0ull);
  EXPECT_EQ(wait_set.size_of_timers, 0ull);

  // now lets get some fields
  rcl_ret_t ret =
    rcl_wait_set_init(&wait_set, 1, 1, 1, 1, 1, 1, context_ptr, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_TRUE(rcl_wait_set_is_valid(&wait_set));
  EXPECT_EQ(wait_set.size_of_subscriptions, 1ull);
  EXPECT_EQ(wait_set.size_of_guard_conditions, 1ull);
  EXPECT_EQ(wait_set.size_of_clients, 1ull);
  EXPECT_EQ(wait_set.size_of_services, 1ull);
  EXPECT_EQ(wait_set.size_of_timers, 1ull);
  EXPECT_EQ(wait_set.size_of_events, 1ull);

  // check that we have arrays for return values
  EXPECT_NE(wait_set.subscriptions, nullptr);
  EXPECT_NE(wait_set.guard_conditions, nullptr);
  EXPECT_NE(wait_set.timers, nullptr);
  EXPECT_NE(wait_set.clients, nullptr);
  EXPECT_NE(wait_set.services, nullptr);
  EXPECT_NE(wait_set.events, nullptr);

  // check that we have timestamp arrays as well
  EXPECT_NE(wait_set.subscriptions_timestamps, nullptr);
  EXPECT_NE(wait_set.timers_timestamps, nullptr);
  EXPECT_NE(wait_set.clients_timestamps, nullptr);
  EXPECT_NE(wait_set.services_timestamps, nullptr);

  // look into the implementation
  EXPECT_NE(wait_set.impl->rmw_subscriptions.subscribers, nullptr);
  EXPECT_NE(wait_set.impl->rmw_guard_conditions.guard_conditions, nullptr);
  EXPECT_NE(wait_set.impl->rmw_clients.clients, nullptr);
  EXPECT_NE(wait_set.impl->rmw_services.services, nullptr);
  EXPECT_NE(wait_set.impl->rmw_events.events, nullptr);

  // and now for the timestamps in the implementation
  EXPECT_NE(wait_set.impl->rmw_subscriptions.timestamps, nullptr);
  EXPECT_NE(wait_set.impl->rmw_clients.timestamps, nullptr);
  EXPECT_NE(wait_set.impl->rmw_services.timestamps, nullptr);

  // finalized wait set is invalid
  ret = rcl_wait_set_fini(&wait_set);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  EXPECT_FALSE(rcl_wait_set_is_valid(&wait_set));
}
