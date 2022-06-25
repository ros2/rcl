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


#ifndef RCL_INTROSPECTION_H_
#define RCL_INTROSPECTION_H_


#include "./service_impl.h"

#include "rcl/error_handling.h"
#include "rmw/error_handling.h"

#include "rcutils/macros.h"
#include "rcutils/logging_macros.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#include "unique_identifier_msgs/msg/uuid.h"
#include "builtin_interfaces/msg/time.h"
#include "rcl_interfaces/msg/service_event.h"
#include "rcl_interfaces/msg/service_event_type.h"



rcl_ret_t 
send_introspection_message(
  const rcl_service_t * service,
  uint8_t event_type,
  void * ros_response_request,
  rmw_request_id_t * header,
  const rcl_service_options_t * options)
{
  // need a copy of this function for request/response (?) and also in client.c unless there's 
  // a utils.c or something...
  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Sending introspection message -----------------");
  
  rcl_ret_t ret;
  rcl_serialized_message_t serialized_message = rmw_get_zero_initialized_serialized_message();

  ret = rmw_serialized_message_init(&serialized_message, 0u, &options->allocator); // why is this 0u?
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message -----------------");
  rcl_interfaces__msg__ServiceEvent msg;
  rcl_interfaces__msg__ServiceEvent__init(&msg);
  msg.event_type = event_type;
  msg.sequence_number = header->sequence_number;
  rosidl_runtime_c__String__assign(&msg.service_name, rcl_service_get_service_name(service));
  printf("sequence_number: %ld\n", msg.sequence_number);
  printf("service_name: %s\n", msg.service_name.data);
  printf("event_type: %d\n", msg.event_type);
   
  rosidl_runtime_c__String__assign(&msg.event_name, "AddTwoInts"); // TODO(ihasdapie): get this properly, the type name is hardcoded throughout right now

  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: Building time message -----------------");
  builtin_interfaces__msg__Time timestamp;
  builtin_interfaces__msg__Time__init(&timestamp);

  rcl_time_point_value_t now;
  ret = rcl_clock_get_now(service->impl->introspection_utils->clock, &now);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  // is there a rcl method for getting the seconds too?
  timestamp.sec = RCL_NS_TO_S(now);
  timestamp.nanosec = now % (1000LL * 1000LL * 1000LL);
  msg.stamp = timestamp;

  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: Building UUID message -----------------");
  unique_identifier_msgs__msg__UUID uuid;
  unique_identifier_msgs__msg__UUID__init(&uuid);
  memcpy(uuid.uuid, header->writer_guid, 16 * sizeof(uint8_t));
  msg.client_id = uuid;

  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: Serializing introspected message -----------------");

  rosidl_message_type_support_t * serialized_message_ts;
  switch (event_type) {
    case rcl_interfaces__msg__ServiceEventType__REQUEST_RECEIVED:
      serialized_message_ts = service->impl->introspection_utils->request_type_support;
      break;
    case rcl_interfaces__msg__ServiceEventType__REQUEST_SENT:
      serialized_message_ts = service->impl->introspection_utils->response_type_support;
      break;
    case rcl_interfaces__msg__ServiceEventType__RESPONSE_RECEIVED:
      serialized_message_ts = service->impl->introspection_utils->response_type_support;
      break;
    case rcl_interfaces__msg__ServiceEventType__RESPONSE_SENT:
      serialized_message_ts = service->impl->introspection_utils->request_type_support;
      break;
    default:
      RCL_SET_ERROR_MSG("Invalid event type");
      return RCL_RET_ERROR;
  }

  ret = rmw_serialize(ros_response_request, serialized_message_ts, &serialized_message);
  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: memcpying introspected message -----------------");
  printf("serialized_message.buffer_length: %zu, serialized_message size: %zu\n", serialized_message.buffer_length, serialized_message.buffer_capacity);
  rosidl_runtime_c__octet__Sequence__init(&msg.serialized_event, serialized_message.buffer_length);

  // This segfaults, but thought this should be ok? since serialized_message.buffer is a uint8_t* which _should_ be the same as a byte array
  // rosidl_runtime_c__octet__Sequence__copy((rosidl_runtime_c__octet__Sequence *) serialized_message.buffer, &msg.serialized_event);

  memcpy(msg.serialized_event.data, serialized_message.buffer, serialized_message.buffer_length);


  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Publishing ServiceEvent Message -----------------");
  ret = rcl_publish(service->impl->introspection_utils->publisher, &msg, NULL);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Cleanup -----------------");

  unique_identifier_msgs__msg__UUID__fini(&uuid);
  builtin_interfaces__msg__Time__fini(&timestamp);
  ret = rmw_serialized_message_fini(&serialized_message);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  // have to fini the request/response message here as well
  rcl_interfaces__msg__ServiceEvent__fini(&msg);
  return RCL_RET_OK;
}


#endif // RCL_INTROSPECTION_H_
