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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl_action/action_client.h"
#include "./action_client_impl.h"

#include "rcl_action/default_qos.h"
#include "rcl_action/names.h"
#include "rcl_action/types.h"
#include "rcl_action/wait.h"

#include "rcl/client.h"
#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/node_type_cache.h"
#include "rcl/subscription.h"
#include "rcl/types.h"
#include "rcl/wait.h"

#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"

#include "rmw/qos_profiles.h"
#include "rmw/types.h"


rcl_action_client_t
rcl_action_get_zero_initialized_client(void)
{
  // All members are initialized to 0 or NULL by C99 6.7.8/10.
  static rcl_action_client_t null_action_client;
  return null_action_client;
}

rcl_action_client_impl_t
_rcl_action_get_zero_initialized_client_impl(void)
{
  rcl_client_t null_client = rcl_get_zero_initialized_client();
  rcl_subscription_t null_subscription = rcl_get_zero_initialized_subscription();
  rcl_action_client_impl_t null_action_client = {
    null_client,
    null_client,
    null_client,
    null_subscription,
    null_subscription,
    rcl_action_client_get_default_options(),
    NULL,
    0,
    0,
    0,
    0,
    0,
    rosidl_get_zero_initialized_type_hash(),
    false
  };
  return null_action_client;
}

rcl_ret_t
_rcl_action_client_fini_impl(
  rcl_action_client_t * action_client, rcl_node_t * node, rcl_allocator_t allocator)
{
  if (NULL == action_client->impl) {
    return RCL_RET_OK;
  }
  rcl_ret_t ret = RCL_RET_OK;
  if (RCL_RET_OK != rcl_client_fini(&action_client->impl->goal_client, node)) {
    ret = RCL_RET_ERROR;
  }
  if (RCL_RET_OK != rcl_client_fini(&action_client->impl->cancel_client, node)) {
    ret = RCL_RET_ERROR;
  }
  if (RCL_RET_OK != rcl_client_fini(&action_client->impl->result_client, node)) {
    ret = RCL_RET_ERROR;
  }
  if (RCL_RET_OK != rcl_subscription_fini(&action_client->impl->feedback_subscription, node)) {
    ret = RCL_RET_ERROR;
  }
  if (RCL_RET_OK != rcl_subscription_fini(&action_client->impl->status_subscription, node)) {
    ret = RCL_RET_ERROR;
  }
  if (
    ROSIDL_TYPE_HASH_VERSION_UNSET != action_client->impl->type_hash.version &&
    RCL_RET_OK != rcl_node_type_cache_unregister_type(node, &action_client->impl->type_hash))
  {
    ret = RCL_RET_ERROR;
  }
  allocator.deallocate(action_client->impl->remapped_action_name, allocator.state);
  allocator.deallocate(action_client->impl, allocator.state);
  action_client->impl = NULL;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action client finalized");
  return ret;
}

