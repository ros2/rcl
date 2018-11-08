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

#include "rcl_action/action_server.h"

#include "rcl_action/default_qos.h"
#include "rcl_action/goal_handle.h"
#include "rcl_action/names.h"
#include "rcl_action/types.h"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcutils/logging_macros.h"


/// Internal rcl_action implementation struct.
typedef struct rcl_action_server_impl_t
{
  rcl_service_t goal_service;
  rcl_service_t cancel_service;
  rcl_service_t result_service;
  rcl_publisher_t feedback_publisher;
  rcl_publisher_t status_publisher;
  char * action_name;
  rcl_action_server_options_t options;
  // Array of goal handles
  rcl_action_goal_handle_t * goal_handles;
  size_t num_goal_handles;
} rcl_action_server_impl_t;

rcl_action_server_t
rcl_action_get_zero_initialized_server(void)
{
  static rcl_action_server_t null_action_server = {0};
  return null_action_server;
}

#define SERVICE_INIT(Type) \
  char * Type ## _service_name = NULL; \
  ret = rcl_action_get_ ## Type ## _service_name(action_name, *allocator, &Type ## _service_name); \
  if (RCL_RET_OK != ret) { \
    RCL_SET_ERROR_MSG("failed to get " #Type " service name"); \
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
    goto fail; \
  } \
  rcl_service_options_t Type ## _service_options = { \
    .qos = options->Type ## _service_qos, .allocator = *allocator \
  }; \
  action_server->impl->Type ## _service = rcl_get_zero_initialized_service(); \
  ret = rcl_service_init( \
    &action_server->impl->Type ## _service, \
    node, \
    type_support->Type ## _service_type_support, \
    Type ## _service_name, \
    &Type ## _service_options); \
  allocator->deallocate(Type ## _service_name, allocator->state); \
  if (RCL_RET_OK != ret) { \
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
    goto fail; \
  }

#define PUBLISHER_INIT(Type) \
  char * Type ## _topic_name = NULL; \
  ret = rcl_action_get_ ## Type ## _topic_name(action_name, *allocator, &Type ## _topic_name); \
  if (RCL_RET_OK != ret) { \
    RCL_SET_ERROR_MSG("failed to get " #Type " topic name"); \
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
    goto fail; \
  } \
  rcl_publisher_options_t Type ## _publisher_options = { \
    .qos = options->Type ## _topic_qos, .allocator = *allocator \
  }; \
  action_server->impl->Type ## _publisher = rcl_get_zero_initialized_publisher(); \
  ret = rcl_publisher_init( \
    &action_server->impl->Type ## _publisher, \
    node, \
    type_support->Type ## _message_type_support, \
    Type ## _topic_name, \
    &Type ## _publisher_options); \
  allocator->deallocate(Type ## _topic_name, allocator->state); \
  if (RCL_RET_OK != ret) { \
    if (RCL_RET_BAD_ALLOC == ret) { \
      ret = RCL_RET_BAD_ALLOC; \
    } else { \
      ret = RCL_RET_ERROR; \
    } \
    goto fail; \
  }

rcl_ret_t
rcl_action_server_init(
  rcl_action_server_t * action_server,
  const rcl_node_t * node,
  const rosidl_action_type_support_t * type_support,
  const char * action_name,
  const rcl_action_server_options_t * options)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_server, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(action_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME, "Initializing action server for action name '%s'", action_name);
  if (action_server->impl) {
    RCL_SET_ERROR_MSG("action server already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }

  // Allocate for action server
  action_server->impl = (rcl_action_server_impl_t *)allocator->allocate(
    sizeof(rcl_action_server_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    action_server->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);

  // TODO(jacobperron): Expand the given action name
  // Test if name is valid

  // Initialize services
  rcl_ret_t ret;
  SERVICE_INIT(goal);
  SERVICE_INIT(cancel);
  SERVICE_INIT(result);

  // Initialize publishers
  printf("Initializing publishers\n");
  PUBLISHER_INIT(feedback);
  PUBLISHER_INIT(status);

  // Copy action name
  action_server->impl->action_name = rcutils_strdup(
    action_name, *((rcutils_allocator_t *)allocator));
  if (NULL == action_server->impl->action_name) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }

  // Copy options
  action_server->impl->options = *options;

  action_server->impl->goal_handles = NULL;
  action_server->impl->num_goal_handles = 0;

  ret = RCL_RET_OK;
  goto cleanup;
fail:
  printf("Fail\n");
  if (action_server->impl) {
    // Finalize any services/publishers that were initialized and deallocate action_server->impl
    rcl_action_server_fini(action_server, node);
  }
cleanup:
  // TODO(jacobperron) Cleanup from expanded action name
  printf("cleanup\n");
  return ret;
}

rcl_ret_t
rcl_action_server_fini(rcl_action_server_t * action_server, rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_server, RCL_RET_ACTION_SERVER_INVALID);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }

  rcl_ret_t ret = RCL_RET_OK;
  if (action_server->impl) {
    // Finalize services
    if (rcl_service_fini(&action_server->impl->goal_service, node) != RCL_RET_OK) {
      ret = RCL_RET_ERROR;
    }
    if (rcl_service_fini(&action_server->impl->cancel_service, node) != RCL_RET_OK) {
      ret = RCL_RET_ERROR;
    }
    if (rcl_service_fini(&action_server->impl->result_service, node) != RCL_RET_OK) {
      ret = RCL_RET_ERROR;
    }
    // Finalize publishers
    if (rcl_publisher_fini(&action_server->impl->feedback_publisher, node) != RCL_RET_OK) {
      ret = RCL_RET_ERROR;
    }
    if (rcl_publisher_fini(&action_server->impl->status_publisher, node) != RCL_RET_OK) {
      ret = RCL_RET_ERROR;
    }
    // Deallocate struct
    rcl_allocator_t allocator = action_server->impl->options.allocator;
    allocator.deallocate(action_server->impl, allocator.state);
    action_server->impl = NULL;
  }
  return ret;
}

rcl_action_server_options_t
rcl_action_server_get_default_options(void)
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_action_server_options_t default_options;
  default_options.goal_service_qos = rmw_qos_profile_services_default;
  default_options.cancel_service_qos = rmw_qos_profile_services_default;
  default_options.result_service_qos = rmw_qos_profile_services_default;
  default_options.feedback_topic_qos = rmw_qos_profile_default;
  default_options.status_topic_qos = rcl_action_qos_profile_status_default;
  default_options.allocator = rcl_get_default_allocator();
  default_options.result_timeout.nanoseconds = 9e+11;  // 15 minutes
  return default_options;
}

rcl_ret_t
rcl_action_take_goal_request(
  const rcl_action_server_t * action_server,
  void * ros_goal_request)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_send_goal_response(
  const rcl_action_server_t * action_server,
  const void * ros_goal_response)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_action_goal_handle_t *
rcl_action_accept_new_goal(
  const rcl_action_server_t * action_server,
  const rcl_action_goal_info_t * goal_info)
{
  if (!rcl_action_server_is_valid(action_server)) {
    return NULL;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(goal_info, NULL);

  // Check if goal with same ID already exists
  if (rcl_action_server_goal_exists(action_server, goal_info)) {
    RCL_SET_ERROR_MSG("goal ID already exists");
    return NULL;
  }

  // Allocate space for a new goal handle
  rcl_allocator_t allocator = action_server->impl->options.allocator;
  rcl_action_goal_handle_t * goal_handles = action_server->impl->goal_handles;
  const size_t num_goal_handles = action_server->impl->num_goal_handles;
  // TODO(jacobperron): Don't allocate for every accepted goal handle,
  //                    instead double the memory allocated if needed.
  const size_t new_num_goal_handles = num_goal_handles + 1;
  void * tmp_ptr = allocator.reallocate(
    goal_handles, new_num_goal_handles * sizeof(rcl_action_goal_handle_t), allocator.state);
  if (!tmp_ptr) {
    RCL_SET_ERROR_MSG("memory allocation failed for new goal handle");
    return NULL;
  }
  goal_handles = (rcl_action_goal_handle_t *)tmp_ptr;

  // Create a new goal handle
  goal_handles[num_goal_handles] = rcl_action_get_zero_initialized_goal_handle();
  rcl_ret_t ret = rcl_action_goal_handle_init(
    &goal_handles[num_goal_handles], goal_info, allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("failed to initialize goal handle");
    return NULL;
  }

  action_server->impl->goal_handles = goal_handles;
  action_server->impl->num_goal_handles = new_num_goal_handles;
  return &goal_handles[num_goal_handles];
}

rcl_ret_t
rcl_action_publish_feedback(
  const rcl_action_server_t * action_server,
  void * ros_feedback)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_get_goal_status_array(
  const rcl_action_server_t * action_server,
  rcl_action_goal_status_array_t * status_message)
{
  if (!rcl_action_server_is_valid(action_server)) {
    return RCL_RET_ACTION_SERVER_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(status_message, RCL_RET_INVALID_ARGUMENT);

  // If number of goals is zero, then we don't need to do any allocation
  size_t num_goals = action_server->impl->num_goal_handles;
  if (0 == num_goals) {
    status_message->msg.status_list.size = 0u;
    return RCL_RET_OK;
  }

  // Allocate status array
  rcl_allocator_t allocator = action_server->impl->options.allocator;
  rcl_ret_t ret = rcl_action_goal_status_array_init(status_message, num_goals, allocator);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_ERROR;
  }

  // Populate status array
  for (size_t i = 0; i < num_goals; ++i) {
    ret = rcl_action_goal_handle_get_info(
      &action_server->impl->goal_handles[i], &status_message->msg.status_list.data[i].goal_info);
    if (RCL_RET_OK != ret) {
      return RCL_RET_ERROR;
    }
    ret = rcl_action_goal_handle_get_status(
      &action_server->impl->goal_handles[i], &status_message->msg.status_list.data[i].status);
    if (RCL_RET_OK != ret) {
      return RCL_RET_ERROR;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_publish_status(
  const rcl_action_server_t * action_server,
  const rcl_action_goal_status_array_t * status_message)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_result_request(
  const rcl_action_server_t * action_server,
  void * ros_result_request)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_send_result_response(
  const rcl_action_server_t * action_server,
  const void * ros_result_response)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_clear_expired_goals(
  const rcl_action_server_t * action_server,
  size_t * num_expired)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_take_cancel_request(
  const rcl_action_server_t * action_server,
  void * ros_cancel_request)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_process_cancel_request(
  const rcl_action_server_t * action_server,
  const rcl_action_cancel_request_t * cancel_request,
  rcl_action_cancel_response_t * cancel_response)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_send_cancel_response(
  const rcl_action_server_t * action_server,
  const void * ros_cancel_response)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

const char *
rcl_action_server_get_action_name(const rcl_action_server_t * action_server)
{
  if (!rcl_action_server_is_valid(action_server)) {
    return NULL;  // error already set
  }
  return action_server->impl->action_name;
}

const rcl_action_server_options_t *
rcl_action_server_get_options(const rcl_action_server_t * action_server)
{
  if (!rcl_action_server_is_valid(action_server)) {
    return NULL;  // error already set
  }
  return &action_server->impl->options;
}

const rcl_action_goal_handle_t *
rcl_action_server_get_goal_handles(
  const rcl_action_server_t * action_server,
  size_t * num_goals)
{
  if (!rcl_action_server_is_valid(action_server)) {
    return NULL;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(num_goals, NULL);
  *num_goals = action_server->impl->num_goal_handles;
  return action_server->impl->goal_handles;
}

bool
rcl_action_server_goal_exists(
  const rcl_action_server_t * action_server,
  const rcl_action_goal_info_t * goal_info)
{
  if (!rcl_action_server_is_valid(action_server)) {
    return false;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(goal_info, false);

  rcl_action_goal_info_t gh_goal_info = rcl_action_get_zero_initialized_goal_info();
  rcl_ret_t ret;
  for (int i = 0; i < action_server->impl->num_goal_handles; ++i) {
    ret = rcl_action_goal_handle_get_info(&action_server->impl->goal_handles[i], &gh_goal_info);
    if (RCL_RET_OK != ret) {
      RCL_SET_ERROR_MSG("failed to get info for goal handle");
      return false;
    }
    // Compare UUIDs
    // TODO(jacobperron): Move to new function a common place (e.g. goal_handle.h)
    bool uuid_match = true;
    for (int i = 0; (i < 16) && uuid_match; ++i) {
      uuid_match = (gh_goal_info.uuid[i] == goal_info->uuid[i]);
    }
    if (uuid_match) {
      return true;
    }
  }
  return false;
}

bool
rcl_action_server_is_valid(const rcl_action_server_t * action_server)
{
  RCL_CHECK_FOR_NULL_WITH_MSG(action_server, "action server pointer is invalid", return false);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    action_server->impl, "action server implementation is invalid", return false);
  if (!rcl_service_is_valid(&action_server->impl->goal_service)) {
    RCL_SET_ERROR_MSG("goal service is invalid");
    return false;
  }
  if (!rcl_service_is_valid(&action_server->impl->cancel_service)) {
    RCL_SET_ERROR_MSG("cancel service is invalid");
    return false;
  }
  if (!rcl_service_is_valid(&action_server->impl->result_service)) {
    RCL_SET_ERROR_MSG("result service is invalid");
    return false;
  }
  if (!rcl_publisher_is_valid(&action_server->impl->feedback_publisher)) {
    RCL_SET_ERROR_MSG("feedback publisher is invalid");
    return false;
  }
  if (!rcl_publisher_is_valid(&action_server->impl->status_publisher)) {
    RCL_SET_ERROR_MSG("status publisher is invalid");
    return false;
  }
  return true;
}

#ifdef __cplusplus
}
#endif
