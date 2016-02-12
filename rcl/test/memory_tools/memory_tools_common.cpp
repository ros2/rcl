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

#include <inttypes.h>

#include <atomic>
#include <cstdlib>

#if defined(__APPLE__)
#include <malloc/malloc.h>
#define MALLOC_PRINTF malloc_printf
#else  // defined(__APPLE__)
#define MALLOC_PRINTF printf
#endif  // defined(__APPLE__)

#include "./memory_tools.hpp"
#include "../scope_exit.hpp"

static std::atomic<bool> enabled(false);

static __thread bool malloc_expected = true;
static __thread UnexpectedCallbackType * unexpected_malloc_callback = nullptr;
void set_on_unexpected_malloc_callback(UnexpectedCallbackType callback)
{
  if (unexpected_malloc_callback) {
    unexpected_malloc_callback->~UnexpectedCallbackType();
    free(unexpected_malloc_callback);
    unexpected_malloc_callback = nullptr;
  }
  if (!callback) {
    return;
  }
  if (!unexpected_malloc_callback) {
    unexpected_malloc_callback =
      reinterpret_cast<UnexpectedCallbackType *>(malloc(sizeof(UnexpectedCallbackType)));
    if (!unexpected_malloc_callback) {
      throw std::bad_alloc();
    }
    new (unexpected_malloc_callback) UnexpectedCallbackType();
  }
  *unexpected_malloc_callback = callback;
}

void *
custom_malloc(size_t size)
{
  if (!enabled.load()) {return malloc(size);}
  auto foo = SCOPE_EXIT(enabled.store(true););
  enabled.store(false);
  if (!malloc_expected) {
    if (unexpected_malloc_callback) {
      (*unexpected_malloc_callback)();
    }
  }
  void * memory = malloc(size);
  uint64_t fw_size = size;
  if (!malloc_expected) {
    MALLOC_PRINTF(
      " malloc (%s) %p %" PRIu64 "\n",
      malloc_expected ? "    expected" : "not expected", memory, fw_size);
  }
  return memory;
}

static __thread bool realloc_expected = true;
static __thread UnexpectedCallbackType * unexpected_realloc_callback = nullptr;
void set_on_unexpected_realloc_callback(UnexpectedCallbackType callback)
{
  if (unexpected_realloc_callback) {
    unexpected_realloc_callback->~UnexpectedCallbackType();
    free(unexpected_realloc_callback);
    unexpected_realloc_callback = nullptr;
  }
  if (!callback) {
    return;
  }
  if (!unexpected_realloc_callback) {
    unexpected_realloc_callback =
      reinterpret_cast<UnexpectedCallbackType *>(malloc(sizeof(UnexpectedCallbackType)));
    if (!unexpected_realloc_callback) {
      throw std::bad_alloc();
    }
    new (unexpected_realloc_callback) UnexpectedCallbackType();
  }
  *unexpected_realloc_callback = callback;
}

void *
custom_realloc(void * memory_in, size_t size)
{
  if (!enabled.load()) {return realloc(memory_in, size);}
  auto foo = SCOPE_EXIT(enabled.store(true););
  enabled.store(false);
  if (!realloc_expected) {
    if (unexpected_realloc_callback) {
      (*unexpected_realloc_callback)();
    }
  }
  void * memory = realloc(memory_in, size);
  uint64_t fw_size = size;
  if (!realloc_expected) {
    MALLOC_PRINTF(
      "realloc (%s) %p %p %" PRIu64 "\n",
      realloc_expected ? "    expected" : "not expected", memory_in, memory, fw_size);
  }
  return memory;
}

static __thread bool free_expected = true;
static __thread UnexpectedCallbackType * unexpected_free_callback = nullptr;
void set_on_unexpected_free_callback(UnexpectedCallbackType callback)
{
  if (unexpected_free_callback) {
    unexpected_free_callback->~UnexpectedCallbackType();
    free(unexpected_free_callback);
    unexpected_free_callback = nullptr;
  }
  if (!callback) {
    return;
  }
  if (!unexpected_free_callback) {
    unexpected_free_callback =
      reinterpret_cast<UnexpectedCallbackType *>(malloc(sizeof(UnexpectedCallbackType)));
    if (!unexpected_free_callback) {
      throw std::bad_alloc();
    }
    new (unexpected_free_callback) UnexpectedCallbackType();
  }
  *unexpected_free_callback = callback;
}

void
custom_free(void * memory)
{
  if (!enabled.load()) {return free(memory);}
  auto foo = SCOPE_EXIT(enabled.store(true););
  enabled.store(false);
  if (!free_expected) {
    if (unexpected_free_callback) {
      (*unexpected_free_callback)();
    }
  }
  if (!free_expected) {
    MALLOC_PRINTF(
      "   free (%s) %p\n", free_expected ? "    expected" : "not expected", memory);
  }
  free(memory);
}

void assert_no_malloc_begin() {malloc_expected = false;}
void assert_no_malloc_end() {malloc_expected = true;}
void assert_no_realloc_begin() {realloc_expected = false;}
void assert_no_realloc_end() {realloc_expected = true;}
void assert_no_free_begin() {free_expected = false;}
void assert_no_free_end() {free_expected = true;}
