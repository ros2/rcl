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

#ifndef RCL__ALLOCATOR_H_
#define RCL__ALLOCATOR_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/types.h"

/// Encapsulation of an allocator.
/* To use malloc, free, and realloc use rcl_get_default_allocator(). */
typedef struct rcl_allocator_t
{
  /// Allocate memory, given a size and state structure.
  /* An error should be indicated by returning NULL. */
  void * (*allocate)(size_t size, void * state);
  /// Deallocate previously allocated memory, mimicking free().
  void (* deallocate)(void * pointer, void * state);
  /// Reallocates if possible, otherwise it deallocates and allocates.
  /* If unsupported then do deallocate and then allocate.
   * \TODO(wjwwood): should this behave as reallocf?
   * This should behave as realloc is described, as opposed to reallocf, i.e.
   * the memory given by pointer will not be free'd automatically if realloc
   * fails.
   * This function must be able to take an input pointer of NULL and succeed.
   */
  void * (*reallocate)(void * pointer, size_t size, void * state);
  /// Implementation defined state storage.
  /* This is passed as the second parameter to other allocator functions. */
  void * state;
} rcl_allocator_t;

/// Return a properly initialized rcl_allocator_t with default values.
/* This function does not allocate heap memory.
 * This function is thread-safe.
 * This function is lock-free.
 */
rcl_allocator_t
rcl_get_default_allocator();

#if __cplusplus
}
#endif

#endif  // RCL__ALLOCATOR_H_
