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

#include "rcl/graph.h"

#include "./common.h"

rcl_ret_t
rcl_get_topic_names_and_types(
  const rcl_node_t * node,
  rcl_topic_names_and_types_t * topic_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types, RCL_RET_INVALID_ARGUMENT);
  return rmw_get_topic_names_and_types(
    rcl_node_get_rmw_handle(node),
    topic_names_and_types
  );
}

rcl_ret_t
rcl_destroy_topic_names_and_types(
  rcl_topic_names_and_types_t * topic_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types, RCL_RET_INVALID_ARGUMENT);
  return rmw_destroy_topic_names_and_types(topic_names_and_types);
}

rcl_ret_t
rcl_count_publishers(
  const rcl_node_t * node,
  const char * topic_name,
  size_t * count)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(count, RCL_RET_INVALID_ARGUMENT);
  return rmw_count_publishers(rcl_node_get_rmw_handle(node), topic_name, count);
}

rcl_ret_t
rcl_count_subscribers(
  const rcl_node_t * node,
  const char * topic_name,
  size_t * count)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(topic_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(count, RCL_RET_INVALID_ARGUMENT);
  return rmw_count_subscribers(rcl_node_get_rmw_handle(node), topic_name, count);
}

rcl_ret_t
rcl_service_server_is_available(
  const rcl_node_t * node,
  const rcl_client_t * client,
  bool * is_available)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(client, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(is_available, RCL_RET_INVALID_ARGUMENT);
  return rmw_service_server_is_available(
    rcl_node_get_rmw_handle(node),
    rcl_client_get_rmw_handle(client),
    is_available
  );
}

#if __cplusplus
}
#endif
