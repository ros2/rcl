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

#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>
#include <geometry_msgs/msg/twist.h>
#include <gtest/gtest.h>
#include <vector>
#include <chrono>
#include <thread>

#include "rcl_executor/let_executor.h"
#include "osrf_testing_tools_cpp/scope_exit.hpp"


// 27.06.2019, unit test adapted from ros2/rcl/rcl_lifecycle/test/test_default_state_machine.cpp
// by Jan Staschulat, under Apache 2.0 License

/******************************* CALLBACKS for subscriptions ************************/

// some variables for testing the let-scheduler
// array holds the msg_id in the order as they are received
// the callback calls the function _results_add(msg_id)
// at the end the array contains the order of received events.
static const unsigned int kMax = 3;
static const unsigned int MSG_MAX = 3 * kMax;  // (#publishers * #published messages)
static unsigned int _executor_results[MSG_MAX];
static unsigned int _executor_results_i;

// callback for topic "chatter1"
static unsigned int _cb1_cnt = 0;
// callback for topic "chatter2"
static unsigned int _cb2_cnt = 0;
// callback for topic "chatter3"
static unsigned int _cb3_cnt = 0;

// number of function calls
static unsigned int _fn_cnt = 0;

static
void
_results_callback_counters_init()
{
  _cb1_cnt = 0;
  _cb2_cnt = 0;
  _cb3_cnt = 0;
}

static
unsigned int
_results_callback_num_received()
{
  return _cb1_cnt + _cb2_cnt + _cb3_cnt;
}

static
void
_executor_results_init(void)
{
  for (unsigned int i = 0; i < MSG_MAX; i++) {
    _executor_results[i] = 0;
  }
  _executor_results_i = 0;

  _results_callback_counters_init();
}

/// assumption msg_id > 0
void
_executor_results_add(unsigned int msg_id)
{
  if (_executor_results_i < MSG_MAX) {
    _executor_results[_executor_results_i] = msg_id;
    _executor_results_i++;
  } else {
    printf("_executor_results_add: buffer overflow!\n");
  }
}

bool
_executor_results_all_msg_received()
{
  // total number of expected messages is MSG_MAX
  return _results_callback_num_received() == MSG_MAX;
}
void
_executor_results_print(void)
{
  printf("Results: ");
  for (unsigned int i = 0; i < MSG_MAX; i++) {
    // if the value is zero, then it was not used => finished
    // assume positive message_id's
    if (_executor_results[i] > 0) {
      printf("%d ", _executor_results[i]);
    } else {
      break;
    }
  }
  printf("\n");
}

void
_executor_array_print(unsigned int * array, unsigned int max)
{
  printf("Results: ");
  for (unsigned int i = 0; i < max; i++) {
    // if the value is zero, then it was not used => finished
    // assume positive message_id's
    if (array[i] > 0) {
      printf("%d ", array[i]);
    } else {
      break;
    }
  }
  printf("\n");
}

bool
_executor_results_compare(unsigned int * array)
{
  // assume that array has same size as MSG_MAX
  for (unsigned int i = 0; i < MSG_MAX; i++) {
    if (_executor_results[i] != array[i]) {
      return false;
    }
  }
  return true;
}


void int32_callback1(const void * msgin)
{
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  if (msg == NULL) {
    printf("Test CB: msg NULL\n");
  } else {
    // printf("Test CB 1: msg %d\n", msg->data);
  }
  _cb1_cnt++;
  _executor_results_add(1);
}

void int32_callback2(const void * msgin)
{
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  if (msg == NULL) {
    printf("Test CB: msg NULL\n");
  } else {
    // printf("Test CB 2: msg %d\n", msg->data);
  }
  _cb2_cnt++;
  _executor_results_add(2);
}

void int32_callback3(const void * msgin)
{
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  if (msg == NULL) {
    printf("Test CB: msg NULL\n");
  } else {
    // printf("Test CB 3: msg %d\n", msg->data);
  }
  _cb3_cnt++;
  _executor_results_add(3);
}


