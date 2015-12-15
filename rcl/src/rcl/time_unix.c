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
#error time_unix.c is not intended to be used with win32 based systems
#endif

#if __cplusplus
extern "C"
{
#endif

#include "rcl/time.h"

#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "./common.h"
#include "rcl/error_handling.h"

#if !defined(__MACH__)  // Assume clock_get_time is available on OS X.
// This id an appropriate check for clock_gettime() according to:
//   http://man7.org/linux/man-pages/man2/clock_gettime.2.html
#if (!defined(_POSIX_TIMERS) || !_POSIX_TIMERS)
#error no monotonic clock function available
#endif
#endif

#define __WOULD_BE_NEGATIVE(seconds, subseconds) (seconds < 0 || (subseconds < 0 && seconds == 0))

rcl_ret_t
rcl_system_time_point_now(rcl_system_time_point_t * now)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(now, RCL_RET_INVALID_ARGUMENT);
  struct timespec timespec_now;
#if defined(__MACH__)
  // On OS X use clock_get_time.
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  timespec_now.tv_sec = mts.tv_sec;
  timespec_now.tv_nsec = mts.tv_nsec;
#else
  // Otherwise use clock_gettime.
  clock_gettime(CLOCK_REALTIME, &timespec_now);
#endif  // if defined(__MACH__)
  if (__WOULD_BE_NEGATIVE(timespec_now.tv_sec, timespec_now.tv_nsec)) {
    RCL_SET_ERROR_MSG("unexpected negative time");
    return RCL_RET_ERROR;
  }
  now->nanoseconds = RCL_S_TO_NS(timespec_now.tv_sec) + timespec_now.tv_nsec;
  return RCL_RET_OK;
}

rcl_ret_t
rcl_steady_time_point_now(rcl_steady_time_point_t * now)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(now, RCL_RET_INVALID_ARGUMENT);
  // If clock_gettime is available or on OS X, use a timespec.
  struct timespec timespec_now;
#if defined(__MACH__)
  // On OS X use clock_get_time.
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  timespec_now.tv_sec = mts.tv_sec;
  timespec_now.tv_nsec = mts.tv_nsec;
#else
  // Otherwise use clock_gettime.
#ifdef CLOCK_MONOTONIC_RAW
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec_now);
#else
  clock_gettime(CLOCK_MONOTONIC, &timespec_now);
#endif  // CLOCK_MONOTONIC_RAW
#endif  // if defined(__MACH__)
  if (__WOULD_BE_NEGATIVE(timespec_now.tv_sec, timespec_now.tv_nsec)) {
    RCL_SET_ERROR_MSG("unexpected negative time");
    return RCL_RET_ERROR;
  }
  now->nanoseconds = RCL_S_TO_NS(timespec_now.tv_sec) + timespec_now.tv_nsec;
  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
