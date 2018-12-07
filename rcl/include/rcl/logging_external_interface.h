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

#ifndef RCL__LOGGING_EXTERNAL_INTERFACE_H_
#define RCL__LOGGING_EXTERNAL_INTERFACE_H_

#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

/// Initialize the external logging library.
/**
 * \param[in] config_file The location of a config file that the external
 *   logging library should use to configure itself.
 *   If no config file is provided this will be set to an empty string.
 *   Must be a NULL terminated c string.
 * \return RCL_RET_OK if initialized successfully, or
 * \return RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_logging_external_initialize(const char * config_file);

/// Free the resources allocated for the external logging system.
/**
 * This puts the system into a state equivalent to being uninitialized.
 *
 * \return RCL_RET_OK if successfully shutdown, or
 * \return RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_logging_external_shutdown();

/// Log a message.
/**
 * \param[in] severity The severity level of the message being logged.
 * \param[in] name The name of the logger, must either be a null terminated
 *   c string or NULL.
 *   If NULL or empty the root logger will be used.
 * \param[in] msg The message to be logged. Must be a null terminated c string.
 */
RCL_PUBLIC
void
rcl_logging_external_log(int severity, const char * name, const char * msg);

/// Set the severity level for a logger.
/**
 * This function sets the severity level for the specified logger.
 * If the name provided is an empty string or NULL it will change the level of
 * the root logger.
 *
 * \param[in] name The name of the logger.
 *   Must be a NULL terminated c string or NULL.
 * \param[in] level The severity level to be used for the specified logger.
 * \return RCL_RET_OK if set successfully, or
 * \return RCL_RET_ERROR if an unspecified error occurs.
 */
RCL_PUBLIC
rcl_ret_t rcl_logging_external_set_logger_level(const char * name, int level);

#endif  // RCL__LOGGING_EXTERNAL_INTERFACE_H_
