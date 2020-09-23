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

#include <string>

#include "performance_test_fixture/performance_test_fixture.hpp"

#include "rcl_yaml_param_parser/parser.h"

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/filesystem.h"

using performance_test_fixture::PerformanceTest;

class PerformanceTestPaserYaml : public performance_test_fixture::PerformanceTest
{
public:
  void InitYamlStructure(const std::string & filename, ::benchmark::State & st)
  {
    rcutils_reset_error();
    allocator = rcutils_get_default_allocator();
    if (!rcutils_get_cwd(cur_dir, 1024)) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    test_path = rcutils_join_path(cur_dir, "test/benchmark", allocator);
    if (test_path == NULL) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    path = rcutils_join_path(test_path, filename.c_str(), allocator);
    if (path == NULL) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    params_hdl = rcl_yaml_node_struct_init(allocator);
    if (NULL == params_hdl) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    params_hdl = rcl_yaml_node_struct_init(allocator);
    if (NULL == params_hdl) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    reset_heap_counters();
  }

  void TearDown(::benchmark::State & st)
  {
    performance_test_fixture::PerformanceTest::TearDown(st);
    allocator.deallocate(test_path, allocator.state);
    allocator.deallocate(path, allocator.state);
    rcl_yaml_node_struct_fini(params_hdl);
  }

  char cur_dir[1024];
  char * path;
  char * test_path;
  rcutils_allocator_t allocator;
  rcl_params_t * params_hdl;
};

BENCHMARK_DEFINE_F(PerformanceTestPaserYaml, parser_yaml_max_params)(benchmark::State & st)
{
  InitYamlStructure("max_num_params.yaml", st);
  for (auto _ : st) {

    bool res = rcl_parse_yaml_file(path, params_hdl);
    if (!res) {
      rcl_yaml_node_struct_fini(params_hdl);
      st.SkipWithError(rcutils_get_error_string().str);
    }
  }
}
BENCHMARK_REGISTER_F(PerformanceTestPaserYaml, parser_yaml_max_params);
