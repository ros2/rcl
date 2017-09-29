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

// Disable -Wmissing-field-initializers, as it is overly strict and will be
// relaxed in future versions of GCC (it is not a warning for clang).
// See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=36750#c13
#ifndef _WIN32
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <string>
#include <thread>

#include "rcl/rcl.h"
#include "rcl/graph.h"

#include "rcutils/logging_macros.h"

#include "std_msgs/msg/string.h"
#include "example_interfaces/srv/add_two_ints.h"

#include "../memory_tools/memory_tools.hpp"
#include "../scope_exit.hpp"
#include "rcl/error_handling.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

bool is_connext =
  std::string(rmw_get_implementation_identifier()).find("rmw_connext") == 0;

class CLASSNAME (TestGraphFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_node_t * old_node_ptr;
  rcl_node_t * node_ptr;
  rcl_wait_set_t * wait_set_ptr;
  void SetUp()
  {
    stop_memory_checking();
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->old_node_ptr = new rcl_node_t;
    *this->old_node_ptr = rcl_get_zero_initialized_node();
    const char * old_name = "old_node_name";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->old_node_ptr, old_name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_shutdown();  // after this, the old_node_ptr should be invalid
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "test_graph_node";
    ret = rcl_node_init(this->node_ptr, name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    this->wait_set_ptr = new rcl_wait_set_t;
    *this->wait_set_ptr = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(this->wait_set_ptr, 0, 1, 0, 0, 0, rcl_get_default_allocator());

    set_on_unexpected_malloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED MALLOC";});
    set_on_unexpected_realloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED REALLOC";});
    set_on_unexpected_free_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED FREE";});
    start_memory_checking();
  }

  void TearDown()
  {
    assert_no_malloc_end();
    assert_no_realloc_end();
    assert_no_free_end();
    stop_memory_checking();
    set_on_unexpected_malloc_callback(nullptr);
    set_on_unexpected_realloc_callback(nullptr);
    set_on_unexpected_free_callback(nullptr);
    rcl_ret_t ret;
    ret = rcl_node_fini(this->old_node_ptr);
    delete this->old_node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    ret = rcl_wait_set_fini(this->wait_set_ptr);
    delete this->wait_set_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    ret = rcl_node_fini(this->node_ptr);
    delete this->node_ptr;
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};

/* Test the rcl_get_topic_names_and_types and rcl_destroy_topic_names_and_types functions.
 *
 * This does not test content of the rcl_topic_names_and_types_t structure.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_get_and_destroy_topic_names_and_types
) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_names_and_types_t tnat {};
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  // invalid node
  ret = rcl_get_topic_names_and_types(nullptr, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_get_topic_names_and_types(&zero_node, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_get_topic_names_and_types(this->old_node_ptr, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // invalid allocator
  ret = rcl_get_topic_names_and_types(this->node_ptr, nullptr, false, &tnat);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // invalid topic_names_and_types
  ret = rcl_get_topic_names_and_types(this->node_ptr, &allocator, false, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // invalid argument to rcl_destroy_topic_names_and_types
  ret = rcl_names_and_types_fini(nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // valid calls
  ret = rcl_get_topic_names_and_types(this->node_ptr, &allocator, false, &tnat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_names_and_types_fini(&tnat);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
}

/* Test the rcl_count_publishers function.
 *
 * This does not test content the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_count_publishers
) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * topic_name = "/topic_test_rcl_count_publishers";
  size_t count;
  // invalid node
  ret = rcl_count_publishers(nullptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_count_publishers(&zero_node, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_count_publishers(this->old_node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // invalid topic name
  ret = rcl_count_publishers(this->node_ptr, nullptr, &count);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // TODO(wjwwood): test valid strings with invalid topic names in them
  // invalid count
  ret = rcl_count_publishers(this->node_ptr, topic_name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // valid call
  ret = rcl_count_publishers(this->node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}

/* Test the rcl_count_subscribers function.
 *
 * This does not test content the response.
 */
TEST_F(
  CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION),
  test_rcl_count_subscribers
) {
  stop_memory_checking();
  rcl_ret_t ret;
  rcl_node_t zero_node = rcl_get_zero_initialized_node();
  const char * topic_name = "/topic_test_rcl_count_subscribers";
  size_t count;
  // invalid node
  ret = rcl_count_subscribers(nullptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_count_subscribers(&zero_node, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  ret = rcl_count_subscribers(this->old_node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_NODE_INVALID, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // invalid topic name
  ret = rcl_count_subscribers(this->node_ptr, nullptr, &count);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // TODO(wjwwood): test valid strings with invalid topic names in them
  // invalid count
  ret = rcl_count_subscribers(this->node_ptr, topic_name, nullptr);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // valid call
  ret = rcl_count_subscribers(this->node_ptr, topic_name, &count);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
}

void
check_graph_state(
  const rcl_node_t * node_ptr,
  rcl_wait_set_t * wait_set_ptr,
  const rcl_guard_condition_t * graph_guard_condition,
  std::string & topic_name,
  size_t expected_publisher_count,
  size_t expected_subscriber_count,
  bool expected_in_tnat,
  size_t number_of_tries)
{
  RCUTILS_LOG_INFO(
    "Expecting %zu publishers, %zu subscribers, and that the topic is%s in the graph.",
    expected_publisher_count,
    expected_subscriber_count,
    expected_in_tnat ? "" : " not"
  )
  size_t publisher_count = 0;
  size_t subscriber_count = 0;
  bool is_in_tnat = false;
  rcl_names_and_types_t tnat {};
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  for (size_t i = 0; i < number_of_tries; ++i) {
    ret = rcl_count_publishers(node_ptr, topic_name.c_str(), &publisher_count);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    rcl_reset_error();

    ret = rcl_count_subscribers(node_ptr, topic_name.c_str(), &subscriber_count);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    rcl_reset_error();

    tnat = rcl_get_zero_initialized_names_and_types();
    ret = rcl_get_topic_names_and_types(node_ptr, &allocator, false, &tnat);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    rcl_reset_error();
    is_in_tnat = false;
    for (size_t i = 0; RCL_RET_OK == ret && i < tnat.names.size; ++i) {
      if (topic_name == std::string(tnat.names.data[i])) {
        ASSERT_FALSE(is_in_tnat) << "duplicates in the tnat";  // Found it more than once!
        is_in_tnat = true;
      }
    }
    if (RCL_RET_OK == ret) {
      ret = rcl_names_and_types_fini(&tnat);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      rcl_reset_error();
    }

    RCUTILS_LOG_INFO(
      " Try %zu: %zu publishers, %zu subscribers, and that the topic is%s in the graph.",
      i + 1,
      publisher_count,
      subscriber_count,
      is_in_tnat ? "" : " not"
    )
    if (
      expected_publisher_count == publisher_count &&
      expected_subscriber_count == subscriber_count &&
      expected_in_tnat == is_in_tnat)
    {
      RCUTILS_LOG_INFO("  state correct!")
      break;
    }
    // Wait for graph change before trying again.
    if ((i + 1) == number_of_tries) {
      // Don't wait for the graph to change on the last loop because we won't check again.
      continue;
    }
    ret = rcl_wait_set_clear_guard_conditions(wait_set_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_add_guard_condition(wait_set_ptr, graph_guard_condition);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(200);
    RCUTILS_LOG_INFO(
      "  state wrong, waiting up to '%s' nanoseconds for graph changes... ",
      std::to_string(time_to_sleep.count()).c_str())
    ret = rcl_wait(wait_set_ptr, time_to_sleep.count());
    if (ret == RCL_RET_TIMEOUT) {
      RCUTILS_LOG_INFO("timeout")
      continue;
    }
    RCUTILS_LOG_INFO("change occurred")
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
  EXPECT_EQ(expected_publisher_count, publisher_count);
  EXPECT_EQ(expected_subscriber_count, subscriber_count);
  if (expected_in_tnat) {
    EXPECT_TRUE(is_in_tnat);
  } else {
    EXPECT_FALSE(is_in_tnat);
  }
}

/* Test graph queries with a hand crafted graph.
 */
TEST_F(CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION), test_graph_query_functions) {
  stop_memory_checking();
  std::string topic_name("/test_graph_query_functions__");
  std::chrono::nanoseconds now = std::chrono::system_clock::now().time_since_epoch();
  topic_name += std::to_string(now.count());
  RCUTILS_LOG_INFO("Using topic name: %s", topic_name.c_str())
  rcl_ret_t ret;
  const rcl_guard_condition_t * graph_guard_condition =
    rcl_node_get_graph_guard_condition(this->node_ptr);
  // First assert the "topic_name" is not in use.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    0,  // expected publishers on topic
    0,  // expected subscribers on topic
    false,  // topic expected in graph
    9);  // number of retries
  // Now create a publisher on "topic_name" and check that it is seen.
  rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
  auto ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ret = rcl_publisher_init(&pub, this->node_ptr, ts, topic_name.c_str(), &pub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // Check the graph.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    1,  // expected publishers on topic
    0,  // expected subscribers on topic
    true,  // topic expected in graph
    9);  // number of retries
  // Now create a subscriber.
  rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&sub, this->node_ptr, ts, topic_name.c_str(), &sub_ops);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // Check the graph again.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    1,  // expected publishers on topic
    1,  // expected subscribers on topic
    true,  // topic expected in graph
    9);  // number of retries
  // Destroy the publisher.
  ret = rcl_publisher_fini(&pub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // Check the graph again.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    0,  // expected publishers on topic
    1,  // expected subscribers on topic
    true,  // topic expected in graph
    9);  // number of retries
  // Destroy the subscriber.
  ret = rcl_subscription_fini(&sub, this->node_ptr);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  rcl_reset_error();
  // Check the graph again.
  check_graph_state(
    this->node_ptr,
    this->wait_set_ptr,
    graph_guard_condition,
    topic_name,
    0,  // expected publishers on topic
    0,  // expected subscribers on topic
    false,  // topic expected in graph
    9);  // number of retries
}

/* Test the graph guard condition notices topic changes.
 *
 * Note: this test could be impacted by other communications on the same ROS Domain.
 */
TEST_F(CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION), test_graph_guard_condition_topics) {
  stop_memory_checking();
  rcl_ret_t ret;
  // Create a thread to sleep for a time, then create a publisher, sleep more, then a subscriber,
  // sleep more, destroy the subscriber, sleep more, and then destroy the publisher.
  std::promise<bool> topic_changes_promise;
  std::thread topic_thread(
    [this, &topic_changes_promise]() {
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      // create the publisher
      rcl_publisher_t pub = rcl_get_zero_initialized_publisher();
      rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
      rcl_ret_t ret = rcl_publisher_init(
        &pub, this->node_ptr, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "/chatter_test_graph_guard_condition_topics", &pub_ops);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      // create the subscription
      rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
      rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
      ret = rcl_subscription_init(
        &sub, this->node_ptr, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "/chatter_test_graph_guard_condition_topics", &sub_ops);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      // destroy the subscription
      ret = rcl_subscription_fini(&sub, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      // sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      // destroy the publication
      ret = rcl_publisher_fini(&pub, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      // notify that the thread is done
      topic_changes_promise.set_value(true);
    });
  // Wait for the graph state to change, expecting it to do so at least 4 times,
  // once for each change in the topics thread.
  const rcl_guard_condition_t * graph_guard_condition =
    rcl_node_get_graph_guard_condition(this->node_ptr);
  ASSERT_NE(nullptr, graph_guard_condition) << rcl_get_error_string_safe();
  std::shared_future<bool> future = topic_changes_promise.get_future();
  size_t graph_changes_count = 0;
  // while the topic thread is not done, wait and count the graph changes
  while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    ret = rcl_wait_set_clear_guard_conditions(this->wait_set_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    ret = rcl_wait_set_add_guard_condition(this->wait_set_ptr, graph_guard_condition);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(200);
    RCUTILS_LOG_INFO(
      "waiting up to '%s' nanoseconds for graph changes",
      std::to_string(time_to_sleep.count()).c_str())
    ret = rcl_wait(this->wait_set_ptr, time_to_sleep.count());
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    graph_changes_count++;
  }
  topic_thread.join();
  // expect at least 4 changes
  ASSERT_GE(graph_changes_count, 4ul);
}

/* Test the rcl_service_server_is_available function.
 */
TEST_F(CLASSNAME(TestGraphFixture, RMW_IMPLEMENTATION), test_rcl_service_server_is_available) {
  stop_memory_checking();
  rcl_ret_t ret;
  // First create a client which will be used to call the function.
  rcl_client_t client = rcl_get_zero_initialized_client();
  auto ts = ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, AddTwoInts);
  const char * service_name = "/service_test_rcl_service_server_is_available";
  rcl_client_options_t client_options = rcl_client_get_default_options();
  ret = rcl_client_init(&client, this->node_ptr, ts, service_name, &client_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  auto client_exit = make_scope_exit(
    [&client, this]() {
      stop_memory_checking();
      rcl_ret_t ret = rcl_client_fini(&client, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    });
  // Check, knowing there is no service server (created by us at least).
  bool is_available;
  ret = rcl_service_server_is_available(this->node_ptr, &client, &is_available);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ASSERT_FALSE(is_available);
  // Setup function to wait for service state to change using graph guard condition.
  const rcl_guard_condition_t * graph_guard_condition =
    rcl_node_get_graph_guard_condition(this->node_ptr);
  ASSERT_NE(nullptr, graph_guard_condition) << rcl_get_error_string_safe();
  auto wait_for_service_state_to_change = [this, &graph_guard_condition, &client](
    bool expected_state,
    bool & is_available)
    {
      is_available = false;
      auto start = std::chrono::steady_clock::now();
      auto end = start + std::chrono::seconds(10);
      while (std::chrono::steady_clock::now() < end) {
        // We wait multiple times in case other graph changes are occurring simultaneously.
        std::chrono::nanoseconds time_left = end - start;
        std::chrono::nanoseconds time_to_sleep = time_left;
        std::chrono::seconds min_sleep(1);
        if (time_to_sleep > min_sleep) {
          time_to_sleep = min_sleep;
        }
        rcl_ret_t ret = rcl_wait_set_clear_guard_conditions(this->wait_set_ptr);
        ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
        ret = rcl_wait_set_add_guard_condition(this->wait_set_ptr, graph_guard_condition);
        ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
        RCUTILS_LOG_INFO(
          "waiting up to '%s' nanoseconds for graph changes",
          std::to_string(time_to_sleep.count()).c_str())
        ret = rcl_wait(this->wait_set_ptr, time_to_sleep.count());
        if (ret == RCL_RET_TIMEOUT) {
          if (!is_connext) {
            // TODO(wjwwood):
            //   Connext has a race condition which can cause the graph guard
            //   condition to wake up due to the necessary topics going away,
            //   but afterwards rcl_service_server_is_available() still does
            //   not reflect that the service is "no longer available".
            //   The result is that some tests are flaky unless you not only
            //   check right after a graph change but again in the future where
            //   rcl_service_server_is_available() eventually reports the
            //   service is no longer there. This condition can be removed and
            //   we can always continue when we get RCL_RET_TIMEOUT once that
            //   is fixed.
            continue;
          }
        } else {
          ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
        }
        ret = rcl_service_server_is_available(this->node_ptr, &client, &is_available);
        ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
        if (is_available == expected_state) {
          break;
        }
      }
    };
  {
    // Create the service server.
    rcl_service_t service = rcl_get_zero_initialized_service();
    rcl_service_options_t service_options = rcl_service_get_default_options();
    ret = rcl_service_init(&service, this->node_ptr, ts, service_name, &service_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    auto service_exit = make_scope_exit(
      [&service, this]() {
        stop_memory_checking();
        rcl_ret_t ret = rcl_service_fini(&service, this->node_ptr);
        EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      });
    // Wait for and then assert that it is available.
    wait_for_service_state_to_change(true, is_available);
    ASSERT_TRUE(is_available);
  }
  // Assert the state goes back to "not available" after the service is removed.
  wait_for_service_state_to_change(false, is_available);
  ASSERT_FALSE(is_available);
}
