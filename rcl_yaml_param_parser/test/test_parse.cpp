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
#include "rcl_yaml_param_parser/impl/parse.h"
#include "rcl_yaml_param_parser/impl/node_params.h"
#include "rcutils/filesystem.h"

TEST(TestParse, parse_value) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yaml_event_t event;
  event.type = YAML_NO_EVENT;

  bool is_seq = false;
  size_t node_idx = 0u;
  size_t parameter_idx = 0u;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&params_st->params[0], allocator));
  params_st->num_nodes = 1u;

  // bool value
  yaml_char_t bool_value[] = "true";
  const size_t bool_value_length = sizeof(bool_value) / sizeof(bool_value[0]);
  event.data.scalar.value = bool_value;
  event.data.scalar.length = bool_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].bool_value);
  EXPECT_TRUE(*params_st->params[0].parameter_values[0].bool_value);
  allocator.deallocate(params_st->params[0].parameter_values[0].bool_value, allocator.state);
  params_st->params[0].parameter_values[0].bool_value = nullptr;

  // integer value
  yaml_char_t integer_value[] = "42";
  const size_t integer_value_length = sizeof(integer_value) / sizeof(integer_value[0]);
  event.data.scalar.value = integer_value;
  event.data.scalar.length = integer_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].integer_value);
  EXPECT_EQ(42, *params_st->params[0].parameter_values[0].integer_value);
  allocator.deallocate(params_st->params[0].parameter_values[0].integer_value, allocator.state);
  params_st->params[0].parameter_values[0].integer_value = nullptr;

  // double value
  yaml_char_t double_value[] = "3.14159";
  const size_t double_value_length = sizeof(double_value) / sizeof(double_value[0]);
  event.data.scalar.value = double_value;
  event.data.scalar.length = double_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].double_value);
  EXPECT_EQ(3.14159, *params_st->params[0].parameter_values[0].double_value);
  allocator.deallocate(params_st->params[0].parameter_values[0].double_value, allocator.state);
  params_st->params[0].parameter_values[0].double_value = nullptr;

  // double value
  yaml_char_t string_value[] = "hello, I am a string";
  const size_t string_value_length = sizeof(string_value) / sizeof(string_value[0]);
  event.data.scalar.value = string_value;
  event.data.scalar.length = string_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].string_value);
  EXPECT_STREQ("hello, I am a string", params_st->params[0].parameter_values[0].string_value);
  allocator.deallocate(params_st->params[0].parameter_values[0].string_value, allocator.state);
  params_st->params[0].parameter_values[0].string_value = nullptr;
}

TEST(TestParse, parse_value_sequence) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yaml_event_t event;
  event.type = YAML_NO_EVENT;

  bool is_seq = true;
  size_t node_idx = 0u;
  size_t parameter_idx = 0u;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&params_st->params[0], allocator));
  params_st->num_nodes = 1u;

  // bool array value
  yaml_char_t bool_value[] = "true";
  const size_t bool_value_length = sizeof(bool_value) / sizeof(bool_value[0]);
  event.data.scalar.value = bool_value;
  event.data.scalar.length = bool_value_length;

  // Check bad sequence type for bool
  seq_data_type = DATA_TYPE_STRING;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_EQ(nullptr, params_st->params[0].parameter_values[0].integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].bool_array_value);
  EXPECT_TRUE(params_st->params[0].parameter_values[0].bool_array_value->values[0]);
  allocator.deallocate(
    params_st->params[0].parameter_values[0].bool_array_value->values, allocator.state);
  allocator.deallocate(params_st->params[0].parameter_values[0].bool_array_value, allocator.state);
  params_st->params[0].parameter_values[0].bool_array_value = nullptr;

  // integer array value
  yaml_char_t integer_value[] = "42";
  const size_t integer_value_length = sizeof(integer_value) / sizeof(integer_value[0]);
  event.data.scalar.value = integer_value;
  event.data.scalar.length = integer_value_length;

  // Check bad sequence type for int
  seq_data_type = DATA_TYPE_STRING;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_EQ(nullptr, params_st->params[0].parameter_values[0].integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].integer_array_value);
  EXPECT_EQ(42, params_st->params[0].parameter_values[0].integer_array_value->values[0]);
  allocator.deallocate(
    params_st->params[0].parameter_values[0].integer_array_value->values, allocator.state);
  allocator.deallocate(
    params_st->params[0].parameter_values[0].integer_value, allocator.state);
  params_st->params[0].parameter_values[0].integer_array_value = nullptr;

  // double value
  yaml_char_t double_value[] = "3.14159";
  const size_t double_value_length = sizeof(double_value) / sizeof(double_value[0]);
  event.data.scalar.value = double_value;
  event.data.scalar.length = double_value_length;

  // Check bad sequence type for double
  seq_data_type = DATA_TYPE_STRING;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_EQ(nullptr, params_st->params[0].parameter_values[0].integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].double_array_value);
  EXPECT_EQ(3.14159, params_st->params[0].parameter_values[0].double_array_value->values[0]);
  allocator.deallocate(
    params_st->params[0].parameter_values[0].double_array_value->values, allocator.state);
  allocator.deallocate(
    params_st->params[0].parameter_values[0].double_array_value, allocator.state);
  params_st->params[0].parameter_values[0].double_array_value = nullptr;

  // double value
  yaml_char_t string_value[] = "hello, I am a string";
  const size_t string_value_length = sizeof(string_value) / sizeof(string_value[0]);
  event.data.scalar.value = string_value;
  event.data.scalar.length = string_value_length;

  // Check bad sequence type for string
  seq_data_type = DATA_TYPE_BOOL;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_EQ(nullptr, params_st->params[0].parameter_values[0].integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, params_st->params[0].parameter_values[0].string_array_value);
  EXPECT_STREQ(
    "hello, I am a string",
    params_st->params[0].parameter_values[0].string_array_value->data[0]);
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_string_array_fini(params_st->params[0].parameter_values[0].string_array_value)) <<
    rcutils_get_error_string().str;
  params_st->params[0].parameter_values[0].string_array_value = nullptr;
}

