// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include "../scope_exit.hpp"
#include "rcl/rcl.h"
#include "rcl/error_handling.h"
#include "rcl/wait.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif


#define TOLERANCE RCL_MS_TO_NS(6)

TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), test_resize_to_zero) {
  // Initialize a waitset with a subscription and then resize it to zero.
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_wait_set_resize_subscriptions(&wait_set, 0);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  EXPECT_EQ(wait_set.size_of_subscriptions, 0ull);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

// Test rcl_wait with a positive finite timeout value (1ms)
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), finite_timeout) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 0, 1, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  int64_t timeout = RCL_MS_TO_NS(10);  // nanoseconds
  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  ASSERT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  // Check time
  int64_t diff = std::chrono::duration_cast<std::chrono::nanoseconds>(after_sc - before_sc).count();
  EXPECT_LE(diff, timeout + TOLERANCE);

  ret = rcl_wait_set_fini(&wait_set);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

// Check that a timer overrides a negative timeout value (blocking forever)
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), negative_timeout) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 1, 1, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  // Add a dummy guard condition to avoid an error
  rcl_guard_condition_t guard_cond = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_cond, rcl_guard_condition_get_default_options());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_guard_condition(&wait_set, &guard_cond);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  ret = rcl_timer_init(&timer, RCL_MS_TO_NS(10), nullptr, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_timer(&wait_set, &timer);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  auto scope_exit = make_scope_exit([&guard_cond, &wait_set, &timer]() {
    rcl_ret_t ret = rcl_guard_condition_fini(&guard_cond);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_timer_fini(&timer);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });

  int64_t timeout = -1;
  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  // We expect a timeout here (timer value reached)
  ASSERT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  // Check time
  int64_t diff = std::chrono::duration_cast<std::chrono::nanoseconds>(after_sc - before_sc).count();
  EXPECT_LE(diff, RCL_MS_TO_NS(10) + TOLERANCE);
}

// Test rcl_wait with a timeout value of 0 (non-blocking)
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), zero_timeout) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 1, 1, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  // Add a dummy guard condition to avoid an error
  rcl_guard_condition_t guard_cond = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_cond, rcl_guard_condition_get_default_options());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_guard_condition(&wait_set, &guard_cond);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  auto scope_exit = make_scope_exit([&guard_cond, &wait_set]() {
    rcl_ret_t ret = rcl_guard_condition_fini(&guard_cond);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });

  // Time spent during wait should be negligible.
  int64_t timeout = 0;
  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  // We expect a timeout here (timer value reached)
  ASSERT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
  int64_t diff = std::chrono::duration_cast<std::chrono::nanoseconds>(after_sc - before_sc).count();
  EXPECT_LE(diff, TOLERANCE);
}

// Test rcl_wait_set_t with excess capacity works.
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), excess_capacity) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 42, 42, 42, 42, 42, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  int64_t timeout = 1;
  ret = rcl_wait(&wait_set, timeout);
  ASSERT_EQ(RCL_RET_TIMEOUT, ret) << rcl_get_error_string_safe();
}

