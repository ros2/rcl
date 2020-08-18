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

#include "rcutils/allocator.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"

#include "rcl_yaml_param_parser/impl/types.h"
#include "rcl_yaml_param_parser/impl/yaml_variant.h"
#include "rcl_yaml_param_parser/types.h"

void rcl_yaml_variant_fini(
  rcl_variant_t * param_var,
  const rcutils_allocator_t allocator)
{
  if (NULL == param_var) {
    return;
  }

  if (NULL != param_var->bool_value) {
    allocator.deallocate(param_var->bool_value, allocator.state);
    param_var->bool_value = NULL;
  } else if (NULL != param_var->integer_value) {
    allocator.deallocate(param_var->integer_value, allocator.state);
    param_var->integer_value = NULL;
  } else if (NULL != param_var->double_value) {
    allocator.deallocate(param_var->double_value, allocator.state);
    param_var->double_value = NULL;
  } else if (NULL != param_var->string_value) {
    allocator.deallocate(param_var->string_value, allocator.state);
    param_var->string_value = NULL;
  } else if (NULL != param_var->bool_array_value) {
    if (NULL != param_var->bool_array_value->values) {
      allocator.deallocate(param_var->bool_array_value->values, allocator.state);
    }
    allocator.deallocate(param_var->bool_array_value, allocator.state);
    param_var->bool_array_value = NULL;
  } else if (NULL != param_var->integer_array_value) {
    if (NULL != param_var->integer_array_value->values) {
      allocator.deallocate(param_var->integer_array_value->values, allocator.state);
    }
    allocator.deallocate(param_var->integer_array_value, allocator.state);
    param_var->integer_array_value = NULL;
  } else if (NULL != param_var->double_array_value) {
    if (NULL != param_var->double_array_value->values) {
      allocator.deallocate(param_var->double_array_value->values, allocator.state);
    }
    allocator.deallocate(param_var->double_array_value, allocator.state);
    param_var->double_array_value = NULL;
  } else if (NULL != param_var->string_array_value) {
    if (RCUTILS_RET_OK != rcutils_string_array_fini(param_var->string_array_value)) {
      // Log and continue ...
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error deallocating string array");
    }
    allocator.deallocate(param_var->string_array_value, allocator.state);
    param_var->string_array_value = NULL;
  } else {
    /// Nothing to do to keep pclint happy
  }
}
