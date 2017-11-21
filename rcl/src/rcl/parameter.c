// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#if __cplusplus
extern "C"
{
#include <string>
#endif

#include <rcl_interfaces/msg/parameter__functions.h>
#include <rcl_interfaces/msg/parameter__struct.h>
#include <rcl_interfaces/msg/parameter_event__struct.h>
#include <rcl_interfaces/msg/parameter_type__functions.h>

#include <string.h>
#include "rosidl_generator_c/string_functions.h"
#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/types.h"
#include "./common.h"

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_generator_c/service_type_support.h"

#define RCL_DEFINE_SET_PARAMETER(TYPE, CTYPE, ENUM_TYPE) \
  rcl_ret_t \
  rcl_parameter_set_ ## TYPE( \
    rcl_interfaces__msg__Parameter * parameter, const char * parameter_name, CTYPE value) \
  { \
    if (!rosidl_generator_c__String__assign(&parameter->name, parameter_name)) { \
      return RCL_RET_ERROR; \
    } \
    parameter->value.type = rcl_interfaces__msg__ParameterType__ ## ENUM_TYPE; \
    parameter->value.TYPE ## _value = value; \
    return RCL_RET_OK; \
  }

#define RCL_DEFINE_SET_PARAMETER_ARRAY_TYPE(TYPE, ROSIDL_TYPE, CTYPE, ENUM_TYPE) \
  rcl_ret_t \
  rcl_parameter_set_ ## TYPE( \
    rcl_interfaces__msg__Parameter * parameter, const char * parameter_name, const CTYPE * value) \
  { \
    if (!rosidl_generator_c__String__assign(&parameter->name, parameter_name)) { \
      return RCL_RET_ERROR; \
    } \
    parameter->value.type = rcl_interfaces__msg__ParameterType__ ## ENUM_TYPE; \
    ROSIDL_TYPE ## __assign(&parameter->value.TYPE ## _value, value); \
    return RCL_RET_OK; \
  }

RCL_DEFINE_SET_PARAMETER(bool, bool, PARAMETER_BOOL)
RCL_DEFINE_SET_PARAMETER(integer, int, PARAMETER_INTEGER)
RCL_DEFINE_SET_PARAMETER(double, double, PARAMETER_DOUBLE)

RCL_DEFINE_SET_PARAMETER_ARRAY_TYPE(string, rosidl_generator_c__String, char, PARAMETER_STRING)

#define RCL_DEFINE_SET_PARAMETER_VALUE(TYPE, CTYPE, ENUM_TYPE) \
  rcl_ret_t \
  rcl_parameter_set_value_ ## TYPE( \
    rcl_interfaces__msg__ParameterValue * parameter_value, CTYPE value) \
  { \
    parameter_value->type = rcl_interfaces__msg__ParameterType__ ## ENUM_TYPE; \
    parameter_value->TYPE ## _value = value; \
    return RCL_RET_OK; \
  }

#define RCL_DEFINE_SET_PARAMETER_VALUE_ARRAY_TYPE(TYPE, ROSIDL_TYPE, CTYPE, ENUM_TYPE) \
  rcl_ret_t \
  rcl_parameter_set_value_ ## TYPE( \
    rcl_interfaces__msg__ParameterValue * parameter_value, const CTYPE * value) \
  { \
    parameter_value->type = rcl_interfaces__msg__ParameterType__ ## ENUM_TYPE; \
    ROSIDL_TYPE ## __assign(&parameter_value->TYPE ## _value, value); \
    return RCL_RET_OK; \
  }

RCL_DEFINE_SET_PARAMETER_VALUE(bool, bool, PARAMETER_BOOL)
RCL_DEFINE_SET_PARAMETER_VALUE(integer, int, PARAMETER_INTEGER)
RCL_DEFINE_SET_PARAMETER_VALUE(double, double, PARAMETER_DOUBLE)

RCL_DEFINE_SET_PARAMETER_VALUE_ARRAY_TYPE(string, rosidl_generator_c__String, char,
  PARAMETER_STRING)

