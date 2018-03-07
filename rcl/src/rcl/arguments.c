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
#include "rcutils/strdup.h"
#include "./arguments_impl.h"
#include "./remap_impl.h"

#if __cplusplus
extern "C"
{
#endif

// instance of global arguments
rcl_arguments_t __rcl_arguments;


/// \return true if c is in [a-zA-Z0-9_]
/// \internal
RCL_LOCAL
bool
_rcl_valid_token_char(char c)
{
  // assumes ASCII
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}


/// Parse an argument that may or may not be a remap rule
/// \param[in] arg the argument to parse
/// \param[in] allocator an allocator to use
/// \param[in,out] output_rule input a zero intialized rule, output a fully initialized one
/// \return RCL_RET_OK if a valid rule was parsed
/// \return RLC_RET_ERROR if no valid rule was parsed
/// \internal
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_rule(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_remap_t * output_rule)
{
  int len_node_name = 0;
  int len_match = 0;
  int len_replacement = 0;

  const char * separator = NULL;
  const char * colon = NULL;
  const char * match_begin = arg;

  // A valid rule has two parts separated by :=
  separator = strstr(arg, ":=");
  if (NULL == separator) {
    return RCL_RET_ERROR;
  }

  // must have characters on both sides of the separator
  len_match = separator - arg;
  len_replacement = strlen(separator + 2);
  if (len_match <= 0 || len_replacement <= 0) {
    return RCL_RET_ERROR;
  }

  colon = strchr(arg, ':');
  if (NULL != colon) {
    if (colon < separator) {
      // If there is a : on the match side then there is a node-name prefix
      match_begin = colon + 1;
      len_node_name = colon - arg;
      len_match = separator - match_begin;
      // node name must have at least one character
      if (len_node_name <= 0) {
        return RCL_RET_ERROR;
      }
    } else if (colon > separator) {
      // If the colon is on the replacement side then this couldn't be a valid rule
      return RCL_RET_ERROR;
    }
  }

  // match side must have at least 1 character
  if (len_match <= 0) {
    return RCL_RET_ERROR;
  }

  // Make sure node name contains only valid characters
  for (int i = 0; i < len_node_name; ++i) {
    if (!_rcl_valid_token_char(arg[i])) {
      return RCL_RET_ERROR;
    }
  }

  // Figure out what type of rule this is, default is to apply to topic and service names
  rcl_remap_type_t type = RCL_TOPIC_REMAP | RCL_SERVICE_REMAP;
  if (0 == strncmp("__ns", match_begin, len_match)) {
    type = RCL_NAMESPACE_REMAP;
  } else if (0 == strncmp("__node", match_begin, len_match)) {
    type = RCL_NODENAME_REMAP;
  }

  if (type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP | RCL_NAMESPACE_REMAP)) {
    // Replacement must be a valid topic name
    char * copy_replacement = rcutils_strndup(separator + 2, len_replacement, allocator);
    if (NULL == copy_replacement) {
      return RCL_RET_ERROR;
    }
    int validation_result;
    size_t invalid_index;
    rcl_ret_t ret = rcl_validate_topic_name(copy_replacement, &validation_result, &invalid_index);
    if (ret != RCL_RET_OK || validation_result != RCL_TOPIC_NAME_VALID) {
      ret = RCL_RET_ERROR;
    }
    // namespace replacement must be fully qualified
    if (RCL_NAMESPACE_REMAP == type && '/' != copy_replacement[0]) {
      ret = RCL_RET_ERROR;
    }
    allocator.deallocate(copy_replacement, allocator.state);
    if (RCL_RET_OK != ret) {
      return RCL_RET_ERROR;
    }
  }
  if (type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
    // Match must be a valid topic name
    char * copy_match = rcutils_strndup(match_begin, len_match, allocator);
    if (NULL == copy_match) {
      return RCL_RET_ERROR;
    }
    int validation_result;
    size_t invalid_index;
    rcl_ret_t ret = rcl_validate_topic_name(copy_match, &validation_result, &invalid_index);
    if (ret != RCL_RET_OK || validation_result != RCL_TOPIC_NAME_VALID) {
      ret = RCL_RET_ERROR;
    }
    allocator.deallocate(copy_match, allocator.state);
    if (RCL_RET_OK != ret) {
      return RCL_RET_ERROR;
    }
  }
  if (RCL_NODENAME_REMAP == type) {
    // Replacement may only be a token
    for (int i = 0; i < len_replacement; ++i) {
      if (!_rcl_valid_token_char(separator[i + 2])) {
        return RCL_RET_ERROR;
      }
    }
  }

  // Rule is valid, construct a structure for it
  output_rule->type = type;
  if (len_node_name > 0) {
    output_rule->node_name = rcutils_strndup(arg, len_node_name, allocator);
    if (NULL == output_rule->node_name) {
      goto cleanup_rule;
    }
  }
  if (type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
    output_rule->match = rcutils_strndup(separator - len_match, len_match, allocator);
    if (NULL == output_rule->match) {
      goto cleanup_rule;
    }
  }
  output_rule->replacement = rcutils_strndup(separator + 2, len_replacement, allocator);
  if (NULL == output_rule->replacement) {
    goto cleanup_rule;
  }
  return RCL_RET_OK;

cleanup_rule:
  rcl_remap_fini(output_rule, allocator);
  return RCL_RET_ERROR;
}


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
  args_impl->num_remap_rules = 0;
  args_impl->remap_rules = NULL;
  args_impl->unparsed_args = NULL;
  args_impl->num_unparsed_args = 0;

  if (argc == 0) {
    // there are no arguments to parse
    return RCL_RET_OK;
  }

  // over-allocate arrays to match the number of arguments
  args_impl->remap_rules = allocator.allocate(sizeof(rcl_remap_t) * argc, allocator.state);
  if (NULL == args_impl->remap_rules) {
    return RCL_RET_BAD_ALLOC;
  }
  args_impl->unparsed_args = allocator.allocate(sizeof(int) * argc, allocator.state);
  if (NULL == args_impl->unparsed_args) {
    allocator.deallocate(args_impl->remap_rules, allocator.state);
    args_impl->remap_rules = NULL;
    return RCL_RET_BAD_ALLOC;
  }

  // Attempt to parse arguments are remap rules
  for (int i = 0; i < argc; ++i) {
    rcl_remap_t * rule = &(args_impl->remap_rules[args_impl->num_remap_rules]);
    *rule = rcl_remap_get_zero_initialized();
    if (RCL_RET_OK == _rcl_parse_remap_rule(argv[i], allocator, rule)) {
      ++(args_impl->num_remap_rules);
    } else {
      args_impl->unparsed_args[args_impl->num_unparsed_args] = i;
      ++(args_impl->num_unparsed_args);
    }
  }

  // Shrink remap_rules array to match number of successfully parsed rules
  if (args_impl->num_remap_rules > 0) {
    void * shrunk_rules = allocator.reallocate(
      args_impl->remap_rules, sizeof(rcl_remap_t) * args_impl->num_remap_rules, allocator.state);
    if (NULL == shrunk_rules) {
      return RCL_RET_BAD_ALLOC;
    }
    args_impl->remap_rules = shrunk_rules;
  } else {
    // No remap rules
    allocator.deallocate(args_impl->remap_rules, allocator.state);
    args_impl->remap_rules = NULL;
  }
  // Shrink unparsed_args
  if (0 == args_impl->num_unparsed_args) {
    // No unparsed args
    allocator.deallocate(args_impl->unparsed_args, allocator.state);
    args_impl->unparsed_args = NULL;
  } else if (args_impl->num_unparsed_args < argc) {
    void * shrunk = allocator.reallocate(
      args_impl->unparsed_args, sizeof(int) * args_impl->num_unparsed_args, allocator.state);
    if (NULL == shrunk) {
      return RCL_RET_BAD_ALLOC;
    }
    args_impl->unparsed_args = shrunk;
  }

  return RCL_RET_OK;
}

