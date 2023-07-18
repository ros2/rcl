// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#include "rcl/error_handling.h"
#include "rcl/type_description_conversions.h"

#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/type_description/field__functions.h"
#include "rosidl_runtime_c/type_description/individual_type_description__functions.h"
#include "rosidl_runtime_c/type_description/type_description__functions.h"
#include "rosidl_runtime_c/type_description/type_source__functions.h"
#include "type_description_interfaces/msg/detail/field__functions.h"
#include "type_description_interfaces/msg/individual_type_description.h"

static bool individual_type_description_runtime_to_msg(
  const rosidl_runtime_c__type_description__IndividualTypeDescription * in,
  type_description_interfaces__msg__IndividualTypeDescription * out)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(in, false);
  RCL_CHECK_ARGUMENT_FOR_NULL(out, false);

  const bool success =
    rosidl_runtime_c__String__copy(&in->type_name, &out->type_name) &&
    type_description_interfaces__msg__Field__Sequence__init(
    &out->fields,
    in->fields.size);
  if (!success) {
    goto error;
  }

  for (size_t i = 0; i < in->fields.size; ++i) {
    if (!rosidl_runtime_c__String__copy(
        &(in->fields.data[i].name),
        &(out->fields.data[i].name)))
    {
      goto error;
    }

    if (in->fields.data[i].default_value.size) {
      if (!rosidl_runtime_c__String__copy(
          &(in->fields.data[i].default_value),
          &(out->fields.data[i].default_value)))
      {
        goto error;
      }
    }

    // type_id
    out->fields.data[i].type.type_id = in->fields.data[i].type.type_id;
    // capacity
    out->fields.data[i].type.capacity = in->fields.data[i].type.capacity;
    // string_capacity
    out->fields.data[i].type.string_capacity =
      in->fields.data[i].type.string_capacity;

    // nested_type_name
    if (in->fields.data[i].type.nested_type_name.size) {
      if (!rosidl_runtime_c__String__copy(
          &(in->fields.data[i].type.nested_type_name),
          &(out->fields.data[i].type.nested_type_name)))
      {
        goto error;
      }
    }
  }

  return true;

error:
  type_description_interfaces__msg__IndividualTypeDescription__fini(out);
  return false;
}

static bool individual_type_description_msg_to_runtime(
  const type_description_interfaces__msg__IndividualTypeDescription * in,
  rosidl_runtime_c__type_description__IndividualTypeDescription * out)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(in, false);
  RCL_CHECK_ARGUMENT_FOR_NULL(out, false);

  const bool success =
    rosidl_runtime_c__String__copy(&in->type_name, &out->type_name) &&
    rosidl_runtime_c__type_description__Field__Sequence__init(
    &out->fields, in->fields.size);
  if (!success) {
    goto error;
  }

  for (size_t i = 0; i < in->fields.size; ++i) {
    if (!rosidl_runtime_c__String__copy(
        &(in->fields.data[i].name),
        &(out->fields.data[i].name)))
    {
      goto error;
    }

    if (in->fields.data[i].default_value.size) {
      if (!rosidl_runtime_c__String__copy(
          &(in->fields.data[i].default_value),
          &(out->fields.data[i].default_value)))
      {
        goto error;
      }
    }

    // type_id
    out->fields.data[i].type.type_id = in->fields.data[i].type.type_id;
    // capacity
    out->fields.data[i].type.capacity = in->fields.data[i].type.capacity;
    // string_capacity
    out->fields.data[i].type.string_capacity =
      in->fields.data[i].type.string_capacity;

    // nested_type_name
    if (in->fields.data[i].type.nested_type_name.size) {
      if (!rosidl_runtime_c__String__copy(
          &(in->fields.data[i].type.nested_type_name),
          &(out->fields.data[i].type.nested_type_name)))
      {
        goto error;
      }
    }
  }

  return true;

error:
  rosidl_runtime_c__type_description__IndividualTypeDescription__init(out);
  return false;
}

type_description_interfaces__msg__TypeDescription *
rcl_convert_type_description_runtime_to_msg(
  const rosidl_runtime_c__type_description__TypeDescription * runtime_description)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(runtime_description, NULL);

  // Create the object
  type_description_interfaces__msg__TypeDescription * out =
    type_description_interfaces__msg__TypeDescription__create();
  RCL_CHECK_ARGUMENT_FOR_NULL(out, NULL);

  // init referenced_type_descriptions with the correct size
  if (!type_description_interfaces__msg__IndividualTypeDescription__Sequence__init(
      &out->referenced_type_descriptions,
      runtime_description->referenced_type_descriptions.size))
  {
    goto fail;
  }

  // Convert individual type description
  if (!individual_type_description_runtime_to_msg(
      &runtime_description->type_description,
      &out->type_description))
  {
    goto fail;
  }

  // Convert referenced type descriptions
  for (size_t i = 0; i < runtime_description->referenced_type_descriptions.size; ++i) {
    if (!individual_type_description_runtime_to_msg(
        &runtime_description->referenced_type_descriptions.data[i],
        &out->referenced_type_descriptions.data[i]))
    {
      goto fail;
    }
  }

  return out;

