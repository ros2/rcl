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
#include "rcutils/macros.h"
#include "rcutils/types.h"
#include "rmw/error_handling.h"
#include "rmw/get_node_info_and_types.h"
#include "rmw/get_service_names_and_types.h"
#include "rmw/get_topic_endpoint_info.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw/topic_endpoint_info_array.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "./common.h"

rcl_ret_t
__validate_node_name_and_namespace(
  const char * node_name,
  const char * node_namespace)
{
  int validation_result = 0;
  rmw_ret_t rmw_ret = rmw_validate_namespace(node_namespace, &validation_result, NULL);

  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  if (validation_result != RMW_NAMESPACE_VALID) {
    const char * msg = rmw_namespace_validation_result_string(validation_result);
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("%s, result: %d", msg, validation_result);
    return RCL_RET_NODE_INVALID_NAMESPACE;
  }

  validation_result = 0;
  rmw_ret = rmw_validate_node_name(node_name, &validation_result, NULL);
  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  if (RMW_NODE_NAME_VALID != validation_result) {
    const char * msg = rmw_node_name_validation_result_string(validation_result);
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("%s, result: %d", msg, validation_result);
    return RCL_RET_NODE_INVALID_NAME;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_get_publisher_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  bool no_demangle,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * topic_names_and_types)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_namespace, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types, RCL_RET_INVALID_ARGUMENT);

  const char * valid_namespace = "/";
  if (strlen(node_namespace) > 0) {
    valid_namespace = node_namespace;
  }
  rmw_ret_t rmw_ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (RMW_RET_OK != rmw_ret) {
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rcl_ret_t rcl_ret = __validate_node_name_and_namespace(node_name, valid_namespace);
  if (RCL_RET_OK != rcl_ret) {
    return rcl_ret;
  }
  rcutils_allocator_t rcutils_allocator = *allocator;
  rmw_ret = rmw_get_publisher_names_and_types_by_node(
    rcl_node_get_rmw_handle(node),
    &rcutils_allocator,
    node_name,
    valid_namespace,
    no_demangle,
    topic_names_and_types
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_get_subscriber_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  bool no_demangle,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * topic_names_and_types)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_namespace, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types, RCL_RET_INVALID_ARGUMENT);

  const char * valid_namespace = "/";
  if (strlen(node_namespace) > 0) {
    valid_namespace = node_namespace;
  }
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rcutils_allocator_t rcutils_allocator = *allocator;
  rcl_ret_t rcl_ret = __validate_node_name_and_namespace(node_name, valid_namespace);
  if (RCL_RET_OK != rcl_ret) {
    return rcl_ret;
  }
  rmw_ret = rmw_get_subscriber_names_and_types_by_node(
    rcl_node_get_rmw_handle(node),
    &rcutils_allocator,
    node_name,
    valid_namespace,
    no_demangle,
    topic_names_and_types
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_get_service_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * service_names_and_types)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_namespace, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_names_and_types, RCL_RET_INVALID_ARGUMENT);

  const char * valid_namespace = "/";
  if (strlen(node_namespace) > 0) {
    valid_namespace = node_namespace;
  }
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_names_and_types_check_zero(service_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rcl_ret_t rcl_ret = __validate_node_name_and_namespace(node_name, valid_namespace);
  if (RCL_RET_OK != rcl_ret) {
    return rcl_ret;
  }
  rcutils_allocator_t rcutils_allocator = *allocator;
  rmw_ret = rmw_get_service_names_and_types_by_node(
    rcl_node_get_rmw_handle(node),
    &rcutils_allocator,
    node_name,
    valid_namespace,
    service_names_and_types
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_get_client_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * service_names_and_types)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(node_namespace, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(service_names_and_types, RCL_RET_INVALID_ARGUMENT);

  const char * valid_namespace = "/";
  if (strlen(node_namespace) > 0) {
    valid_namespace = node_namespace;
  }
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_names_and_types_check_zero(service_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rcl_ret_t rcl_ret = __validate_node_name_and_namespace(node_name, valid_namespace);
  if (RCL_RET_OK != rcl_ret) {
    return rcl_ret;
  }
  rcutils_allocator_t rcutils_allocator = *allocator;
  rmw_ret = rmw_get_client_names_and_types_by_node(
    rcl_node_get_rmw_handle(node),
    &rcutils_allocator,
    node_name,
    valid_namespace,
    service_names_and_types
  );
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

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
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
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
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
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
rcl_names_and_types_init(
  rcl_names_and_types_t * names_and_types,
  size_t size,
  rcl_allocator_t * allocator)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);

  RCL_CHECK_ARGUMENT_FOR_NULL(names_and_types, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR(allocator, return RCL_RET_INVALID_ARGUMENT);
  rmw_ret_t rmw_ret = rmw_names_and_types_init(names_and_types, size, allocator);
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_names_and_types_fini(rcl_names_and_types_t * topic_names_and_types)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);

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
rcl_get_node_names_with_enclaves(
  const rcl_node_t * node,
  rcl_allocator_t allocator,
  rcutils_string_array_t * node_names,
  rcutils_string_array_t * node_namespaces,
  rcutils_string_array_t * enclaves)
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
  RCL_CHECK_ARGUMENT_FOR_NULL(enclaves, RCL_RET_INVALID_ARGUMENT);
  if (enclaves->size != 0) {
    RCL_SET_ERROR_MSG("enclaves size is not zero");
    return RCL_RET_INVALID_ARGUMENT;
  }
  if (enclaves->data) {
    RCL_SET_ERROR_MSG("enclaves is not null");
    return RCL_RET_INVALID_ARGUMENT;
  }
  (void)allocator;  // to be used in rmw_get_node_names in the future
  rmw_ret_t rmw_ret = rmw_get_node_names_with_enclaves(
    rcl_node_get_rmw_handle(node),
    node_names,
    node_namespaces,
    enclaves);
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

typedef rmw_ret_t (* get_topic_endpoint_info_func_t)(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * info_array);

rcl_ret_t
__rcl_get_info_by_topic(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * info_array,
  get_topic_endpoint_info_func_t get_topic_endpoint_info)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set.
  }
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  if (!node_options) {
    return RCL_RET_NODE_INVALID;  // shouldn't happen, but error is already set if so
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  rmw_error_string_t error_string;
  rmw_ret_t rmw_ret = rmw_topic_endpoint_info_array_check_zero(info_array);
  if (rmw_ret != RMW_RET_OK) {
    error_string = rmw_get_error_string();
    rmw_reset_error();
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "rmw_topic_endpoint_info_array_t must be zero initialized: %s,\n"
      "Use rmw_get_zero_initialized_topic_endpoint_info_array",
      error_string.str);
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }
  rmw_ret = get_topic_endpoint_info(
    rcl_node_get_rmw_handle(node),
    allocator,
    topic_name,
    no_mangle,
    info_array);
  if (rmw_ret != RMW_RET_OK) {
    error_string = rmw_get_error_string();
    rmw_reset_error();
    RCL_SET_ERROR_MSG(error_string.str);
  }
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
}

rcl_ret_t
rcl_get_publishers_info_by_topic(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * publishers_info)
{
  return __rcl_get_info_by_topic(
    node,
    allocator,
    topic_name,
    no_mangle,
    publishers_info,
    rmw_get_publishers_info_by_topic);
}

rcl_ret_t
rcl_get_subscriptions_info_by_topic(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * subscriptions_info)
{
  return __rcl_get_info_by_topic(
    node,
    allocator,
    topic_name,
    no_mangle,
    subscriptions_info,
    rmw_get_subscriptions_info_by_topic);
}

rcl_ret_t
rcl_service_server_is_available(
  const rcl_node_t * node,
  const rcl_client_t * client,
  bool * is_available)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCL_RET_NODE_INVALID);

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
