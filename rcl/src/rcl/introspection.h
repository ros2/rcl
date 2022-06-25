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


#ifndef RCL__SERVICE_INTROSPECTION_H_
#define RCL__SERVICE_INTROSPECTION_H_

#include "rcl/time.h"
#include "rcl/publisher.h"
#include "rcl/service.h"
#include "rmw/rmw.h"



#define RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX "/_service_event"

typedef struct rcl_service_introspection_utils_s {
  rcl_clock_t * clock;
  rcl_publisher_t * publisher;
  rosidl_message_type_support_t * request_type_support;
  rosidl_message_type_support_t * response_type_support;
} rcl_service_introspection_utils_t;


rcl_ret_t 
send_introspection_message(
  const rcl_service_t * service,
  uint8_t event_type,
  void * ros_response_request,
  rmw_request_id_t * header,
  const rcl_service_options_t * options);


#endif // RCL__SERVICE_INTROSPECTION_H_
