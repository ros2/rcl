// Copyright 2018 Apex.AI, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/format_string.h"
#include "rcutils/strdup.h"

#include "./impl/add_to_arrays.h"
#include "./impl/parse.h"
#include "./impl/namespace.h"
#include "./impl/node_params.h"
#include "rcl_yaml_param_parser/parser.h"

///
/// Determine the type of the value and return the converted value
/// NOTE: Only canonical forms supported as of now
///
void * get_value(
  const char * const value,
  yaml_scalar_style_t style,
  data_types_t * val_type,
  const rcutils_allocator_t allocator)
{
  void * ret_val;
  int64_t ival;
  double dval;
  char * endptr = NULL;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(value, NULL);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(val_type, NULL);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "allocator is invalid", return NULL);

  /// Check if it is bool
  if (style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
    style != YAML_DOUBLE_QUOTED_SCALAR_STYLE)
  {
    if ((0 == strcmp(value, "Y")) ||
      (0 == strcmp(value, "y")) ||
      (0 == strcmp(value, "yes")) ||
      (0 == strcmp(value, "Yes")) ||
      (0 == strcmp(value, "YES")) ||
      (0 == strcmp(value, "true")) ||
      (0 == strcmp(value, "True")) ||
      (0 == strcmp(value, "TRUE")) ||
      (0 == strcmp(value, "on")) ||
      (0 == strcmp(value, "On")) ||
      (0 == strcmp(value, "ON")))
    {
      *val_type = DATA_TYPE_BOOL;
      ret_val = allocator.zero_allocate(1U, sizeof(bool), allocator.state);
      if (NULL == ret_val) {
        return NULL;
      }
      *((bool *)ret_val) = true;
      return ret_val;
    }

    if ((0 == strcmp(value, "N")) ||
      (0 == strcmp(value, "n")) ||
      (0 == strcmp(value, "no")) ||
      (0 == strcmp(value, "No")) ||
      (0 == strcmp(value, "NO")) ||
      (0 == strcmp(value, "false")) ||
      (0 == strcmp(value, "False")) ||
      (0 == strcmp(value, "FALSE")) ||
      (0 == strcmp(value, "off")) ||
      (0 == strcmp(value, "Off")) ||
      (0 == strcmp(value, "OFF")))
    {
      *val_type = DATA_TYPE_BOOL;
      ret_val = allocator.zero_allocate(1U, sizeof(bool), allocator.state);
      if (NULL == ret_val) {
        return NULL;
      }
      *((bool *)ret_val) = false;
      return ret_val;
    }
  }

  /// Check for int
  if (style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
    style != YAML_DOUBLE_QUOTED_SCALAR_STYLE)
  {
    errno = 0;
    ival = strtol(value, &endptr, 0);
    if ((0 == errno) && (NULL != endptr)) {
      if ((NULL != endptr) && (endptr != value)) {
        if (('\0' != *value) && ('\0' == *endptr)) {
          *val_type = DATA_TYPE_INT64;
          ret_val = allocator.zero_allocate(1U, sizeof(int64_t), allocator.state);
          if (NULL == ret_val) {
            return NULL;
          }
          *((int64_t *)ret_val) = ival;
          return ret_val;
        }
      }
    }
  }

  /// Check for float
  if (style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
    style != YAML_DOUBLE_QUOTED_SCALAR_STYLE)
  {
    errno = 0;
    endptr = NULL;
    const char * iter_ptr = NULL;
    if ((0 == strcmp(value, ".nan")) ||
      (0 == strcmp(value, ".NaN")) ||
      (0 == strcmp(value, ".NAN")) ||
      (0 == strcmp(value, ".inf")) ||
      (0 == strcmp(value, ".Inf")) ||
      (0 == strcmp(value, ".INF")) ||
      (0 == strcmp(value, "+.inf")) ||
      (0 == strcmp(value, "+.Inf")) ||
      (0 == strcmp(value, "+.INF")) ||
      (0 == strcmp(value, "-.inf")) ||
      (0 == strcmp(value, "-.Inf")) ||
      (0 == strcmp(value, "-.INF")))
    {
      for (iter_ptr = value; !isalpha(*iter_ptr); ) {
        iter_ptr += 1;
      }
      dval = strtod(iter_ptr, &endptr);
      if (*value == '-') {
        dval = -dval;
      }
    } else {
      dval = strtod(value, &endptr);
    }
    if ((0 == errno) && (NULL != endptr)) {
      if ((NULL != endptr) && (endptr != value)) {
        if (('\0' != *value) && ('\0' == *endptr)) {
          *val_type = DATA_TYPE_DOUBLE;
          ret_val = allocator.zero_allocate(1U, sizeof(double), allocator.state);
          if (NULL == ret_val) {
            return NULL;
          }
          *((double *)ret_val) = dval;
          return ret_val;
        }
      }
    }
    errno = 0;
  }

  /// It is a string
  *val_type = DATA_TYPE_STRING;
  ret_val = rcutils_strdup(value, allocator);
  return ret_val;
}

