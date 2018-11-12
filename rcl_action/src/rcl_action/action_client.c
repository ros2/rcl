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

#include "rcl_action/default_qos.h"
#include "rcl_action/names.h"
#include "rcl_action/types.h"
#include "rcl_action/wait.h"

#include "rcl/client.h"
#include "rcl/error_handling.h"
#include "rcl/subscription.h"
#include "rcl/types.h"
#include "rcl/wait.h"

#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"

#include "rmw/qos_profiles.h"
#include "rmw/types.h"


typedef struct rcl_action_client_impl_t
{
  rcl_client_t goal_client;
  rcl_client_t cancel_client;
  rcl_client_t result_client;
  rcl_subscription_t feedback_subscription;
  rcl_subscription_t status_subscription;
  rcl_action_client_options_t options;
  char * action_name;
} rcl_action_client_impl_t;

rcl_action_client_t
rcl_action_get_zero_initialized_client(void)
{
  static rcl_action_client_t null_action_client = {0};
  return null_action_client;
}

// \internal Initialize client for the given action's goal service.
static rcl_ret_t rcl_action_goal_service_client_init(
  rcl_client_t * goal_client,
  const rcl_node_t * node,
  const rosidl_service_type_support_t * goal_service_type_support,
  const char * action_name,
  const rcl_client_options_t * goal_client_options)
{
  assert(NULL != goal_client);
  assert(NULL != node);
  assert(NULL != goal_service_type_support);
  assert(NULL != action_name);
  assert(NULL != goal_client_options);

  rcl_ret_t ret;
  rcl_allocator_t allocator = goal_client_options->allocator;

  char * goal_service_name = NULL;
  ret = rcl_action_get_goal_service_name(action_name, allocator, &goal_service_name);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }

  *goal_client = rcl_get_zero_initialized_client();

  ret = rcl_client_init(
    goal_client, node,
    goal_service_type_support,
    goal_service_name,
    goal_client_options);

  allocator.deallocate(goal_service_name, allocator.state);

  if (RCL_RET_OK != ret) {
    if (RCL_RET_SERVICE_NAME_INVALID == ret) {
      return RCL_RET_ACTION_NAME_INVALID;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

// \internal Initialize client for the given action's goal cancel service.
static rcl_ret_t rcl_action_cancel_service_client_init(
  rcl_client_t * cancel_client,
  const rcl_node_t * node,
  const rosidl_service_type_support_t * cancel_service_type_support,
  const char * action_name,
  const rcl_client_options_t * cancel_client_options)
{
  assert(NULL != cancel_client);
  assert(NULL != node);
  assert(NULL != cancel_service_type_support);
  assert(NULL != action_name);
  assert(NULL != cancel_client_options);

  rcl_ret_t ret;
  rcl_allocator_t allocator = cancel_client_options->allocator;

  char * cancel_service_name = NULL;
  ret = rcl_action_get_cancel_service_name(action_name, allocator, &cancel_service_name);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }

  *cancel_client = rcl_get_zero_initialized_client();

  ret = rcl_client_init(
    cancel_client, node,
    cancel_service_type_support,
    cancel_service_name,
    cancel_client_options);

  allocator.deallocate(cancel_service_name, allocator.state);

  if (RCL_RET_OK != ret) {
    if (RCL_RET_SERVICE_NAME_INVALID == ret) {
      return RCL_RET_ACTION_NAME_INVALID;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

// \internal Initialize client for the given action's goal result service.
static rcl_ret_t rcl_action_result_client_init(
  rcl_client_t * result_client,
  const rcl_node_t * node,
  const rosidl_service_type_support_t * result_service_type_support,
  const char * action_name,
  const rcl_client_options_t * result_client_options)
{
  assert(NULL != result_client);
  assert(NULL != node);
  assert(NULL != result_service_type_support);
  assert(NULL != action_name);
  assert(NULL != result_client_options);

  rcl_ret_t ret;
  rcl_allocator_t allocator = result_client_options->allocator;

  char * result_service_name = NULL;
  ret = rcl_action_get_result_service_name(action_name, allocator, &result_service_name);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }

  *result_client = rcl_get_zero_initialized_client();

  ret = rcl_client_init(
    result_client, node,
    result_service_type_support,
    result_service_name,
    result_client_options);

  allocator.deallocate(result_service_name, allocator.state);

  if (RCL_RET_OK != ret) {
    if (RCL_RET_SERVICE_NAME_INVALID == ret) {
      return RCL_RET_ACTION_NAME_INVALID;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

// \internal Initialize subscription to the given action's feedback topic.
static rcl_ret_t rcl_action_feedback_subscription_init(
  rcl_subscription_t * feedback_subscription,
  const rcl_node_t * node,
  const rosidl_message_type_support_t * feedback_message_type_support,
  const char * action_name,
  const rcl_subscription_options_t * feedback_subscription_options)
{
  assert(NULL != feedback_subscription);
  assert(NULL != node);
  assert(NULL != feedback_message_type_support);
  assert(NULL != action_name);
  assert(NULL != feedback_subscription_options);

  rcl_ret_t ret;
  rcl_allocator_t allocator = feedback_subscription_options->allocator;

  char * feedback_topic_name = NULL;
  ret = rcl_action_get_feedback_topic_name(action_name, allocator, &feedback_topic_name);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }

  *feedback_subscription = rcl_get_zero_initialized_subscription();

  ret = rcl_subscription_init(
    feedback_subscription, node,
    feedback_message_type_support,
    feedback_topic_name,
    feedback_subscription_options);

  allocator.deallocate(feedback_topic_name, allocator.state);

  if (RCL_RET_OK != ret) {
    if (RCL_RET_TOPIC_NAME_INVALID == ret) {
      return RCL_RET_ACTION_NAME_INVALID;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

// \internal Initialize subscription to the given action's status topic.
static rcl_ret_t rcl_action_status_subscription_init(
  rcl_subscription_t * status_subscription, const rcl_node_t * node,
  const rosidl_message_type_support_t * status_message_type_support,
  const char * action_name,
  const rcl_subscription_options_t * status_subscription_options)
{
  assert(NULL != status_subscription);
  assert(NULL != node);
  assert(NULL != status_message_type_support);
  assert(NULL != action_name);
  assert(NULL != status_subscription_options);

  rcl_ret_t ret;
  rcl_allocator_t allocator = status_subscription_options->allocator;

  char * status_topic_name = NULL;
  ret = rcl_action_get_status_topic_name(action_name, allocator, &status_topic_name);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }

  *status_subscription = rcl_get_zero_initialized_subscription();

  ret = rcl_subscription_init(
    status_subscription, node,
    status_message_type_support,
    status_topic_name,
    status_subscription_options);

  allocator.deallocate(status_topic_name, allocator.state);

  if (RCL_RET_OK != ret) {
    if (RCL_RET_TOPIC_NAME_INVALID == ret) {
      return RCL_RET_ACTION_NAME_INVALID;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
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
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(action_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t * allocator = (rcl_allocator_t *)&options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret = RCL_RET_OK;
  rcl_ret_t fini_ret = RCL_RET_OK;
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing client for action name '%s'", action_name);
  if (NULL != action_client->impl) {
    RCL_SET_ERROR_MSG("action client already initialized, or memory was uninitialized");
    return RCL_RET_ALREADY_INIT;
  }
  // Allocate space for the implementation struct.
  rcl_action_client_impl_t * impl = action_client->impl = allocator->allocate(
    sizeof(rcl_action_client_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  // Copy action client name and options.
  impl->options = *options;
  impl->action_name = rcutils_strdup(action_name, *((rcutils_allocator_t *)allocator));
  if (NULL == impl->action_name) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }
  // Initialize action goal service client.
  rcl_client_options_t goal_client_options = {
    .qos = options->goal_service_qos, .allocator = *allocator
  };
  ret = rcl_action_goal_service_client_init(
    &impl->goal_client, node,
    type_support->goal_service_type_support,
    impl->action_name, &goal_client_options);
  if (RCL_RET_OK != ret) {
    RCUTILS_LOG_DEBUG_NAMED(
      ROS_PACKAGE_NAME, "Failed to initialize action goal client");
    goto fail;
  }
  // Initialize action cancel service client.
  rcl_client_options_t cancel_client_options = {
    .qos = options->cancel_service_qos, .allocator = *allocator
  };
  ret = rcl_action_cancel_service_client_init(
    &impl->cancel_client, node,
    type_support->cancel_service_type_support,
    impl->action_name, &cancel_client_options);
  if (RCL_RET_OK != ret) {
    RCUTILS_LOG_DEBUG_NAMED(
      ROS_PACKAGE_NAME, "Failed to initialize action cancel client");
    goto fail;
  }
  // Initialize action result service client.
  rcl_client_options_t result_client_options = {
    .qos = options->result_service_qos, .allocator = *allocator
  };
  ret = rcl_action_result_client_init(
    &impl->result_client, node,
    type_support->result_service_type_support,
    impl->action_name, &result_client_options);
  if (RCL_RET_OK != ret) {
    RCUTILS_LOG_DEBUG_NAMED(
      ROS_PACKAGE_NAME, "Failed to initialize action result client");
    goto fail;
  }
  // Initialize action feedback subscription client.
  rcl_subscription_options_t feedback_subscription_options = {
    .qos = options->feedback_topic_qos,
    .ignore_local_publications = false,
    .allocator = *allocator
  };
  ret = rcl_action_feedback_subscription_init(
    &impl->feedback_subscription, node,
    type_support->feedback_message_type_support,
    impl->action_name, &feedback_subscription_options);
  if (RCL_RET_OK != ret) {
    RCUTILS_LOG_DEBUG_NAMED(
      ROS_PACKAGE_NAME, "Failed to initialize feedback subscription");
    goto fail;
  }
  // Initialize action status subscription client.
  rcl_subscription_options_t status_subscription_options = {
    .qos = options->status_topic_qos,
    .ignore_local_publications = false,
    .allocator = *allocator
  };
  ret = rcl_action_status_subscription_init(
    &impl->status_subscription, node,
    type_support->status_message_type_support,
    impl->action_name, &status_subscription_options);
  if (RCL_RET_OK != ret) {
    RCUTILS_LOG_DEBUG_NAMED(
      ROS_PACKAGE_NAME, "Failed to initialize status subscription");
    goto fail;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action client initialized");
  return ret;
fail:
  fini_ret = rcl_action_client_fini(action_client, node);
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
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  // TODO(hidmic): ideally we should rollback to a valid state if any
  //               finalization failed, but it seems that's currently
  //               not possible.
  rcl_ret_t ret = RCL_RET_OK;
  rcl_ret_t fini_ret = RCL_RET_OK;
  rcl_action_client_impl_t * impl = action_client->impl;
  if (rcl_client_is_valid(&impl->goal_client)) {
    fini_ret = rcl_client_fini(&impl->goal_client, node);
    if (RCL_RET_OK != fini_ret) {
      ret = RCL_RET_ERROR;
    }
  }
  if (rcl_client_is_valid(&impl->cancel_client)) {
    fini_ret = rcl_client_fini(&impl->cancel_client, node);
    if (RCL_RET_OK != fini_ret) {
      ret = RCL_RET_ERROR;
    }
  }
  if (rcl_client_is_valid(&impl->result_client)) {
    fini_ret = rcl_client_fini(&impl->result_client, node);
    if (RCL_RET_OK != fini_ret) {
      ret = RCL_RET_ERROR;
    }
  }
  if (rcl_subscription_is_valid(&impl->feedback_subscription)) {
    fini_ret = rcl_subscription_fini(&impl->feedback_subscription, node);
    if (RCL_RET_OK != fini_ret) {
      ret = RCL_RET_ERROR;
    }
  }
  if (rcl_subscription_is_valid(&impl->status_subscription)) {
    fini_ret = rcl_subscription_fini(&impl->status_subscription, node);
    if (RCL_RET_OK != fini_ret) {
      ret = RCL_RET_ERROR;
    }
  }
  if (RCL_RET_OK != ret) {
    rcl_allocator_t * allocator = &impl->options.allocator;
    allocator->deallocate(impl->action_name, allocator->state);
    allocator->deallocate(action_client->impl, allocator->state);
    action_client->impl = NULL;
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action client finalized");
  }
  return ret;
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
rcl_action_send_goal_request(
  const rcl_action_client_t * action_client,
  const void * ros_goal_request)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Sending action goal request");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_goal_request, RCL_RET_INVALID_ARGUMENT);
  int64_t ignored_sequence_number;
  rcl_ret_t ret = rcl_send_request(
    &action_client->impl->goal_client,
    ros_goal_request, &ignored_sequence_number);
  if (RCL_RET_OK != ret) {
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action goal request sent");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_goal_response(
  const rcl_action_client_t * action_client,
  void * ros_goal_response)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action goal response");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_goal_response, RCL_RET_INVALID_ARGUMENT);
  rmw_request_id_t ignored_request_header;
  rcl_ret_t ret = rcl_take_response(
    &action_client->impl->goal_client,
    &ignored_request_header, ros_goal_response);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_CLIENT_TAKE_FAILED == ret) {
      ret = RCL_RET_ACTION_CLIENT_TAKE_FAILED;
    } else {
      ret = RCL_RET_ERROR;
    }
    return ret;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action goal response taken");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_feedback(
  const rcl_action_client_t * action_client,
  void * ros_feedback)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action feedback");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_feedback, RCL_RET_INVALID_ARGUMENT);
  rmw_message_info_t ignored_message_info;
  rcl_ret_t ret = rcl_take(
    &action_client->impl->feedback_subscription,
    ros_feedback, &ignored_message_info);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_SUBSCRIPTION_TAKE_FAILED == ret) {
      return RCL_RET_ACTION_CLIENT_TAKE_FAILED;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action feedback taken");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_status(
  const rcl_action_client_t * action_client,
  void * ros_status_array)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action status");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_status_array, RCL_RET_INVALID_ARGUMENT);
  rmw_message_info_t ignored_message_info;
  rcl_ret_t ret = rcl_take(
    &action_client->impl->status_subscription,
    ros_status_array, &ignored_message_info);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_SUBSCRIPTION_TAKE_FAILED == ret) {
      return RCL_RET_ACTION_CLIENT_TAKE_FAILED;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action status taken");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_send_result_request(
  const rcl_action_client_t * action_client,
  const void * ros_result_request)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Sending action result request");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_result_request, RCL_RET_INVALID_ARGUMENT);
  int64_t ignored_sequence_number;
  rcl_ret_t ret = rcl_send_request(
    &action_client->impl->result_client,
    ros_result_request, &ignored_sequence_number);
  if (RCL_RET_OK != ret) {
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action result request sent");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_result_response(
  const rcl_action_client_t * action_client,
  void * ros_result)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action result response");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_result, RCL_RET_INVALID_ARGUMENT);
  rmw_request_id_t ignored_response_header;
  rcl_ret_t ret = rcl_take_response(
    &action_client->impl->result_client,
    &ignored_response_header, ros_result);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_CLIENT_TAKE_FAILED == ret) {
      return RCL_RET_ACTION_CLIENT_TAKE_FAILED;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action result response taken");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_send_cancel_request(
  const rcl_action_client_t * action_client,
  const void * ros_cancel_request)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Sending action cancel request");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_cancel_request, RCL_RET_INVALID_ARGUMENT);
  int64_t ignored_sequence_number;
  rcl_ret_t ret = rcl_send_request(
    &action_client->impl->cancel_client,
    ros_cancel_request, &ignored_sequence_number);
  if (RCL_RET_OK != ret) {
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action cancel request sent");
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_cancel_response(
  const rcl_action_client_t * action_client,
  void * ros_cancel_response)
{
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Taking action cancel response");
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_cancel_response, RCL_RET_INVALID_ARGUMENT);
  rmw_request_id_t ignored_response_header;
  rcl_ret_t ret = rcl_take_response(
    &action_client->impl->cancel_client,
    &ignored_response_header, ros_cancel_response);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_CLIENT_TAKE_FAILED == ret) {
      return RCL_RET_ACTION_CLIENT_TAKE_FAILED;
    }
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Action cancel response taken");
  return RCL_RET_OK;
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
  return true;
}

rcl_ret_t
rcl_action_wait_set_add_action_client(
  rcl_wait_set_t * wait_set,
  const rcl_action_client_t * action_client)
{
  rcl_ret_t ret;
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_WAIT_SET_INVALID);
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  // Wait on action goal service response messages.
  ret = rcl_wait_set_add_client(
    wait_set, &action_client->impl->goal_client);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action cancel service response messages.
  ret = rcl_wait_set_add_client(
    wait_set, &action_client->impl->cancel_client);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action result service response messages.
  ret = rcl_wait_set_add_client(
    wait_set, &action_client->impl->result_client);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Wait on action feedback messages.
  ret = rcl_wait_set_add_subscription(
    wait_set, &action_client->impl->feedback_subscription);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  return RCL_RET_OK;
  // Wait on action status messages.
  ret = rcl_wait_set_add_subscription(
    wait_set, &action_client->impl->status_subscription);
  if (RCL_RET_OK != ret) {
    return ret;
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
  if (!rcl_action_client_is_valid(action_client)) {
    return RCL_RET_ACTION_CLIENT_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(is_feedback_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_status_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_goal_response_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_cancel_response_ready, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_result_response_ready, RCL_RET_INVALID_ARGUMENT);
  if (2 != wait_set->size_of_subscriptions || 3 != wait_set->size_of_clients) {
    RCL_SET_ERROR_MSG("wait set not initialized or not used by the action client alone");
    return RCL_RET_WAIT_SET_INVALID;
  }
  *is_feedback_ready = !!wait_set->subscriptions[0];
  *is_status_ready = !!wait_set->subscriptions[1];
  *is_goal_response_ready = !!wait_set->clients[0];
  *is_cancel_response_ready = !!wait_set->clients[1];
  *is_result_response_ready = !!wait_set->clients[2];
  return RCL_RET_OK;
}


#ifdef __cplusplus
}
#endif
