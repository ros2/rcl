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

#ifndef RCL__TYPES_H_
#define RCL__TYPES_H_

#include <rmw/types.h>

typedef rmw_ret_t rcl_ret_t;
#define RCL_RET_OK RMW_RET_OK
#define RCL_RET_ERROR RMW_RET_ERROR
#define RCL_RET_TIMEOUT RMW_RET_TIMEOUT
// rcl specific ret codes start at 100
#define RCL_RET_ALREADY_INIT 100
#define RCL_RET_NOT_INIT 101
#define RCL_RET_BAD_ALLOC 102
#define RCL_RET_INVALID_ARGUMENT 103
#define RCL_RET_MISMATCHED_RMW_ID 104
// rcl node specific ret codes in 2XX
#define RCL_RET_NODE_INVALID 200
// rcl publisher specific ret codes in 3XX
#define RCL_RET_PUBLISHER_INVALID 300
// rcl subscription specific ret codes in 4XX
#define RCL_RET_SUBSCRIPTION_INVALID 400
#define RCL_RET_SUBSCRIPTION_TAKE_FAILED 401
// rcl service client specific ret codes in 5XX
#define RCL_RET_CLIENT_INVALID 500
#define RCL_RET_CLIENT_TAKE_FAILED 501
// rcl service server specific ret codes in 6XX
#define RCL_RET_SERVICE_INVALID 600
#define RCL_RET_SERVICE_TAKE_FAILED 601
// rcl guard condition specific ret codes in 7XX
// rcl timer specific ret codes in 8XX
#define RCL_RET_TIMER_INVALID 800
#define RCL_RET_TIMER_CANCELED 801
// rcl wait and wait set specific ret codes in 9XX
#define RCL_RET_WAIT_SET_INVALID 900
#define RCL_RET_WAIT_SET_EMPTY 901
#define RCL_RET_WAIT_SET_FULL 902

#endif  // RCL__TYPES_H_
