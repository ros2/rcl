// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include "rcutils_yaml_param_parser/parser.h"
#include "rcutils/error_handling.h"
#include "rcutils/filesystem.h"

static char cur_dir[1024];

rcutils_allocator_t allocator = rcutils_get_default_allocator();

TEST(test_file_parser, correct_syntax) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "correct_config.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_TRUE(res);
  print_node_struct(&params_hdl);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, indented_ns) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "indented_name_space.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, invalid_map1) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "invalid_map1.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, invalid_map2) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "invalid_map2.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, seq_map1) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "seq_map1.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, seq_map2) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "seq_map2.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, no_alias_support) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "no_alias_support.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, max_string_sz) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "max_string_sz.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

TEST(test_file_parser, no_value1) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024));
  char * path = rcutils_join_path(cur_dir, "test");
  path = rcutils_join_path(path, "no_value1.yaml");
  fprintf(stderr, "cur_path: %s\n", path);
  EXPECT_TRUE(rcutils_exists(path));
  params_t params_hdl;
  bool res = parse_yaml_file(path, &params_hdl);
  fprintf(stderr, "%s\n", rcutils_get_error_string_safe());
  EXPECT_FALSE(res);
  free_node_struct(&params_hdl, allocator);
}

int32_t main(int32_t argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
