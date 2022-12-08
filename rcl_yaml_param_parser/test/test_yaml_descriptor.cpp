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

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "../src/impl/yaml_descriptor.h"
#include "rcutils/allocator.h"
#include "rcutils/strdup.h"

#define TEST_DESCRIPTOR_COPY(field, tmp_field) \
  do { \
    SCOPED_TRACE("TEST_DESCRIPTOR_COPY " #field); \
    rcl_param_descriptor_t src_descriptor{}; \
    rcl_param_descriptor_t dest_descriptor{}; \
    rcutils_allocator_t allocator = rcutils_get_default_allocator(); \
    src_descriptor.field = &tmp_field; \
    EXPECT_TRUE(rcl_yaml_descriptor_copy(&dest_descriptor, &src_descriptor, allocator)); \
    ASSERT_NE(nullptr, dest_descriptor.field); \
    EXPECT_EQ(*src_descriptor.field, *dest_descriptor.field); \
    rcl_yaml_descriptor_fini(&dest_descriptor, allocator); \
    src_descriptor.field = nullptr; \
  } while (0)

TEST(TestYamlDescriptor, copy_fini) {
  rcl_param_descriptor_t descriptor{};
  rcl_param_descriptor_t copy{};
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  EXPECT_FALSE(rcl_yaml_descriptor_copy(nullptr, &descriptor, allocator));
  EXPECT_FALSE(rcl_yaml_descriptor_copy(&copy, nullptr, allocator));

  ASSERT_TRUE(rcl_yaml_descriptor_copy(&copy, &descriptor, allocator));

  rcl_yaml_descriptor_fini(&copy, allocator);

  // Check second fini works fine
  rcl_yaml_descriptor_fini(&copy, allocator);

  // Check fini with a nullptr doesn't crash.
  rcl_yaml_descriptor_fini(nullptr, allocator);
}

TEST(TestYamlDescriptor, copy_fields) {
  bool tmp_read_only = true;
  TEST_DESCRIPTOR_COPY(read_only, tmp_read_only);

  uint8_t tmp_type = 2;
  TEST_DESCRIPTOR_COPY(type, tmp_type);

  double tmp_min_value_double = -5.5;
  TEST_DESCRIPTOR_COPY(min_value_double, tmp_min_value_double);

  double tmp_max_value_double = 16.4;
  TEST_DESCRIPTOR_COPY(max_value_double, tmp_max_value_double);

  double tmp_step_double = 0.1;
  TEST_DESCRIPTOR_COPY(step_double, tmp_step_double);

  int64_t tmp_min_value_int = 1;
  TEST_DESCRIPTOR_COPY(min_value_int, tmp_min_value_int);

  int64_t tmp_max_value_int = 1001;
  TEST_DESCRIPTOR_COPY(max_value_int, tmp_max_value_int);

  int64_t tmp_step_int = 5;
  TEST_DESCRIPTOR_COPY(step_int, tmp_step_int);
}

TEST(TestYamlDescriptor, copy_string_fields) {
  // String version is slightly different and can't use the above macro
  rcl_param_descriptor_t src_descriptor{};
  rcl_param_descriptor_t dest_descriptor{};
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  char * tmp_description = rcutils_strdup("param description", allocator);
  ASSERT_STREQ("param description", tmp_description);

  char * tmp_additional_constraints = rcutils_strdup("param additional constraints", allocator);
  ASSERT_STREQ("param additional constraints", tmp_additional_constraints);

  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(tmp_description, allocator.state);
    allocator.deallocate(tmp_additional_constraints, allocator.state);
  });

  src_descriptor.description = tmp_description;
  src_descriptor.additional_constraints = tmp_additional_constraints;

  EXPECT_TRUE(rcl_yaml_descriptor_copy(&dest_descriptor, &src_descriptor, allocator));
  ASSERT_NE(nullptr, dest_descriptor.description);
  ASSERT_NE(nullptr, dest_descriptor.additional_constraints);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_descriptor_fini(&dest_descriptor, allocator);
  });
  EXPECT_STREQ(tmp_description, dest_descriptor.description);
  EXPECT_STREQ(tmp_additional_constraints, dest_descriptor.additional_constraints);
}
