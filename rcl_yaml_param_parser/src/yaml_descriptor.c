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

#include "./impl/types.h"
#include "./impl/yaml_descriptor.h"
#include "rcl_yaml_param_parser/types.h"

void rcl_yaml_descriptor_fini(
  rcl_param_descriptor_t * param_descriptor,
  const rcutils_allocator_t allocator)
{
  if (NULL == param_descriptor) {
    return;
  }

  if (NULL != param_descriptor->name) {
    allocator.deallocate(param_descriptor->name, allocator.state);
    param_descriptor->name = NULL;
  }
  if (NULL != param_descriptor->read_only) {
    allocator.deallocate(param_descriptor->read_only, allocator.state);
    param_descriptor->read_only = NULL;
  }
  if (NULL != param_descriptor->type) {
    allocator.deallocate(param_descriptor->type, allocator.state);
    param_descriptor->type = NULL;
  }
  if (NULL != param_descriptor->description) {
    allocator.deallocate(param_descriptor->description, allocator.state);
    param_descriptor->description = NULL;
  }
  if (NULL != param_descriptor->additional_constraints) {
    allocator.deallocate(param_descriptor->additional_constraints, allocator.state);
    param_descriptor->additional_constraints = NULL;
  }
  if (NULL != param_descriptor->min_value_double) {
    allocator.deallocate(param_descriptor->min_value_double, allocator.state);
    param_descriptor->min_value_double = NULL;
  }
  if (NULL != param_descriptor->max_value_double) {
    allocator.deallocate(param_descriptor->max_value_double, allocator.state);
    param_descriptor->max_value_double = NULL;
  }
  if (NULL != param_descriptor->step_double) {
    allocator.deallocate(param_descriptor->step_double, allocator.state);
    param_descriptor->step_double = NULL;
  }
  if (NULL != param_descriptor->min_value_int) {
    allocator.deallocate(param_descriptor->min_value_int, allocator.state);
    param_descriptor->min_value_int = NULL;
  }
  if (NULL != param_descriptor->max_value_int) {
    allocator.deallocate(param_descriptor->max_value_int, allocator.state);
    param_descriptor->max_value_int = NULL;
  }
  if (NULL != param_descriptor->step_int) {
    allocator.deallocate(param_descriptor->step_int, allocator.state);
    param_descriptor->step_int = NULL;
  }
}

bool rcl_yaml_descriptor_copy(
  rcl_param_descriptor_t * out_param_descriptor, const rcl_param_descriptor_t * param_descriptor, rcutils_allocator_t allocator)
{
  if (NULL == param_descriptor || NULL == out_param_descriptor) {
    return false;
  }
  if (NULL != param_descriptor->name) {
    out_param_descriptor->name =
      rcutils_strdup(param_descriptor->name, allocator);
    if (NULL == out_param_descriptor->name) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
  }
  if (NULL != param_descriptor->description) {
    out_param_descriptor->description =
      rcutils_strdup(param_descriptor->description, allocator);
    if (NULL == out_param_descriptor->description) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
  }
  if (NULL != param_descriptor->additional_constraints) {
    out_param_descriptor->additional_constraints =
      rcutils_strdup(param_descriptor->additional_constraints, allocator);
    if (NULL == out_param_descriptor->additional_constraints) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
  }
  if (NULL != param_descriptor->type) {
    out_param_descriptor->type = allocator.allocate(sizeof(uint8_t), allocator.state);
    if (NULL == out_param_descriptor->type) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->type) = *(param_descriptor->type);
  }
  if (NULL != param_descriptor->min_value_int) {
    out_param_descriptor->min_value_int = allocator.allocate(sizeof(int64_t), allocator.state);
    if (NULL == out_param_descriptor->min_value_int) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->min_value_int) = *(param_descriptor->min_value_int);
  }
  if (NULL != param_descriptor->min_value_double) {
    out_param_descriptor->min_value_double =
      allocator.allocate(sizeof(double), allocator.state);
    if (NULL == out_param_descriptor->min_value_double) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->min_value_double) = *(param_descriptor->min_value_double);
  }
  if (NULL != param_descriptor->max_value_int) {
    out_param_descriptor->max_value_int = allocator.allocate(sizeof(int64_t), allocator.state);
    if (NULL == out_param_descriptor->max_value_int) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->max_value_int) = *(param_descriptor->max_value_int);
  }
  if (NULL != param_descriptor->max_value_double) {
    out_param_descriptor->max_value_double =
      allocator.allocate(sizeof(double), allocator.state);
    if (NULL == out_param_descriptor->max_value_double) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->max_value_double) = *(param_descriptor->max_value_double);
  }
  if (NULL != param_descriptor->step_int) {
    out_param_descriptor->step_int = allocator.allocate(sizeof(int64_t), allocator.state);
    if (NULL == out_param_descriptor->step_int) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->step_int) = *(param_descriptor->step_int);
  }
  if (NULL != param_descriptor->step_double) {
    out_param_descriptor->step_double = allocator.allocate(sizeof(double), allocator.state);
    if (NULL == out_param_descriptor->step_double) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->step_double) = *(param_descriptor->step_double);
  }
  if (NULL != param_descriptor->read_only) {
    out_param_descriptor->read_only = allocator.allocate(sizeof(bool), allocator.state);
    if (NULL == out_param_descriptor->read_only) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Error allocating mem");
      return false;
    }
    *(out_param_descriptor->read_only) = *(param_descriptor->read_only);
  }
  return true;
}
