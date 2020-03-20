// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__WAIT_SET_IMPL_H_
#define RCL__WAIT_SET_IMPL_H_

#include <inttypes.h>

#include "rcl/context.h"
#include "rcl/allocator.h"
#include "rmw/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \internal
typedef struct rcl_wait_set_impl_t
{
  // number of subscriptions that have been added to the wait set
  size_t subscription_index;
  rmw_subscriptions_t rmw_subscriptions;
  // number of guard_conditions that have been added to the wait set
  size_t guard_condition_index;
  rmw_guard_conditions_t rmw_guard_conditions;
  // number of clients that have been added to the wait set
  size_t client_index;
  rmw_clients_t rmw_clients;
  // number of services that have been added to the wait set
  size_t service_index;
  rmw_services_t rmw_services;
  // number of events that have been added to the wait set
  size_t event_index;
  rmw_events_t rmw_events;

  rmw_wait_set_t * rmw_wait_set;
  // number of timers that have been added to the wait set
  size_t timer_index;
  // context with which the wait set is associated
  rcl_context_t * context;
  // allocator used in the wait set
  rcl_allocator_t allocator;
} rcl_wait_set_impl_t;

#ifdef __cplusplus
}
#endif

#endif  // RCL__WAIT_SET_IMPL_H_
