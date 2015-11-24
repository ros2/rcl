// Copyright 2014 Open Source Robotics Foundation, Inc.
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

#ifndef RCL__RCL_H_
#define RCL__RCL_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/subscription.h"
#include "rcl/types.h"

/// Global initialization of rcl.
/* Unless otherwise noted, this must be called before using any rcl functions.
 *
 * This function can only be run once after starting the program, and once
 * after each call to rcl_fini.
 * Repeated calls will fail with RCL_RET_ALREADY_INIT.
 * This function is not thread safe.
 *
 * This function can be called any time after rcl_fini is called, but it
 * cannot be called from within a callback being executed by an rcl executor.
 * For example, you can call rcl_fini from within a timer callback, but
 * you have to return from the callback, and therefore exit any in-progress
 * call to a spin function, before calling rcl_init again.
 *
 * The argc and argv parameters can contain command line arguments for the
 * program.
 * rcl specific arguments will be parsed and removed, but other arguments will
 * be ignored.
 *
 * \param[in] argc number of strings in argv
 * \param[in] argv command line arguments; rcl specific arguments are removed
 * \return RCL_RET_OK if initialization is successful, otherwise RCL_RET_ERROR
 *         or RCL_RET_ALREADY_INIT if rcl_init has already been called
 */
rcl_ret_t
rcl_init(int argc, char ** argv);

/// Signal global shutdown of rcl.
/* This function does not have to be called on exit, but does have to be called
 * making a repeat call to rcl_init.
 *
 * This function can only be called once after each call to rcl_init.
 * Repeated calls will fail with RCL_RET_NOT_INIT.
 * This function is not thread safe.
 *
 * When this function is called:
 *  - Any rcl objects created since the last call to rcl_init are invalidated.
 *  - Calls to rcl_ok will return false.
 *  - Any executors waiting for work (within a call to spin) are interrupted.
 *  - No new work (executing callbacks) will be done in executors.
 *  - Currently running work in executors will be finished.
 *
 * \return RCL_RET_OK if shutdown is successful, otherwise RCL_RET_ERROR or
 *         RCL_RET_NOT_INIT if rcl_init has not yet been called
 */
rcl_ret_t
rcl_fini();

/// Return true until rcl_fini is called, then false.
/* This function is thread safe. */
bool
rcl_ok();

#if __cplusplus
}
#endif

#endif  // RCL__RCL_H_
