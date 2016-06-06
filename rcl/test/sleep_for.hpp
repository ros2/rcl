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

#ifndef SLEEP_FOR_HPP_
#define SLEEP_FOR_HPP_

#include <chrono>
#include <thread>

namespace rcl_test
{

bool
sleep_for(const std::chrono::nanoseconds & nanoseconds)
{
  std::chrono::nanoseconds time_left = nanoseconds;
  auto start = std::chrono::steady_clock::now();
  std::this_thread::sleep_for(time_left);
  time_left -= std::chrono::steady_clock::now() - start;
  if (time_left > std::chrono::nanoseconds::zero()) {
    return sleep_for(time_left);
  }
  return false;
}

}

#endif  // SLEEP_FOR_HPP_
