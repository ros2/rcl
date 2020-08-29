// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include <yaml.h>

#include <string>
#include <vector>

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl_yaml_param_parser/parser.h"
#include "../src/impl/yaml_variant.h"
#include "rcutils/allocator.h"
#include "rcutils/strdup.h"
#include "rcutils/testing/fault_injection.h"

#define TEST_VARIANT_COPY(dest_variant, src_variant, field, tmp_var, allocator) \
  do { \
    SCOPED_TRACE("TEST_VARIANT_COPY " #field); \
    src_variant.field = &tmp_var; \
    EXPECT_TRUE(rcl_yaml_variant_copy(&dest_variant, &src_variant, allocator)); \
    ASSERT_NE(nullptr, dest_variant.field); \
    EXPECT_EQ(*src_variant.field, *dest_variant.field); \
    rcl_yaml_variant_fini(&dest_variant, allocator); \
    variant.field = nullptr; \
  } while (0)

#define TEST_VARIANT_ARRAY_COPY( \
    dest_variant, src_variant, field, array_type, value_type, tmp_array, array_size, allocator) \
  do { \
    SCOPED_TRACE("TEST_VARIANT_ARRAY_COPY " #field); \
    src_variant.field = \
      static_cast<array_type *>(allocator.allocate(sizeof(array_type), allocator.state)); \
    ASSERT_NE(nullptr, src_variant.field); \
    src_variant.field->values = \
      static_cast<value_type *>( \
      allocator.zero_allocate(array_size, sizeof(value_type), allocator.state)); \
    ASSERT_NE(nullptr, src_variant.field->values); \
    src_variant.field->size = array_size; \
    for (size_t i = 0; i < array_size; ++i) { \
      variant.field->values[i] = tmp_array[i]; \
    } \
    EXPECT_TRUE(rcl_yaml_variant_copy(&dest_variant, &src_variant, allocator)); \
    ASSERT_NE(nullptr, dest_variant.field); \
    ASSERT_NE(nullptr, dest_variant.field->values); \
    for (size_t i = 0; i < array_size; ++i) { \
      SCOPED_TRACE(i); \
      EXPECT_EQ(src_variant.field->values[i], dest_variant.field->values[i]); \
    } \
    rcl_yaml_variant_fini(&src_variant, allocator); \
    rcl_yaml_variant_fini(&dest_variant, allocator); \
    src_variant.field = nullptr; \
    dest_variant.field = nullptr; \
  } while (0)

TEST(TestYamlVariant, copy_fini) {
  rcl_variant_t variant =
  {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rcl_variant_t copy =
  {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
  EXPECT_FALSE(rcl_yaml_variant_copy(nullptr, &variant, allocator));
  EXPECT_FALSE(rcl_yaml_variant_copy(&copy, nullptr, allocator));

  bool tmp_bool = true;
  TEST_VARIANT_COPY(copy, variant, bool_value, tmp_bool, allocator);

  int64_t tmp_int = 42;
  TEST_VARIANT_COPY(copy, variant, integer_value, tmp_int, allocator);

  double tmp_double = 3.14159;
  TEST_VARIANT_COPY(copy, variant, double_value, tmp_double, allocator);

  // String version is slightly different and can't use the above macro
  char * tmp_string = rcutils_strdup("hello there", allocator);
  variant.string_value = tmp_string;
  EXPECT_TRUE(rcl_yaml_variant_copy(&copy, &variant, allocator));
  ASSERT_NE(nullptr, copy.string_value);
  EXPECT_STREQ(tmp_string, copy.string_value);
  rcl_yaml_variant_fini(&copy, allocator);
  allocator.deallocate(tmp_string, allocator.state);
  variant.string_value = nullptr;

  constexpr size_t size = 3u;
  constexpr bool bool_arry[size] = {true, false, true};
  TEST_VARIANT_ARRAY_COPY(
    copy, variant, bool_array_value, rcl_bool_array_t, bool, bool_arry, size, allocator);

  constexpr int64_t int_arry[size] = {1, 2, 3};
  TEST_VARIANT_ARRAY_COPY(
    copy, variant, integer_array_value, rcl_int64_array_t, int64_t, int_arry, size, allocator);

  constexpr double double_arry[size] = {10.0, 11.0, 12.0};
  TEST_VARIANT_ARRAY_COPY(
    copy, variant, double_array_value, rcl_double_array_t, double, double_arry, size, allocator);

  // Strings just have to be different
  variant.string_array_value =
    static_cast<rcutils_string_array_t *>(
    allocator.allocate(sizeof(rcutils_string_array_t), allocator.state));
  ASSERT_NE(nullptr, variant.string_array_value);
  *variant.string_array_value = rcutils_get_zero_initialized_string_array();
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_string_array_init(variant.string_array_value, size, &allocator));
  variant.string_array_value->size = size;
  variant.string_array_value->data[0] = rcutils_strdup("string1", allocator);
  variant.string_array_value->data[1] = rcutils_strdup("string2", allocator);
  variant.string_array_value->data[2] = rcutils_strdup("string3", allocator);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_NE(nullptr, variant.string_array_value->data[i]);
  }
  EXPECT_TRUE(rcl_yaml_variant_copy(&copy, &variant, allocator));
  ASSERT_NE(nullptr, copy.string_array_value);
  ASSERT_NE(nullptr, copy.string_array_value->data);
  for (size_t i = 0; i < size; ++i) {
    SCOPED_TRACE(i);
    EXPECT_STREQ(variant.string_array_value->data[i], copy.string_array_value->data[i]);
  }
  rcl_yaml_variant_fini(&variant, allocator);
  rcl_yaml_variant_fini(&copy, allocator);

  // Check second fini works fine
  rcl_yaml_variant_fini(&copy, allocator);

  // Check fini with a nullptr doesn't crash.
  rcl_yaml_variant_fini(nullptr, allocator);
}
