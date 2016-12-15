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

#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

/// Encapsulation of an allocator.
/**
 * The default allocator uses std::malloc(), std::free(), and std::realloc().
 * It can be obtained using rcl_get_default_allocator().
 *
 * The allocator should be trivially copyable.
 * Meaning that the struct should continue to work after being assignment
 * copied into a new struct.
 * Specifically the object pointed to by the state pointer should remain valid
 * until all uses of the allocator have been made.
 * Particular care should be taken when giving an allocator to rcl_init_* where
 * it is stored within another object and used later.
 */
typedef struct rcl_allocator_t
{
  /// Allocate memory, given a size and the `state` pointer.
  /** An error should be indicated by returning `NULL`. */
  void * (*allocate)(size_t size, void * state);
  /// Deallocate previously allocated memory, mimicking std::free().
  /** Also takes the `state` pointer. */
  void (* deallocate)(void * pointer, void * state);
  /// Reallocate if possible, otherwise it deallocates and allocates.
  /**
   * Also takes the `state` pointer.
   *
   * If unsupported then do deallocate and then allocate.
   * This should behave as std::realloc() does, as opposed to posix's
   * [reallocf](https://linux.die.net/man/3/reallocf), i.e. the memory given
   * by pointer will not be free'd automatically if std::realloc() fails.
   * For reallocf-like behavior use rcl_reallocf().
   * This function must be able to take an input pointer of `NULL` and succeed.
   */
  void * (*reallocate)(void * pointer, size_t size, void * state);
  /// Implementation defined state storage.
  /** This is passed as the final parameter to other allocator functions. */
  void * state;
} rcl_allocator_t;

/// Return a properly initialized rcl_allocator_t with default values.
/**
 * This defaults to:
 *
 * - allocate = wraps std::malloc()
 * - deallocate = wraps std::free()
 * - reallocate = wrapps std::realloc()
 * - state = `NULL`
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 */
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_allocator_t
rcl_get_default_allocator(void);

/// Emulate the behavior of [reallocf](https://linux.die.net/man/3/reallocf).
/**
 * This function will return `NULL` if the allocator is `NULL` or has `NULL` for
 * function pointer fields.
 */
RCL_PUBLIC
RCL_WARN_UNUSED
void *
rcl_reallocf(void * pointer, size_t size, rcl_allocator_t * allocator);

#if __cplusplus
}
#endif

#endif  // RCL__ALLOCATOR_H_
