// Copyright 2025 cellumation GmbH
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

#ifndef RCL__GUARD_CONDITION_IMPL_H_
#define RCL__GUARD_CONDITION_IMPL_H_

#include "rcl/guard_condition.h"

struct rcl_guard_condition_impl_s
{
  rmw_guard_condition_t * rmw_handle;
  bool allocated_rmw_guard_condition;
  rcl_guard_condition_options_t options;
  bool in_use_by_waitset;
};

#endif  // RCL__GUARD_CONDITION_IMPL_H_
