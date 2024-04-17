// Copyright 2018-2019 Open Source Robotics Foundation, Inc.
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

#include "rcl/logging_rosout.h"

#include "rcl/allocator.h"
#include "rcl/common.h"
#include "rcl/error_handling.h"
#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/time.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"
#include "rcl_interfaces/msg/log.h"
#include "rcutils/allocator.h"
#include "rcutils/format_string.h"
#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"
#include "rcutils/types/hash_map.h"
#include "rcutils/types/rcutils_ret.h"
#include "rosidl_runtime_c/string_functions.h"

#define ROSOUT_TOPIC_NAME "/rosout"

typedef struct rosout_map_entry_t
{
  rcl_node_t * node;
  rcl_publisher_t publisher;
} rosout_map_entry_t;

static rcutils_hash_map_t __logger_map;
static bool __is_initialized = false;
static rcl_allocator_t __rosout_allocator;

typedef struct rosout_sublogger_entry_t
{
  // name is to store the allocated memory, and then finalize it at the end
  char * name;
  // count is something like a reference count that removes the entry if it is 0
  uint64_t * count;
} rosout_sublogger_entry_t;

static rcutils_hash_map_t __sublogger_map;

rcl_ret_t rcl_logging_rosout_init(const rcl_allocator_t * allocator)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t status = RCL_RET_OK;
  if (__is_initialized) {
    return RCL_RET_OK;
  }
  __logger_map = rcutils_get_zero_initialized_hash_map();
  status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_init(
      &__logger_map, 2, sizeof(const char *), sizeof(rosout_map_entry_t),
      rcutils_hash_map_string_hash_func, rcutils_hash_map_string_cmp_func, allocator));
  if (RCL_RET_OK != status) {
    return status;
  }

  __sublogger_map = rcutils_get_zero_initialized_hash_map();
  status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_init(
      &__sublogger_map, 2, sizeof(const char *), sizeof(rosout_sublogger_entry_t),
      rcutils_hash_map_string_hash_func, rcutils_hash_map_string_cmp_func, allocator));
  if (RCL_RET_OK != status) {
    rcl_ret_t fini_status = rcl_convert_rcutils_ret_to_rcl_ret(
      rcutils_hash_map_fini(&__logger_map));
    if (RCL_RET_OK != fini_status) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to finalize the hash map for logger: ");
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
      rcl_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    }
    return status;
  }

  __rosout_allocator = *allocator;
  __is_initialized = true;

  return status;
}

static rcl_ret_t
_rcl_logging_rosout_remove_logger_map(rcl_node_t * node)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t status = RCL_RET_OK;
  char * previous_key = NULL;
  char * key = NULL;
  rosout_map_entry_t entry;
  rcutils_ret_t hashmap_ret = rcutils_hash_map_get_next_key_and_data(
    &__logger_map, NULL, &key, &entry);
  while (RCL_RET_OK == status && RCUTILS_RET_OK == hashmap_ret) {
    if (entry.node == node) {
      status = rcl_convert_rcutils_ret_to_rcl_ret(rcutils_hash_map_unset(&__logger_map, &key));
      previous_key = NULL;
    } else {
      previous_key = key;
    }
    if (RCL_RET_OK == status) {
      hashmap_ret = rcutils_hash_map_get_next_key_and_data(
        &__logger_map, previous_key ? &previous_key : NULL, &key, &entry);
    }
  }
  return RCL_RET_OK;
}

static rcl_ret_t
_rcl_logging_rosout_clear_logger_map_item(void * value)
{
  rosout_map_entry_t * entry = (rosout_map_entry_t *)value;
  // Teardown publisher
  rcl_ret_t status = rcl_publisher_fini(&entry->publisher, entry->node);
  if (RCL_RET_OK == status) {
    // delete all entries using this node
    status = rcl_convert_rcutils_ret_to_rcl_ret(
      _rcl_logging_rosout_remove_logger_map(entry->node));
  }

  return status;
}

static rcl_ret_t
_rcl_logging_rosout_clear_sublogger_map_item(void * value)
{
  rosout_sublogger_entry_t * entry = (rosout_sublogger_entry_t *)value;
  rcl_ret_t status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_unset(&__sublogger_map, &entry->name));
  __rosout_allocator.deallocate(entry->name, __rosout_allocator.state);
  __rosout_allocator.deallocate(entry->count, __rosout_allocator.state);

  return status;
}

