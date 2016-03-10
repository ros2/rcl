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

#ifndef RCL__PARAMETER_CLIENT_H_
#define RCL__PARAMETER_CLIENT_H_

#if __cplusplus
extern "C"
{
#endif

#include <rcl_interfaces/msg/list_parameters_result__struct.h>
#include <rcl_interfaces/msg/parameter__struct.h>
#include <rcl_interfaces/msg/parameter_event__struct.h>
#include <rcl_interfaces/msg/set_parameters_result__struct.h>

#include "rcl/client.h"
#include "rcl/node.h"
#include "rcl/parameter.h"
#include "rcl/wait.h"

/// TODO: documentation!!!

/// Internal rcl implementation struct
struct rcl_parameter_client_impl_t;

/// There is no sync/async parameter client distinction in rcl.
typedef struct rcl_parameter_client_t
{
  struct rcl_parameter_client_impl_t * impl;
} rcl_parameter_client_t;

typedef struct rcl_parameter_client_options_t
{
  // quality of service settings for all parameter-related services
  rmw_qos_profile_t qos;
  rcl_allocator_t allocator;
  char * remote_node_name;
  rmw_qos_profile_t parameter_event_qos;
} rcl_parameter_client_options_t;

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_parameter_client_options_t
rcl_parameter_client_get_default_options(void);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_parameter_client_t
rcl_get_zero_initialized_parameter_client(void);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_init(
  rcl_parameter_client_t * parameter_client,
  rcl_node_t * node,
  const rcl_parameter_client_options_t * options
);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_fini(rcl_parameter_client_t * parameter_client);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_send_set_request(
  const rcl_parameter_client_t * parameter_client,
  const rcl_interfaces__msg__Parameter__Array * parameters,
  int64_t * sequence_number);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_interfaces__msg__SetParametersResult__Array *
rcl_parameter_client_take_set_response(
  const rcl_parameter_client_t * parameter_client,
  rmw_request_id_t * request_header);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_send_get_request(
  const rcl_parameter_client_t * client,
  const rosidl_generator_c__String__Array * names,
  int64_t * sequence_number);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_interfaces__msg__ParameterValue__Array *
rcl_parameter_client_take_get_response(
  const rcl_parameter_client_t * client,
  rmw_request_id_t * request_header);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_send_get_types_request(
  const rcl_parameter_client_t * client,
  const rosidl_generator_c__String__Array * parameter_names,
  int64_t * sequence_number);

RCL_PUBLIC
RCL_WARN_UNUSED
rosidl_generator_c__uint8__Array *
rcl_parameter_client_take_get_types_response(
  const rcl_parameter_client_t * client,
  rmw_request_id_t * request_header);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_send_set_atomically_request(
  const rcl_parameter_client_t * client,
  const rcl_interfaces__msg__Parameter__Array * parameter_values,
  int64_t * sequence_number);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_interfaces__msg__SetParametersResult *
rcl_parameter_client_take_set_atomically_response(
  const rcl_parameter_client_t * client,
  rmw_request_id_t * request_header);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_send_list_request(
  const rcl_parameter_client_t * client,
  const rosidl_generator_c__String__Array * prefixes,
  uint64_t depth,
  int64_t * sequence_number);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_interfaces__msg__ListParametersResult *
rcl_parameter_client_take_list_response(
  const rcl_parameter_client_t * client,
  rmw_request_id_t * request_header);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_take_event(
  const rcl_parameter_client_t * client,
  rcl_interfaces__msg__ParameterEvent * parameter_event,
  rmw_message_info_t * message_info
);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_wait_set_add_parameter_client(
  rcl_wait_set_t * wait_set,
  const rcl_parameter_client_t * parameter_client);

// To be called after rcl_wait
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_parameter_client_get_pending_action(
  const rcl_wait_set_t * wait_set,
  const rcl_parameter_client_t * parameter_client,
  rcl_param_action_t * action);

#if __cplusplus
}
#endif

#endif  // RCL__PARAMETER_CLIENT_H_
