// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include <assert.h>
#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/node.h"
#include "rcutils/strdup.h"

#include "rcl_action/graph.h"
#include "rcl_action/names.h"

static
rcl_ret_t
_filter_action_names(
  rcl_names_and_types_t * topic_names_and_types,
  rcl_allocator_t * allocator,
  rcl_names_and_types_t * action_names_and_types)
{
  assert(topic_names_and_types);
  assert(allocator);
  assert(action_names_and_types);

  // Assumption: actions provide a topic name with the suffix "/_action/feedback"
  // and it has type with the suffix "_FeedbackMessage"
  const char * action_name_identifier = "/_action/feedback";
  const char * action_type_identifier = "_FeedbackMessage";

  rcl_ret_t ret;
  const size_t num_names = topic_names_and_types->names.size;
  char ** names = topic_names_and_types->names.data;

  // Count number of actions to determine how much memory to allocate
  size_t num_actions = 0u;
  for (size_t i = 0u; i < num_names; ++i) {
    const char * identifier_index = strstr(names[i], action_name_identifier);
    if (identifier_index && strlen(identifier_index) == strlen(action_name_identifier)) {
      ++num_actions;
    }
  }

  if (0u == num_actions) {
    return RCL_RET_OK;
  }

  ret = rcl_names_and_types_init(action_names_and_types, num_actions, allocator);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = RCL_RET_OK;

  // Prune names/types that are not actions (ie. do not contain the suffix)
  const size_t suffix_len = strlen(action_name_identifier);
  size_t j = 0u;
  for (size_t i = 0u; i < num_names; ++i) {
    const char * identifier_index = strstr(names[i], action_name_identifier);
    if (identifier_index && strlen(identifier_index) == strlen(action_name_identifier)) {
      const size_t action_name_len = strlen(names[i]) - suffix_len;
      char * action_name = rcutils_strndup(names[i], action_name_len, *allocator);
      if (!action_name) {
        RCL_SET_ERROR_MSG("Failed to allocate memory for action name");
        ret = RCL_RET_BAD_ALLOC;
        break;
      }

      action_names_and_types->names.data[j] = action_name;

      // Allocate storage for type list
      rcutils_ret_t rcutils_ret = rcutils_string_array_init(
        &action_names_and_types->types[j],
        topic_names_and_types->types[i].size,
        allocator);
      if (RCUTILS_RET_OK != rcutils_ret) {
        RCL_SET_ERROR_MSG(rcutils_get_error_string().str);
        ret = RCL_RET_BAD_ALLOC;
        break;
      }

      // Populate types list
      for (size_t k = 0u; k < topic_names_and_types->types[i].size; ++k) {
        char * type_name = topic_names_and_types->types[i].data[k];
        size_t action_type_len = strlen(type_name);
        // Trim type name suffix
        const size_t type_suffix_len = strlen(action_type_identifier);
        const char * type_identifier_index = strstr(type_name, action_type_identifier);
        if (type_identifier_index &&
          strlen(type_identifier_index) == strlen(action_type_identifier))
        {
          action_type_len = strlen(type_name) - type_suffix_len;
        }
        // Copy name to output struct
        char * action_type_name = rcutils_strndup(type_name, action_type_len, *allocator);
        if (!action_type_name) {
          RCL_SET_ERROR_MSG("Failed to allocate memory for action type");
          ret = RCL_RET_BAD_ALLOC;
          break;
        }
        action_names_and_types->types[j].data[k] = action_type_name;
      }
      ++j;
    }
  }

  // Cleanup if there is an error
  if (RCL_RET_OK != ret) {
    rcl_ret_t fini_ret = rcl_names_and_types_fini(action_names_and_types);
    if (RCL_RET_OK != fini_ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
        "Freeing names and types failed while handling a previous error. Leaking memory!\n");
    }
  }

  return ret;
}