///
/// Parse the value part of the <key:value> pair
///
rcutils_ret_t parse_value(
  const yaml_event_t event,
  const bool is_seq,
  rcl_node_params_t * node_params_st,
  const char * parameter_name,
  data_types_t * seq_data_type,
  rcl_params_t * params_st)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_params_st, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(parameter_name, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(seq_data_type, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(params_st, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_allocator_t allocator = params_st->allocator;
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  const size_t val_size = event.data.scalar.length;
  const char * value = (char *)event.data.scalar.value;
  yaml_scalar_style_t style = event.data.scalar.style;
  const uint32_t line_num = ((uint32_t)(event.start_mark.line) + 1U);

  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    value, "event argument has no value", return RCUTILS_RET_INVALID_ARGUMENT);

  if (style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
    style != YAML_DOUBLE_QUOTED_SCALAR_STYLE &&
    0U == val_size)
  {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("No value at line %d", line_num);
    return RCUTILS_RET_ERROR;
  }

  rcutils_ret_t ret;
  rcl_variant_t * param_value = NULL;
  ret = find_parameter(node_params_st, parameter_name, &param_value, allocator);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SET_ERROR_MSG("Internal error: find_parameter");
    return ret;
  }

  data_types_t val_type;
  void * ret_val = get_value(value, style, &val_type, allocator);
  if (NULL == ret_val) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error parsing value %s at line %d", value, line_num);
    return RCUTILS_RET_ERROR;
  }

  ret = RCUTILS_RET_OK;
  switch (val_type) {
    case DATA_TYPE_UNKNOWN:
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Unknown data type of value %s at line %d\n", value, line_num);
      ret = RCUTILS_RET_ERROR;
      break;
    case DATA_TYPE_BOOL:
      if (!is_seq) {
        if (NULL != param_value->bool_value) {
          // Overwriting, deallocate original
          allocator.deallocate(param_value->bool_value, allocator.state);
        }
        param_value->bool_value = (bool *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          *seq_data_type = val_type;
          if (NULL != param_value->bool_array_value) {
            allocator.deallocate(param_value->bool_array_value->values, allocator.state);
            allocator.deallocate(param_value->bool_array_value, allocator.state);
            param_value->bool_array_value = NULL;
          }
          param_value->bool_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_bool_array_t), allocator.state);
          if (NULL == param_value->bool_array_value) {
            allocator.deallocate(ret_val, allocator.state);
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem\n");
            ret = RCUTILS_RET_BAD_ALLOC;
            break;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'bool' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            ret = RCUTILS_RET_ERROR;
            break;
          }
        }
        ret = add_val_to_bool_arr(param_value->bool_array_value, ret_val, allocator);
        if (RCUTILS_RET_OK != ret) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
        }
      }
      break;
    case DATA_TYPE_INT64:
      if (!is_seq) {
        if (NULL != param_value->integer_value) {
          // Overwriting, deallocate original
          allocator.deallocate(param_value->integer_value, allocator.state);
        }
        param_value->integer_value = (int64_t *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          if (NULL != param_value->integer_array_value) {
            allocator.deallocate(param_value->integer_array_value->values, allocator.state);
            allocator.deallocate(param_value->integer_array_value, allocator.state);
            param_value->integer_array_value = NULL;
          }
          *seq_data_type = val_type;
          param_value->integer_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_int64_array_t), allocator.state);
          if (NULL == param_value->integer_array_value) {
            allocator.deallocate(ret_val, allocator.state);
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem\n");
            ret = RCUTILS_RET_BAD_ALLOC;
            break;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'integer' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            ret = RCUTILS_RET_ERROR;
            break;
          }
        }
        ret = add_val_to_int_arr(param_value->integer_array_value, ret_val, allocator);
        if (RCUTILS_RET_OK != ret) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
        }
      }
      break;
    case DATA_TYPE_DOUBLE:
      if (!is_seq) {
        if (NULL != param_value->double_value) {
          // Overwriting, deallocate original
          allocator.deallocate(param_value->double_value, allocator.state);
        }
        param_value->double_value = (double *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          if (NULL != param_value->double_array_value) {
            allocator.deallocate(param_value->double_array_value->values, allocator.state);
            allocator.deallocate(param_value->double_array_value, allocator.state);
            param_value->double_array_value = NULL;
          }
          *seq_data_type = val_type;
          param_value->double_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_double_array_t), allocator.state);
          if (NULL == param_value->double_array_value) {
            allocator.deallocate(ret_val, allocator.state);
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem\n");
            ret = RCUTILS_RET_BAD_ALLOC;
            break;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'double' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            ret = RCUTILS_RET_ERROR;
            break;
          }
        }
        ret = add_val_to_double_arr(param_value->double_array_value, ret_val, allocator);
        if (RCUTILS_RET_OK != ret) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
        }
      }
      break;
    case DATA_TYPE_STRING:
      if (!is_seq) {
        if (NULL != param_value->string_value) {
          // Overwriting, deallocate original
          allocator.deallocate(param_value->string_value, allocator.state);
        }
        param_value->string_value = (char *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          if (NULL != param_value->string_array_value) {
            if (RCUTILS_RET_OK != rcutils_string_array_fini(param_value->string_array_value)) {
              // Log and continue ...
              RCUTILS_SAFE_FWRITE_TO_STDERR("Error deallocating string array");
            }
            allocator.deallocate(param_value->string_array_value, allocator.state);
            param_value->string_array_value = NULL;
          }
          *seq_data_type = val_type;
          param_value->string_array_value =
            allocator.zero_allocate(1U, sizeof(rcutils_string_array_t), allocator.state);
          if (NULL == param_value->string_array_value) {
            allocator.deallocate(ret_val, allocator.state);
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem\n");
            ret = RCUTILS_RET_BAD_ALLOC;
            break;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'string' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            ret = RCUTILS_RET_ERROR;
            break;
          }
        }
        ret = add_val_to_string_arr(param_value->string_array_value, ret_val, allocator);
        if (RCUTILS_RET_OK != ret) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
        }
      }
      break;
    default:
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Unknown data type of value %s at line %d", value, line_num);
      ret = RCUTILS_RET_ERROR;
      allocator.deallocate(ret_val, allocator.state);
      break;
  }
  return ret;
}

