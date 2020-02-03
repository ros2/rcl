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

#include <assert.h>
#include <string.h>

#include "./arguments_impl.h"
#include "./remap_impl.h"
#include "rcl/error_handling.h"
#include "rcl/lexer_lookahead.h"
#include "rcl/validate_topic_name.h"
#include "rcl_yaml_param_parser/parser.h"
#include "rcl_yaml_param_parser/types.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/format_string.h"
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

/// Parse an argument that may or may not be a param rule.
/**
 * \param[in] arg the argument to parse
 * \param[in,out] params param overrides structure to populate.
 *     This structure must have been initialized by the caller.
 * \return RCL_RET_OK if a valid rule was parsed, or
 * \return RCL_RET_INVALID_ARGUMENT if an argument is invalid, or
 * \return RCL_RET_INVALID_PARAM_RULE if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
rcl_ret_t
_rcl_parse_param_rule(
  const char * arg,
  rcl_params_t * params);

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

rcl_ret_t
rcl_arguments_get_param_overrides(
  const rcl_arguments_t * arguments,
  rcl_params_t ** parameter_overrides)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arguments, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(arguments->impl, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_overrides, RCL_RET_INVALID_ARGUMENT);

  if (NULL != *parameter_overrides) {
    RCL_SET_ERROR_MSG("Output parameter override pointer is not null. May leak memory.");
    return RCL_RET_INVALID_ARGUMENT;
  }
  *parameter_overrides = NULL;
  if (NULL != arguments->impl->parameter_overrides) {
    *parameter_overrides = rcl_yaml_node_struct_copy(arguments->impl->parameter_overrides);
    if (NULL == *parameter_overrides) {
      return RCL_RET_BAD_ALLOC;
    }
  }
  return RCL_RET_OK;
}

/// Parse an argument that may or may not be a log level rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] log_level parsed log level represented by `RCUTILS_LOG_SEVERITY` enum
 * \return RCL_RET_OK if a valid log level was parsed, or
 * \return RCL_RET_INVALID_LOG_LEVEL if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_log_level(
  const char * arg,
  rcl_allocator_t allocator,
  int * log_level);

/// Parse an argument that may or may not be a log level rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] log_level parsed log level represented by `RCUTILS_LOG_SEVERITY` enum
 * \return RCL_RET_OK if a valid log level was parsed, or
 * \return RCL_RET_INVALID_LOG_LEVEL_RULE if the argument is not a valid rule, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 * \deprecated to be removed in F-Turtle
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_log_level_rule(
  const char * arg,
  rcl_allocator_t allocator,
  int * log_level);

/// Parse an argument that may or may not be a log configuration file.
/**
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] log_config_file parsed log configuration file
 * \return RCL_RET_OK if a valid log config file was parsed, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_external_log_config_file(
  const char * arg,
  rcl_allocator_t allocator,
  char ** log_config_file);

/// Parse an argument that may or may not be a log file rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] log_config_file parsed log configuration file
 * \return RCL_RET_OK if a valid log config file was parsed, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 * \deprecated to be removed in F-Turtle
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_external_log_config_file_rule(
  const char * arg,
  rcl_allocator_t allocator,
  char ** log_config_file);

/// Parse an argument that may or may not be a parameter file.
/**
 * The syntax of the file name is not validated.
 * \param[in] arg the argument to parse
 * \param[in] allocator an allocator to use
 * \param[in,out] param_file string that could be a parameter file name
 * \return RCL_RET_OK if the rule was parsed correctly, or
 * \return RCL_RET_BAD_ALLOC if an allocation failed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_param_file(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_params_t * params,
  char ** param_file);

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
 * \deprecated to be removed in F-Turtle
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_param_file_rule(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_params_t * params,
  char ** param_file);

#define RCL_ENABLE_FLAG_PREFIX "--enable-"
#define RCL_DISABLE_FLAG_PREFIX "--disable-"

/// Parse a bool argument that may or may not be for the provided key rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] key the key for the argument to parse. Should be a null terminated string
 * \param[in,out] value parsed boolean value
 * \return RCL_RET_OK if the bool argument was parsed successfully, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_disabling_flag(
  const char * arg,
  const char * key,
  bool * value);

/// Parse a bool argument that may or may not be for the provided key rule.
/**
 * \param[in] arg the argument to parse
 * \param[in] key the key for the argument to parse. Should be a null terminated string
 * \param[in,out] value parsed boolean value
 * \return RCL_RET_OK if the bool argument was parsed successfully, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 * \deprecated to be removed in F-Turtle
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_bool_arg(
  const char * arg,
  const char * key,
  bool * value);

/// Parse a null terminated string to a boolean value.
/**
 * The case sensitive values: "T", "t", "True", "true", "Y", "y", "Yes", "yes",
 * and "1" will all map to true.
 * The case sensitive values: "F", "f", "False", "false", "N", "n", "No", "no",
 * and "0" will all map to false.
 *
 * \param[in] str a null terminated string to be parsed into a boolean
 * \param[in,out] val the boolean value parsed from the string.
 *   Left unchanged if string cannot be parsed to a valid bool.
 * \return RCL_RET_OK if a valid boolean parsed, or
 * \return RLC_RET_ERROR if an unspecified error occurred.
 * \deprecated to be removed in F-Turtle
 */
