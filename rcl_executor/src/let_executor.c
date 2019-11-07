// Copyright (c) 2018 - for information on the respective copyright owner
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

#include "rcl_executor/let_executor.h"

#include <sys/time.h>  // for gettimeofday()
#include <unistd.h>  // for usleep()

// declarations of helper functions

/// get new data from DDS queue for handle i
static
rcl_ret_t
_rcle_read_input_data(rcle_let_executor_t * executor, rcl_wait_set_t * wait_set, size_t i);

/// execute callback of handle i
static
rcl_ret_t
_rcle_execute(rcle_let_executor_t * executor, rcl_wait_set_t * wait_set, size_t i);

static
rcl_ret_t
_rcle_let_scheduling(rcle_let_executor_t * executor, rcl_wait_set_t * wait_set);


void
_rcle_print_handles(rcle_let_executor_t * executor)
{
  for (unsigned int i = 0; i < executor->max_handles && executor->handles[i].initialized; i++) {
    rcle_handle_print(&executor->handles[i]);
  }
}

rcl_ret_t
rcle_let_executor_init(
  rcle_let_executor_t * e,
  rcl_context_t * context,
  const size_t number_of_handles,
  const rcl_allocator_t * allocator)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(e, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  if (number_of_handles == 0) {
    RCL_SET_ERROR_MSG("number_of_handles is 0. Must be larger or equal to 1");
    return RCL_RET_INVALID_ARGUMENT;
  }

  rcl_ret_t ret = RCL_RET_OK;
  e->context = context;
  e->max_handles = number_of_handles;
  e->index = 0;

  // e->wait_set willl be initialized only once in the rcle_executor_spin_some function
  e->wait_set_initialized = false;

  e->allocator = allocator;
  e->timeout_ns = 100000000;  // default value 100ms = 100 000 000 ns
  // allocate memory for the array
  e->handles = e->allocator->allocate( (number_of_handles * sizeof(rcle_handle_t)),
      e->allocator->state);
  if (NULL == e->handles) {
    RCL_SET_ERROR_MSG("Could not allocate memory for 'handles'.");
    return RCL_RET_BAD_ALLOC;
  }

  // initialize handle
  for (size_t i = 0; i < number_of_handles; i++) {
    rcle_handle_init(&e->handles[i], number_of_handles);
  }

  // debug: output handle
  /*
  for (size_t i = 0; i < number_of_handles;i++)
  {
          rcle_handle_print(& e->handles[i] );
  }
  */
  // initialize #counts for handle types
  rcle_handle_size_zero_init(&e->info);
  e->initialized = true;
  return ret;
}

rcl_ret_t
rcle_let_executor_set_timeout(rcle_let_executor_t * executor, const uint64_t timeout_ns)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    executor, "executor is null pointer", return RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t ret = RCL_RET_OK;
  if (executor->initialized) {
    executor->timeout_ns = timeout_ns;
  } else {
    RCL_SET_ERROR_MSG("executor not initialized.");
    return RCL_RET_ERROR;
  }
  return ret;
}

rcl_ret_t
rcle_let_executor_fini(rcle_let_executor_t * executor)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    executor, "executor is null pointer", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    executor->handles, "memory for exector->handles is NULL", return RCL_RET_ERROR);
  rcl_ret_t ret = RCL_RET_OK;

  if (executor->initialized) {
    executor->allocator->deallocate(executor->handles, executor->allocator->state);
    executor->max_handles = 0;
    executor->index = 0;
    rcle_handle_size_zero_init(&executor->info);

    // free memory of wait_set if it has been initialized
    // calling it with un-initialized wait_set will fail.
    if (executor->wait_set_initialized) {
      rcl_ret_t rc = rcl_wait_set_fini(&executor->wait_set);
      if (rc != RCL_RET_OK) {
        PRINT_RCL_ERROR(rcle_let_executor_fini, rcl_wait_set_fini);
      }
      executor->wait_set_initialized = false;
    }

    executor->initialized = false;
    // todo(jst3si) reset timeout to default value
  } else {
    RCL_SET_ERROR_MSG("executor not initialized or called _fini function twice");
    ret = RCL_RET_ERROR;     // TODO(jst3si) better name for "calling this function multiple times"
  }
  return ret;
}


