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

/// \cond INTERNAL  // Internal Doxygen documentation

#include "rcl/arguments.h"

#include <string.h>

#include "./arguments_impl.h"
#include "./remap_impl.h"
#include "rcl/error_handling.h"
#include "rcl/lexer_lookahead.h"
#include "rcl/validate_topic_name.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/logging.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Parse an argument that may or may not be a remap rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] output_rule input a zero intialized rule, output a fully initialized one
 * \return RCL_RET_OK if a valid rule was parsed, or
 * \return RCL_RET_INVALID_REMAP_RULE if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_rule(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_remap_t * output_rule);

/// Parse an argument that may or may not be a parameter file rule.
/**
 * The syntax of the file name is not validated.
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] param_file string that could be a parameter file name
 * \return RCL_RET_OK if the rule was parsed correctly, or
 * \return RCL_RET_INVALID_PARAM_RULE if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_param_file_rule(
  const char * arg,
  rcl_allocator_t allocator,
  char ** param_file);

rcl_ret_t
rcl_arguments_get_param_files(
  const rcl_arguments_t * arguments,
  rcl_allocator_t allocator,
  char *** parameter_files)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(arguments, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(arguments->impl, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_files, RCL_RET_INVALID_ARGUMENT);
  *(parameter_files) = allocator.allocate(
    sizeof(char *) * arguments->impl->num_param_files_args, allocator.state);
  if (NULL == *parameter_files) {
    return RCL_RET_BAD_ALLOC;
  }
  for (int i = 0; i < arguments->impl->num_param_files_args; ++i) {
    (*parameter_files)[i] = rcutils_strdup(arguments->impl->parameter_files[i], allocator);
    if (NULL == (*parameter_files)[i]) {
      // deallocate allocated memory
      for (int r = i; r >= 0; --r) {
        if (NULL == (*parameter_files[r])) {
          break;
        }
        allocator.deallocate((*parameter_files[r]), allocator.state);
      }
      allocator.deallocate((*parameter_files), allocator.state);
      (*parameter_files) = NULL;
      return RCL_RET_BAD_ALLOC;
    }
  }
  return RCL_RET_OK;
}

int
rcl_arguments_get_param_files_count(
  const rcl_arguments_t * args)
{
  if (NULL == args || NULL == args->impl) {
    return -1;
  }
  return args->impl->num_param_files_args;
}

/// Parse an argument that may or may not be a log level rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] log_level parsed log level represented by `RCUTILS_LOG_SEVERITY` enum
 * \return RCL_RET_OK if a valid log level was parsed, or
 * \return RCL_RET_INVALID_LOG_LEVEL_RULE if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_log_level_rule(
  const char * arg,
  rcl_allocator_t allocator,
  int * log_level);

rcl_ret_t
rcl_parse_arguments(
  int argc,
  const char * const argv[],
  rcl_allocator_t allocator,
  rcl_arguments_t * args_output)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  if (argc < 0) {
    RCL_SET_ERROR_MSG("Argument count cannot be negative");
    return RCL_RET_INVALID_ARGUMENT;
  } else if (argc > 0) {
    RCL_CHECK_ARGUMENT_FOR_NULL(argv, RCL_RET_INVALID_ARGUMENT);
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(args_output, RCL_RET_INVALID_ARGUMENT);

  if (args_output->impl != NULL) {
    RCL_SET_ERROR_MSG("Parse output is not zero-initialized");
    return RCL_RET_INVALID_ARGUMENT;
  }

  rcl_ret_t ret;
  rcl_ret_t fail_ret;

  args_output->impl = allocator.allocate(sizeof(rcl_arguments_impl_t), allocator.state);
  if (NULL == args_output->impl) {
    return RCL_RET_BAD_ALLOC;
  }
  rcl_arguments_impl_t * args_impl = args_output->impl;
  args_impl->num_remap_rules = 0;
  args_impl->remap_rules = NULL;
  args_impl->log_level = -1;
  args_impl->unparsed_args = NULL;
  args_impl->num_unparsed_args = 0;
  args_impl->parameter_files = NULL;
  args_impl->num_param_files_args = 0;
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
  args_impl->parameter_files = allocator.allocate(sizeof(char *) * argc, allocator.state);
  if (NULL == args_impl->parameter_files) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }

  for (int i = 0; i < argc; ++i) {
    // Attempt to parse argument as parameter file rule
    args_impl->parameter_files[args_impl->num_param_files_args] = NULL;
    if (
      RCL_RET_OK == _rcl_parse_param_file_rule(
        argv[i], allocator, &(args_impl->parameter_files[args_impl->num_param_files_args])))
    {
      ++(args_impl->num_param_files_args);
      RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
        "params rule : %s\n total num param rules %d",
        args_impl->parameter_files[args_impl->num_param_files_args - 1],
        args_impl->num_param_files_args);
      continue;
    }
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
      "Couldn't parse arg %d (%s) as parameter file rule. Error: %s", i, argv[i],
      rcl_get_error_string());
    rcl_reset_error();

    // Attempt to parse argument as remap rule
    rcl_remap_t * rule = &(args_impl->remap_rules[args_impl->num_remap_rules]);
    *rule = rcl_remap_get_zero_initialized();
    if (RCL_RET_OK == _rcl_parse_remap_rule(argv[i], allocator, rule)) {
      ++(args_impl->num_remap_rules);
      continue;
    }
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
      "Couldn't parse arg %d (%s) as remap rule. Error: %s", i, argv[i], rcl_get_error_string());
    rcl_reset_error();

    // Attempt to parse argument as log level configuration
    int log_level;
    if (RCL_RET_OK == _rcl_parse_log_level_rule(argv[i], allocator, &log_level)) {
      args_impl->log_level = log_level;
      continue;
    }
    RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME,
      "Couldn't parse arg %d (%s) as log level rule. Error: %s", i, argv[i],
      rcl_get_error_string());
    rcl_reset_error();

    // Argument wasn't parsed by any rule
    args_impl->unparsed_args[args_impl->num_unparsed_args] = i;
    ++(args_impl->num_unparsed_args);
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
  // Shrink Parameter files
  if (0 == args_impl->num_param_files_args) {
    allocator.deallocate(args_impl->parameter_files, allocator.state);
    args_impl->parameter_files = NULL;
  } else if (args_impl->num_param_files_args < argc) {
    args_impl->parameter_files = rcutils_reallocf(
      args_impl->parameter_files, sizeof(char *) * args_impl->num_param_files_args, &allocator);
    if (NULL == args_impl->parameter_files) {
      ret = RCL_RET_BAD_ALLOC;
      goto fail;
    }
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
  const rcl_arguments_t * args)
{
  if (NULL == args || NULL == args->impl) {
    return -1;
  }
  return args->impl->num_unparsed_args;
}

rcl_ret_t
rcl_arguments_get_unparsed(
  const rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int ** output_unparsed_indices)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args->impl, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_unparsed_indices, RCL_RET_INVALID_ARGUMENT);

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
rcl_remove_ros_arguments(
  char const * const argv[],
  const rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int * nonros_argc,
  const char ** nonros_argv[])
{
  RCL_CHECK_ARGUMENT_FOR_NULL(argv, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(nonros_argc, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);

  *nonros_argc = rcl_arguments_get_count_unparsed(args);
  *nonros_argv = NULL;

  if (*nonros_argc <= 0) {
    return RCL_RET_INVALID_ARGUMENT;
  }

  int * unparsed_indices = NULL;
  rcl_ret_t ret;
  ret = rcl_arguments_get_unparsed(args, allocator, &unparsed_indices);

  if (RCL_RET_OK != ret) {
    return ret;
  }

  size_t alloc_size = sizeof(char *) * *nonros_argc;
  *nonros_argv = allocator.allocate(alloc_size, allocator.state);
  if (NULL == *nonros_argv) {
    allocator.deallocate(unparsed_indices, allocator.state);
    return RCL_RET_BAD_ALLOC;
  }
  for (int i = 0; i < *nonros_argc; ++i) {
    (*nonros_argv)[i] = argv[unparsed_indices[i]];
  }

  allocator.deallocate(unparsed_indices, allocator.state);
  return RCL_RET_OK;
}

rcl_ret_t
rcl_arguments_copy(
  const rcl_arguments_t * args,
  rcl_arguments_t * args_out)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args->impl, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args_out, RCL_RET_INVALID_ARGUMENT);
  if (NULL != args_out->impl) {
    RCL_SET_ERROR_MSG("args_out must be zero initialized");
    return RCL_RET_INVALID_ARGUMENT;
  }

  rcl_allocator_t allocator = args->impl->allocator;

  args_out->impl = allocator.allocate(sizeof(rcl_arguments_impl_t), allocator.state);
  if (NULL == args_out->impl) {
    return RCL_RET_BAD_ALLOC;
  }

  args_out->impl->allocator = allocator;

  // Zero so it's safe to call rcl_arguments_fini() if an error occurrs while copying.
  args_out->impl->num_remap_rules = 0;
  args_out->impl->num_unparsed_args = 0;
  args_out->impl->num_param_files_args = 0;

  // Copy unparsed args
  args_out->impl->unparsed_args = allocator.allocate(
    sizeof(int) * args->impl->num_unparsed_args, allocator.state);
  if (NULL == args_out->impl->unparsed_args) {
    if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
      RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
    }
    return RCL_RET_BAD_ALLOC;
  }
  for (int i = 0; i < args->impl->num_unparsed_args; ++i) {
    args_out->impl->unparsed_args[i] = args->impl->unparsed_args[i];
  }
  args_out->impl->num_unparsed_args = args->impl->num_unparsed_args;

  // Copy remap rules
  args_out->impl->remap_rules = allocator.allocate(
    sizeof(rcl_remap_t) * args->impl->num_remap_rules, allocator.state);
  if (NULL == args_out->impl->remap_rules) {
    if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
      RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
    }
    return RCL_RET_BAD_ALLOC;
  }
  args_out->impl->num_remap_rules = args->impl->num_remap_rules;
  for (int i = 0; i < args->impl->num_remap_rules; ++i) {
    args_out->impl->remap_rules[i] = rcl_remap_get_zero_initialized();
    rcl_ret_t ret = rcl_remap_copy(
      &(args->impl->remap_rules[i]), &(args_out->impl->remap_rules[i]));
    if (RCL_RET_OK != ret) {
      if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
        RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
      }
      return ret;
    }
  }
  // Copy parameter files
  if (args->impl->num_param_files_args) {
    args_out->impl->parameter_files = allocator.allocate(
      sizeof(char *) * args->impl->num_param_files_args, allocator.state);
    if (NULL == args_out->impl->parameter_files) {
      if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
        RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
      }
      return RCL_RET_BAD_ALLOC;
    }
    args_out->impl->num_param_files_args = args->impl->num_param_files_args;
    for (int i = 0; i < args->impl->num_param_files_args; ++i) {
      args_out->impl->parameter_files[i] =
        rcutils_strdup(args->impl->parameter_files[i], allocator);
      if (NULL == args_out->impl->parameter_files[i]) {
        if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
          RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
        }
        return RCL_RET_BAD_ALLOC;
      }
    }
  } else {
    args_out->impl->parameter_files = NULL;
  }
  return RCL_RET_OK;
}

rcl_ret_t
rcl_arguments_fini(
  rcl_arguments_t * args)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT);
  if (args->impl) {
    rcl_ret_t ret = RCL_RET_OK;
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

    if (args->impl->parameter_files) {
      for (int p = 0; p < args->impl->num_param_files_args; ++p) {
        args->impl->allocator.deallocate(
          args->impl->parameter_files[p], args->impl->allocator.state);
      }
      args->impl->allocator.deallocate(args->impl->parameter_files, args->impl->allocator.state);
      args->impl->num_param_files_args = 0;
      args->impl->parameter_files = NULL;
    }

    args->impl->allocator.deallocate(args->impl, args->impl->allocator.state);
    args->impl = NULL;
    return ret;
  }
  RCL_SET_ERROR_MSG("rcl_arguments_t finalized twice");
  return RCL_RET_ERROR;
}

/// Parses a fully qualified namespace for a namespace replacement rule (ex: `/foo/bar`)
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_fully_qualified_namespace(
  rcl_lexer_lookahead2_t * lex_lookahead)
{
  rcl_ret_t ret;

  // Must have at least one Forward slash /
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_FORWARD_SLASH, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }

  // repeated tokens and slashes (allow trailing slash, but don't require it)
  while (true) {
    ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_TOKEN, NULL, NULL);
    if (RCL_RET_WRONG_LEXEME == ret) {
      rcl_reset_error();
      break;
    }
    ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_FORWARD_SLASH, NULL, NULL);
    if (RCL_RET_WRONG_LEXEME == ret) {
      rcl_reset_error();
      break;
    }
  }
  return RCL_RET_OK;
}

/// Parse either a token or a backreference (ex: `bar`, or `\7`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_replacement_token(rcl_lexer_lookahead2_t * lex_lookahead)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;

  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  if (
    RCL_LEXEME_BR1 == lexeme || RCL_LEXEME_BR2 == lexeme || RCL_LEXEME_BR3 == lexeme ||
    RCL_LEXEME_BR4 == lexeme || RCL_LEXEME_BR5 == lexeme || RCL_LEXEME_BR6 == lexeme ||
    RCL_LEXEME_BR7 == lexeme || RCL_LEXEME_BR8 == lexeme || RCL_LEXEME_BR9 == lexeme)
  {
    RCL_SET_ERROR_MSG("Backreferences are not implemented");
    return RCL_RET_ERROR;
  } else if (RCL_LEXEME_TOKEN == lexeme) {
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  } else {
    ret = RCL_RET_INVALID_REMAP_RULE;
  }

  return ret;
}

/// Parse the replacement side of a name remapping rule (ex: `bar/\1/foo`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_replacement_name(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;

  const char * replacement_start = rcl_lexer_lookahead2_get_text(lex_lookahead);
  if (NULL == replacement_start) {
    RCL_SET_ERROR_MSG("failed to get start of replacement");
    return RCL_RET_ERROR;
  }

  // private name (~/...) or fully qualified name (/...) ?
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  if (RCL_LEXEME_TILDE_SLASH == lexeme || RCL_LEXEME_FORWARD_SLASH == lexeme) {
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  }
  if (RCL_RET_OK != ret) {
    return ret;
  }

  // token ( '/' token )*
  ret = _rcl_parse_remap_replacement_token(lex_lookahead);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  while (RCL_LEXEME_EOF != lexeme) {
    ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_FORWARD_SLASH, NULL, NULL);
    if (RCL_RET_WRONG_LEXEME == ret) {
      return RCL_RET_INVALID_REMAP_RULE;
    }
    ret = _rcl_parse_remap_replacement_token(lex_lookahead);
    if (RCL_RET_OK != ret) {
      return ret;
    }
    ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  }

  // Copy replacement into rule
  const char * replacement_end = rcl_lexer_lookahead2_get_text(lex_lookahead);
  size_t length = (size_t)(replacement_end - replacement_start);
  rule->replacement = rcutils_strndup(replacement_start, length, rule->allocator);
  if (NULL == rule->replacement) {
    RCL_SET_ERROR_MSG("failed to copy replacement");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

/// Parse either a token or a wildcard (ex: `foobar`, or `*`, or `**`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_match_token(rcl_lexer_lookahead2_t * lex_lookahead)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;

  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  if (RCL_LEXEME_TOKEN == lexeme) {
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  } else if (RCL_LEXEME_WILD_ONE == lexeme) {
    RCL_SET_ERROR_MSG("Wildcard '*' is not implemented");
    return RCL_RET_ERROR;
  } else if (RCL_LEXEME_WILD_MULTI == lexeme) {
    RCL_SET_ERROR_MSG("Wildcard '**' is not implemented");
    return RCL_RET_ERROR;
  } else {
    RCL_SET_ERROR_MSG("Expecting token or wildcard");
    ret = RCL_RET_INVALID_REMAP_RULE;
  }

  return ret;
}

/// Parse the match side of a name remapping rule (ex: `rostopic://foo`)
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_match_name(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;
  // rostopic:// rosservice://
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  if (RCL_LEXEME_URL_SERVICE == lexeme) {
    rule->type = RCL_SERVICE_REMAP;
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  } else if (RCL_LEXEME_URL_TOPIC == lexeme) {
    rule->type = RCL_TOPIC_REMAP;
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  } else {
    rule->type = (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP);
  }
  if (RCL_RET_OK != ret) {
    return ret;
  }

  const char * match_start = rcl_lexer_lookahead2_get_text(lex_lookahead);
  if (NULL == match_start) {
    RCL_SET_ERROR_MSG("failed to get start of match");
    return RCL_RET_ERROR;
  }

  // private name (~/...) or fully qualified name (/...) ?
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  if (RCL_LEXEME_TILDE_SLASH == lexeme || RCL_LEXEME_FORWARD_SLASH == lexeme) {
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  }
  if (RCL_RET_OK != ret) {
    return ret;
  }

  // token ( '/' token )*
  ret = _rcl_parse_remap_match_token(lex_lookahead);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  while (RCL_LEXEME_SEPARATOR != lexeme) {
    ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_FORWARD_SLASH, NULL, NULL);
    if (RCL_RET_WRONG_LEXEME == ret) {
      return RCL_RET_INVALID_REMAP_RULE;
    }
    ret = _rcl_parse_remap_match_token(lex_lookahead);
    if (RCL_RET_OK != ret) {
      return ret;
    }
    ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  }

  // Copy match into rule
  const char * match_end = rcl_lexer_lookahead2_get_text(lex_lookahead);
  size_t length = (size_t)(match_end - match_start);
  rule->match = rcutils_strndup(match_start, length, rule->allocator);
  if (NULL == rule->match) {
    RCL_SET_ERROR_MSG("failed to copy match");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

/// Parse a name remapping rule (ex: `rostopic:///foo:=bar`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_name_remap(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  // match
  ret = _rcl_parse_remap_match_name(lex_lookahead, rule);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // :=
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_SEPARATOR, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  // replacement
  ret = _rcl_parse_remap_replacement_name(lex_lookahead, rule);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  return RCL_RET_OK;
}

/// Parse a namespace replacement rule (ex: `__ns:=/new/ns`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_namespace_replacement(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  // __ns
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_NS, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  // :=
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_SEPARATOR, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  // /foo/bar
  const char * ns_start = rcl_lexer_lookahead2_get_text(lex_lookahead);
  if (NULL == ns_start) {
    RCL_SET_ERROR_MSG("failed to get start of namespace");
    return RCL_RET_ERROR;
  }
  ret = _rcl_parse_remap_fully_qualified_namespace(lex_lookahead);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_INVALID_REMAP_RULE == ret) {
      // The name didn't start with a leading forward slash
      RCUTILS_LOG_WARN_NAMED(
        ROS_PACKAGE_NAME, "Namespace not remapped to a fully qualified name (found: %s)", ns_start);
    }
    return ret;
  }
  // There should be nothing left
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_EOF, NULL, NULL);
  if (RCL_RET_OK != ret) {
    // The name must have started with a leading forward slash but had an otherwise invalid format
    RCUTILS_LOG_WARN_NAMED(
      ROS_PACKAGE_NAME, "Namespace not remapped to a fully qualified name (found: %s)", ns_start);
    return ret;
  }

  // Copy namespace into rule
  const char * ns_end = rcl_lexer_lookahead2_get_text(lex_lookahead);
  size_t length = (size_t)(ns_end - ns_start);
  rule->replacement = rcutils_strndup(ns_start, length, rule->allocator);
  if (NULL == rule->replacement) {
    RCL_SET_ERROR_MSG("failed to copy namespace");
    return RCL_RET_BAD_ALLOC;
  }

  rule->type = RCL_NAMESPACE_REMAP;
  return RCL_RET_OK;
}

/// Parse a nodename replacement rule (ex: `__node:=new_name`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_nodename_replacement(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  const char * node_name;
  size_t length;

  // __node
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_NODE, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  // :=
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_SEPARATOR, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  // new_node_name
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_TOKEN, &node_name, &length);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // copy the node name into the replacement side of the rule
  rule->replacement = rcutils_strndup(node_name, length, rule->allocator);
  if (NULL == rule->replacement) {
    RCL_SET_ERROR_MSG("failed to allocate node name");
    return RCL_RET_BAD_ALLOC;
  }

  rule->type = RCL_NODENAME_REMAP;
  return RCL_RET_OK;
}

/// Parse a nodename prefix including trailing colon (ex: `node_name:`).
/**
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_nodename_prefix(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  const char * node_name;
  size_t length;

  // Expect a token and a colon
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_TOKEN, &node_name, &length);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_COLON, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }

  // copy the node name into the rule
  rule->node_name = rcutils_strndup(node_name, length, rule->allocator);
  if (NULL == rule->node_name) {
    RCL_SET_ERROR_MSG("failed to allocate node name");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

/// Start recursive descent parsing of a remap rule.
/**
 * \param[in] lex_lookahead a lookahead(2) buffer for the parser to use.
 * \param[in,out] rule input a zero intialized rule, output a fully initialized one.
 * \return RCL_RET_OK if a valid rule was parsed, or
 * \return RCL_RET_INVALID_REMAP_RULE if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_begin_remap_rule(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme1;
  rcl_lexeme_t lexeme2;

  // Check for optional nodename prefix
  ret = rcl_lexer_lookahead2_peek2(lex_lookahead, &lexeme1, &lexeme2);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  if (RCL_LEXEME_TOKEN == lexeme1 && RCL_LEXEME_COLON == lexeme2) {
    ret = _rcl_parse_remap_nodename_prefix(lex_lookahead, rule);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  }

  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme1);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  // What type of rule is this (node name replacement, namespace replacement, or name remap)?
  if (RCL_LEXEME_NODE == lexeme1) {
    ret = _rcl_parse_remap_nodename_replacement(lex_lookahead, rule);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  } else if (RCL_LEXEME_NS == lexeme1) {
    ret = _rcl_parse_remap_namespace_replacement(lex_lookahead, rule);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  } else {
    ret = _rcl_parse_remap_name_remap(lex_lookahead, rule);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  }

  // Make sure all characters in string have been consumed
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_EOF, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    return RCL_RET_INVALID_REMAP_RULE;
  }
  return ret;
}

rcl_ret_t
_rcl_parse_log_level_rule(
  const char * arg,
  rcl_allocator_t allocator,
  int * log_level)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(log_level, RCL_RET_INVALID_ARGUMENT);

  if (strncmp(RCL_LOG_LEVEL_ARG_RULE, arg, strlen(RCL_LOG_LEVEL_ARG_RULE)) != 0) {
    RCL_SET_ERROR_MSG("Argument does not start with '" RCL_LOG_LEVEL_ARG_RULE "'");
    return RCL_RET_INVALID_LOG_LEVEL_RULE;
  }
  rcutils_ret_t ret = rcutils_logging_severity_level_from_string(
    arg + strlen(RCL_LOG_LEVEL_ARG_RULE), allocator, log_level);
  if (RCUTILS_RET_OK == ret) {
    return RCL_RET_OK;
  }
  RCL_SET_ERROR_MSG("Argument does not use a valid severity level");
  return RCL_RET_INVALID_LOG_LEVEL_RULE;
}


rcl_ret_t
_rcl_parse_remap_rule(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_remap_t * output_rule)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_rule, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t ret;

  output_rule->allocator = allocator;
  rcl_lexer_lookahead2_t lex_lookahead = rcl_get_zero_initialized_lexer_lookahead2();

  ret = rcl_lexer_lookahead2_init(&lex_lookahead, arg, allocator);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _rcl_parse_remap_begin_remap_rule(&lex_lookahead, output_rule);

  if (RCL_RET_OK != ret) {
    // cleanup stuff, but return the original error code
    if (RCL_RET_OK != rcl_remap_fini(output_rule)) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to fini remap rule after error occurred");
    }
    if (RCL_RET_OK != rcl_lexer_lookahead2_fini(&lex_lookahead)) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to fini lookahead2 after error occurred");
    }
  } else {
    ret = rcl_lexer_lookahead2_fini(&lex_lookahead);
  }
  return ret;
}

rcl_ret_t
_rcl_parse_param_file_rule(
  const char * arg,
  rcl_allocator_t allocator,
  char ** param_file)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);

  const size_t param_prefix_len = strlen(RCL_PARAM_FILE_ARG_RULE);
  if (strncmp(RCL_PARAM_FILE_ARG_RULE, arg, param_prefix_len) == 0) {
    size_t outlen = strlen(arg) - param_prefix_len;
    *param_file = allocator.allocate(sizeof(char) * (outlen + 1), allocator.state);
    if (NULL == *param_file) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to allocate memory for parameters file path\n");
      return RCL_RET_BAD_ALLOC;
    }
    snprintf(*param_file, outlen + 1, "%s", arg + param_prefix_len);
    return RCL_RET_OK;
  }
  RCL_SET_ERROR_MSG("Argument does not start with '" RCL_PARAM_FILE_ARG_RULE "'");
  return RCL_RET_INVALID_PARAM_RULE;
}

#ifdef __cplusplus
}
#endif

/// \endcond  // Internal Doxygen documentation
