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

#ifndef RCL_YAML_PARAM_PARSER__PARSER_H_
#define RCL_YAML_PARAM_PARSER__PARSER_H_

#include <stdlib.h>

#include "rcl_yaml_param_parser/types.h"
#include "rcl_yaml_param_parser/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \brief Init param structure
/// \param[in] allocator memory allocator to be used
/// \return a pointer to param structure on success or NULL on failure
RCL_YAML_PARAM_PARSER_PUBLIC
rcl_params_t * rcl_yaml_node_struct_init(
  const rcutils_allocator_t allocator);

/// \brief Free param structure
/// \param[in] params_st points to the populated paramter struct
RCL_YAML_PARAM_PARSER_PUBLIC
void rcl_yaml_node_struct_fini(
  rcl_params_t * params_st);

/// \brief Parse the YAML file, initialize and populate params_st
/// \param[in] file_path is the path to the YAML file
/// \param[in/out] params_st points to the populated paramter struct
/// \return true on success and false on failure
RCL_YAML_PARAM_PARSER_PUBLIC
bool rcl_parse_yaml_file(
  const char * file_path,
  rcl_params_t * params_st);

/// \brief Print the parameter structure to stdout
/// \param[in] params_st points to the populated parameter struct
RCL_YAML_PARAM_PARSER_PUBLIC
void rcl_yaml_node_struct_print(
  const rcl_params_t * const params_st);

#ifdef __cplusplus
}
#endif

#endif  // RCL_YAML_PARAM_PARSER__PARSER_H_