int
rcl_get_num_unparsed_arguments(
  rcl_arguments_t * args)
{
  if (NULL == args || NULL == args->impl) {
    return -1;
  }
  return args->impl->num_unparsed_args;
}

rcl_ret_t
rcl_get_unparsed_arguments(
  rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int ** output_unparsed_indices)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT, allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(args->impl, RCL_RET_INVALID_ARGUMENT, allocator);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_unparsed_indices, RCL_RET_INVALID_ARGUMENT, allocator);

  *output_unparsed_indices = NULL;
  if (args->impl->num_unparsed_args) {
    *output_unparsed_indices = allocator.allocate(
      sizeof(int) * args->impl->num_unparsed_args, allocator.state);
    if (NULL == *output_unparsed_indices) {
      return RCL_RET_BAD_ALLOC;
    }
    for (int i = 0; i < args->impl->num_unparsed_args; ++i) {
      (*output_unparsed_indices)[i] = args->impl->unparsed_args[i];
    }
  }
  return RCL_RET_OK;
}

rcl_arguments_t
rcl_get_zero_initialized_arguments(void)
{
  static rcl_arguments_t default_arguments = {
    .impl = NULL
  };
  return default_arguments;
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
    if (args->impl->remap_rules) {
      for (int i = 0; i < args->impl->num_remap_rules; ++i) {
        rcl_remap_fini(&(args->impl->remap_rules[i]), allocator);
      }
      allocator.deallocate(args->impl->remap_rules, allocator.state);
      args->impl->remap_rules = NULL;
      args->impl->num_remap_rules = 0;
    }

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
