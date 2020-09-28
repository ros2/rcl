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
#include "../src/impl/parse.h"
#include "../src/impl/node_params.h"
#include "rcutils/filesystem.h"

#include "./mocking_utils/patch.hpp"

TEST(TestParse, parse_value) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yaml_event_t event;
  event.type = YAML_NO_EVENT;
  event.start_mark = {0u, 0u, 0u};
  event.data.scalar = {NULL, NULL, NULL, 1u, 0, 0, YAML_ANY_SCALAR_STYLE};

  bool is_seq = false;
  rcl_node_params_t node_param;
  const char * parameter_name = "test";
  rcl_variant_t * parameter_value = NULL;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&node_param, allocator));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_params_fini(&node_param, allocator);
  });

  // bool value
  yaml_char_t bool_value[] = "true";
  const size_t bool_value_length = sizeof(bool_value) / sizeof(bool_value[0]);
  event.data.scalar.value = bool_value;
  event.data.scalar.length = bool_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_hash_map_get(&node_param.node_params_map, &parameter_name, &parameter_value)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, parameter_value->bool_value);
  EXPECT_TRUE(*parameter_value->bool_value);
  allocator.deallocate(parameter_value->bool_value, allocator.state);
  parameter_value->bool_value = nullptr;

  // integer value
  yaml_char_t integer_value[] = "42";
  const size_t integer_value_length = sizeof(integer_value) / sizeof(integer_value[0]);
  event.data.scalar.value = integer_value;
  event.data.scalar.length = integer_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, parameter_value->integer_value);
  EXPECT_EQ(42, *parameter_value->integer_value);
  allocator.deallocate(parameter_value->integer_value, allocator.state);
  parameter_value->integer_value = nullptr;

  // double value
  yaml_char_t double_value[] = "3.14159";
  const size_t double_value_length = sizeof(double_value) / sizeof(double_value[0]);
  event.data.scalar.value = double_value;
  event.data.scalar.length = double_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, parameter_value->double_value);
  EXPECT_EQ(3.14159, *parameter_value->double_value);
  allocator.deallocate(
    parameter_value->double_value, allocator.state);
  parameter_value->double_value = nullptr;

  // double value
  yaml_char_t string_value[] = "hello, I am a string";
  const size_t string_value_length = sizeof(string_value) / sizeof(string_value[0]);
  event.data.scalar.value = string_value;
  event.data.scalar.length = string_value_length;

  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, parameter_value->string_value);
  EXPECT_STREQ(
    "hello, I am a string",
    parameter_value->string_value);
  allocator.deallocate(
    parameter_value->string_value, allocator.state);
  parameter_value->string_value = nullptr;
}

