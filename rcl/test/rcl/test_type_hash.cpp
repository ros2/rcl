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

#include <gtest/gtest.h>

#include <cstring>
#include <tuple>
#include <vector>

#include "type_description_interfaces/msg/type_description.h"
#include "type_description_interfaces/msg/individual_type_description.h"
#include "type_description_interfaces/msg/field.h"
#include "type_description_interfaces/msg/field_type.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rcl/allocator.h"
#include "rcl/type_hash.h"
#include "rcutils/sha256.h"

// Copied directly from generated code
static const rosidl_type_hash_t sensor_msgs__msg__PointCloud2__TYPE_HASH__copy = {1, {
    0x91, 0x98, 0xca, 0xbf, 0x7d, 0xa3, 0x79, 0x6a,
    0xe6, 0xfe, 0x19, 0xc4, 0xcb, 0x3b, 0xdd, 0x35,
    0x25, 0x49, 0x29, 0x88, 0xc7, 0x05, 0x22, 0x62,
    0x8a, 0xf5, 0xda, 0xa1, 0x24, 0xba, 0xe2, 0xb5,
  }};


static void init_individual_type_description(
  type_description_interfaces__msg__IndividualTypeDescription * itd,
  const char * name,
  const std::vector<std::tuple<const char *, uint8_t, const char *>> & fields)
{
  rosidl_runtime_c__String__assign(
    &itd->type_name, name);
  type_description_interfaces__msg__Field__Sequence__init(&itd->fields, fields.size());
  for (size_t i = 0; i < fields.size(); i++) {
    auto * field = &itd->fields.data[i];
    const char * name = std::get<0>(fields[i]);

    rosidl_runtime_c__String__assign(&field->name, name);
    field->type.type_id = std::get<1>(fields[i]);
    const char * nested_type = std::get<2>(fields[i]);
    if (nested_type != NULL) {
      rosidl_runtime_c__String__assign(&field->type.nested_type_name, nested_type);
    }
  }
}

TEST(TestTypeVersionHash, field_type_from_install) {
  rcl_ret_t res = RCL_RET_OK;
  type_description_interfaces__msg__TypeDescription * td_msg =
    type_description_interfaces__msg__TypeDescription__create();

  init_individual_type_description(
    &td_msg->type_description,
    "type_description_interfaces/msg/FieldType", {
    {"type_id", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT8, NULL},
    {"capacity", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT64, NULL},
    {"string_capacity", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT64, NULL},
    {
      "nested_type_name",
      type_description_interfaces__msg__FieldType__FIELD_TYPE_BOUNDED_STRING,
      NULL}}
  );
  td_msg->type_description.fields.data[3].type.string_capacity = 255;

  rosidl_type_hash_t direct_hash;
  res = rcl_calculate_type_hash(td_msg, &direct_hash);
  ASSERT_EQ(res, RCL_RET_OK);

  rosidl_type_hash_t hash_from_repr;
  hash_from_repr.version = 1;
  {
    rcutils_sha256_ctx_t sha;
    rcutils_char_array_t msg_repr = rcutils_get_zero_initialized_char_array();
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rcutils_ret_t rcutils_result = rcutils_char_array_init(&msg_repr, 0, &allocator);
    ASSERT_EQ(rcutils_result, RCL_RET_OK);

    res = rcl_type_description_to_hashable_json(td_msg, &msg_repr);
    ASSERT_EQ(res, RCL_RET_OK);

    rcutils_sha256_init(&sha);
    rcutils_sha256_update(&sha, (const uint8_t *)msg_repr.buffer, msg_repr.buffer_length - 1);
    rcutils_sha256_final(&sha, hash_from_repr.value);

    res = rcutils_char_array_fini(&msg_repr);
    ASSERT_EQ(res, RCL_RET_OK);
  }

  // NOTE: testing this against the actual installed one, forces an up to date test
  const rosidl_type_hash_t * validation_hash =
    type_description_interfaces__msg__FieldType__get_type_hash(NULL);
  ASSERT_EQ(direct_hash.version, hash_from_repr.version);
  ASSERT_EQ(direct_hash.version, validation_hash->version);
  ASSERT_EQ(0, memcmp(direct_hash.value, hash_from_repr.value, ROSIDL_TYPE_HASH_SIZE));
  ASSERT_EQ(0, memcmp(direct_hash.value, validation_hash->value, ROSIDL_TYPE_HASH_SIZE));
}


