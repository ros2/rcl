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

#include "rcl/time.h"

rcl_time_t
rcl_time_t_from_int64_t_nanoseconds(int64_t nanoseconds)
{
  rcl_time_t result;
  result.sec = RCL_NS_TO_S(nanoseconds);
  result.nsec = nanoseconds % 1000000000;
  return result;
}

rcl_time_t
rcl_time_t_from_uint64_t_nanoseconds(uint64_t nanoseconds)
{
  rcl_time_t result;
  result.sec = RCL_NS_TO_S(nanoseconds);
  result.nsec = nanoseconds % 1000000000;
  return result;
}

#if __cplusplus
}
#endif