TEST(TestParse, parse_value_sequence) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yaml_event_t event;
  event.type = YAML_NO_EVENT;
  event.start_mark = {0u, 0u, 0u};
  event.data.scalar = {NULL, NULL, NULL, 1u, 0, 0, YAML_ANY_SCALAR_STYLE};

  bool is_seq = true;
  rcl_node_params_t node_param;
  const char * parameter_name = "test";
  rcl_variant_t * parameter_value = NULL;
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&node_param, allocator));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_params_fini(&node_param, allocator);
  });

  // bool array value
  yaml_char_t bool_value[] = "true";
  const size_t bool_value_length = sizeof(bool_value) / sizeof(bool_value[0]);
  event.data.scalar.value = bool_value;
  event.data.scalar.length = bool_value_length;

  // Check bad sequence type for bool
  seq_data_type = DATA_TYPE_STRING;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_hash_map_get(&node_param.node_params_map, &parameter_name, &parameter_value)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, parameter_value);
  EXPECT_EQ(
    nullptr,
    parameter_value->integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(
    nullptr, parameter_value->bool_array_value);
  EXPECT_TRUE(
    parameter_value->bool_array_value->values[0]);
  allocator.deallocate(
    parameter_value->bool_array_value->values,
    allocator.state);
  allocator.deallocate(
    parameter_value->bool_array_value, allocator.state);
  parameter_value->bool_array_value = nullptr;

  // integer array value
  yaml_char_t integer_value[] = "42";
  const size_t integer_value_length = sizeof(integer_value) / sizeof(integer_value[0]);
  event.data.scalar.value = integer_value;
  event.data.scalar.length = integer_value_length;

  // Check bad sequence type for int
  seq_data_type = DATA_TYPE_STRING;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(
    nullptr, parameter_value->integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(
    nullptr,
    parameter_value->integer_array_value);
  EXPECT_EQ(
    42,
    parameter_value->integer_array_value->values[0]);
  allocator.deallocate(
    parameter_value->integer_array_value->values,
    allocator.state);
  allocator.deallocate(
    parameter_value->integer_array_value,
    allocator.state);
  parameter_value->integer_array_value = nullptr;

  // double value
  yaml_char_t double_value[] = "3.14159";
  const size_t double_value_length = sizeof(double_value) / sizeof(double_value[0]);
  event.data.scalar.value = double_value;
  event.data.scalar.length = double_value_length;

  // Check bad sequence type for double
  seq_data_type = DATA_TYPE_STRING;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(
    nullptr, parameter_value->integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(
    nullptr, parameter_value->double_array_value);
  EXPECT_EQ(
    3.14159,
    parameter_value->double_array_value->values[0]);
  allocator.deallocate(
    parameter_value->double_array_value->values,
    allocator.state);
  allocator.deallocate(
    parameter_value->double_array_value,
    allocator.state);
  parameter_value->double_array_value = nullptr;

  // double value
  yaml_char_t string_value[] = "hello, I am a string";
  const size_t string_value_length = sizeof(string_value) / sizeof(string_value[0]);
  event.data.scalar.value = string_value;
  event.data.scalar.length = string_value_length;

  // Check bad sequence type for string
  seq_data_type = DATA_TYPE_BOOL;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  rcutils_reset_error();
  EXPECT_EQ(
    nullptr, parameter_value->integer_array_value);

  // Check proper sequence type
  seq_data_type = DATA_TYPE_UNKNOWN;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(
    nullptr, parameter_value->string_array_value);
  EXPECT_STREQ(
    "hello, I am a string",
    parameter_value->string_array_value->data[0]);
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_string_array_fini(
      parameter_value->string_array_value)) <<
    rcutils_get_error_string().str;
  allocator.deallocate(
    parameter_value->string_array_value,
    allocator.state);
  parameter_value->string_array_value = nullptr;
}

TEST(TestParse, parse_value_bad_args) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yaml_event_t event;
  event.type = YAML_NO_EVENT;
  event.start_mark = {0u, 0u, 0u};
  event.data.scalar = {NULL, NULL, NULL, 1u, 0, 0, YAML_ANY_SCALAR_STYLE};

  bool is_seq = false;
  rcl_node_params_t node_param;
  const char * parameter_name = "test";
  data_types_t seq_data_type = DATA_TYPE_UNKNOWN;
  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, &node_param, parameter_name, nullptr, params_st));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, nullptr));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&node_param, allocator));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_params_fini(&node_param, allocator);
  });

  // event.data.scalar.value is NULL, but event.data.scalar.length > 0
  event.data.scalar.value = NULL;
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st));
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // event.data.scalar.length is 0 and style is not a QUOTED_SCALAR_STYLE
  event.data.scalar.length = 0u;
  yaml_char_t event_value[] = "non_empty_string";
//  const size_t event_value_length = sizeof(event_value) / sizeof(event_value[0]);
  event.data.scalar.value = event_value;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_value(event, is_seq, &node_param, parameter_name, &seq_data_type, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
}

