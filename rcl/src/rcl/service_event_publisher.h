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

#ifndef RCL__SERVICE_EVENT_PUBLISHER_H_
#define RCL__SERVICE_EVENT_PUBLISHER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/macros.h"
#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/time.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#include "rosidl_runtime_c/service_type_support_struct.h"


typedef struct rcl_service_event_publisher_options_s
{
  // Enable/disable service introspection during runtime
  bool _enabled;
  // Enable/disable including request/response payload in service event message during runtime
  bool _content_enabled;
  /// Handle to clock for timestamping service events
  rcl_clock_t * clock;
  /// publisher options for service event publisher
  rcl_publisher_options_t publisher_options;
} rcl_service_event_publisher_options_t;

typedef struct rcl_service_event_publisher_impl_s
{
  /// Handle to publisher for publishing service events
  rcl_publisher_t * publisher;
  /// Name of service introspection topic: <service_name>/<RCL_SERVICE_INTROSPECTION_TOPIC_POSTFIX>
  char * service_event_topic_name;
  /// rcl_service_event_publisher options
  rcl_service_event_publisher_options_t options;
  /// Handle to service typesupport
  const rosidl_service_type_support_t * service_type_support;
} rcl_service_event_publisher_impl_t;

typedef struct rcl_service_event_publisher_s
{
  /// Pointer to implementation struct
  rcl_service_event_publisher_impl_t * impl;
} rcl_service_event_publisher_t;

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_service_event_publisher_options_t
rcl_service_event_publisher_get_default_options();

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_service_event_publisher_t
rcl_get_zero_initialized_service_event_publisher();

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_event_publisher_init(
  rcl_service_event_publisher_t * service_event_publisher,
  const rcl_node_t * node,
  const rcl_service_event_publisher_options_t * options,
  const char * service_name,
  const rosidl_service_type_support_t * service_type_support);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_event_publisher_fini(
  rcl_service_event_publisher_t * service_event_publisher,
  rcl_node_t * node);

RCL_PUBLIC
bool
rcl_service_event_publisher_is_valid(const rcl_service_event_publisher_t * service_event_publisher);

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_send_service_event_message(
  const rcl_service_event_publisher_t * service_event_publisher,
  uint8_t event_type,
  const void * ros_response_request,
  int64_t sequence_number,
  const uint8_t guid[16]);

/*  Enables service introspection by reconstructing the introspection clock and publisher
 *
 *  Does nothing and returns RCL_RET_OK if already enabled
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_enable(
  rcl_service_event_publisher_t * service_event_publisher,
  const rcl_node_t * node);

/*  Disables service introspection by fini-ing and freeing the introspection clock and publisher
 *
 *  Does nothing and returns RCL_RET_OK if already disabled
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_introspection_disable(
  rcl_service_event_publisher_t * service_event_publisher,
  rcl_node_t * node);

#ifdef __cplusplus
}
#endif
#endif  // RCL__SERVICE_EVENT_PUBLISHER_H_
