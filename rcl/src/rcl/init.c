// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/init.h"

#include "./arguments_impl.h"
#include "./common.h"
#include "./context_impl.h"
#include "./init_options_impl.h"
#include "rcl/arguments.h"
#include "rcl/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/stdatomic_helper.h"
#include "rmw/error_handling.h"

static atomic_uint_least64_t __rcl_next_unique_id = ATOMIC_VAR_INIT(1);

rcl_ret_t
rcl_init(
  int argc,
  char const * const * argv,
  const rcl_init_options_t * options,
  rcl_context_t * context)
{
  rcl_ret_t fail_ret = RCL_RET_ERROR;

  if (argc > 0) {
    RCL_CHECK_ARGUMENT_FOR_NULL(argv, RCL_RET_INVALID_ARGUMENT);
  } else {
    if (NULL != argv) {
      RCL_SET_ERROR_MSG("argc is <= 0, but argv is not NULL");
      return RCL_RET_INVALID_ARGUMENT;
    }
  }
  RCL_CHECK_ARGUMENT_FOR_NULL(options, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(options->impl, RCL_RET_INVALID_ARGUMENT);
  rcl_allocator_t allocator = options->impl->allocator;
  RCL_CHECK_ALLOCATOR(&allocator, return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(context, RCL_RET_INVALID_ARGUMENT);

  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME,
    "Initializing ROS client library, for context at address: %p", context);

  // test expectation that given context is zero initialized
  if (NULL != context->impl) {
    // note that this can also occur when the given context is used before initialization
    // i.e. it is declared on the stack but never defined or zero initialized
    RCL_SET_ERROR_MSG("rcl_init called on an already initialized context");
    return RCL_RET_ALREADY_INIT;
  }

  // Zero initialize global arguments.
  context->global_arguments = rcl_get_zero_initialized_arguments();

  // Setup impl for context.
  // use zero_allocate so the cleanup function will not try to clean up uninitialized parts later
  context->impl = allocator.zero_allocate(1, sizeof(rcl_context_impl_t), allocator.state);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    context->impl, "failed to allocate memory for context impl", return RCL_RET_BAD_ALLOC);

  // Copy the options into the context for future reference.
  rcl_ret_t ret = rcl_init_options_copy(options, &(context->impl->init_options));
  if (RCL_RET_OK != ret) {
    fail_ret = ret;  // error message already set
    goto fail;
  }

  // Copy the argc and argv into the context, if argc >= 0.
  context->impl->argc = argc;
  context->impl->argv = NULL;
  if (0 != argc && argv != NULL) {
    context->impl->argv = (char **)allocator.zero_allocate(argc, sizeof(char *), allocator.state);
    RCL_CHECK_FOR_NULL_WITH_MSG(
      context->impl->argv,
      "failed to allocate memory for argv",
      fail_ret = RCL_RET_BAD_ALLOC; goto fail);
    int64_t i;
    for (i = 0; i < argc; ++i) {
      size_t argv_i_length = strlen(argv[i]);
      context->impl->argv[i] = (char *)allocator.allocate(argv_i_length, allocator.state);
      RCL_CHECK_FOR_NULL_WITH_MSG(
        context->impl->argv[i],
        "failed to allocate memory for string entry in argv",
        fail_ret = RCL_RET_BAD_ALLOC; goto fail);
      memcpy(context->impl->argv[i], argv[i], argv_i_length);
    }
  }

  // Parse the ROS specific arguments.
  ret = rcl_parse_arguments(argc, argv, allocator, &context->global_arguments);
  if (RCL_RET_OK != ret) {
    fail_ret = ret;
    RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "Failed to parse global arguments");
    goto fail;
  }
  // Update the default log level if specified in arguments.
  if (context->global_arguments.impl->log_level >= 0) {
    rcutils_logging_set_default_logger_level(context->global_arguments.impl->log_level);
  }

  // Set the instance id.
  uint64_t next_instance_id = rcutils_atomic_fetch_add_uint64_t(&__rcl_next_unique_id, 1);
  if (0 == next_instance_id) {
    // Roll over occurred, this is an extremely unlikely occurrence.
    RCL_SET_ERROR_MSG("unique rcl instance ids exhausted");
    // Roll back to try to avoid the next call succeeding, but there's a data race here.
    rcutils_atomic_store(&__rcl_next_unique_id, -1);
    goto fail;
  }
  rcutils_atomic_store((atomic_uint_least64_t *)(&context->instance_id_storage), next_instance_id);
  context->impl->init_options.impl->rmw_init_options.instance_id = next_instance_id;

  // Initialize rmw_init.
  context->impl->rmw_context = rmw_get_zero_initialized_context();
  rmw_ret_t rmw_ret = rmw_init(
    &(context->impl->init_options.impl->rmw_init_options),
    &(context->impl->rmw_context));
  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    fail_ret = rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
    goto fail;
  }

  // Store the allocator.
  context->impl->allocator = allocator;

  return RCL_RET_OK;
fail:
  __cleanup_context(context);
  return fail_ret;
}

rcl_ret_t
rcl_shutdown(rcl_context_t * context)
{
  RCUTILS_LOG_DEBUG_NAMED(
    ROS_PACKAGE_NAME,
    "Shutting down ROS client library, for context at address: %p", context);
  RCL_CHECK_ARGUMENT_FOR_NULL(context, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    context->impl, "context is zero-initialized", return RCL_RET_INVALID_ARGUMENT);
  if (!rcl_context_is_valid(context)) {
    RCL_SET_ERROR_MSG("rcl_shutdown already called on the given context");
    return RCL_RET_ALREADY_SHUTDOWN;
  }

  // reset the instance id to 0 to indicate "invalid"
  rcutils_atomic_store((atomic_uint_least64_t *)(&context->instance_id_storage), 0);

  rmw_ret_t rmw_ret = rmw_shutdown(&(context->impl->rmw_context));
  if (RMW_RET_OK != rmw_ret) {
    RCL_SET_ERROR_MSG(rmw_get_error_string().str);
    return rcl_convert_rmw_ret_to_rcl_ret(rmw_ret);
  }

  return RCL_RET_OK;
}

#ifdef __cplusplus
}
#endif