TEST(TestParse, parse_key_bad_args)
{
  yaml_event_t event;
  event.type = YAML_NO_EVENT;
  event.start_mark = {0u, 0u, 0u};

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  uint32_t map_level = MAP_NODE_NAME_LVL;
  bool is_new_map = false;
  rcl_node_params_t node_param;
  rcl_node_params_t * node_params_st = NULL;
  char * parameter_name = NULL;
  namespace_tracker_t ns_tracker;

  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_st);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });

  ASSERT_EQ(RCUTILS_RET_OK, node_params_init(&node_param, allocator));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_params_fini(&node_param, allocator);
  });

  // map_level is nullptr
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(
      event, nullptr, &is_new_map, &node_params_st, &parameter_name, &ns_tracker,
      params_st)) << rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // params_st is nullptr
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(
      event, &map_level, &is_new_map, &node_params_st, &parameter_name, &ns_tracker,
      nullptr)) << rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // event.data.scalar.value is nullptr
  event.data.scalar.value = nullptr;
  event.data.scalar.length = 1u;
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    parse_key(
      event, &map_level, &is_new_map, &node_params_st, &parameter_name, &ns_tracker, params_st)) <<
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
      event, &map_level, &is_new_map, &node_params_st, &parameter_name, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
  event.data.scalar.length = string_value_length;

  // map_level is MAP_UNINIT_LVL
  map_level = MAP_UNINIT_LVL;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_key(
      event, &map_level, &is_new_map, &node_params_st, &parameter_name, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();

  // map_level is not a valid value
  map_level = 42;
  EXPECT_EQ(
    RCUTILS_RET_ERROR,
    parse_key(
      event, &map_level, &is_new_map, &node_params_st, &parameter_name, &ns_tracker, params_st)) <<
    rcutils_get_error_string().str;
  EXPECT_TRUE(rcutils_error_is_set());
  rcutils_reset_error();
}

TEST(TestParse, parse_file_events_mock_yaml_parser_parse) {
  char cur_dir[1024];
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024)) << rcutils_get_error_string().str;

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * test_path = rcutils_join_path(cur_dir, "test", allocator);
  char * path = rcutils_join_path(test_path, "correct_config.yaml", allocator);
  ASSERT_TRUE(NULL != path) << rcutils_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(test_path, allocator.state);
    allocator.deallocate(path, allocator.state);
  });

  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(nullptr, params_hdl);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_hdl);
  });

  yaml_parser_t parser;
  ASSERT_NE(0, yaml_parser_initialize(&parser));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    yaml_parser_delete(&parser);
  });

  FILE * yaml_file = fopen(path, "r");
  ASSERT_NE(nullptr, yaml_file);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    fclose(yaml_file);
  });
  yaml_parser_set_input_file(&parser, yaml_file);

  namespace_tracker_t ns_tracker;
  memset(&ns_tracker, 0, sizeof(namespace_tracker_t));

  auto mock = mocking_utils::patch(
    "lib:rcl_yaml_param_parser", yaml_parser_parse, [](yaml_parser_t *, yaml_event_t * event) {
      event->start_mark.line = 0u;
      event->type = YAML_NO_EVENT;
      return 1;
    });
  EXPECT_EQ(RCUTILS_RET_ERROR, parse_file_events(&parser, &ns_tracker, params_hdl));
}

TEST(TestParse, parse_value_events_mock_yaml_parser_parse) {
  constexpr char node_name[] = "node name";
  constexpr char param_name[] = "param name";
  constexpr char yaml_value[] = "true";
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rcl_params_t * params_st = rcl_yaml_node_struct_init(allocator);
  ASSERT_NE(params_st, nullptr);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_st);
  });
  auto mock = mocking_utils::patch(
    "lib:rcl_yaml_param_parser", yaml_parser_parse, [](yaml_parser_t *, yaml_event_t * event) {
      event->start_mark.line = 0u;
      event->type = YAML_NO_EVENT;
      return 1;
    });
  EXPECT_FALSE(rcl_parse_yaml_value(node_name, param_name, yaml_value, params_st));
}
