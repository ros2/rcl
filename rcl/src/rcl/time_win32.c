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

#ifdef WIN32
#ifndef WIN32
#error time_win32.c is only intended to be used with win32 based systems
#endif

#if __cplusplus
extern "C"
{
#endif

#include "rcl/time.h"

#include <stdatomic.h>
#include <windows.h>

#include "./common.h"
#include "rcl/error_handling.h"

#define __WOULD_BE_NEGATIVE(seconds, subseconds) (seconds < 0 || (subseconds < 0 && seconds == 0))

rcl_ret_t
rcl_system_time_point_now(rcl_system_time_point_t * now)
{

}
#if 0
{
  RCL_CHECK_ARGUMENT_FOR_NULL(now, RCL_RET_INVALID_ARGUMENT);
  /* Windows implementation adapted from roscpp_core (3-clause BSD), see:
   *   https://github.com/ros/roscpp_core/blob/0.5.6/rostime/src/time.cpp#L96
   *
   * > Win32 implementation
   *   unless I've missed something obvious, the only way to get high-precision
   *   time on Windows is via the QueryPerformanceCounter() call. However,
   *   this is somewhat problematic in Windows XP on some processors, especially
   *   AMD, because the Windows implementation can freak out when the CPU clocks
   *   down to save power. Time can jump or even go backwards. Microsoft has
   *   fixed this bug for most systems now, but it can still show up if you have
   *   not installed the latest CPU drivers (an oxymoron). They fixed all these
   *   problems in Windows Vista, and this API is by far the most accurate that
   *   I know of in Windows, so I'll use it here despite all these caveats
   *
   * I've further modified it to be thread safe using a atomic_uint_least64_t.
   */
  LARGE_INTEGER cpu_freq;
  static LARGE_INTEGER cpu_freq_global;
  LARGE_INTEGER init_cpu_time;
  static LARGE_INTEGER init_cpu_time_global;
  static atomic_uint_least64_t start_ns = ATOMIC_VAR_INIT(0);
  rcl_time_t start = {0, 0};
  // If start_ns (static/global) is 0, then set it up on the first call.
  uint64_t start_ns_loaded = atomic_load(&start_ns);
  if (start_ns_loaded == 0) {
    QueryPerformanceFrequency(&cpu_freq);
    if (cpu_freq.QuadPart == 0) {
      RCL_SET_ERROR_MSG("no high performance timer found");
      return RCL_RET_ERROR;
    }
    QueryPerformanceCounter(&init_cpu_time);
    // compute an offset from the Epoch using the lower-performance timer API
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    LARGE_INTEGER start_li;
    start_li.LowPart = ft.dwLowDateTime;
    start_li.HighPart = ft.dwHighDateTime;
    // why did they choose 1601 as the time zero, instead of 1970?
    // there were no outstanding hard rock bands in 1601.
#ifdef _MSC_VER
    start_li.QuadPart -= 116444736000000000Ui64;
#else
    start_li.QuadPart -= 116444736000000000ULL;
#endif
    start.sec = (uint64_t)(start_li.QuadPart / 10000000);  // 100-ns units. odd.
    start.nsec = (start_li.LowPart % 10000000) * 100;
    if (atomic_compare_exchange_strong(&start_ns, 0, RCL_S_TO_NS(start.sec) + start.nsec)) {
      // If it matched 0 this call was first to setup, set the cpu_freq and init_cpu_time globals.
      init_cpu_time_global = init_cpu_time;
      cpu_freq_global = cpu_freq;
    } else {
      // Another concurrent first call managed to set this up first; reset start so it gets set.
      start = {0, 0};
    }
  }
  if (start.sec == 0 && start.nsec == 0) {
    start.sec = RCL_NS_TO_S(start_ns_loaded);
    start.nsec = start_ns_loaded % 1000000000;
  }
  LARGE_INTEGER cur_time;
  QueryPerformanceCounter(&cur_time);
  LARGE_INTEGER delta_cpu_time;
  delta_cpu_time.QuadPart = cur_time.QuadPart - init_cpu_time_global.QuadPart;
  double d_delta_cpu_time = delta_cpu_time.QuadPart / (double) cpu_freq_global.QuadPart;
  uint64_t delta_sec = (uint64_t) floor(d_delta_cpu_time);
  uint64_t delta_nsec = (uint64_t) floor((d_delta_cpu_time - delta_sec) * 1e9);

  *now = start;
  now->sec += delta_sec;
  now->nsec += delta_nsec;

  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
  return RCL_RET_OK;
}
#endif

rcl_ret_t
rcl_steady_time_point_now(rcl_steady_time_point_t * now)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(now, RCL_RET_INVALID_ARGUMENT);
  WINAPI ret = QueryPerformanceFrequency();
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif

#endif
