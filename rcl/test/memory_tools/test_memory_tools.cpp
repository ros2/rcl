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

#include <gtest/gtest.h>

#include "./memory_tools.hpp"

/* Tests the allocatation checking tools.
 */
TEST(TestMemoryTools, test_allocation_checking_tools) {
  size_t unexpected_mallocs = 0;
  auto on_unexpected_malloc = ([&unexpected_mallocs]() {
    unexpected_mallocs++;
  });
  set_on_unexpected_malloc_callback(on_unexpected_malloc);
  size_t unexpected_reallocs = 0;
  auto on_unexpected_realloc = ([&unexpected_reallocs]() {
    unexpected_reallocs++;
  });
  set_on_unexpected_realloc_callback(on_unexpected_realloc);
  size_t unexpected_frees = 0;
  auto on_unexpected_free = ([&unexpected_frees]() {
    unexpected_frees++;
  });
  set_on_unexpected_free_callback(on_unexpected_free);
  void * mem = nullptr;
  void * remem = nullptr;
  // First try before enabling, should have no effect.
  mem = malloc(1024);
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  EXPECT_EQ(unexpected_mallocs, 0u);
  EXPECT_EQ(unexpected_reallocs, 0u);
  EXPECT_EQ(unexpected_frees, 0u);
  // Enable checking, but no assert, should have no effect.
  start_memory_checking();
  mem = malloc(1024);
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  EXPECT_EQ(unexpected_mallocs, 0u);
  EXPECT_EQ(unexpected_reallocs, 0u);
  EXPECT_EQ(unexpected_frees, 0u);
  // Enable no_* asserts, should increment all once.
  assert_no_malloc_begin();
  assert_no_realloc_begin();
  assert_no_free_begin();
  mem = malloc(1024);
  assert_no_malloc_end();
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  assert_no_realloc_end();
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  assert_no_free_end();
  EXPECT_EQ(unexpected_mallocs, 1u);
  EXPECT_EQ(unexpected_reallocs, 1u);
  EXPECT_EQ(unexpected_frees, 1u);
  // Enable on malloc assert, only malloc should increment.
  assert_no_malloc_begin();
  mem = malloc(1024);
  assert_no_malloc_end();
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  EXPECT_EQ(unexpected_mallocs, 2u);
  EXPECT_EQ(unexpected_reallocs, 1u);
  EXPECT_EQ(unexpected_frees, 1u);
  // Enable on realloc assert, only realloc should increment.
  assert_no_realloc_begin();
  mem = malloc(1024);
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  assert_no_realloc_end();
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  EXPECT_EQ(unexpected_mallocs, 2u);
  EXPECT_EQ(unexpected_reallocs, 2u);
  EXPECT_EQ(unexpected_frees, 1u);
  // Enable on free assert, only free should increment.
  assert_no_free_begin();
  mem = malloc(1024);
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  assert_no_free_end();
  EXPECT_EQ(unexpected_mallocs, 2u);
  EXPECT_EQ(unexpected_reallocs, 2u);
  EXPECT_EQ(unexpected_frees, 2u);
  // Go again, after disabling asserts, should have no effect.
  mem = malloc(1024);
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  EXPECT_EQ(unexpected_mallocs, 2u);
  EXPECT_EQ(unexpected_reallocs, 2u);
  EXPECT_EQ(unexpected_frees, 2u);
  // Go once more after disabling everything, should have no effect.
  stop_memory_checking();
  mem = malloc(1024);
  ASSERT_NE(mem, nullptr);
  remem = realloc(mem, 2048);
  ASSERT_NE(remem, nullptr);
  if (!remem) {free(mem);}
  free(remem);
  EXPECT_EQ(unexpected_mallocs, 2u);
  EXPECT_EQ(unexpected_reallocs, 2u);
  EXPECT_EQ(unexpected_frees, 2u);
}
