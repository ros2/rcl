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
#include "rcl/subscription.h"
#include "rcl/types.h"
#include "rcl/wait.h"

#include "rcutils/logging_macros.h"
#include "rcutils/format_string.h"
#include "rcutils/strdup.h"

#include "rmw/qos_profiles.h"
#include "rmw/types.h"


rcl_action_client_t
rcl_action_get_zero_initialized_client(void)
{
  static rcl_action_client_t null_action_client = {0};
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
    rcutils_get_zero_initialized_hash_map()
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
  allocator.deallocate(action_client->impl->action_name, allocator.state);
  if (NULL != action_client->impl->goal_uuids.impl) {
    uint8_t uuid[UUID_SIZE];
    char * value = NULL;
    rcl_allocator_t default_allocator = rcl_get_default_allocator();
    rcutils_ret_t hashmap_ret = rcutils_hash_map_get_next_key_and_data(
      &action_client->impl->goal_uuids, NULL, uuid, &value);
    while (RCUTILS_RET_OK == hashmap_ret) {
      RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "remove a uuid: %s", value);
      default_allocator.deallocate(value, default_allocator.state);
      RCL_RET_FROM_RCUTIL_RET(
        ret, rcutils_hash_map_unset(&action_client->impl->goal_uuids, uuid));
      if (ret == RCL_RET_OK) {
        hashmap_ret = rcutils_hash_map_get_next_key_and_data(
          &action_client->impl->goal_uuids, NULL, uuid, &value);
      }
    }
    if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES != hashmap_ret) {
      RCL_RET_FROM_RCUTIL_RET(ret, hashmap_ret);
    }
    if (RCL_RET_OK == ret) {
      RCL_RET_FROM_RCUTIL_RET(ret, rcutils_hash_map_fini(&action_client->impl->goal_uuids));
    }
  }

  allocator.deallocate(action_client->impl, allocator.state);
  action_client->impl = NULL;
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action client finalized");
  return ret;
}

