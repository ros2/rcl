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

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <yaml.h>

#include "rcl_yaml_param_parser/parser.h"
#include "rcl_yaml_param_parser/types.h"

#include "rcl/error_handling.h"
#include "rcl/types.h"
#include "rcutils/strdup.h"

/// NOTE: Will allow a max YAML mapping depth of 5
/// map level 1 : Node name mapping
/// map level 2 : Params mapping
typedef enum yaml_map_lvl_e
{
  MAP_UNINIT_LVL = 0U,
  MAP_NODE_NAME_LVL = 1U,
  MAP_PARAMS_LVL = 2U,
} yaml_map_lvl_t;

/// Basic supported data types in the yaml file
typedef enum data_types_e
{
  DATA_TYPE_UNKNOWN = 0U,
  DATA_TYPE_BOOL = 1U,
  DATA_TYPE_INT64 = 2U,
  DATA_TYPE_DOUBLE = 3U,
  DATA_TYPE_STRING = 4U
} data_types_t;

typedef enum namespace_type_e
{
  NS_TYPE_NODE = 1U,
  NS_TYPE_PARAM = 2U
} namespace_type_t;

/// Keep track of node and parameter name spaces
typedef struct namespace_tracker_s
{
  char * node_ns;
  uint32_t num_node_ns;
  char * parameter_ns;
  uint32_t num_parameter_ns;
} namespace_tracker_t;

#define MAX_STRING_SIZE 256U
#define PARAMS_KEY "ros__parameters"
#define NODE_NS_SEPERATOR "/"
#define PARAMETER_NS_SEPERATOR "."

#define MAX_NUM_NODE_ENTRIES 256U
#define MAX_NUM_PARAMS_PER_NODE 512U

static rcl_ret_t node_params_init(
  rcl_node_params_t * node_params,
  const rcl_allocator_t allocator);

static rcl_ret_t add_val_to_bool_arr(
  rcl_bool_array_t * const val_array,
  bool * value,
  const rcl_allocator_t allocator);

static rcl_ret_t add_val_to_int_arr(
  rcl_int64_array_t * const val_array,
  int64_t * value,
  const rcl_allocator_t allocator);

static rcl_ret_t add_val_to_double_arr(
  rcl_double_array_t * const val_array,
  double * value,
  const rcl_allocator_t allocator);

static rcl_ret_t add_val_to_string_arr(
  rcutils_string_array_t * const val_array,
  char * value,
  const rcl_allocator_t allocator);

///
/// TODO (anup.pemmaiah): Support byte array
///

static rcl_ret_t add_name_to_ns(
  namespace_tracker_t * ns_tracker,
  const char * name,
  const namespace_type_t namespace_type,
  rcl_allocator_t allocator);

static rcl_ret_t rem_name_from_ns(
  namespace_tracker_t * ns_tracker,
  const namespace_type_t namespace_type,
  rcl_allocator_t allocator);

static rcl_ret_t replace_ns(
  namespace_tracker_t * ns_tracker,
  char * const new_ns,
  const uint32_t new_ns_count,
  const namespace_type_t namespace_type,
  rcl_allocator_t allocator);

static void * get_value(
  const char * const value,
  yaml_scalar_style_t style,
  data_types_t * val_type,
  const rcl_allocator_t allocator);

static rcl_ret_t parse_value(
  const yaml_event_t event,
  const bool is_seq,
  data_types_t * seq_data_type,
  rcl_params_t * params_st);

static rcl_ret_t parse_key(
  const yaml_event_t event,
  uint32_t * map_level,
  bool * is_new_map,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st);

static rcl_ret_t parse_events(
  yaml_parser_t * parser,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st);

