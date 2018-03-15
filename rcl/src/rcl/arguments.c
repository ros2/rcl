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

#include "./arguments_impl.h"
#include "./remap_impl.h"
#include "rcl/error_handling.h"
#include "rcl/validate_topic_name.h"
#include "rcutils/allocator.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#if __cplusplus
extern "C"
{
#endif

// Instance of global arguments.
static rcl_arguments_t __rcl_global_arguments;

/// Parse an argument that may or may not be a remap rule.
/// \param[in] arg the argument to parse
/// \param[in] allocator an allocator to use
/// \param[in,out] output_rule input a zero intialized rule, output a fully initialized one
/// \return RCL_RET_OK if a valid rule was parsed, or
/// \return RCL_RET_INVALID_REMAP_RULE if the argument is not a valid rule, or
/// \return RCL_RET_BAD_ALLOC if an allocation failed, or
/// \return RLC_RET_ERROR if an unspecified error occurred.
/// \internal
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_rule(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_remap_t * output_rule)
{
  size_t len_node_name = 0;
  size_t len_match = 0;
  size_t len_replacement = 0;

  const char * separator = NULL;
  const char * colon = NULL;
  const char * match_begin = arg;
  const char * replacement_begin = NULL;

  // A valid rule has two parts separated by :=
  separator = strstr(arg, ":=");
  if (NULL == separator) {
    RCL_SET_ERROR_MSG("missing :=", allocator);
    return RCL_RET_INVALID_REMAP_RULE;
  }

  replacement_begin = separator + 2;

  // must have characters on both sides of the separator
  len_match = separator - arg;
  len_replacement = strlen(replacement_begin);
  if (0 == len_match) {
    RCL_SET_ERROR_MSG("match is zero length", allocator);
    return RCL_RET_INVALID_REMAP_RULE;
  } else if (0 == len_replacement) {
    RCL_SET_ERROR_MSG("replacement has zero length", allocator);
    return RCL_RET_INVALID_REMAP_RULE;
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
        RCL_SET_ERROR_MSG("node name previx has zero length", allocator);
        return RCL_RET_INVALID_REMAP_RULE;
      }
    } else if (colon > separator) {
      // If the colon is on the replacement side then this couldn't be a valid rule
      RCL_SET_ERROR_MSG("replacement side cannot contain a :", allocator);
      return RCL_RET_INVALID_REMAP_RULE;
    }
  }

  // Maybe match length changed because there was a node name prefix
  if (0 == len_match) {
    RCL_SET_ERROR_MSG("match is zero length", allocator);
    return RCL_RET_INVALID_REMAP_RULE;
  }

  // Make sure node name contains only valid characters
  if (len_node_name) {
    int validation_result;
    size_t invalid_index;
    if (
      RMW_RET_OK != rmw_validate_node_name_with_size(arg, len_node_name, &validation_result,
      &invalid_index))
    {
      RCL_SET_ERROR_MSG("failed to run check on node name", allocator);
      return RCL_RET_ERROR;
    }
    if (RMW_NODE_NAME_VALID != validation_result) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        allocator,
        "node name prefix invalid: %s", rmw_node_name_validation_result_string(validation_result));
      return RCL_RET_INVALID_REMAP_RULE;
    }
  }

  // Figure out what type of rule this is, default is to apply to topic and service names
  rcl_remap_type_t type = RCL_TOPIC_REMAP | RCL_SERVICE_REMAP;
  if (0 == strncmp("__ns", match_begin, len_match)) {
    type = RCL_NAMESPACE_REMAP;
  } else if (0 == strncmp("__node", match_begin, len_match)) {
    type = RCL_NODENAME_REMAP;
  }

  if (type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
    // Replacement must be a valid topic name
    int validation_result;
    size_t invalid_index;
    rcl_ret_t ret = rcl_validate_topic_name(replacement_begin, &validation_result, &invalid_index);
    if (ret != RCL_RET_OK) {
      return RCL_RET_ERROR;
    } else if (validation_result != RCL_TOPIC_NAME_VALID) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        allocator,
        "replacement is invalid: %s", rcl_topic_name_validation_result_string(validation_result));
      return RCL_RET_INVALID_REMAP_RULE;
    }
    // Match must be a valid topic name
    ret = rcl_validate_topic_name_with_size(
      match_begin, len_match, &validation_result, &invalid_index);
    if (ret != RCL_RET_OK) {
      return RCL_RET_ERROR;
    } else if (validation_result != RCL_TOPIC_NAME_VALID) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        allocator,
        "match is invalid: %s", rcl_topic_name_validation_result_string(validation_result));
      return RCL_RET_INVALID_REMAP_RULE;
    }
  } else if (RCL_NAMESPACE_REMAP == type) {
    int validation_result;
    size_t invalid_idx;
    if (RMW_RET_OK != rmw_validate_namespace(replacement_begin, &validation_result, &invalid_idx)) {
      RCL_SET_ERROR_MSG("failed to run check on namespace", allocator);
      return RCL_RET_ERROR;
    }
    if (RMW_NAMESPACE_VALID != validation_result) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        allocator,
        "namespace is invalid: %s", rmw_namespace_validation_result_string(validation_result));
      return RCL_RET_INVALID_REMAP_RULE;
    }
  } else if (RCL_NODENAME_REMAP == type) {
    int validation_result;
    size_t invalid_idx;
    if (RMW_RET_OK != rmw_validate_node_name(replacement_begin, &validation_result, &invalid_idx)) {
      RCL_SET_ERROR_MSG("failed to run check on node name", allocator);
      return RCL_RET_ERROR;
    }
    if (RMW_NODE_NAME_VALID != validation_result) {
      RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
        allocator,
        "node name is invalid: %s", rmw_node_name_validation_result_string(validation_result));
      return RCL_RET_INVALID_REMAP_RULE;
    }
  }

  // Rule is valid, construct a structure for it
  output_rule->allocator = allocator;
  output_rule->type = type;
  if (len_node_name > 0) {
    output_rule->node_name = rcutils_strndup(arg, len_node_name, allocator);
    if (NULL == output_rule->node_name) {
      goto cleanup_rule;
    }
  }
  if (type & (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP)) {
    output_rule->match = rcutils_strndup(match_begin, len_match, allocator);
    if (NULL == output_rule->match) {
      goto cleanup_rule;
    }
  }
  output_rule->replacement = rcutils_strndup(replacement_begin, len_replacement, allocator);
  if (NULL == output_rule->replacement) {
    goto cleanup_rule;
  }
  return RCL_RET_OK;

