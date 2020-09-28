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

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#include "rcl_yaml_param_parser/parser.h"
#include "rcl_yaml_param_parser/types.h"

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"

#include "./impl/types.h"
#include "./impl/parse.h"
#include "./impl/node_params.h"
#include "./impl/yaml_variant.h"

#define INIT_NUM_NODE_ENTRIES 128U

///
/// Create the rcl_params_t parameter structure
///
rcl_params_t * rcl_yaml_node_struct_init(
  const rcutils_allocator_t allocator)
{
  return rcl_yaml_node_struct_init_with_capacity(INIT_NUM_NODE_ENTRIES, allocator);
}

rcl_params_t * rcl_yaml_node_struct_init_with_capacity(
  size_t capacity,
  const rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return NULL);
  if (capacity == 0) {
    RCUTILS_SET_ERROR_MSG("capacity can't be zero");
    return NULL;
  }
  rcl_params_t * params_st = allocator.zero_allocate(1, sizeof(rcl_params_t), allocator.state);
  if (NULL == params_st) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for parameters");
    return NULL;
  }

  params_st->allocator = allocator;

  rcutils_hash_map_t map = rcutils_get_zero_initialized_hash_map();
  rcutils_ret_t ret = rcutils_hash_map_init(
    &map, capacity, sizeof(char *), sizeof(rcl_node_params_t *),
    rcutils_hash_map_string_hash_func, rcutils_hash_map_string_cmp_func, &allocator);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SET_ERROR_MSG("Failed to initialize hash map for parameters");
    goto clean;
  }

  params_st->params_map = map;
  return params_st;

clean:
  allocator.deallocate(params_st, allocator.state);
  return NULL;
}

///
/// Copy the rcl_params_t parameter structure
///
rcl_params_t * rcl_yaml_node_struct_copy(
  const rcl_params_t * params_st)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(params_st, NULL);

  size_t capacity;
  rcutils_ret_t ret;
  ret = rcutils_hash_map_get_capacity(&params_st->params_map, &capacity);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Error getting capacity from a hash map\n");
    return NULL;
  }

  rcutils_allocator_t allocator = params_st->allocator;
  rcl_params_t * out_params_st = rcl_yaml_node_struct_init_with_capacity(
    capacity,
    allocator);

  if (NULL == out_params_st) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating memory for parameter\n");
    return NULL;
  }

  char * node_name = NULL;
  rcl_node_params_t * node_param = NULL;
  char * out_node_name = NULL;
  rcl_node_params_t * out_node_param = NULL;
  ret = rcutils_hash_map_get_next_key_and_data(
    &params_st->params_map, NULL, &node_name, &node_param);
  while (RCUTILS_RET_OK == ret) {
    if (node_name == NULL || node_param == NULL) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error getting invalid key and data\n");
      break;
    }

    out_node_name = rcutils_strdup(node_name, allocator);
    if (out_node_name == NULL) {
      goto fail;
    }

    ret = rcutils_hash_map_get_capacity(&node_param->node_params_map, &capacity);
    if (RCUTILS_RET_OK != ret) {
      goto fail;
    }

    out_node_param = allocator.allocate(
      sizeof(rcl_node_params_t), allocator.state);
    if (NULL == out_node_param) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating memory for parameter values");
      goto fail;
    }

    ret = node_params_init_with_capacity(
      out_node_param,
      capacity,
      allocator);
    if (RCUTILS_RET_OK != ret) {
      goto fail;
    }

    ret = node_params_copy(node_param, out_node_param, allocator);
    if (RCUTILS_RET_OK != ret) {
      goto after_init;
    }

    ret = rcutils_hash_map_set(&out_params_st->params_map, &out_node_name, &out_node_param);
    if (ret != RCUTILS_RET_OK) {
      goto after_init;
    }

    out_node_name = NULL;
    out_node_param = NULL;
    ret = rcutils_hash_map_get_next_key_and_data(
      &params_st->params_map, &node_name, &node_name, &node_param);
  }

  if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES != ret) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to get next item for node parameters");
    goto fail;
  }

  return out_params_st;

