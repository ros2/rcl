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

#include "rcl/client.h"
#include "rcl/macros.h"
#include "rcl/publisher.h"
#include "rcl/service.h"
#include "rcl/subscription.h"
#include "rcl/visibility_control.h"


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
rcl_take_event(
        const rcl_event_t * event,
        void * event_status);


RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_event_fini(rcl_event_t * event);


/// Return the rmw event handle.
/**
 * The handle returned is a pointer to the internally held rmw handle.
 * This function can fail, and therefore return `NULL`, if the:
 *   - event is `NULL`
 *   - event is invalid (never called init, called fini, or invalid node)
 *
 * The returned handle is made invalid if the event is finalized or if
 * rcl_shutdown() is called.
 * The returned handle is not guaranteed to be valid for the life time of the
 * event as it may be finalized and recreated itself.
 * Therefore it is recommended to get the handle from the event using
 * this function each time it is needed and avoid use of the handle
 * concurrently with functions that might change it.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] event pointer to the rcl event
 * \return rmw event handle if successful, otherwise `NULL`
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rmw_event_t *
rcl_event_get_rmw_handle(const rcl_event_t * event);

#ifdef __cplusplus
}
#endif

#endif  // RCL__EVENT_H_