static rcl_ret_t
_rcl_logging_rosout_clear_hashmap(
  rcutils_hash_map_t * map, rcl_ret_t (* predicate)(void *), void * entry)
{
  rcl_ret_t status = RCL_RET_OK;
  char * key = NULL;

  rcutils_ret_t hashmap_ret = rcutils_hash_map_get_next_key_and_data(
    map, NULL, &key, entry);
  while (RCUTILS_RET_OK == hashmap_ret) {
    status = predicate(entry);
    if (RCL_RET_OK != status) {
      break;
    }

    hashmap_ret = rcutils_hash_map_get_next_key_and_data(map, NULL, &key, entry);
  }
  if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES != hashmap_ret) {
    status = rcl_convert_rcutils_ret_to_rcl_ret(hashmap_ret);
  }

  if (RCL_RET_OK == status) {
    status = rcl_convert_rcutils_ret_to_rcl_ret(rcutils_hash_map_fini(map));
  }

  return status;
}

rcl_ret_t rcl_logging_rosout_fini(void)
{
  if (!__is_initialized) {
    return RCL_RET_OK;
  }
  rcl_ret_t status = RCL_RET_OK;
  rosout_map_entry_t entry;
  rosout_sublogger_entry_t sublogger_entry;

  status = _rcl_logging_rosout_clear_hashmap(
    &__logger_map, _rcl_logging_rosout_clear_logger_map_item, &entry);
  if (RCL_RET_OK != status) {
    return status;
  }

  status = _rcl_logging_rosout_clear_hashmap(
    &__sublogger_map, _rcl_logging_rosout_clear_sublogger_map_item, &sublogger_entry);
  if (RCL_RET_OK != status) {
    return status;
  }

  __is_initialized = false;

  return status;
}

rcl_ret_t rcl_logging_rosout_init_publisher_for_node(rcl_node_t * node)
{
  if (!__is_initialized) {
    return RCL_RET_OK;
  }

  const char * logger_name = NULL;
  rosout_map_entry_t new_entry;
  rcl_ret_t status = RCL_RET_OK;

  // Verify input and make sure it's not already initialized
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_NODE_INVALID);
  logger_name = rcl_node_get_logger_name(node);
  if (NULL == logger_name) {
    RCL_SET_ERROR_MSG("Logger name was null.");
    return RCL_RET_ERROR;
  }
  if (rcutils_hash_map_key_exists(&__logger_map, &logger_name)) {
    // @TODO(nburek) Update behavior to either enforce unique names or work with non-unique
    // names based on the outcome here: https://github.com/ros2/design/issues/187
    const char * node_name = rcl_node_get_name(node);
    if (NULL == node_name) {
      node_name = "unknown node";
    }

    RCUTILS_LOG_WARN_NAMED(
      "rcl.logging_rosout",
      "Publisher already registered for node name: '%s'. If this is due to multiple nodes "
      "with the same name then all logs for the logger named '%s' will go out over "
      "the existing publisher. As soon as any node with that name is destructed "
      "it will unregister the publisher, preventing any further logs for that name from "
      "being published on the rosout topic.",
      node_name,
      logger_name);
    return RCL_RET_OK;
  }

  // Create a new Log message publisher on the node
  const rosidl_message_type_support_t * type_support =
    rosidl_typesupport_c__get_message_type_support_handle__rcl_interfaces__msg__Log();
  rcl_publisher_options_t options = rcl_publisher_get_default_options();

  // Late joining subscriptions get the user's setting of rosout qos options.
  const rcl_node_options_t * node_options = rcl_node_get_options(node);
  RCL_CHECK_FOR_NULL_WITH_MSG(node_options, "Node options was null.", return RCL_RET_ERROR);

  options.qos = node_options->rosout_qos;
  options.allocator = node_options->allocator;
  new_entry.publisher = rcl_get_zero_initialized_publisher();
  status =
    rcl_publisher_init(&new_entry.publisher, node, type_support, ROSOUT_TOPIC_NAME, &options);

  // Add the new publisher to the map
  if (RCL_RET_OK == status) {
    new_entry.node = node;
    status = rcl_convert_rcutils_ret_to_rcl_ret(
      rcutils_hash_map_set(&__logger_map, &logger_name, &new_entry));
    if (RCL_RET_OK != status) {
      RCL_SET_ERROR_MSG("Failed to add publisher to map.");
      // We failed to add to the map so destroy the publisher that we created
      rcl_ret_t fini_status = rcl_publisher_fini(&new_entry.publisher, new_entry.node);
      // ignore the return status in favor of the failure from set
      RCL_UNUSED(fini_status);
    }
  }

  return status;
}

