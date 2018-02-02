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

#ifndef RCL__REMAP_IMPL_H_
#define RCL__REMAP_IMPL_H_

#if __cplusplus
extern "C"
{
#endif

typedef struct rcl_remap_t
{
  /// \brief match portion of a rule
  char * match;
  /// \brief replacement portion of a rule
  char * replacement;
} rcl_remap_t;


#if __cplusplus
}
#endif

#endif  // RCL__REMAP_IMPL_H_
