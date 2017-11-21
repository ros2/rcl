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

#include <rcl_interfaces/srv/get_parameters.h>
#include <rcl_interfaces/srv/get_parameter_types.h>
#include <rcl_interfaces/srv/list_parameters.h>
#include <rcl_interfaces/srv/set_parameters.h>
#include <rcl_interfaces/srv/set_parameters_atomically.h>

#include <rcl_interfaces/msg/parameter_event.h>
#include <rcl_interfaces/msg/parameter_type__functions.h>

#include <string.h>

#include "rmw/qos_profiles.h"

#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/parameter_client.h"
#include "rcl/subscription.h"

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_generator_c/service_type_support_struct.h"

#include "./common.h"

typedef struct rcl_parameter_client_impl_t
{
  rcl_parameter_client_options_t options;
  rcl_node_t * node;
  rcl_client_t get_client;
  rcl_client_t get_types_client;  // Also referred to as describe parameters in docs
  rcl_client_t set_client;
  rcl_client_t set_atomically_client;
  rcl_client_t list_client;

  rcl_subscription_t event_subscription;

  // Storage for requests/responses for each client
  rcl_interfaces__srv__GetParameters_Request get_request;
  rcl_interfaces__srv__GetParameters_Response get_response;

  rcl_interfaces__srv__GetParameterTypes_Request get_types_request;
  rcl_interfaces__srv__GetParameterTypes_Response get_types_response;

  rcl_interfaces__srv__SetParameters_Request set_request;
  rcl_interfaces__srv__SetParameters_Response set_response;

  rcl_interfaces__srv__SetParametersAtomically_Request set_atomically_request;
  rcl_interfaces__srv__SetParametersAtomically_Response set_atomically_response;

  rcl_interfaces__srv__ListParameters_Request list_request;
  rcl_interfaces__srv__ListParameters_Response list_response;

  int64_t get_sequence_number;
  int64_t get_types_sequence_number;
  int64_t set_sequence_number;
  int64_t set_atomically_sequence_number;
  int64_t list_sequence_number;
} rcl_parameter_client_impl_t;

rcl_parameter_client_options_t
rcl_parameter_client_get_default_options(void)
{
  static rcl_parameter_client_options_t default_options;
  default_options.qos = rmw_qos_profile_parameters;
  default_options.parameter_event_qos = rmw_qos_profile_parameter_events;
  default_options.allocator = rcl_get_default_allocator();
  default_options.remote_node_name = NULL;
  return default_options;
}

rcl_parameter_client_t
rcl_get_zero_initialized_parameter_client(void)
{
  static rcl_parameter_client_t null_client = {0};
  return null_client;
}

