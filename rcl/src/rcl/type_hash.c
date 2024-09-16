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

#include <stdio.h>

#include <yaml.h>

#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/type_hash.h"
#include "rcutils/types/char_array.h"
#include "rcutils/sha256.h"
#include "type_description_interfaces/msg/type_description.h"

#include "./common.h"

static int yaml_write_handler(void * ext, uint8_t * buffer, size_t size)
{
  rcutils_char_array_t * repr = (rcutils_char_array_t *)ext;
  rcutils_ret_t res = rcutils_char_array_strncat(repr, (char *)buffer, size);
  return res == RCL_RET_OK ? 1 : 0;
}

static inline int start_sequence(yaml_emitter_t * emitter)
{
  yaml_event_t event;
  return
    yaml_sequence_start_event_initialize(&event, NULL, NULL, 1, YAML_FLOW_SEQUENCE_STYLE) &&
    yaml_emitter_emit(emitter, &event);
}

static inline int end_sequence(yaml_emitter_t * emitter)
{
  yaml_event_t event;
  return
    yaml_sequence_end_event_initialize(&event) &&
    yaml_emitter_emit(emitter, &event);
}

static inline int start_mapping(yaml_emitter_t * emitter)
{
  yaml_event_t event;
  return
    yaml_mapping_start_event_initialize(&event, NULL, NULL, 1, YAML_FLOW_MAPPING_STYLE) &&
    yaml_emitter_emit(emitter, &event);
}

static inline int end_mapping(yaml_emitter_t * emitter)
{
  yaml_event_t event;
  return
    yaml_mapping_end_event_initialize(&event) &&
    yaml_emitter_emit(emitter, &event);
}

static int emit_key(yaml_emitter_t * emitter, const char * key)
{
  yaml_event_t event;
  return
    yaml_scalar_event_initialize(
    &event, NULL, NULL,
    (yaml_char_t *)key, (int)strlen(key),
    0, 1, YAML_DOUBLE_QUOTED_SCALAR_STYLE) &&
    yaml_emitter_emit(emitter, &event);
}

static int emit_int(yaml_emitter_t * emitter, size_t val, const char * fmt)
{
  // longest uint64 is 20 decimal digits, plus one byte for trailing \0
  char decimal_buf[21];
  yaml_event_t event;
  int ret = snprintf(decimal_buf, sizeof(decimal_buf), fmt, val);
  if (ret < 0) {
    emitter->problem = "Failed expanding integer";
    return 0;
  }
  if ((size_t)ret >= sizeof(decimal_buf)) {
    emitter->problem = "Decimal buffer overflow";
    return 0;
  }
  return
    yaml_scalar_event_initialize(
    &event, NULL, NULL,
    (yaml_char_t *)decimal_buf, (int)strlen(decimal_buf),
    1, 0, YAML_PLAIN_SCALAR_STYLE) &&
    yaml_emitter_emit(emitter, &event);
}

static int emit_str(yaml_emitter_t * emitter, const rosidl_runtime_c__String * val)
{
  yaml_event_t event;
  return
    yaml_scalar_event_initialize(
    &event, NULL, NULL,
    (yaml_char_t *)val->data, (int)val->size,
    0, 1, YAML_DOUBLE_QUOTED_SCALAR_STYLE) &&
    yaml_emitter_emit(emitter, &event);
}

static int emit_field_type(
  yaml_emitter_t * emitter,
  const type_description_interfaces__msg__FieldType * field_type)
{
  return
    start_mapping(emitter) &&

    emit_key(emitter, "type_id") &&
    emit_int(emitter, field_type->type_id, "%d") &&

    emit_key(emitter, "capacity") &&
    emit_int(emitter, field_type->capacity, "%zu") &&

    emit_key(emitter, "string_capacity") &&
    emit_int(emitter, field_type->string_capacity, "%zu") &&

    emit_key(emitter, "nested_type_name") &&
    emit_str(emitter, &field_type->nested_type_name) &&

    end_mapping(emitter);
}

static int emit_field(
  yaml_emitter_t * emitter,
  const type_description_interfaces__msg__Field * field)
{
  return
    start_mapping(emitter) &&

    emit_key(emitter, "name") &&
    emit_str(emitter, &field->name) &&

    emit_key(emitter, "type") &&
    emit_field_type(emitter, &field->type) &&

    end_mapping(emitter);
}