after_init:
  rcl_yaml_node_params_fini(out_node_param, allocator);

fail:
  if (out_node_param) {
    allocator.deallocate(out_node_param, allocator.state);
  }
  if (out_node_name) {
    allocator.deallocate(out_node_name, allocator.state);
  }
  rcl_yaml_node_struct_fini(out_params_st);
  return NULL;
}

///
/// Free param structure
/// NOTE: If there is an error, would recommend just to safely exit the process instead
/// of calling this free function and continuing
///
void rcl_yaml_node_struct_fini(
  rcl_params_t * params_st)
{
  if (NULL == params_st) {
    return;
  }
  rcutils_allocator_t allocator = params_st->allocator;

  char * node_name = NULL;
  rcl_node_params_t * node_parameter = NULL;
  rcutils_ret_t ret = rcutils_hash_map_get_next_key_and_data(
    &params_st->params_map, NULL, &node_name, &node_parameter);
  while (RCUTILS_RET_OK == ret) {
    rcl_yaml_node_params_fini(node_parameter, allocator);
    ret = rcutils_hash_map_unset(&params_st->params_map, &node_name);
    if (ret != RCUTILS_RET_OK) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to unset key '%s' hash map for node parameters", node_name);
      return;
    }

    allocator.deallocate(node_name, allocator.state);      // we use duplicate key, must delete it
    allocator.deallocate(node_parameter, allocator.state);
    ret = rcutils_hash_map_get_next_key_and_data(
      &params_st->params_map, NULL, &node_name, &node_parameter);
  }
  if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES != ret) {
    RCUTILS_SET_ERROR_MSG("Failed to get next item for node parameters");
    return;
  }

  ret = rcutils_hash_map_fini(&params_st->params_map);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SET_ERROR_MSG("Failed to deallocate hash map");
    return;
  }

  allocator.deallocate(params_st, allocator.state);
}

///
/// TODO (anup.pemmaiah): Support Mutiple yaml files
///
///
/// Parse the YAML file and populate params_st
///
bool rcl_parse_yaml_file(
  const char * file_path,
  rcl_params_t * params_st)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    file_path, "YAML file path is NULL", return false);

  if (NULL == params_st) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Pass an initialized parameter structure");
    return false;
  }

  yaml_parser_t parser;
  int success = yaml_parser_initialize(&parser);
  if (0 == success) {
    RCUTILS_SET_ERROR_MSG("Could not initialize the parser");
    return false;
  }

  FILE * yaml_file = fopen(file_path, "r");
  if (NULL == yaml_file) {
    yaml_parser_delete(&parser);
    RCUTILS_SET_ERROR_MSG("Error opening YAML file");
    return false;
  }

  yaml_parser_set_input_file(&parser, yaml_file);

  namespace_tracker_t ns_tracker;
  memset(&ns_tracker, 0, sizeof(namespace_tracker_t));
  rcutils_ret_t ret = parse_file_events(&parser, &ns_tracker, params_st);

  fclose(yaml_file);

  yaml_parser_delete(&parser);

  rcutils_allocator_t allocator = params_st->allocator;
  if (NULL != ns_tracker.node_ns) {
    allocator.deallocate(ns_tracker.node_ns, allocator.state);
  }
  if (NULL != ns_tracker.parameter_ns) {
    allocator.deallocate(ns_tracker.parameter_ns, allocator.state);
  }

  return RCUTILS_RET_OK == ret;
}

///
/// Parse a YAML string and populate params_st
///
bool rcl_parse_yaml_value(
  const char * node_name,
  const char * param_name,
  const char * yaml_value,
  rcl_params_t * params_st)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_name, false);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(param_name, false);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(yaml_value, false);

  if (0U == strlen(node_name) || 0U == strlen(param_name) || 0U == strlen(yaml_value)) {
    return false;
  }

  if (NULL == params_st) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Pass an initialized parameter structure");
    return false;
  }

  rcl_node_params_t * node_params_st = NULL;
  rcutils_ret_t ret = find_node(node_name, params_st, &node_params_st);
  if (RCUTILS_RET_OK != ret) {
    return false;
  }

  yaml_parser_t parser;
  int success = yaml_parser_initialize(&parser);
  if (0 == success) {
    RCUTILS_SET_ERROR_MSG("Could not initialize the parser");
    return false;
  }

  yaml_parser_set_input_string(
    &parser, (const unsigned char *)yaml_value, strlen(yaml_value));

  ret = parse_value_events(&parser, node_params_st, param_name, params_st);

  yaml_parser_delete(&parser);

  return RCUTILS_RET_OK == ret;
}