///
/// Add name to namespace tracker
///
static rcl_ret_t add_name_to_ns(
  namespace_tracker_t * ns_tracker,
  const char * name,
  const namespace_type_t namespace_type,
  rcl_allocator_t allocator)
{
  char * cur_ns;
  uint32_t * cur_count;
  char * sep_str;
  size_t name_len;
  size_t ns_len;
  size_t sep_len;
  size_t tot_len;

  switch (namespace_type) {
    case NS_TYPE_NODE:
      cur_ns = ns_tracker->node_ns;
      cur_count = &(ns_tracker->num_node_ns);
      sep_str = NODE_NS_SEPERATOR;
      break;
    case NS_TYPE_PARAM:
      cur_ns = ns_tracker->parameter_ns;
      cur_count = &(ns_tracker->num_parameter_ns);
      sep_str = PARAMETER_NS_SEPERATOR;
      break;
    default:
      return RCL_RET_ERROR;
  }

  /// Add a name to ns
  if (NULL == name) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  if (0U == *cur_count) {
    cur_ns = rcutils_strdup(name, allocator);
    if (NULL == cur_ns) {
      return RCL_RET_BAD_ALLOC;
    }
  } else {
    ns_len = strlen(cur_ns);
    name_len = strlen(name);
    sep_len = strlen(sep_str);
    // Check the last sep_len characters of the current NS against the separator string.
    if (strcmp(cur_ns + ns_len - sep_len, sep_str) == 0) {
      // Current NS already ends with the separator: don't put another separator in.
      sep_len = 0;
      sep_str = "";
    }

    tot_len = ns_len + sep_len + name_len + 1U;

    if (tot_len > MAX_STRING_SIZE) {
      RCL_SET_ERROR_MSG("New namespace string is exceeding max string size");
      return RCL_RET_ERROR;
    }
    cur_ns = allocator.reallocate(cur_ns, tot_len, allocator.state);
    if (NULL == cur_ns) {
      return RCL_RET_BAD_ALLOC;
    }
    memmove((cur_ns + ns_len), sep_str, sep_len);
    memmove((cur_ns + ns_len + sep_len), name, name_len);
    cur_ns[tot_len - 1U] = '\0';
  }
  *cur_count = (*cur_count + 1U);

  if (NS_TYPE_NODE == namespace_type) {
    ns_tracker->node_ns = cur_ns;
  } else {
    ns_tracker->parameter_ns = cur_ns;
  }
  return RCL_RET_OK;
}

///
/// Remove name from namespace tracker
///
static rcl_ret_t rem_name_from_ns(
  namespace_tracker_t * ns_tracker,
  const namespace_type_t namespace_type,
  rcl_allocator_t allocator)
{
  char * cur_ns;
  uint32_t * cur_count;
  char * sep_str;
  size_t ns_len;
  size_t tot_len;

  switch (namespace_type) {
    case NS_TYPE_NODE:
      cur_ns = ns_tracker->node_ns;
      cur_count = &(ns_tracker->num_node_ns);
      sep_str = NODE_NS_SEPERATOR;
      break;
    case NS_TYPE_PARAM:
      cur_ns = ns_tracker->parameter_ns;
      cur_count = &(ns_tracker->num_parameter_ns);
      sep_str = PARAMETER_NS_SEPERATOR;
      break;
    default:
      return RCL_RET_ERROR;
  }

  /// Remove last name from ns
  if (*cur_count > 0U) {
    if (1U == *cur_count) {
      allocator.deallocate(cur_ns, allocator.state);
      cur_ns = NULL;
    } else {
      ns_len = strlen(cur_ns);
      char * last_idx = NULL;
      char * next_str = NULL;
      const char * end_ptr = (cur_ns + ns_len);

      next_str = strstr(cur_ns, sep_str);
      while (NULL != next_str) {
        if (next_str > end_ptr) {
          RCL_SET_ERROR_MSG("Internal error. Crossing arrau boundary");
          return RCL_RET_ERROR;
        }
        last_idx = next_str;
        next_str = (next_str + strlen(sep_str));
        next_str = strstr(next_str, sep_str);
      }
      if (NULL != last_idx) {
        tot_len = ((size_t)(last_idx - cur_ns) + 1U);
        cur_ns = allocator.reallocate(cur_ns, tot_len, allocator.state);
        if (NULL == cur_ns) {
          return RCL_RET_BAD_ALLOC;
        }
        cur_ns[tot_len - 1U] = '\0';
      }
    }
    *cur_count = (*cur_count - 1U);
  }
  if (NS_TYPE_NODE == namespace_type) {
    ns_tracker->node_ns = cur_ns;
  } else {
    ns_tracker->parameter_ns = cur_ns;
  }
  return RCL_RET_OK;
}

///
/// Replace namespace in namespace tracker
///
static rcl_ret_t replace_ns(
  namespace_tracker_t * ns_tracker,
  char * const new_ns,
  const uint32_t new_ns_count,
  const namespace_type_t namespace_type,
  rcl_allocator_t allocator)
{
  rcl_ret_t res = RCL_RET_OK;

  /// Remove the old namespace and point to the new namespace
  switch (namespace_type) {
    case NS_TYPE_NODE:
      if (NULL != ns_tracker->node_ns) {
        allocator.deallocate(ns_tracker->node_ns, allocator.state);
      }
      ns_tracker->node_ns = new_ns;
      ns_tracker->num_node_ns = new_ns_count;
      break;
    case NS_TYPE_PARAM:
      if (NULL != ns_tracker->parameter_ns) {
        allocator.deallocate(ns_tracker->parameter_ns, allocator.state);
      }
      ns_tracker->parameter_ns = new_ns;
      ns_tracker->num_parameter_ns = new_ns_count;
      break;
    default:
      res = RCL_RET_ERROR;
      break;
  }
  return res;
}