#define RCL_PARAMETER_INITIALIZE_CLIENT(VERB, SRV_TYPE_NAME, SRV_SUFFIX) \
  { \
    const rosidl_service_type_support_t * VERB ## _ts = ROSIDL_GET_SRV_TYPE_SUPPORT( \
      rcl_interfaces, SRV_TYPE_NAME \
      ); \
 \
    size_t VERB ## len = strlen(node_name) + strlen(SRV_SUFFIX) + 1; \
    char * VERB ## _service_name = (char *)options->allocator.allocate( \
      VERB ## len, options->allocator.state); \
    memset(VERB ## _service_name, 0, VERB ## len); \
    VERB ## _service_name = memcpy( \
      VERB ## _service_name, node_name, strlen(node_name) + 1); \
    memcpy((VERB ## _service_name + strlen(node_name)), \
      SRV_SUFFIX, strlen(SRV_SUFFIX) + 1); \
 \
    ret = rcl_client_init( \
      &parameter_client->impl->VERB ## _client, \
      node, \
      VERB ## _ts, \
      VERB ## _service_name, \
      &client_options); \
    options->allocator.deallocate(VERB ## _service_name, options->allocator.state); \
    if (ret != RCL_RET_OK) { \
      fail_ret = ret; \
      goto fail_ ## VERB; \
    } \
    rcl_interfaces__srv__ ## SRV_TYPE_NAME ## _Request__init( \
      &parameter_client->impl->VERB ## _request); \
    rcl_interfaces__srv__ ## SRV_TYPE_NAME ## _Response__init( \
      &parameter_client->impl->VERB ## _response); \
    parameter_client->impl->VERB ## _sequence_number = 0; \
  } \

#define RCL_PARAMETER_CLIENT_FINI(VERB, SRV_TYPE_NAME) \
  ret = rcl_client_fini(&parameter_client->impl->VERB ## _client, parameter_client->impl->node); \
  if (ret != RCL_RET_OK) { \
    fprintf(stderr, "rcl_client_fini failed for client " #VERB "\n"); \
    fail_ret = ret; \
  } \
  rcl_interfaces__srv__ ## SRV_TYPE_NAME ## _Request__fini( \
    &parameter_client->impl->VERB ## _request); \
  rcl_interfaces__srv__ ## SRV_TYPE_NAME ## _Response__fini( \
    &parameter_client->impl->VERB ## _response); \


rcl_ret_t
rcl_parameter_client_init(
  rcl_parameter_client_t * parameter_client,
  rcl_node_t * node,
  const rcl_parameter_client_options_t * options
)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  if (!node->impl) {
    RCL_SET_ERROR_MSG("invalid node", rcl_get_default_allocator());
    return RCL_RET_NODE_INVALID;
  }
  if (parameter_client->impl) {
    RCL_SET_ERROR_MSG("client already initialized, or memory was unintialized", rcl_get_default_allocator());
    return RCL_RET_ALREADY_INIT;
  }

  rcl_ret_t ret;
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  parameter_client->impl = (rcl_parameter_client_impl_t *)options->allocator.allocate(
    sizeof(rcl_parameter_client_impl_t), options->allocator.state);
  if (!parameter_client->impl) {
    goto fail_alloc;
  }
  memset(parameter_client->impl, 0, sizeof(rcl_parameter_client_impl_t));

  parameter_client->impl->node = node;
  rcl_client_options_t client_options = rcl_client_get_default_options();
  client_options.qos = options->qos;
  client_options.allocator = options->allocator;

  const char * node_name = options->remote_node_name !=
    NULL ? node_name = options->remote_node_name : rcl_node_get_name(node);

  // Initialize all clients in impl storage
  RCL_PARAMETER_INITIALIZE_CLIENT(get, GetParameters, "__get_parameters");
  RCL_PARAMETER_INITIALIZE_CLIENT(get_types, GetParameterTypes, "__get_parameter_types");
  RCL_PARAMETER_INITIALIZE_CLIENT(set, SetParameters, "__set_parameters");
  RCL_PARAMETER_INITIALIZE_CLIENT(set_atomically, SetParametersAtomically,
    "__set_parameters_atomically");
  RCL_PARAMETER_INITIALIZE_CLIENT(list, ListParameters, "__list_parameters");

  const rosidl_message_type_support_t * event_ts = ROSIDL_GET_MSG_TYPE_SUPPORT(
    rcl_interfaces, msg, ParameterEvent);
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  subscription_options.allocator = options->allocator;
  subscription_options.qos = options->parameter_event_qos;
  const char * event_topic_name = "parameter_events";
  parameter_client->impl->event_subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(
    &parameter_client->impl->event_subscription, node, event_ts, event_topic_name,
    &subscription_options);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
    goto fail;
  }

  parameter_client->impl->options = *options;
  return RCL_RET_OK;

fail:
  ret = rcl_subscription_fini(&parameter_client->impl->event_subscription,
      parameter_client->impl->node);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
    fprintf(stderr, "rcl_subscription_fini failed in fail block of rcl_parameter_client_init\n");
  }

fail_list:
  RCL_PARAMETER_CLIENT_FINI(list, ListParameters);

fail_set_atomically:
  RCL_PARAMETER_CLIENT_FINI(set_atomically, SetParametersAtomically);

fail_set:
  RCL_PARAMETER_CLIENT_FINI(set, SetParameters);

fail_get_types:
  RCL_PARAMETER_CLIENT_FINI(get_types, GetParameterTypes);

fail_get:
  RCL_PARAMETER_CLIENT_FINI(get, GetParameters);

fail_alloc:

  return fail_ret;
}


rcl_ret_t
rcl_parameter_client_fini(rcl_parameter_client_t * parameter_client)
{
  rcl_ret_t ret;
  rcl_ret_t fail_ret = RCL_RET_OK;
  RCL_PARAMETER_CLIENT_FINI(get, GetParameters);
  RCL_PARAMETER_CLIENT_FINI(get_types, GetParameterTypes);
  RCL_PARAMETER_CLIENT_FINI(set, SetParameters);
  RCL_PARAMETER_CLIENT_FINI(set_atomically, SetParametersAtomically);
  RCL_PARAMETER_CLIENT_FINI(list, ListParameters);

  ret = rcl_subscription_fini(&parameter_client->impl->event_subscription,
      parameter_client->impl->node);
  if (ret != RCL_RET_OK) {
    fail_ret = ret;
  }

  rcl_allocator_t allocator = parameter_client->impl->options.allocator;
  allocator.deallocate(parameter_client->impl, allocator.state);

  if (fail_ret != RCL_RET_OK) {
    return fail_ret;
  }
  return ret;
}


#define DEFINE_RCL_PARAMETER_CLIENT_SEND_REQUEST(VERB, REQUEST_SUBTYPE, SUBFIELD_NAME) \
  rcl_ret_t \
  rcl_parameter_client_send_ ## VERB ## _request( \
    const rcl_parameter_client_t * parameter_client, \
    const REQUEST_SUBTYPE * SUBFIELD_NAME, \
    int64_t * sequence_number) \
  { \
    RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator()); \
    RCL_CHECK_ARGUMENT_FOR_NULL(SUBFIELD_NAME, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator()); \
    RCL_CHECK_ARGUMENT_FOR_NULL(sequence_number, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator()); \
 \
    parameter_client->impl->VERB ## _request.SUBFIELD_NAME = *SUBFIELD_NAME; \
 \
    rcl_ret_t ret = rcl_send_request( \
      &parameter_client->impl->VERB ## _client, &parameter_client->impl->VERB ## _request, \
      &parameter_client->impl->VERB ## _sequence_number); \
    *sequence_number = parameter_client->impl->VERB ## _sequence_number; \
    return ret; \
  }

