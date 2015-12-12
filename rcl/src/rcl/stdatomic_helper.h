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

#ifndef RCL__STDATOMIC_HELPER_H_
#define RCL__STDATOMIC_HELPER_H_

#if !defined(WIN32)

#include <stdatomic.h>

#define rcl_atomic_load(object, out) (out) = atomic_load(object)

#define rcl_atomic_compare_exchange_strong(object, out, expected, desired) \
  (out) = atomic_compare_exchange_strong(object, expected, desired)

#define rcl_atomic_exchange(object, out, desired) (out) = atomic_exchange(object, desired)

#define rcl_atomic_store(object, desired) atomic_store(object, desired)

#else  // !defined(WIN32)

#include "./stdatomic_helper/win32/stdatomic.h"

#define rcl_atomic_load(object, out) rcl_win32_atomic_load(object, out)

#define rcl_atomic_compare_exchange_strong(object, out, expected, desired) \
  rcl_win32_atomic_compare_exchange_strong(object, out, expected, desired)

#define rcl_atomic_exchange(object, out, desired) rcl_win32_atomic_exchange(object, out, desired)

#define rcl_atomic_store(object, desired) rcl_win32_atomic_store(object, desired)

#endif  // !defined(WIN32)

static inline bool
rcl_atomic_load_bool(atomic_bool * a_bool)
{
  bool result = false;
  rcl_atomic_load(a_bool, result);
  return result;
}

static inline uint64_t
rcl_atomic_load_uint64_t(atomic_uint_least64_t * a_uint64_t)
{
  uint64_t result = 0;
  rcl_atomic_load(a_uint64_t, result);
  return result;
}

static inline uintptr_t
rcl_atomic_load_uintptr_t(atomic_uintptr_t * a_uintptr_t)
{
  uintptr_t result = 0;
  rcl_atomic_load(a_uintptr_t, result);
  return result;
}

static inline bool
rcl_atomic_compare_exchange_strong_uint_least64_t(
  atomic_uint_least64_t * a_uint_least64_t, uint64_t * expected, uint64_t desired)
{
  bool result;
  rcl_atomic_compare_exchange_strong(a_uint_least64_t, result, expected, desired);
  return result;
}

static inline bool
rcl_atomic_exchange_bool(atomic_bool * a_bool, bool desired)
{
  bool result;
  rcl_atomic_exchange(a_bool, result, desired);
  return result;
}

static inline uint64_t
rcl_atomic_exchange_uint64_t(atomic_uint_least64_t * a_uint64_t, uint64_t desired)
{
  uint64_t result;
  rcl_atomic_exchange(a_uint64_t, result, desired);
  return result;
}

static inline uint64_t
rcl_atomic_exchange_uintptr_t(atomic_uintptr_t * a_uintptr_t, uintptr_t desired)
{
  uintptr_t result;
  rcl_atomic_exchange(a_uintptr_t, result, desired);
  return result;
}

#endif  // RCL__STDATOMIC_HELPER_H_
