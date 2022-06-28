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

#include "dlfcn.h"

#include "rcutils/macros.h"
#include "rcutils/logging_macros.h"
#include "rcutils/shared_library.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#include "unique_identifier_msgs/msg/uuid.h"
#include "builtin_interfaces/msg/time.h"
#include "rcl_interfaces/msg/service_event.h"
#include "rcl_interfaces/msg/service_event_type.h"

#include "rosidl_typesupport_introspection_c/service_introspection.h"

#include "rosidl_typesupport_introspection_c/identifier.h"



rcl_service_introspection_utils_t
rcl_get_zero_initialized_introspection_utils ()
{
  static rcl_service_introspection_utils_t null_introspection_utils = {
    .response_type_support = NULL,
    .request_type_support = NULL,
    .publisher = NULL,
    .clock = NULL,
    .service_name = NULL
  };
  return null_introspection_utils;
}

rcl_ret_t
rcl_service_introspection_init(
  rcl_service_introspection_utils_t * introspection_utils,
  const rosidl_service_type_support_t * service_type_support,
  const char * service_name,
  const rcl_node_t * node,
  rcl_allocator_t * allocator
  )
{
  
  // the reason why we can't use a macro and concatenate the Type to get what we want is 
  // because there we had always {goal status result} but here we can have any service type name...
  // TODO(ihasdapie): Need to get the package and message name somehow
  // probably hook into rosidl
  rcl_ret_t ret;

  introspection_utils->service_name = allocator->allocate(
      strlen(service_name) + 1, allocator->state);
  strcpy(introspection_utils->service_name, service_name);



  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Typesupport -----------------");

  printf("Service typesupport identifier: %s\n", service_type_support->typesupport_identifier);

  const rosidl_service_type_support_t * introspection_type_support = get_service_typesupport_handle(
      service_type_support, 
      // "rosidl_typesupport_introspection_cpp" // this will "work" and then segfault when trying to serialize
      rosidl_typesupport_introspection_c__identifier);
  

  if (!introspection_type_support) {
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to get introspection typesupport handle");
    return RCL_RET_ERROR;
  }

  // printf("Typesupport identifier: %s\n", introspection_type_support->typesupport_identifier);



  rosidl_typesupport_introspection_c__ServiceMembers * members;

  if (strcmp(introspection_type_support->typesupport_identifier, "rosidl_typesupport_introspection_c") == 0) {
  members = (rosidl_typesupport_introspection_c__ServiceMembers *) introspection_type_support->data;

  } else if (strcmp(introspection_type_support->typesupport_identifier, "rosidl_typesupport_introspection_cpp") == 0) {
    // TODO(ihasdapie): Grabbing cpp members  (?)
    /* ServiceMembers * members = 
      (ServiceMembers *) introspection_type_support->data; */
  members = (rosidl_typesupport_introspection_c__ServiceMembers *) introspection_type_support->data;
  } 
  else {
    printf("Unknown typesupport identifier: %s\n", introspection_type_support->typesupport_identifier);
    return RCL_RET_ERROR;
    // unknown ts
  }

  const rosidl_message_type_support_t * request_ts = members->request_members_->members_->members_;
  const rosidl_message_type_support_t * response_ts = members->response_members_->members_->members_;
  introspection_utils->request_type_support = request_ts;
  introspection_utils->response_type_support = response_ts;


  RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Typesupport DONE -----------------");






  char * service_event_name = (char*) allocator->zero_allocate(
      strlen(service_name) + strlen(RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX) + 1,
      sizeof(char), allocator->state);
  strcpy(service_event_name, service_name);
  strcat(service_event_name, RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX);

  // Make a publisher
  introspection_utils->publisher = allocator->allocate(sizeof(rcl_publisher_t), allocator->state);
  *introspection_utils->publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * service_event_typesupport = 
    ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, ServiceEvent);  // ROSIDL_GET_MSG_TYPE_SUPPORT gives an int?? And complier warns that it is being converted to a ptr
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(introspection_utils->publisher, node,
    service_event_typesupport, service_event_name, &publisher_options);


  // there has to be a macro for this, right?
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  // make a clock
  introspection_utils->clock = allocator->allocate(sizeof(rcl_clock_t), allocator->state);
  ret = rcl_clock_init(RCL_STEADY_TIME, introspection_utils->clock, allocator);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }
  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "Clock initialized");

  allocator->deallocate(service_event_name, allocator->state);

  return RCL_RET_OK;
}


