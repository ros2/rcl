// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#if __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rmw/rmw.h"

#include "rcl/types.h"
#include "./common.h"

// Extracted this portable method of doing a "shared library constructor" from SO:
//   http://stackoverflow.com/a/2390626/671658
// Initializer/finalizer sample for MSVC and GCC/Clang.
// 2010-2016 Joe Lowe. Released into the public domain.
#if defined(_MSC_VER)
  #pragma section(".CRT$XCU", read)
  #define INITIALIZER2_(f, p) \
  static void f(void); \
  __declspec(allocate(".CRT$XCU")) void (*f ## _)(void) = f; \
  __pragma(comment(linker, "/include:" p #f "_")) \
  static void f(void)
  #ifdef _WIN64
    #define INITIALIZER(f) INITIALIZER2_(f, "")
  #else
    #define INITIALIZER(f) INITIALIZER2_(f, "_")
  #endif
#else
  #define INITIALIZER(f) \
  static void f(void) __attribute__((constructor)); \
  static void f(void)
#endif

INITIALIZER(initialize) {
  // If the environment variable RMW_IMPLEMENTATION is set, or
  // the environment variable RCL_ASSERT_RMW_ID_MATCHES is set,
  // check that the result of `rmw_get_implementation_identifier` matches.
  rcl_allocator_t allocator = rcl_get_default_allocator();
  char * expected_rmw_impl = NULL;
  const char * expected_rmw_impl_env = NULL;
  rcl_ret_t ret = rcl_impl_getenv("RMW_IMPLEMENTATION", &expected_rmw_impl_env);
  if (ret != RCL_RET_OK) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME,
      "Error getting environement variable 'RMW_IMPLEMENTATION': %s",
      rcl_get_error_string_safe()
    )
    exit(ret);
  }
  if (strlen(expected_rmw_impl_env) > 0) {
    // Copy the environment variable so it doesn't get over-written by the next getenv call.
    expected_rmw_impl = (char *)allocator.allocate(
      strlen(expected_rmw_impl_env) + 1, allocator.state);
    if (!expected_rmw_impl) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "allocation failed")
      exit(RCL_RET_BAD_ALLOC);
    }
    memcpy(expected_rmw_impl, expected_rmw_impl_env, strlen(expected_rmw_impl_env) + 1);
  }

  char * asserted_rmw_impl = NULL;
  const char * asserted_rmw_impl_env = NULL;
  ret = rcl_impl_getenv("RCL_ASSERT_RMW_ID_MATCHES", &asserted_rmw_impl_env);
  if (ret != RCL_RET_OK) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME,
      "Error getting environement variable 'RCL_ASSERT_RMW_ID_MATCHES': %s",
      rcl_get_error_string_safe()
    )
    exit(ret);
  }
  if (strlen(asserted_rmw_impl_env) > 0) {
    // Copy the environment variable so it doesn't get over-written by the next getenv call.
    asserted_rmw_impl = (char *)allocator.allocate(
      strlen(asserted_rmw_impl_env) + 1, allocator.state);
    if (!asserted_rmw_impl) {
      RCUTILS_LOG_ERROR_NAMED(ROS_PACKAGE_NAME, "allocation failed")
      exit(RCL_RET_BAD_ALLOC);
    }
    memcpy(asserted_rmw_impl, asserted_rmw_impl_env, strlen(asserted_rmw_impl_env) + 1);
  }

  // If both environment variables are set, and they do not match, print a warning and exit.
  if (expected_rmw_impl && asserted_rmw_impl && strcmp(expected_rmw_impl, asserted_rmw_impl) != 0) {
    RCUTILS_LOG_ERROR_NAMED(
      ROS_PACKAGE_NAME,
      "Values of RMW_IMPLEMENTATION ('%s') and RCL_ASSERT_RMW_ID_MATCHES ('%s') environment "
      "variables do not match, exiting with %d.",
      expected_rmw_impl, asserted_rmw_impl, RCL_RET_ERROR
    )
    exit(RCL_RET_ERROR);
  }

  // Collapse the expected_rmw_impl and asserted_rmw_impl variables so only expected_rmw_impl needs
  // to be used from now on.
  if (expected_rmw_impl && asserted_rmw_impl) {
    // The strings at this point must be equal.
    // No need for asserted_rmw_impl anymore, free the memory.
    allocator.deallocate((char *)asserted_rmw_impl, allocator.state);
  } else {
    if (asserted_rmw_impl) {
      expected_rmw_impl = asserted_rmw_impl;
    }
  }

  // If either environment variable is set, and it does not match, print a warning and exit.
  if (expected_rmw_impl) {
    if(strcmp(rmw_get_implementation_identifier(), expected_rmw_impl) != 0) {
      RCUTILS_LOG_ERROR_NAMED(
        ROS_PACKAGE_NAME,
        "Expected RMW implementation identifier of '%s' but instead found '%s', exiting with %d.",
        expected_rmw_impl,
        rmw_get_implementation_identifier(),
        RCL_RET_MISMATCHED_RMW_ID
      )
      exit(RCL_RET_MISMATCHED_RMW_ID);
    }
    // Free the memory now that all checking has passed.
    allocator.deallocate((char *)expected_rmw_impl, allocator.state);
  }
}

#if __cplusplus
}
#endif
