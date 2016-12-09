// Copyright 2016 Open Source Robotics Foundation, Inc.
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


#ifndef RCL_LIFECYCLE__TRANSITION_MAP_H_
#define RCL_LIFECYCLE__TRANSITION_MAP_H_

#include <rcl_lifecycle/data_types.h>

#if __cplusplus
extern "C"
{
#endif

RCL_LIFECYCLE_PUBLIC
void
rcl_lifecycle_register_state(
  rcl_lifecycle_transition_map_t * map,
  rcl_lifecycle_state_t state);

RCL_LIFECYCLE_PUBLIC
void
rcl_lifecycle_register_transition(
  rcl_lifecycle_transition_map_t * transition_map,
  rcl_lifecycle_transition_t transition,
  const rcl_lifecycle_state_t * start,
  const rcl_lifecycle_state_t * goal,
  const rcl_lifecycle_state_t * error);

RCL_LIFECYCLE_PUBLIC
const rcl_lifecycle_state_t *
rcl_lifecycle_get_state(
  rcl_lifecycle_transition_map_t * transition_map,
  unsigned int state_id);

RCL_LIFECYCLE_PUBLIC
rcl_lifecycle_transition_array_t *
rcl_lifecycle_get_transitions(
  rcl_lifecycle_transition_map_t * transition_map,
  unsigned int state_id);

#if __cplusplus
}
#endif

#endif  // RCL_LIFECYCLE__TRANSITION_MAP_H_
