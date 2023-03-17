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

#include "rmw/dynamic_typesupport.h"

#include "rcl/types.h"
#include "rcl/error_handling.h"
#include "rcl/rcl_dynamic_typesupport_c/identifier.h"
#include "rcl/rcl_dynamic_typesupport_c/message_introspection.h"

#include <rcutils/logging_macros.h>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/type_description/type_description__struct.h>  // TEMPORARY
// #include <type_description_interfaces/msg/type_description.h>  // Use this when conversion is ok


/// Create a rosidl_message_type_support_t from a TypeDescription message
RCL_PUBLIC
RCL_WARN_UNUSED
rosidl_message_type_support_t *
rcl_get_dynamic_message_typesupport_handle(
  const char * serialization_lib_name,
  // TODO(methylDragon): This should be const type_description_interfaces__msg__TypeDescription
  const rosidl_runtime_c__type_description__TypeDescription * description)
{
  return rmw_get_dynamic_message_typesupport_handle(
    rmw_get_serialization_support(serialization_lib_name),
    rmw_feature_supported(RMW_MIDDLEWARE_SUPPORTS_TYPE_DISCOVERY),
    rmw_feature_supported(RMW_MIDDLEWARE_CAN_TAKE_DYNAMIC_DATA),
    // TODO(methylDragon): We need convert type_description_interfaces__msg__TypeDescription to
    //                     rosidl_runtime_c__type_description__TypeDescription here
    description
  );
}


rcl_ret_t
rcl_dynamic_message_typesupport_handle_fini(rosidl_message_type_support_t * ts)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(ts, RCL_RET_INVALID_ARGUMENT);
  return rmw_dynamic_message_typesupport_handle_fini(ts);
}

#ifdef __cplusplus
}
#endif