rcl_ret_t
rcle_let_executor_add_subscription(
  rcle_let_executor_t * executor,
  rcl_subscription_t * subscription,
  void * msg,
  rcle_callback_t callback,
  rcle_invocation_t invocation)
{
  rcl_ret_t ret = RCL_RET_OK;

  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(subscription, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(msg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(callback, RCL_RET_INVALID_ARGUMENT);

  // array bound check
  if (executor->index >= executor->max_handles) {
    rcl_ret_t ret = RCL_RET_ERROR;     // TODO(jst3si) better name : RCLE_RET_BUFFER_OVERFLOW
    RCL_SET_ERROR_MSG("Buffer overflow of 'executor->handles'. Increase 'max_handles'");
    return ret;
  }

  // assign data fields
  executor->handles[executor->index].type = SUBSCRIPTION;
  executor->handles[executor->index].subscription = subscription;
  executor->handles[executor->index].data = msg;
  executor->handles[executor->index].callback = callback;
  executor->handles[executor->index].invocation = invocation;
  executor->handles[executor->index].initialized = true;

  // increase index of handle array
  executor->index++;

  executor->info.number_of_subscriptions++;

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Added a subscription.");
  return ret;
}


rcl_ret_t
rcle_let_executor_add_timer(
  rcle_let_executor_t * executor,
  rcl_timer_t * timer)
{
  rcl_ret_t ret = RCL_RET_OK;

  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(timer, RCL_RET_INVALID_ARGUMENT);

  // array bound check
  if (executor->index >= executor->max_handles) {
    rcl_ret_t ret = RCL_RET_ERROR;     // TODO(jst3si) better name : RCLE_RET_BUFFER_OVERFLOW
    RCL_SET_ERROR_MSG("Buffer overflow of 'executor->handles'. Increase 'max_handles'");
    return ret;
  }

  // assign data fields
  executor->handles[executor->index].type = TIMER;
  executor->handles[executor->index].timer = timer;
  executor->handles[executor->index].invocation = ON_NEW_DATA;  // i.e. when timer elapsed
  executor->handles[executor->index].initialized = true;

  // increase index of handle array
  executor->index++;

  executor->info.number_of_timers++;

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Added a timer.");
  return ret;
}

/***
 * operates on handle executor->handles[i]
 * - evaluates the status bit in the wait_set for this handles
 * - if new data is available, rcl_take fetches this data from DDS and copies message to
 *   executor->handles[i].data
 * - and sets executor->handles[i].data_available = true
 */
static
rcl_ret_t
_rcle_read_input_data(rcle_let_executor_t * executor, rcl_wait_set_t * wait_set, size_t i)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t rc = RCL_RET_OK;

  // initialize status
  executor->handles[i].data_available = false;

  switch (executor->handles[i].type) {
    case SUBSCRIPTION:
      // if handle is available, call rcl_take, which copies the message to 'msg'
      if (wait_set->subscriptions[executor->handles[i].index]) {
        rmw_message_info_t messageInfo;
        rc = rcl_take(executor->handles[i].subscription, executor->handles[i].data, &messageInfo,
            NULL);
        if (rc != RCL_RET_OK) {
          // it is documented, that rcl_take might return this error with successfull rcl_wait
          if (rc != RCL_RET_SUBSCRIPTION_TAKE_FAILED) {
            PRINT_RCL_ERROR(rcle_read_input_data, rcl_take);
            RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Error number: %d", rc);
          }

          return rc;
        }
        executor->handles[i].data_available = true;
      }
      break;

    case TIMER:
      if (wait_set->timers[executor->handles[i].index]) {
        // get timer
        bool timer_is_ready = false;
        rc = rcl_timer_is_ready(executor->handles[i].timer, &timer_is_ready);
        if (rc != RCL_RET_OK) {
          PRINT_RCL_ERROR(rcle_read_input_data, rcl_timer_is_ready);
          return rc;
        }

        // actually this is a double check: if wait_set.timers[i] is true, then also the function
        // rcl_timer_is_ready should return true.
        if (timer_is_ready) {
          executor->handles[i].data_available = true;
        } else {
          PRINT_RCL_ERROR(rcle_read_input_data, rcl_timer_should_be_ready);
          return RCL_RET_ERROR;
        }
      }
      break;

    default:
      RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Error:wait_set unknwon handle type: %d",
        executor->handles[i].type);
      return RCL_RET_ERROR;
  }    // switch-case
  return rc;
}

/***
 * operates on executor->handles[i] object
 * - calls every callback of each object depending on its type
 */
static
rcl_ret_t
_rcle_execute(rcle_let_executor_t * executor, rcl_wait_set_t * wait_set, size_t i)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t rc = RCL_RET_OK;

  bool invoke_callback = false;

  // determine, if callback shall be called
  if (executor->handles[i].invocation == ON_NEW_DATA &&
    executor->handles[i].data_available == true)
  {
    invoke_callback = true;
  }

  if (executor->handles[i].invocation == ALWAYS) {
    invoke_callback = true;
  }

  // printf("execute: handles[%d] :  %d\n", i, invoke_callback);  // debug(jst3si)
  // execute callback
  if (invoke_callback) {
    switch (executor->handles[i].type) {
      case SUBSCRIPTION:
        executor->handles[i].callback(executor->handles[i].data);
        break;

      case TIMER:
        rc = rcl_timer_call(executor->handles[i].timer);
        if (rc != RCL_RET_OK) {
          PRINT_RCL_ERROR(rcle_execute, rcl_timer_call);
          return rc;
        }
        break;

      default:
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Execute callback: unknwon handle type: %d",
          executor->handles[i].type);
        return RCL_RET_ERROR;
    }    // switch-case
  }

  return rc;
}

