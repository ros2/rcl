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

#include <stdlib.h>

#include "rcl/allocator.h"
#include "rcl/error_handling.h"

static void *
__default_allocate(size_t size, void * state)
{
  (void)state;  // unused
  return malloc(size);
}

static void
__default_deallocate(void * pointer, void * state)
{
  (void)state;  // unused
  free(pointer);
}

static void *
__default_reallocate(void * pointer, size_t size, void * state)
{
  (void)state;  // unused
  return realloc(pointer, size);
}

rcl_allocator_t
rcl_get_default_allocator()
{
  static rcl_allocator_t default_allocator = {
    __default_allocate,
    __default_deallocate,
    __default_reallocate,
    NULL
  };
  return default_allocator;
}

void *
rcl_reallocf(void * pointer, size_t size, rcl_allocator_t * allocator)
{
  if (!allocator || !allocator->reallocate || !allocator->deallocate) {
    RCL_SET_ERROR_MSG("invalid allocator or allocator function pointers");
    return NULL;
  }
  void * new_pointer = allocator->reallocate(pointer, size, allocator->state);
  if (!new_pointer) {
    allocator->deallocate(pointer, allocator->state);
  }
  return new_pointer;
}
