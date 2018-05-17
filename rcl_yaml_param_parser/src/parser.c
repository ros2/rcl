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
#include <unistd.h>
#include <yaml.h>

#include "rcl_yaml_param_parser/parser.h"
#include "rcl_yaml_param_parser/types.h"

#include "rcl/error_handling.h"
#include "rcl/types.h"
#include "rcutils/strdup.h"

/// NOTE: Will allow a max YAML mapping depth of 5
/// map level 1 : Top level mapping of the config file
/// map level 2 : Node namespace mapping
/// map level 3 : Node name mapping
/// map level 4 : Params mapping
/// map level 5 : Params namespace mapping
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

typedef enum ns_type_e
{
  NS_TYPE_NODE = 1U,
  NS_TYPE_PARAM = 2U
} ns_type_t;

/// Keep track of node and parameter name spaces
typedef struct name_space_tracker_s
{
  char * node_ns;
  uint32_t num_node_ns;
  char * param_ns;
  uint32_t num_param_ns;
} name_space_tracker_t;

#define MAX_STRING_SZ 128U
#define PARAMS_KEY "ros__parameters"

#define MAX_NUM_NODE_ENTRIES 256U
#define MAX_NUM_PARAMS_PER_NODE 512U

static rcl_ret_t node_params_init(
  rcl_node_params_t * node_params,
  const rcl_allocator_t allocator);

static rcl_ret_t param_struct_init(
  rcl_params_t * params_st,
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
  name_space_tracker_t * ns_tracker,
  const char * name,
  const ns_type_t ns_type,
  rcl_allocator_t allocator);

static rcl_ret_t rem_name_from_ns(
  name_space_tracker_t * ns_tracker,
  const ns_type_t ns_type,
  rcl_allocator_t allocator);

static rcl_ret_t replace_ns(
  name_space_tracker_t * ns_tracker,
  char * const new_ns,
  const uint32_t new_ns_count,
  const ns_type_t ns_type,
  rcl_allocator_t allocator);

static void * get_value(
  const char * const value,
  data_types_t * val_type,
  const rcl_allocator_t allocator);

static rcl_ret_t parse_value(
  const yaml_event_t event,
  const bool is_seq,
  data_types_t * seq_data_type,
  rcl_params_t * params_st,
  const rcl_allocator_t allocator);

static rcl_ret_t parse_key(
  const yaml_event_t event,
  uint32_t * map_level,
  bool * is_new_map,
  name_space_tracker_t * ns_tracker,
  rcl_params_t * params_st,
  const rcl_allocator_t allocator);

static rcl_ret_t parse_events(
  yaml_parser_t * parser,
  name_space_tracker_t * ns_tracker,
  rcl_params_t * params_st,
  const rcl_allocator_t allocator);


///
/// Add name to namespace tracker
///
static rcl_ret_t add_name_to_ns(
  name_space_tracker_t * ns_tracker,
  const char * name,
  const ns_type_t ns_type,
  rcl_allocator_t allocator)
{
  char * cur_ns;
  uint32_t * cur_count;
  char sep_char;
  size_t name_len;
  size_t ns_len;
  size_t tot_len;
  rcl_ret_t res = RCL_RET_OK;

  switch (ns_type) {
    case NS_TYPE_NODE:
      cur_ns = ns_tracker->node_ns;
      cur_count = &(ns_tracker->num_node_ns);
      sep_char = '/';
      break;
    case NS_TYPE_PARAM:
      cur_ns = ns_tracker->param_ns;
      cur_count = &(ns_tracker->num_param_ns);
      sep_char = '.';
      break;
    default:
      res = RCL_RET_ERROR;
      break;
  }

  if (RCL_RET_OK == res) {
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
      tot_len = ns_len + name_len + 2U;

      cur_ns = allocator.reallocate(cur_ns, tot_len, NULL);
      if (NULL == cur_ns) {
        return RCL_RET_BAD_ALLOC;
      }
      cur_ns[ns_len] = sep_char;
      memmove((cur_ns + ns_len + 1U), name, name_len);
      cur_ns[tot_len - 1U] = '\0';
    }
    *cur_count = (*cur_count + 1U);

    if (NS_TYPE_NODE == ns_type) {
      ns_tracker->node_ns = cur_ns;
    } else {
      ns_tracker->param_ns = cur_ns;
    }
  }
  return res;
}