// result vector
// old stuff

// callback for topic "cmd_hello"
void cmd_hello_callback(const void * msgin)
{
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;


  if (msg == NULL) {
    printf("Callback: 'cmd_hello' msg NULL\n");
  } else {
    printf("Callback 'cmd_hello': I heard xx x: %s\n", msg->data.data);
  }
}

// callback for topic "cmd_vel"
int numberMsgCmdVel = 0;
void cmd_vel_callback(const void * msgin)  // TwistConstPtr
{
  const geometry_msgs__msg__Twist * twist = (const geometry_msgs__msg__Twist *)msgin;
  numberMsgCmdVel++;
  // printf("cmd_vel received(#%d)\n", numberMsgCmdVel);

  if (twist != NULL) {
    // printf("[#%d] Callback 'cmd_vel': tv=%f rv=%f \n", numberMsgCmdVel, (twist->linear.x),
    // (twist->angular.z));
  } else {
    printf("Error callback commandVelCallback: Twist message is NULL.\n");
  }
}

// timer callback
void my_timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  // Do timer work...
  // Optionally reconfigure, cancel, or reset the timer...
  if (timer != NULL) {
    printf("Timer: time since last call %d\n", static_cast<int>(last_call_time));
  }
}

// function call
void my_function_call(void)
{
  _fn_cnt++;
  // printf("Function called\n");
}

class TestDefaultExecutor : public ::testing::Test
{
public:
  rcl_context_t * context_ptr;
  rcl_node_t * node_ptr;


  // publisher 1
  rcl_publisher_t * pub1_ptr;
  const char * pub1_topic_name;
  const rosidl_message_type_support_t * pub1_type_support;
  rcl_publisher_options_t pub1_options;
  geometry_msgs__msg__Twist pub1_msg;
  // publisher 2

  // subscription 1
  rcl_subscription_t * sub1_ptr;
  const char * sub1_topic_name;
  const rosidl_message_type_support_t * sub1_type_support;
  rcl_subscription_options_t sub1_options;
  geometry_msgs__msg__Twist sub1_msg;

  // subscription 2
  rcl_subscription_t * sub2_ptr;
  const char * sub2_topic_name;
  const rosidl_message_type_support_t * sub2_type_support;
  rcl_subscription_options_t sub2_options;
  std_msgs__msg__String sub2_msg;


  // timer 1
  rcl_timer_t * timer1_ptr;
  const unsigned int timer1_timeout = 100;
  rcl_clock_t * clock_ptr;
  rcl_allocator_t * clock_allocator_ptr;


