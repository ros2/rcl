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


#ifndef RCL__CLIENT_IMPL_H_
#define RCL__CLIENT_IMPL_H_


#include "rcl/client.h"

#include "rcutils/stdatomic_helper.h"
#include "rcl/introspection.h"

struct rcl_client_impl_s
{
  rcl_client_options_t options;
  rmw_qos_profile_t actual_request_publisher_qos;
  rmw_qos_profile_t actual_response_subscription_qos;
  rmw_client_t * rmw_handle;
  atomic_int_least64_t sequence_number;
  rcl_service_introspection_utils_t * introspection_utils;
};

#endif  // RCL__CLIENT_IMPL_H_