///
/// Remove name from namespace tracker
///
static rcl_ret_t rem_name_from_ns(
  name_space_tracker_t * ns_tracker,
  const ns_type_t ns_type,
  rcl_allocator_t allocator)
{
  char * cur_ns;
  uint32_t * cur_count;
  char sep_char;
  size_t name_len;
  size_t ns_len;
  size_t tot_len;
  rcl_ret_t res = RCL_RET_OK;

  switch (ns_type) {
    case NS_TYPE_NODE:
      cur_ns = ns_tracker->node_ns;
      cur_count = &(ns_tracker->num_node_ns);
      sep_char = '/';
      break;
    case NS_TYPE_PARAM:
      cur_ns = ns_tracker->param_ns;
      cur_count = &(ns_tracker->num_param_ns);
      sep_char = '.';
      break;
    default:
      res = RCL_RET_ERROR;
      break;
  }

  if (RCL_RET_OK == res) {
    /// Remove last name from ns
    if (*cur_count > 0U) {
      if (1U == *cur_count) {
        allocator.deallocate(cur_ns, NULL);
        cur_ns = NULL;
      } else {
        char * last_name = cur_ns;
        ns_len = strlen(cur_ns);
        name_len = 0U;

        last_name = (last_name + ns_len);
        while ((NULL != last_name) && (sep_char != *(last_name))) {
          last_name--;
          name_len++;
        }
        tot_len = (ns_len - name_len);
        cur_ns = allocator.reallocate(cur_ns, tot_len, NULL);
        if (NULL == cur_ns) {
          return RCL_RET_BAD_ALLOC;
        }
        cur_ns[tot_len] = '\0';
      }
      *cur_count = (*cur_count - 1U);
    }
    if (NS_TYPE_NODE == ns_type) {
      ns_tracker->node_ns = cur_ns;
    } else {
      ns_tracker->param_ns = cur_ns;
    }
  }
  return res;
}