  const rcl_allocator_t * allocator;
  void SetUp()
  {
    rcl_ret_t ret;
    {
      rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
      ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
      OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
        EXPECT_EQ(RCL_RET_OK, rcl_init_options_fini(&init_options)) << rcl_get_error_string().str;
      });
      this->context_ptr = new rcl_context_t;
      *this->context_ptr = rcl_get_zero_initialized_context();
      ret = rcl_init(0, nullptr, &init_options, this->context_ptr);
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    }
    // create ROS node
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "example_executor_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", this->context_ptr, &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    const rcl_node_options_t * node_ops = rcl_node_get_options(this->node_ptr);
    this->allocator = &node_ops->allocator;

    // create publisher 1 - cmd_vel
    this->pub1_topic_name = "cmd_vel";
    const char * expected_topic_name = "/cmd_vel";
    this->pub1_ptr = new rcl_publisher_t;
    *this->pub1_ptr = rcl_get_zero_initialized_publisher();
    this->pub1_type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist);
    this->pub1_options = rcl_publisher_get_default_options();
    ret = rcl_publisher_init(this->pub1_ptr, this->node_ptr, this->pub1_type_support,
        this->pub1_topic_name, &this->pub1_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      rcl_ret_t ret = rcl_publisher_fini(this->pub1_ptr, this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    });
    EXPECT_EQ(strcmp(rcl_publisher_get_topic_name(this->pub1_ptr), expected_topic_name), 0);

    // create publisher 2

    // create subscription for 'cmd_vel'
    this->sub1_topic_name = "cmd_vel";
    this->sub1_ptr = new rcl_subscription_t;
    *this->sub1_ptr = rcl_get_zero_initialized_subscription();
    this->sub1_type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist);
    this->sub1_options = rcl_subscription_get_default_options();
    ret = rcl_subscription_init(this->sub1_ptr, this->node_ptr, this->sub1_type_support,
        this->sub1_topic_name, &this->sub1_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    // create subscription for 'cmd_hello'
    this->sub2_topic_name = "cmd_hello";
    this->sub2_ptr = new rcl_subscription_t;
    *this->sub2_ptr = rcl_get_zero_initialized_subscription();
    this->sub2_type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
    this->sub2_options = rcl_subscription_get_default_options();
    ret = rcl_subscription_init(this->sub2_ptr, this->node_ptr, this->sub2_type_support,
        this->sub2_topic_name, &this->sub2_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

    // create timer with rcl
    this->clock_ptr = new rcl_clock_t;
    this->clock_allocator_ptr = new rcl_allocator_t;
    // TODO(jst3si) can 'this->allocator' be used here as well?
    *this->clock_allocator_ptr = rcl_get_default_allocator();
    ret = rcl_clock_init(RCL_STEADY_TIME, this->clock_ptr, this->clock_allocator_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    this->timer1_ptr = new rcl_timer_t;
    *this->timer1_ptr = rcl_get_zero_initialized_timer();
    ret =
      rcl_timer_init(this->timer1_ptr, this->clock_ptr, this->context_ptr, RCL_MS_TO_NS(
          this->timer1_timeout),
        my_timer_callback, *this->clock_allocator_ptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }

  void TearDown()
  {
    rcl_ret_t ret;

    ret = rcl_subscription_fini(this->sub1_ptr, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->sub1_ptr;

    ret = rcl_subscription_fini(this->sub2_ptr, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->sub2_ptr;

    ret = rcl_timer_fini(this->timer1_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->timer1_ptr;

    ret = rcl_clock_fini(this->clock_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    delete this->clock_ptr;

    delete this->clock_allocator_ptr;

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


/*
 * Test suite
 */
TEST_F(TestDefaultExecutor, executor_init) {
  rcl_ret_t rc;

  rcle_let_executor_t executor;
  rc = rcle_let_executor_init(&executor, this->context_ptr, 10, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // Executor: NULL executor
  rc = rcle_let_executor_init(NULL, this->context_ptr, 10, this->allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rc) << rcl_get_error_string().str;
  rcutils_reset_error();

  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_ERROR, rc) << rcl_get_error_string().str;
  rcutils_reset_error();

  // Error case: zero handles
  rc = rcle_let_executor_init(&executor, this->context_ptr, 0, this->allocator);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rc) << rcl_get_error_string().str;
  rcutils_reset_error();

  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_ERROR, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
}

/*
 * Test suite
 */
TEST_F(TestDefaultExecutor, executor_fini) {
  rcl_ret_t rc;
  rcle_let_executor_t executor;

  // test normal case and failure-cases:
  // - _fini function was called before.

  // failure: executor not initialized
  // rc = rcle_let_executor_fini(&executor);
  // EXPECT_EQ(RCL_RET_ERROR, rc) << rcl_get_error_string().str;
  // rcutils_reset_error();
  // result : is not detected, even in un-initialized executor had in this case
  //          executor->initialized == true !!!

  rc = rcle_let_executor_init(&executor, this->context_ptr, 10, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // normal case
  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // failure: call fini twice
  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_ERROR, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
}

TEST_F(TestDefaultExecutor, executor_add_subscription) {
  rcl_ret_t rc;
  rcle_let_executor_t executor;

  // test with normal arguemnt and NULL pointers as arguments
  rc = rcle_let_executor_init(&executor, this->context_ptr, 10, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // normal case
  rc = rcle_let_executor_add_subscription(&executor, this->sub1_ptr, &this->sub1_msg,
      &cmd_vel_callback, ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
  size_t num_subscriptions = 1;
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // test NULL pointer for executor
  rc = rcle_let_executor_add_subscription(NULL, this->sub1_ptr, &this->sub1_msg, &cmd_vel_callback,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // test NULL pointer for subscription
  rc = rcle_let_executor_add_subscription(&executor, NULL, &this->sub1_msg, &cmd_vel_callback,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // test NULL pointer for message
  rc = rcle_let_executor_add_subscription(&executor, this->sub1_ptr, NULL, &cmd_vel_callback,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // test NULL pointer for callback
  rc = rcle_let_executor_add_subscription(&executor, this->sub1_ptr, &this->sub1_msg, NULL,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // tear down
  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
}

TEST_F(TestDefaultExecutor, executor_add_subscription_too_many) {
  rcl_ret_t rc;
  rcle_let_executor_t executor;

  // insert one handle, add two subscriptions

  rc = rcle_let_executor_init(&executor, this->context_ptr, 1, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // test 1: add subscription
  rc = rcle_let_executor_add_subscription(&executor, this->sub1_ptr, &this->sub1_msg,
      &cmd_vel_callback, ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
  size_t num_subscriptions = 1;
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // test 2: add another subscription : failure (array full)
  rc = rcle_let_executor_add_subscription(&executor, this->sub2_ptr, &this->sub2_msg,
      &cmd_hello_callback, ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_ERROR, rc) << rcl_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected to be one";

  // tear down
  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
}

TEST_F(TestDefaultExecutor, executor_add_timer) {
  rcl_ret_t rc;
  rcle_let_executor_t executor;

  // add a timer
  rc = rcle_let_executor_init(&executor, this->context_ptr, 10, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  size_t exp_number_of_timers = 0;
  EXPECT_EQ(executor.info.number_of_timers, exp_number_of_timers) << "#times should be 0";
  rc = rcle_let_executor_add_timer(&executor, this->timer1_ptr);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
  exp_number_of_timers = 1;
  EXPECT_EQ(executor.info.number_of_timers, exp_number_of_timers) << "#timers should be 1";

  // tear down
  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
}

TEST_F(TestDefaultExecutor, executor_spin_some_API) {
  rcl_ret_t rc;
  rcle_let_executor_t executor;

  // add a timer
  rc = rcle_let_executor_init(&executor, this->context_ptr, 10, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  rc = rcle_let_executor_add_timer(&executor, this->timer1_ptr);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  const unsigned int timeout_ms = 100;
  rc = rcle_let_executor_spin_some(&executor, timeout_ms);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // tear down
  rc = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;
}


void
wait_for_subscription_to_be_ready(
  rcl_subscription_t * subscription,
  rcl_context_t * context_ptr,
  size_t max_tries,
  int64_t period_ms,
  bool & success)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, 0, context_ptr,
      rcl_get_default_allocator());
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

TEST_F(TestDefaultExecutor, pub_sub_example) {
  // 27.06.2019, copied from ros2/rcl/rcl/test/rcl/test_subscriptions.cpp
  // by Jan Staschulat, under Apache 2.0 License
  rcl_ret_t ret;
  unsigned int expected_msg;

  // publisher
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);
  const char * topic = "chatter";
  const char * expected_topic = "/chatter";
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher, this->node_ptr, ts, topic, &publisher_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // subscription
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription, this->node_ptr, ts, topic, &subscription_options);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_EQ(strcmp(rcl_subscription_get_topic_name(&subscription), expected_topic), 0);
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription));
  rcl_reset_error();

  // TODO(wjwwood): add logic to wait for the connection to be established
  //                probably using the count_subscriptions busy wait mechanism
  //                until then we will sleep for a short period of time
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  {
    std_msgs__msg__Int32 msg;
    std_msgs__msg__Int32__init(&msg);
    msg.data = 42;
    ret = rcl_publish(&publisher, &msg, nullptr);
    std_msgs__msg__Int32__fini(&msg);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  }
  bool success;
  wait_for_subscription_to_be_ready(&subscription, this->context_ptr, 10, 100, success);
  ASSERT_TRUE(success);
  {
    std_msgs__msg__Int32 msg;
    std_msgs__msg__Int32__init(&msg);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      std_msgs__msg__Int32__fini(&msg);
    });
    ret = rcl_take(&subscription, &msg, nullptr, nullptr);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
    ASSERT_EQ(42, msg.data);

    // initialize all callback counters
    _results_callback_counters_init();
    // call callback with msg
    int32_callback1(&msg);
    // check result
    expected_msg = 1;
    ASSERT_EQ(_cb1_cnt, expected_msg) << "expect = 1";
  }
}


TEST_F(TestDefaultExecutor, spin_some_let_semantic) {
  // 27.06.2019, adopted from ros2/rcl/rcl/test/rcl/test_subscriptions.cpp
  // by Jan Staschulat, under Apache 2.0 License
  rcl_ret_t ret;
  rcle_let_executor_t executor;
  unsigned int expected_msg;

  // publisher 1
  rcl_publisher_t publisher1 = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts1 =
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);
  const char * topic1 = "chatter1";
  rcl_publisher_options_t publisher_options1 = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher1, this->node_ptr, ts1, topic1, &publisher_options1);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher1, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // publisher 2
  rcl_publisher_t publisher2 = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts2 =
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);
  const char * topic2 = "chatter2";
  rcl_publisher_options_t publisher_options2 = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher2, this->node_ptr, ts2, topic2, &publisher_options2);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher2, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // publisher 3
  rcl_publisher_t publisher3 = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts3 =
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);
  const char * topic3 = "chatter3";
  rcl_publisher_options_t publisher_options3 = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher3, this->node_ptr, ts3, topic3, &publisher_options3);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher3, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });


  // subscription 1
  rcl_subscription_t subscription1 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options1 = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription1, this->node_ptr, ts1, topic1, &subscription_options1);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription1, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription1));
  rcl_reset_error();

  // subscription 2
  rcl_subscription_t subscription2 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options2 = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription2, this->node_ptr, ts2, topic2, &subscription_options2);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription2, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription2));
  rcl_reset_error();

  // subscription 3
  rcl_subscription_t subscription3 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options3 = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription3, this->node_ptr, ts3, topic3, &subscription_options3);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription3, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription3));
  rcl_reset_error();

  // initialize result variables
  _executor_results_init();
  // initialize executor with 3 handles
  ret = rcle_let_executor_init(&executor, this->context_ptr, 3, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // define subscription messages
  std_msgs__msg__Int32 sub_msg1;
  std_msgs__msg__Int32__init(&sub_msg1);

  std_msgs__msg__Int32 sub_msg2;
  std_msgs__msg__Int32__init(&sub_msg2);

  std_msgs__msg__Int32 sub_msg3;
  std_msgs__msg__Int32__init(&sub_msg3);

  // add subscription to the executor
  ret =
    rcle_let_executor_add_subscription(&executor, &subscription1, &sub_msg1, &int32_callback1,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret =
    rcle_let_executor_add_subscription(&executor, &subscription2, &sub_msg2, &int32_callback2,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret =
    rcle_let_executor_add_subscription(&executor, &subscription3, &sub_msg3, &int32_callback3,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  // check, if all subscriptions were added
  size_t num_subscriptions = 3;
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions is expected 3";

  // publish message 1
  std_msgs__msg__Int32 pub_msg1;
  std_msgs__msg__Int32__init(&pub_msg1);
  pub_msg1.data = 1;

  // publish message 2
  std_msgs__msg__Int32 pub_msg2;
  std_msgs__msg__Int32__init(&pub_msg2);
  pub_msg2.data = 2;

  // publish message 3
  std_msgs__msg__Int32 pub_msg3;
  std_msgs__msg__Int32__init(&pub_msg3);
  pub_msg3.data = 3;

  ///////////////////////////////////////////////////////////////////////////////////
  /////////// test case 1 : sent in same order
  ///////////////////////////////////////////////////////////////////////////////////

  for (unsigned int i = 0; i < kMax; i++) {
    ret = rcl_publish(&publisher1, &pub_msg1, nullptr);
    EXPECT_EQ(RCL_RET_OK, ret) << " pub1 not published";

    ret = rcl_publish(&publisher2, &pub_msg2, nullptr);
    EXPECT_EQ(RCL_RET_OK, ret) << " pub2 not published";

    ret = rcl_publish(&publisher3, &pub_msg3, nullptr);
    EXPECT_EQ(RCL_RET_OK, ret) << " pub3 not published";
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  // running the executor
  for (unsigned int i = 0; i < 30; i++) {
    const unsigned int timeout_ms = 100;
    ret = rcle_let_executor_spin_some(&executor, timeout_ms);
    if ((ret == RCL_RET_OK) || (ret == RCL_RET_TIMEOUT)) {
      // valid return values
    } else {
      // any other error
      EXPECT_EQ(RCL_RET_OK, ret) << "spin_some error";
    }

    // finish - if all messages have been received
    if (_executor_results_all_msg_received()) {
      break;
    }
  }
  // check total number of received messages
  expected_msg = kMax;
  EXPECT_EQ(_cb1_cnt, expected_msg) << "cb1 msg does not match";
  EXPECT_EQ(_cb2_cnt, expected_msg) << "cb2 msg does not match";
  EXPECT_EQ(_cb3_cnt, expected_msg) << "cb3 msg does not match";
  // test order of received handles
  // _executor_results_print();

  // assumption !!! expecting kMax=3 => 9 messages
  unsigned int result_tc1[] = {1, 2, 3, 1, 2, 3, 1, 2, 3};
  EXPECT_EQ(_executor_results_compare(result_tc1), true) << "[1,2,3, ...] expected";

  ///////////////////////////////////////////////////////////////////////////////////
  /////////// test case 2 : sent in reverse order
  ///////////////////////////////////////////////////////////////////////////////////
  _executor_results_init();
  // now sent in different order
  for (unsigned int i = 0; i < kMax; i++) {
    ret = rcl_publish(&publisher3, &pub_msg3, nullptr);
    EXPECT_EQ(RCL_RET_OK, ret) << " pub3 not published";

    ret = rcl_publish(&publisher2, &pub_msg2, nullptr);
    EXPECT_EQ(RCL_RET_OK, ret) << " pub2 not published";

    ret = rcl_publish(&publisher1, &pub_msg1, nullptr);
    EXPECT_EQ(RCL_RET_OK, ret) << " pub1 not published";
  }
  // wait for some time until DDS queue is ready
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  // running the executor
  for (unsigned int i = 0; i < 30; i++) {
    const unsigned int timeout_ms = 100;
    ret = rcle_let_executor_spin_some(&executor, timeout_ms);
    if ((ret == RCL_RET_OK) || (ret == RCL_RET_TIMEOUT)) {
      // valid return values
    } else {
      // any other error
      EXPECT_EQ(RCL_RET_OK, ret) << "spin_some error";
    }

    if (_executor_results_all_msg_received()) {
      break;
    }

    // wait for some time
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  // check total number of received messages
  expected_msg = kMax;
  EXPECT_EQ(_cb1_cnt, expected_msg) << "cb1 msg does not match";
  EXPECT_EQ(_cb2_cnt, expected_msg) << "cb2 msg does not match";
  EXPECT_EQ(_cb3_cnt, expected_msg) << "cb3 msg does not match";
  // test order of received handles
  // _executor_results_print();

  // assumption !!! expecting kMax=3 => 9 messages
  unsigned int result_tc2[] = {1, 2, 3, 1, 2, 3, 1, 2, 3};
  EXPECT_EQ(_executor_results_compare(result_tc2), true) << "[1,2,3, ...] expected";

  // clean-up
  std_msgs__msg__Int32__init(&pub_msg1);
  std_msgs__msg__Int32__init(&pub_msg2);
  std_msgs__msg__Int32__init(&pub_msg3);

  std_msgs__msg__Int32__init(&sub_msg1);
  std_msgs__msg__Int32__init(&sub_msg2);
  std_msgs__msg__Int32__init(&sub_msg3);

  ret = rcle_let_executor_fini(&executor);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
}

TEST_F(TestDefaultExecutor, invocation_type) {
  // 27.06.2019, adopted from ros2/rcl/rcl/test/rcl/test_subscriptions.cpp
  // by Jan Staschulat, under Apache 2.0 License

  // test for invocation type ALWAYS ON_NEW_DATA
  /*
  publisher A
  send 1 message
  subscriber A' with invocation=ALWAYS

  publisher B
  send 1 message
  subscriber B' with invocation=ON_NEW_DATA

  executor setup()
  executor_spin_some()
  executor_spin_some()

  expected result
  number of invocations of callback A' = 2
  number of invocations of callback B' = 1

   */

// 27.06.2019, adopted from ros2/rcl/rcl/test/rcl/test_subscriptions.cpp
  // by Jan Staschulat, under Apache 2.0 License
  rcl_ret_t ret;
  rcle_let_executor_t executor;

  // publisher 1
  rcl_publisher_t publisher1 = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts1 =
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);
  const char * topic1 = "chatter1";
  rcl_publisher_options_t publisher_options1 = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher1, this->node_ptr, ts1, topic1, &publisher_options1);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher1, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // publisher 2
  rcl_publisher_t publisher2 = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * ts2 =
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);
  const char * topic2 = "chatter2";
  rcl_publisher_options_t publisher_options2 = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(&publisher2, this->node_ptr, ts2, topic2, &publisher_options2);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_publisher_fini(&publisher2, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });

  // subscription 1
  rcl_subscription_t subscription1 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options1 = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription1, this->node_ptr, ts1, topic1, &subscription_options1);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription1, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription1));
  rcl_reset_error();

  // subscription 2
  rcl_subscription_t subscription2 = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_options2 = rcl_subscription_get_default_options();
  ret = rcl_subscription_init(&subscription2, this->node_ptr, ts2, topic2, &subscription_options2);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    rcl_ret_t ret = rcl_subscription_fini(&subscription2, this->node_ptr);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  });
  EXPECT_TRUE(rcl_subscription_is_valid(&subscription2));
  rcl_reset_error();

  // initialize result variables
  _executor_results_init();
  // initialize executor with 2 handles
  ret = rcle_let_executor_init(&executor, this->context_ptr, 2, this->allocator);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;

  // define subscription messages
  std_msgs__msg__Int32 sub_msg1;
  std_msgs__msg__Int32__init(&sub_msg1);

  std_msgs__msg__Int32 sub_msg2;
  std_msgs__msg__Int32__init(&sub_msg2);

  // add subscription to the executor
  ret =
    rcle_let_executor_add_subscription(&executor, &subscription1, &sub_msg1, &int32_callback1,
      ALWAYS);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  ret =
    rcle_let_executor_add_subscription(&executor, &subscription2, &sub_msg2, &int32_callback2,
      ON_NEW_DATA);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string().str;
  rcutils_reset_error();

  // check, if all subscriptions were added
  size_t num_subscriptions = 2;
  EXPECT_EQ(executor.info.number_of_subscriptions, num_subscriptions) <<
    "number of subscriptions should be = 3";

  // publish message 1
  std_msgs__msg__Int32 pub_msg1;
  std_msgs__msg__Int32__init(&pub_msg1);
  pub_msg1.data = 1;

  // publish message 2
  std_msgs__msg__Int32 pub_msg2;
  std_msgs__msg__Int32__init(&pub_msg2);
  pub_msg2.data = 2;


  ///////////////////////////////////////////////////////////////////////////////////
  /////////// test case 1 : publish one data for each publisher
  ///////////////////////////////////////////////////////////////////////////////////


  ret = rcl_publish(&publisher1, &pub_msg1, nullptr);
  EXPECT_EQ(RCL_RET_OK, ret) << " publisher1 did not publish!";

  ret = rcl_publish(&publisher2, &pub_msg2, nullptr);
  EXPECT_EQ(RCL_RET_OK, ret) << " publisher2 did not publish!";

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // initialize result variables
  _cb1_cnt = 0;
  _cb2_cnt = 0;

  // running the executor
  unsigned int max_iterations = 2;
  for (unsigned int i = 0; i < max_iterations; i++) {
    const unsigned int timeout_ms = 100;
    ret = rcle_let_executor_spin_some(&executor, timeout_ms);
    if ((ret == RCL_RET_OK) || (ret == RCL_RET_TIMEOUT)) {
      // valid return values
    } else {
      // any other error
      EXPECT_EQ(RCL_RET_OK, ret) << "spin_some error";
    }
  }
  // check total number of received messages
  EXPECT_EQ(_cb1_cnt, (unsigned int) 2) << "cb1 msg does not match";
  EXPECT_EQ(_cb2_cnt, (unsigned int) 1) << "cb2 msg does not match";
}