///
/// Create rcl_node_params_t structure
///
static rcl_ret_t node_params_init(
  rcl_node_params_t * node_params,
  const rcl_allocator_t allocator)
{
  if (NULL == node_params) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  node_params->parameter_names = allocator.zero_allocate(MAX_NUM_PARAMS_PER_NODE,
      sizeof(char *), allocator.state);
  if (NULL == node_params->parameter_names) {
    return RCL_RET_BAD_ALLOC;
  }

  node_params->parameter_values = allocator.zero_allocate(MAX_NUM_PARAMS_PER_NODE,
      sizeof(rcl_variant_t), allocator.state);
  if (NULL == node_params->parameter_values) {
    allocator.deallocate(node_params->parameter_names, allocator.state);
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

///
/// Create the rcl_params_t parameter structure
///
rcl_params_t * rcl_yaml_node_struct_init(
  const rcl_allocator_t allocator)
{
  rcl_params_t * params_st;

  params_st = allocator.zero_allocate(1, sizeof(rcl_params_t), allocator.state);
  if (NULL == params_st) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
    return NULL;
  }

  params_st->node_names = allocator.zero_allocate(MAX_NUM_NODE_ENTRIES,
      sizeof(char *), allocator.state);
  if (NULL == params_st->node_names) {
    rcl_yaml_node_struct_fini(params_st);
    RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
    return NULL;
  }

  params_st->params = allocator.zero_allocate(MAX_NUM_NODE_ENTRIES, sizeof(rcl_node_params_t),
      allocator.state);
  if (NULL == params_st->params) {
    rcl_yaml_node_struct_fini(params_st);
    RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
    return NULL;
  }

  params_st->num_nodes = 0U;
  params_st->allocator = allocator;

  return params_st;
}

///
/// Free param structure
/// NOTE: If there is an error, would recommend just to safely exit the process instead
/// of calling this free function and continuing
///
void rcl_yaml_node_struct_fini(
  rcl_params_t * params_st)
{
  uint32_t node_idx;
  size_t parameter_idx = 0U;
  rcl_allocator_t allocator;

  if (NULL == params_st) {
    return;
  }
  allocator = params_st->allocator;

  for (node_idx = 0; node_idx < params_st->num_nodes; node_idx++) {
    char * node_name = params_st->node_names[node_idx];
    if (NULL != node_name) {
      allocator.deallocate(node_name, allocator.state);
    }

    if (NULL != params_st->params) {
      char * param_name;
      rcl_variant_t * param_var;
      for (parameter_idx = 0; parameter_idx < params_st->params[node_idx].num_params;
        parameter_idx++)
      {
        if ((NULL != params_st->params[node_idx].parameter_names) &&
          (NULL != params_st->params[node_idx].parameter_values))
        {
          param_name = params_st->params[node_idx].parameter_names[parameter_idx];
          param_var = &(params_st->params[node_idx].parameter_values[parameter_idx]);
          if (NULL != param_name) {
            allocator.deallocate(param_name, allocator.state);
          }

          if (NULL != param_var) {
            if (NULL != param_var->bool_value) {
              allocator.deallocate(param_var->bool_value, allocator.state);
            } else if (NULL != param_var->integer_value) {
              allocator.deallocate(param_var->integer_value, allocator.state);
            } else if (NULL != param_var->double_value) {
              allocator.deallocate(param_var->double_value, allocator.state);
            } else if (NULL != param_var->string_value) {
              allocator.deallocate(param_var->string_value, allocator.state);
            } else if (NULL != param_var->bool_array_value) {
              if (NULL != param_var->bool_array_value->values) {
                allocator.deallocate(param_var->bool_array_value->values, allocator.state);
              }
              allocator.deallocate(param_var->bool_array_value, allocator.state);
            } else if (NULL != param_var->integer_array_value) {
              if (NULL != param_var->integer_array_value->values) {
                allocator.deallocate(param_var->integer_array_value->values, allocator.state);
              }
              allocator.deallocate(param_var->integer_array_value, allocator.state);
            } else if (NULL != param_var->double_array_value) {
              if (NULL != param_var->double_array_value->values) {
                allocator.deallocate(param_var->double_array_value->values, allocator.state);
              }
              allocator.deallocate(param_var->double_array_value, allocator.state);
            } else if (NULL != param_var->string_array_value) {
              if (RCL_RET_OK != rcutils_string_array_fini(param_var->string_array_value)) {
                // Log and continue ...
                RCUTILS_SAFE_FWRITE_TO_STDERR("Error deallocating string array");
              }
              allocator.deallocate(param_var->string_array_value, allocator.state);
            } else {
              /// Nothing to do to keep pclint happy
            }
          }  // if (param_var)
        }
      }  // for (parameter_idx)
      if (NULL != params_st->params[node_idx].parameter_values) {
        allocator.deallocate(params_st->params[node_idx].parameter_values, allocator.state);
      }
      if (NULL != params_st->params[node_idx].parameter_names) {
        allocator.deallocate(params_st->params[node_idx].parameter_names, allocator.state);
      }
    }  // if (params)
  }  // for (node_idx)
  if (NULL != params_st->node_names) {
    allocator.deallocate(params_st->node_names, allocator.state);
    params_st->node_names = NULL;
  }
  if (NULL != params_st->params) {
    allocator.deallocate(params_st->params, allocator.state);
    params_st->params = NULL;
  }
  params_st->num_nodes = 0U;
  allocator.deallocate(params_st, allocator.state);
}

///
/// Dump the param structure
///
void rcl_yaml_node_struct_print(
  const rcl_params_t * const params_st)
{
  size_t node_idx;
  size_t parameter_idx = 0U;

  if (NULL == params_st) {
    return;
  }

  printf("\n Node Name\t\t\t\tParameters\n");
  for (node_idx = 0; node_idx < params_st->num_nodes; node_idx++) {
    int32_t param_col = 50;
    const char * const node_name = params_st->node_names[node_idx];
    if (NULL != node_name) {
      printf("%s\n", node_name);
    }

    if (NULL != params_st->params) {
      char * param_name;
      rcl_variant_t * param_var;
      for (parameter_idx = 0; parameter_idx < params_st->params[node_idx].num_params;
        parameter_idx++)
      {
        if ((NULL != params_st->params[node_idx].parameter_names) &&
          (NULL != params_st->params[node_idx].parameter_values))
        {
          param_name = params_st->params[node_idx].parameter_names[parameter_idx];
          param_var = &(params_st->params[node_idx].parameter_values[parameter_idx]);
          if (NULL != param_name) {
            printf("%*s", param_col, param_name);
          }

          if (NULL != param_var) {
            if (NULL != param_var->bool_value) {
              printf(": %s\n", *(param_var->bool_value) ? "true" : "false");
            } else if (NULL != param_var->integer_value) {
              printf(": %" PRId64 "\n", *(param_var->integer_value));
            } else if (NULL != param_var->double_value) {
              printf(": %lf\n", *(param_var->double_value));
            } else if (NULL != param_var->string_value) {
              printf(": %s\n", param_var->string_value);
            } else if (NULL != param_var->bool_array_value) {
              printf(": ");
              for (size_t i = 0; i < param_var->bool_array_value->size; i++) {
                if (param_var->bool_array_value->values) {
                  printf("%s, ",
                    (param_var->bool_array_value->values[i]) ? "true" : "false");
                }
              }
              printf("\n");
            } else if (NULL != param_var->integer_array_value) {
              printf(": ");
              for (size_t i = 0; i < param_var->integer_array_value->size; i++) {
                if (param_var->integer_array_value->values) {
                  printf("%" PRId64 ", ", param_var->integer_array_value->values[i]);
                }
              }
              printf("\n");
            } else if (NULL != param_var->double_array_value) {
              printf(": ");
              for (size_t i = 0; i < param_var->double_array_value->size; i++) {
                if (param_var->double_array_value->values) {
                  printf("%lf, ", param_var->double_array_value->values[i]);
                }
              }
              printf("\n");
            } else if (NULL != param_var->string_array_value) {
              printf(": ");
              for (size_t i = 0; i < param_var->string_array_value->size; i++) {
                if (param_var->string_array_value->data[i]) {
                  printf("%s, ", param_var->string_array_value->data[i]);
                }
              }
              printf("\n");
            }
          }
        }
      }
    }
  }
}

///
/// Add a value to a bool array. Create the array if it does not exist
///
static rcl_ret_t add_val_to_bool_arr(
  rcl_bool_array_t * const val_array,
  bool * value,
  const rcl_allocator_t allocator)
{
  if ((NULL == value) || (NULL == val_array)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (NULL == val_array->values) {
    val_array->values = value;
    val_array->size++;
  } else {
    /// Increase the array size by one and add the new value
    bool * tmp_arr = val_array->values;
    val_array->values = allocator.zero_allocate(val_array->size + 1U, sizeof(bool),
        allocator.state);
    if (NULL == val_array->values) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->values, tmp_arr, (val_array->size * sizeof(bool)));
    val_array->values[val_array->size] = *value;
    val_array->size++;
    allocator.deallocate(value, allocator.state);
    allocator.deallocate(tmp_arr, allocator.state);
  }
  return RCL_RET_OK;
}

///
/// Add a value to an integer array. Create the array if it does not exist
///
static rcl_ret_t add_val_to_int_arr(
  rcl_int64_array_t * const val_array,
  int64_t * value,
  const rcl_allocator_t allocator)
{
  if ((NULL == value) || (NULL == val_array)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (NULL == val_array->values) {
    val_array->values = value;
    val_array->size++;
  } else {
    /// Increase the array size by one and add the new value
    int64_t * tmp_arr = val_array->values;
    val_array->values = allocator.zero_allocate(val_array->size + 1U, sizeof(int64_t),
        allocator.state);
    if (NULL == val_array->values) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->values, tmp_arr, (val_array->size * sizeof(int64_t)));
    val_array->values[val_array->size] = *value;
    val_array->size++;
    allocator.deallocate(value, allocator.state);
    allocator.deallocate(tmp_arr, allocator.state);
  }
  return RCL_RET_OK;
}

///
/// Add a value to a double array. Create the array if it does not exist
///
static rcl_ret_t add_val_to_double_arr(
  rcl_double_array_t * const val_array,
  double * value,
  const rcl_allocator_t allocator)
{
  if ((NULL == value) || (NULL == val_array)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (NULL == val_array->values) {
    val_array->values = value;
    val_array->size++;
  } else {
    /// Increase the array size by one and add the new value
    double * tmp_arr = val_array->values;
    val_array->values = allocator.zero_allocate(val_array->size + 1U, sizeof(double),
        allocator.state);
    if (NULL == val_array->values) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->values, tmp_arr, (val_array->size * sizeof(double)));
    val_array->values[val_array->size] = *value;
    val_array->size++;
    allocator.deallocate(value, allocator.state);
    allocator.deallocate(tmp_arr, allocator.state);
  }
  return RCL_RET_OK;
}

///
/// Add a value to a string array. Create the array if it does not exist
///
static rcl_ret_t add_val_to_string_arr(
  rcutils_string_array_t * const val_array,
  char * value,
  const rcl_allocator_t allocator)
{
  if ((NULL == value) || (NULL == val_array)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (NULL == val_array->data) {
    rcl_ret_t res;

    res = rcutils_string_array_init(val_array, 1, &allocator);
    if (RCL_RET_OK != res) {
      return res;
    }
    val_array->data[0U] = value;
  } else {
    /// Increase the array size by one and add the new value
    val_array->data = allocator.reallocate(val_array->data,
        ((val_array->size + 1U) * sizeof(char *)), allocator.state);
    if (NULL == val_array->data) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return RCL_RET_BAD_ALLOC;
    }
    val_array->data[val_array->size] = value;
    val_array->size++;
  }
  return RCL_RET_OK;
}

///
/// Determine the type of the value and return the converted value
/// NOTE: Only canonical forms supported as of now
///
static void * get_value(
  const char * const value,
  yaml_scalar_style_t style,
  data_types_t * val_type,
  const rcl_allocator_t allocator)
{
  void * ret_val;
  int64_t ival;
  double dval;
  char * endptr = NULL;

  if ((NULL == value) || (NULL == val_type)) {
    RCL_SET_ERROR_MSG("Invalid arguments");
    return NULL;
  }

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
    dval = strtod(value, &endptr);
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
static rcl_ret_t parse_value(
  const yaml_event_t event,
  const bool is_seq,
  data_types_t * seq_data_type,
  rcl_params_t * params_st)
{
  void * ret_val;
  data_types_t val_type;
  int res = RCL_RET_OK;
  rcl_allocator_t allocator;

  if ((NULL == params_st) || (0U == params_st->num_nodes) || (NULL == seq_data_type)) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  allocator = params_st->allocator;

  const size_t node_idx = (params_st->num_nodes - 1U);
  if (0U == params_st->params[node_idx].num_params) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  const size_t parameter_idx = ((params_st->params[node_idx].num_params) - 1U);
  const size_t val_size = event.data.scalar.length;
  const char * value = (char *)event.data.scalar.value;
  yaml_scalar_style_t style = event.data.scalar.style;
  const uint32_t line_num = ((uint32_t)(event.start_mark.line) + 1U);
  rcl_variant_t * param_value;

  if (val_size > MAX_STRING_SIZE) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Scalar value at line %d is bigger than %d bytes", line_num, MAX_STRING_SIZE);
    return RCL_RET_ERROR;
  } else {
    if (style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
      style != YAML_DOUBLE_QUOTED_SCALAR_STYLE &&
      0U == val_size)
    {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("No value at line %d", line_num);
      return RCL_RET_ERROR;
    }
  }

  if (NULL == value) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (NULL == params_st->params[node_idx].parameter_values) {
    RCL_SET_ERROR_MSG("Internal error: Invalid mem");
    return RCL_RET_BAD_ALLOC;
  }

  param_value = &(params_st->params[node_idx].parameter_values[parameter_idx]);

  // param_value->string_value = rcutils_strdup(value, allocator);
  ret_val = get_value(value, style, &val_type, allocator);
  if (NULL == ret_val) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Error parsing value %s at line %d", value, line_num);
    return RCL_RET_ERROR;
  }

  switch (val_type) {
    case DATA_TYPE_UNKNOWN:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Unknown data type of value %s at line %d\n", value, line_num);
      res = RCL_RET_ERROR;
      break;
    case DATA_TYPE_BOOL:
      if (false == is_seq) {
        param_value->bool_value = (bool *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          *seq_data_type = val_type;
          param_value->bool_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_bool_array_t), allocator.state);
          if (NULL == param_value->bool_array_value) {
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'bool' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_bool_arr(param_value->bool_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
          return res;
        }
      }
      break;
    case DATA_TYPE_INT64:
      if (false == is_seq) {
        param_value->integer_value = (int64_t *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          *seq_data_type = val_type;
          param_value->integer_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_int64_array_t), allocator.state);
          if (NULL == param_value->integer_array_value) {
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'integer' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_int_arr(param_value->integer_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
          return res;
        }
      }
      break;
    case DATA_TYPE_DOUBLE:
      if (false == is_seq) {
        param_value->double_value = (double *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          *seq_data_type = val_type;
          param_value->double_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_double_array_t), allocator.state);
          if (NULL == param_value->double_array_value) {
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'double' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_double_arr(param_value->double_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
          return res;
        }
      }
      break;
    case DATA_TYPE_STRING:
      if (false == is_seq) {
        param_value->string_value = (char *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          *seq_data_type = val_type;
          param_value->string_array_value =
            allocator.zero_allocate(1U, sizeof(rcutils_string_array_t), allocator.state);
          if (NULL == param_value->string_array_value) {
            RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
            if (NULL != ret_val) {
              allocator.deallocate(ret_val, allocator.state);
            }
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Sequence should be of same type. Value type 'string' do not belong at line_num %d",
              line_num);
            allocator.deallocate(ret_val, allocator.state);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_string_arr(param_value->string_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, allocator.state);
          }
          return res;
        }
      }
      break;
    default:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Unknown data type of value %s at line %d", value, line_num);
      res = RCL_RET_ERROR;
      break;
  }
  return res;
}

