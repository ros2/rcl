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

#ifndef IMPL__PARSE_H_
#define IMPL__PARSE_H_

#include <yaml.h>

#include "rcutils/types.h"

#include "./types.h"
#include "rcl_yaml_param_parser/types.h"
#include "rcl_yaml_param_parser/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
void * get_value(
  const char * const value,
  yaml_scalar_style_t style,
  data_types_t * val_type,
  const rcutils_allocator_t allocator);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_value(
  const yaml_event_t event,
  const bool is_seq,
  rcl_node_params_t * node_params_st,
  const char * parameter_name,
  data_types_t * seq_data_type,
  rcl_params_t * params_st);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_key(
  const yaml_event_t event,
  uint32_t * map_level,
  bool * is_new_map,
  rcl_node_params_t ** node_params_st,
  char ** parameter_name,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_file_events(
  yaml_parser_t * parser,
  namespace_tracker_t * ns_tracker,
  rcl_params_t * params_st);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_value_events(
  yaml_parser_t * parser,
  rcl_node_params_t * node_params_st,
  const char * parameter_name,
  rcl_params_t * params_st);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t find_node(
  const char * node_name,
  rcl_params_t * param_st,
  rcl_node_params_t ** node_param_st);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t find_parameter(
  rcl_node_params_t * node_params_st,
  const char * parameter_name,
  rcl_variant_t ** parameter_value,
  rcutils_allocator_t allocator);

#ifdef __cplusplus
}
#endif

#endif  // IMPL__PARSE_H_
