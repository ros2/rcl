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

#include "rcl/service.h"
#include "rcl/introspection.h"
#include "rcl_interfaces/msg/service_event.h"
#include "rcl/macros.h"
#include "rmw/rmw.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"
#include "publisher.h"

rcl_ret_t 
send_introspection_message(
  rcl_service_t * service,
  void * service_payload,
  rmw_request_id_t * header,
  rcl_allocator_t * allocator)
{
  rcl_ret_t ret;
  rcl_serialized_message_t serialized_message;
  ret = rmw_serialized_message_init(&serialized_message, 0u, allocator); // why is this 0u?
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  rcl_interfaces__msg__ServiceEvent msg;
  rcl_interfaces__msg__ServiceEvent__init(msg);
  msg.event_type;
  msg.client_id = header->request_id;
  msg.sequence_number = header->sequence_numberl
  msg.stamp;


  rosidl_runtime_c__String__assign(&msg.service_name, rcl_service_get_service_name(service));
  rosidl_runtime_c__String__assign(&msg.event_name, rcl_service_get); // ..._{Request, Response}); // TODO(ihasdapie): impl this
  rosidl_runtime_c__octet__Sequence__init(&msg.serialized_event, serialized_message.buffer_length);
  memcpy(msg.serialized_event.data, serialized_message.buffer, serialized_message.buffer_length);

  ret = rcl_publish(service->impl->service_event_publisher, &serialized_message, NULL);
  if (RMW_RET_OK != ret) { // there has to be a macro for this right?
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  ret = rmw_serialized_message_fini(&serialized_message);
  if (RMW_RET_OK != ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return RCL_RET_ERROR;
  }

  // have to fini the request/response message as well
  rcl_interfaces__msg__ServiceEvent__fini(&msg);




  return ret;
}


