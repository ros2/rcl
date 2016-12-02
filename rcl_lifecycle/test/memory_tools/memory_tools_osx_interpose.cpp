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

// Pulled from:
//  https://github.com/emeryberger/Heap-Layers/blob/
//    076e9e7ef53b66380b159e40473b930f25cc353b/wrappers/macinterpose.h

// The interposition data structure (just pairs of function pointers),
// used an interposition table like the following:
//

typedef struct interpose_s
{
  void * new_func;
  void * orig_func;
} interpose_t;

#define OSX_INTERPOSE(newf, oldf) \
  __attribute__((used)) static const interpose_t \
  macinterpose ## newf ## oldf __attribute__ ((section("__DATA, __interpose"))) = { \
    reinterpret_cast<void *>(newf), \
    reinterpret_cast<void *>(oldf), \
  }

// End Interpose.

#include "./memory_tools_common.cpp"

void osx_start_memory_checking()
{
  // No loading required, it is handled by DYLD_INSERT_LIBRARIES and dynamic library interposing.
  if (!enabled.exchange(true)) {
    MALLOC_PRINTF("starting memory checking...\n");
  }
}

void osx_stop_memory_checking()
{
  if (enabled.exchange(false)) {
    MALLOC_PRINTF("stopping memory checking...\n");
  }
}

OSX_INTERPOSE(custom_malloc, malloc);
OSX_INTERPOSE(custom_realloc, realloc);
OSX_INTERPOSE(custom_free, free);