rcl_ret_t
rcl_action_get_client_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * action_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_names_and_types, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();
  ret = rcl_get_subscriber_names_and_types_by_node(
    node, allocator, false, node_name, node_namespace, &topic_names_and_types);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _filter_action_names(
    &topic_names_and_types,
    allocator,
    action_names_and_types);

  rcl_ret_t nat_fini_ret = rcl_names_and_types_fini(&topic_names_and_types);
  if (RCL_RET_OK != nat_fini_ret) {
    ret = rcl_names_and_types_fini(action_names_and_types);
    if (RCL_RET_OK != ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
        "Freeing names and types failed while handling a previous error. Leaking memory!\n");
    }
    return nat_fini_ret;
  }
  return ret;
}

rcl_ret_t
rcl_action_get_server_names_and_types_by_node(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rcl_names_and_types_t * action_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_names_and_types, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();
  ret = rcl_get_publisher_names_and_types_by_node(
    node, allocator, false, node_name, node_namespace, &topic_names_and_types);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _filter_action_names(
    &topic_names_and_types,
    allocator,
    action_names_and_types);

  rcl_ret_t nat_fini_ret = rcl_names_and_types_fini(&topic_names_and_types);
  if (RCL_RET_OK != nat_fini_ret) {
    ret = rcl_names_and_types_fini(action_names_and_types);
    if (RCL_RET_OK != ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
        "Freeing names and types failed while handling a previous error. Leaking memory!\n");
    }

    return nat_fini_ret;
  }
  return ret;
}

rcl_ret_t
rcl_action_get_names_and_types(
  const rcl_node_t * node,
  rcl_allocator_t * allocator,
  rcl_names_and_types_t * action_names_and_types)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(action_names_and_types, RCL_RET_INVALID_ARGUMENT);
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();
  rcl_ret_t ret = rcl_get_topic_names_and_types(node, allocator, false, &topic_names_and_types);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _filter_action_names(
    &topic_names_and_types,
    allocator,
    action_names_and_types);

  rcl_ret_t nat_fini_ret = rcl_names_and_types_fini(&topic_names_and_types);
  if (RCL_RET_OK != nat_fini_ret) {
    ret = rcl_names_and_types_fini(action_names_and_types);
    if (RCL_RET_OK != ret) {
      RCUTILS_SET_ERROR_MSG(
        "Freeing names and types failed while handling a previous error. Leaking memory!\n");
    }
    return nat_fini_ret;
  }
  return ret;
}

static rcl_ret_t
_rcl_action_count_entities(
  const rcl_node_t * node,
  const char * action_name,
  size_t * count,
  int is_client)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(action_name, RCL_RET_INVALID_ARGUMENT);
  if (action_name[0] == '\0') {
    RCL_SET_ERROR_MSG("action_name must not be empty");
    return RCL_RET_INVALID_ARGUMENT;
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(count, RCL_RET_INVALID_ARGUMENT);

  *count = 0u;

  // Count entities on the action's goal service instead of walking the list
  // of discovered nodes: every action client has exactly one goal service
  // client and every action server has exactly one goal service.
  // Name-based per-node queries double-count when the graph momentarily
  // holds duplicate node names (e.g. a stale instance of a restarted node).
  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * goal_service_name = NULL;
  rcl_ret_t ret = rcl_action_get_goal_service_name(
    action_name, allocator, &goal_service_name);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  if (is_client) {
    ret = rcl_count_clients(node, goal_service_name, count);
  } else {
    ret = rcl_count_services(node, goal_service_name, count);
  }

  allocator.deallocate(goal_service_name, allocator.state);
  return ret;
}

rcl_ret_t
rcl_action_count_clients(
  const rcl_node_t * node,
  const char * action_name,
  size_t * count)
{
  return _rcl_action_count_entities(node, action_name, count, 1);
}

rcl_ret_t
rcl_action_count_servers(
  const rcl_node_t * node,
  const char * action_name,
  size_t * count)
{
  return _rcl_action_count_entities(node, action_name, count, 0);
}

