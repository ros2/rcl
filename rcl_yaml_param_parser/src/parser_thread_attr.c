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
#include "rcutils/types/rcutils_ret.h"

#include "./impl/types.h"
#include "./impl/parse.h"
#include "./impl/parse_thread_attr.h"
#include "./impl/node_params.h"
#include "./impl/yaml_variant.h"

#define INIT_NUM_THREAD_ATTRIBUTE 0U

rcl_thread_attrs_t
rcl_get_zero_initialized_thread_attrs(void)
{
  rcl_thread_attrs_t ret = {
    NULL,
  };
  return ret;
}

rcutils_ret_t
rcl_thread_attrs_init(
  rcl_thread_attrs_t * thread_attrs,
  rcutils_allocator_t allocator)
{
  return rcl_thread_attrs_init_with_capacity(
    thread_attrs, allocator, INIT_NUM_THREAD_ATTRIBUTE);
}

rcutils_ret_t
rcl_thread_attrs_init_with_capacity(
  rcl_thread_attrs_t * thread_attrs,
  rcutils_allocator_t allocator,
  size_t capacity)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  thread_attrs->allocator = allocator;
  thread_attrs->num_attributes = 0U;
  thread_attrs->capacity_attributes = capacity;
  if (capacity > 0) {
    thread_attrs->attributes =
      allocator.zero_allocate(capacity, sizeof(rcl_thread_attr_t), allocator.state);
    if (NULL == thread_attrs->attributes) {
      *thread_attrs = rcl_get_zero_initialized_thread_attrs();
      RCUTILS_SET_ERROR_MSG("Failed to allocate memory for thread attributes");
      return RCUTILS_RET_BAD_ALLOC;
    }
  }
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcl_thread_attrs_fini(rcl_thread_attrs_t * thread_attrs)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  rcutils_allocator_t * allocator = &thread_attrs->allocator;
  if (NULL == thread_attrs->attributes) {
    return RCUTILS_RET_OK;
  }
  // check the allocator only if attributes is available to avoid checking after zero-initialized
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);
  for (size_t i = 0; i < thread_attrs->num_attributes; ++i) {
    rcl_thread_attr_t * attr = thread_attrs->attributes + i;
    if (NULL != attr->name) {
      allocator->deallocate(attr->name, allocator->state);
    }
  }
  allocator->deallocate(thread_attrs->attributes, allocator->state);
  *thread_attrs = rcl_get_zero_initialized_thread_attrs();

  return RCUTILS_RET_OK;
}

///
/// Parse the YAML file and populate thread_attrs
///
rcutils_ret_t rcl_parse_yaml_thread_attrs_file(
  const char * file_path,
  rcl_thread_attrs_t * thread_attrs)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    file_path, "YAML file path is NULL", return RCUTILS_RET_INVALID_ARGUMENT);

  if (NULL == thread_attrs) {
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
  rcl_thread_attrs_t * thread_attrs)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(yaml_value, false);

  if (0U == strlen(yaml_value)) {
    return RCUTILS_RET_ERROR;
  }

  if (NULL == thread_attrs) {
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
