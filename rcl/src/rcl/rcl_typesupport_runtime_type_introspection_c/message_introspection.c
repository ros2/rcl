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

#ifdef __cplusplus
extern "C"
{
#endif

#include "evolving_serialization_lib/description.h"

#include "rcl/types.h"
#include "rcl/error_handling.h"
#include "rcl/rcl_typesupport_runtime_type_introspection_c/identifier.h"
#include "rcl/rcl_typesupport_runtime_type_introspection_c/message_introspection.h"

#include "rcutils/logging_macros.h"
#include "rosidl_runtime_c/message_type_support_struct.h"

// NOTE(methylDragon): My use of the evolving_serialization_lib::type_description_t struct is for
//                     convenience only. We should be passing the TypeDescription message

/// Create a rosidl_message_type_support_t from a TypeDescription message
rosidl_message_type_support_t *
rcl_get_runtime_type_message_typesupport_handle(type_description_t * desc)
{
  // TODO(methylDragon): Do I need to use an allocator...?
  struct rosidl_message_type_support_t * ts = malloc(sizeof(struct rosidl_message_type_support_t));
  if(!ts) {
    RCUTILS_LOG_ERROR_NAMED(rcl_typesupport_runtime_type_introspection_c__identifier,
                            "Could not allocate rosidl_message_type_support_t struct");
    return NULL;
  }

  // TODO(methylDragon): This is just temporary. We'd ideally pass in the TypeDescription msg
  //                     The type_description_t object doesn't support string upper bounds or
  //                     default values

  // NOTE(methylDragon): Also, rosidl has a set of field types that we don't follow...
  //                     Should we unify them? rosidl doesn't use the field types to define
  //                     array or sequence types though...
  // https://github.com/ros2/rosidl/blob/rolling/rosidl_typesupport_introspection_c/include/rosidl_typesupport_introspection_c/field_types.h
  ts->typesupport_identifier = rcl_typesupport_runtime_type_introspection_c__identifier;
  ts->data = desc;
  ts->func = get_message_typesupport_handle_function;

  return ts;

  // NOTE(methylDragon): Handy methods I'm pasting for myself for use in the future...
  // get_ref_description_as_type_description(desc, KEY);
  // desc->type_description->type_name/type_version_hash
}


rcl_ret_t
rcl_runtime_type_message_typesupport_handle_fini(rosidl_message_type_support_t * ts)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(ts, RCL_RET_INVALID_ARGUMENT);

  // NOTE(methylDragon): Ignores const...
  if (ts->typesupport_identifier != rcl_typesupport_runtime_type_introspection_c__identifier)
  {
    RCL_SET_ERROR_MSG("type support not from this implementation");
    return RCL_RET_INVALID_ARGUMENT;
  }

  type_description_fini((type_description_t *)ts->data);
  free(ts);

  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