cleanup_rule:
  if (RCL_RET_OK != rcl_remap_fini(output_rule)) {
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to fini remap rule after error occurred");
  }
  return RCL_RET_BAD_ALLOC;
}

rcl_ret_t
rcl_parse_arguments(
  int argc,
  const char * const argv[],
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

  rcl_ret_t ret;
  rcl_ret_t fail_ret;

  args_output->impl = allocator.allocate(sizeof(rcl_arguments_impl_t), allocator.state);
  if (NULL == args_output->impl) {
    return RCL_RET_BAD_ALLOC;
  }
  rcl_arguments_impl_t * args_impl = args_output->impl;
  args_impl->num_remap_rules = 0;
  args_impl->remap_rules = NULL;
  args_impl->unparsed_args = NULL;
  args_impl->num_unparsed_args = 0;
  args_impl->allocator = allocator;

  if (argc == 0) {
    // there are no arguments to parse
    return RCL_RET_OK;
  }

  // over-allocate arrays to match the number of arguments
  args_impl->remap_rules = allocator.allocate(sizeof(rcl_remap_t) * argc, allocator.state);
  if (NULL == args_impl->remap_rules) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }
  args_impl->unparsed_args = allocator.allocate(sizeof(int) * argc, allocator.state);
  if (NULL == args_impl->unparsed_args) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }

  // Attempt to parse arguments as remap rules
  for (int i = 0; i < argc; ++i) {
    rcl_remap_t * rule = &(args_impl->remap_rules[args_impl->num_remap_rules]);
    *rule = rcl_remap_get_zero_initialized();
    if (RCL_RET_OK == _rcl_parse_remap_rule(argv[i], allocator, rule)) {
      ++(args_impl->num_remap_rules);
    } else {
      RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "arg %d error '%s'", i, rcl_get_error_string());
      rcl_reset_error();
      args_impl->unparsed_args[args_impl->num_unparsed_args] = i;
      ++(args_impl->num_unparsed_args);
    }
  }

  // Shrink remap_rules array to match number of successfully parsed rules
  if (args_impl->num_remap_rules > 0) {
    args_impl->remap_rules = rcutils_reallocf(
      args_impl->remap_rules, sizeof(rcl_remap_t) * args_impl->num_remap_rules, &allocator);
    if (NULL == args_impl->remap_rules) {
      ret = RCL_RET_BAD_ALLOC;
      goto fail;
    }
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
    args_impl->unparsed_args = rcutils_reallocf(
      args_impl->unparsed_args, sizeof(int) * args_impl->num_unparsed_args, &allocator);
    if (NULL == args_impl->unparsed_args) {
      ret = RCL_RET_BAD_ALLOC;
      goto fail;
    }
  }

  return RCL_RET_OK;
fail:
  fail_ret = ret;
  if (NULL != args_impl) {
    ret = rcl_arguments_fini(args_output);
    if (RCL_RET_OK != ret) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to fini arguments after earlier failure");
    }
  }
  return fail_ret;
}

int
rcl_arguments_get_count_unparsed(
  rcl_arguments_t * args)
{
  if (NULL == args || NULL == args->impl) {
    return -1;
  }
  return args->impl->num_unparsed_args;
}

rcl_ret_t
rcl_arguments_get_unparsed(
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
  rcl_arguments_t * args)
{
  rcl_allocator_t alloc = rcl_get_default_allocator();
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT, alloc);
  if (args->impl) {
    rcl_ret_t ret = RCL_RET_OK;
    alloc = args->impl->allocator;
    if (args->impl->remap_rules) {
      for (int i = 0; i < args->impl->num_remap_rules; ++i) {
        rcl_ret_t remap_ret = rcl_remap_fini(&(args->impl->remap_rules[i]));
        if (remap_ret != RCL_RET_OK) {
          ret = remap_ret;
          RCUTILS_LOG_ERROR_NAMED(
            ROS_PACKAGE_NAME,
            "Failed to finalize remap rule while finalizing arguments. Continuing...");
        }
      }
      args->impl->allocator.deallocate(args->impl->remap_rules, args->impl->allocator.state);
      args->impl->remap_rules = NULL;
      args->impl->num_remap_rules = 0;
    }

    args->impl->allocator.deallocate(args->impl->unparsed_args, args->impl->allocator.state);
    args->impl->num_unparsed_args = 0;
    args->impl->unparsed_args = NULL;

    args->impl->allocator.deallocate(args->impl, args->impl->allocator.state);
    args->impl = NULL;
    return ret;
  }
  RCL_SET_ERROR_MSG("rcl_arguments_t finalized twice", alloc);
  return RCL_RET_ERROR;
}

RCL_PUBLIC
RCL_WARN_UNUSED
rcl_arguments_t *
rcl_get_global_arguments()
{
  return &__rcl_global_arguments;
}

#if __cplusplus
}
#endif
