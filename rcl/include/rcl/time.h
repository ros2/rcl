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
#include "rmw/rmw.h"

#define RCL_S_TO_NS(seconds) seconds * 1000000000
#define RCL_MS_TO_NS(milliseconds) milliseconds * 1000000
#define RCL_US_TO_NS(microseconds) microseconds * 1000

#define RCL_NS_TO_S(nanoseconds) nanoseconds / 1000000000
#define RCL_NS_TO_MS(nanoseconds) nanoseconds / 1000000
#define RCL_NS_TO_US(nanoseconds) nanoseconds / 1000

typedef rmw_time_t rcl_time_t;

/// Create a rcl_time_t struct with a given signed number of nanoseconds.
rcl_time_t
rcl_time_from_int64_t_nanoseconds(int64_t nanoseconds);

/// Create a rcl_time_t struct with a given unsigned number of nanoseconds.
rcl_time_t
rcl_time_from_uint64_t_nanoseconds(uint64_t nanoseconds);

/// Retrieve the current time as a rcl_time_t struct.
/* The now argument must point to an allocated rcl_time_t struct, as the
 * result is copied into this variable.
 *
 * This function is thread-safe.
 * This function is lock-free, with an exception on Windows.
 * On Windows this is lock-free if the C11's stdatomic.h function
 * atomic_is_lock_free() returns true for atomic_int_least64_t.
 *
 * \param[out] now a rcl_time_t struct in which the current time is stored
 * \return RCL_RET_OK if the current time was successfully obtained, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR an unspecified error occur.
 */
rcl_ret_t
rcl_time_now(rcl_time_t * now);

/// Normalize a time struct.
/* If there are more than 10^9 nanoseconds in the nsec field, the extra seconds
 * are removed from the nsec field and added to the sec field.
 * Overflow of the sec field due to this normalization is ignored.
 *
 * This function is thread-safe.
 * This function is lock-free, with an exception on Windows.
 * On Windows this is lock-free if the C11's stdatomic.h function
 * atomic_is_lock_free() returns true for atomic_int_least64_t.
 *
 * \param[out] now a rcl_time_t struct to be normalized
 * \return RCL_RET_OK if the struct was normalized successfully, or
 *         RCL_RET_INVALID_ARGUMENT if any arguments are invalid, or
 *         RCL_RET_ERROR an unspecified error occur.
 */
rcl_ret_t
rcl_time_normalize(rcl_time_t * now);

#if __cplusplus
}
#endif

#endif  // RCL__TIME_H_
