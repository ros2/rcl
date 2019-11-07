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

#include "rcl_ext/rcl_ext.h"

rcl_ret_t
rcl_ext_init(
  rcl_ext_init_t * init_obj,
  int argc,
  char const * const * argv,
  rcl_allocator_t * allocator)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "init_obj is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t rc = RCL_RET_OK;

  init_obj->init_options = rcl_get_zero_initialized_init_options();
  rc = rcl_init_options_init(&init_obj->init_options, (*allocator) );
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_init, rcl_init_options_init);
    return rc;
  }

  init_obj->context = rcl_get_zero_initialized_context();
  rc = rcl_init(argc, argv, &init_obj->init_options, &init_obj->context);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_init, rcl_init);
    return rc;
  }
  init_obj->allocator = allocator;
  return rc;
}

rcl_ret_t
rcl_ext_init_fini(rcl_ext_init_t * init_obj)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "init_obj is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t rc;
  rc = rcl_init_options_fini(&init_obj->init_options);
  // other objects are on the stack and not on the heap
  return rc;
}

rcl_node_t *
rcl_ext_create_node(
  const char * name,
  const char * namespace_,
  rcl_ext_init_t * init_obj)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    name, "name is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    namespace_, "namespace_ is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "init_obj is a null pointer", return NULL);

  rcl_ret_t rc;
  rcl_node_t * node = init_obj->allocator->allocate(sizeof(rcl_node_t), init_obj->allocator->state);
  if (node != NULL) {
    (*node) = rcl_get_zero_initialized_node();
    // node_ops is copied to ...->impl->node-options, therefore temporary scope sufficient
    rcl_node_options_t node_ops = rcl_node_get_default_options();
    rc = rcl_node_init(node, name, namespace_,
        &init_obj->context, &node_ops);
    if (rc != RCL_RET_OK) {
      init_obj->allocator->deallocate(node, init_obj->allocator->state);
      PRINT_RCL_ERROR(rcl_ext_create_node, rcl_node_init);
      return NULL;
    }
  }
  return node;
}

rcl_ret_t
rcl_ext_node_fini(rcl_ext_init_t * init_obj, rcl_node_t * node)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node, "node is a null pointer", return RCL_RET_INVALID_ARGUMENT);

  // clean-up rcl_node_t
  rcl_ret_t rc = rcl_node_fini(node);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_node_fini, rcl_node_fini);
  }

  // de-allocate node itself
  init_obj->allocator->deallocate(node, init_obj->allocator->state);

  return rc;
}

rcl_publisher_t *
rcl_ext_create_publisher(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node, "node is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator, "allocator is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    type_support, "type_support is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    topic_name, "topic_name is a null pointer", return NULL);

  rcl_publisher_t * pub = allocator->allocate(
    sizeof(rcl_publisher_t), allocator->state);
  (*pub) = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t pub_opt = rcl_publisher_get_default_options();
  rcl_ret_t rc = rcl_publisher_init(
    pub,
    node,
    type_support,
    topic_name,
    &pub_opt);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_create_publisher, rcl_publisher_init);
    allocator->deallocate(pub, allocator->state);
    return NULL;
  }
  return pub;
}

rcl_ret_t
rcl_ext_publisher_fini(rcl_ext_init_t * init_obj, rcl_publisher_t * publisher, rcl_node_t * node)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "init_obj is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    publisher, "publisher is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t rc;

  // clean-up publisher
  rc = rcl_publisher_fini(publisher, node);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_publisher_fini, rcl_publisher_fini);
  }

  // de-allocate publisher itself
  init_obj->allocator->deallocate(publisher, init_obj->allocator->state);
  return rc;
}

rcl_subscription_t *
rcl_ext_create_subscription(
  rcl_node_t * node,
  rcl_allocator_t * allocator,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node, "node is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator, "allocator is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    type_support, "type_support is a null pointer", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    topic_name, "topic_name is a null pointer", return NULL);

  rcl_subscription_t * sub = allocator->allocate(sizeof(rcl_subscription_t), allocator->state);
  (*sub) = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
  rcl_ret_t rc = rcl_subscription_init(
    sub,
    node,
    type_support,
    topic_name,
    &sub_ops);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_create_subscription, rcl_subscription_init);
    allocator->deallocate(sub, allocator->state);
    return NULL;
  }
  return sub;
}

rcl_ret_t
rcl_ext_subscription_fini(
  rcl_ext_init_t * init_obj, rcl_subscription_t * subscription,
  rcl_node_t * node)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "init_obj is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    subscription, "subscription is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    node, "node is a null pointer", return RCL_RET_INVALID_ARGUMENT);

  // de-allocate memory inside subscription
  rcl_ret_t rc = rcl_subscription_fini(subscription, node);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_subscription_fini, rcl_subscription_fini);
  }

  // de-allocate subscription itself
  init_obj->allocator->deallocate(subscription, init_obj->allocator->state);
  return rc;
}

rcl_timer_t *
rcl_ext_create_timer(
  rcl_ext_init_t * init_obj,
  const uint64_t timeout_ns,
  const rcl_timer_callback_t callback)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "node is a null pointer", return NULL);

  rcl_ret_t rc;
  rcl_timer_t * timer = init_obj->allocator->allocate(sizeof(rcl_timer_t),
      init_obj->allocator->state);

  rc = rcl_clock_init(RCL_STEADY_TIME, &init_obj->clock, init_obj->allocator);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_create_timer, rcl_clock_init);
    init_obj->allocator->deallocate(timer, init_obj->allocator->state);
    return NULL;
  }

  (*timer) = rcl_get_zero_initialized_timer();
  rc = rcl_timer_init(timer, &init_obj->clock, &init_obj->context, timeout_ns,
      callback, (*init_obj->allocator));
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_create_timer, rcl_timer_init);
    return NULL;
  } else {
    RCUTILS_LOG_INFO("Created a timer with period %ld ms.\n", timeout_ns / 1000000);
  }

  return timer;
}

rcl_ret_t
rcl_ext_timer_fini(rcl_ext_init_t * init_obj, rcl_timer_t * timer)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    init_obj, "init_obj is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    timer, "timer is a null pointer", return RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t rc;

  // de-allocate the memory within rcl_timer_t
  rc = rcl_timer_fini(timer);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(rcl_ext_timer_fini, rcl_timer_fini);
  }
  // de-allocate the timer itself
  init_obj->allocator->deallocate(timer, init_obj->allocator->state);
  return rc;
}
