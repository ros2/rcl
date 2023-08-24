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

#ifndef RCL_YAML_PARAM_PARSER__PARSER_THREAD_ATTR_H_
#define RCL_YAML_PARAM_PARSER__PARSER_THREAD_ATTR_H_

#include <stddef.h>


#include "rcl_yaml_param_parser/visibility_control.h"

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/thread_attr.h"
#include "rcutils/types/rcutils_ret.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \brief Parse the YAML file and populate \p thread_attrs
/// \pre Given \p thread_attrs must be a valid thread attribute struct
/// \param[in] file_path is the path to the YAML file
/// \param[in,out] thread_attrs points to the struct to be populated
/// \return true on success and false on failure
RCL_YAML_PARAM_PARSER_PUBLIC
rcutils_ret_t rcl_parse_yaml_thread_attrs_file(
  const char * file_path,
  rcutils_thread_attrs_t * thread_attrs);

/// \brief Parse a thread attribute value as a YAML string, updating thread_attrs accordingly
/// \param[in] yaml_value is the thread attribute value as a YAML string to be parsed
/// \param[in,out] thread_attrs points to the thread attribute struct
/// \return true on success and false on failure
RCL_YAML_PARAM_PARSER_PUBLIC
rcutils_ret_t rcl_parse_yaml_thread_attrs_value(
  const char * yaml_value,
  rcutils_thread_attrs_t * thread_attrs);

#ifdef __cplusplus
}
#endif

#endif  // RCL_YAML_PARAM_PARSER__PARSER_THREAD_ATTR_H_
