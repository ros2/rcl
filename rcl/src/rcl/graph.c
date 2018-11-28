// Copyright 2016-2017 Open Source Robotics Foundation, Inc.
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

#include "rcl/graph.h"

#include "rcl/error_handling.h"
#include "rcutils/allocator.h"
#include "rcutils/types.h"
#include "rmw/get_service_names_and_types.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"

#include "./common.h"

rcl_ret_t
rcl_get_topic_names_and_types(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  bool no_demangle,
  rcl_names_and_types_t * topic_names_and_types)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rcutils_allocator_t rcutils_allocator = *allocator;
  rmw_ret = rmw_get_topic_names_and_types(
    rcl_node_get_rmw_handle(node),
    &rcutils_allocator,
    no_demangle,
    topic_names_and_types
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_get_service_names_and_types(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  rcl_names_and_types_t * service_names_and_types)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(service_names_and_types, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_names_and_types_check_zero(service_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rcutils_allocator_t rcutils_allocator = *allocator;
  rmw_ret = rmw_get_service_names_and_types(
    rcl_node_get_rmw_handle(node),
    &rcutils_allocator,
    service_names_and_types
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_names_and_types_fini(rcl_names_and_types_t * topic_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret = rmw_names_and_types_fini(topic_names_and_types);
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_get_node_names(
  const rcl_node_t * node,
  rcl_allocator_t allocator,
  rcutils_string_array_t * node_names,
  rcutils_string_array_t * node_namespaces)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(node_names, RCL_RET_INVALID_ARGUMENT);
  if (node_names->size != 0) {
    RCL_SET_ERROR_MSG("node_names size is not zero");
    return RCL_RET_INVALID_ARGUMENT;
  }
  if (node_names->data) {
    RCL_SET_ERROR_MSG("node_names is not null");
    return RCL_RET_INVALID_ARGUMENT;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(node_namespaces, RCL_RET_INVALID_ARGUMENT);
  if (node_namespaces->size != 0) {
    RCL_SET_ERROR_MSG("node_namespaces size is not zero");
    return RCL_RET_INVALID_ARGUMENT;
  }
  if (node_namespaces->data) {
    RCL_SET_ERROR_MSG("node_namespaces is not null");
    return RCL_RET_INVALID_ARGUMENT;
  }
  (void)allocator;  // to be used in rmw_get_node_names in the future
  rmw_ret_t rmw_ret = rmw_get_node_names(
    rcl_node_get_rmw_handle(node),
    node_names,
    node_namespaces);
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_count_publishers(
  const rcl_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;  // shouldn't happen, but error is already set if so
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(count, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret = rmw_count_publishers(rcl_node_get_rmw_handle(node), topic_name, count);
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_count_subscribers(
  const rcl_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;  // shouldn't happen, but error is already set if so
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(count, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret = rmw_count_subscribers(rcl_node_get_rmw_handle(node), topic_name, count);
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_service_server_is_available(
  const rcl_node_t * node,
  const rcl_client_t * client,
  bool * is_available)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;  // shouldn't happen, but error is already set if so
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_available, RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret = rmw_service_server_is_available(
    rcl_node_get_rmw_handle(node),
    rcl_client_get_rmw_handle(client),
    is_available
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

#ifdef __cplusplus
}
#endif