rcl_action_endpoint_info_t
rcl_action_get_zero_initialized_endpoint_info(void)
{
  rcl_action_endpoint_info_t zero_info;
  zero_info.goal_service_info = rmw_get_zero_initialized_service_endpoint_info();
  zero_info.cancel_service_info = rmw_get_zero_initialized_service_endpoint_info();
  zero_info.result_service_info = rmw_get_zero_initialized_service_endpoint_info();
  zero_info.feedback_topic_info = rmw_get_zero_initialized_topic_endpoint_info();
  zero_info.status_topic_info = rmw_get_zero_initialized_topic_endpoint_info();
  return zero_info;
}

rcl_action_endpoint_info_array_t
rcl_action_get_zero_initialized_endpoint_info_array(void)
{
  const rcl_action_endpoint_info_array_t zero_array = {0u, NULL};
  return zero_array;
}

rcl_ret_t
rcl_action_endpoint_info_array_fini(
  rcl_action_endpoint_info_array_t * info_array,
  rcutils_allocator_t * allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(info_array, RCL_RET_INVALID_ARGUMENT);
  if (info_array->size > 0u && NULL == info_array->info_array) {
    RCL_SET_ERROR_MSG("info_array->info_array is NULL but size is non-zero");
    return RCL_RET_INVALID_ARGUMENT;
  }

  rcl_ret_t ret = RCL_RET_OK;
  for (size_t i = 0u; i < info_array->size; ++i) {
    rcl_action_endpoint_info_t * info = &info_array->info_array[i];
    if (RMW_RET_OK != rmw_service_endpoint_info_fini(&info->goal_service_info, allocator)) {
      ret = RCL_RET_ERROR;
    }
    if (RMW_RET_OK != rmw_service_endpoint_info_fini(&info->cancel_service_info, allocator)) {
      ret = RCL_RET_ERROR;
    }
    if (RMW_RET_OK != rmw_service_endpoint_info_fini(&info->result_service_info, allocator)) {
      ret = RCL_RET_ERROR;
    }
    if (RMW_RET_OK != rmw_topic_endpoint_info_fini(&info->feedback_topic_info, allocator)) {
      ret = RCL_RET_ERROR;
    }
    if (RMW_RET_OK != rmw_topic_endpoint_info_fini(&info->status_topic_info, allocator)) {
      ret = RCL_RET_ERROR;
    }
  }
  if (NULL != info_array->info_array) {
    allocator->deallocate(info_array->info_array, allocator->state);
  }
  info_array->info_array = NULL;
  info_array->size = 0u;
  return ret;
}

/// Find the first not yet claimed service endpoint info matching the given node.
/**
 * Claimed entries are recognized by a `NULL` node name (see the shallow move
 * in _rcl_action_get_info_by_action below).
 */
static rmw_service_endpoint_info_t *
_find_service_endpoint_info_by_node(
  rcl_service_endpoint_info_array_t * info_array,
  const char * node_name,
  const char * node_namespace)
{
  for (size_t i = 0u; i < info_array->size; ++i) {
    rmw_service_endpoint_info_t * info = &info_array->info_array[i];
    if (NULL == info->node_name || NULL == info->node_namespace) {
      continue;
    }
    if (0 == strcmp(info->node_name, node_name) &&
      0 == strcmp(info->node_namespace, node_namespace))
    {
      return info;
    }
  }
  return NULL;
}

/// Find the first not yet claimed topic endpoint info matching the given node.
static rmw_topic_endpoint_info_t *
_find_topic_endpoint_info_by_node(
  rcl_topic_endpoint_info_array_t * info_array,
  const char * node_name,
  const char * node_namespace)
{
  for (size_t i = 0u; i < info_array->size; ++i) {
    rmw_topic_endpoint_info_t * info = &info_array->info_array[i];
    if (NULL == info->node_name || NULL == info->node_namespace) {
      continue;
    }
    if (0 == strcmp(info->node_name, node_name) &&
      0 == strcmp(info->node_namespace, node_namespace))
    {
      return info;
    }
  }
  return NULL;
}

typedef rcl_ret_t (* rcl_action_get_name_func_t)(
  const char * action_name,
  rcl_allocator_t allocator,
  char ** name);

