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

#if __cplusplus
extern "C"
{
#endif

#include "com_interface.h"  // NOLINT

#include <stdio.h>
#include <string.h>

#include <rcl/error_handling.h>
#include <rcutils/concat.h>
#include <rmw/validate_full_topic_name.h>
#include <rosidl_generator_c/message_type_support_struct.h>
#include <rosidl_generator_c/string_functions.h>

#include <lifecycle_msgs/msg/transition_event.h>

#include "rcl_lifecycle/data_types.h"


static lifecycle_msgs__msg__TransitionEvent msg;
static const char * pub_transition_event_suffix = "transition_event";
static const char * srv_change_state_suffix = "change_state";
static const char * srv_get_state_suffix = "get_state";
static const char * srv_get_available_states_suffix = "get_available_states";
static const char * srv_get_available_transitions_suffix = "get_available_transitions";

rmw_ret_t
rcl_lifecycle_validate_topic_name(const char * topic_name)
{
  static rmw_ret_t ret = RMW_RET_ERROR;
  static int validation_result = RMW_TOPIC_INVALID_IS_EMPTY_STRING;

  ret = rmw_validate_full_topic_name(topic_name, &validation_result, NULL);
  if (ret != RMW_RET_OK) {
    RCL_SET_ERROR_MSG("unable to validate topic name", rcl_get_default_allocator());
    return RMW_RET_ERROR;
  }
  // TODO(karsten1987): Handle absolute case
  if (validation_result != RMW_TOPIC_VALID && validation_result != RMW_TOPIC_INVALID_NOT_ABSOLUTE) {
    RCL_SET_ERROR_MSG(
      rmw_full_topic_name_validation_result_string(validation_result), rcl_get_default_allocator())
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rcl_lifecycle_com_interface_t
rcl_lifecycle_get_zero_initialized_com_interface()
{
  rcl_lifecycle_com_interface_t com_interface;
  com_interface.pub_transition_event = rcl_get_zero_initialized_publisher();
  com_interface.srv_change_state = rcl_get_zero_initialized_service();
  com_interface.srv_get_state = rcl_get_zero_initialized_service();
  com_interface.srv_get_available_states = rcl_get_zero_initialized_service();
  com_interface.srv_get_available_transitions = rcl_get_zero_initialized_service();
  return com_interface;
}

rcl_ret_t
rcl_lifecycle_com_interface_init(
  rcl_lifecycle_com_interface_t * com_interface,
  rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_change_state,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_get_available_states,
  const rosidl_service_type_support_t * ts_srv_get_available_transitions)
{
  if (!node_handle) {
    RCL_SET_ERROR_MSG("node_handle is null", rcl_get_default_allocator());
    return RCL_RET_INVALID_ARGUMENT;
  }
  const rcl_node_options_t * node_options = rcl_node_get_options(node_handle);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;
  }
  if (!ts_pub_notify) {
    RCL_SET_ERROR_MSG("ts_pub_notify is NULL", node_options->allocator);
    return RCL_RET_ERROR;
  }
  if (!ts_srv_change_state) {
    RCL_SET_ERROR_MSG("ts_srv_change_state is NULL", node_options->allocator);
    return RCL_RET_ERROR;
  }
  if (!ts_srv_get_state) {
    RCL_SET_ERROR_MSG("ts_srv_get_state is NULL", node_options->allocator);
    return RCL_RET_ERROR;
  }
  if (!ts_srv_get_available_states) {
    RCL_SET_ERROR_MSG("ts_srv_get_available_states is NULL", node_options->allocator);
    return RCL_RET_ERROR;
  }
  if (!ts_srv_get_available_states) {
    RCL_SET_ERROR_MSG("ts_srv_get_available_transitions is NULL", node_options->allocator);
    return RCL_RET_ERROR;
  }

  const char * node_name = rcl_node_get_name(node_handle);
  char * topic_name = NULL;
  rmw_ret_t ret = RMW_RET_ERROR;

  // initialize publisher
  {
    topic_name = rcutils_concat(node_name, pub_transition_event_suffix, "__");
    ret = rcl_lifecycle_validate_topic_name(topic_name);
    if (ret != RMW_RET_OK) {
      goto fail;
    }

    rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
    rcl_ret_t ret = rcl_publisher_init(
      &com_interface->pub_transition_event, node_handle,
      ts_pub_notify, topic_name, &publisher_options);
    free(topic_name);
    topic_name = NULL;

    if (ret != RCL_RET_OK) {
      goto fail;
    }

    // initialize static message for notification
    lifecycle_msgs__msg__TransitionEvent__init(&msg);
  }

  // initialize change state service
  {
    topic_name = rcutils_concat(node_name, srv_change_state_suffix, "__");
    ret = rcl_lifecycle_validate_topic_name(topic_name);
    if (ret != RMW_RET_OK) {
      goto fail;
    }

    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(
      &com_interface->srv_change_state, node_handle,
      ts_srv_change_state, topic_name, &service_options);
    free(topic_name);
    topic_name = NULL;

    if (ret != RCL_RET_OK) {
      goto fail;
    }
  }

  // initialize get state service
  {
    topic_name = rcutils_concat(node_name, srv_get_state_suffix, "__");
    ret = rcl_lifecycle_validate_topic_name(topic_name);
    if (ret != RMW_RET_OK) {
      goto fail;
    }

    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(
      &com_interface->srv_get_state, node_handle,
      ts_srv_get_state, topic_name, &service_options);
    free(topic_name);
    topic_name = NULL;

    if (ret != RCL_RET_OK) {
      goto fail;
    }
  }

  // initialize get available states service
  {
    topic_name = rcutils_concat(node_name, srv_get_available_states_suffix, "__");
    ret = rcl_lifecycle_validate_topic_name(topic_name);
    if (ret != RMW_RET_OK) {
      goto fail;
    }

    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(
      &com_interface->srv_get_available_states, node_handle,
      ts_srv_get_available_states, topic_name, &service_options);
    free(topic_name);
    topic_name = NULL;

    if (ret != RCL_RET_OK) {
      goto fail;
    }
  }

  // initialize get available transitions service
  {
    topic_name = rcutils_concat(node_name, srv_get_available_transitions_suffix, "__");
    ret = rcl_lifecycle_validate_topic_name(topic_name);
    if (ret != RMW_RET_OK) {
      goto fail;
    }

    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(
      &com_interface->srv_get_available_transitions, node_handle,
      ts_srv_get_available_transitions, topic_name, &service_options);
    free(topic_name);
    topic_name = NULL;

    if (ret != RCL_RET_OK) {
      goto fail;
    }
  }

  return RCL_RET_OK;

fail:
  if (RCL_RET_OK != rcl_publisher_fini(&com_interface->pub_transition_event, node_handle)) {
    fprintf(stderr, "%s:%u, Failed to destroy transition_event publisher\n",
      __FILE__, __LINE__);
  }
  if (RCL_RET_OK != rcl_service_fini(&com_interface->srv_change_state, node_handle)) {
    fprintf(stderr, "%s:%u, Failed to destroy change_state service\n",
      __FILE__, __LINE__);
  }
  if (RCL_RET_OK != rcl_service_fini(&com_interface->srv_get_state, node_handle)) {
    fprintf(stderr, "%s:%u, Failed to destroy get_state service\n",
      __FILE__, __LINE__);
  }
  if (RCL_RET_OK != rcl_service_fini(&com_interface->srv_get_available_states, node_handle)) {
    fprintf(stderr, "%s:%u, Failed to destroy get_available_states service\n",
      __FILE__, __LINE__);
  }
  if (RCL_RET_OK != rcl_service_fini(&com_interface->srv_get_available_transitions, node_handle)) {
    fprintf(stderr, "%s:%u, Failed to destroy get_available_transitions service\n",
      __FILE__, __LINE__);
  }

  if (topic_name) {
    free(topic_name);
    topic_name = NULL;
  }

  com_interface = NULL;
  return RCL_RET_ERROR;
}

rcl_ret_t
rcl_lifecycle_com_interface_fini(
  rcl_lifecycle_com_interface_t * com_interface,
  rcl_node_t * node_handle)
{
  rcl_ret_t fcn_ret = RCL_RET_OK;

  // destroy get available transitions srv
  {
    rcl_ret_t ret = rcl_service_fini(
      &com_interface->srv_get_available_transitions, node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  // destroy get available states srv
  {
    rcl_ret_t ret = rcl_service_fini(
      &com_interface->srv_get_available_states, node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  // destroy get state srv
  {
    rcl_ret_t ret = rcl_service_fini(
      &com_interface->srv_get_state, node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  // destroy change state srv
  {
    rcl_ret_t ret = rcl_service_fini(
      &com_interface->srv_change_state, node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  // destroy the publisher
  {
    lifecycle_msgs__msg__TransitionEvent__fini(&msg);

    rcl_ret_t ret = rcl_publisher_fini(
      &com_interface->pub_transition_event, node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  return fcn_ret;
}

rcl_ret_t
rcl_lifecycle_com_interface_publish_notification(
  rcl_lifecycle_com_interface_t * com_interface,
  const rcl_lifecycle_state_t * start, const rcl_lifecycle_state_t * goal)
{
  msg.start_state.id = start->id;
  rosidl_generator_c__String__assign(&msg.start_state.label, start->label);
  msg.goal_state.id = goal->id;
  rosidl_generator_c__String__assign(&msg.goal_state.label, goal->label);

  return rcl_publish(&com_interface->pub_transition_event, &msg);
}

#if __cplusplus
}
#endif
