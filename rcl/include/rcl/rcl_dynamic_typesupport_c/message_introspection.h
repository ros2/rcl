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

#ifndef RCL_DYNAMIC_TYPESUPPORT_C__MESSAGE_INTROSPECTION_H_
#define RCL_DYNAMIC_TYPESUPPORT_C__MESSAGE_INTROSPECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#include <rosidl_runtime_c/type_description/type_description__struct.h>  // TEMPORARY
// #include <type_description_interfaces/msg/type_description.h>  // Use this when conversion is ok


// TODO(methylDragon): !!! Document that the user is in charge of the lifetime of the struct...
// TODO(methylDragon): ...

/// If the user passes a NULL desc, it is deferred instead, the middleware is responsibile for
/// populating the fields on type discovery!!!
///
/// Does not take ownership of description (copies)
/// Allocates the `ts` arg. The caller takes ownership of the `ts` arg.
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_typesupport_handle_init(
  const char * serialization_lib_name,
  // TODO(methylDragon): This should be const type_description_interfaces__msg__TypeDescription
  const rosidl_runtime_c__type_description__TypeDescription * desc,
  rosidl_message_type_support_t ** ts);

/// Finalize a rosidl_message_type_support_t obtained with
/// rcl_dynamic_message_typesupport_handle_init
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_typesupport_handle_fini(rosidl_message_type_support_t * ts);


#ifdef __cplusplus
}
#endif

#endif  // RCL_DYNAMIC_TYPESUPPORT_C__MESSAGE_INTROSPECTION_H_
