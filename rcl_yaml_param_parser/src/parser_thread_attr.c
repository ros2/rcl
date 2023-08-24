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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "rcl_yaml_param_parser/parser_thread_attr.h"
#include "rcl_yaml_param_parser/types.h"

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/thread_attr.h"
#include "rcutils/types/rcutils_ret.h"

#include "./impl/types.h"
#include "./impl/parse.h"
#include "./impl/parse_thread_attr.h"
#include "./impl/node_params.h"
#include "./impl/yaml_variant.h"

///
/// Parse the YAML file and populate thread_attrs
///
rcutils_ret_t rcl_parse_yaml_thread_attrs_file(
  const char * file_path,
  rcutils_thread_attrs_t * thread_attrs)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    file_path, "YAML file path is NULL", return RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);

  if (0 < thread_attrs->num_attributes) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Pass an initialized thread attr structure");
    return RCUTILS_RET_ERROR;
  }

  yaml_parser_t parser;
  int success = yaml_parser_initialize(&parser);
  if (0 == success) {
    RCUTILS_SET_ERROR_MSG("Could not initialize the parser");
    return RCUTILS_RET_ERROR;
  }

  FILE * yaml_file = fopen(file_path, "r");
  if (NULL == yaml_file) {
    yaml_parser_delete(&parser);
    RCUTILS_SET_ERROR_MSG("Error opening YAML file");
    return RCUTILS_RET_ERROR;
  }

  yaml_parser_set_input_file(&parser, yaml_file);
  rcutils_ret_t ret = parse_thread_attr_events(&parser, thread_attrs);

  fclose(yaml_file);

  yaml_parser_delete(&parser);

  return ret;
}

///
/// Parse a YAML string and populate thread_attrs
///
rcutils_ret_t rcl_parse_yaml_thread_attrs_value(
  const char * yaml_value,
  rcutils_thread_attrs_t * thread_attrs)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(yaml_value, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);

  if (0 < thread_attrs->num_attributes) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Pass an initialized thread attr structure");
    return RCUTILS_RET_ERROR;
  }

  yaml_parser_t parser;
  int success = yaml_parser_initialize(&parser);
  if (0 == success) {
    RCUTILS_SET_ERROR_MSG("Could not initialize the parser");
    return RCUTILS_RET_ERROR;
  }

  yaml_parser_set_input_string(
    &parser, (const unsigned char *)yaml_value, strlen(yaml_value));

  rcutils_ret_t ret = parse_thread_attr_events(&parser, thread_attrs);

  yaml_parser_delete(&parser);

  return ret;
}