RCL_LOCAL
rcl_ret_t
_atob(
  const char * str,
  bool * val);

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
  args_impl->external_log_config_file = NULL;
  args_impl->unparsed_args = NULL;
  args_impl->num_unparsed_args = 0;
  args_impl->unparsed_ros_args = NULL;
  args_impl->num_unparsed_ros_args = 0;
  args_impl->parameter_overrides = NULL;
  args_impl->parameter_files = NULL;
  args_impl->num_param_files_args = 0;
  args_impl->log_stdout_disabled = false;
  args_impl->log_rosout_disabled = false;
  args_impl->log_ext_lib_disabled = false;
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

  args_impl->parameter_overrides = rcl_yaml_node_struct_init(allocator);
  if (NULL == args_impl->parameter_overrides) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }

  args_impl->parameter_files = allocator.allocate(sizeof(char *) * argc, allocator.state);
  if (NULL == args_impl->parameter_files) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }
  args_impl->unparsed_ros_args = allocator.allocate(sizeof(int) * argc, allocator.state);
  if (NULL == args_impl->unparsed_ros_args) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }
  args_impl->unparsed_args = allocator.allocate(sizeof(int) * argc, allocator.state);
  if (NULL == args_impl->unparsed_args) {
    ret = RCL_RET_BAD_ALLOC;
    goto fail;
  }

  bool parsing_ros_args = false;
  for (int i = 0; i < argc; ++i) {
    if (parsing_ros_args) {
      // Ignore ROS specific arguments flags
      if (strcmp(RCL_ROS_ARGS_FLAG, argv[i]) == 0) {
        continue;
      }

      // Check for ROS specific arguments explicit end token
      if (strcmp(RCL_ROS_ARGS_EXPLICIT_END_TOKEN, argv[i]) == 0) {
        parsing_ros_args = false;
        continue;
      }

      // Attempt to parse argument as parameter override flag
      if (strcmp(RCL_PARAM_FLAG, argv[i]) == 0 || strcmp(RCL_SHORT_PARAM_FLAG, argv[i]) == 0) {
        if (i + 1 < argc) {
          // Attempt to parse next argument as parameter override rule
          if (RCL_RET_OK == _rcl_parse_param_rule(argv[i + 1], args_impl->parameter_overrides)) {
            RCUTILS_LOG_DEBUG_NAMED(
              ROS_PACKAGE_NAME, "Got param override rule : %s\n", argv[i + 1]);
            ++i;  // Skip flag here, for loop will skip rule.
            continue;
          }
          rcl_error_string_t prev_error_string = rcl_get_error_string();
          rcl_reset_error();
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse parameter override rule: '%s %s'. Error: %s", argv[i], argv[i + 1],
            prev_error_string.str);
        } else {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse trailing %s flag. No parameter override rule found.", argv[i]);
        }
        ret = RCL_RET_INVALID_ROS_ARGS;
        goto fail;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME, "Arg %d (%s) is not a %s nor a %s flag.",
        i, argv[i], RCL_PARAM_FLAG, RCL_SHORT_PARAM_FLAG);

      // Attempt to parse argument as remap rule flag
      if (strcmp(RCL_REMAP_FLAG, argv[i]) == 0 || strcmp(RCL_SHORT_REMAP_FLAG, argv[i]) == 0) {
        if (i + 1 < argc) {
          // Attempt to parse next argument as remap rule
          rcl_remap_t * rule = &(args_impl->remap_rules[args_impl->num_remap_rules]);
          *rule = rcl_get_zero_initialized_remap();
          if (RCL_RET_OK == _rcl_parse_remap_rule(argv[i + 1], allocator, rule)) {
            ++(args_impl->num_remap_rules);
            RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Got remap rule : %s\n", argv[i + 1]);
            ++i;  // Skip flag here, for loop will skip rule.
            continue;
          }
          rcl_error_string_t prev_error_string = rcl_get_error_string();
          rcl_reset_error();
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse remap rule: '%s %s'. Error: %s", argv[i], argv[i + 1],
            prev_error_string.str);
        } else {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse trailing %s flag. No remap rule found.", argv[i]);
        }
        ret = RCL_RET_INVALID_ROS_ARGS;
        goto fail;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME, "Arg %d (%s) is not a %s nor a %s flag.",
        i, argv[i], RCL_REMAP_FLAG, RCL_SHORT_REMAP_FLAG);

      // Attempt to parse argument as parameter file rule
      if (strcmp(RCL_PARAM_FILE_FLAG, argv[i]) == 0) {
        if (i + 1 < argc) {
          // Attempt to parse next argument as parameter file rule
          args_impl->parameter_files[args_impl->num_param_files_args] = NULL;
          if (
            RCL_RET_OK == _rcl_parse_param_file(
              argv[i + 1], allocator, args_impl->parameter_overrides,
              &args_impl->parameter_files[args_impl->num_param_files_args]))
          {
            ++(args_impl->num_param_files_args);
            RCUTILS_LOG_DEBUG_NAMED(
              ROS_PACKAGE_NAME,
              "Got params file : %s\ntotal num param files %d",
              args_impl->parameter_files[args_impl->num_param_files_args - 1],
              args_impl->num_param_files_args);
            ++i;  // Skip flag here, for loop will skip rule.
            continue;
          }
          rcl_error_string_t prev_error_string = rcl_get_error_string();
          rcl_reset_error();
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse params file: '%s %s'. Error: %s", argv[i], argv[i + 1],
            prev_error_string.str);
        } else {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse trailing %s flag. No file path provided.", argv[i]);
        }
        ret = RCL_RET_INVALID_ROS_ARGS;
        goto fail;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME, "Arg %d (%s) is not a %s flag.",
        i, argv[i], RCL_PARAM_FILE_FLAG);

      // Attempt to parse argument as log level configuration
      if (strcmp(RCL_LOG_LEVEL_FLAG, argv[i]) == 0) {
        if (i + 1 < argc) {
          int log_level;
          if (RCL_RET_OK == _rcl_parse_log_level(argv[i + 1], allocator, &log_level)) {
            RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Got log level: %s\n", argv[i + 1]);
            args_impl->log_level = log_level;
            ++i;  // Skip flag here, for loop will skip value.
            continue;
          }
          rcl_error_string_t prev_error_string = rcl_get_error_string();
          rcl_reset_error();
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse log level: '%s %s'. Error: %s", argv[i], argv[i + 1],
            prev_error_string.str);
        } else {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse trailing log level flag: '%s'. No log level provided.", argv[i]);
        }
        ret = RCL_RET_INVALID_ROS_ARGS;
        goto fail;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME, "Arg %d (%s) is not a %s flag.",
        i, argv[i], RCL_LOG_LEVEL_FLAG);

      // Attempt to parse argument as log configuration file
      if (strcmp(RCL_EXTERNAL_LOG_CONFIG_FLAG, argv[i]) == 0) {
        if (i + 1 < argc) {
          if (NULL != args_impl->external_log_config_file) {
            RCUTILS_LOG_DEBUG_NAMED(
              ROS_PACKAGE_NAME, "Overriding log configuration file : %s\n",
              args_impl->external_log_config_file);
            allocator.deallocate(args_impl->external_log_config_file, allocator.state);
            args_impl->external_log_config_file = NULL;
          }
          if (RCL_RET_OK == _rcl_parse_external_log_config_file(
              argv[i + 1], allocator, &args_impl->external_log_config_file))
          {
            RCUTILS_LOG_DEBUG_NAMED(
              ROS_PACKAGE_NAME, "Got log configuration file : %s\n",
              args_impl->external_log_config_file);
            ++i;  // Skip flag here, for loop will skip value.
            continue;
          }
          rcl_error_string_t prev_error_string = rcl_get_error_string();
          rcl_reset_error();
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse log configuration file: '%s %s'. Error: %s", argv[i], argv[i + 1],
            prev_error_string.str);
        } else {
          RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Couldn't parse trailing %s flag. No file path provided.", argv[i]);
        }
        ret = RCL_RET_INVALID_ROS_ARGS;
        goto fail;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME, "Arg %d (%s) is not a %s flag.",
        i, argv[i], RCL_EXTERNAL_LOG_CONFIG_FLAG);

      // Attempt to parse --enable/disable-stdout-logs flag
      ret = _rcl_parse_disabling_flag(
        argv[i], RCL_LOG_STDOUT_FLAG_SUFFIX, &args_impl->log_stdout_disabled);
      if (RCL_RET_OK == ret) {
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Disable log stdout ? %s\n",
          args_impl->log_stdout_disabled ? "true" : "false");
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as %s%s or %s%s flag. Error: %s",
        i, argv[i], RCL_ENABLE_FLAG_PREFIX, RCL_LOG_STDOUT_FLAG_SUFFIX,
        RCL_DISABLE_FLAG_PREFIX, RCL_LOG_STDOUT_FLAG_SUFFIX, rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse --enable/disable-rosout-logs flag
      ret = _rcl_parse_disabling_flag(
        argv[i], RCL_LOG_ROSOUT_FLAG_SUFFIX, &args_impl->log_rosout_disabled);
      if (RCL_RET_OK == ret) {
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Disable log rosout ? %s\n",
          args_impl->log_rosout_disabled ? "true" : "false");
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as %s%s or %s%s flag. Error: %s",
        i, argv[i], RCL_ENABLE_FLAG_PREFIX, RCL_LOG_ROSOUT_FLAG_SUFFIX,
        RCL_DISABLE_FLAG_PREFIX, RCL_LOG_ROSOUT_FLAG_SUFFIX, rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse --enable/disable-external-lib-logs flag
      ret = _rcl_parse_disabling_flag(
        argv[i], RCL_LOG_EXT_LIB_FLAG_SUFFIX, &args_impl->log_ext_lib_disabled);
      if (RCL_RET_OK == ret) {
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Disable log external lib ? %s\n",
          args_impl->log_ext_lib_disabled ? "true" : "false");
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as %s%s or %s%s flag. Error: %s",
        i, argv[i], RCL_ENABLE_FLAG_PREFIX, RCL_LOG_EXT_LIB_FLAG_SUFFIX,
        RCL_DISABLE_FLAG_PREFIX, RCL_LOG_EXT_LIB_FLAG_SUFFIX, rcl_get_error_string().str);
      rcl_reset_error();

      // Argument is an unknown ROS specific argument
      args_impl->unparsed_ros_args[args_impl->num_unparsed_ros_args] = i;
      ++(args_impl->num_unparsed_ros_args);
    } else {
      // Check for ROS specific arguments flags
      if (strcmp(RCL_ROS_ARGS_FLAG, argv[i]) == 0) {
        parsing_ros_args = true;
        continue;
      }

      // Attempt to parse argument as remap rule in its deprecated form
      rcl_remap_t * rule = &(args_impl->remap_rules[args_impl->num_remap_rules]);
      *rule = rcl_get_zero_initialized_remap();
      if (RCL_RET_OK == _rcl_parse_remap_rule(argv[i], allocator, rule)) {
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found remap rule '%s'. This syntax is deprecated. Use '%s %s %s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, RCL_REMAP_FLAG, argv[i]);
        RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "Got remap rule : %s\n", argv[i + 1]);
        ++(args_impl->num_remap_rules);
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a remap rule in its deprecated form. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse argument as parameter file rule
      args_impl->parameter_files[args_impl->num_param_files_args] = NULL;
      if (
        RCL_RET_OK == _rcl_parse_param_file_rule(
          argv[i], allocator, args_impl->parameter_overrides,
          &args_impl->parameter_files[args_impl->num_param_files_args]))
      {
        ++(args_impl->num_param_files_args);
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found parameter file rule '%s'. This syntax is deprecated. Use '%s %s %s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, RCL_PARAM_FILE_FLAG,
          args_impl->parameter_files[args_impl->num_param_files_args - 1]);
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME,
          "params rule : %s\n total num param rules %d",
          args_impl->parameter_files[args_impl->num_param_files_args - 1],
          args_impl->num_param_files_args);
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a deprecated parameter file rule. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse argument as log level configuration
      int log_level;
      if (RCL_RET_OK == _rcl_parse_log_level_rule(argv[i], allocator, &log_level)) {
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found log level rule '%s'. This syntax is deprecated. Use '%s %s %s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, RCL_LOG_LEVEL_FLAG, g_rcutils_log_severity_names[log_level]);
        args_impl->log_level = log_level;
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a deprecated log level rule. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse argument as log configuration file rule
      rcl_ret_t ret = _rcl_parse_external_log_config_file_rule(
        argv[i], allocator, &args_impl->external_log_config_file);
      if (RCL_RET_OK == ret) {
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found log config rule '%s'. This syntax is deprecated. Use '%s %s %s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, RCL_EXTERNAL_LOG_CONFIG_FLAG,
          args_impl->external_log_config_file);
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Got log configuration file : %s\n",
          args_impl->external_log_config_file);
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a deprecated log config rule. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse argument as log_stdout_disabled
      ret = _rcl_parse_bool_arg(
        argv[i], RCL_LOG_DISABLE_STDOUT_ARG_RULE, &args_impl->log_stdout_disabled);
      if (RCL_RET_OK == ret) {
        const char * flag_prefix =
          args_impl->log_stdout_disabled ? RCL_DISABLE_FLAG_PREFIX : RCL_ENABLE_FLAG_PREFIX;
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found '%s'. This syntax is deprecated. Use '%s %s%s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, flag_prefix, RCL_LOG_STDOUT_FLAG_SUFFIX);
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Disable log stdout ? %s\n",
          args_impl->log_stdout_disabled ? "true" : "false");
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a deprecated log_stdout_disabled rule. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse argument as log_rosout_disabled
      ret = _rcl_parse_bool_arg(
        argv[i], RCL_LOG_DISABLE_ROSOUT_ARG_RULE, &args_impl->log_rosout_disabled);
      if (RCL_RET_OK == ret) {
        const char * flag_prefix =
          args_impl->log_rosout_disabled ? RCL_DISABLE_FLAG_PREFIX : RCL_ENABLE_FLAG_PREFIX;
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found '%s'. This syntax is deprecated. Use '%s %s%s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, flag_prefix, RCL_LOG_ROSOUT_FLAG_SUFFIX);
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Disable log rosout ? %s\n",
          args_impl->log_rosout_disabled ? "true" : "false");
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a deprecated log_rosout_disabled rule. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Attempt to parse argument as log_ext_lib_disabled
      ret = _rcl_parse_bool_arg(
        argv[i], RCL_LOG_DISABLE_EXT_LIB_ARG_RULE, &args_impl->log_ext_lib_disabled);
      if (RCL_RET_OK == ret) {
        const char * flag_prefix =
          args_impl->log_ext_lib_disabled ? RCL_DISABLE_FLAG_PREFIX : RCL_ENABLE_FLAG_PREFIX;
        RCUTILS_LOG_WARN_NAMED(
          ROS_PACKAGE_NAME,
          "Found '%s'. This syntax is deprecated. Use '%s %s%s' instead.",
          argv[i], RCL_ROS_ARGS_FLAG, flag_prefix, RCL_LOG_EXT_LIB_FLAG_SUFFIX);
        RCUTILS_LOG_DEBUG_NAMED(
          ROS_PACKAGE_NAME, "Disable log external lib ? %s\n",
          args_impl->log_ext_lib_disabled ? "true" : "false");
        continue;
      }
      RCUTILS_LOG_DEBUG_NAMED(
        ROS_PACKAGE_NAME,
        "Couldn't parse arg %d (%s) as a deprecated log_ext_lib_disabled rule. Error: %s",
        i, argv[i], rcl_get_error_string().str);
      rcl_reset_error();

      // Argument is not a ROS specific argument
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

  // Drop parameter overrides if none was found.
  if (0U == args_impl->parameter_overrides->num_nodes) {
    rcl_yaml_node_struct_fini(args_impl->parameter_overrides);
    args_impl->parameter_overrides = NULL;
  }

  // Shrink unparsed_ros_args
  if (0 == args_impl->num_unparsed_ros_args) {
    // No unparsed ros args
    allocator.deallocate(args_impl->unparsed_ros_args, allocator.state);
    args_impl->unparsed_ros_args = NULL;
  } else if (args_impl->num_unparsed_ros_args < argc) {
    args_impl->unparsed_ros_args = rcutils_reallocf(
      args_impl->unparsed_ros_args, sizeof(int) * args_impl->num_unparsed_ros_args, &allocator);
    if (NULL == args_impl->unparsed_ros_args) {
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

int
rcl_arguments_get_count_unparsed_ros(
  const rcl_arguments_t * args)
{
  if (NULL == args || NULL == args->impl) {
    return -1;
  }
  return args->impl->num_unparsed_ros_args;
}

rcl_ret_t
rcl_arguments_get_unparsed_ros(
  const rcl_arguments_t * args,
  rcl_allocator_t allocator,
  int ** output_unparsed_ros_indices)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(args->impl, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_unparsed_ros_indices, RCL_RET_INVALID_ARGUMENT);

  *output_unparsed_ros_indices = NULL;
  if (args->impl->num_unparsed_ros_args) {
    *output_unparsed_ros_indices = allocator.allocate(
      sizeof(int) * args->impl->num_unparsed_ros_args, allocator.state);
    if (NULL == *output_unparsed_ros_indices) {
      return RCL_RET_BAD_ALLOC;
    }
    for (int i = 0; i < args->impl->num_unparsed_ros_args; ++i) {
      (*output_unparsed_ros_indices)[i] = args->impl->unparsed_ros_args[i];
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
  RCL_CHECK_ARGUMENT_FOR_NULL(args, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(nonros_argc, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(nonros_argv, RCL_RET_INVALID_ARGUMENT);
  if (NULL != *nonros_argv) {
    RCL_SET_ERROR_MSG("Output nonros_argv pointer is not null. May leak memory.");
    return RCL_RET_INVALID_ARGUMENT;
  }

  *nonros_argc = rcl_arguments_get_count_unparsed(args);
  if (*nonros_argc < 0) {
    RCL_SET_ERROR_MSG("Failed to get unparsed non ROS specific arguments count.");
    return RCL_RET_INVALID_ARGUMENT;
  } else if (*nonros_argc > 0) {
    RCL_CHECK_ARGUMENT_FOR_NULL(argv, RCL_RET_INVALID_ARGUMENT);
  }

  *nonros_argv = NULL;
  if (0 == *nonros_argc) {
    return RCL_RET_OK;
  }

  int * unparsed_indices = NULL;
  rcl_ret_t ret = rcl_arguments_get_unparsed(args, allocator, &unparsed_indices);

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
  args_out->impl->remap_rules = NULL;
  args_out->impl->unparsed_args = NULL;
  args_out->impl->num_unparsed_args = 0;
  args_out->impl->unparsed_ros_args = NULL;
  args_out->impl->num_unparsed_ros_args = 0;
  args_out->impl->parameter_overrides = NULL;
  args_out->impl->parameter_files = NULL;
  args_out->impl->num_param_files_args = 0;

  if (args->impl->num_unparsed_args) {
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
  }

  if (args->impl->num_unparsed_ros_args) {
    // Copy unparsed ROS args
    args_out->impl->unparsed_ros_args = allocator.allocate(
      sizeof(int) * args->impl->num_unparsed_ros_args, allocator.state);
    if (NULL == args_out->impl->unparsed_ros_args) {
      if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
        RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
      }
      return RCL_RET_BAD_ALLOC;
    }
    for (int i = 0; i < args->impl->num_unparsed_ros_args; ++i) {
      args_out->impl->unparsed_ros_args[i] = args->impl->unparsed_ros_args[i];
    }
    args_out->impl->num_unparsed_ros_args = args->impl->num_unparsed_ros_args;
  }

  if (args->impl->num_remap_rules) {
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
      args_out->impl->remap_rules[i] = rcl_get_zero_initialized_remap();
      rcl_ret_t ret = rcl_remap_copy(
        &(args->impl->remap_rules[i]), &(args_out->impl->remap_rules[i]));
      if (RCL_RET_OK != ret) {
        if (RCL_RET_OK != rcl_arguments_fini(args_out)) {
          RCL_SET_ERROR_MSG("Error while finalizing arguments due to another error");
        }
        return ret;
      }
    }
  }

  // Copy parameter rules
  if (args->impl->parameter_overrides) {
    args_out->impl->parameter_overrides =
      rcl_yaml_node_struct_copy(args->impl->parameter_overrides);
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

    args->impl->allocator.deallocate(args->impl->unparsed_ros_args, args->impl->allocator.state);
    args->impl->num_unparsed_ros_args = 0;
    args->impl->unparsed_ros_args = NULL;

    if (args->impl->parameter_overrides) {
      rcl_yaml_node_struct_fini(args->impl->parameter_overrides);
      args->impl->parameter_overrides = NULL;
    }

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

  // Check arguments sanity
  assert(NULL != lex_lookahead);

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

  // Check arguments sanity
  assert(NULL != lex_lookahead);

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

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

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
  rule->impl->replacement = rcutils_strndup(
    replacement_start, length, rule->impl->allocator);
  if (NULL == rule->impl->replacement) {
    RCL_SET_ERROR_MSG("failed to copy replacement");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

/// Parse either a resource name token or a wildcard (ex: `foobar`, or `*`, or `**`).
/**
 * \sa _rcl_parse_resource_match()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_resource_match_token(rcl_lexer_lookahead2_t * lex_lookahead)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;

  // Check arguments sanity
  assert(NULL != lex_lookahead);

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
    ret = RCL_RET_WRONG_LEXEME;
  }

  return ret;
}

/// Parse a resource name match side of a rule (ex: `rostopic://foo`)
/**
 * \sa _rcl_parse_remap_match_name()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_resource_match(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_allocator_t allocator,
  char ** resource_match)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(rcutils_allocator_is_valid(&allocator));
  assert(NULL != resource_match);
  assert(NULL == *resource_match);

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
    if (RCL_RET_OK != ret) {
      return ret;
    }
  }

  // token ( '/' token )*
  ret = _rcl_parse_resource_match_token(lex_lookahead);
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
    ret = _rcl_parse_resource_match_token(lex_lookahead);
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
  const size_t length = (size_t)(match_end - match_start);
  *resource_match = rcutils_strndup(match_start, length, allocator);
  if (NULL == *resource_match) {
    RCL_SET_ERROR_MSG("failed to copy match");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

/// Parse a parameter name in a parameter override rule (ex: `foo.bar`)
/**
 * \sa _rcl_parse_param_rule()
 */
// TODO(hidmic): remove when parameter names are standardized to use slashes
//               in lieu of dots.
RCL_LOCAL
rcl_ret_t
_rcl_parse_param_name(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_allocator_t allocator,
  char ** param_name)
{
  rcl_ret_t ret;
  rcl_lexeme_t lexeme;

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(rcutils_allocator_is_valid(&allocator));
  assert(NULL != param_name);
  assert(NULL == *param_name);

  const char * name_start = rcl_lexer_lookahead2_get_text(lex_lookahead);
  if (NULL == name_start) {
    RCL_SET_ERROR_MSG("failed to get start of param name");
    return RCL_RET_ERROR;
  }

  // token ( '.' token )*
  ret = _rcl_parse_resource_match_token(lex_lookahead);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  while (RCL_LEXEME_SEPARATOR != lexeme) {
    ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_DOT, NULL, NULL);
    if (RCL_RET_WRONG_LEXEME == ret) {
      return RCL_RET_INVALID_REMAP_RULE;
    }
    ret = _rcl_parse_resource_match_token(lex_lookahead);
    if (RCL_RET_OK != ret) {
      return ret;
    }
    ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
    if (RCL_RET_OK != ret) {
      return ret;
    }
  }

  // Copy param name
  const char * name_end = rcl_lexer_lookahead2_get_text(lex_lookahead);
  const size_t length = (size_t)(name_end - name_start);
  *param_name = rcutils_strndup(name_start, length, allocator);
  if (NULL == *param_name) {
    RCL_SET_ERROR_MSG("failed to copy param name");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
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

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

  // rostopic:// rosservice://
  ret = rcl_lexer_lookahead2_peek(lex_lookahead, &lexeme);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  if (RCL_LEXEME_URL_SERVICE == lexeme) {
    rule->impl->type = RCL_SERVICE_REMAP;
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  } else if (RCL_LEXEME_URL_TOPIC == lexeme) {
    rule->impl->type = RCL_TOPIC_REMAP;
    ret = rcl_lexer_lookahead2_accept(lex_lookahead, NULL, NULL);
  } else {
    rule->impl->type = (RCL_TOPIC_REMAP | RCL_SERVICE_REMAP);
  }
  if (RCL_RET_OK != ret) {
    return ret;
  }

  ret = _rcl_parse_resource_match(
    lex_lookahead, rule->impl->allocator, &rule->impl->match);
  if (RCL_RET_WRONG_LEXEME == ret) {
    ret = RCL_RET_INVALID_REMAP_RULE;
  }
  return ret;
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

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

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

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

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
  rule->impl->replacement = rcutils_strndup(ns_start, length, rule->impl->allocator);
  if (NULL == rule->impl->replacement) {
    RCL_SET_ERROR_MSG("failed to copy namespace");
    return RCL_RET_BAD_ALLOC;
  }

  rule->impl->type = RCL_NAMESPACE_REMAP;
  return RCL_RET_OK;
}

/// Parse a nodename replacement rule (ex: `__node:=new_name` or `__name:=new_name`).
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

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

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
    node_name = rcl_lexer_lookahead2_get_text(lex_lookahead);
    RCUTILS_LOG_WARN_NAMED(
      ROS_PACKAGE_NAME, "Node name not remapped to invalid name: '%s'", node_name);
    return RCL_RET_INVALID_REMAP_RULE;
  }
  if (RCL_RET_OK != ret) {
    return ret;
  }
  // copy the node name into the replacement side of the rule
  rule->impl->replacement = rcutils_strndup(node_name, length, rule->impl->allocator);
  if (NULL == rule->impl->replacement) {
    RCL_SET_ERROR_MSG("failed to allocate node name");
    return RCL_RET_BAD_ALLOC;
  }

  rule->impl->type = RCL_NODENAME_REMAP;
  return RCL_RET_OK;
}

/// Parse a nodename prefix including trailing colon (ex: `node_name:`).
RCL_LOCAL
rcl_ret_t
_rcl_parse_nodename_prefix(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_allocator_t allocator,
  char ** node_name)
{
  size_t length = 0;
  const char * token = NULL;

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(rcutils_allocator_is_valid(&allocator));
  assert(NULL != node_name);
  assert(NULL == *node_name);

  // Expect a token and a colon
  rcl_ret_t ret =
    rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_TOKEN, &token, &length);
  if (RCL_RET_OK != ret) {
    return ret;
  }
  ret = rcl_lexer_lookahead2_expect(lex_lookahead, RCL_LEXEME_COLON, NULL, NULL);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  // Copy the node name
  *node_name = rcutils_strndup(token, length, allocator);
  if (NULL == *node_name) {
    RCL_SET_ERROR_MSG("failed to allocate node name");
    return RCL_RET_BAD_ALLOC;
  }

  return RCL_RET_OK;
}

/// Parse a nodename prefix for a remap rule.
/**
 * \sa _rcl_parse_nodename_prefix()
 * \sa _rcl_parse_remap_begin_remap_rule()
 */
RCL_LOCAL
rcl_ret_t
_rcl_parse_remap_nodename_prefix(
  rcl_lexer_lookahead2_t * lex_lookahead,
  rcl_remap_t * rule)
{
  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

  rcl_ret_t ret = _rcl_parse_nodename_prefix(
    lex_lookahead, rule->impl->allocator, &rule->impl->node_name);
  if (RCL_RET_WRONG_LEXEME == ret) {
    ret = RCL_RET_INVALID_REMAP_RULE;
  }
  return ret;
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

  // Check arguments sanity
  assert(NULL != lex_lookahead);
  assert(NULL != rule);

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
_rcl_parse_log_level(
  const char * arg,
  rcl_allocator_t allocator,
  int * log_level)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(log_level, RCL_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = rcutils_logging_severity_level_from_string(arg, allocator, log_level);
  if (RCUTILS_RET_OK == ret) {
    return RCL_RET_OK;
  }
  RCL_SET_ERROR_MSG("Argument does not use a valid severity level");
  return RCL_RET_ERROR;
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

  output_rule->impl = allocator.allocate(sizeof(rcl_remap_impl_t), allocator.state);
  if (NULL == output_rule->impl) {
    return RCL_RET_BAD_ALLOC;
  }
  output_rule->impl->allocator = allocator;
  output_rule->impl->type = RCL_UNKNOWN_REMAP;
  output_rule->impl->node_name = NULL;
  output_rule->impl->match = NULL;
  output_rule->impl->replacement = NULL;

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
_rcl_parse_param_rule(
  const char * arg,
  rcl_params_t * params)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(params, RCL_RET_INVALID_ARGUMENT);

  rcl_lexer_lookahead2_t lex_lookahead = rcl_get_zero_initialized_lexer_lookahead2();

  rcl_ret_t ret = rcl_lexer_lookahead2_init(&lex_lookahead, arg, params->allocator);
  if (RCL_RET_OK != ret) {
    return ret;
  }

  rcl_lexeme_t lexeme1;
  rcl_lexeme_t lexeme2;
  char * node_name = NULL;
  char * param_name = NULL;

  // Check for optional nodename prefix
  ret = rcl_lexer_lookahead2_peek2(&lex_lookahead, &lexeme1, &lexeme2);
  if (RCL_RET_OK != ret) {
    goto cleanup;
  }

  if (RCL_LEXEME_TOKEN == lexeme1 && RCL_LEXEME_COLON == lexeme2) {
    ret = _rcl_parse_nodename_prefix(&lex_lookahead, params->allocator, &node_name);
    if (RCL_RET_OK != ret) {
      if (RCL_RET_WRONG_LEXEME == ret) {
        ret = RCL_RET_INVALID_PARAM_RULE;
      }
      goto cleanup;
    }
  } else {
    node_name = rcutils_strdup("/**", params->allocator);
    if (NULL == node_name) {
      ret = RCL_RET_BAD_ALLOC;
      goto cleanup;
    }
  }

  // TODO(hidmic): switch to _rcl_parse_resource_match() when parameter names
  //               are standardized to use slashes in lieu of dots.
  ret = _rcl_parse_param_name(&lex_lookahead, params->allocator, &param_name);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_WRONG_LEXEME == ret) {
      ret = RCL_RET_INVALID_PARAM_RULE;
    }
    goto cleanup;
  }

  ret = rcl_lexer_lookahead2_expect(&lex_lookahead, RCL_LEXEME_SEPARATOR, NULL, NULL);
  if (RCL_RET_WRONG_LEXEME == ret) {
    ret = RCL_RET_INVALID_PARAM_RULE;
    goto cleanup;
  }

  const char * yaml_value = rcl_lexer_lookahead2_get_text(&lex_lookahead);
  if (!rcl_parse_yaml_value(node_name, param_name, yaml_value, params)) {
    ret = RCL_RET_INVALID_PARAM_RULE;
    goto cleanup;
  }

cleanup:
  params->allocator.deallocate(param_name, params->allocator.state);
  params->allocator.deallocate(node_name, params->allocator.state);
  if (RCL_RET_OK != ret) {
    if (RCL_RET_OK != rcl_lexer_lookahead2_fini(&lex_lookahead)) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to fini lookahead2 after error occurred");
    }
  } else {
    ret = rcl_lexer_lookahead2_fini(&lex_lookahead);
  }
  return ret;
}

rcl_ret_t
_rcl_parse_param_file(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_params_t * params,
  char ** param_file)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(params, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(param_file, RCL_RET_INVALID_ARGUMENT);
  *param_file = rcutils_strdup(arg, allocator);
  if (NULL == *param_file) {
    RCL_SET_ERROR_MSG("Failed to allocate memory for parameters file path");
    return RCL_RET_BAD_ALLOC;
  }
  if (!rcl_parse_yaml_file(*param_file, params)) {
    // Error message already set.
    return RCL_RET_ERROR;
  }
  return RCL_RET_OK;
}

rcl_ret_t
_rcl_parse_param_file_rule(
  const char * arg,
  rcl_allocator_t allocator,
  rcl_params_t * params,
  char ** param_file)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(params, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(param_file, RCL_RET_INVALID_ARGUMENT);

  const size_t param_prefix_len = strlen(RCL_PARAM_FILE_ARG_RULE);
  if (strncmp(RCL_PARAM_FILE_ARG_RULE, arg, param_prefix_len) == 0) {
    size_t outlen = strlen(arg) - param_prefix_len;
    *param_file = allocator.allocate(sizeof(char) * (outlen + 1), allocator.state);
    if (NULL == *param_file) {
      RCL_SET_ERROR_MSG("Failed to allocate memory for parameters file path");
      return RCL_RET_BAD_ALLOC;
    }
    snprintf(*param_file, outlen + 1, "%s", arg + param_prefix_len);
    if (!rcl_parse_yaml_file(*param_file, params)) {
      // Error message already set.
      return RCL_RET_ERROR;
    }
    return RCL_RET_OK;
  }
  RCL_SET_ERROR_MSG("Argument does not start with '" RCL_PARAM_FILE_ARG_RULE "'");
  return RCL_RET_INVALID_PARAM_RULE;
}

rcl_ret_t
_rcl_parse_external_log_config_file(
  const char * arg,
  rcl_allocator_t allocator,
  char ** log_config_file)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(log_config_file, RCL_RET_INVALID_ARGUMENT);

  *log_config_file = rcutils_strdup(arg, allocator);
  // TODO(hidmic): add file checks
  if (NULL == *log_config_file) {
    RCL_SET_ERROR_MSG("Failed to allocate memory for external log config file");
    return RCL_RET_BAD_ALLOC;
  }
  return RCL_RET_OK;
}

