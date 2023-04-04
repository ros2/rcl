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
#include <rosidl_runtime_c/type_description/type_description__struct.h>  // TEMPORARY
// #include <type_description_interfaces/msg/type_description.h>  // Use this when conversion is ok

#include "rmw/dynamic_message_type_support.h"

#include "rcl/common.h"
#include "rcl/error_handling.h"
#include "rcl/dynamic_message_type_support.h"
#include "rcl/type_hash.h"
#include "rcl/types.h"


/// Create a rosidl_message_type_support_t from a TypeDescription message
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_type_support_handle_init(
  const char * serialization_lib_name,
  // TODO(methylDragon): This should be const type_description_interfaces__msg__TypeDescription
  const rosidl_runtime_c__type_description__TypeDescription * description,
  rosidl_message_type_support_t ** ts)
{
  rosidl_dynamic_typesupport_serialization_support_t * serialization_support = NULL;
  rcl_ret_t ret = rcl_convert_rmw_ret_to_rcl_ret(
    rmw_get_serialization_support(serialization_lib_name, &serialization_support));
  if (ret != RCL_RET_OK || serialization_support == NULL) {
    RCL_SET_ERROR_MSG("failed to get serialization support");
    if (ret == RCL_RET_OK) {
      return RCL_RET_ERROR;
    } else {
      return ret;
    }
  }

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_type_hash_t * type_hash = allocator.zero_allocate(
    1, sizeof(rosidl_type_hash_t), &allocator.state);
  if (!type_hash) {
    RCUTILS_SET_ERROR_MSG("Could not allocate type hash");
    return RCL_RET_ERROR;
  }

  ret = rcl_calculate_type_hash(
    // TODO(methylDragon): Replace this cast with the conversion function when it is ready
    (const type_description_interfaces__msg__TypeDescription *) description, type_hash);
  if (ret != RCL_RET_OK || type_hash == NULL) {
    RCL_SET_ERROR_MSG("failed to get type hash");
    allocator.deallocate(type_hash, &allocator.state);
    if (ret == RCL_RET_OK) {
      return RCL_RET_ERROR;
    } else {
      return ret;
    }
  }

  ret = rcl_convert_rmw_ret_to_rcl_ret(
    rmw_dynamic_message_type_support_handle_init(
      serialization_support,
      rmw_feature_supported(RMW_MIDDLEWARE_SUPPORTS_TYPE_DISCOVERY),
      // TODO(methylDragon): We need to convert type_description_interfaces__msg__TypeDescription to
      //                     rosidl_runtime_c__type_description__TypeDescription here
      type_hash,    // type_hash
      description,  // type_description
      NULL,         // type_description_sources
      ts
    )
  );

  if (!ts) {
    RCL_SET_ERROR_MSG("failed to init rosidl_message_type_support");
    allocator.deallocate(type_hash, &allocator.state);
    if (ret == RCL_RET_OK) {
      return RCL_RET_ERROR;
    } else {
      return ret;
    }
  }
  return RCL_RET_OK;
}


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_dynamic_message_type_support_handle_destroy(rosidl_message_type_support_t * ts)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(ts, RCL_RET_INVALID_ARGUMENT);
  return rcl_convert_rmw_ret_to_rcl_ret(rmw_dynamic_message_type_support_handle_destroy(ts));
}

#ifdef __cplusplus
}
#endif