static
rcl_ret_t
_rcle_let_scheduling(rcle_let_executor_t * executor, rcl_wait_set_t * wait_set)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t rc = RCL_RET_OK;

  // logical execution time
  // 1. read all input
  // 2. process
  // 3. write data (*) data is written not at the end of all callbacks, but it will not be
  //    processed by the callbacks 'in this round' because all input data is read in the
  //    beginning and the incoming messages were copied.

  // step 1:
  // take available input data from DDS queue by calling rcl_take()
  // complexity: O(n) where n denotes the number of handles
  for (size_t i = 0; (i < executor->max_handles && executor->handles[i].initialized); i++) {
    rc = _rcle_read_input_data(executor, wait_set, i);
    if ((rc != RCL_RET_OK) && (rc != RCL_RET_SUBSCRIPTION_TAKE_FAILED)) {
      return rc;
    }
  }  // for-loop


  // step 2/ step 3
  // execute the callbacks in the order of the elements in the array 'executor->handles'
  // complexity: O(n) where n denotes the number of handles
  for (size_t i = 0; (i < executor->max_handles && executor->handles[i].initialized); i++) {
    rc = _rcle_execute(executor, wait_set, i);
    if (rc != RCL_RET_OK) {
      return rc;
    }
  }
  return rc;
}

rcl_ret_t
rcle_let_executor_spin_some(rcle_let_executor_t * executor, const uint64_t timeout_ns)
{
  rcl_ret_t rc = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "spin");

  // the wait_set is initialized only once (aka in the first call of this function)
  if (executor->wait_set_initialized == false) {
    executor->wait_set = rcl_get_zero_initialized_wait_set();
    rc = rcl_wait_set_init(&executor->wait_set, executor->info.number_of_subscriptions,
        executor->info.number_of_guard_conditions, executor->info.number_of_timers,
        executor->info.number_of_clients, executor->info.number_of_services,
        executor->info.number_of_events,
        executor->context, rcl_get_default_allocator());
    if (rc != RCL_RET_OK) {
      PRINT_RCL_ERROR(rcle_let_executor_spin_some, rcl_wait_set_init);
      return rc;
    }

    executor->wait_set_initialized = true;
  }

  // set rmw fields to NULL
  rc = rcl_wait_set_clear(&executor->wait_set);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcle_let_executor_spin_some, rcl_wait_set_clear);
    return rc;
  }

  // (jst3si) put in a sub-function - for improved readability
  // add handles to wait_set
  for (size_t i = 0; (i < executor->max_handles && executor->handles[i].initialized); i++) {
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "wait_set_add_* %d", executor->handles[i].type);
    switch (executor->handles[i].type) {
      case SUBSCRIPTION:
        // add subscription to wait_set and save index
        rc = rcl_wait_set_add_subscription(&executor->wait_set, executor->handles[i].subscription,
            &executor->handles[i].index);
        if (rc != RCL_RET_OK) {
          PRINT_RCL_ERROR(rcle_let_executor_spin_some, rcl_wait_set_add_subscription);
          return rc;
        } else {
          RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
            "Subscription added to wait_set_subscription[%ld]",
            executor->handles[i].index);
        }
        break;

      case TIMER:
        // add timer to wait_set and save index
        rc = rcl_wait_set_add_timer(&executor->wait_set, executor->handles[i].timer,
            &executor->handles[i].index);
        if (rc != RCL_RET_OK) {
          PRINT_RCL_ERROR(rcle_let_executor_spin_some, rcl_wait_set_add_timer);
          return rc;
        } else {
          RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Timer added to wait_set_timers[%ld]",
            executor->handles[i].index);
        }
        break;

      default:
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Error: unknown handle type: %d",
          executor->handles[i].type);
        PRINT_RCL_ERROR(rcle_let_executor_spin_some, rcl_wait_set_unknown_handle);
        return RCL_RET_ERROR;
    }
  }

  // wait up to 'timeout_ns' to receive notification about which handles reveived
  // new data from DDS queue.
  rc = rcl_wait(&executor->wait_set, timeout_ns);

  rc = _rcle_let_scheduling(executor, &executor->wait_set);

  if (rc != RCL_RET_OK) {
    // PRINT_RCL_ERROR has already been called in _rcle_let_scheduling()
    return rc;
  }

  return rc;
}

