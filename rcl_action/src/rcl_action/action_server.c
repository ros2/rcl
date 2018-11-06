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

#include "rcl_action/goal_handle.h"
#include "rcl_action/names.h"
#include "rcl_action/types.h"
#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/time.h"

/// Internal rcl_action implementation struct.
typedef struct rcl_action_server_impl_t
{
  rcl_service_t goal_service;
  rcl_service_t cancel_service
  rcl_service_t result_service
  rcl_publisher_t feedback_publisher;
  rcl_publisher_t status_publisher;
  rcl_action_server_options_t options;
  char * action_name;
  // Array of goal handles
  rcl_action_goal_handle_t * goal_handles;
} rcl_action_server_impl_t;

rcl_action_server_t
rcl_action_get_zero_initialized_server(void)
{
  static rcl_action_server_t null_action_server = {0};
  return null_action_server;
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

  // TODO(jacobperron): Expand the given action name?
  // Test if name is valid

  // TODO(jacobperron): Consider using a macro to initialize services/publishers
  // Initialize goal service
  char * goal_service_name = NULL;
  rcl_ret_t ret = rcl_action_get_goal_service_name(action_name, allocator, &goal_service_name);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG("failed to get goal service name");
    if (RCL_RET_BAD_ALLOC == ret) {
      // TODO: goto fail/cleanup
      return RCL_RET_BAD_ALLOC;
    }
    // TODO: goto fail/cleanup
    return RCL_RET_ERROR;
  }
  rcl_client_options_t goal_service_options = {
    .qos = options->goal_service_qos, .allocator = *allocator
  };
  action_server->impl->goal_service = rcl_get_zero_initialized_service();
  ret = rcl_service_init(
    &action_server->impl->goal_service,
    node,
    &type_support->goal_service_type_support,
    goal_service_name,
    &goal_service_options);
  allocator.deallocate(goal_service_name, allocator.state);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_BAD_ALLOC == ret) {
      // TODO: goto fail/cleanup
      return RCL_RET_BAD_ALLOC;
    }
    // TODO: goto fail/cleanup
    return RCL_RET_ERROR;
  }

  // TODO(jacobperron): Initialize cancel service
  // TODO(jacobperron): Initialize result service

  // TODO(jacobperron): Initialize feedback publisher
  // TODO(jacobperron): Initialize status publisher

  return RCL_RET_OK;
}

rcl_ret_t
rcl_action_server_fini(rcl_action_server_t * action_server, rcl_node_t * node)
{
  // TODO(jacobperron): impl
  return RCL_RET_OK;
}

rcl_action_server_options_t
rcl_action_server_get_default_options(void)
{
  // !!! MAKE SURE THAT CHANGES TO THESE DEFAULTS ARE REFLECTED IN THE HEADER DOC STRING
  static rcl_action_server_options_t default_options;
  // TODO(jacobperron): impl
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
  // TODO(jacobperron): impl
  return NULL;
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
  // TODO(jacobperron): impl
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
  uint32_t * num_expired)
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
  // TODO(jacobperron): impl
  return action_server->impl->action_name;
}

const rcl_action_server_options_t *
rcl_action_server_get_options(const rcl_action_server_t * action_server)
{
  // TODO(jacobperron): impl
  return &action_server->impl->options;
}

const rcl_action_goal_handle_t *
rcl_action_server_get_goal_handles(
  const rcl_action_server_t * action_server,
  uint32_t * num_goals)
{
  // TODO(jacobperron): impl
  return action_server->impl->goal_handles;
}

bool
rcl_action_server_is_valid(const rcl_action_server_t * action_server)
{
  // TODO(jacobperron): impl
  return true;
}

#ifdef __cplusplus
}
#endif
