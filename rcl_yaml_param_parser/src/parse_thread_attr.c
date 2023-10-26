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
#include "rcutils/thread_attr.h"

#include "./impl/parse.h"  // to use get_value
#include "./impl/parse_thread_attr.h"

#define PARSE_WITH_CHECK_ERROR() \
  do { \
    int _parse_ret_; \
    _parse_ret_ = yaml_parser_parse(parser, &event); \
    if (0 == _parse_ret_) { \
      RCUTILS_SET_ERROR_MSG("Failed to parse thread attributes"); \
      ret = RCUTILS_RET_ERROR; \
      goto error; \
    } \
  } while (0)

#define PARSE_WITH_CHECK_EVENT(event_type) \
  do { \
    PARSE_WITH_CHECK_ERROR(); \
    if (event.type != (event_type)) { \
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
rcutils_thread_scheduling_policy_t parse_thread_attr_scheduling_policy(
  const char * value)
{
  rcutils_thread_scheduling_policy_t ret;
  if (strcmp(value, "FIFO") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_FIFO;
  } else if (strcmp(value, "RR") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_RR;
  } else if (strcmp(value, "SPORADIC") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_SPORADIC;
  } else if (strcmp(value, "OTHER") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_OTHER;
  } else if (strcmp(value, "IDLE") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_IDLE;
  } else if (strcmp(value, "BATCH") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_BATCH;
  } else if (strcmp(value, "DEADLINE") == 0) {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_DEADLINE;
  } else {
    ret = RCUTILS_THREAD_SCHEDULING_POLICY_UNKNOWN;
  }
  return ret;
}

///
/// parse a thread attribute YAML value string and process them
///
rcutils_ret_t parse_thread_attr(
  yaml_parser_t * parser,
  rcutils_thread_attrs_t * attrs)
{
  rcutils_ret_t ret;
  yaml_event_t event;
  thread_attr_key_bits_t key_bits = THREAD_ATTR_KEY_BITS_NONE;
  rcutils_thread_scheduling_policy_t sched_policy;
  rcutils_thread_core_affinity_t core_affinity =
    rcutils_get_zero_initialized_thread_core_affinity();
  int priority;
  char const * name = NULL;
  rcutils_allocator_t allocator = attrs->allocator;
  void * ret_val = NULL;

  while (1) {
    PARSE_WITH_CHECK_ERROR();

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

    PARSE_WITH_CHECK_ERROR();

    const char * value = (char *)event.data.scalar.value;
    yaml_scalar_style_t style = event.data.scalar.style;
    const yaml_char_t * tag = event.data.scalar.tag;
    data_types_t val_type;

    switch (key_type) {
      case THREAD_ATTR_KEY_CORE_AFFINITY:
        if (event.type != YAML_SEQUENCE_START_EVENT) {
          ret = RCUTILS_RET_ERROR;
          goto error;
        }
        ret = rcutils_thread_core_affinity_init(&core_affinity, 0, allocator);
        if (RCUTILS_RET_OK != ret) {
          goto error;
        }
        while (1) {
          PARSE_WITH_CHECK_ERROR();
          if (YAML_SEQUENCE_END_EVENT == event.type) {
            break;
          }
          value = (char *)event.data.scalar.value;
          style = event.data.scalar.style;
          tag = event.data.scalar.tag;
          ret_val = get_value(value, style, tag, &val_type, allocator);
          if (DATA_TYPE_INT64 != val_type) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Unrecognized value for thread core affinity: %s", value);
            ret = RCUTILS_RET_ERROR;
            goto error;
          }
          size_t core_no = ((size_t)*(int64_t *)(ret_val));
          allocator.deallocate(ret_val, allocator.state);
          ret_val = NULL;
          ret = rcutils_thread_core_affinity_set(&core_affinity, core_no);
          if (RCUTILS_RET_OK != ret) {
            goto error;
          }
        }
        break;
      case THREAD_ATTR_KEY_PRIORITY:
        if (event.type != YAML_SCALAR_EVENT) {
          goto error;
        }
        ret_val = get_value(value, style, tag, &val_type, allocator);
        if (DATA_TYPE_INT64 != val_type) {
          RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Unrecognized value for thread priority: %s", value);
          ret = RCUTILS_RET_ERROR;
          goto error;
        }
        priority = ((int)*(int64_t *)(ret_val));
        break;
      case THREAD_ATTR_KEY_SCHEDULING_POLICY:
        if (event.type != YAML_SCALAR_EVENT) {
          goto error;
        }
        sched_policy = parse_thread_attr_scheduling_policy(value);
        break;
      case THREAD_ATTR_KEY_NAME:
        if (event.type != YAML_SCALAR_EVENT) {
          goto error;
        }
        if (*value == '\0') {
          RCUTILS_SET_ERROR_MSG("Empty thread name");
          ret = RCUTILS_RET_ERROR;
          goto error;
        }
        name = rcutils_strdup(value, allocator);
        if (NULL == name) {
          ret = RCUTILS_RET_BAD_ALLOC;
          goto error;
        }
        break;
    }

    if (NULL != ret_val) {
      allocator.deallocate(ret_val, allocator.state);
      ret_val = NULL;
    }

    key_bits |= (thread_attr_key_bits_t)key_type;
  }

  if (THREAD_ATTR_KEY_BITS_ALL != key_bits) {
    RCUTILS_SET_ERROR_MSG("A thread attribute does not have enough parameters");
    ret = RCUTILS_RET_ERROR;
    goto error;
  }

  ret = rcutils_thread_attrs_add_attr(attrs, sched_policy, &core_affinity, priority, name);
  if (RCUTILS_RET_OK != ret) {
    goto error;
  }

  allocator.deallocate((char *)name, allocator.state);

  return RCUTILS_RET_OK;

error:
  if (NULL != name) {
    allocator.deallocate((char *)name, allocator.state);
  }
  if (NULL != ret_val) {
    allocator.deallocate(ret_val, allocator.state);
  }
  if (0 < core_affinity.core_count) {
    rcutils_ret_t tmp_ret =
      rcutils_thread_core_affinity_fini(&core_affinity);
    (void)tmp_ret;
  }
  return ret;
}

///
/// Get events from parsing thread attributes YAML value string and process them
///
rcutils_ret_t parse_thread_attr_events(
  yaml_parser_t * parser,
  rcutils_thread_attrs_t * thread_attrs)
{
  yaml_event_t event;
  rcutils_ret_t ret;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(parser, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);

  PARSE_WITH_CHECK_EVENT(YAML_STREAM_START_EVENT);
  PARSE_WITH_CHECK_EVENT(YAML_DOCUMENT_START_EVENT);
  PARSE_WITH_CHECK_EVENT(YAML_SEQUENCE_START_EVENT);

  while (1) {
    PARSE_WITH_CHECK_ERROR();

    if (YAML_SEQUENCE_END_EVENT == event.type) {
      break;
    } else if (YAML_MAPPING_START_EVENT != event.type) {
      RCUTILS_SET_ERROR_MSG("Unexpected element in a configuration of thread attributes");
      ret = RCUTILS_RET_ERROR;
      goto error;
    }

    ret = parse_thread_attr(parser, thread_attrs);
    if (RCUTILS_RET_OK != ret) {
      goto error;
    }
  }
  PARSE_WITH_CHECK_EVENT(YAML_DOCUMENT_END_EVENT);
  PARSE_WITH_CHECK_EVENT(YAML_STREAM_END_EVENT);

  if (0 == thread_attrs->num_attributes) {
    RCUTILS_SET_ERROR_MSG("No thread attributes.");
    ret = RCUTILS_RET_ERROR;
    goto error;
  }

  return RCUTILS_RET_OK;

error:
  if (0 < thread_attrs->capacity_attributes) {
    rcutils_ret_t thread_attrs_ret = rcutils_thread_attrs_fini(thread_attrs);
    (void)thread_attrs_ret;
    // Since an error has already occurred, ignore this result.
  }
  return ret;
}
