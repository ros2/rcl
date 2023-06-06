// Copyright 2023 eSOL Co.,Ltd.
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

#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "rcl_yaml_param_parser/parser_thread_attr.h"

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/format_string.h"
#include "rcutils/strdup.h"
#include "rcutils/types/rcutils_ret.h"

#include "./impl/parse.h"  // to use get_value
#include "./impl/parse_thread_attr.h"

#define PARSE_WITH_CHECK_ERROR(out_event) \
  do { \
    int _parse_ret_; \
    _parse_ret_ = yaml_parser_parse(parser, (out_event)); \
    if (0 == _parse_ret_) { \
      RCUTILS_SET_ERROR_MSG("Failed to parse thread attributes"); \
      ret = RCUTILS_RET_ERROR; \
      goto error; \
    } \
  } while (0)

#define PARSE_WITH_EVENT_CHECK(event_type) \
  do { \
    yaml_event_t _event_; \
    int _parse_ret_; \
    _parse_ret_ = yaml_parser_parse(parser, &_event_); \
    if (0 == _parse_ret_) { \
      RCUTILS_SET_ERROR_MSG("Failed to parse thread attributes"); \
      ret = RCUTILS_RET_ERROR; \
      goto error; \
    } \
    if (_event_.type != (event_type)) { \
      RCUTILS_SET_ERROR_MSG("Unexpected element in a configuration of thread attributes"); \
      ret = RCUTILS_RET_ERROR; \
      goto error; \
    } \
  } while (0)

///
/// Parse the key part of a thread attribute
///
rcutils_ret_t
parse_thread_attr_key(
  const char * str,
  thread_attr_key_type_t * key_type)
{
  rcutils_ret_t ret;
  if (strcmp(str, "core_affinity") == 0) {
    *key_type = THREAD_ATTR_KEY_CORE_AFFINITY;
  } else if (strcmp(str, "priority") == 0) {
    *key_type = THREAD_ATTR_KEY_PRIORITY;
  } else if (strcmp(str, "scheduling_policy") == 0) {
    *key_type = THREAD_ATTR_KEY_SCHEDULING_POLICY;
  } else if (strcmp(str, "name") == 0) {
    *key_type = THREAD_ATTR_KEY_NAME;
  } else if (*str == '\0') {
    RCUTILS_SET_ERROR_MSG("empty name for a thread attribute");
    ret = RCUTILS_RET_ERROR;
    goto error;
  } else {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("unrecognized key for a thread attribute: %s", str);
    ret = RCUTILS_RET_ERROR;
    goto error;
  }
  return RCUTILS_RET_OK;

error:
  return ret;
}

///
/// Parse the value of the scheduling policy of a thread attribute
///
rcl_thread_scheduling_policy_type_t parse_thread_attr_scheduling_policy(
  const char * value)
{
  rcl_thread_scheduling_policy_type_t ret;
  if (strcmp(value, "FIFO") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_FIFO;
  } else if (strcmp(value, "RR") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_RR;
  } else if (strcmp(value, "SPORADIC") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_SPORADIC;
  } else if (strcmp(value, "OTHER") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_OTHER;
  } else if (strcmp(value, "IDLE") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_IDLE;
  } else if (strcmp(value, "BATCH") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_BATCH;
  } else if (strcmp(value, "DEADLINE") == 0) {
    ret = RCL_THREAD_SCHEDULING_POLICY_DEADLINE;
  } else {
    ret = RCL_THREAD_SCHEDULING_POLICY_UNKNOWN;
  }
  return ret;
}

