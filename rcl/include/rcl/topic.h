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

#ifndef RCL__TOPIC_H_
#define RCL__TOPIC_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/rcl.h"
#include "rcl/allocator.h"
#include "rcl/types.h"
#include "rcl/macros.h"
#include "rcl/visibility_control.h"

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_get_remote_topic_names_and_types(rcl_strings_t* topic_names_string, rcl_strings_t* type_names_string);

#if __cplusplus
}
#endif

#endif  // RCL__TOPIC_H_
