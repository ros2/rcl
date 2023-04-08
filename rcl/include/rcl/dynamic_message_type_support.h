// Copyright 2022 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__DYNAMIC_MESSAGE_TYPE_SUPPORT_H_
#define RCL__DYNAMIC_MESSAGE_TYPE_SUPPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <rosidl_runtime_c/type_description/type_description__struct.h>
#include <rosidl_runtime_c/message_type_support_struct.h>

#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

/// Does not take ownership of description (copies)
/// Allocates the `ts` arg. The caller takes ownership of the `ts` arg.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_type_support_handle_create(
  const char * serialization_lib_name,
  const rosidl_runtime_c__type_description__TypeDescription * desc,
  rosidl_message_type_support_t ** ts);  // OUT

/// Finalize a rosidl_message_type_support_t obtained with
/// `rcl_dynamic_message_type_support_handle_create()`
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_type_support_handle_destroy(rosidl_message_type_support_t * ts);

#ifdef __cplusplus
}
#endif

#endif  // RCL__DYNAMIC_MESSAGE_TYPE_SUPPORT_H_
