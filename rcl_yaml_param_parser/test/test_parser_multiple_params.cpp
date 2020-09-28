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

#include <yaml.h>

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcl_yaml_param_parser/parser.h"
#include "rcutils/error_handling.h"
#include "rcutils/filesystem.h"
#include "rcutils/testing/fault_injection.h"
#include "./mocking_utils/patch.hpp"

static char cur_dir[1024];

TEST(RclYamlParamParserMultipleParams, multiple_params) {
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024)) << rcutils_get_error_string().str;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * test_path = rcutils_join_path(cur_dir, "test", allocator);
  ASSERT_TRUE(NULL != test_path) << rcutils_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(test_path, allocator.state);
  });
  char * path = rcutils_join_path(test_path, "multiple_params.yaml", allocator);
  ASSERT_TRUE(NULL != path) << rcutils_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(path, allocator.state);
  });
  ASSERT_TRUE(rcutils_exists(path)) << "No test YAML file found at " << path;
  rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
  ASSERT_TRUE(NULL != params_hdl) << rcutils_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rcl_yaml_node_struct_fini(params_hdl);
  });
  ASSERT_TRUE(rcl_parse_yaml_file(path, params_hdl));
  size_t node_size;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_hash_map_get_size(&params_hdl->params_map, &node_size)) <<
    rcutils_get_error_string().str;
  ASSERT_EQ(1u, node_size);

  const char * node_name = "foo_ns/foo_name";
  EXPECT_TRUE(rcutils_hash_map_key_exists(&params_hdl->params_map, &node_name));
  rcl_node_params_t * node_param;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_hash_map_get(&params_hdl->params_map, &node_name, &node_param)) <<
    rcutils_get_error_string().str;
  ASSERT_NE(nullptr, node_param);

  size_t param_size;
  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_hash_map_get_size(&node_param->node_params_map, &param_size)) <<
    rcutils_get_error_string().str;
  ASSERT_EQ(513u, param_size);

  for (size_t i = 0; i < param_size; ++i) {
    std::string name = "param" + std::to_string(i + 1);
    const char * param_name = name.c_str();
    rcl_variant_t * param_value = NULL;
    EXPECT_EQ(
      RCUTILS_RET_OK,
      rcutils_hash_map_get(&node_param->node_params_map, &param_name, &param_value)) <<
      rcutils_get_error_string().str;
    EXPECT_EQ(static_cast<int64_t>(i + 1), *param_value->integer_value);
  }
}

TEST(RclYamlParamParserMultipleParams, test_multiple_params_with_bad_allocator) {
  char cur_dir[1024];
  rcutils_reset_error();
  EXPECT_TRUE(rcutils_get_cwd(cur_dir, 1024)) << rcutils_get_error_string().str;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * test_path = rcutils_join_path(cur_dir, "test", allocator);
  ASSERT_TRUE(NULL != test_path) << rcutils_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(test_path, allocator.state);
  });

  std::string filename = "multiple_params.yaml";
  SCOPED_TRACE(filename);
  char * path = rcutils_join_path(test_path, filename.c_str(), allocator);
  ASSERT_TRUE(NULL != path) << rcutils_get_error_string().str;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    allocator.deallocate(path, allocator.state);
  });
  ASSERT_TRUE(rcutils_exists(path)) << "No test YAML file found at " << path;

  RCUTILS_FAULT_INJECTION_TEST(
  {
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rcl_params_t * params_hdl = rcl_yaml_node_struct_init(allocator);
    if (NULL == params_hdl) {
      continue;
    }

    bool res = rcl_parse_yaml_file(path, params_hdl);
    // Not verifying res is true or false here, because eventually it will come back with an ok
    // result. We're just trying to make sure that bad allocations are properly handled
    (void)res;

    // If `rcutils_string_array_fini` fails, there will be a small memory leak here.
    // However, it's necessary for coverage
    rcl_yaml_node_struct_fini(params_hdl);
    params_hdl = NULL;
  });
}

int32_t main(int32_t argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