///
/// Parse the key part of the <key:value> pair
///
static rcl_ret_t parse_key(
  const yaml_event_t event,
  uint32_t * map_level,
  bool * is_new_map,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st)
{
  int32_t res = RCL_RET_OK;
  const size_t val_size = event.data.scalar.length;
  const char * value = (char *)event.data.scalar.value;
  const uint32_t line_num = ((uint32_t)(event.start_mark.line) + 1U);
  size_t num_nodes;
  size_t node_idx = 0U;
  rcl_allocator_t allocator;

  if ((NULL == map_level) || (NULL == params_st)) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  allocator = params_st->allocator;

  if (val_size > MAX_STRING_SIZE) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Scalar value at line %d is bigger than %d bytes",
      line_num, MAX_STRING_SIZE);
    return RCL_RET_ERROR;
  } else {
    if (0U == val_size) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("No key at line %d", line_num);
      return RCL_RET_ERROR;
    }
  }

  if (NULL == value) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  num_nodes = params_st->num_nodes;  // New node index
  if (num_nodes > 0U) {
    node_idx = (num_nodes - 1U);  // Current node index
  }

  switch (*map_level) {
    case MAP_UNINIT_LVL:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Unintialized map level at line %d", line_num);
      res = RCL_RET_ERROR;
      break;
    case MAP_NODE_NAME_LVL:
      {
        /// Till we get PARAMS_KEY, keep adding to node namespace
        if (0 != strncmp(PARAMS_KEY, value, strlen(PARAMS_KEY))) {
          res = add_name_to_ns(ns_tracker, value, NS_TYPE_NODE, allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Internal error adding node namespace at line %d", line_num);
            return res;
          }
        } else {
          if (0U == ns_tracker->num_node_ns) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "There are no node names before %s at line %d", PARAMS_KEY, line_num);
            return RCL_RET_ERROR;
          }
          /// The previous key(last name in namespace) was the node name. Remove it
          /// from the namespace
          char * node_name_ns = rcutils_strdup(ns_tracker->node_ns, allocator);
          if (NULL == node_name_ns) {
            return RCL_RET_BAD_ALLOC;
          }
          params_st->node_names[num_nodes] = node_name_ns;

          res = rem_name_from_ns(ns_tracker, NS_TYPE_NODE, allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Internal error adding node namespace at line %d", line_num);
            return res;
          }
          res = node_params_init(&(params_st->params[num_nodes]), allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Error creating node parameter at line %d", line_num);
            return RCL_RET_ERROR;
          }
          params_st->num_nodes++;
          /// Bump the map level to PARAMS
          (*map_level)++;
        }
      }
      break;
    case MAP_PARAMS_LVL:
      {
        char * parameter_ns;
        size_t parameter_idx;
        char * param_name;

        /// If it is a new map, the previous key is param namespace
        if (true == *is_new_map) {
          params_st->params[node_idx].num_params--;
          parameter_idx = params_st->params[node_idx].num_params;
          parameter_ns = params_st->params[node_idx].parameter_names[parameter_idx];
          if (NULL == parameter_ns) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Internal error creating param namespace at line %d", line_num);
            return RCL_RET_ERROR;
          }
          res = replace_ns(ns_tracker, parameter_ns, (ns_tracker->num_parameter_ns + 1U),
              NS_TYPE_PARAM, allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "Internal error replacing namespace at line %d", line_num);
            return RCL_RET_ERROR;
          }
          *is_new_map = false;
        }

        // Guard against adding more than the maximum allowed parameters
        if (params_st->params[node_idx].num_params >= MAX_NUM_PARAMS_PER_NODE) {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Exceeded maximum allowed number of parameters for a node (%d)",
            MAX_NUM_PARAMS_PER_NODE);
          return RCL_RET_ERROR;
        }

        /// Add a parameter name into the node parameters
        parameter_idx = params_st->params[node_idx].num_params;
        parameter_ns = ns_tracker->parameter_ns;
        if (NULL == parameter_ns) {
          param_name = rcutils_strdup(value, allocator);
          if (NULL == param_name) {
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          const size_t params_ns_len = strlen(parameter_ns);
          const size_t param_name_len = strlen(value);
          const size_t tot_len = (params_ns_len + param_name_len + 2U);

          if (tot_len > MAX_STRING_SIZE) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
              "The name length exceeds the MAX size %d at line %d", MAX_STRING_SIZE, line_num);
            return RCL_RET_OK;
          }

          param_name = allocator.zero_allocate(1U, tot_len, allocator.state);
          if (NULL == param_name) {
            return RCL_RET_BAD_ALLOC;
          }

          memmove(param_name, parameter_ns, params_ns_len);
          param_name[params_ns_len] = '.';
          memmove((param_name + params_ns_len + 1U), value, param_name_len);
          param_name[tot_len - 1U] = '\0';
        }
        params_st->params[node_idx].parameter_names[parameter_idx] = param_name;
        params_st->params[node_idx].num_params++;
      }
      break;
    default:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Unknown map level at line %d", line_num);
      res = RCL_RET_ERROR;
      break;
  }
  return res;
}