// \internal Initializes an action client specific service client.
#define CLIENT_INIT(Type) \
  char * Type ## _service_name = NULL; \
  ret = rcl_action_get_ ## Type ## _service_name( \
    action_client->impl->remapped_action_name, allocator, &Type ## _service_name); \
  if (RCL_RET_OK != ret) { \
    rcl_reset_error(); \
    RCL_SET_ERROR_MSG("failed to get " #Type " service name"); \
    goto fail; \
  } \
  rcl_client_options_t Type ## _service_client_options = { \
    .qos = options->Type ## _service_qos, .allocator = allocator \
  }; \
  action_client->impl->Type ## _client = rcl_get_zero_initialized_client(); \
  ret = rcl_client_init( \
    &action_client->impl->Type ## _client, \
    node, \
    type_support->Type ## _service_type_support, \
    Type ## _service_name, \
    &Type ## _service_client_options); \
  allocator.deallocate(Type ## _service_name, allocator.state); \
  if (RCL_RET_OK != ret) { \
    if (RCL_RET_SERVICE_NAME_INVALID == ret) { \
      ret = RCL_RET_ACTION_NAME_INVALID; \
    } \
    goto fail; \
  }

// \internal Initializes an action client specific topic subscription.
#define SUBSCRIPTION_INIT(Type) \
  char * Type ## _topic_name = NULL; \
  ret = rcl_action_get_ ## Type ## _topic_name( \
    action_client->impl->remapped_action_name, allocator, &Type ## _topic_name); \
  if (RCL_RET_OK != ret) { \
    rcl_reset_error(); \
    RCL_SET_ERROR_MSG("failed to get " #Type " topic name"); \
    goto fail; \
  } \
  rcl_subscription_options_t Type ## _topic_subscription_options = \
    rcl_subscription_get_default_options(); \
  Type ## _topic_subscription_options.qos = options->Type ## _topic_qos; \
  Type ## _topic_subscription_options.allocator = allocator; \
  action_client->impl->Type ## _subscription = rcl_get_zero_initialized_subscription(); \
  ret = rcl_subscription_init( \
    &action_client->impl->Type ## _subscription, \
    node, \
    type_support->Type ## _message_type_support, \
    Type ## _topic_name, \
    &Type ## _topic_subscription_options); \
  allocator.deallocate(Type ## _topic_name, allocator.state); \
  if (RCL_RET_OK != ret) { \
    if (RCL_RET_TOPIC_NAME_INVALID == ret) { \
      ret = RCL_RET_ACTION_NAME_INVALID; \
    } \
    goto fail; \
  }

rcl_ret_t
rcl_action_client_init(
  rcl_action_client_t * action_client,
  rcl_node_t * node,
  const rosidl_action_type_support_t * type_support,
  const char * action_name,
  const rcl_action_client_options_t * options)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_client, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(action_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t allocator = options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret = RCL_RET_OK;
  rcl_ret_t fini_ret = RCL_RET_OK;
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing client for action name '%s'", action_name);
  if (NULL != action_client->impl) {
    RCL_SET_ERROR_MSG("action client already initialized, or memory was uninitialized");
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for the implementation struct.
  action_client->impl = allocator.allocate(sizeof(rcl_action_client_impl_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    action_client->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);

  // Avoid uninitialized pointers should initialization fail
  *action_client->impl = _rcl_action_get_zero_initialized_client_impl();

  // Remap/Expand the action name
  ret = rcl_node_resolve_name(
    node,
    action_name,
    allocator,
    false, false,
    &action_client->impl->remapped_action_name
  );

  if (RCL_RET_OK != ret) {
    if (RCL_RET_TOPIC_NAME_INVALID == ret || RCL_RET_UNKNOWN_SUBSTITUTION == ret) {
      ret = RCL_RET_ACTION_NAME_INVALID;
    } else if (RCL_RET_BAD_ALLOC != ret) {
      ret = RCL_RET_ERROR;
    }
    goto fail;
  }
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME,
    "Remapped and expanded action name '%s'",
    action_client->impl->remapped_action_name
  );

  // Copy action client name and options.
  action_client->impl->options = *options;

  // Initialize action service clients.
  CLIENT_INIT(goal);
  CLIENT_INIT(cancel);
  CLIENT_INIT(result);

  // Initialize action topic subscriptions.
  SUBSCRIPTION_INIT(feedback);
  SUBSCRIPTION_INIT(status);

  ret = rcl_node_type_cache_register_type(
      node, type_support->get_type_hash_func(type_support),
      type_support->get_type_description_func(type_support),
      type_support->get_type_description_sources_func(type_support));
  if (RCL_RET_OK != ret) {
    rcutils_reset_error();
    RCL_SET_ERROR_MSG("Failed to register type for action");
    goto fail;
  }
  action_client->impl->type_hash = *type_support->get_type_hash_func(type_support);

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action client initialized");
  return ret;
fail:

  fini_ret = _rcl_action_client_fini_impl(action_client, node, allocator);
  if (RCL_RET_OK != fini_ret) {
    RCL_SET_ERROR_MSG("failed to cleanup action client");
    ret = RCL_RET_ERROR;
  }
  return ret;
}

rcl_ret_t
rcl_action_client_fini(rcl_action_client_t * action_client, rcl_node_t * node)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing action client");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  if (!rcl_node_is_valid_except_context(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  return
    _rcl_action_client_fini_impl(action_client, node, action_client->impl->options.allocator);
}

rcl_action_client_options_t
rcl_action_client_get_default_options(void)
{
  rcl_action_client_options_t default_options;
  default_options.goal_service_qos = rmw_qos_profile_services_default;
  default_options.cancel_service_qos = rmw_qos_profile_services_default;
  default_options.result_service_qos = rmw_qos_profile_services_default;
  default_options.feedback_topic_qos = rmw_qos_profile_default;
  default_options.status_topic_qos = rcl_action_qos_profile_status_default;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

rcl_ret_t
rcl_action_server_is_available(
  const rcl_node_t * node,
  const rcl_action_client_t * client,
  bool * is_available)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error is already set
  }
  if (!rcl_action_client_is_valid(client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error is already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(is_available, RCL_RET_INVALID_ARGUMENT);

  bool temp;
  rcl_ret_t ret;
  *is_available = true;

  ret = rcl_service_server_is_available(node, &(client->impl->goal_client), &temp);
  if (RCL_RET_OK != ret) {
    return ret;  // error is already set
  }
  *is_available = (*is_available && temp);

  ret = rcl_service_server_is_available(node, &(client->impl->cancel_client), &temp);
  if (RCL_RET_OK != ret) {
    return ret;  // error is already set
  }
  *is_available = (*is_available && temp);

  ret = rcl_service_server_is_available(node, &(client->impl->result_client), &temp);
  if (RCL_RET_OK != ret) {
    return ret;  // error is already set
  }
  *is_available = (*is_available && temp);

  size_t number_of_publishers;

  ret = rcl_subscription_get_publisher_count(
    &(client->impl->feedback_subscription), &number_of_publishers);
  if (RCL_RET_OK != ret) {
    return ret;  // error is already set
  }
  *is_available = *is_available && (number_of_publishers != 0);

  ret = rcl_subscription_get_publisher_count(
    &(client->impl->status_subscription), &number_of_publishers);
  if (RCL_RET_OK != ret) {
    return ret;  // error is already set
  }
  *is_available = *is_available && (number_of_publishers != 0);

  return RCL_RET_OK;
}

// \internal Sends an action client specific service request.
#define SEND_SERVICE_REQUEST(Type, request, sequence_number) \
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Sending action " #Type " request"); \
  if (!rcl_action_client_is_valid(action_client)) { \
    return RCL_RET_ACTION_CLIENT_INVALID;  /* error already set */ \
  } \
  RCL_CHECK_ARGUMENT_FOR_NULL(request, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_ARGUMENT_FOR_NULL(sequence_number, RCL_RET_INVALID_ARGUMENT); \
  rcl_ret_t ret = rcl_send_request( \
    &action_client->impl->Type ## _client, request, sequence_number); \
  if (RCL_RET_OK != ret) { \
    return RCL_RET_ERROR;  /* error already set */ \
  } \
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action " #Type " request sent"); \
  return RCL_RET_OK;

// \internal Takes an action client specific service response.
#define TAKE_SERVICE_RESPONSE(Type, response_header, response) \
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action " #Type " response"); \
  if (!rcl_action_client_is_valid(action_client)) { \
    return RCL_RET_ACTION_CLIENT_INVALID;  /* error already set */ \
  } \
  RCL_CHECK_ARGUMENT_FOR_NULL(response_header, RCL_RET_INVALID_ARGUMENT); \
  RCL_CHECK_ARGUMENT_FOR_NULL(response, RCL_RET_INVALID_ARGUMENT); \
  rcl_ret_t ret = rcl_take_response( \
    &action_client->impl->Type ## _client, response_header, response); \
  if (RCL_RET_OK != ret) { \
    if (RCL_RET_BAD_ALLOC == ret) { \
      return RCL_RET_BAD_ALLOC;  /* error already set */ \
    } \
    if (RCL_RET_CLIENT_TAKE_FAILED == ret) { \
      return RCL_RET_ACTION_CLIENT_TAKE_FAILED; \
    } \
    return RCL_RET_ERROR;  /* error already set */ \
  } \
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action " #Type " response taken"); \
  return RCL_RET_OK;


rcl_ret_t
rcl_action_send_goal_request(
  const rcl_action_client_t * action_client,
  const void * ros_goal_request,
  int64_t * sequence_number)
{
  SEND_SERVICE_REQUEST(goal, ros_goal_request, sequence_number);
}

rcl_ret_t
rcl_action_take_goal_response(
  const rcl_action_client_t * action_client,
  rmw_request_id_t * response_header,
  void * ros_goal_response)
{
  TAKE_SERVICE_RESPONSE(goal, response_header, ros_goal_response);
}

rcl_ret_t
rcl_action_send_result_request(
  const rcl_action_client_t * action_client,
  const void * ros_result_request,
  int64_t * sequence_number)
{
  SEND_SERVICE_REQUEST(result, ros_result_request, sequence_number);
}

rcl_ret_t
rcl_action_take_result_response(
  const rcl_action_client_t * action_client,
  rmw_request_id_t * response_header,
  void * ros_result_response)
{
  TAKE_SERVICE_RESPONSE(result, response_header, ros_result_response);
}

rcl_ret_t
rcl_action_send_cancel_request(
  const rcl_action_client_t * action_client,
  const void * ros_cancel_request,
  int64_t * sequence_number)
{
  SEND_SERVICE_REQUEST(cancel, ros_cancel_request, sequence_number);
}

rcl_ret_t
rcl_action_take_cancel_response(
  const rcl_action_client_t * action_client,
  rmw_request_id_t * response_header,
  void * ros_cancel_response)
{
  TAKE_SERVICE_RESPONSE(cancel, response_header, ros_cancel_response);
}

// \internal Takes an action client specific topic message.
#define TAKE_MESSAGE(Type) \
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action " #Type); \
  if (!rcl_action_client_is_valid(action_client)) { \
    return RCL_RET_ACTION_CLIENT_INVALID;  /* error already set */ \
  } \
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_ ## Type, RCL_RET_INVALID_ARGUMENT); \
  rmw_message_info_t message_info; /* ignored */ \
  rcl_ret_t ret = rcl_take( \
    &action_client->impl->Type ## _subscription, ros_ ## Type, &message_info, NULL); \
  if (RCL_RET_OK != ret) { \
    if (RCL_RET_SUBSCRIPTION_TAKE_FAILED == ret) { \
      return RCL_RET_ACTION_CLIENT_TAKE_FAILED; \
    } \
    if (RCL_RET_BAD_ALLOC == ret) { \
      return RCL_RET_BAD_ALLOC; \
    } \
    return RCL_RET_ERROR; \
  } \
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action " #Type " taken"); \
  return RCL_RET_OK;

rcl_ret_t
rcl_action_take_feedback(
  const rcl_action_client_t * action_client,
  void * ros_feedback)
{
  TAKE_MESSAGE(feedback);
}

rcl_ret_t
rcl_action_take_status(
  const rcl_action_client_t * action_client,
  void * ros_status)
{
  TAKE_MESSAGE(status);
}

const char *
rcl_action_client_get_action_name(const rcl_action_client_t * action_client)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return NULL;
  }
  return action_client->impl->remapped_action_name;
}

const rcl_action_client_options_t *
rcl_action_client_get_options(const rcl_action_client_t * action_client)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return NULL;
  }
  return &action_client->impl->options;
}

bool
rcl_action_client_is_valid(const rcl_action_client_t * action_client)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(
    action_client, "action client pointer is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    action_client->impl, "action client implementation is invalid", return false);
  if (!rcl_client_is_valid(&action_client->impl->goal_client)) {
    rcl_reset_error();
    RCL_SET_ERROR_MSG("goal client is invalid");
    return false;
  }
  if (!rcl_client_is_valid(&action_client->impl->cancel_client)) {
    rcl_reset_error();
    RCL_SET_ERROR_MSG("cancel client is invalid");
    return false;
  }
  if (!rcl_client_is_valid(&action_client->impl->result_client)) {
    rcl_reset_error();
    RCL_SET_ERROR_MSG("result client is invalid");
    return false;
  }
  if (!rcl_subscription_is_valid(&action_client->impl->feedback_subscription)) {
    rcl_reset_error();
    RCL_SET_ERROR_MSG("feedback subscription is invalid");
    return false;
  }
  if (!rcl_subscription_is_valid(&action_client->impl->status_subscription)) {
    rcl_reset_error();
    RCL_SET_ERROR_MSG("status subscription is invalid");
    return false;
  }
  return true;
}

rcl_ret_t
rcl_action_wait_set_add_action_client(
  rcl_wait_set_t * wait_set,
  const rcl_action_client_t * action_client,
  size_t * client_index,
  size_t * subscription_index)
{
  rcl_ret_t ret;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_WAIT_SET_INVALID);
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }

  // Wait on action goal service response messages.
  ret = rcl_wait_set_add_client(
    wait_set,
    &action_client->impl->goal_client,
    &action_client->impl->wait_set_goal_client_index);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action cancel service response messages.
  ret = rcl_wait_set_add_client(
    wait_set,
    &action_client->impl->cancel_client,
    &action_client->impl->wait_set_cancel_client_index);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action result service response messages.
  ret = rcl_wait_set_add_client(
    wait_set,
    &action_client->impl->result_client,
    &action_client->impl->wait_set_result_client_index);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action feedback messages.
  ret = rcl_wait_set_add_subscription(
    wait_set,
    &action_client->impl->feedback_subscription,
    &action_client->impl->wait_set_feedback_subscription_index);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action status messages.
  ret = rcl_wait_set_add_subscription(
    wait_set,
    &action_client->impl->status_subscription,
    &action_client->impl->wait_set_status_subscription_index);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  if (NULL != client_index) {
    // The goal client was the first added
    *client_index = action_client->impl->wait_set_goal_client_index;
  }
  if (NULL != subscription_index) {
    // The feedback subscription was the first added
    *subscription_index = action_client->impl->wait_set_feedback_subscription_index;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_client_wait_set_get_num_entities(
  const rcl_action_client_t * action_client,
  size_t * num_subscriptions,
  size_t * num_guard_conditions,
  size_t * num_timers,
  size_t * num_clients,
  size_t * num_services)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(num_subscriptions, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(num_guard_conditions, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(num_timers, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(num_clients, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(num_services, RCL_RET_INVALID_ARGUMENT);
  *num_subscriptions = 2;
  *num_guard_conditions = 0;
  *num_timers = 0;
  *num_clients = 3;
  *num_services = 0;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_client_wait_set_get_entities_ready(
  const rcl_wait_set_t * wait_set,
  const rcl_action_client_t * action_client,
  bool * is_feedback_ready,
  bool * is_status_ready,
  bool * is_goal_response_ready,
  bool * is_cancel_response_ready,
  bool * is_result_response_ready)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_WAIT_SET_INVALID);
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(is_feedback_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_status_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_goal_response_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_cancel_response_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_result_response_ready, RCL_RET_INVALID_ARGUMENT);

  const rcl_action_client_impl_t * impl = action_client->impl;
  const size_t feedback_index = impl->wait_set_feedback_subscription_index;
  const size_t status_index = impl->wait_set_status_subscription_index;
  const size_t goal_index = impl->wait_set_goal_client_index;
  const size_t cancel_index = impl->wait_set_cancel_client_index;
  const size_t result_index = impl->wait_set_result_client_index;
  if (feedback_index >= wait_set->size_of_subscriptions) {
    RCL_SET_ERROR_MSG("wait set index for feedback subscription is out of bounds");
    return RCL_RET_ERROR;
  }
  if (status_index >= wait_set->size_of_subscriptions) {
    RCL_SET_ERROR_MSG("wait set index for status subscription is out of bounds");
    return RCL_RET_ERROR;
  }
  if (goal_index >= wait_set->size_of_clients) {
    RCL_SET_ERROR_MSG("wait set index for goal client is out of bounds");
    return RCL_RET_ERROR;
  }
  if (cancel_index >= wait_set->size_of_clients) {
    RCL_SET_ERROR_MSG("wait set index for cancel client is out of bounds");
    return RCL_RET_ERROR;
  }
  if (result_index >= wait_set->size_of_clients) {
    RCL_SET_ERROR_MSG("wait set index for result client is out of bounds");
    return RCL_RET_ERROR;
  }

  const rcl_subscription_t * feedback_subscription = wait_set->subscriptions[feedback_index];
  const rcl_subscription_t * status_subscription = wait_set->subscriptions[status_index];
  const rcl_client_t * goal_client = wait_set->clients[goal_index];
  const rcl_client_t * cancel_client = wait_set->clients[cancel_index];
  const rcl_client_t * result_client = wait_set->clients[result_index];
  *is_feedback_ready = (&impl->feedback_subscription == feedback_subscription);
  *is_status_ready = (&impl->status_subscription == status_subscription);
  *is_goal_response_ready = (&impl->goal_client == goal_client);
  *is_cancel_response_ready = (&impl->cancel_client == cancel_client);
  *is_result_response_ready = (&impl->result_client == result_client);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_client_set_goal_client_callback(
  const rcl_action_client_t * action_client,
  rcl_event_callback_t callback,
  const void * user_data)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  return rcl_client_set_on_new_response_callback(
    &action_client->impl->goal_client,
    callback,
    user_data);
}

rcl_ret_t
rcl_action_client_set_cancel_client_callback(
  const rcl_action_client_t * action_client,
  rcl_event_callback_t callback,
  const void * user_data)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  return rcl_client_set_on_new_response_callback(
    &action_client->impl->cancel_client,
    callback,
    user_data);
}

rcl_ret_t
rcl_action_client_set_result_client_callback(
  const rcl_action_client_t * action_client,
  rcl_event_callback_t callback,
  const void * user_data)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  return rcl_client_set_on_new_response_callback(
    &action_client->impl->result_client,
    callback,
    user_data);
}

rcl_ret_t
rcl_action_client_set_feedback_subscription_callback(
  const rcl_action_client_t * action_client,
  rcl_event_callback_t callback,
  const void * user_data)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  return rcl_subscription_set_on_new_message_callback(
    &action_client->impl->feedback_subscription,
    callback,
    user_data);
}

rcl_ret_t
rcl_action_client_set_status_subscription_callback(
  const rcl_action_client_t * action_client,
  rcl_event_callback_t callback,
  const void * user_data)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  return rcl_subscription_set_on_new_message_callback(
    &action_client->impl->status_subscription,
    callback,
    user_data);
}

