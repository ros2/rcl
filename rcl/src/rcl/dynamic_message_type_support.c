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

#include <rcutils/logging_macros.h>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/type_description/type_description__struct.h>

#include "rmw/dynamic_message_type_support.h"

#include "rcl/allocator.h"
#include "rcl/common.h"
#include "rcl/error_handling.h"
#include "rcl/dynamic_message_type_support.h"
#include "rcl/type_hash.h"
#include "rcl/types.h"


/// Initialize a rosidl_message_type_support_t from a TypeDescription message
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_type_support_handle_init(
  const char * serialization_lib_name,
  const rosidl_runtime_c__type_description__TypeDescription * description,
  rcl_allocator_t * allocator,
  rosidl_message_type_support_t * ts)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ts, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RCUTILS_RET_INVALID_ARGUMENT);
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("allocator is invalid");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // TODO(methylDragon): Remove if and when the deferred description path is supported
  if (description == NULL) {
    RCUTILS_SET_ERROR_MSG(
      "Deferred type description is not currently supported. You must provide a type description.");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  bool middleware_supports_type_discovery = rmw_feature_supported(
    RMW_MIDDLEWARE_SUPPORTS_TYPE_DISCOVERY);
  if (!middleware_supports_type_discovery && description == NULL) {
    RCL_SET_ERROR_MSG(
      "Middleware does not support type discovery. Deferred dynamic type message type support will "
      "never be populated. You must provide a type description.");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // TODO(methylDragon): Remove if and when the deferred description path is supported
  if (description == NULL) {
    RCL_SET_ERROR_MSG(
      "Deferred type description is not currently supported. You must provide a type description.");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  rosidl_dynamic_typesupport_serialization_support_t serialization_support;
  rcl_ret_t ret = rcl_convert_rmw_ret_to_rcl_ret(
    rmw_serialization_support_init(serialization_lib_name, allocator, &serialization_support));
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("failed to get serialization support");
    if (ret == RCL_RET_OK) {  // It means serialization support was NULL
      return RCL_RET_ERROR;
    } else {
      return ret;
    }
  }

  rosidl_type_hash_t type_hash;
  ret = rcl_calculate_type_hash(
    // TODO(methylDragon): Replace this cast with the conversion function when it is ready
    //  Either a custom function, or from https://github.com/ros2/rcl/pull/1052
    (const type_description_interfaces__msg__TypeDescription *) description, &type_hash);
  if (ret != RCL_RET_OK) {
    RCL_SET_ERROR_MSG("failed to get type hash");
    return ret;
  }

  ret = rcl_convert_rcutils_ret_to_rcl_ret(
    rosidl_dynamic_message_type_support_handle_init(
      &serialization_support,
      &type_hash,   // type_hash
      description,  // type_description
      NULL,         // type_description_sources
      allocator,
      ts
    )
  );
  if (ret != RCL_RET_OK) {
    rcutils_error_string_t error_string = rcutils_get_error_string();
    rcutils_reset_error();
    RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "failed to init rosidl_message_type_support:\n%s", error_string.str);
    return ret;
  }

  return RCL_RET_OK;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_type_support_handle_fini(rosidl_message_type_support_t * ts)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(ts, RCL_RET_INVALID_ARGUMENT);
  return rcl_convert_rcutils_ret_to_rcl_ret(rosidl_dynamic_message_type_support_handle_fini(ts));
}

#ifdef __cplusplus
}
#endif