fail:
  type_description_interfaces__msg__TypeDescription__destroy(out);
  return NULL;
}

rosidl_runtime_c__type_description__TypeDescription *
rcl_convert_type_description_msg_to_runtime(
  const type_description_interfaces__msg__TypeDescription * description_msg)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(description_msg, NULL);

  // Create the object
  rosidl_runtime_c__type_description__TypeDescription * out =
    rosidl_runtime_c__type_description__TypeDescription__create();
  RCL_CHECK_ARGUMENT_FOR_NULL(out, NULL);

  // init referenced_type_descriptions with the correct size
  if (!rosidl_runtime_c__type_description__IndividualTypeDescription__Sequence__init(
      &out->referenced_type_descriptions,
      description_msg->referenced_type_descriptions.size))
  {
    goto fail;
  }

  if (!individual_type_description_msg_to_runtime(
      &description_msg->type_description,
      &out->type_description))
  {
    goto fail;
  }

  for (size_t i = 0; i < description_msg->referenced_type_descriptions.size; ++i) {
    if (!individual_type_description_msg_to_runtime(
        &description_msg->referenced_type_descriptions.data[i],
        &out->referenced_type_descriptions.data[i]))
    {
      goto fail;
    }
  }

  return out;

fail:
  rosidl_runtime_c__type_description__TypeDescription__destroy(out);
  return NULL;
}

type_description_interfaces__msg__TypeSource__Sequence *
rcl_convert_type_source_sequence_runtime_to_msg(
  const rosidl_runtime_c__type_description__TypeSource__Sequence * runtime_type_sources)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(runtime_type_sources, NULL);

  // Create the object
  type_description_interfaces__msg__TypeSource__Sequence * out =
    type_description_interfaces__msg__TypeSource__Sequence__create(runtime_type_sources->size);
  RCL_CHECK_ARGUMENT_FOR_NULL(out, NULL);

  // Copy type sources
  for (size_t i = 0; i < runtime_type_sources->size; ++i) {
    // type_name
    if (!rosidl_runtime_c__String__copy(
        &(runtime_type_sources->data[i].type_name), &(out->data[i].type_name)))
    {
      goto fail;
    }
    // encoding
    if (!rosidl_runtime_c__String__copy(
        &(runtime_type_sources->data[i].encoding), &(out->data[i].encoding)))
    {
      goto fail;
    }
    // raw_file_contents
    if (runtime_type_sources->data[i].raw_file_contents.size > 0) {
      if (!rosidl_runtime_c__String__copy(
          &(runtime_type_sources->data[i].raw_file_contents), &(out->data[i].raw_file_contents)))
      {
        goto fail;
      }
    }
  }

  return out;

fail:
  type_description_interfaces__msg__TypeSource__Sequence__destroy(out);
  return NULL;
}

rosidl_runtime_c__type_description__TypeSource__Sequence *
rcl_convert_type_source_sequence_msg_to_runtime(
  const type_description_interfaces__msg__TypeSource__Sequence * type_sources_msg)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(type_sources_msg, NULL);

  // Create the object
  rosidl_runtime_c__type_description__TypeSource__Sequence * out =
    rosidl_runtime_c__type_description__TypeSource__Sequence__create(type_sources_msg->size);
  RCL_CHECK_ARGUMENT_FOR_NULL(out, NULL);

  // Copy type sources
  for (size_t i = 0; i < type_sources_msg->size; ++i) {
    // type_name
    if (!rosidl_runtime_c__String__copy(
        &(type_sources_msg->data[i].type_name), &(out->data[i].type_name)))
    {
      goto fail;
    }
    // encoding
    if (!rosidl_runtime_c__String__copy(
        &(type_sources_msg->data[i].encoding), &(out->data[i].encoding)))
    {
      goto fail;
    }
    // raw_file_contents
    if (!rosidl_runtime_c__String__copy(
        &(type_sources_msg->data[i].raw_file_contents), &(out->data[i].raw_file_contents)))
    {
      goto fail;
    }
  }

  return out;

fail:
  rosidl_runtime_c__type_description__TypeSource__Sequence__destroy(out);
  return NULL;
}
