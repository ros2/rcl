// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS_YAML_PARAM_PARSER__PARSER_H_
#define RCUTILS_YAML_PARAM_PARSER__PARSER_H_

#include <stdlib.h>
#include "rcutils/allocator.h"
#include "rcutils/types/string_array.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \typedef bool_array_t
/// \brief Array of bool values
typedef struct bool_array_s
{
  bool * values;
  size_t size;
} bool_array_t;

/// \typedef int64_array_t
/// \brief Array of int64_t values
typedef struct int64_array_s
{
  int64_t * values;
  size_t size;
} int64_array_t;

/// \typedef double_array_t
/// \brief Array of double values
typedef struct double_array_s
{
  double * values;
  size_t size;
} double_array_t;

/// \typedef variant_t
/// \brief variant_t stores the value of a parameter
/// Only one pointer in this struct will store the value
typedef struct variant_s
{
  bool * bool_value;  ///< If bool, gets stored here
  int64_t * integer_value;  ///< If integer, gets stored here
  double * double_value;  ///< If double, gets stored here
  char * string_value;  ///< If string, gets stored here
  // byte_array_t * byte_array_value;  ///< If array of bytes
  bool_array_t * bool_array_value;  ///< If array of bool's
  int64_array_t * integer_array_value;  ///< If array of integers
  double_array_t * double_array_value;  ///< If array of doubles
  rcutils_string_array_t * string_array_value;  ///< If array of strings
  rcutils_allocator_t allocator;  ///< Allocator used
} variant_t;

/// \typedef node_params_t
/// \brief node_params_t stores all the parameters(key:value) of a single node
typedef struct node_params_s
{
  char ** parameter_names;  ///< Array of parameter names (keys)
  variant_t * parameter_values;  ///< Array of coressponding parameter values
  uint32_t num_params;  ///< Number of parameters in the node
} node_params_t;

/// \typedef params_t
/// \brief params_t stores all the parameters of all nodes of a process
typedef struct params_s
{
  char ** node_namespaces;  ///< List of namespaces the corresponding node belongs to
  char ** node_names;  ///< List of names of the node
  node_params_t * params;  ///<  Array of parameters
  uint32_t num_nodes;       ///< Number of nodes
} params_t;

/// \brief Parse the YAML file and populate params_hdl
/// \param[in] file_path is the path to the YAML file
/// \param[out] params_st points to the populated paramter struct
/// \return true on success and false on failure
bool parse_yaml_file(const char * file_path, params_t * params_st);

/// \brief Free param structure
/// \param[in] params_st points to the populated paramter struct
/// \param[in] allocator memeory allocator to be used
void free_node_struct(params_t * params_st, const rcutils_allocator_t allocator);

/// \brief Print the parameter structure to stdout
/// \param[in] params_st points to the populated paramter struct
void print_node_struct(const params_t * const params_st);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS_YAML_PARAM_PARSER__PARSER_H_