// \internal Initializes an action client specific service client.
#define CLIENT_INIT(Type) \
  char * Type ## _service_name = NULL; \
  ret = rcl_action_get_ ## Type ## _service_name(action_name, allocator, &Type ## _service_name); \
  if (RCL_RET_OK != ret) { \
    rcl_reset_error(); \
    RCL_SET_ERROR_MSG("failed to get " #Type " service name"); \
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
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
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else if (RCL_RET_SERVICE_NAME_INVALID == ret) { \
      ret = RCL_RET_ACTION_NAME_INVALID; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
    goto fail; \
  }

// \internal Initializes an action client specific topic subscription.
#define SUBSCRIPTION_INIT(Type) \
  char * Type ## _topic_name = NULL; \
  ret = rcl_action_get_ ## Type ## _topic_name(action_name, allocator, &Type ## _topic_name); \
  if (RCL_RET_OK != ret) { \
    rcl_reset_error(); \
    RCL_SET_ERROR_MSG("failed to get " #Type " topic name"); \
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
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
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else if (RCL_RET_TOPIC_NAME_INVALID == ret) { \
      ret = RCL_RET_ACTION_NAME_INVALID; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
    goto fail; \
  }

size_t hash_map_uuid_hash_func(const void * uuid)
{
  const uint8_t * ckey_str = (const uint8_t *) uuid;
  size_t hash = 5381;

  for (size_t i = 0; i < UUID_SIZE; ++i) {
    const char c = *(ckey_str++);
    hash = ((hash << 5) + hash) + (size_t)c; /* hash * 33 + c */
  }

  return hash;
}

int hash_map_uuid_cmp_func(const void * val1, const void * val2)
{
  const uint8_t * cval1 = (const uint8_t *)val1;
  const uint8_t * cval2 = (const uint8_t *)val2;
  return memcmp(cval1, cval2, UUID_SIZE);
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
  // Copy action client name and options.
  action_client->impl->action_name = rcutils_strdup(action_name, allocator);
  if (NULL == action_client->impl->action_name) {
    RCL_SET_ERROR_MSG("failed to duplicate action name");
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }
  action_client->impl->options = *options;

  // Initialize action service clients.
  CLIENT_INIT(goal);
  CLIENT_INIT(cancel);
  CLIENT_INIT(result);

  // Initialize action topic subscriptions.
  SUBSCRIPTION_INIT(feedback);
  SUBSCRIPTION_INIT(status);

  // Initialize goal_uuids map
  RCL_RET_FROM_RCUTIL_RET(
    ret, rcutils_hash_map_init(
      &action_client->impl->goal_uuids, 2, sizeof(uint8_t[16]), sizeof(const char **),
      hash_map_uuid_hash_func, hash_map_uuid_cmp_func, &allocator));
  if (RCL_RET_OK != ret) {
    goto fail;
  }

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
  static rcl_action_client_options_t default_options;
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
  return action_client->impl->action_name;
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

static char *
to_uuid_string(const uint8_t * uuid, rcl_allocator_t allocator)
{
  char * uuid_str = rcutils_format_string(allocator, UUID_FMT, UUID_FMT_ARGS(uuid));
  return uuid_str;
}

static
rcl_ret_t set_content_filtered_topic(
  const rcl_action_client_t * action_client)
{
  rcl_ret_t ret;
  rcutils_ret_t rcutils_ret;
  uint8_t uuid[UUID_SIZE];
  char * uuid_str = NULL;
  size_t size;
  char * feedback_filter = NULL;
  char * feedback_filter_update = NULL;
  rcl_allocator_t allocator = rcl_get_default_allocator();

  feedback_filter = rcutils_strdup("", allocator);
  if (NULL == feedback_filter) {
    RCL_SET_ERROR_MSG("failed to allocate memory for feedback filter string");
    ret = RCL_RET_BAD_ALLOC;
    goto clean;
  }

  RCL_RET_FROM_RCUTIL_RET(
    ret, rcutils_hash_map_get_size(&action_client->impl->goal_uuids, &size));
  if (RCL_RET_OK != ret) {
    goto clean;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "size: %zu", size);
  if (0 == size) {
    // mainly to reset content filtered topic with empty string
    goto set_cft;
  }

  RCL_RET_FROM_RCUTIL_RET(
    ret, rcutils_hash_map_get_next_key_and_data(
      &action_client->impl->goal_uuids, NULL, uuid, &uuid_str));
  if (RCL_RET_OK != ret) {
    goto clean;
  }

  feedback_filter_update =
    rcutils_format_string(allocator, "goal_id.uuid = &hex(%s)", uuid_str);
  if (NULL == feedback_filter_update) {
    RCL_SET_ERROR_MSG("failed to format string for feedback filter");
    ret = RCL_RET_BAD_ALLOC;
    goto clean;
  }
  allocator.deallocate(feedback_filter, allocator.state);
  feedback_filter = feedback_filter_update;

  while (RCUTILS_RET_OK == (rcutils_ret = rcutils_hash_map_get_next_key_and_data(
      &action_client->impl->goal_uuids, uuid, uuid, &uuid_str)))
  {
    feedback_filter_update = rcutils_format_string(
      allocator, "%s or goal_id.uuid = &hex(%s)", feedback_filter, uuid_str);
    if (NULL == feedback_filter_update) {
      RCL_SET_ERROR_MSG("failed to format string for feedback filter");
      ret = RCL_RET_BAD_ALLOC;
      goto clean;
    }
    allocator.deallocate(feedback_filter, allocator.state);
    feedback_filter = feedback_filter_update;
  }
  if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES != rcutils_ret) {
    RCL_RET_FROM_RCUTIL_RET(ret, rcutils_ret);
  }
  if (RCL_RET_OK != ret) {
    goto clean;
  }

set_cft:

  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "feedback_filter: %s", feedback_filter);
  ret = rcl_subscription_set_cft_expression_parameters(
    &action_client->impl->feedback_subscription, feedback_filter, NULL);
  if (RCL_RET_OK != ret) {
    goto clean;
  }

  // TODO(iuhilnehc-ynos) set status_subscription with filter expression for the sequence
  // by wildcard if rti_connext_dds support this feature in the future,
  // otherwise, it seems it's necessary to update the action message type or something else.

clean:

  allocator.deallocate(feedback_filter, allocator.state);
  return ret;
}

rcl_ret_t rcl_action_add_goal_uuid(
  const rcl_action_client_t * action_client,
  const uint8_t * uuid)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  /* error already set */
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(uuid, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;
  char * uuid_str = NULL;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  uuid_str = to_uuid_string(uuid, allocator);
  if (NULL == uuid_str) {
    RCL_SET_ERROR_MSG("failed to allocate memory for uuid value");
    ret = RCL_RET_BAD_ALLOC;
    goto end;
  }

  RCL_RET_FROM_RCUTIL_RET(
    ret, rcutils_hash_map_set(&action_client->impl->goal_uuids, uuid, &uuid_str));
  if (RCL_RET_OK != ret) {
    goto clean;
  }

  // to set content filtered topic
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "set content filtered topic after adding a uuid: %s", uuid_str);
  ret = set_content_filtered_topic(action_client);
  if (RCL_RET_OK != ret) {
    char * err = rcutils_strdup(rcl_get_error_string().str, allocator);
    if (NULL == err) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "failed to allocate memory for error");
      ret = RCL_RET_BAD_ALLOC;
      goto clean;
    }
    rcl_reset_error();
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("failed to set_content_filtered_topic: %s", err);
    allocator.deallocate(err, allocator.state);
  }
  goto end;

clean:
  allocator.deallocate(uuid_str, allocator.state);

end:
  return ret;
}

