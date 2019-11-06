// Copyright (c) 2018 - for information on the respective copyright owner
// see the NOTICE file and/or the repository https://github.com/micro-ROS/rcl_executor.
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

#include "rcl_executor/handle.h"

#include <stdio.h>

#include <rcl/error_handling.h>
#include <rcutils/logging_macros.h>

// initialization of info object
rcl_ret_t
rcle_handle_size_zero_init(rcle_handle_size_t * info)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(info, RCL_RET_INVALID_ARGUMENT);

  info->number_of_subscriptions = 0;
  info->number_of_guard_conditions = 0;
  info->number_of_timers = 0;
  info->number_of_clients = 0;
  info->number_of_events = 0;
  info->number_of_services = 0;

  return RCL_RET_OK;
}

// initialization of handle object
rcl_ret_t
rcle_handle_init(rcle_handle_t * h, size_t max_handles)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(h, RCL_RET_INVALID_ARGUMENT);

  h->type = NONE;
  h->invocation = ON_NEW_DATA;
  h->subscription = NULL;
  h->timer = NULL;
  h->data = NULL;
  h->callback = NULL;
  h->index = max_handles;
  h->initialized = false;
  h->data_available = false;

  return RCL_RET_OK;
}

rcl_ret_t
rcle_handle_clear(rcle_handle_t * h, size_t max_handles)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(h, RCL_RET_INVALID_ARGUMENT);

  h->index = max_handles;
  h->initialized = false;

  return RCL_RET_OK;
}


rcl_ret_t
rcle_handle_print(rcle_handle_t * h)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(h, RCL_RET_INVALID_ARGUMENT);

  char * typeName;

  switch (h->type) {
    case NONE:
      typeName = "None";
      break;
    case SUBSCRIPTION:
      typeName = "Sub";
      break;
    case TIMER:
      typeName = "Timer";
      break;
    default:
      typeName = "Unknown";
  }
  RCUTILS_LOG_DEBUG_NAMED(ROS_PACKAGE_NAME, "  %s\n", typeName);

  return RCL_RET_OK;
}
