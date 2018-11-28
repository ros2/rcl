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

#ifdef __cplusplus
extern "C"
{
#endif

/// \internal
typedef struct rcl_arguments_impl_t
{
  /// Array of indices that were not valid ROS arguments.
  int * unparsed_args;
  /// Length of unparsed_args.
  int num_unparsed_args;

  // TODO(mikaelarguedas) consider storing CLI parameter rules here
  /// Array of yaml parameter file paths
  char ** parameter_files;
  /// Length of parameter_files.
  int num_param_files_args;

  /// Array of rules for name remapping.
  rcl_remap_t * remap_rules;
  /// Length of remap_rules.
  int num_remap_rules;

  /// Default log level (represented by `RCUTILS_LOG_SEVERITY` enum) or -1 if not specified.
  int log_level;
  /// A file used to configure the external logging library
  char * external_log_config_file;
  /// A boolean value indicating if the standard out handler should be used for log output
  bool log_stdout_disabled;
  /// A boolean value indicating if the rosout topic handler should be used for log output
  bool log_rosout_disabled;
  /// A boolean value indicating if the external lib handler should be used for log output
  bool log_ext_lib_disabled;

  /// Allocator used to allocate objects in this struct
  rcl_allocator_t allocator;
} rcl_arguments_impl_t;

#ifdef __cplusplus
}
#endif

#endif  // RCL__ARGUMENTS_IMPL_H_
