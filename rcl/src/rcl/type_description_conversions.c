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

#include "rcl/type_description_conversions.h"

#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/type_description/field__functions.h"
#include "rosidl_runtime_c/type_description/individual_type_description__functions.h"
#include "rosidl_runtime_c/type_description/type_description__functions.h"
#include "rosidl_runtime_c/type_description/type_source__functions.h"
#include "type_description_interfaces/msg/detail/field__functions.h"
#include "type_description_interfaces/msg/individual_type_description.h"

#ifdef __cplusplus
extern "C" {
#endif

static bool individual_type_description_runtime_to_msg(
  const rosidl_runtime_c__type_description__IndividualTypeDescription * in,
  type_description_interfaces__msg__IndividualTypeDescription * out)
{
  if (NULL == in) {
    return false;
  }

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
  if (NULL == in) {
    return false;
  }

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
  const rosidl_runtime_c__type_description__TypeDescription * in)
{
  if (NULL == in) {
    return NULL;
  }

  // Create the object
  type_description_interfaces__msg__TypeDescription * out =
    type_description_interfaces__msg__TypeDescription__create();
  if (NULL == out) {
    return NULL;
  }

  // init referenced_type_descriptions with the correct size
  if (!type_description_interfaces__msg__IndividualTypeDescription__Sequence__init(
      &out->referenced_type_descriptions,
      in->referenced_type_descriptions.size))
  {
    goto fail;
  }

  // Convert individual type description
  if (!individual_type_description_runtime_to_msg(
      &in->type_description,
      &out->type_description))
  {
    goto fail;
  }

  // Convert referenced type descriptions
  for (size_t i = 0; i < in->referenced_type_descriptions.size; ++i) {
    if (!individual_type_description_runtime_to_msg(
        &in->referenced_type_descriptions.data[i],
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
  const type_description_interfaces__msg__TypeDescription * in)
{
  if (NULL == in) {
    return NULL;
  }

  // Create the object
  rosidl_runtime_c__type_description__TypeDescription * out =
    rosidl_runtime_c__type_description__TypeDescription__create();
  if (NULL == out) {
    return NULL;
  }

  // init referenced_type_descriptions with the correct size
  if (!rosidl_runtime_c__type_description__IndividualTypeDescription__Sequence__init(
      &out->referenced_type_descriptions,
      in->referenced_type_descriptions.size))
  {
    goto fail;
  }

  if (!individual_type_description_msg_to_runtime(
      &in->type_description,
      &out->type_description))
  {
    goto fail;
  }

  for (size_t i = 0; i < in->referenced_type_descriptions.size; ++i) {
    if (!individual_type_description_msg_to_runtime(
        &in->referenced_type_descriptions.data[i],
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
  const rosidl_runtime_c__type_description__TypeSource__Sequence * in)
{
  if (NULL == in) {
    return NULL;
  }

  // Create the object
  type_description_interfaces__msg__TypeSource__Sequence * out =
    type_description_interfaces__msg__TypeSource__Sequence__create(in->size);
  if (NULL == out) {
    return NULL;
  }

  // Copy type sources
  for (size_t i = 0; i < in->size; ++i) {
    // type_name
    if (!rosidl_runtime_c__String__copy(
        &(in->data[i].type_name), &(out->data[i].type_name)))
    {
      goto fail;
    }
    // encoding
    if (!rosidl_runtime_c__String__copy(
        &(in->data[i].encoding), &(out->data[i].encoding)))
    {
      goto fail;
    }
    // raw_file_contents
    if (!rosidl_runtime_c__String__copy(
        &(in->data[i].raw_file_contents), &(out->data[i].raw_file_contents)))
    {
      goto fail;
    }
  }

  return out;

fail:
  type_description_interfaces__msg__TypeSource__Sequence__destroy(out);
  return NULL;
}

rosidl_runtime_c__type_description__TypeSource__Sequence *
rcl_convert_type_source_sequence_msg_to_runtime(
  const type_description_interfaces__msg__TypeSource__Sequence * in)
{
  if (NULL == in) {
    return NULL;
  }

  // Create the object
  rosidl_runtime_c__type_description__TypeSource__Sequence * out =
    rosidl_runtime_c__type_description__TypeSource__Sequence__create(in->size);
  if (NULL == out) {
    return NULL;
  }

  // Copy type sources
  for (size_t i = 0; i < in->size; ++i) {
    // type_name
    if (!rosidl_runtime_c__String__copy(
        &(in->data[i].type_name), &(out->data[i].type_name)))
    {
      goto fail;
    }
    // encoding
    if (!rosidl_runtime_c__String__copy(
        &(in->data[i].encoding), &(out->data[i].encoding)))
    {
      goto fail;
    }
    // raw_file_contents
    if (!rosidl_runtime_c__String__copy(
        &(in->data[i].raw_file_contents), &(out->data[i].raw_file_contents)))
    {
      goto fail;
    }
  }

  return out;

fail:
  rosidl_runtime_c__type_description__TypeSource__Sequence__destroy(out);
  return NULL;
}

#ifdef __cplusplus
}
#endif
