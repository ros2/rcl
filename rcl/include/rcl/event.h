// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__EVENT_H_
#define RCL__EVENT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/macros.h"
#include "rcl/visibility_control.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"
#include "rcl/client.h"
#include "rcl/service.h"


/// Internal rcl implementation struct.
struct rcl_event_impl_t;

/// Structure which encapsulates a ROS Subscription.
typedef struct rcl_event_t
{
  struct rcl_event_impl_t * impl;
} rcl_event_t;


/// Return a rcl_event_t struct with members set to `NULL`.
/**
 * Should be called to get a null rcl_event_t before passing to
 * rcl_event_init().
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_event_t
rcl_get_zero_initialized_event(void);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_publisher_event_init(
  rcl_event_t * event,
  const rcl_publisher_t * publisher);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_subscription_event_init(
  rcl_event_t * event,
  const rcl_subscription_t * subscription);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_client_event_init(
  rcl_event_t * event,
  const rcl_client_t * client);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_service_event_init(
  rcl_event_t * event,
  const rcl_service_t * service);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_event_fini(rcl_event_t * event);


#ifdef __cplusplus
}
#endif

#endif  // RCL__EVENT_H_