static int emit_individual_type_description(
  yaml_emitter_t * emitter,
  const type_description_interfaces__msg__IndividualTypeDescription * individual_type_description)
{
  if (!(
      start_mapping(emitter) &&

      emit_key(emitter, "type_name") &&
      emit_str(emitter, &individual_type_description->type_name) &&

      emit_key(emitter, "fields") &&
      start_sequence(emitter)))
  {
    return 0;
  }
  for (size_t i = 0; i < individual_type_description->fields.size; i++) {
    if (!emit_field(emitter, &individual_type_description->fields.data[i])) {
      return 0;
    }
  }
  return end_sequence(emitter) && end_mapping(emitter);
}

static int emit_type_description(
  yaml_emitter_t * emitter,
  const type_description_interfaces__msg__TypeDescription * type_description)
{
  if (!(
      start_mapping(emitter) &&

      emit_key(emitter, "type_description") &&
      emit_individual_type_description(emitter, &type_description->type_description) &&

      emit_key(emitter, "referenced_type_descriptions") &&
      start_sequence(emitter)))
  {
    return 0;
  }
  for (size_t i = 0; i < type_description->referenced_type_descriptions.size; i++) {
    if (!emit_individual_type_description(
        emitter, &type_description->referenced_type_descriptions.data[i]))
    {
      return 0;
    }
  }
  return end_sequence(emitter) && end_mapping(emitter);
}

rcl_ret_t
rcl_type_description_to_hashable_json(
  const type_description_interfaces__msg__TypeDescription * type_description,
  rcutils_char_array_t * output_repr)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(type_description, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_repr, RCL_RET_INVALID_ARGUMENT);

  yaml_emitter_t emitter;
  yaml_event_t event;

  if (!yaml_emitter_initialize(&emitter)) {
    goto error;
  }

  // Disable line breaks based on line length
  yaml_emitter_set_width(&emitter, -1);
  // Circumvent EOF line break by providing invalid break style
  yaml_emitter_set_break(&emitter, -1);
  yaml_emitter_set_output(&emitter, yaml_write_handler, output_repr);

  if (!(
      yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING) &&
      yaml_emitter_emit(&emitter, &event) &&

      yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1) &&
      yaml_emitter_emit(&emitter, &event) &&

      emit_type_description(&emitter, type_description) &&

      yaml_document_end_event_initialize(&event, 1) &&
      yaml_emitter_emit(&emitter, &event) &&

      yaml_stream_end_event_initialize(&event) &&
      yaml_emitter_emit(&emitter, &event)))
  {
    goto error;
  }

  yaml_emitter_delete(&emitter);
  return RCL_RET_OK;

error:
  rcl_set_error_state(emitter.problem, __FILE__, __LINE__);
  yaml_emitter_delete(&emitter);
  return RCL_RET_ERROR;
}

rcl_ret_t
rcl_calculate_type_hash(
  const type_description_interfaces__msg__TypeDescription * type_description,
  rosidl_type_hash_t * output_hash)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(type_description, RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(output_hash, RCL_RET_INVALID_ARGUMENT);

  rcl_ret_t result = RCL_RET_OK;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_char_array_t msg_repr = rcutils_get_zero_initialized_char_array();
  rcutils_ret_t rcutils_result = rcutils_char_array_init(&msg_repr, 0, &allocator);
  if (rcutils_result != RCL_RET_OK) {
    // rcutils_char_array_init already set the error
    return rcl_convert_rcutils_ret_to_rcl_ret(rcutils_result);
  }

  output_hash->version = 1;
  result = rcl_type_description_to_hashable_json(type_description, &msg_repr);
  if (result == RCL_RET_OK) {
    rcutils_sha256_ctx_t sha_ctx;
    rcutils_sha256_init(&sha_ctx);
    // Last item in char_array is null terminator, which should not be hashed.
    rcutils_sha256_update(&sha_ctx, (const uint8_t *)msg_repr.buffer, msg_repr.buffer_length - 1);
    rcutils_sha256_final(&sha_ctx, output_hash->value);
  }
  result = rcutils_char_array_fini(&msg_repr);
  return result;
}