rcl_ret_t rcl_logging_rosout_fini_publisher_for_node(rcl_node_t * node)
{
  if (!__is_initialized) {
    return RCL_RET_OK;
  }

  rosout_map_entry_t entry;
  const char * logger_name = NULL;
  rcl_ret_t status = RCL_RET_OK;

  // Verify input and make sure it's initialized
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_NODE_INVALID);
  logger_name = rcl_node_get_logger_name(node);
  if (NULL == logger_name) {
    return RCL_RET_ERROR;
  }
  if (!rcutils_hash_map_key_exists(&__logger_map, &logger_name)) {
    return RCL_RET_OK;
  }

  // fini the publisher and remove the entry from the map
  status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_get(&__logger_map, &logger_name, &entry));
  if (RCL_RET_OK == status && node == entry.node) {
    status = rcl_publisher_fini(&entry.publisher, entry.node);
  }
  if (RCL_RET_OK == status) {
    // delete all entries using this node
    status = rcl_convert_rcutils_ret_to_rcl_ret(_rcl_logging_rosout_remove_logger_map(entry.node));
  }

  return status;
}

static void shallow_assign(rosidl_runtime_c__String * target, const char * source)
{
  target->data = (char *)source;
  size_t len = strlen(source);
  target->size = len;
  target->capacity = len + 1;
}

void rcl_logging_rosout_output_handler(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  rcutils_time_point_value_t timestamp,
  const char * format,
  va_list * args)
{
  rosout_map_entry_t entry;
  rcl_ret_t status = RCL_RET_OK;
  if (!__is_initialized) {
    return;
  }
  rcutils_ret_t rcutils_ret = rcutils_hash_map_get(&__logger_map, &name, &entry);
  if (RCUTILS_RET_OK == rcutils_ret) {
    char msg_buf[1024] = "";
    rcutils_char_array_t msg_array = {
      .buffer = msg_buf,
      .owns_buffer = false,
      .buffer_length = 0u,
      .buffer_capacity = sizeof(msg_buf),
      .allocator = __rosout_allocator
    };

    status = rcl_convert_rcutils_ret_to_rcl_ret(
      rcutils_char_array_vsprintf(&msg_array, format, *args));
    if (RCL_RET_OK != status) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to format log string: ");
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
      rcl_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    } else {
      rcl_interfaces__msg__Log log_message;
      log_message.stamp.sec = (int32_t) RCL_NS_TO_S(timestamp);
      log_message.stamp.nanosec = (timestamp % RCL_S_TO_NS(1));
      log_message.level = severity;
      log_message.line = (int32_t) location->line_number;
      shallow_assign(&log_message.name, name);
      shallow_assign(&log_message.msg, msg_array.buffer);
      shallow_assign(&log_message.file, location->file_name);
      shallow_assign(&log_message.function, location->function_name);
      status = rcl_publish(&entry.publisher, &log_message, NULL);
      if (RCL_RET_OK != status) {
        RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to publish log message to rosout: ");
        RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
        rcl_reset_error();
        RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
      }
    }

    status = rcl_convert_rcutils_ret_to_rcl_ret(rcutils_char_array_fini(&msg_array));
    if (RCL_RET_OK != status) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("failed to fini char_array: ");
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
      rcl_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    }
  }
}