/* NOTE
  this unit test is by default disabled, because spin_period runs
  indefinitly - therefore call it manually and switch on the statistics
  code in rcl_executor.c as described below.

to run this test case you also need to set the define
#define unit_test_spin_period
execute the unit not with colcon test but direcly with
$>rcl_executor/build/rcl_executor/test_let_executor

test result (24.07.2017 Linux Ubunutu 16.04)
avarage is computed over 1000 iterations of spin_some()

period 100ms:
[ RUN      ] TestDefaultExecutor.spin_period
period average 100.000511
period average 100.000000
period average 99.999611
period average 100.000046
period average 100.000359
period average 99.999634
period average 99.999954
period average 100.000053
period average 100.000343
period average 99.999969
...
period 20ms:
[ RUN      ] TestDefaultExecutor.spin_period
period average 20.000525
period average 19.999645
period average 20.000040
period average 19.999886
period average 20.000019
period average 20.000000
period average 20.000423
period average 19.999517
period average 20.000116
period average 19.999918
...
period 10ms:
[ RUN      ] TestDefaultExecutor.spin_period
period average 10.000107
period average 10.000002
period average 9.999947
period average 10.000008
period average 10.000076
period average 10.000026
period average 9.999919
period average 10.000435
period average 9.999641
period average 10.000361

...
period 1ms:
[ RUN      ] TestDefaultExecutor.spin_period
period average 1.000101
period average 1.000021
period average 1.000009
period average 1.000118
period average 0.999885
period average 1.000282
period average 0.999637
period average 1.000084
period average 0.999999
period average 0.999971
...
*/
/*
TEST_F(TestDefaultExecutor, spin_period) {
  // 27.06.2019, adopted from ros2/rcl/rcl/test/rcl/test_subscriptions.cpp
  // by Jan Staschulat, under Apache 2.0 License
  rcl_ret_t rc;
  rcle_let_executor_t executor;

  // to make it work, go to the implementation of the function rcle_let_executor_spin_period
  // and un-comment the lines regarding the statistics as well as the printf
  // // printf("period %ld \n", p_micros_used );
  // then you should see the average period which is expected to be close to the specified one.

  // initialize result variables
  _fn_cnt = 0;

  // initialize executor with 1 handle
  rc = rcle_let_executor_init(&executor, this->context_ptr, 1, this->allocator);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  rc = rcle_let_executor_add_timer(&executor, this->timer1_ptr);
  EXPECT_EQ(RCL_RET_OK, rc) << rcl_get_error_string().str;

  // spin with period 10ms
  rcle_let_executor_spin_period(&executor, 10);
  rcle_let_executor_fini(&executor);
}
*/