rcl_variant_t * rcl_yaml_node_struct_get(
  const char * node_name,
  const char * param_name,
  rcl_params_t * params_st)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_name, NULL);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(param_name, NULL);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(params_st, NULL);

  rcl_node_params_t * node_param = NULL;
  rcutils_ret_t ret = find_node(node_name, params_st, &node_param);
  if (RCUTILS_RET_OK == ret) {
    rcl_variant_t * param_value = NULL;
    ret = find_parameter(node_param, param_name, &param_value, params_st->allocator);
    if (RCUTILS_RET_OK == ret) {
      return param_value;
    }
  }
  return NULL;
}

///
/// Dump the param structure
///
void rcl_yaml_node_struct_print(
  const rcl_params_t * const params_st)
{
  if (NULL == params_st) {
    return;
  }

  printf("\n Node Name\t\t\t\tParameters\n");
  int32_t param_col = 50;
  char * node_name = NULL;
  rcl_node_params_t * node_param = NULL;
  rcutils_ret_t ret = rcutils_hash_map_get_next_key_and_data(
    &params_st->params_map, NULL, &node_name, &node_param);
  while (RCUTILS_RET_OK == ret) {
    if (NULL != node_name) {
      printf("%s\n", node_name);
    }

    char * param_name = NULL;
    rcl_variant_t * param_value = NULL;
    ret = rcutils_hash_map_get_next_key_and_data(
      &node_param->node_params_map, NULL, &param_name, &param_value);
    while (RCUTILS_RET_OK == ret) {
      if (NULL != param_name) {
        printf("%*s", param_col, param_name);
      }

      if (NULL != param_value) {
        if (NULL != param_value->bool_value) {
          printf(": %s\n", *(param_value->bool_value) ? "true" : "false");
        } else if (NULL != param_value->integer_value) {
          printf(": %" PRId64 "\n", *(param_value->integer_value));
        } else if (NULL != param_value->double_value) {
          printf(": %lf\n", *(param_value->double_value));
        } else if (NULL != param_value->string_value) {
          printf(": %s\n", param_value->string_value);
        } else if (NULL != param_value->bool_array_value) {
          printf(": ");
          for (size_t i = 0; i < param_value->bool_array_value->size; i++) {
            if (param_value->bool_array_value->values) {
              printf(
                "%s, ",
                (param_value->bool_array_value->values[i]) ? "true" : "false");
            }
          }
          printf("\n");
        } else if (NULL != param_value->integer_array_value) {
          printf(": ");
          for (size_t i = 0; i < param_value->integer_array_value->size; i++) {
            if (param_value->integer_array_value->values) {
              printf("%" PRId64 ", ", param_value->integer_array_value->values[i]);
            }
          }
          printf("\n");
        } else if (NULL != param_value->double_array_value) {
          printf(": ");
          for (size_t i = 0; i < param_value->double_array_value->size; i++) {
            if (param_value->double_array_value->values) {
              printf("%lf, ", param_value->double_array_value->values[i]);
            }
          }
          printf("\n");
        } else if (NULL != param_value->string_array_value) {
          printf(": ");
          for (size_t i = 0; i < param_value->string_array_value->size; i++) {
            if (param_value->string_array_value->data[i]) {
              printf("%s, ", param_value->string_array_value->data[i]);
            }
          }
          printf("\n");
        }
      }

      ret = rcutils_hash_map_get_next_key_and_data(
        &node_param->node_params_map, &param_name, &param_name, &param_value);
    }

    ret = rcutils_hash_map_get_next_key_and_data(
      &params_st->params_map, &node_name, &node_name, &node_param);
  }
}
