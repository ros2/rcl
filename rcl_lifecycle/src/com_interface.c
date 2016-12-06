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

#include <stdio.h>
#include <string.h>

#include "rosidl_generator_c/message_type_support.h"
#include "rosidl_generator_c/string_functions.h"

#include "lifecycle_msgs/msg/transition_event.h"
#include "lifecycle_msgs/srv/get_state.h"
#include "lifecycle_msgs/srv/change_state.h"

#include "rcl/error_handling.h"

#include "rcl_lifecycle/data_types.h"

#include "com_interface.hxx"

static lifecycle_msgs__msg__TransitionEvent msg;

bool concatenate(const char ** prefix, const char ** suffix, char ** result)
{
  size_t prefix_size = strlen(*prefix);
  size_t suffix_size = strlen(*suffix);
  if ((prefix_size + suffix_size) >= 255) {
    return false;
  }
  *result = malloc((prefix_size + suffix_size) * sizeof(char));
  memcpy(*result, *prefix, prefix_size);
  memcpy(*result + prefix_size, *suffix, suffix_size + 1);
  return true;
}

rcl_com_interface_t
rcl_get_zero_initialized_com_interface()
{
  rcl_com_interface_t com_interface;
  com_interface.state_publisher = rcl_get_zero_initialized_publisher();
  com_interface.srv_get_state = rcl_get_zero_initialized_service();
  com_interface.srv_change_state = rcl_get_zero_initialized_service();

  return com_interface;
}

rcl_ret_t
rcl_com_interface_init(rcl_com_interface_t * com_interface,
  rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_change_state)
{
  if (!ts_pub_notify) {
    RCL_SET_ERROR_MSG("ts_pub_notify is NULL");
    return RCL_RET_ERROR;
  }
  if (!ts_srv_get_state) {
    RCL_SET_ERROR_MSG("ts_srv_get_state is NULL");
    return RCL_RET_ERROR;
  }
  if (!ts_srv_change_state) {
    RCL_SET_ERROR_MSG("ts_srv_change_state is NULL");
    return RCL_RET_ERROR;
  }


  const char * node_name = rcl_node_get_name(node_handle);

  {  // initialize publisher
     // Build topic, topic suffix hardcoded for now
     // and limited in length of 255
    const char * topic_prefix = "__transition_notify";
    char * topic_name;
    if (concatenate(&node_name, &topic_prefix, &topic_name) != true) {
      RCL_SET_ERROR_MSG("Topic name exceeds maximum size of 255");
      com_interface = NULL;
      return RCL_RET_ERROR;
    }

    rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
    rcl_ret_t ret = rcl_publisher_init(&com_interface->state_publisher,
        node_handle, ts_pub_notify, topic_name, &publisher_options);
    free(topic_name);

    if (ret != RCL_RET_OK) {
      com_interface = NULL;
      return ret;
    }

    // initialize static message for notification
    lifecycle_msgs__msg__TransitionEvent__init(&msg);
  }

  {  // initialize get state service
     // Build topic, topic suffix hardcoded for now
     // and limited in length of 255
    const char * topic_prefix = "__get_state";
    char * topic_name;
    if (concatenate(&node_name, &topic_prefix, &topic_name) != true) {
      RCL_SET_ERROR_MSG("Topic name exceeds maximum size of 255");
      com_interface = NULL;
      return RCL_RET_ERROR;
    }

    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(&com_interface->srv_get_state,
        node_handle, ts_srv_get_state, topic_name, &service_options);
    free(topic_name);

    if (ret != RCL_RET_OK) {
      com_interface = NULL;
      return ret;
    }
  }

  {  // initialize change state service
     // Build topic, topic suffix hardcoded for now
     // and limited in length of 255
    const char * topic_prefix = "__change_state";
    char * topic_name;
    if (concatenate(&node_name, &topic_prefix, &topic_name) != true) {
      RCL_SET_ERROR_MSG("Topic name exceeds maximum size of 255");
      com_interface = NULL;
      return RCL_RET_ERROR;
    }

    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(&com_interface->srv_change_state,
        node_handle, ts_srv_change_state, topic_name, &service_options);
    free(topic_name);

    if (ret != RCL_RET_OK) {
      com_interface = NULL;
      return ret;
    }
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_com_interface_fini(rcl_com_interface_t * com_interface,
  rcl_node_t * node_handle)
{
  rcl_ret_t fcn_ret = RCL_RET_OK;
  {  // destroy get state srv
    rcl_ret_t ret = rcl_service_fini(&com_interface->srv_get_state,
        node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  {  // destroy change state srv
    rcl_ret_t ret = rcl_service_fini(&com_interface->srv_change_state,
        node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
  }

  {  // destroy the publisher
    rcl_ret_t ret = rcl_publisher_fini(&com_interface->state_publisher,
        node_handle);
    if (ret != RCL_RET_OK) {
      fcn_ret = RCL_RET_ERROR;
    }
    lifecycle_msgs__msg__TransitionEvent__fini(&msg);
  }

  return fcn_ret;
}

rcl_ret_t
rcl_com_interface_publish_notification(rcl_com_interface_t * com_interface,
  const rcl_state_t * start, const rcl_state_t * goal)
{
  msg.start_state.id = start->index;
  rosidl_generator_c__String__assign(&msg.start_state.label, start->label);
  msg.goal_state.id = goal->index;
  rosidl_generator_c__String__assign(&msg.goal_state.label, goal->label);

  return rcl_publish(&com_interface->state_publisher, &msg);
}

#if __cplusplus
}
#endif
