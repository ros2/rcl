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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/publisher.h"
#include "rcl/service.h"
#include "rcl/time.h"
#include "rmw/rmw.h"
#include "rcl/client.h"

#define RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX "/_service_event"

typedef struct rcl_service_introspection_utils_s {
  rcl_clock_t * clock;
  rcl_publisher_t * publisher;
  rosidl_message_type_support_t * request_type_support;
  rosidl_message_type_support_t * response_type_support;
  char * service_name;
  char * service_type_name;
  char * service_event_topic_name;
  bool _enabled;
  bool _content_enabled;
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





/*  Enables service introspection by reconstructing the introspection clock and publisher
 *  
 *  Does nothing and returns RCL_RET_OK if already enabled
 */
RCL_LOCAL
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_enable(
    rcl_service_introspection_utils_t * introspection_utils,
    const rcl_node_t * node,
    rcl_allocator_t * allocator);

/*  Disabled service introspection by fini-ing and freeing the introspection clock and publisher
 *  
 *  Does nothing and returns RCL_RET_OK if already disabled
 *
 *
 *
 */

RCL_LOCAL
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_disable(
    rcl_service_introspection_utils_t * introspection_utils,
    rcl_node_t * node,
    const rcl_allocator_t * allocator);




/*
 * Enables/disables service introspection for client/service
 * These functions are thin wrappers around rcl_service_introspection_{enable, disable}
 *
 *
 *
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_enable_service_events(rcl_service_t * service, rcl_node_t * node);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_disable_service_events(rcl_service_t * service, rcl_node_t * node);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_enable_client_events(rcl_client_t * client, rcl_node_t * node);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_disable_client_events(rcl_client_t * client, rcl_node_t * node);


/*
 *
 */
RCL_PUBLIC
void
rcl_service_introspection_enable_client_content(rcl_client_t * client);

RCL_PUBLIC
void
rcl_service_introspection_enable_service_content(rcl_service_t * service);

RCL_PUBLIC
void
rcl_service_introspection_disable_client_content(rcl_client_t * client);

RCL_PUBLIC
void
rcl_service_introspection_disable_service_content(rcl_service_t * service);



// TODO(ihasdapie): Do we want some getters for if content and/or introspection is enabled/disabled?








#ifdef __cplusplus
}
#endif


#endif // RCL__SERVICE_INTROSPECTION_H_
