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

#include <rmw/rmw.h>

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
  __declspec(allocate(".CRT$XCU")) void(*f ## _)(void) = f; \
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
  // If the environement variable RCL_ASSERT_RMW_ID_MATCHES is set,
  // check that the result of `rmw_get_implementation_identifier` matches.
  const char * expected = NULL;
  rcl_ret_t ret = rcl_impl_getenv("RCL_ASSERT_RMW_ID_MATCHES", &expected);
  if (ret != RCL_RET_OK) {
    fprintf(stderr,
      "Error getting environement variable 'RCL_ASSERT_RMW_ID_MATCHES': %s\n",
      rcl_get_error_string_safe()
    );
    exit(ret);
  }
  // If the environment variable is set, and it does not match, print a warning and exit.
  if (strlen(expected) > 0 && strcmp(rmw_get_implementation_identifier(), expected) != 0) {
    fprintf(stderr,
      "Expected RMW implementation identifier of '%s' but instead found '%s', exiting with %d.\n",
      expected,
      rmw_get_implementation_identifier(),
      RCL_RET_MISMATCHED_RMW_ID
    );
    exit(RCL_RET_MISMATCHED_RMW_ID);
  }
}

#if __cplusplus
}
#endif