///
/// Parse the key part of the <key:value> pair
///
rcutils_ret_t parse_key(
  const yaml_event_t event,
  uint32_t * map_level,
  bool * is_new_map,
  rcl_node_params_t ** node_params_st,
  char ** parameter_name,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(map_level, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(is_new_map, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_params_st, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(parameter_name, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(params_st, RCUTILS_RET_INVALID_ARGUMENT);
  rcutils_allocator_t allocator = params_st->allocator;
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  const size_t val_size = event.data.scalar.length;
  const char * value = (char *)event.data.scalar.value;
  const uint32_t line_num = ((uint32_t)(event.start_mark.line) + 1U);

  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    value, "event argument has no value", return RCUTILS_RET_INVALID_ARGUMENT);

  if (0U == val_size) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("No key at line %d", line_num);
    return RCUTILS_RET_ERROR;
  }

  rcutils_ret_t ret = RCUTILS_RET_OK;
  switch (*map_level) {
    case MAP_UNINIT_LVL:
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Unintialized map level at line %d", line_num);
      ret = RCUTILS_RET_ERROR;
      break;
    case MAP_NODE_NAME_LVL:
      {
        /// Till we get PARAMS_KEY, keep adding to node namespace
        if (0 != strncmp(PARAMS_KEY, value, strlen(PARAMS_KEY))) {
          ret = add_name_to_ns(ns_tracker, value, NS_TYPE_NODE, allocator);
          if (RCUTILS_RET_OK != ret) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Internal error adding node namespace at line %d", line_num);
            break;
          }
        } else {
          if (0U == ns_tracker->num_node_ns) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "There are no node names before %s at line %d", PARAMS_KEY, line_num);
            ret = RCUTILS_RET_ERROR;
            break;
          }
          /// The previous key(last name in namespace) was the node name. Remove it
          /// from the namespace
          char * node_name_ns = rcutils_strdup(ns_tracker->node_ns, allocator);
          if (NULL == node_name_ns) {
            ret = RCUTILS_RET_BAD_ALLOC;
            break;
          }

          ret = find_node(node_name_ns, params_st, node_params_st);
          allocator.deallocate(node_name_ns, allocator.state);
          if (RCUTILS_RET_OK != ret) {
            break;
          }

          ret = rem_name_from_ns(ns_tracker, NS_TYPE_NODE, allocator);
          if (RCUTILS_RET_OK != ret) {
            RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Internal error adding node namespace at line %d", line_num);
            break;
          }
          /// Bump the map level to PARAMS
          (*map_level)++;
        }
      }
      break;
    case MAP_PARAMS_LVL:
      {
        char * parameter_ns = NULL;

        /// If it is a new map, the previous key is param namespace
        if (*is_new_map) {
          parameter_ns = *parameter_name;
          if (NULL != parameter_ns) {
            ret = replace_ns(
              ns_tracker, parameter_ns, (ns_tracker->num_parameter_ns + 1U),
              NS_TYPE_PARAM, allocator);
            if (RCUTILS_RET_OK != ret) {
              RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Internal error replacing namespace at line %d", line_num);
              ret = RCUTILS_RET_ERROR;
              break;
            }
          }

          *is_new_map = false;
        }

        /// Add a parameter name into the node parameters
        parameter_ns = ns_tracker->parameter_ns;

        char * param_name = NULL;
        if (NULL == parameter_ns) {
          param_name = rcutils_strdup(value, allocator);
        } else {
          param_name = rcutils_format_string(allocator, "%s.%s", parameter_ns, value);
        }
        if (NULL == param_name) {
          ret = RCUTILS_RET_BAD_ALLOC;
          break;
        }

        if (*parameter_name) {
          allocator.deallocate(*parameter_name, allocator.state);
        }

        *parameter_name = param_name;
      }
      break;
    default:
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Unknown map level at line %d", line_num);
      ret = RCUTILS_RET_ERROR;
      break;
  }
  return ret;
}

