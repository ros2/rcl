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

#include "rcpputils/filesystem_helper.hpp"

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
    path = rcpputils::fs::current_path().string() + "/test/benchmark/" + filename.c_str();
    if (path.empty()) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    params_hdl = rcl_yaml_node_struct_init(rcutils_get_default_allocator());
    if (NULL == params_hdl) {
      st.SkipWithError(rcutils_get_error_string().str);
    }
    reset_heap_counters();
  }

  void TearDown(::benchmark::State & st)
  {
    performance_test_fixture::PerformanceTest::TearDown(st);
    rcl_yaml_node_struct_fini(params_hdl);
  }
  std::string path;
  rcl_params_t * params_hdl;
};

BENCHMARK_F(PerformanceTestPaserYaml, parser_yaml_max_params)(benchmark::State & st)
{
  InitYamlStructure("max_num_params.yaml", st);
  for (auto _ : st) {
    bool res = rcl_parse_yaml_file(path.c_str(), params_hdl);
    if (!res) {
      rcl_yaml_node_struct_fini(params_hdl);
      st.SkipWithError(rcutils_get_error_string().str);
    }
  }
}
