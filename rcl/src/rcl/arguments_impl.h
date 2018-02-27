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

#ifndef RCL__ARGUMENTS_IMPL_H_
#define RCL__ARGUMENTS_IMPL_H_

#include "rcl/arguments.h"
#include "./remap_impl.h"

#if __cplusplus
extern "C"
{
#endif


typedef struct rcl_arguments_impl_t
{
  // TODO(sloretz) array of namespace replacement rules
  // TODO(sloretz) node name replacement rules
  /// \brief A namespace replacement rule
  rcl_remap_t namespace_replacement;

  /// \brief Array of rules for name replacement
  rcl_remap_t * topic_remaps;
  int num_topic_remaps;
} rcl_arguments_impl_t;


/// \brief Global instance of parsed arguments
/// \sa rcl_init(int, char **, rcl_allocator_t)
/// \internal
RCL_LOCAL
extern rcl_arguments_t __rcl_arguments;


#if __cplusplus
}
#endif

#endif  // RCL__ARGUMENTS_IMPL_H_