///
/// Get events from parsing a parameter YAML file and process them
///
rcutils_ret_t parse_file_events(
  yaml_parser_t * parser,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st)
{
  int32_t done_parsing = 0;
  bool is_key = true;
  bool is_seq = false;
  uint32_t line_num = 0;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  uint32_t map_level = 1U;
  uint32_t map_depth = 0U;
  bool is_new_map = false;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(parser, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(params_st, RCUTILS_RET_INVALID_ARGUMENT);
  rcutils_allocator_t allocator = params_st->allocator;
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  yaml_event_t event;
  rcl_node_params_t * node_params_st = NULL;
  char * parameter_name = NULL;
  rcutils_ret_t ret = RCUTILS_RET_OK;
  while (0 == done_parsing) {
    if (RCUTILS_RET_OK != ret) {
      break;
    }
    int success = yaml_parser_parse(parser, &event);
    if (0 == success) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Error parsing a event near line %d", line_num);
      ret = RCUTILS_RET_ERROR;
      break;
    }
    line_num = ((uint32_t)(event.start_mark.line) + 1U);
    switch (event.type) {
      case YAML_STREAM_END_EVENT:
        done_parsing = 1;
        yaml_event_delete(&event);
        break;
      case YAML_SCALAR_EVENT:
        {
          /// Need to toggle between key and value at params level
          if (is_key) {
            ret = parse_key(
              event, &map_level, &is_new_map, &node_params_st, &parameter_name, ns_tracker,
              params_st);
            if (RCUTILS_RET_OK != ret) {
              break;
            }
            is_key = false;
          } else {
            /// It is a value
            if (map_level < (uint32_t)(MAP_PARAMS_LVL)) {
              RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Cannot have a value before %s at line %d", PARAMS_KEY, line_num);
              ret = RCUTILS_RET_ERROR;
              break;
            }

            ret = parse_value(
              event, is_seq, node_params_st, parameter_name, &seq_data_type, params_st);
            if (RCUTILS_RET_OK != ret) {
              break;
            }
            if (!is_seq) {
              is_key = true;
            }
          }
        }
        break;
      case YAML_SEQUENCE_START_EVENT:
        if (is_key) {
          RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Sequences cannot be key at line %d", line_num);
          ret = RCUTILS_RET_ERROR;
          break;
        }
        if (map_level < (uint32_t)(MAP_PARAMS_LVL)) {
          RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Sequences can only be values and not keys in params. Error at line %d\n", line_num);
          ret = RCUTILS_RET_ERROR;
          break;
        }
        is_seq = true;
        seq_data_type = DATA_TYPE_UNKNOWN;
        break;
      case YAML_SEQUENCE_END_EVENT:
        is_seq = false;
        is_key = true;
        break;
      case YAML_MAPPING_START_EVENT:
        map_depth++;
        is_new_map = true;
        is_key = true;
        /// Disable new map if it is PARAMS_KEY map
        if ((MAP_PARAMS_LVL == map_level) &&
          ((map_depth - (ns_tracker->num_node_ns + 1U)) == 2U))
        {
          is_new_map = false;
        }
        break;
      case YAML_MAPPING_END_EVENT:
        if (MAP_PARAMS_LVL == map_level) {
          if (ns_tracker->num_parameter_ns > 0U) {
            /// Remove param namesapce
            ret = rem_name_from_ns(ns_tracker, NS_TYPE_PARAM, allocator);
            if (RCUTILS_RET_OK != ret) {
              RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Internal error removing parameter namespace at line %d", line_num);
              break;
            }
          } else {
            map_level--;
          }
        } else {
          if ((MAP_NODE_NAME_LVL == map_level) &&
            (map_depth == (ns_tracker->num_node_ns + 1U)))
          {
            /// Remove node namespace
            ret = rem_name_from_ns(ns_tracker, NS_TYPE_NODE, allocator);
            if (RCUTILS_RET_OK != ret) {
              RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Internal error removing node namespace at line %d", line_num);
              break;
            }
          }
        }
        map_depth--;
        break;
      case YAML_ALIAS_EVENT:
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Will not support aliasing at line %d\n", line_num);
        ret = RCUTILS_RET_ERROR;
        break;
      case YAML_STREAM_START_EVENT:
        break;
      case YAML_DOCUMENT_START_EVENT:
        break;
      case YAML_DOCUMENT_END_EVENT:
        break;
      case YAML_NO_EVENT:
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Received an empty event at line %d", line_num);
        ret = RCUTILS_RET_ERROR;
        break;
      default:
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Unknown YAML event at line %d", line_num);
        ret = RCUTILS_RET_ERROR;
        break;
    }
    yaml_event_delete(&event);
  }

  if (parameter_name) {
    allocator.deallocate(parameter_name, allocator.state);
  }
  return ret;
}