rcl_ret_t
rcle_let_executor_spin(rcle_let_executor_t * executor)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t ret = RCL_RET_OK;
  printf("INFO: rcl_wait timeout %ld ms\n", ((executor->timeout_ns / 1000) / 1000));
  while (rcl_context_is_valid(executor->context) ) {
    ret = rcle_let_executor_spin_some(executor, executor->timeout_ns);
    if (!((ret == RCL_RET_OK) || (ret == RCL_RET_TIMEOUT))) {
      RCL_SET_ERROR_MSG("rcle_let_executor_spin_some error");
      return ret;
    }
  }
  return ret;
}

// calculates addition of positive arguments
struct timeval
timeval_add(const struct timeval * a, const struct timeval * b)
{
  struct timeval result;
  result.tv_sec = a->tv_sec + b->tv_sec;
  result.tv_usec = a->tv_usec + b->tv_usec;
  if (result.tv_usec >= 1000000) {
    result.tv_sec++;
    result.tv_usec -= 1000000;
  }
  return result;
}
/*
 period in nanoseconds
 initial test results show an accuracy of 10^-4 milli-seconds accurary
 (tested with 10ms, 20ms 100ms period on Linux Ubuntu 16.04) see unit test
 sleeping only if there is time left, i.e. when the spin_some takes longer than
 period, then usleep is not called (hoping to catch up)

 /// TODO (jst3si) write unit test to validate length of period
 */

// #define unit_test_spin_period  // enable this #define only for the Unit Test.

rcl_ret_t
rcle_let_executor_spin_period(rcle_let_executor_t * executor, const uint64_t period)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(executor, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t ret = RCL_RET_OK;

  struct timeval start, end;
  struct timeval next_time;
  struct timeval period_val;
  int64_t secs_wait, micros_wait;


  #ifdef unit_test_spin_period
  // variables for statistics
  struct timeval prev_start;
  int64_t p_secs_used, p_micros_used;
  unsigned int period_sum = 0;
  unsigned int cnt = 0;
  #endif

  // conversion from nano-seconds to micro-seconds
  uint64_t period_usec = period / 1000;

  // convert period to timeval
  if (period_usec > 1000000) {
    period_val.tv_sec = period_usec / 1000000;
    period_val.tv_usec = period_usec - period_val.tv_sec * 1000000;
  } else {
    period_val.tv_sec = 0;
    period_val.tv_usec = period_usec;
  }

  // initialization of timepoints
  gettimeofday(&start, NULL);

  #ifdef unit_test_spin_period
  prev_start.tv_sec = 0;
  prev_start.tv_usec = 0;
  #endif

  // guarantees fixed period
  next_time = timeval_add(&start, &period_val);

  // printf("period_val %u: %u\n", (unsigned int )period_val.tv_sec,
  //    (unsigned int) period_val.tv_usec);
  // printf("next_time %d : %d\n", next_time.tv_sec, next_time.tv_usec);

  while (rcl_context_is_valid(executor->context) ) {
    #ifdef unit_test_spin_period
    // only for statistics: measure start time
    gettimeofday(&start, NULL);
    #endif

    // call spin_some
    ret = rcle_let_executor_spin_some(executor, executor->timeout_ns);
    if (!((ret == RCL_RET_OK) || (ret == RCL_RET_TIMEOUT))) {
      RCL_SET_ERROR_MSG("rcle_let_executor_spin_some error");
      return ret;
    }

    // wait for x micro-seconds, where x = micros_wait = next_time - end
    gettimeofday(&end, NULL);
    secs_wait = (next_time.tv_sec - end.tv_sec);  // avoid overflow by subtracting first
    micros_wait = ((secs_wait * 1000000) + next_time.tv_usec) - (end.tv_usec);
    // printf("micro_wait %d\n", micros_wait);
    // sleep until next_time timepoint
    if (micros_wait > 0) {
      usleep(micros_wait);
    }

    // compute next timepoint to wake up
    next_time = timeval_add(&next_time, &period_val);

    #ifdef unit_test_spin_period
    if (prev_start.tv_sec == 0 && prev_start.tv_usec == 0) {
      // skip first round
    } else {
      // statistics - measure the time difference of gettimeofday(start) in each iteration
      p_secs_used = (start.tv_sec - prev_start.tv_sec);   // avoid overflow by subtracting first
      p_micros_used = ((p_secs_used * 1000000) + start.tv_usec) - (prev_start.tv_usec);
      // printf("period %ld \n", p_micros_used );

      // statistics
      period_sum += p_micros_used;
      cnt++;
      if (cnt == 1000) {
        printf("period average %f\n", (float) period_sum / (1000 * (float) cnt ));
        period_sum = 0;
        cnt = 0;
      }
    }
    // save start timepoint
    prev_start = start;
    #endif
  }
  return ret;
}