DEFINE_RCL_PARAMETER_CLIENT_SEND_REQUEST(get, rosidl_generator_c__String__Array, names)
DEFINE_RCL_PARAMETER_CLIENT_SEND_REQUEST(get_types, rosidl_generator_c__String__Array, names)
DEFINE_RCL_PARAMETER_CLIENT_SEND_REQUEST(set, rcl_interfaces__msg__Parameter__Array, parameters)
DEFINE_RCL_PARAMETER_CLIENT_SEND_REQUEST(set_atomically, rcl_interfaces__msg__Parameter__Array,
  parameters)

rcl_ret_t
rcl_parameter_client_send_list_request(
  const rcl_parameter_client_t * parameter_client,
  const rosidl_generator_c__String__Array * prefixes,
  uint64_t depth,
  int64_t * sequence_number)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(prefixes, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(sequence_number, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());

  parameter_client->impl->list_request.prefixes = *prefixes;
  parameter_client->impl->list_request.depth = depth;

  rcl_ret_t ret = rcl_send_request(
    &parameter_client->impl->list_client, &parameter_client->impl->list_request,
    &parameter_client->impl->list_sequence_number);
  *sequence_number = parameter_client->impl->list_sequence_number;
  return ret;
}

#define DEFINE_RCL_PARAMETER_CLIENT_TAKE_RESPONSE(VERB, REQUEST_SUBTYPE, SUBFIELD_NAME) \
  REQUEST_SUBTYPE * \
  rcl_parameter_client_take_ ## VERB ## _response( \
    const rcl_parameter_client_t * parameter_client, \
    rmw_request_id_t * request_header) \
  { \
    RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, NULL, rcl_get_default_allocator()); \
 \
    rcl_ret_t ret = rcl_take_response( \
      &parameter_client->impl->VERB ## _client, request_header, \
      &parameter_client->impl->VERB ## _response); \
    if (ret != RCL_RET_OK) { \
      return NULL; \
    } \
 \
    return &parameter_client->impl->VERB ## _response.SUBFIELD_NAME; \
  }