/// Resolve the name of an underlying action service and query its endpoint info.
static rcl_ret_t
_rcl_action_get_service_info(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_get_name_func_t get_service_name,
  int is_client,
  rcl_service_endpoint_info_array_t * infos)
{
  char * service_name = NULL;
  rcl_ret_t ret = get_service_name(action_name, *allocator, &service_name);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Action clients are clients of the underlying services,
  // while action servers are servers of the underlying services.
  if (is_client) {
    ret = rcl_get_clients_info_by_service(node, allocator, service_name, false, infos);
  } else {
    ret = rcl_get_servers_info_by_service(node, allocator, service_name, false, infos);
  }
  allocator->deallocate(service_name, allocator->state);
  return ret;
}

/// Resolve the name of an underlying action topic and query its endpoint info.
static rcl_ret_t
_rcl_action_get_topic_info(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_get_name_func_t get_topic_name,
  int is_client,
  rcl_topic_endpoint_info_array_t * infos)
{
  char * topic_name = NULL;
  rcl_ret_t ret = get_topic_name(action_name, *allocator, &topic_name);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // Action clients are subscriptions on the underlying topics,
  // while action servers are publishers on the underlying topics.
  if (is_client) {
    ret = rcl_get_subscriptions_info_by_topic(node, allocator, topic_name, false, infos);
  } else {
    ret = rcl_get_publishers_info_by_topic(node, allocator, topic_name, false, infos);
  }
  allocator->deallocate(topic_name, allocator->state);
  return ret;
}