TEST(TestTypeVersionHash, nested_real_type) {
  rcl_ret_t res = RCL_RET_OK;
  type_description_interfaces__msg__TypeDescription * td_msg =
    type_description_interfaces__msg__TypeDescription__create();
  // 3 referenced types: std_msgs/Header, builtin_interfaces/Time, sensor_msgs/PointField
  type_description_interfaces__msg__IndividualTypeDescription__Sequence__init(
    &td_msg->referenced_type_descriptions, 3);

  // PointCloud2.msg
  //
  // std_msgs/Header header
  // uint32 height
  // uint32 width
  // PointField[] fields
  // bool    is_bigendian
  // uint32  point_step
  // uint32  row_step
  // uint8[] data
  // bool is_dense
  init_individual_type_description(
    &td_msg->type_description,
    "sensor_msgs/msg/PointCloud2", {
    {
      "header",
      type_description_interfaces__msg__FieldType__FIELD_TYPE_NESTED_TYPE,
      "std_msgs/msg/Header"},
    {"height", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL},
    {"width", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL},
    {
      "fields",
      type_description_interfaces__msg__FieldType__FIELD_TYPE_NESTED_TYPE_UNBOUNDED_SEQUENCE,
      "sensor_msgs/msg/PointField"},
    {"is_bigendian", type_description_interfaces__msg__FieldType__FIELD_TYPE_BOOLEAN, NULL},
    {"point_step", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL},
    {"row_step", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL},
    {
      "data",
      type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT8_UNBOUNDED_SEQUENCE,
      NULL},
    {"is_dense", type_description_interfaces__msg__FieldType__FIELD_TYPE_BOOLEAN, NULL}}
  );

  // add referenced types in alphabetical order
  size_t nested_field_index = 0;

  // builtin_interfaces/msg/Time.msg
  //
  // int32 sec
  // uint32 nanosec
  init_individual_type_description(
    &td_msg->referenced_type_descriptions.data[nested_field_index++],
    "builtin_interfaces/msg/Time", {
    {"sec", type_description_interfaces__msg__FieldType__FIELD_TYPE_INT32, NULL},
    {"nanosec", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL}}
  );

  // sensor_msgs/msg/PointField.msg
  //
  // string name
  // uint32 offset
  // uint8  datatype
  // uint32 count
  init_individual_type_description(
    &td_msg->referenced_type_descriptions.data[nested_field_index++],
    "sensor_msgs/msg/PointField", {
    {"name", type_description_interfaces__msg__FieldType__FIELD_TYPE_STRING, NULL},
    {"offset", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL},
    {"datatype", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT8, NULL},
    {"count", type_description_interfaces__msg__FieldType__FIELD_TYPE_UINT32, NULL}}
  );

  // std_msgs/msg/Header.msg
  //
  // builtin_interfaces/Time stamp
  // string frame_id
  init_individual_type_description(
    &td_msg->referenced_type_descriptions.data[nested_field_index++],
    "std_msgs/msg/Header", {
    {
      "stamp",
      type_description_interfaces__msg__FieldType__FIELD_TYPE_NESTED_TYPE,
      "builtin_interfaces/msg/Time"},
    {
      "frame_id",
      type_description_interfaces__msg__FieldType__FIELD_TYPE_STRING,
      NULL}}
  );

  rosidl_type_hash_t direct_hash;
  res = rcl_calculate_type_hash(td_msg, &direct_hash);
  ASSERT_EQ(res, RCL_RET_OK);

  rosidl_type_hash_t hash_from_repr;
  hash_from_repr.version = 1;
  {
    rcutils_sha256_ctx_t sha;
    rcutils_char_array_t msg_repr = rcutils_get_zero_initialized_char_array();
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rcutils_ret_t rcutils_result = rcutils_char_array_init(&msg_repr, 0, &allocator);
    ASSERT_EQ(rcutils_result, RCL_RET_OK);

    res = rcl_type_description_to_hashable_json(td_msg, &msg_repr);
    ASSERT_EQ(res, RCL_RET_OK);

    rcutils_sha256_init(&sha);
    rcutils_sha256_update(&sha, (const uint8_t *)msg_repr.buffer, msg_repr.buffer_length - 1);
    rcutils_sha256_final(&sha, hash_from_repr.value);

    res = rcutils_char_array_fini(&msg_repr);
    ASSERT_EQ(res, RCL_RET_OK);
  }

  auto validation_hash = sensor_msgs__msg__PointCloud2__TYPE_HASH__copy;
  ASSERT_EQ(direct_hash.version, hash_from_repr.version);
  ASSERT_EQ(direct_hash.version, validation_hash.version);
  ASSERT_EQ(0, memcmp(direct_hash.value, hash_from_repr.value, ROSIDL_TYPE_HASH_SIZE));
  ASSERT_EQ(0, memcmp(direct_hash.value, validation_hash.value, ROSIDL_TYPE_HASH_SIZE));
}