///
/// Get events from parsing a parameter YAML value string and process them
///
rcutils_ret_t parse_value_events(
  yaml_parser_t * parser,
  rcl_node_params_t * node_params_st,
  const char * parameter_name,
  rcl_params_t * params_st)
{
  bool is_seq = false;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcutils_ret_t ret = RCUTILS_RET_OK;
  bool done_parsing = false;
  while (RCUTILS_RET_OK == ret && !done_parsing) {
    yaml_event_t event;
    int success = yaml_parser_parse(parser, &event);
    if (0 == success) {
      RCUTILS_SET_ERROR_MSG("Error parsing an event");
      ret = RCUTILS_RET_ERROR;
      break;
    }
    switch (event.type) {
      case YAML_STREAM_END_EVENT:
        done_parsing = true;
        break;
      case YAML_SCALAR_EVENT:
        ret = parse_value(
          event, is_seq, node_params_st, parameter_name, &seq_data_type, params_st);
        break;
      case YAML_SEQUENCE_START_EVENT:
        is_seq = true;
        seq_data_type = DATA_TYPE_UNKNOWN;
        break;
      case YAML_SEQUENCE_END_EVENT:
        is_seq = false;
        break;
      case YAML_STREAM_START_EVENT:
        break;
      case YAML_DOCUMENT_START_EVENT:
        break;
      case YAML_DOCUMENT_END_EVENT:
        break;
      case YAML_NO_EVENT:
        RCUTILS_SET_ERROR_MSG("Received an empty event");
        ret = RCUTILS_RET_ERROR;
        break;
      default:
        RCUTILS_SET_ERROR_MSG("Unknown YAML event");
        ret = RCUTILS_RET_ERROR;
        break;
    }
    yaml_event_delete(&event);
  }
  return ret;
}

