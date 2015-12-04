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

#if __cplusplus
extern "C"
{
#endif

// Appropriate check accorind to:
//   http://man7.org/linux/man-pages/man2/clock_gettime.2.html
#define HAS_CLOCK_GETTIME (_POSIX_C_SOURCE >= 199309L)

#include "rcl/time.h"

#include <math.h>
#include <sys/time.h>
#include <time.h>

#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#if defined(WIN32)
#include <stdatomic.h>
#include <windows.h>
#endif

#include "./common.h"
#include "rcl/error_handling.h"

rcl_time_t
rcl_time_from_int64_t_nanoseconds(int64_t nanoseconds)
{
  rcl_time_t result;
  result.sec = RCL_NS_TO_S(nanoseconds);
  result.nsec = nanoseconds % 1000000000;
  return result;
}

rcl_time_t
rcl_time_from_uint64_t_nanoseconds(uint64_t nanoseconds)
{
  rcl_time_t result;
  result.sec = RCL_NS_TO_S(nanoseconds);
  result.nsec = nanoseconds % 1000000000;
  return result;
}

uint64_t
rcl_time_to_uint64_t_nanoseconds(const rcl_time_t * rcl_time)
{
  if (!rcl_time) {
    return 0;
  }
  return RCL_S_TO_NS(rcl_time->sec) + rcl_time->nsec;
}

static void
__timespec_get_now(struct timespec * timespec_now)
{
#if defined(__MACH__)
  // On OS X use clock_get_time.
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  timespec_now->tv_sec = mts.tv_sec;
  timespec_now->tv_nsec = mts.tv_nsec;
#else  // if defined(__MACH__)
  // Otherwise use clock_gettime.
  clock_gettime(CLOCK_REALTIME, timespec_now);
#endif  // if defined(__MACH__)
}

rcl_ret_t
rcl_time_now(rcl_time_t * now)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(now, RCL_RET_INVALID_ARGUMENT);
#ifndef WIN32
  // Unix implementations
#if HAS_CLOCK_GETTIME || defined(__MACH__)
  // If clock_gettime is available or on OS X, use a timespec.
  struct timespec timespec_now;
  __timespec_get_now(&timespec_now);
  now->sec  = timespec_now.tv_sec;
  now->nsec = timespec_now.tv_nsec;
#else  // if HAS_CLOCK_GETTIME || defined(__MACH__)
  // Otherwise we have to fallback to gettimeofday.
  struct timeval timeofday;
  gettimeofday(&timeofday, NULL);
  now->sec  = timeofday.tv_sec;
  now->nsec = timeofday.tv_usec * 1000;
#endif  // if HAS_CLOCK_GETTIME || defined(__MACH__)
#else
  // Windows implementation adapted from roscpp_core (3-clause BSD), see:
  //   https://github.com/ros/roscpp_core/blob/0.5.6/rostime/src/time.cpp#L96
  // Win32 implementation
  // unless I've missed something obvious, the only way to get high-precision
  // time on Windows is via the QueryPerformanceCounter() call. However,
  // this is somewhat problematic in Windows XP on some processors, especially
  // AMD, because the Windows implementation can freak out when the CPU clocks
  // down to save power. Time can jump or even go backwards. Microsoft has
  // fixed this bug for most systems now, but it can still show up if you have
  // not installed the latest CPU drivers (an oxymoron). They fixed all these
  // problems in Windows Vista, and this API is by far the most accurate that
  // I know of in Windows, so I'll use it here despite all these caveats
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
    start.sec = (uint64_t)(start_li.QuadPart / 10000000); // 100-ns units. odd.
    start.nsec = (start_li.LowPart % 10000000) * 100;
    if (atomic_compare_exchange_strong(&start_ns, 0, (start.sec * 1000000000) + start.nsec)) {
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

  // Normalize the time struct.
  rcl_ret_t ret = rcl_time_normalize(now);
  if (ret != RCL_RET_OK) {
    return ret;  // rcl error state should already be set.
  }
#endif
  return RCL_RET_OK;
}

rcl_ret_t
rcl_time_normalize(rcl_time_t * now)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(now, RCL_RET_INVALID_ARGUMENT);
  now->sec += now->nsec / 1000000000;
  now->nsec = now->nsec % 1000000000;
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