DEFINE_RCL_PARAMETER_CLIENT_TAKE_RESPONSE(get, rcl_interfaces__msg__ParameterValue__Array, values)
DEFINE_RCL_PARAMETER_CLIENT_TAKE_RESPONSE(get_types, rosidl_generator_c__uint8__Array, types)
DEFINE_RCL_PARAMETER_CLIENT_TAKE_RESPONSE(set, rcl_interfaces__msg__SetParametersResult__Array,
  results)
DEFINE_RCL_PARAMETER_CLIENT_TAKE_RESPONSE(set_atomically, rcl_interfaces__msg__SetParametersResult,
  result)
DEFINE_RCL_PARAMETER_CLIENT_TAKE_RESPONSE(list, rcl_interfaces__msg__ListParametersResult, result)

rcl_ret_t
rcl_parameter_client_take_event(
  const rcl_parameter_client_t * parameter_client,
  rcl_interfaces__msg__ParameterEvent * parameter_event,
  rmw_message_info_t * message_info
)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_event, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());

  rcl_ret_t ret = rcl_take(&parameter_client->impl->event_subscription, parameter_event,
      message_info);

  return ret;
}

rcl_ret_t
rcl_wait_set_add_parameter_client(
  rcl_wait_set_t * wait_set,
  const rcl_parameter_client_t * parameter_client)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  rcl_ret_t ret;

  ret = rcl_wait_set_add_client(wait_set, &parameter_client->impl->get_client);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("Failed to add get_parameters client to waitset!", rcl_get_default_allocator());
    return ret;
  }
  ret = rcl_wait_set_add_client(wait_set, &parameter_client->impl->get_types_client);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("Failed to add get_parameter_types client to waitset!", rcl_get_default_allocator());
    return ret;
  }
  ret = rcl_wait_set_add_client(wait_set, &parameter_client->impl->set_client);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("Failed to add set_parameters client to waitset!", rcl_get_default_allocator());
    return ret;
  }
  ret = rcl_wait_set_add_client(wait_set, &parameter_client->impl->set_atomically_client);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("Failed to add set_parameters_atomically client to waitset!", rcl_get_default_allocator());
    return ret;
  }
  ret = rcl_wait_set_add_client(wait_set, &parameter_client->impl->list_client);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("Failed to add list_parameters client to waitset!", rcl_get_default_allocator());
    return ret;
  }

  ret = rcl_wait_set_add_subscription(wait_set, &parameter_client->impl->event_subscription);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("Failed to add parameter events subscription to waitset!", rcl_get_default_allocator());
    return ret;
  }

  return ret;
}

rcl_ret_t
rcl_parameter_client_get_pending_action(
  const rcl_wait_set_t * wait_set,
  const rcl_parameter_client_t * parameter_client,
  rcl_param_action_t * action)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(wait_set, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_client, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(action, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  size_t i = 0;
  size_t j = 0;

  for (i = 0; i < wait_set->size_of_clients; ++i) {
    for (j = 0; j < RCL_NUMBER_OF_PARAMETER_ACTIONS; ++j) {
      rcl_client_t * client_ptr = NULL;
      *action = j;
      switch (j) {
        case RCL_GET_PARAMETERS:
          client_ptr = &parameter_client->impl->get_client;
          break;
        case RCL_GET_PARAMETER_TYPES:
          client_ptr = &parameter_client->impl->get_types_client;
          break;
        case RCL_SET_PARAMETERS:
          client_ptr = &parameter_client->impl->set_client;
          break;
        case RCL_SET_PARAMETERS_ATOMICALLY:
          client_ptr = &parameter_client->impl->set_atomically_client;
          break;
        case RCL_LIST_PARAMETERS:
          client_ptr = &parameter_client->impl->list_client;
          break;
        default:
          return RCL_RET_ERROR;
      }
      if (client_ptr == wait_set->clients[i]) {
        return RCL_RET_OK;
      }
    }
    *action = RCL_PARAMETER_ACTION_UNKNOWN;
  }
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