///
/// Find parameter value entry in node parameters' structure
///
rcutils_ret_t find_parameter(
  rcl_node_params_t * node_params_st,
  const char * parameter_name,
  rcl_variant_t ** parameter_value,
  rcutils_allocator_t allocator
)
{
  assert(NULL != node_params_st);
  assert(NULL != parameter_name);
  assert(NULL != parameter_value);

  rcutils_ret_t ret = rcutils_hash_map_get(
    &node_params_st->node_params_map, &parameter_name, parameter_value);
  if (RCUTILS_RET_OK == ret) {
    // Node found.
    return RCUTILS_RET_OK;
  } else if (ret != RCUTILS_RET_NOT_FOUND) {
    return ret;
  }

  // Node not found, add it.
  char * param_name = rcutils_strdup(parameter_name, allocator);
  if (NULL == param_name) {
    RCUTILS_SET_ERROR_MSG("Failed to rcutils_strdup for parameter name");
    return RCUTILS_RET_BAD_ALLOC;
  }

  void * param_value = allocator.allocate(
    sizeof(rcl_variant_t), allocator.state);
  if (NULL == param_value) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for parameter value");
    ret = RCUTILS_RET_BAD_ALLOC;
    goto fail;
  }
  memset(param_value, 0, sizeof(rcl_variant_t));

  ret = rcutils_hash_map_set(&node_params_st->node_params_map, &param_name, &param_value);
  if (RCUTILS_RET_OK != ret) {
    goto fail;
  }

  *parameter_value = param_value;
  return RCUTILS_RET_OK;

fail:
  if (param_value) {
    allocator.deallocate(param_value, allocator.state);
  }
  if (param_name) {
    allocator.deallocate(param_name, allocator.state);
  }
  return ret;
}

///
/// Find node parameter entry in parameters' structure
///
rcutils_ret_t find_node(
  const char * node_name,
  rcl_params_t * param_st,
  rcl_node_params_t ** node_param_st)
{
  assert(NULL != node_name);
  assert(NULL != param_st);
  assert(NULL != node_param_st);

  rcutils_allocator_t allocator = param_st->allocator;

  rcutils_ret_t ret = rcutils_hash_map_get(&param_st->params_map, &node_name, node_param_st);
  if (RCUTILS_RET_OK == ret) {
    // Node found.
    return RCUTILS_RET_OK;
  } else if (ret != RCUTILS_RET_NOT_FOUND) {
    return ret;
  }

  // Node not found, add it.
  char * name = rcutils_strdup(node_name, allocator);
  if (NULL == name) {
    RCUTILS_SET_ERROR_MSG("Failed to rcutils_strdup for node parameter name");
    return RCUTILS_RET_BAD_ALLOC;
  }

  void * node_param = allocator.allocate(
    sizeof(rcl_node_params_t), allocator.state);
  if (NULL == node_param) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for node parameter value");
    ret = RCUTILS_RET_BAD_ALLOC;
    goto fail;
  }

  ret = node_params_init(node_param, param_st->allocator);
  if (RCUTILS_RET_OK != ret) {
    goto fail;
  }

  ret = rcutils_hash_map_set(&param_st->params_map, &name, &node_param);
  if (RCUTILS_RET_OK != ret) {
    goto after_init;
  }

  *node_param_st = node_param;
  return RCUTILS_RET_OK;

after_init:
  rcl_yaml_node_params_fini(node_param, allocator);

fail:
  if (node_param) {
    allocator.deallocate(node_param, allocator.state);
  }
  if (name) {
    allocator.deallocate(name, allocator.state);
  }
  return ret;
}