rcl_ret_t
_rcl_parse_external_log_config_file_rule(
  const char * arg,
  rcl_allocator_t allocator,
  char ** log_config_file)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(log_config_file, RCL_RET_INVALID_ARGUMENT);

  const size_t param_prefix_len = sizeof(RCL_EXTERNAL_LOG_CONFIG_ARG_RULE) - 1;
  if (strncmp(RCL_EXTERNAL_LOG_CONFIG_ARG_RULE, arg, param_prefix_len) == 0) {
    size_t outlen = strlen(arg) - param_prefix_len;
    *log_config_file = rcutils_format_string_limit(allocator, outlen, "%s", arg + param_prefix_len);
    if (NULL == *log_config_file) {
      RCL_SET_ERROR_MSG("Failed to allocate memory for external log config file");
      return RCL_RET_BAD_ALLOC;
    }
    return RCL_RET_OK;
  }

  RCL_SET_ERROR_MSG("Argument does not start with '" RCL_EXTERNAL_LOG_CONFIG_ARG_RULE "'");
  return RCL_RET_INVALID_PARAM_RULE;
}

rcl_ret_t
_rcl_parse_disabling_flag(
  const char * arg,
  const char * suffix,
  bool * disable)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(suffix, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(disable, RCL_RET_INVALID_ARGUMENT);

  const size_t enable_prefix_len = strlen(RCL_ENABLE_FLAG_PREFIX);
  if (
    strncmp(RCL_ENABLE_FLAG_PREFIX, arg, enable_prefix_len) == 0 &&
    strcmp(suffix, arg + enable_prefix_len) == 0)
  {
    *disable = false;
    return RCL_RET_OK;
  }

  const size_t disable_prefix_len = strlen(RCL_DISABLE_FLAG_PREFIX);
  if (
    strncmp(RCL_DISABLE_FLAG_PREFIX, arg, disable_prefix_len) == 0 &&
    strcmp(suffix, arg + disable_prefix_len) == 0)
  {
    *disable = true;
    return RCL_RET_OK;
  }

  RCL_SET_ERROR_MSG_WITH_FORMAT_STRING(
    "Argument is not a %s%s nor a %s%s flag.",
    RCL_ENABLE_FLAG_PREFIX, suffix,
    RCL_DISABLE_FLAG_PREFIX, suffix);
  return RCL_RET_ERROR;
}

