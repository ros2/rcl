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
#include "rcl/macros.h"

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t 
send_introspection_message(
  rcl_service_t * service,
  void * service_payload,
  rmw_request_id_t * header,
  rcl_allocator_t * allocator);
