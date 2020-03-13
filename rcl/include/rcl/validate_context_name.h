// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__VALIDATE_CONTEXT_NAME_H_
#define RCL__VALIDATE_CONTEXT_NAME_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#define RCL_CONTEXT_NAME_VALID RMW_NAMESPACE_VALID
#define RCL_CONTEXT_NAME_INVALID_IS_EMPTY_STRING RMW_NAMESPACE_INVALID_IS_EMPTY_STRING
#define RCL_CONTEXT_NAME_INVALID_NOT_ABSOLUTE RMW_NAMESPACE_INVALID_NOT_ABSOLUTE
#define RCL_CONTEXT_NAME_INVALID_ENDS_WITH_FORWARD_SLASH \
  RMW_NAMESPACE_INVALID_ENDS_WITH_FORWARD_SLASH
#define RCL_CONTEXT_NAME_INVALID_CONTAINS_UNALLOWED_CHARACTERS \
  RMW_NAMESPACE_INVALID_CONTAINS_UNALLOWED_CHARACTERS
#define RCL_CONTEXT_NAME_INVALID_CONTAINS_REPEATED_FORWARD_SLASH \
  RMW_NAMESPACE_INVALID_CONTAINS_REPEATED_FORWARD_SLASH
#define RCL_CONTEXT_NAME_INVALID_NAME_TOKEN_STARTS_WITH_NUMBER \
  RMW_NAMESPACE_INVALID_NAME_TOKEN_STARTS_WITH_NUMBER
#define RCL_CONTEXT_NAME_INVALID_TOO_LONG RMW_NAMESPACE_INVALID_TOO_LONG

#define RCL_CONTEXT_NAME_MAX_LENGTH RMW_NODE_NAME_MAX_NAME_LENGTH

/// Determine if a given context name is valid.
/**
 * /sa The same rules as rmw_validate_namespace are used.
 * The only difference is the maximum length, which can be 255 characters.
 *
 * \param[in] context_name context_name to be validated
 * \param[out] validation_result int in which the result of the check is stored
 * \param[out] invalid_index index of the input string where an error occurred
 * \returns `RMW_RET_OK` on successfully running the check, or
 * \returns `RMW_RET_INVALID_ARGUMENT` on invalid parameters, or
 * \returns `RMW_RET_ERROR` when an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_validate_context_name(
  const char * context_name,
  int * validation_result,
  size_t * invalid_index);

/// Deterimine if a given context name is valid.
/**
 * This is an overload with an extra parameter for the length of context_name.
 * \param[in] context_name The number of characters in context_name.
 *
 * \sa rcl_validate_context_name(const char *, int *, size_t *)
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_validate_context_name_with_size(
  const char * context_name,
  size_t context_name_length,
  int * validation_result,
  size_t * invalid_index);

/// Return a validation result description, or NULL if unknown or RCL_CONTEXT_NAME_VALID.
RCL_PUBLIC
RCL_WARN_UNUSED
const char *
rcl_context_name_validation_result_string(int validation_result);

#ifdef __cplusplus
}
#endif

#endif  // RCL__VALIDATE_CONTEXT_NAME_H_