///
/// Replace namespace in namespace tracker
///
static rcl_ret_t replace_ns(
  name_space_tracker_t * ns_tracker,
  char * const new_ns,
  const uint32_t new_ns_count,
  const ns_type_t ns_type,
  rcl_allocator_t allocator)
{
  rcl_ret_t res = RCL_RET_OK;

  /// Remove the old namespace and point to the new namespace
  switch (ns_type) {
    case NS_TYPE_NODE:
      if (NULL != ns_tracker->node_ns) {
        allocator.deallocate(ns_tracker->node_ns, NULL);
      }
      ns_tracker->node_ns = new_ns;
      ns_tracker->num_node_ns = new_ns_count;
      break;
    case NS_TYPE_PARAM:
      if (NULL != ns_tracker->param_ns) {
        allocator.deallocate(ns_tracker->param_ns, NULL);
      }
      ns_tracker->param_ns = new_ns;
      ns_tracker->num_param_ns = new_ns_count;
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
      sizeof(char *), NULL);
  if (NULL == node_params->parameter_names) {
    return RCL_RET_BAD_ALLOC;
  }

  node_params->parameter_values = allocator.zero_allocate(MAX_NUM_PARAMS_PER_NODE,
      sizeof(rcl_variant_t), NULL);
  if (NULL == node_params->parameter_values) {
    allocator.deallocate(node_params->parameter_names, NULL);
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

///
/// Create the rcl_params_t parameter structure
///
static rcl_ret_t param_struct_init(
  rcl_params_t * params_st,
  const rcl_allocator_t allocator)
{
  if (NULL == params_st) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  params_st->node_names = allocator.zero_allocate(MAX_NUM_NODE_ENTRIES,
      sizeof(char *), NULL);
  if (NULL == params_st->node_names) {
    rcl_yaml_node_struct_fini(params_st, allocator);
    RCL_SET_ERROR_MSG("Error allocating mem", allocator);
    return RCL_RET_BAD_ALLOC;
  }

  params_st->params = allocator.zero_allocate(MAX_NUM_NODE_ENTRIES, sizeof(rcl_node_params_t),
      NULL);
  if (NULL == params_st->params) {
    rcl_yaml_node_struct_fini(params_st, allocator);
    RCL_SET_ERROR_MSG("Error allocating mem", allocator);
    return RCL_RET_BAD_ALLOC;
  }

  params_st->num_nodes = 0U;

  return RCL_RET_OK;
}

///
/// Free param structure
/// NOTE: If there is an error, would recommend just to safely exit the process instead
/// of calling this free function and continuing
///
void rcl_yaml_node_struct_fini(
  rcl_params_t * params_st,
  const rcl_allocator_t allocator)
{
  uint32_t nd_idx;
  uint32_t pr_idx = 0U;

  if (NULL == params_st) {
    return;
  }

  for (nd_idx = 0; nd_idx < params_st->num_nodes; nd_idx++) {
    char * node_name = params_st->node_names[nd_idx];
    if (NULL != node_name) {
      allocator.deallocate(node_name, NULL);
    }

    if (NULL != params_st->params) {
      char * param_name;
      rcl_variant_t * param_var;
      for (pr_idx = 0; pr_idx < params_st->params[nd_idx].num_params; pr_idx++) {
        param_name = params_st->params[nd_idx].parameter_names[pr_idx];
        param_var = &(params_st->params[nd_idx].parameter_values[pr_idx]);
        if (NULL != param_name) {
          allocator.deallocate(param_name, NULL);
        }

        if (NULL != param_var) {
          if (NULL != param_var->bool_value) {
            allocator.deallocate(param_var->bool_value, NULL);
          } else if (NULL != param_var->integer_value) {
            allocator.deallocate(param_var->integer_value, NULL);
          } else if (NULL != param_var->double_value) {
            allocator.deallocate(param_var->double_value, NULL);
          } else if (NULL != param_var->string_value) {
            allocator.deallocate(param_var->string_value, NULL);
          } else if (NULL != param_var->bool_array_value) {
            if (NULL != param_var->bool_array_value->values) {
              allocator.deallocate(param_var->bool_array_value->values, NULL);
            }
            allocator.deallocate(param_var->bool_array_value, NULL);
          } else if (NULL != param_var->integer_array_value) {
            if (NULL != param_var->integer_array_value->values) {
              allocator.deallocate(param_var->integer_array_value->values, NULL);
            }
            allocator.deallocate(param_var->integer_array_value, NULL);
          } else if (NULL != param_var->double_array_value) {
            if (NULL != param_var->double_array_value->values) {
              allocator.deallocate(param_var->double_array_value->values, NULL);
            }
            allocator.deallocate(param_var->double_array_value, NULL);
          } else if (NULL != param_var->string_array_value) {
            for (uint32_t i = 0; i < param_var->string_array_value->size; i++) {
              if (NULL != param_var->string_array_value->data[i]) {
                allocator.deallocate(param_var->string_array_value->data[i], NULL);
              }
            }
            allocator.deallocate(param_var->string_array_value, NULL);
          } else {
            /// Nothing to do to keep pclint happy
          }
        }  // if (param_var)
      }  // for (pr_idx)
      if (NULL != params_st->params[nd_idx].parameter_values) {
        allocator.deallocate(params_st->params[nd_idx].parameter_values, NULL);
      }
      if (NULL != params_st->params[nd_idx].parameter_names) {
        allocator.deallocate(params_st->params[nd_idx].parameter_names, NULL);
      }
    }  // if (params)
  }  // for (node_idx)
  if (NULL != params_st->node_names) {
    allocator.deallocate(params_st->node_names, NULL);
    params_st->node_names = NULL;
  }
  if (NULL != params_st->params) {
    allocator.deallocate(params_st->params, NULL);
    params_st->params = NULL;
  }
  params_st->num_nodes = 0U;
}

///
/// Dump the param structure
///
void rcl_yaml_node_struct_print(
  const rcl_params_t * const params_st)
{
  uint32_t nd_idx;
  uint32_t pr_idx = 0U;

  if (NULL == params_st) {
    return;
  }

  printf("\n Node Name\t\t\t\tParameters\n");
  for (nd_idx = 0; nd_idx < params_st->num_nodes; nd_idx++) {
    int32_t param_col = 50;
    const char * const node_name = params_st->node_names[nd_idx];
    if (NULL != node_name) {
      printf("%s\n", node_name);
    }

    if (NULL != params_st->params) {
      char * param_name;
      rcl_variant_t * param_var;
      for (pr_idx = 0; pr_idx < params_st->params[nd_idx].num_params; pr_idx++) {
        param_name = params_st->params[nd_idx].parameter_names[pr_idx];
        param_var = &(params_st->params[nd_idx].parameter_values[pr_idx]);
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
            for (uint32_t i = 0; i < param_var->bool_array_value->size; i++) {
              if (param_var->bool_array_value->values) {
                printf("%s, ",
                  (param_var->bool_array_value->values[i]) ? "true" : "false");
              }
            }
            printf("\n");
          } else if (NULL != param_var->integer_array_value) {
            printf(": ");
            for (uint32_t i = 0; i < param_var->integer_array_value->size; i++) {
              if (param_var->integer_array_value->values) {
                printf("%" PRId64 ", ", param_var->integer_array_value->values[i]);
              }
            }
            printf("\n");
          } else if (NULL != param_var->double_array_value) {
            printf(": ");
            for (uint32_t i = 0; i < param_var->double_array_value->size; i++) {
              if (param_var->double_array_value->values) {
                printf("%lf, ", param_var->double_array_value->values[i]);
              }
            }
            printf("\n");
          } else if (NULL != param_var->string_array_value) {
            printf(": ");
            for (uint32_t i = 0; i < param_var->string_array_value->size; i++) {
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
    val_array->values = allocator.zero_allocate(val_array->size + 1U, sizeof(bool), NULL);
    if (NULL == val_array->values) {
      RCL_SET_ERROR_MSG("Error allocating memory", allocator);
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->values, tmp_arr, (val_array->size * sizeof(bool)));
    val_array->values[val_array->size] = *value;
    val_array->size++;
    allocator.deallocate(value, NULL);
    allocator.deallocate(tmp_arr, NULL);
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
        NULL);
    if (NULL == val_array->values) {
      RCL_SET_ERROR_MSG("Error allocating memory", allocator);
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->values, tmp_arr, (val_array->size * sizeof(int64_t)));
    val_array->values[val_array->size] = *value;
    val_array->size++;
    allocator.deallocate(value, NULL);
    allocator.deallocate(tmp_arr, NULL);
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
        NULL);
    if (NULL == val_array->values) {
      RCL_SET_ERROR_MSG("Error allocating memory", allocator);
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->values, tmp_arr, (val_array->size * sizeof(double)));
    val_array->values[val_array->size] = *value;
    val_array->size++;
    allocator.deallocate(value, NULL);
    allocator.deallocate(tmp_arr, NULL);
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
    char ** tmp_arr = val_array->data;
    val_array->data = allocator.zero_allocate(val_array->size + 1U, sizeof(char *), NULL);
    if (NULL == val_array->data) {
      RCL_SET_ERROR_MSG("Error allocating memory", allocator);
      return RCL_RET_BAD_ALLOC;
    }
    memmove(val_array->data, tmp_arr, (val_array->size * sizeof(char *)));
    val_array->data[val_array->size] = value;
    val_array->size++;
    allocator.deallocate(tmp_arr, NULL);
  }
  return RCL_RET_OK;
}

///
/// Determine the type of the value and return the converted value
/// NOTE: Only canonical forms supported as of now
///
static void * get_value(
  const char * const value,
  data_types_t * val_type,
  const rcl_allocator_t allocator)
{
  void * ret_val;
  int64_t ival;
  double dval;
  char * endptr = NULL;

  if ((NULL == value) || (NULL == val_type)) {
    RCL_SET_ERROR_MSG("Invalid arguments", allocator);
    return NULL;
  }

  /// Check if it is bool
  if ((0 == strncmp(value, "Y", strlen(value))) ||
    (0 == strncmp(value, "y", strlen(value))) ||
    (0 == strncmp(value, "yes", strlen(value))) ||
    (0 == strncmp(value, "Yes", strlen(value))) ||
    (0 == strncmp(value, "YES", strlen(value))) ||
    (0 == strncmp(value, "true", strlen(value))) ||
    (0 == strncmp(value, "True", strlen(value))) ||
    (0 == strncmp(value, "TRUE", strlen(value))) ||
    (0 == strncmp(value, "on", strlen(value))) ||
    (0 == strncmp(value, "On", strlen(value))) ||
    (0 == strncmp(value, "ON", strlen(value))))
  {
    *val_type = DATA_TYPE_BOOL;
    ret_val = allocator.zero_allocate(1U, sizeof(bool), NULL);
    if (NULL == ret_val) {
      return NULL;
    }
    *((bool *)ret_val) = true;
    return ret_val;
  }

  if ((0 == strncmp(value, "N", strlen(value))) ||
    (0 == strncmp(value, "n", strlen(value))) ||
    (0 == strncmp(value, "no", strlen(value))) ||
    (0 == strncmp(value, "No", strlen(value))) ||
    (0 == strncmp(value, "NO", strlen(value))) ||
    (0 == strncmp(value, "false", strlen(value))) ||
    (0 == strncmp(value, "False", strlen(value))) ||
    (0 == strncmp(value, "FALSE", strlen(value))) ||
    (0 == strncmp(value, "off", strlen(value))) ||
    (0 == strncmp(value, "Off", strlen(value))) ||
    (0 == strncmp(value, "OFF", strlen(value))))
  {
    *val_type = DATA_TYPE_BOOL;
    ret_val = allocator.zero_allocate(1U, sizeof(bool), NULL);
    if (NULL == ret_val) {
      return NULL;
    }
    *((bool *)ret_val) = false;
    return ret_val;
  }

  /// Check for int
  errno = 0;
  ival = strtol(value, &endptr, 0);
  if ((0 == errno) && (NULL != endptr)) {
    if ((NULL != endptr) && (endptr != value)) {
      if (('\0' != *value) && ('\0' == *endptr)) {
        *val_type = DATA_TYPE_INT64;
        ret_val = allocator.zero_allocate(1U, sizeof(int64_t), NULL);
        if (NULL == ret_val) {
          return NULL;
        }
        *((int64_t *)ret_val) = ival;
        return ret_val;
      }
    }
  }

  /// Check for float
  errno = 0;
  endptr = NULL;
  dval = strtod(value, &endptr);
  if ((0 == errno) && (NULL != endptr)) {
    if ((NULL != endptr) && (endptr != value)) {
      if (('\0' != *value) && ('\0' == *endptr)) {
        *val_type = DATA_TYPE_DOUBLE;
        ret_val = allocator.zero_allocate(1U, sizeof(double), NULL);
        if (NULL == ret_val) {
          return NULL;
        }
        *((double *)ret_val) = dval;
        return ret_val;
      }
    }
  }
  errno = 0;

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
  rcl_params_t * params_st,
  const rcl_allocator_t allocator)
{
  void * ret_val;
  data_types_t val_type;
  int res = RCL_RET_OK;

  if ((NULL == params_st) || (0U == params_st->num_nodes) || (NULL == seq_data_type)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  const uint32_t node_idx = (params_st->num_nodes - 1U);
  if (0U == params_st->params[node_idx].num_params) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  const uint32_t param_idx = ((params_st->params[node_idx].num_params) - 1U);
  const size_t val_size = event.data.scalar.length;
  const char * value = (char *)event.data.scalar.value;
  const uint32_t line_num = ((uint32_t)(event.start_mark.line) + 1U);
  rcl_variant_t * param_value;

  if (val_size > MAX_STRING_SZ) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Scalar value at line %d"
      " is bigger than %d bytes", line_num, MAX_STRING_SZ);
    return RCL_RET_ERROR;
  } else {
    if (0U == val_size) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "No value at line %d", line_num);
      return RCL_RET_ERROR;
    }
  }

  if (NULL == value) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (NULL == params_st->params[node_idx].parameter_values) {
    RCL_SET_ERROR_MSG("Internal error: Invalid mem", allocator);
    return RCL_RET_BAD_ALLOC;
  }

  param_value = &(params_st->params[node_idx].parameter_values[param_idx]);
  param_value->allocator = allocator;

  // param_value->string_value = rcutils_strdup(value, allocator);
  ret_val = get_value(value, &val_type, allocator);
  if (NULL == ret_val) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Error parsing value %s at"
      " line %d", value, line_num);
    return RCL_RET_ERROR;
  }

  switch (val_type) {
    case DATA_TYPE_UNKNOWN:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Unknown data type of"
        " value %s at line %d\n", value, line_num);
      res = RCL_RET_ERROR;
      break;
    case DATA_TYPE_BOOL:
      if (false == is_seq) {
        param_value->bool_value = (bool *)ret_val;
      } else {
        if (DATA_TYPE_UNKNOWN == *seq_data_type) {
          *seq_data_type = val_type;
          param_value->bool_array_value =
            allocator.zero_allocate(1U, sizeof(rcl_bool_array_t), NULL);
          if (NULL == param_value->bool_array_value) {
            RCL_SET_ERROR_MSG("Error allocating memory", allocator);
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Sequence should be of same"
              " type. Value type bool do not belong at line_num %d", line_num);
            allocator.deallocate(ret_val, NULL);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_bool_arr(param_value->bool_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, NULL);
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
            allocator.zero_allocate(1U, sizeof(rcl_int64_array_t), NULL);
          if (NULL == param_value->integer_array_value) {
            RCL_SET_ERROR_MSG("Error allocating memory", allocator);
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Sequence should be of same"
              " type. Value type integer do not belong at line_num %d", line_num);
            allocator.deallocate(ret_val, NULL);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_int_arr(param_value->integer_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, NULL);
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
            allocator.zero_allocate(1U, sizeof(rcl_double_array_t), NULL);
          if (NULL == param_value->double_array_value) {
            RCL_SET_ERROR_MSG("Error allocating memory", allocator);
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Sequence should be of same"
              " type. Value type double do not belong at line_num %d", line_num);
            allocator.deallocate(ret_val, NULL);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_double_arr(param_value->double_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, NULL);
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
            allocator.zero_allocate(1U, sizeof(rcutils_string_array_t), NULL);
          if (NULL == param_value->string_array_value) {
            RCL_SET_ERROR_MSG("Error allocating memory", allocator);
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          if (*seq_data_type != val_type) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Sequence should be of same"
              " type. Value type string do not belong at line_num %d", line_num);
            allocator.deallocate(ret_val, NULL);
            return RCL_RET_ERROR;
          }
        }
        res = add_val_to_string_arr(param_value->string_array_value, ret_val, allocator);
        if (RCL_RET_OK != res) {
          if (NULL != ret_val) {
            allocator.deallocate(ret_val, NULL);
          }
          return res;
        }
      }
      break;
    default:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Unknown data type of value"
        " %s at line %d", value, line_num);
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
  name_space_tracker_t * ns_tracker,
  rcl_params_t * params_st,
  const rcl_allocator_t allocator)
{
  int32_t res = RCL_RET_OK;
  const size_t val_size = event.data.scalar.length;
  const char * value = (char *)event.data.scalar.value;
  const uint32_t line_num = ((uint32_t)(event.start_mark.line) + 1U);
  uint32_t num_nodes;
  uint32_t node_idx = 0U;

  if ((NULL == map_level) || (NULL == params_st)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  if (val_size > MAX_STRING_SZ) {
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Scalar value at line %d"
      " is bigger than %d bytes", line_num, MAX_STRING_SZ);
    return RCL_RET_ERROR;
  } else {
    if (0U == val_size) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "No key at line %d", line_num);
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
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Unintialized map level"
        " at line %d", line_num);
      res = RCL_RET_ERROR;
      break;
    case MAP_NODE_NAME_LVL:
      {
        /// Till we get PARAMS_KEY, keep adding to node namespace
        if (0 != strncmp(PARAMS_KEY, value, strlen(PARAMS_KEY))) {
          res = add_name_to_ns(ns_tracker, value, NS_TYPE_NODE, allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Internal error"
              " adding node namespace at line %d", line_num);
            return res;
          }
        } else {
          if (0U == ns_tracker->num_node_ns) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "There are no node names"
              " before %s at line %d", PARAMS_KEY, line_num);
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
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Internal error"
              " adding node namespace at line %d", line_num);
            return res;
          }
          res = node_params_init(&(params_st->params[num_nodes]), allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Error creating node"
              " parameter at line %d", line_num);
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
        char * param_ns;
        uint32_t param_idx;
        char * param_name;

        /// If it is a new map, the previous key is param namespace
        if (true == *is_new_map) {
          params_st->params[node_idx].num_params--;
          param_idx = params_st->params[node_idx].num_params;
          param_ns = params_st->params[node_idx].parameter_names[param_idx];
          if (NULL == param_ns) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Internal error"
              " creating param namespace at line %d", line_num);
            return RCL_RET_ERROR;
          }
          res = replace_ns(ns_tracker, param_ns, (ns_tracker->num_param_ns + 1U),
              NS_TYPE_PARAM, allocator);
          if (RCL_RET_OK != res) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Internal error replacing"
              " namespace at line %d", line_num);
            return RCL_RET_ERROR;
          }
          *is_new_map = false;
        }
        /// Add a parameter name into the node parameters
        param_idx = params_st->params[node_idx].num_params;
        param_ns = ns_tracker->param_ns;
        if (NULL == param_ns) {
          param_name = rcutils_strdup(value, allocator);
          if (NULL == param_name) {
            return RCL_RET_BAD_ALLOC;
          }
        } else {
          const size_t params_ns_len = strlen(param_ns);
          const size_t param_name_len = strlen(value);
          const size_t tot_len = (params_ns_len + param_name_len + 2U);

          if (tot_len > MAX_STRING_SZ) {
            RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "The name length"
              " exceeds the MAX size %d at line %d", MAX_STRING_SZ, line_num);
            return RCL_RET_OK;
          }

          param_name = allocator.zero_allocate(1U, tot_len, NULL);
          if (NULL == param_name) {
            return RCL_RET_BAD_ALLOC;
          }

          memmove(param_name, param_ns, params_ns_len);
          param_name[params_ns_len] = '.';
          memmove((param_name + params_ns_len + 1U), value, param_name_len);
          param_name[tot_len - 1U] = '\0';
        }
        params_st->params[node_idx].parameter_names[param_idx] = param_name;
        params_st->params[node_idx].num_params++;
      }
      break;
    default:
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Unknown map level at"
        " line %d", line_num);
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
  name_space_tracker_t * ns_tracker,
  rcl_params_t * params_st,
  const rcl_allocator_t allocator)
{
  int32_t done_parsing = 0;
  yaml_event_t event;
  int32_t res = RCL_RET_OK;
  bool is_key = true;
  bool is_seq = false;
  uint32_t line_num = 0;
  data_types_t seq_data_type;
  uint32_t map_level = 1U;
  uint32_t map_depth = 0U;
  bool is_new_map = false;

  if ((NULL == parser) || (NULL == params_st)) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  while (0 == done_parsing) {
    if (RCL_RET_OK != res) {
      return res;
    }
    res = yaml_parser_parse(parser, &event);
    if (0 == res) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Error parsing a"
        " event near line %d", line_num);
      return RCL_RET_ERROR;
    } else {
      res = RCL_RET_OK;
    }
    line_num = ((uint32_t)(event.start_mark.line) + 1U);
    switch (event.type) {
      case YAML_STREAM_END_EVENT:
        done_parsing = 1;
        break;
      case YAML_SCALAR_EVENT:
        {
          /// Need to toggle between key and value at params level
          if (true == is_key) {
            res = parse_key(event, &map_level, &is_new_map, ns_tracker,
                params_st, allocator);
            if (RCL_RET_OK != res) {
              return res;
            }
            is_key = false;
          } else {
            /// It is a value
            if (map_level < (uint32_t)(MAP_PARAMS_LVL)) {
              RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Cannot have a value"
                " before %s at line %d", PARAMS_KEY, line_num);
              return RCL_RET_ERROR;
            }
            res = parse_value(event, is_seq, &seq_data_type, params_st, allocator);
            if (RCL_RET_OK != res) {
              return res;
            }
            if (false == is_seq) {
              is_key = true;
            }
          }
        }
        break;
      case YAML_SEQUENCE_START_EVENT:
        if (true == is_key) {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Sequences cannot be key"
            " at line %d", line_num);
          return RCL_RET_ERROR;
        }
        if (map_level < (uint32_t)(MAP_PARAMS_LVL)) {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Sequences can only be"
            " values and not keys in params. Error at line %d\n", line_num);
          return RCL_RET_ERROR;
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
          if (ns_tracker->num_param_ns > 0U) {
            /// Remove param namesapce
            res = rem_name_from_ns(ns_tracker, NS_TYPE_PARAM, allocator);
            if (RCL_RET_OK != res) {
              RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Internal error"
                " removing parameter namespace at line %d", line_num);
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
              RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Internal error"
                " removing node namespace at line %d", line_num);
              return res;
            }
          }
        }
        map_depth--;
        break;
      case YAML_ALIAS_EVENT:
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Will not support aliasing"
          " at line %d\n", line_num);
        res = RCL_RET_ERROR;
        break;
      case YAML_STREAM_START_EVENT:
        break;
      case YAML_DOCUMENT_START_EVENT:
        break;
      case YAML_DOCUMENT_END_EVENT:
        break;
      case YAML_NO_EVENT:
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Received an empty event at"
          " line %d", line_num);
        res = RCL_RET_ERROR;
        break;
      default:
        RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(allocator, "Unknown YAML event at line"
          " %d", line_num);
        res = RCL_RET_ERROR;
        break;
    }
  }
  return RCL_RET_OK;
}

