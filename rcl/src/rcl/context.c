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

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcl/context.h"

#include <stdbool.h>

#include "./context_impl.h"
#include "rcutils/stdatomic_helper.h"

rcl_context_t
rcl_get_zero_initialized_context(void)
{
  static rcl_context_t context = {
    .impl = NULL,
    .instance_id_storage = {0},
  };
  // this is not constexpr so it cannot be in the struct initialization
  context.global_arguments = rcl_get_zero_initialized_arguments();
  // ensure assumption about static storage
  static_assert(
    sizeof(context.instance_id_storage) >= sizeof(atomic_uint_least64_t),
    "expected rcl_context_t's instance id storage to be >= size of atomic_uint_least64_t");
  // initialize atomic
  atomic_init((atomic_uint_least64_t *)(&context.instance_id_storage), 0);
  return context;
}

// See `rcl_init()` for initialization of the context.

rcl_ret_t
rcl_context_fini(rcl_context_t * context)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(context, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_FOR_NULL_WITH_MSG(
    context->impl, "context is zero-initialized", return RCL_RET_INVALID_ARGUMENT);
  if (rcl_context_is_valid(context)) {
    RCL_SET_ERROR_MSG("rcl_shutdown() not called on the given context");
    return RCL_RET_INVALID_ARGUMENT;
  }
  RCL_CHECK_ALLOCATOR_WITH_MSG(
    &(context->impl->allocator), "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  __cleanup_context(context);
  return RCL_RET_OK;
}

// See `rcl_shutdown()` for invalidation of the context.

const rcl_init_options_t *
rcl_context_get_init_options(rcl_context_t * context)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(context, NULL);
  RCL_CHECK_FOR_NULL_WITH_MSG(context->impl, "context is zero-initialized", return NULL);
  return &(context->impl->init_options);
}

rcl_context_instance_id_t
rcl_context_get_instance_id(rcl_context_t * context)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(context, 0);
  return rcutils_atomic_load_uint64_t((atomic_uint_least64_t *)(&context->instance_id_storage));
}

bool
rcl_context_is_valid(rcl_context_t * context)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(context, false);
  return 0 != rcl_context_get_instance_id(context);
}

void
__cleanup_context(rcl_context_t * context)
{
  // if null, nothing can be done
  if (NULL == context) {
    return;
  }

  // reset the instance id to 0 to indicate "invalid" (should already be 0, but this is defensive)
  rcutils_atomic_store((atomic_uint_least64_t *)(&context->instance_id_storage), 0);

  // clean up global_arguments if initialized
  if (NULL != context->global_arguments.impl) {
    rcl_ret_t ret = rcl_arguments_fini(&(context->global_arguments));
    if (RCL_RET_OK != ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
        "[rcl|init.c:" RCUTILS_STRINGIFY(__LINE__)
        "] failed to finalize global arguments while cleaning up context, memory may be leaked: ");
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
      rcl_reset_error();
    }
  }

  // if impl is null, nothing else can be cleaned up
  if (NULL != context->impl) {
    // pull allocator out for use during deallocation
    rcl_allocator_t allocator = context->impl->allocator;

    // finalize init options if valid
    if (NULL != context->impl->init_options.impl) {
      rcl_ret_t ret = rcl_init_options_fini(&(context->impl->init_options));
      if (RCL_RET_OK != ret) {
        RCUTILS_SAFE_FWRITE_TO_STDERR(
          "[rcl|init.c:" RCUTILS_STRINGIFY(__LINE__)
          "] failed to finalize init options while cleaning up context, memory may be leaked: ");
        RCUTILS_SAFE_FWRITE_TO_STDERR(rcl_get_error_string().str);
        RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
        rcl_reset_error();
      }
    }

    // clean up copy of argv if valid
    if (NULL != context->impl->argv) {
      int64_t i;
      for (i = 0; i < context->impl->argc; ++i) {
        if (NULL != context->impl->argv[i]) {
          allocator.deallocate(context->impl->argv[i], allocator.state);
        }
      }
      allocator.deallocate(context->impl->argv, allocator.state);
    }
  }  // if (NULL != context->impl)

  // zero-initialize the context
  *context = rcl_get_zero_initialized_context();
}

#ifdef __cplusplus
}
#endif