TEST(TestParse, parse_value_bad_args) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yaml_event_t event;
  event.type = YAML_NO_EVENT;

  bool is_seq = false;
  size_t node_idx = 0u;
  size_t parameter_idx = 0u;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, node_idx, parameter_idx, nullptr, params_st));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, nullptr));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // No node to update
  const size_t num_nodes = params_st->num_nodes;
  params_st->num_nodes = 0u;
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
  params_st->num_nodes = num_nodes;

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&params_st->params[0], allocator));
  params_st->num_nodes = 1u;

  // event.data.scaler.value is NULL, but event.data.scalar.length > 0
  event.start_mark = {0u, 0u, 0u};
  event.data.scalar = {NULL, NULL, NULL, 1u, 0, 0, YAML_ANY_SCALAR_STYLE};
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // event.data.scalar.length is 0 and style is not a QUOTED_SCALAR_STYLE
  event.data.scalar.length = 0u;
  yaml_char_t event_value[] = "non_empty_string";
  const size_t event_value_length = sizeof(event_value) / sizeof(event_value[0]);
  event.data.scalar.value = event_value;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // parameter_values is NULL
  event.data.scalar.length = event_value_length;
  rcl_variant_t * tmp_variant_array = params_st->params[0].parameter_values;
  params_st->params[0].parameter_values = NULL;
  EXPECT_EQ(
    RCUTILS_RET_BAD_ALLOC,
    parse_value(event, is_seq, node_idx, parameter_idx, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
  params_st->params[0].parameter_values = tmp_variant_array;
}

TEST(TestParse, parse_key_bad_args)
{
  yaml_event_t event;
  event.type = YAML_NO_EVENT;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  uint32_t map_level = MAP_NODE_NAME_LVL;
  bool is_new_map = false;
  size_t node_idx = 0;
  size_t parameter_idx = 0;
  namespace_tracker_t ns_tracker;

  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&params_st->params[0], allocator));
  params_st->num_nodes = 1u;

  // map_level is nullptr
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(event, nullptr, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // map_level is nullptr
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(event, nullptr, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // params_st is nullptr
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(event, &map_level, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, nullptr)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // event.data.scalar.value is nullptr
  event.data.scalar.value = nullptr;
  event.data.scalar.length = 1u;
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(
      event, &map_level, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  yaml_char_t string_value[] = "key_name";
  const size_t string_value_length = sizeof(string_value) / sizeof(string_value[0]);

  // event.data.scalar.length is 0
  event.data.scalar.value = string_value;
  event.data.scalar.length = 0u;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_key(
      event, &map_level, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
  event.data.scalar.length = string_value_length;

  // map_level is MAP_UNINIT_LVL
  map_level = MAP_UNINIT_LVL;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_key(
      event, &map_level, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // map_level is not a valid value
  map_level = 42;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_key(
      event, &map_level, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // previous parameter names required for parameter namespace
  map_level = MAP_PARAMS_LVL;
  is_new_map = true;
  params_st->params[0].parameter_names[0] = nullptr;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_key(
      event, &map_level, &is_new_map, &node_idx, &parameter_idx, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
}
