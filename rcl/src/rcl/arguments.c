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

#include "rcl/arguments.h"

#include <string.h>

#include "rcl/error_handling.h"
#include "rcl/validate_topic_name.h"
#include "rcutils/logging_macros.h"
#include "./arguments_impl.h"
#include "./remap_impl.h"

#if __cplusplus
extern "C"
{
#endif

// instance of global arguments
rcl_arguments_t __rcl_arguments;


rcl_ret_t
rcl_parse_arguments(
  int argc,
  const char ** argv,
  rcl_allocator_t allocator,
  rcl_arguments_t * args_output)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  if (argc < 0) {
    return RCL_RET_INVALID_ARGUMENT;
  } else if (argc > 0) {
    RCL_CHECK_ARGUMENT_FOR_NULL(argv, RCL_RET_INVALID_ARGUMENT, allocator);
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(args_output, RCL_RET_INVALID_ARGUMENT, allocator);

  args_output->impl = allocator.allocate(sizeof(rcl_arguments_impl_t), allocator.state);
  if (NULL == args_output->impl) {
    return RCL_RET_BAD_ALLOC;
  }
  rcl_arguments_impl_t * args_impl = args_output->impl;
  args_impl->namespace_replacement = rcl_remap_get_zero_initialized();
  args_impl->num_topic_remaps = 0;
  args_impl->topic_remaps = NULL;

  if (argc == 0 || argc == 1) {
    // first argument is assumed to be the process name
    // there are no arguments to parse
    return RCL_RET_OK;
  }

  // Allocate size to match the number of arguments
  args_impl->topic_remaps = allocator.allocate(sizeof(rcl_remap_t) * (argc - 1), allocator.state);
  if (NULL == args_impl->topic_remaps) {
    return RCL_RET_BAD_ALLOC;
  }

  for (int i = 1; i < argc; ++i) {
    const char * arg = argv[i];
    // TODO(sloretz) replace code below with implementation of ROS2 static remapping design doc
    // Parses ROS1 style remmapping args
    const char * separator = strstr(arg, ":=");
    if (NULL == separator) {
      // assume not a ROS argument
      continue;
    }

    int len_match = separator - arg;
    int len_replacement = strlen(separator + 2);
    if (len_match <= 0 || len_replacement <= 0) {
      // ill-formed; assume it is not a ROS argument
      continue;
    }

    int len_node_name = 0;
    // Split match by ":" because it could have a node-name prefix
    const char * colon = strchr(arg, ':');
    if (colon < separator) {
      len_node_name = colon - arg;
      len_match = separator - colon - 1;
    }

    if (len_match <= 0) {
      // ill-formed; assume it is not a ROS argument
      continue;
    }

    if (0 == strncmp(arg, "__ns", len_match)) {
      // namespace replacement rule
      if (args_impl->namespace_replacement.replacement != NULL) {
        // Already have namespace replacement and ROS1 doesn't support multiple
        continue;
      }
      // TODO(sloretz) make sure names are valid before storing the rule
      args_impl->namespace_replacement.replacement = allocator.allocate(
        len_replacement + 1, allocator.state);
      strncpy(args_impl->namespace_replacement.replacement, separator + 2, len_replacement);
      args_impl->namespace_replacement.replacement[len_replacement] = '\0';
    } else {
      // topic remap rule
      // TODO(sloretz) make sure names are valid before storing the rule
      rcl_remap_t * rule = &(args_impl->topic_remaps[args_impl->num_topic_remaps]);
      *rule = rcl_remap_get_zero_initialized();
      ++(args_impl->num_topic_remaps);

      rule->match = allocator.allocate(sizeof(char) * len_match + 1, allocator.state);
      if (NULL == rule->match) {
        return RCL_RET_BAD_ALLOC;
      }
      rule->replacement = allocator.allocate(sizeof(char) * len_replacement + 1, allocator.state);
      if (NULL == rule->replacement) {
        return RCL_RET_BAD_ALLOC;
      }
      strncpy(rule->match, arg, len_match);
      rule->match[len_match] = '\0';
      strncpy(rule->replacement, separator + 2, len_replacement);
      rule->replacement[len_replacement] = '\0';
    }
  }

  if (args_impl->num_topic_remaps > 0) {
    // Shrink topic remap rules array to match number parsed
    void * shrunk_rules = allocator.reallocate(
      args_impl->topic_remaps, sizeof(rcl_remap_t) * args_impl->num_topic_remaps, allocator.state);
    if (NULL == shrunk_rules) {
      return RCL_RET_BAD_ALLOC;
    }
    args_impl->topic_remaps = shrunk_rules;
  } else {
    // No remap rules
    allocator.deallocate(args_impl->topic_remaps, allocator.state);
    args_impl->topic_remaps = NULL;
  }

  return RCL_RET_OK;
}


rcl_ret_t
rcl_arguments_fini(
  rcl_arguments_t * args,
  rcl_allocator_t allocator)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT, allocator);
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Finalizing arguments");
  if (args->impl) {
    if (args->impl->topic_remaps) {
      for (int i = 0; i < args->impl->num_topic_remaps; ++i) {
        rcl_remap_fini(&(args->impl->topic_remaps[i]), allocator);
      }
      allocator.deallocate(args->impl->topic_remaps, allocator.state);
      args->impl->topic_remaps = NULL;
      args->impl->num_topic_remaps = 0;
    }
    rcl_remap_fini(&(args->impl->namespace_replacement), allocator);

    allocator.deallocate(args->impl, allocator.state);
    args->impl = NULL;
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Arguments finalized");
    return RCL_RET_OK;
  }
  RCUTILS_LOG_WARN_NAMED(ROS_PACKAGE_NAME, "Arguments finalized_twice");
  return RCL_RET_ERROR;
}


#if __cplusplus
}
#endif
