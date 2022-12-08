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

#ifndef IMPL__YAML_DESCRIPTOR_H_
#define IMPL__YAML_DESCRIPTOR_H_

#include "rcutils/allocator.h"

#include "./types.h"
#include "rcl_yaml_param_parser/types.h"
#include "rcl_yaml_param_parser/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

///
/// Finalize an rcl_param_descriptor_t
///
RCL_YAML_PARAM_PARSER_PUBLIC
void rcl_yaml_descriptor_fini(
  rcl_param_descriptor_t * param_desc,
  const rcutils_allocator_t allocator);

///
/// Copy a rcl_param_descriptor_t from param_desc to out_param_desc
///
RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
bool rcl_yaml_descriptor_copy(
  rcl_param_descriptor_t * out_param_desc,
  const rcl_param_descriptor_t * param_desc,
  rcutils_allocator_t allocator);

#ifdef __cplusplus
}
#endif

#endif  // IMPL__YAML_DESCRIPTOR_H_