static rcl_ret_t
_rcl_logging_rosout_get_full_sublogger_name(
  const char * logger_name, const char * sublogger_name, char ** full_sublogger_name)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(logger_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(sublogger_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(full_sublogger_name, RCL_RET_INVALID_ARGUMENT);

  if (logger_name[0] == '\0' || sublogger_name[0] == '\0') {
    RCL_SET_ERROR_MSG("logger name or sub-logger name can't be empty.");
    return RCL_RET_INVALID_ARGUMENT;
  }

  *full_sublogger_name = rcutils_format_string(
    __rosout_allocator, "%s%s%s",
    logger_name, RCUTILS_LOGGING_SEPARATOR_STRING, sublogger_name);
  if (NULL == *full_sublogger_name) {
    RCL_SET_ERROR_MSG("Failed to allocate a full sublogger name.");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

rcl_ret_t
rcl_logging_rosout_add_sublogger(
  const char * logger_name, const char * sublogger_name)
{
  if (!__is_initialized) {
    return RCL_RET_OK;
  }

  rcl_ret_t status = RCL_RET_OK;
  char * full_sublogger_name = NULL;
  uint64_t * sublogger_count = NULL;
  rosout_map_entry_t entry;
  rosout_sublogger_entry_t sublogger_entry;

  RCL_CHECK_ARGUMENT_FOR_NULL(logger_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(sublogger_name, RCL_RET_INVALID_ARGUMENT);
  rcutils_ret_t rcutils_ret = rcutils_hash_map_get(&__logger_map, &logger_name, &entry);
  if (RCUTILS_RET_OK != rcutils_ret) {
    if (RCUTILS_RET_NOT_FOUND == rcutils_ret) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to get logger entry for '%s'.", logger_name);
    }
    return rcl_convert_rcutils_ret_to_rcl_ret(rcutils_ret);
  }

  status =
    _rcl_logging_rosout_get_full_sublogger_name(logger_name, sublogger_name, &full_sublogger_name);
  if (RCL_RET_OK != status) {
    // Error already set
    return status;
  }

  if (rcutils_hash_map_key_exists(&__logger_map, &full_sublogger_name)) {
    // To get the entry and increase the reference count
    status = rcl_convert_rcutils_ret_to_rcl_ret(
      rcutils_hash_map_get(&__sublogger_map, &full_sublogger_name, &sublogger_entry));
    if (RCL_RET_OK != status) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to get item from sublogger map for '%s'.", full_sublogger_name);
      goto cleanup;
    }
    *sublogger_entry.count += 1;
    goto cleanup;
  }

  status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_set(&__logger_map, &full_sublogger_name, &entry));
  if (RCL_RET_OK != status) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Failed to add publisher to map for logger '%s'.", full_sublogger_name);
    goto cleanup;
  }

  sublogger_entry.name = full_sublogger_name;
  sublogger_count = __rosout_allocator.allocate(sizeof(uint64_t), __rosout_allocator.state);
  if (!sublogger_count) {
    RCL_SET_ERROR_MSG(
      "Failed to allocate memory for count of sublogger entry.");
    goto cleanup;
  }
  sublogger_entry.count = sublogger_count;
  *sublogger_entry.count = 1;

  status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_set(&__sublogger_map, &full_sublogger_name, &sublogger_entry));
  if (RCL_RET_OK != status) {
    // revert the previor set operation for __logger_map
    rcutils_ret_t rcutils_ret = rcutils_hash_map_unset(&__logger_map, &full_sublogger_name);
    if (RCUTILS_RET_OK != rcutils_ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("failed to unset hashmap: ");
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
      rcl_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    }
    goto cleanup_count;
  }

  return status;

cleanup_count:
  __rosout_allocator.deallocate(sublogger_count, __rosout_allocator.state);
cleanup:
  __rosout_allocator.deallocate(full_sublogger_name, __rosout_allocator.state);
  return status;
}

rcl_ret_t
rcl_logging_rosout_remove_sublogger(
  const char * logger_name, const char * sublogger_name)
{
  if (!__is_initialized) {
    return RCL_RET_OK;
  }

  rcl_ret_t status = RCL_RET_OK;
  char * full_sublogger_name = NULL;
  rosout_sublogger_entry_t sublogger_entry;
  RCL_CHECK_ARGUMENT_FOR_NULL(logger_name, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(sublogger_name, RCL_RET_INVALID_ARGUMENT);

  status =
    _rcl_logging_rosout_get_full_sublogger_name(logger_name, sublogger_name, &full_sublogger_name);
  if (RCL_RET_OK != status) {
    // Error already set
    return status;
  }

  if (!rcutils_hash_map_key_exists(&__logger_map, &full_sublogger_name)) {
    status = RCL_RET_NOT_FOUND;
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Logger for '%s' not found.", full_sublogger_name);
    goto cleanup;
  }

  // remove the entry from the map
  status = rcl_convert_rcutils_ret_to_rcl_ret(
    rcutils_hash_map_get(&__sublogger_map, &full_sublogger_name, &sublogger_entry));
  if (RCL_RET_OK != status) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Failed to get item from sublogger map for '%s'.", full_sublogger_name);
    goto cleanup;
  }

  *sublogger_entry.count -= 1;
  if (*sublogger_entry.count == 0) {
    status = rcl_convert_rcutils_ret_to_rcl_ret(
      rcutils_hash_map_unset(&__logger_map, &full_sublogger_name));
    if (RCL_RET_OK == status) {
      status = rcl_convert_rcutils_ret_to_rcl_ret(
        rcutils_hash_map_unset(&__sublogger_map, &full_sublogger_name));
      __rosout_allocator.deallocate(sublogger_entry.name, __rosout_allocator.state);
      __rosout_allocator.deallocate(sublogger_entry.count, __rosout_allocator.state);
    }
  }

cleanup:
  __rosout_allocator.deallocate(full_sublogger_name, __rosout_allocator.state);
  return status;
}