// Check if two parameters are equal
bool rcl_parameter_value_compare(const rcl_interfaces__msg__ParameterValue * parameter1,
  const rcl_interfaces__msg__ParameterValue * parameter2)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter1, false, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter2, false, rcl_get_default_allocator());
  if (parameter1->type != parameter2->type) {
    return false;
  }
  switch (parameter1->type) {
    case rcl_interfaces__msg__ParameterType__PARAMETER_BOOL:
      return parameter1->bool_value == parameter2->bool_value;
    case rcl_interfaces__msg__ParameterType__PARAMETER_INTEGER:
      return parameter1->integer_value == parameter2->integer_value;
    case rcl_interfaces__msg__ParameterType__PARAMETER_DOUBLE:
      return parameter1->double_value == parameter2->double_value;
    case rcl_interfaces__msg__ParameterType__PARAMETER_STRING:
      return strcmp(parameter1->string_value.data, parameter2->string_value.data) != 0;
    case rcl_interfaces__msg__ParameterType__PARAMETER_BYTES:
      // Not implemented
      break;
    case rcl_interfaces__msg__ParameterType__PARAMETER_NOT_SET:
    default:
      return false;
  }

  return false;
}

rcl_ret_t
rcl_parameter_value_copy(rcl_interfaces__msg__ParameterValue * dst,
  const rcl_interfaces__msg__ParameterValue * src)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(dst, false, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(src, false, rcl_get_default_allocator());

  dst->type = src->type;

  switch (src->type) {
    case rcl_interfaces__msg__ParameterType__PARAMETER_BOOL:
      dst->bool_value = src->bool_value;
      return RCL_RET_OK;
    case rcl_interfaces__msg__ParameterType__PARAMETER_INTEGER:
      dst->integer_value = src->integer_value;
      return RCL_RET_OK;
    case rcl_interfaces__msg__ParameterType__PARAMETER_DOUBLE:
      dst->double_value = src->double_value;
      return RCL_RET_OK;
    case rcl_interfaces__msg__ParameterType__PARAMETER_STRING:
      return rosidl_generator_c__String__assign(
        &dst->string_value, src->string_value.data) ? RCL_RET_OK : RCL_RET_ERROR;
    case rcl_interfaces__msg__ParameterType__PARAMETER_BYTES:
      // Not implemented
      break;
    case rcl_interfaces__msg__ParameterType__PARAMETER_NOT_SET:
    default:
      return RCL_RET_ERROR;
  }
  return RCL_RET_ERROR;
}

rcl_ret_t
rcl_parameter_copy(rcl_interfaces__msg__Parameter * dst,
  const rcl_interfaces__msg__Parameter * src)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(dst, false, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(src, false, rcl_get_default_allocator());
  if (!rosidl_generator_c__String__assign(&dst->name, src->name.data)) {
    return RCL_RET_ERROR;
  }
  return rcl_parameter_value_copy(&dst->value, &src->value);
}

