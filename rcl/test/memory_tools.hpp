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

// Code to do replacing of malloc/free/etc... inspired by:
//   https://dxr.mozilla.org/mozilla-central/rev/
//    cc9c6cd756cb744596ba039dcc5ad3065a7cc3ea/memory/build/replace_malloc.c

#ifndef TEST__MEMORY_TOOLS_HPP_
#define TEST__MEMORY_TOOLS_HPP_

#include <gtest/gtest.h>

#include <functional>
#include <stddef.h>

typedef std::function<void()> UnexpectedCallbackType;

void start_memory_checking();

#define ASSERT_NO_MALLOC(statements) assert_no_malloc_begin(); statements; assert_no_malloc_end();
void assert_no_malloc_begin();
void assert_no_malloc_end();
void set_on_unepexcted_malloc_callback(UnexpectedCallbackType callback);

#define ASSERT_NO_REALLOC(statements) assert_no_realloc_begin(); statements; assert_no_realloc_end();
void assert_no_realloc_begin();
void assert_no_realloc_end();
void set_on_unepexcted_realloc_callback(UnexpectedCallbackType callback);

#define ASSERT_NO_FREE(statements) assert_no_free_begin(); statements; assert_no_free_end();
void assert_no_free_begin();
void assert_no_free_end();
void set_on_unepexcted_free_callback(UnexpectedCallbackType callback);

void stop_memory_checking();

void memory_checking_thread_init();

#endif  // TEST__MEMORY_TOOLS_HPP_