///
/// Get events from the parser and process the events
///
static rcl_ret_t parse_events(
  yaml_parser_t * parser,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st)
{
  int32_t done_parsing = 0;
  yaml_event_t event;
  rcl_ret_t res = RCL_RET_OK;
  bool is_key = true;
  bool is_seq = false;
  uint32_t line_num = 0;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  uint32_t map_level = 1U;
  uint32_t map_depth = 0U;
  bool is_new_map = false;
  rcl_allocator_t allocator;

  if ((NULL == parser) || (NULL == params_st)) {
    return RCL_RET_INVALID_ARGUMENT;
  }
  allocator = params_st->allocator;

  while (0 == done_parsing) {
    if (RCL_RET_OK != res) {
      return res;
    }
    res = yaml_parser_parse(parser, &event);
    if (0 == res) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Error parsing a event near line %d", line_num);
      return RCL_RET_ERROR;
    } else {
      res = RCL_RET_OK;
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
          if (true == is_key) {
            res = parse_key(event, &map_level, &is_new_map, ns_tracker,
                params_st);
            if (RCL_RET_OK != res) {
              yaml_event_delete(&event);
              return res;
            }
            is_key = false;
          } else {
            /// It is a value
            if (map_level < (uint32_t)(MAP_PARAMS_LVL)) {
              RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Cannot have a value before %s at line %d", PARAMS_KEY, line_num);
              yaml_event_delete(&event);
              return RCL_RET_ERROR;
            }
            res = parse_value(event, is_seq, &seq_data_type, params_st);
            if (RCL_RET_OK != res) {
              yaml_event_delete(&event);
              return res;
            }
            if (false == is_seq) {
              is_key = true;
            }
          }
        }
        yaml_event_delete(&event);
        break;
      case YAML_SEQUENCE_START_EVENT:
        if (true == is_key) {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Sequences cannot be key at line %d", line_num);
          yaml_event_delete(&event);
          return RCL_RET_ERROR;
        }
        if (map_level < (uint32_t)(MAP_PARAMS_LVL)) {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Sequences can only be values and not keys in params. Error at line %d\n", line_num);
          yaml_event_delete(&event);
          return RCL_RET_ERROR;
        }
        is_seq = true;
        seq_data_type = DATA_TYPE_UNKNOWN;
        yaml_event_delete(&event);
        break;
      case YAML_SEQUENCE_END_EVENT:
        is_seq = false;
        is_key = true;
        yaml_event_delete(&event);
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
        yaml_event_delete(&event);
        break;
      case YAML_MAPPING_END_EVENT:
        if (MAP_PARAMS_LVL == map_level) {
          if (ns_tracker->num_parameter_ns > 0U) {
            /// Remove param namesapce
            res = rem_name_from_ns(ns_tracker, NS_TYPE_PARAM, allocator);
            if (RCL_RET_OK != res) {
              RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Internal error removing parameter namespace at line %d", line_num);
              yaml_event_delete(&event);
              return res;
            }
          } else {
            map_level--;
          }
        } else {
          if ((MAP_NODE_NAME_LVL == map_level) &&
            (map_depth == (ns_tracker->num_node_ns + 1U)))
          {
            /// Remove node namespace
            res = rem_name_from_ns(ns_tracker, NS_TYPE_NODE, allocator);
            if (RCL_RET_OK != res) {
              RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
                "Internal error removing node namespace at line %d", line_num);
              yaml_event_delete(&event);
              return res;
            }
          }
        }
        map_depth--;
        yaml_event_delete(&event);
        break;
      case YAML_ALIAS_EVENT:
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Will not support aliasing at line %d\n", line_num);
        res = RCL_RET_ERROR;
        yaml_event_delete(&event);
        break;
      case YAML_STREAM_START_EVENT:
        yaml_event_delete(&event);
        break;
      case YAML_DOCUMENT_START_EVENT:
        yaml_event_delete(&event);
        break;
      case YAML_DOCUMENT_END_EVENT:
        yaml_event_delete(&event);
        break;
      case YAML_NO_EVENT:
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Received an empty event at line %d", line_num);
        res = RCL_RET_ERROR;
        yaml_event_delete(&event);
        break;
      default:
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING("Unknown YAML event at line %d", line_num);
        res = RCL_RET_ERROR;
        yaml_event_delete(&event);
        break;
    }
  }
  return res;
}