///
/// TODO (anup.pemmaiah): Support string yaml similar to yaml file
/// TODO (anup.pemmaiah): Support Mutiple yaml files
///
///
/// Parse the YAML file and populate params_st
///
bool rcl_parse_yaml_file(const char * file_path, rcl_params_t * params_st)
{
  int32_t res;
  FILE * yaml_file;
  yaml_parser_t parser;
  name_space_tracker_t ns_tracker;
  rcl_allocator_t allocator = rcutils_get_default_allocator();

  if (NULL == file_path) {
    RCL_SET_ERROR_MSG("YAML file path is NULL", allocator);
    return false;
  }

  res = yaml_parser_initialize(&parser);
  if (0 == res) {
    RCL_SET_ERROR_MSG("Could not initialize the parser", allocator);
    return false;
  }

  yaml_file = fopen(file_path, "r");
  if (NULL == yaml_file) {
    RCL_SET_ERROR_MSG("Error opening YAML file", allocator);
    return false;
  }

  yaml_parser_set_input_file(&parser, yaml_file);

  res = param_struct_init(params_st, allocator);
  if (RCL_RET_OK != res) {
    RCL_SET_ERROR_MSG("Error creating node structure", allocator);
    return false;
  }

  memset(&ns_tracker, 0, sizeof(name_space_tracker_t));
  res = parse_events(&parser, &ns_tracker, params_st, allocator);
  if (RCL_RET_OK != res) {
    if (NULL != ns_tracker.node_ns) {
      allocator.deallocate(ns_tracker.node_ns, NULL);
    }
    if (NULL != ns_tracker.param_ns) {
      allocator.deallocate(ns_tracker.param_ns, NULL);
    }
    rcl_yaml_node_struct_fini(params_st, allocator);
    return false;
  }

  return true;
}
