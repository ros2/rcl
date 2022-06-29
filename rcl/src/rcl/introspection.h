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

#include "rcl/publisher.h"
#include "rcl/service.h"
#include "rcl/time.h"
#include "rmw/rmw.h"

#define RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX "/_service_event"

typedef struct rcl_service_introspection_utils_s {
  rcl_clock_t * clock;
  rcl_publisher_t * publisher;
  rosidl_message_type_support_t * request_type_support;
  rosidl_message_type_support_t * response_type_support;
  char * service_name;
  char * service_type_name;
} rcl_service_introspection_utils_t;

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_service_introspection_utils_t
rcl_get_zero_initialized_introspection_utils();

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_init(
  rcl_service_introspection_utils_t * introspection_utils,
  const rosidl_service_type_support_t * service_type_support,
  const char * service_name,
  const rcl_node_t * node,
  rcl_allocator_t * allocator);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_fini(
  rcl_service_introspection_utils_t * introspection_utils,
  rcl_allocator_t * allocator,
  rcl_node_t *  node);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_introspection_send_message(
  const rcl_service_introspection_utils_t * introspection_utils,
  const uint8_t event_type,
  const void * ros_response_request,
  const int64_t sequence_number,
  const uint8_t uuid[16], // uuid is uint8_t but the guid is int8_t
  const rcl_allocator_t * allocator);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_disable(rcl_service_t * service);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_enable(rcl_service_t * service);



#endif // RCL__SERVICE_INTROSPECTION_H_