// Check rcl_wait can be called in many threads, each with unique wait sets and resources.
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), multi_wait_set_threaded) {
  rcl_ret_t ret;
  const size_t number_of_threads = 20;  // concurrent waits
  const size_t count_target = 10;  // number of times each wait should wake up before being "done"
  const size_t retry_limit = 100;  // number of times to retry when a timeout occurs waiting
  const uint64_t wait_period = RCL_MS_TO_NS(100);  // timeout passed to rcl_wait each try
  const std::chrono::milliseconds trigger_period(2);  // period between each round of triggers
  struct TestSet
  {
    std::atomic<size_t> wake_count;
    rcl_wait_set_t wait_set;
    rcl_guard_condition_t guard_condition;
    std::thread thread;
    size_t thread_id;
  };
  std::vector<TestSet> test_sets(number_of_threads);
  // Setup common function for waiting on the triggered guard conditions.
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  auto wait_func_factory = [count_target, retry_limit, wait_period](TestSet & test_set)
  {
    return [&test_set, count_target, retry_limit, wait_period]() {
      while (test_set.wake_count < count_target) {
        bool change_detected = false;
        size_t wake_try_count = 0;
        while (wake_try_count < retry_limit) {
          wake_try_count++;
          rcl_ret_t ret;
          ret = rcl_wait_set_clear_guard_conditions(&test_set.wait_set);
          EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
          ret = rcl_wait_set_add_guard_condition(&test_set.wait_set, &test_set.guard_condition);
          EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
          ret = rcl_wait(&test_set.wait_set, wait_period);
          if (ret != RCL_RET_TIMEOUT) {
            ASSERT_EQ(ret, RCL_RET_OK);
            change_detected = true;
            // if not timeout, then the single guard condition should be set
            if (!test_set.wait_set.guard_conditions[0]) {
              test_set.wake_count.store(count_target + 1);  // indicates an error
              ASSERT_NE(nullptr, test_set.wait_set.guard_conditions[0])
                << "[thread " << test_set.thread_id
                << "] expected guard condition to be marked ready after non-timeout wake up";
            }
            // no need to wait again
            break;
          } else {
            std::stringstream ss;
            ss << "[thread " << test_set.thread_id << "] Timeout (try #" << wake_try_count << ")";
            printf("%s\n", ss.str().c_str());
          }
        }
        if (!change_detected) {
          test_set.wake_count.store(count_target + 1);  // indicates an error
          ASSERT_TRUE(change_detected);
        }
        test_set.wake_count++;
      }
    };
  };
  // *INDENT-ON*
  // Setup each test set.
  for (auto & test_set : test_sets) {
    rcl_ret_t ret;
    // init the wake count
    test_set.wake_count.store(0);
    // setup the guard condition
    test_set.guard_condition = rcl_get_zero_initialized_guard_condition();
    ret = rcl_guard_condition_init(
      &test_set.guard_condition, rcl_guard_condition_get_default_options());
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    // setup the wait set
    test_set.wait_set = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(&test_set.wait_set, 0, 1, 0, 0, 0, rcl_get_default_allocator());
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_add_guard_condition(&test_set.wait_set, &test_set.guard_condition);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    test_set.thread_id = 0;
  }
  // Setup safe tear-down.
  auto scope_exit = make_scope_exit([&test_sets]() {
    for (auto & test_set : test_sets) {
      rcl_ret_t ret = rcl_guard_condition_fini(&test_set.guard_condition);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      ret = rcl_wait_set_fini(&test_set.wait_set);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    }
  });
  // Now kick off all the threads.
  size_t thread_enumeration = 0;
  for (auto & test_set : test_sets) {
    thread_enumeration++;
    test_set.thread_id = thread_enumeration;
    test_set.thread = std::thread(wait_func_factory(test_set));
  }
  // Loop, triggering every trigger_period until the threads are done.
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  auto loop_test = [&test_sets, count_target]() -> bool {
    for (const auto & test_set : test_sets) {
      if (test_set.wake_count.load() < count_target) {
        return true;
      }
    }
    return false;
  };
  // *INDENT-ON*
  size_t loop_count = 0;
  while (loop_test()) {
    loop_count++;
    for (auto & test_set : test_sets) {
      ret = rcl_trigger_guard_condition(&test_set.guard_condition);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    }
    std::this_thread::sleep_for(trigger_period);
  }
  // Join the threads.
  for (auto & test_set : test_sets) {
    test_set.thread.join();
  }
  // Ensure the individual wake counts match the target (otherwise there was a problem).
  for (auto & test_set : test_sets) {
    ASSERT_EQ(count_target, test_set.wake_count.load());
  }
}

// Check the interaction of a guard condition and a negative timeout by
// triggering a guard condition in a separate thread
TEST(CLASSNAME(WaitSetTestFixture, RMW_IMPLEMENTATION), guard_condition) {
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 1, 0, 0, 0, rcl_get_default_allocator());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_guard_condition_t guard_cond = rcl_get_zero_initialized_guard_condition();
  ret = rcl_guard_condition_init(&guard_cond, rcl_guard_condition_get_default_options());
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_wait_set_add_guard_condition(&wait_set, &guard_cond);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  auto scope_exit = make_scope_exit([&wait_set, &guard_cond]() {
    rcl_ret_t ret = rcl_wait_set_fini(&wait_set);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_guard_condition_fini(&guard_cond);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  });

  std::promise<rcl_ret_t> p;

  int64_t timeout = -1;

  std::chrono::nanoseconds trigger_diff;
  std::thread trigger_thread([&p, &guard_cond, &trigger_diff]() {
    std::chrono::steady_clock::time_point before_trigger = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    rcl_ret_t ret = rcl_trigger_guard_condition(&guard_cond);
    std::chrono::steady_clock::time_point after_trigger = std::chrono::steady_clock::now();
    trigger_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
      after_trigger - before_trigger);
    p.set_value(ret);
  });
  auto f = p.get_future();

  std::chrono::steady_clock::time_point before_sc = std::chrono::steady_clock::now();
  ret = rcl_wait(&wait_set, timeout);
  std::chrono::steady_clock::time_point after_sc = std::chrono::steady_clock::now();
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  int64_t diff = std::chrono::duration_cast<std::chrono::nanoseconds>(after_sc - before_sc).count();
  trigger_thread.join();
  EXPECT_EQ(RCL_RET_OK, f.get());
  EXPECT_LE(std::abs(diff - trigger_diff.count()), TOLERANCE);
}