rcl_ret_t
rcl_parameter_convert_changes_to_event(
  const rcl_interfaces__msg__Parameter__Array * prior_state,
  const rcl_interfaces__msg__Parameter__Array * new_state,
  rcl_interfaces__msg__ParameterEvent * parameter_event)
{
  // Diff the prior state and the new state and fill the parameter_event struct
  RCL_CHECK_ARGUMENT_FOR_NULL(prior_state, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(new_state, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());
  RCL_CHECK_ARGUMENT_FOR_NULL(parameter_event, RCL_RET_INVALID_ARGUMENT, rcl_get_default_allocator());

  size_t prior_idx, new_idx;
  size_t num_deleted_params = 0;
  size_t num_changed_params = 0;
  size_t num_new_params = 0;
  // go through and count the numbers we're going to need
  for (prior_idx = 0; prior_idx < prior_state->size; ++prior_idx) {
    rcl_interfaces__msg__Parameter * prior_entry = &prior_state->data[prior_idx];
    for (new_idx = 0; new_idx < new_state->size; ++new_idx) {
      rcl_interfaces__msg__Parameter * new_entry = &new_state->data[new_idx];
      // If the parameter has the same name but a different type or value, count it as changed.
      if (strcmp(prior_entry->name.data, new_entry->name.data) == 0) {
        if (!rcl_parameter_value_compare(&prior_entry->value, &new_entry->value)) {
          num_changed_params++;
        }
        break;
      }
    }
    if (new_idx == new_state->size) {
      // No entry in new_state was found with a matching name to prior_entry (deleted)
      num_deleted_params++;
    }
  }
  // Could optimize this if we allocated some space to store matches from the previous comparisons
  for (new_idx = 0; new_idx < new_state->size; ++new_idx) {
    rcl_interfaces__msg__Parameter * new_entry = &new_state->data[new_idx];
    for (prior_idx = 0; prior_idx < prior_state->size; ++prior_idx) {
      rcl_interfaces__msg__Parameter * prior_entry = &prior_state->data[prior_idx];
      if (strcmp(prior_entry->name.data, new_entry->name.data) == 0) {
        break;
      }
    }
    if (prior_idx == prior_state->size) {
      num_new_params++;
    }
  }

  rcl_interfaces__msg__Parameter__Array__fini(&parameter_event->deleted_parameters);
  rcl_interfaces__msg__Parameter__Array__fini(&parameter_event->changed_parameters);
  rcl_interfaces__msg__Parameter__Array__fini(&parameter_event->new_parameters);

  rcl_interfaces__msg__Parameter__Array__init(&parameter_event->deleted_parameters,
    num_deleted_params);
  rcl_interfaces__msg__Parameter__Array__init(&parameter_event->changed_parameters,
    num_changed_params);
  rcl_interfaces__msg__Parameter__Array__init(&parameter_event->new_parameters, num_new_params);

  size_t new_array_idx = 0;
  size_t deleted_idx = 0;
  size_t changed_idx = 0;

  for (prior_idx = 0; prior_idx < prior_state->size; ++prior_idx) {
    rcl_interfaces__msg__Parameter * prior_entry = &prior_state->data[prior_idx];
    for (new_idx = 0; new_idx < new_state->size; ++new_idx) {
      rcl_interfaces__msg__Parameter * new_entry = &new_state->data[new_idx];
      // If the parameter has the same name but a different type or value, count it as changed.
      if (strcmp(prior_entry->name.data, new_entry->name.data) == 0) {
        if (!rcl_parameter_value_compare(&prior_entry->value, &new_entry->value)) {
          rcl_parameter_copy(&parameter_event->changed_parameters.data[changed_idx++], new_entry);
        }
        break;
      }
    }
    if (new_idx == new_state->size) {
      // If no entry in new_state was found with a matching name to prior_entry, count as deleted.
      rcl_parameter_copy(&parameter_event->deleted_parameters.data[deleted_idx++], prior_entry);
    }
  }
  // Could optimize this if we allocated some space to store matches from the previous comparisons
  for (new_idx = 0; new_idx < new_state->size; ++new_idx) {
    rcl_interfaces__msg__Parameter * new_entry = &new_state->data[new_idx];
    for (prior_idx = 0; prior_idx < prior_state->size; ++prior_idx) {
      rcl_interfaces__msg__Parameter * prior_entry = &prior_state->data[prior_idx];
      if (strcmp(prior_entry->name.data, new_entry->name.data) == 0) {
        break;
      }
    }
    if (prior_idx == prior_state->size) {
      rcl_parameter_copy(&parameter_event->new_parameters.data[new_array_idx++], new_entry);
    }
  }
  parameter_event->changed_parameters.size = num_changed_params;
  parameter_event->deleted_parameters.size = num_deleted_params;
  parameter_event->new_parameters.size = num_new_params;

  return RCL_RET_OK;
}

#if __cplusplus
}
#endif