rcl_ret_t rcl_action_remove_goal_uuid(
  const rcl_action_client_t * action_client,
  const uint8_t * uuid)
{
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  /* error already set */
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(uuid, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;
  char * uuid_str = NULL;
  char * uuid_value = NULL;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  uuid_str = to_uuid_string(uuid, allocator);
  if (NULL == uuid_str) {
    RCL_SET_ERROR_MSG("failed to allocate memory for uuid value");
    ret = RCL_RET_BAD_ALLOC;
    goto end;
  }
  if (!rcutils_hash_map_key_exists(&action_client->impl->goal_uuids, uuid)) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "item key [%s] not found in the map of goal uuids",
      uuid_str);
    ret = RCL_RET_ERROR;
    goto end;
  }

  RCL_RET_FROM_RCUTIL_RET(
    ret, rcutils_hash_map_get(&action_client->impl->goal_uuids, uuid, &uuid_value));
  if (RCL_RET_OK != ret) {
    goto end;
  }
  allocator.deallocate(uuid_value, allocator.state);

  RCL_RET_FROM_RCUTIL_RET(
    ret, rcutils_hash_map_unset(&action_client->impl->goal_uuids, uuid));
  if (RCL_RET_OK != ret) {
    goto end;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "set content filtered topic after removing a uuid: %s", uuid_str);
  ret = set_content_filtered_topic(action_client);
  if (RCL_RET_OK != ret) {
    rcl_allocator_t allocator = rcl_get_default_allocator();
    char * err = rcutils_strdup(rcl_get_error_string().str, allocator);
    if (NULL == err) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "failed to allocate memory for error");
      ret = RCL_RET_BAD_ALLOC;
      goto end;
    }
    rcl_reset_error();
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("failed to set_content_filtered_topic: %s", err);
    allocator.deallocate(err, allocator.state);
  }

end:
  allocator.deallocate(uuid_str, allocator.state);
  return ret;
}

#ifdef __cplusplus
}
#endif
