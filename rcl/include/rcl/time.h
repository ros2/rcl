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

#ifndef RCL__TIME_H_
#define RCL__TIME_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/types.h"
#include "rcl/visibility_control.h"

#define RCL_S_TO_NS(seconds) (seconds * 1000000000)
#define RCL_MS_TO_NS(milliseconds) (milliseconds * 1000000)
#define RCL_US_TO_NS(microseconds) (microseconds * 1000)

#define RCL_NS_TO_S(nanoseconds) (nanoseconds / 1000000000)
#define RCL_NS_TO_MS(nanoseconds) (nanoseconds / 1000000)
#define RCL_NS_TO_US(nanoseconds) (nanoseconds / 1000)

/// Struct which encapsulates a point in time according to a system clock.
/* The system clock's point of reference is the Unix Epoch (January 1st, 1970).
 * See: http://stackoverflow.com/a/32189845/671658
 * Therefore all times in this struct are positive and count the nanoseconds
 * since the Unix epoch and this struct cannot be used on systems which report
 * a current time before the Unix Epoch.
 * Leap seconds are not considered.
 *
 * The struct represents time as nanoseconds in an unsigned 64-bit integer.
 * The struct is capable of representing any time until the year 2554 with
 * nanosecond precisions.
 */
typedef struct rcl_system_time_point_t
{
  uint64_t nanoseconds;
} rcl_system_time_point_t;

/// Struct which encapsulates a point in time according to a steady clock.
/* The steady clock's point of reference is system defined, but often the time
 * that the process started plus a signed random offset.
 * See: http://stackoverflow.com/a/32189845/671658
 * However, the clock is guaranteed to be monotonically increasing.
 * Therefore all times in this struct are positive and count nanoseconds
 * since an unspecified, but constant point in time.
 *
 * The struct represents time as nanoseconds in an unsigned 64-bit integer.
 */
typedef struct rcl_steady_time_point_t
{
  uint64_t nanoseconds;
} rcl_steady_time_point_t;

/// Struct which encapsulates a duration of time between two points in time.
/* The struct can represent any time within the range [~292 years, ~-292 years]
 * with nanosecond precision.
 */
typedef struct rcl_duration_t
{
  int64_t nanoseconds;
} rcl_duration_t;

/// Retrieve the current time as a rcl_system_time_point_t struct.
/* This function returns the time from a system clock.
 * The closest equivalent would be to std::chrono::system_clock::now();
 *
 * The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.
 *
 * The now argument must point to an allocated rcl_system_time_point_t struct,
 * as the result is copied into this variable.
 *
 * This function may allocate heap memory when an error occurs.
 * This function is thread-safe.
 * This function is lock-free, with an exception on Windows.
 * On Windows this is lock-free if the C11's stdatomic.h function
 * atomic_is_lock_free() returns true for atomic_int_least64_t.
 *
 * \param[out] now a struct in which the current time is stored
 * \return RCL_RET_OK if the current time was successfully obtained, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR an unspecified error occur.
 */
RCL_PUBLIC
rcl_ret_t
rcl_system_time_point_now(rcl_system_time_point_t * now);

/// Retrieve the current time as a rcl_steady_time_point_t struct.
/* This function returns the time from a monotonically increasing clock.
 * The closest equivalent would be to std::chrono::steady_clock::now();
 *
 * The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.
 *
 * The now argument must point to an allocated rcl_steady_time_point_t struct,
 * as the result is copied into this variable.
 *
 * This function may allocate heap memory when an error occurs.
 * This function is thread-safe.
 * This function is lock-free, with an exception on Windows.
 * On Windows this is lock-free if the C11's stdatomic.h function
 * atomic_is_lock_free() returns true for atomic_int_least64_t.
 *
 * \param[out] now a struct in which the current time is stored
 * \return RCL_RET_OK if the current time was successfully obtained, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR an unspecified error occur.
 */
RCL_PUBLIC
rcl_ret_t
rcl_steady_time_point_now(rcl_steady_time_point_t * now);

#if __cplusplus
}
#endif

#endif  // RCL__TIME_H_