///
/// TODO (anup.pemmaiah): Support string yaml similar to yaml file
/// TODO (anup.pemmaiah): Support Mutiple yaml files
///
///
/// Parse the YAML file and populate params_st
///
bool rcl_parse_yaml_file(
  const char * file_path,
  rcl_params_t * params_st)
{
  int32_t res;
  FILE * yaml_file;
  yaml_parser_t parser;
  namespace_tracker_t ns_tracker;
  rcutils_allocator_t allocator;

  if (NULL == params_st) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Pass a initialized paramter structure");
    return false;
  }
  allocator = params_st->allocator;

  if (NULL == file_path) {
    RCL_SET_ERROR_MSG("YAML file path is NULL");
    return false;
  }

  res = yaml_parser_initialize(&parser);
  if (0 == res) {
    RCL_SET_ERROR_MSG("Could not initialize the parser");
    return false;
  }

  yaml_file = fopen(file_path, "r");
  if (NULL == yaml_file) {
    yaml_parser_delete(&parser);
    RCL_SET_ERROR_MSG("Error opening YAML file");
    return false;
  }

  yaml_parser_set_input_file(&parser, yaml_file);

  memset(&ns_tracker, 0, sizeof(namespace_tracker_t));
  res = parse_events(&parser, &ns_tracker, params_st);

  yaml_parser_delete(&parser);
  fclose(yaml_file);

  if (RCL_RET_OK != res) {
    if (NULL != ns_tracker.node_ns) {
      allocator.deallocate(ns_tracker.node_ns, allocator.state);
    }
    if (NULL != ns_tracker.parameter_ns) {
      allocator.deallocate(ns_tracker.parameter_ns, allocator.state);
    }
    rcl_yaml_node_struct_fini(params_st);
    return false;
  }

  return true;
}