rcl_ret_t
rcl_service_introspection_fini(
  rcl_service_introspection_utils_t * introspection_utils,
  rcl_allocator_t * allocator,
  rcl_node_t * node)
{
  rcl_ret_t ret;
  ret = rcl_publisher_fini(introspection_utils->publisher, node);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }
  ret = rcl_clock_fini(introspection_utils->clock);
  if (RCL_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rcl_get_error_string().str);
    return RCL_RET_ERROR;
  }

  allocator->deallocate(introspection_utils->publisher, allocator->state);
  allocator->deallocate(introspection_utils->clock, allocator->state);
  allocator->deallocate(introspection_utils->service_name, allocator->state);


  return RCL_RET_OK;

}



rcl_ret_t 
rcl_introspection_send_message(
  const rcl_service_introspection_utils_t * introspection_utils,
  const uint8_t event_type,
  const void * ros_response_request,
  const rmw_request_id_t * header,
  const rcl_service_options_t * options)
{
  // need a copy of this function for request/response (?) and also in client.c unless there's 
  // a utils.c or something...
  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Sending introspection message -----------------");
  
  rcl_ret_t ret;
  rcl_serialized_message_t serialized_message = rmw_get_zero_initialized_serialized_message();

  ret = rmw_serialized_message_init(&serialized_message, 0u, &options->allocator); // why is this 0u?
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message -----------------");
  rcl_interfaces__msg__ServiceEvent msg;
  rcl_interfaces__msg__ServiceEvent__init(&msg);
  msg.event_type = event_type;
  msg.sequence_number = header->sequence_number;
  rosidl_runtime_c__String__assign(&msg.service_name, introspection_utils->service_name);

  /* printf("sequence_number: %ld\n", msg.sequence_number);
  printf("service_name: %s\n", msg.service_name.data);
  printf("event_type: %d\n", msg.event_type); */
   
  rosidl_runtime_c__String__assign(&msg.event_name, "AddTwoInts"); // TODO(ihasdapie): get this properly, the type name is hardcoded throughout right now

  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: Building time message -----------------");
  builtin_interfaces__msg__Time timestamp;
  builtin_interfaces__msg__Time__init(&timestamp);

  rcl_time_point_value_t now;
  ret = rcl_clock_get_now(introspection_utils->clock, &now);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }
  // is there a rcl method for getting the seconds too?
  timestamp.sec = RCL_NS_TO_S(now);
  timestamp.nanosec = now % (1000LL * 1000LL * 1000LL);
  msg.stamp = timestamp;

  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: Building UUID message -----------------");
  unique_identifier_msgs__msg__UUID uuid;
  unique_identifier_msgs__msg__UUID__init(&uuid);
  memcpy(uuid.uuid, header->writer_guid, 16 * sizeof(uint8_t));
  msg.client_id = uuid;

  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: Serializing introspected message -----------------");

  rosidl_message_type_support_t * serialized_message_ts;
  switch (event_type) {
    case rcl_interfaces__msg__ServiceEventType__REQUEST_RECEIVED:
      serialized_message_ts = introspection_utils->request_type_support;
      break;
    case rcl_interfaces__msg__ServiceEventType__REQUEST_SENT:
      serialized_message_ts = introspection_utils->response_type_support;
      break;
    case rcl_interfaces__msg__ServiceEventType__RESPONSE_RECEIVED:
      serialized_message_ts = introspection_utils->response_type_support;
      break;
    case rcl_interfaces__msg__ServiceEventType__RESPONSE_SENT:
      serialized_message_ts = introspection_utils->request_type_support;
      break;
    default:
      RCL_SET_ERROR_MSG("Invalid event type");
      return RCL_RET_ERROR;
  }

  ret = rmw_serialize(ros_response_request, serialized_message_ts, &serialized_message);
  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Building ServiceEvent Message: memcpying introspected message -----------------");
  // printf("serialized_message.buffer_length: %zu, serialized_message size: %zu\n", serialized_message.buffer_length, serialized_message.buffer_capacity);
  rosidl_runtime_c__octet__Sequence__init(&msg.serialized_event, serialized_message.buffer_length);

  // This segfaults, but thought this should be ok? since serialized_message.buffer is a uint8_t* which _should_ be the same as a byte array
  // rosidl_runtime_c__octet__Sequence__copy((rosidl_runtime_c__octet__Sequence *) serialized_message.buffer, &msg.serialized_event);

  memcpy(msg.serialized_event.data, serialized_message.buffer, serialized_message.buffer_length);


  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Publishing ServiceEvent Message -----------------");
  ret = rcl_publish(introspection_utils->publisher, &msg, NULL);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  // RCUTILS_LOG_INFO_NAMED(ROS_PACKAGE_NAME, "--------------- Send Introspection Message: Cleanup -----------------");

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