///
/// parse a thread attribute YAML value string and process them
///
rcutils_ret_t parse_thread_attr(
  yaml_parser_t * parser,
  rcl_thread_attr_t * attr,
  rcutils_allocator_t allocator)
{
  rcutils_ret_t ret;
  yaml_event_t event;
  thread_attr_key_bits_t key_bits = THREAD_ATTR_KEY_BITS_NONE;

  while (1) {
    PARSE_WITH_CHECK_ERROR(&event);

    if (YAML_MAPPING_END_EVENT == event.type) {
      break;
    }
    if (YAML_SCALAR_EVENT != event.type) {
      RCUTILS_SET_ERROR_MSG("Unexpected element in a configuration of thread attributes");
      ret = RCUTILS_RET_ERROR;
      goto error;
    }

    const char * str = (char *)event.data.scalar.value;
    thread_attr_key_type_t key_type;
    ret = parse_thread_attr_key(str, &key_type);
    if (RCUTILS_RET_OK != ret) {
      goto error;
    }
    if (key_bits & (thread_attr_key_bits_t)key_type) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("duplicated key for a thread attribute: %s", str);
      ret = RCUTILS_RET_ERROR;
      goto error;
    }

    PARSE_WITH_CHECK_ERROR(&event);

    const char * value = (char *)event.data.scalar.value;
    yaml_scalar_style_t style = event.data.scalar.style;
    const yaml_char_t * tag = event.data.scalar.tag;
    data_types_t val_type;
    void * ret_val = NULL;

    switch (key_type) {
      case THREAD_ATTR_KEY_CORE_AFFINITY:
        ret_val = get_value(value, style, tag, &val_type, allocator);
        if (DATA_TYPE_INT64 != val_type) {
          RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Unrecognized value for thread core affinity: %s", value);
          ret = RCUTILS_RET_ERROR;
          goto error;
        }
        attr->core_affinity = ((int)*(int64_t *)(ret_val));
        break;
      case THREAD_ATTR_KEY_PRIORITY:
        ret_val = get_value(value, style, tag, &val_type, allocator);
        if (DATA_TYPE_INT64 != val_type) {
          RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Unrecognized value for thread priority: %s", value);
          ret = RCUTILS_RET_ERROR;
          goto error;
        }
        attr->priority = ((int)*(int64_t *)(ret_val));
        break;
      case THREAD_ATTR_KEY_SCHEDULING_POLICY:
        attr->scheduling_policy = parse_thread_attr_scheduling_policy(value);
        break;
      case THREAD_ATTR_KEY_NAME:
        if (*value == '\0') {
          RCUTILS_SET_ERROR_MSG("Empty thread name");
          ret = RCUTILS_RET_ERROR;
          goto error;
        }
        attr->name = rcutils_strdup(value, allocator);
        if (NULL == attr->name) {
          ret = RCUTILS_RET_BAD_ALLOC;
          goto error;
        }
        break;
    }

    if (NULL != ret_val) {
      allocator.deallocate(ret_val, allocator.state);
    }

    key_bits |= (thread_attr_key_bits_t)key_type;
  }

  if (THREAD_ATTR_KEY_BITS_ALL != key_bits) {
    RCUTILS_SET_ERROR_MSG("A thread attribute does not have enough parameters");
    ret = RCUTILS_RET_ERROR;
    goto error;
  }

  return RCUTILS_RET_OK;

error:
  if (NULL != attr->name) {
    allocator.deallocate(attr->name, allocator.state);
  }
  return ret;
}

static inline rcutils_ret_t extend_thread_attrs_capacity(
  rcl_thread_attrs_t * attrs,
  size_t new_cap)
{
  size_t cap = attrs->capacity_attributes;
  size_t size = cap * sizeof(rcl_thread_attr_t);
  size_t new_size = new_cap * sizeof(rcl_thread_attr_t);
  rcl_thread_attr_t * new_attrs = attrs->allocator.reallocate(
    attrs->attributes, new_size, attrs->allocator.state);

  if (NULL == new_attrs) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for thread attributes");
    return RCUTILS_RET_BAD_ALLOC;
  }

  memset(new_attrs + cap, 0, new_size - size);

  attrs->capacity_attributes = new_cap;
  attrs->attributes = new_attrs;

  return RCUTILS_RET_OK;
}

///
/// Get events from parsing thread attributes YAML value string and process them
///
rcutils_ret_t parse_thread_attr_events(
  yaml_parser_t * parser,
  rcl_thread_attrs_t * thread_attrs)
{
  yaml_event_t event;
  rcutils_ret_t ret;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(parser, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);

  PARSE_WITH_EVENT_CHECK(YAML_STREAM_START_EVENT);
  PARSE_WITH_EVENT_CHECK(YAML_DOCUMENT_START_EVENT);
  PARSE_WITH_EVENT_CHECK(YAML_SEQUENCE_START_EVENT);

  while (1) {
    PARSE_WITH_CHECK_ERROR(&event);

    if (YAML_SEQUENCE_END_EVENT == event.type) {
      break;
    } else if (YAML_MAPPING_START_EVENT != event.type) {
      RCUTILS_SET_ERROR_MSG("Unexpected element in a configuration of thread attributes");
      ret = RCUTILS_RET_ERROR;
      goto error;
    }

    if (thread_attrs->num_attributes == thread_attrs->capacity_attributes) {
      size_t new_cap = 0;
      if (0 == thread_attrs->capacity_attributes) {
        new_cap = 1;
      } else {
        new_cap = thread_attrs->capacity_attributes * 2;
      }
      // Extend the capacity
      ret = extend_thread_attrs_capacity(thread_attrs, new_cap);
      if (RCUTILS_RET_OK != ret) {
        goto error;
      }
    }

    rcl_thread_attr_t * attr = thread_attrs->attributes + thread_attrs->num_attributes;
    ret = parse_thread_attr(parser, attr, thread_attrs->allocator);
    if (RCUTILS_RET_OK != ret) {
      goto error;
    }

    ++thread_attrs->num_attributes;
  }
  PARSE_WITH_EVENT_CHECK(YAML_DOCUMENT_END_EVENT);
  PARSE_WITH_EVENT_CHECK(YAML_STREAM_END_EVENT);

  if (0 == thread_attrs->num_attributes) {
    RCUTILS_SET_ERROR_MSG("No thread attributes.");
    ret = RCUTILS_RET_ERROR;
    goto error;
  }

  return RCUTILS_RET_OK;

error:
  if (0 < thread_attrs->capacity_attributes) {
    rcl_thread_attrs_fini(thread_attrs);
  }
  return ret;
}