#define CLIENT_CONFIGURE_SERVICE_INTROSPECTION(TYPE, STATE) \
  if (rcl_client_configure_service_introspection( \
      &action_client->impl->TYPE ## _client, \
      node, \
      clock, \
      type_support->TYPE ## _service_type_support, \
      publisher_options, \
      STATE) != RCL_RET_OK) \
  { \
    return RCL_RET_ERROR; \
  }


rcl_ret_t
rcl_action_client_configure_action_introspection(
  rcl_action_client_t * action_client,
  rcl_node_t * node,
  rcl_clock_t * clock,
  const rosidl_action_type_support_t * type_support,
  const rcl_publisher_options_t publisher_options,
  rcl_service_introspection_state_t introspection_state)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(clock, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);

  CLIENT_CONFIGURE_SERVICE_INTROSPECTION(goal, introspection_state);
  CLIENT_CONFIGURE_SERVICE_INTROSPECTION(cancel, introspection_state);
  CLIENT_CONFIGURE_SERVICE_INTROSPECTION(result, introspection_state);
  return RCL_RET_OK;
}

/// \internal
/// Converts a goal ID array (uint8_t) to an array of strings.
static
rcl_ret_t
_goal_id_to_string_array(
  const rcl_allocator_t * allocator,
  const uint8_t * goal_id_array,
  size_t array_size,
  char ** goal_id_string_memory_block,
  char ** goal_id_str_array)
{
  // A uint8_t converted to a string occupies at most 4 bytes.
  char * goal_id_array_str = (char *)allocator->allocate(4 * array_size, allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    goal_id_array_str, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  for (size_t i = 0; i < array_size; ++i) {
    int n = snprintf(
      &goal_id_array_str[i * 4], 4, "%u", goal_id_array[i]); // NOLINT
    if (n < 0 || n >= 4) {
      goto err;
    }
    goal_id_str_array[i] = &goal_id_array_str[i * 4];
  }

  *goal_id_string_memory_block = goal_id_array_str;
  return RCL_RET_OK;

err:
  if (goal_id_array_str != NULL) {
    allocator->deallocate(goal_id_array_str, allocator->state);
  }

  return RCL_RET_ERROR;
}

/// \internal
/// Generates a filter expression string for goal ID array.
/// \param[in] goal_id_array_size The size of goal ID array.
/// \param[out] filter_expression The generated filter expression string. The caller is responsible
///             for releasing this string.
/// \param[in] allocator The allocator to use for memory allocation.
/// \return `RCL_RET_OK` if successful, or an error code otherwise.
static
rcl_ret_t
_generate_goal_id_filter_expression(
  size_t goal_id_array_size,
  char ** filter_expression,
  rcl_allocator_t * allocator)
{
  if (goal_id_array_size == 0 || goal_id_array_size % UUID_SIZE != 0 || allocator == NULL) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  // Each clause looks like: "goal_id.uuid[XX] = %XX"
  // Breakdown of maximum length:
  //   - "goal_id.uuid["            : 13 chars
  //   - index "XX" (0-15)          :  2 chars max
  //   - "] = "                     :  4 chars
  //   - parameter "%XX" (0-99)     :  4 chars max
  //   => "goal_id.uuid[XX] = %XX"  : 23 chars max
  // When multiple UUID elements are combined, we also add " AND " (5 chars) between clauses.
  // So per element we need up to 23 + 5 = 28 chars. We round this up to 30 bytes to include
  // a small safety margin and help cover separators such as ") OR (" between goal ID arrays.
  int expression_capacity = goal_id_array_size * 30;
  char * expression_goal_id_arrays = (char *)allocator->allocate(
    expression_capacity, allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    expression_goal_id_arrays, "allocating memory failed", return RCL_RET_BAD_ALLOC);

  int pos = 0;
  size_t num_arrays = goal_id_array_size / UUID_SIZE;

  // Helper macro to append formatted string and check for errors
  #define APPEND_TO_EXPRESSION(...) \
    do { \
      int written = snprintf( \
        expression_goal_id_arrays + pos, expression_capacity - pos, __VA_ARGS__); \
      if (written < 0 || \
        written >= expression_capacity - pos || pos + written >= expression_capacity) { \
        allocator->deallocate(expression_goal_id_arrays, allocator->state); \
        return RCL_RET_ERROR; \
      } \
      pos += written; \
    } while (0)

  // Traverse each goal ID array
  for (size_t i = 0; i < num_arrays; i++) {
    if (i > 0) {
      // Not the first goal ID array, add a separator
      APPEND_TO_EXPRESSION(") OR (");
    } else if (num_arrays > 1) {
      // First array: if there are multiple arrays, add an opening parenthesis
      APPEND_TO_EXPRESSION("(");
    }

    // Generate expression for one goal ID array
    for (int j = 0; j < UUID_SIZE; j++) {
      if (j > 0) {
        APPEND_TO_EXPRESSION(" AND ");
      }
      // goal_id.uuid[XX]: the index range of the UUID array is 0-15
      // The parameter index is global and the index range is 0-99 (Since DDS spec requires
      // less than 100 parameters in a filter expression)
      int param_index = i * UUID_SIZE + j;
      APPEND_TO_EXPRESSION("goal_id.uuid[%d] = %%%d", j, param_index);
    }
  }

  // Add the closing parenthesis at the end
  if (num_arrays > 1) {
    APPEND_TO_EXPRESSION(")");
  }

  #undef APPEND_TO_EXPRESSION

  *filter_expression = expression_goal_id_arrays;

  return RCL_RET_OK;
}

static
void
_clear_setting_content_filter_on_error(rcl_subscription_t * feedback_subscription)
{
  rcl_subscription_content_filter_options_t content_filter_options =
    rcl_get_zero_initialized_subscription_content_filter_options();

  rcl_ret_t ret = rcl_subscription_content_filter_options_init(
    feedback_subscription,
    "",
    0,
    NULL,
    &content_filter_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to initialize cft options to clear existing filter");
    return;
  }

  // Update content filter options
  ret = rcl_subscription_set_content_filter(
    feedback_subscription,
    &content_filter_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to clear existing cft expression parameters");
  }

  ret = rcl_subscription_content_filter_options_fini(
    feedback_subscription, &content_filter_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to finalize cft options");
  }
}

rcl_ret_t
rcl_action_client_configure_feedback_subscription_filter_add_goal_id(
  const rcl_action_client_t * action_client,
  const uint8_t * goal_id_array,
  size_t array_size)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  RCL_CHECK_ARGUMENT_FOR_NULL(goal_id_array, RCL_RET_INVALID_ARGUMENT);
  if (array_size != UUID_SIZE) {
    RCL_SET_ERROR_MSG("Goal id array size must be equal to UUID_SIZE.");
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (!rcl_subscription_is_cft_supported(&action_client->impl->feedback_subscription)) {
    RCL_SET_ERROR_MSG("RMW implementation does not support content filter.");
    return RCL_RET_UNSUPPORTED;
  }

  if (action_client->impl->disable_feedback_sub_cft) {
    RCL_SET_ERROR_MSG("Content filter has been disabled for feedback subscription.");
    return RCL_RET_ERROR;
  }

  // Converts goal ID array (uint8_t) to an array of strings.
  char * goal_id_string_memory_block = NULL;
  char * goal_id_str_array[UUID_SIZE];
  rcl_ret_t ret = _goal_id_to_string_array(
    &action_client->impl->options.allocator,
    goal_id_array,
    array_size,
    &goal_id_string_memory_block,
    goal_id_str_array);
  if (RCL_RET_OK != ret) {
    return RCL_RET_ERROR;
  }

  bool is_cft_enabled =
    rcl_subscription_is_cft_enabled(&action_client->impl->feedback_subscription);

  rcl_subscription_content_filter_options_t content_filter_options =
    rcl_get_zero_initialized_subscription_content_filter_options();

  size_t existing_expression_params_size = 0;
  char ** existing_expression_params = NULL;
  char ** new_expression_params = NULL;
  char * new_filter_expression = NULL;

  ret = RCL_RET_ERROR;

  if (is_cft_enabled) {
    // Existing content filter and append new goal ID to it.
    ret = rcl_subscription_get_content_filter(
        &action_client->impl->feedback_subscription, &content_filter_options);
    if (RCL_RET_OK != ret) {
      RCL_SET_ERROR_MSG("Failed to get cft expression parameters");
      goto err;
    }
    existing_expression_params_size = content_filter_options
      .rmw_subscription_content_filter_options.expression_parameters.size;
    existing_expression_params = content_filter_options
      .rmw_subscription_content_filter_options.expression_parameters.data;
  }

  size_t new_expression_params_size =
    existing_expression_params_size + array_size;
  if (new_expression_params_size > 100) {
    RCL_SET_ERROR_MSG("Exceeded maximum number of filter expression parameters (100)");
    ret = RCL_RET_ERROR;
    goto err;
  }

  new_expression_params = (char **)action_client->impl->options.allocator.allocate(
    sizeof(char *) * new_expression_params_size, action_client->impl->options.allocator.state);
  if (new_expression_params == NULL) {
    RCL_SET_ERROR_MSG("Failed to allocate memory for expression parameters");
    ret = RCL_RET_BAD_ALLOC;
    goto err;
  }

  // Collect existing expression parameters
  for (size_t i = 0; i < existing_expression_params_size; ++i) {
    new_expression_params[i] = existing_expression_params[i];
  }
  // Append new goal ID array
  for (size_t i = 0; i < array_size; ++i) {
    new_expression_params[existing_expression_params_size + i] = goal_id_str_array[i];
  }

  // Generate new filter expression string
  if (_generate_goal_id_filter_expression(
      new_expression_params_size,
      &new_filter_expression,
      &action_client->impl->options.allocator) != RCL_RET_OK)
  {
    RCL_SET_ERROR_MSG("Failed to generate new filter expression");
    goto err;
  }

  // Update content filter options
  rcl_subscription_content_filter_options_t new_content_filter_options =
    rcl_get_zero_initialized_subscription_content_filter_options();
  ret = rcl_subscription_content_filter_options_init(
    &action_client->impl->feedback_subscription,
    new_filter_expression,
    new_expression_params_size,
    (const char **)new_expression_params,
    &new_content_filter_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to initialize cft options");
    goto err;
  }

  // Set new content filter options
  ret = rcl_subscription_set_content_filter(
    &action_client->impl->feedback_subscription,
    &new_content_filter_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to set cft expression parameters");
  }

  rcl_ret_t fini_ret = rcl_subscription_content_filter_options_fini(
    &action_client->impl->feedback_subscription, &new_content_filter_options);
  if (RCL_RET_OK != fini_ret) {
    RCL_SET_ERROR_MSG("Failed to finalize cft options");
  }

err:
  if (RCL_RET_OK != ret) {
    // Clear existing content filter and disable content filter for feedback subscription
    action_client->impl->disable_feedback_sub_cft = true;
    _clear_setting_content_filter_on_error(&action_client->impl->feedback_subscription);
  }

  if (new_expression_params != NULL) {
    action_client->impl->options.allocator.deallocate(
      new_expression_params, action_client->impl->options.allocator.state);
  }

  if (goal_id_string_memory_block != NULL) {
    action_client->impl->options.allocator.deallocate(
      goal_id_string_memory_block, action_client->impl->options.allocator.state);
  }

  if (new_filter_expression != NULL) {
    action_client->impl->options.allocator.deallocate(
      new_filter_expression, action_client->impl->options.allocator.state);
  }

  fini_ret = rcl_subscription_content_filter_options_fini(
    &action_client->impl->feedback_subscription, &content_filter_options);
  if (RCL_RET_OK != fini_ret) {
    RCL_SET_ERROR_MSG("Failed to finalize cft options");
  }

  return ret;
}

rcl_ret_t
rcl_action_client_configure_feedback_subscription_filter_remove_goal_id(
  const rcl_action_client_t * action_client,
  const uint8_t * goal_id_array,
  size_t array_size)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;
  }

  RCL_CHECK_ARGUMENT_FOR_NULL(goal_id_array, RCL_RET_INVALID_ARGUMENT);
  if (array_size != UUID_SIZE) {
    RCL_SET_ERROR_MSG("Goal id array size must be equal to UUID_SIZE.");
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (!rcl_subscription_is_cft_supported(&action_client->impl->feedback_subscription)) {
    RCL_SET_ERROR_MSG("RMW implementation does not support content filter.");
    return RCL_RET_UNSUPPORTED;
  }

  if (action_client->impl->disable_feedback_sub_cft) {
    RCL_SET_ERROR_MSG("Content filter has been disabled for feedback subscription.");
    return RCL_RET_ERROR;
  }

  // If content filter isn't configured, do nothing.
  bool is_cft_enabled =
    rcl_subscription_is_cft_enabled(&action_client->impl->feedback_subscription);
  if (!is_cft_enabled) {
    return RCL_RET_OK;
  }

  // Converts goal ID array (uint8_t) to an array of strings.
  char * goal_id_string_memory_block = NULL;
  char * goal_id_str_array[UUID_SIZE];
  rcl_ret_t ret = _goal_id_to_string_array(
    &action_client->impl->options.allocator,
    goal_id_array,
    array_size,
    &goal_id_string_memory_block,
    goal_id_str_array);
  if (RCL_RET_OK != ret) {
    return RCL_RET_ERROR;
  }

  // Get existing content filter options
  rcl_subscription_content_filter_options_t content_filter_options =
    rcl_get_zero_initialized_subscription_content_filter_options();
  ret = rcl_subscription_get_content_filter(
      &action_client->impl->feedback_subscription, &content_filter_options);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("Failed to get cft expression parameters");
    return ret;
  }

  char * new_filter_expression = NULL;
  char ** new_expression_params = NULL;

  // Check if goal ID is in expression_parameters in content_filter_options
  size_t expression_params_size = content_filter_options
    .rmw_subscription_content_filter_options.expression_parameters.size;
  char ** expression_params =
    content_filter_options.rmw_subscription_content_filter_options.expression_parameters.data;
  if (expression_params_size % array_size != 0u) {
    RCL_SET_ERROR_MSG("Expression parameters size is not divisible by goal ID array size");
    ret = RCL_RET_ERROR;
    goto err;
  }
  size_t index_expression_params = 0;
  while (index_expression_params < expression_params_size) {
    // one_goal_id_array points to the start of one goal ID array in expression_parameters
    char ** one_goal_id_array = &expression_params[index_expression_params];
    bool is_matched = true;
    for (size_t i = 0; i < array_size; ++i) {
      if (strcmp(one_goal_id_array[i], goal_id_str_array[i]) != 0) {
        is_matched = false;
        break;
      }
    }
    if (is_matched) {
      break;
    }
    index_expression_params += array_size;
  }

  // If found the goal ID in content filter, remove it
  if (index_expression_params != expression_params_size) {
    // Prepare new content filter options
    rcl_subscription_content_filter_options_t new_content_filter_options =
      rcl_get_zero_initialized_subscription_content_filter_options();

    size_t new_expression_params_size = expression_params_size - array_size;
    if (new_expression_params_size == 0) {
      // All goal IDs are removed, disable content filter
      // Generate empty filter expression options
      ret = rcl_subscription_content_filter_options_init(
        &action_client->impl->feedback_subscription,
        "",
        0,
        NULL,
        &new_content_filter_options);
      if (RCL_RET_OK != ret) {
        RCL_SET_ERROR_MSG("Failed to initialize cft options");
        goto err;
      }
    } else {
      // Create new expression parameters array without the removed goal ID
      // DDS spec requires less than 100 parameters in a filter expression, so
      // new_expression_params_size is guaranteed to be less than 100.
      new_expression_params =
        (char **)action_client->impl->options.allocator.allocate(
          sizeof(char *) * new_expression_params_size,
          action_client->impl->options.allocator.state);
      if (new_expression_params == NULL) {
        RCL_SET_ERROR_MSG("Failed to allocate memory for expression parameters");
        ret = RCL_RET_BAD_ALLOC;
        goto err;
      }
      size_t new_index = 0;
      for (size_t i = 0; i < expression_params_size; ++i) {
        if (i >= index_expression_params && i < (index_expression_params + array_size)) {
          // Skip the goal ID to be removed
          continue;
        }
        new_expression_params[new_index++] = expression_params[i];
      }

      // Generate new filter expression string
      ret = _generate_goal_id_filter_expression(
        (expression_params_size - array_size),
        &new_filter_expression,
        &action_client->impl->options.allocator);
      if (RCL_RET_OK != ret) {
        RCL_SET_ERROR_MSG("Failed to generate new filter expression");
        goto err;
      }

      // Generate new filter expression options
      ret = rcl_subscription_content_filter_options_init(
        &action_client->impl->feedback_subscription,
        new_filter_expression,
        expression_params_size - array_size,
        (const char **)new_expression_params,
        &new_content_filter_options);
      if (RCL_RET_OK != ret) {
        RCL_SET_ERROR_MSG("Failed to initialize cft options");
        goto err;
      }
    }

    // Update new content filter options
    ret = rcl_subscription_set_content_filter(
      &action_client->impl->feedback_subscription,
      &new_content_filter_options);
    if (RCL_RET_OK != ret) {
      RCL_SET_ERROR_MSG("Failed to set cft expression parameters");
    }

    rcl_ret_t fini_ret = rcl_subscription_content_filter_options_fini(
      &action_client->impl->feedback_subscription, &new_content_filter_options);
    if (RCL_RET_OK != fini_ret) {
      RCL_SET_ERROR_MSG("Failed to finalize cft options");
    }
  }

err:
  if (NULL != new_expression_params) {
    action_client->impl->options.allocator.deallocate(
      new_expression_params, action_client->impl->options.allocator.state);
  }

  if (RCL_RET_OK != ret) {
    // Clear existing content filter and disable content filter for feedback subscription
    action_client->impl->disable_feedback_sub_cft = true;
    _clear_setting_content_filter_on_error(&action_client->impl->feedback_subscription);
  }

  if (goal_id_string_memory_block != NULL) {
    action_client->impl->options.allocator.deallocate(
      goal_id_string_memory_block, action_client->impl->options.allocator.state);
  }

  if (new_filter_expression != NULL) {
    action_client->impl->options.allocator.deallocate(
      new_filter_expression, action_client->impl->options.allocator.state);
  }

  rcl_ret_t fini_ret = rcl_subscription_content_filter_options_fini(
    &action_client->impl->feedback_subscription, &content_filter_options);
  if (RCL_RET_OK != fini_ret) {
    RCL_SET_ERROR_MSG("Failed to finalize cft options");
  }

  return ret;
}
#ifdef __cplusplus
}
#endif
