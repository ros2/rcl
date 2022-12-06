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

#ifndef RCL_TYPESUPPORT_RUNTIME_TYPE_INTROSPECTION_C__MESSAGE_INTROSPECTION_H_
#define RCL_TYPESUPPORT_RUNTIME_TYPE_INTROSPECTION_C__MESSAGE_INTROSPECTION_H_

#include "evolving_serialization_lib/description.h"

#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#ifdef __cplusplus
extern "C" {
#endif


// TODO(methylDragon): Put this in a .c eventually...
// TODO(methylDragon): !!! Document that the user is in charge of the lifetime of the struct...
// TODO(methylDragon): ...
// NOTE(methylDragon): My use of the evolving_serialization_lib::type_description_t struct is for
//                     convenience only. We should be passing the TypeDescription message
RCL_PUBLIC
RCL_WARN_UNUSED
rosidl_message_type_support_t *
rcl_get_runtime_type_message_typesupport_handle(type_description_t * desc);


/// Finalize a rosidl_message_type_support_t obtained with
/// rcl_get_runtime_type_message_typesupport_handle
RCL_PUBLIC
rcl_ret_t
rcl_runtime_type_message_typesupport_handle_fini(rosidl_message_type_support_t * type_support);


#ifdef __cplusplus
}
#endif

#endif  // RCL_TYPESUPPORT_RUNTIME_TYPE_INTROSPECTION_C__MESSAGE_INTROSPECTION_H_
