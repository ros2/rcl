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

/// @file

#ifndef RCL__THREAD_ATTR_H_
#define RCL__THREAD_ATTR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"
#include "rcutils/thread_attr.h"

extern const char * const RCL_THREAD_ATTR_VALUE_ENV_VAR;
extern const char * const RCL_THREAD_ATTR_FILE_ENV_VAR;

/// Determine the default thread attribute from string, based on the environment.
/// \param[out] thread_attrs Must not be NULL.
/// \param[in] allocator memory allocator to be used
/// \return #RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
/// \return #RCL_RET_ERROR in case of an unexpected error, or
/// \return #RCL_RET_BAD_ALLOC if allocating memory failed, or
/// \return #RCL_RET_OK.
RCL_PUBLIC
rcl_ret_t
rcl_get_default_thread_attrs_from_value(
  rcutils_thread_attrs_t * thread_attrs,
  rcl_allocator_t allocator);

/// Determine the default thread attribute from file path, based on the environment.
/// \param[out] thread_attrs Must not be NULL.
/// \param[in] allocator memory allocator to be used
/// \return #RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
/// \return #RCL_RET_ERROR in case of an unexpected error, or
/// \return #RCL_RET_BAD_ALLOC if allocating memory failed, or
/// \return #RCL_RET_OK.
RCL_PUBLIC
rcl_ret_t
rcl_get_default_thread_attrs_from_file(
  rcutils_thread_attrs_t * thread_attrs,
  rcl_allocator_t allocator);

#ifdef __cplusplus
}
#endif

#endif   // RCL__THREAD_ATTR_H_