rcl_ret_t
_rcl_parse_bool_arg(
  const char * arg,
  const char * key,
  bool * value)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(arg, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(key, RCL_RET_INVALID_ARGUMENT);

  const size_t param_prefix_len = strlen(key);
  if (strncmp(key, arg, param_prefix_len) == 0) {
    return _atob(arg + param_prefix_len, value);
  }

  RCL_SET_ERROR_MSG("Argument does not start with key");
  return RCL_RET_INVALID_PARAM_RULE;
}

rcl_ret_t
_atob(
  const char * str,
  bool * val)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(str, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(val, RCL_RET_INVALID_ARGUMENT);
  const char * true_values[] = {"y", "Y", "yes", "Yes", "t", "T", "true", "True", "1"};
  const char * false_values[] = {"n", "N", "no", "No", "f", "F", "false", "False", "0"};

  for (size_t idx = 0; idx < sizeof(true_values) / sizeof(char *); idx++) {
    if (0 == strncmp(true_values[idx], str, strlen(true_values[idx]))) {
      *val = true;
      return RCL_RET_OK;
    }
  }

  for (size_t idx = 0; idx < sizeof(false_values) / sizeof(char *); idx++) {
    if (0 == strncmp(false_values[idx], str, strlen(false_values[idx]))) {
      *val = false;
      return RCL_RET_OK;
    }
  }
  return RCL_RET_ERROR;
}

#ifdef __cplusplus
}
#endif

/// \endcond  // Internal Doxygen documentation
