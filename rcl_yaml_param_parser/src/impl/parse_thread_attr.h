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

#ifndef IMPL__PARSE_THREAD_ATTR_H_
#define IMPL__PARSE_THREAD_ATTR_H_

#include <yaml.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"

#include "./types.h"
#include "rcl_yaml_param_parser/types.h"
#include "rcl_yaml_param_parser/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

RCL_YAML_PARAM_PARSER_LOCAL
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_thread_attr_key(
  const char * value,
  thread_attr_key_type_t * key_type);

RCL_YAML_PARAM_PARSER_LOCAL
RCUTILS_WARN_UNUSED
rcl_thread_scheduling_policy_type_t parse_thread_attr_scheduling_policy(
  const char * value);

RCL_YAML_PARAM_PARSER_LOCAL
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_thread_attr(
  yaml_parser_t * parser,
  rcl_thread_attr_t * attr,
  rcutils_allocator_t allocator);

RCL_YAML_PARAM_PARSER_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t parse_thread_attr_events(
  yaml_parser_t * parser,
  rcl_thread_attrs_t * thread_attrs);

#ifdef __cplusplus
}
#endif

#endif  // IMPL__PARSE_THREAD_ATTR_H_
