// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__EVENT_IMPL_H_
#define RCL__EVENT_IMPL_H_

#include "rmw/rmw.h"

#include "rcl/event.h"

typedef struct rcl_event_impl_t
{
  rmw_event_t rmw_handle;
  rcl_allocator_t allocator;
} rcl_event_impl_t;

#endif  // RCL__EVENT_IMPL_H_
