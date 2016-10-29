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

#include "rcl/client.h"

#include <string.h>

#include "rmw/rmw.h"

#include "./common.h"
#include "./stdatomic_helper.h"

typedef struct rcl_client_impl_t
{
  rcl_client_options_t options;
  rmw_client_t * rmw_handle;
  atomic_int_least64_t sequence_number;
} rcl_client_impl_t;

rcl_client_t
rcl_get_zero_initialized_client()
{
  static rcl_client_t null_client = {0};
  return null_client;
}

rcl_ret_t
rcl_client_init(
  rcl_client_t * client,
  const rcl_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rcl_client_options_t * options)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!node->impl) {
    RCL_SET_ERROR_MSG("invalid node");
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(type_support, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  if (client->impl) {
    RCL_SET_ERROR_MSG("client already initialized, or memory was unintialized");
    return RCL_RET_ALREADY_INIT;
  }
  const rcl_allocator_t * allocator = &options->allocator;
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->allocate, "allocate not set", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    allocator->deallocate, "deallocate not set", return RCL_RET_INVALID_ARGUMENT);
  // Allocate space for the implementation struct.
  client->impl = (rcl_client_impl_t *)allocator->allocate(
    sizeof(rcl_client_impl_t), allocator->state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "allocating memory failed", return RCL_RET_BAD_ALLOC);
  // Fill out implementation struct.
  // rmw handle (create rmw client)
  // TODO(wjwwood): pass along the allocator to rmw when it supports it
  client->impl->rmw_handle = rmw_create_client(
    rcl_node_get_rmw_handle(node),
    type_support,
    service_name,
    &options->qos);
  if (!client->impl->rmw_handle) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    goto fail;
  }
  // options
  client->impl->options = *options;
  return RCL_RET_OK;
fail:
  if (client->impl) {
    allocator->deallocate(client->impl, allocator->state);
  }
  return fail_ret;
}

rcl_ret_t
rcl_client_fini(rcl_client_t * client, rcl_node_t * node)
{
  (void)node;
  rcl_ret_t result = RCL_RET_OK;
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (client->impl) {
    rmw_ret_t ret =
      rmw_destroy_client(rcl_node_get_rmw_handle(node), client->impl->rmw_handle);
    if (ret != RMW_RET_OK) {
      RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
      result = RCL_RET_ERROR;
    }
    rcl_allocator_t allocator = client->impl->options.allocator;
    allocator.deallocate(client->impl, allocator.state);
  }
  return result;
}

rcl_client_options_t
rcl_client_get_default_options()
{
  static rcl_client_options_t default_options;
  // Must set the allocator and qos after because they are not a compile time constant.
  default_options.qos = rmw_qos_profile_services_default;
  default_options.allocator = rcl_get_default_allocator();
  return default_options;
}

const char *
rcl_client_get_service_name(const rcl_client_t * client)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(client, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "client is invalid", return NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl->rmw_handle, "client is invalid", return NULL);
  return client->impl->rmw_handle->service_name;
}

const rcl_client_options_t *
rcl_client_get_options(const rcl_client_t * client)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(client, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "client is invalid", return NULL);
  return &client->impl->options;
}

rmw_client_t *
rcl_client_get_rmw_handle(const rcl_client_t * client)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(client, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "client is invalid", return NULL);
  return client->impl->rmw_handle;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_send_request(const rcl_client_t * client, const void * ros_request, int64_t * sequence_number)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_request, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(sequence_number, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "client is invalid", return RCL_RET_INVALID_ARGUMENT);
  *sequence_number = rcl_atomic_load_int64_t(&client->impl->sequence_number);
  if (rmw_send_request(
      client->impl->rmw_handle, ros_request, sequence_number) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  rcl_atomic_exchange_int64_t(&client->impl->sequence_number, *sequence_number);
  return RCL_RET_OK;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_take_response(
  const rcl_client_t * client,
  rmw_request_id_t * request_header,
  void * ros_response)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    client->impl, "client is invalid", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(request_header, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(ros_response, RCL_RET_INVALID_ARGUMENT);

  bool taken = false;
  if (rmw_take_response(
      client->impl->rmw_handle, request_header, ros_response, &taken) != RMW_RET_OK)
  {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_ERROR;
  }
  if (!taken) {
    RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
    return RCL_RET_CLIENT_TAKE_FAILED;
  }
  return RCL_RET_OK;
}


#if __cplusplus
}
#endif