static rcl_ret_t
_rcl_action_get_info_by_action(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_endpoint_info_array_t * info_array,
  int is_client)
{
  if (!rcl_node_is_valid(node)) {
    return RCL_RET_NODE_INVALID;  // error already set
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(action_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(info_array, RCL_RET_INVALID_ARGUMENT);
  if (0u != info_array->size || NULL != info_array->info_array) {
    RCL_SET_ERROR_MSG(
      "rcl_action_endpoint_info_array_t must be zero initialized, "
      "use rcl_action_get_zero_initialized_endpoint_info_array");
    return RCL_RET_INVALID_ARGUMENT;
  }

  rcl_service_endpoint_info_array_t goal_infos =
    rcl_get_zero_initialized_service_endpoint_info_array();
  rcl_service_endpoint_info_array_t cancel_infos =
    rcl_get_zero_initialized_service_endpoint_info_array();
  rcl_service_endpoint_info_array_t result_infos =
    rcl_get_zero_initialized_service_endpoint_info_array();
  rcl_topic_endpoint_info_array_t feedback_infos =
    rmw_get_zero_initialized_topic_endpoint_info_array();
  rcl_topic_endpoint_info_array_t status_infos =
    rmw_get_zero_initialized_topic_endpoint_info_array();

  // Query the endpoint information of all the underlying entities of the action.
  rcl_ret_t ret = _rcl_action_get_service_info(
    node, allocator, action_name,
    rcl_action_get_goal_service_name, is_client, &goal_infos);
  if (RCL_RET_OK == ret) {
    ret = _rcl_action_get_service_info(
      node, allocator, action_name,
      rcl_action_get_cancel_service_name, is_client, &cancel_infos);
  }
  if (RCL_RET_OK == ret) {
    ret = _rcl_action_get_service_info(
      node, allocator, action_name,
      rcl_action_get_result_service_name, is_client, &result_infos);
  }
  if (RCL_RET_OK == ret) {
    ret = _rcl_action_get_topic_info(
      node, allocator, action_name,
      rcl_action_get_feedback_topic_name, is_client, &feedback_infos);
  }
  if (RCL_RET_OK == ret) {
    ret = _rcl_action_get_topic_info(
      node, allocator, action_name,
      rcl_action_get_status_topic_name, is_client, &status_infos);
  }

  // The goal service endpoint is the canonical identity of an action client
  // or an action server, correlate the other entities to it by node name and
  // node namespace.
  if (RCL_RET_OK == ret && goal_infos.size > 0u) {
    info_array->info_array = allocator->allocate(
      sizeof(rcl_action_endpoint_info_t) * goal_infos.size, allocator->state);
    if (NULL == info_array->info_array) {
      RCL_SET_ERROR_MSG("Failed to allocate memory for action endpoint info array");
      ret = RCL_RET_BAD_ALLOC;
    } else {
      info_array->size = goal_infos.size;
      for (size_t i = 0u; i < info_array->size; ++i) {
        rcl_action_endpoint_info_t * endpoint_info = &info_array->info_array[i];
        *endpoint_info = rcl_action_get_zero_initialized_endpoint_info();
        // Shallow move: transfer ownership of the allocated members and
        // zero initialize the source so that finalizing the source array
        // does not free them.
        endpoint_info->goal_service_info = goal_infos.info_array[i];
        goal_infos.info_array[i] = rmw_get_zero_initialized_service_endpoint_info();

        const char * node_name = endpoint_info->goal_service_info.node_name;
        const char * node_namespace = endpoint_info->goal_service_info.node_namespace;
        if (NULL == node_name || NULL == node_namespace) {
          continue;
        }
        rmw_service_endpoint_info_t * service_info =
          _find_service_endpoint_info_by_node(&cancel_infos, node_name, node_namespace);
        if (NULL != service_info) {
          endpoint_info->cancel_service_info = *service_info;
          *service_info = rmw_get_zero_initialized_service_endpoint_info();
        }
        service_info =
          _find_service_endpoint_info_by_node(&result_infos, node_name, node_namespace);
        if (NULL != service_info) {
          endpoint_info->result_service_info = *service_info;
          *service_info = rmw_get_zero_initialized_service_endpoint_info();
        }
        rmw_topic_endpoint_info_t * topic_info =
          _find_topic_endpoint_info_by_node(&feedback_infos, node_name, node_namespace);
        if (NULL != topic_info) {
          endpoint_info->feedback_topic_info = *topic_info;
          *topic_info = rmw_get_zero_initialized_topic_endpoint_info();
        }
        topic_info =
          _find_topic_endpoint_info_by_node(&status_infos, node_name, node_namespace);
        if (NULL != topic_info) {
          endpoint_info->status_topic_info = *topic_info;
          *topic_info = rmw_get_zero_initialized_topic_endpoint_info();
        }
      }
    }
  }

  // Cleanup the intermediate arrays.
  // Entries claimed above were zero initialized in the source arrays, so
  // finalizing the source arrays only frees the unclaimed entries.
  if (RMW_RET_OK != rmw_service_endpoint_info_array_fini(&goal_infos, allocator) ||
    RMW_RET_OK != rmw_service_endpoint_info_array_fini(&cancel_infos, allocator) ||
    RMW_RET_OK != rmw_service_endpoint_info_array_fini(&result_infos, allocator) ||
    RMW_RET_OK != rmw_topic_endpoint_info_array_fini(&feedback_infos, allocator) ||
    RMW_RET_OK != rmw_topic_endpoint_info_array_fini(&status_infos, allocator))
  {
    if (RCL_RET_OK == ret) {
      ret = RCL_RET_ERROR;
    }
  }
  if (RCL_RET_OK != ret) {
    rcl_ret_t fini_ret = rcl_action_endpoint_info_array_fini(info_array, allocator);
    if (RCL_RET_OK != fini_ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
        "Freeing action endpoint info array failed while handling a previous error. "
        "Leaking memory!\n");
    }
  }

  return ret;
}

rcl_ret_t
rcl_action_get_clients_info_by_action(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_endpoint_info_array_t * clients_info)
{
  return _rcl_action_get_info_by_action(node, allocator, action_name, clients_info, 1);
}

rcl_ret_t
rcl_action_get_servers_info_by_action(
  const rcl_node_t * node,
  rcutils_allocator_t * allocator,
  const char * action_name,
  rcl_action_endpoint_info_array_t * servers_info)
{
  return _rcl_action_get_info_by_action(node, allocator, action_name, servers_info, 0);
}

#ifdef __cplusplus
}
#endif
