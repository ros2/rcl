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

#if defined(__linux__)
/******************************************************************************
 * Begin Linux
 *****************************************************************************/

// TODO(wjwwood): install custom malloc (and others) for Linux.

#include "./memory_tools_common.cpp"

/******************************************************************************
 * End Linux
 *****************************************************************************/
#elif defined(__APPLE__)
/******************************************************************************
 * Begin Apple
 *****************************************************************************/

// The apple implementation is in a separate shared library, loaded with DYLD_INSERT_LIBRARIES.
// Therefore we do not include the common cpp file here.

void osx_start_memory_checking();
void osx_stop_memory_checking();

void start_memory_checking()
{
  return osx_start_memory_checking();
}

void stop_memory_checking()
{
  return osx_stop_memory_checking();
}

/******************************************************************************
 * End Apple
 *****************************************************************************/
#elif defined(WIN32)
/******************************************************************************
 * Begin Windows
 *****************************************************************************/

// TODO(wjwwood): install custom malloc (and others) for Windows.

/******************************************************************************
 * End Windows
 *****************************************************************************/
#else
// Default case: do nothing.

void start_memory_checking()
{
  MALLOC_PRINTF("starting memory checking... not available\n");
}
void stop_memory_checking()
{
  MALLOC_PRINTF("stopping memory checking... not available\n");
}

#endif  // !defined(WIN32)
