// Copyright 2018 Apex.AI, Inc.
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

#include <stdio.h>
#include <gtest/gtest.h>

#include "rcl_yaml_param_parser/parser.h"

#include "rcutils/error_handling.h"
#include "rcutils/filesystem.h"

static char cur_dir[1024];
rcutils_allocator_t allocator = rcutils_get_default_allocator();

TEST(test_file_parser, correct_syntax) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "correct_config.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_TRUE(res);
  rcl_yaml_node_struct_print(params_hdl);
  rcl_yaml_node_struct_fini(params_hdl);
  free(test_path);
  free(path);
}

TEST(test_file_parser, multi_ns_correct_syntax) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "multi_ns_correct.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_TRUE(res);
  rcl_yaml_node_struct_print(params_hdl);
  rcl_yaml_node_struct_fini(params_hdl);
  free(test_path);
  free(path);
}

TEST(test_file_parser, seq_map1) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "seq_map1.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

TEST(test_file_parser, seq_map2) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "seq_map2.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

TEST(test_file_parser, params_with_no_node) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "params_with_no_node.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

TEST(test_file_parser, no_alias_support) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "no_alias_support.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

TEST(test_file_parser, max_string_sz) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "max_string_sz.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

TEST(test_file_parser, no_value1) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "no_value1.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

TEST(test_file_parser, indented_ns) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * test_path = rcutils_join_path(cur_dir, "test");
  char * path = rcutils_join_path(test_path, "indented_name_space.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  EXPECT_FALSE(NULL == params_hdl);
  bool res = rcl_parse_yaml_file(path, params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free(test_path);
  free(path);
}

int32_t main(int32_t argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
