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

#include "rcl/type_description_conversions.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/type_description/type_description__functions.h"
#include "rosidl_runtime_c/type_description/type_source__functions.h"
#include "test_msgs/msg/basic_types.h"

TEST(TestTypeDescriptionConversions, type_description_conversion_round_trip) {
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);

  type_description_interfaces__msg__TypeDescription * type_description_msg =
    rcl_convert_type_description_runtime_to_msg(ts->get_type_description_func(ts));
  EXPECT_TRUE(NULL != type_description_msg);

  rosidl_runtime_c__type_description__TypeDescription * type_description_rt =
    rcl_convert_type_description_msg_to_runtime(type_description_msg);
  EXPECT_TRUE(NULL != type_description_rt);

  EXPECT_TRUE(
    rosidl_runtime_c__type_description__TypeDescription__are_equal(
      type_description_rt, ts->get_type_description_func(ts)));

  type_description_interfaces__msg__TypeDescription__destroy(
    type_description_msg);
  rosidl_runtime_c__type_description__TypeDescription__destroy(
    type_description_rt);
}

TEST(TestTypeDescriptionConversions, type_description_invalid_input) {
  EXPECT_TRUE(NULL == rcl_convert_type_description_runtime_to_msg(NULL));
  EXPECT_TRUE(NULL == rcl_convert_type_description_msg_to_runtime(NULL));
}

TEST(TestTypeDescriptionConversions, type_source_sequence_conversion_round_trip) {
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);

  type_description_interfaces__msg__TypeSource__Sequence * type_sources_msg =
    rcl_convert_type_source_sequence_runtime_to_msg(ts->get_type_description_sources_func(ts));
  EXPECT_TRUE(NULL != type_sources_msg);

  rosidl_runtime_c__type_description__TypeSource__Sequence * type_sources_rt =
    rcl_convert_type_source_sequence_msg_to_runtime(type_sources_msg);
  EXPECT_TRUE(NULL != type_sources_rt);

  EXPECT_TRUE(
    rosidl_runtime_c__type_description__TypeSource__Sequence__are_equal(
      type_sources_rt, ts->get_type_description_sources_func(ts)));

  type_description_interfaces__msg__TypeSource__Sequence__destroy(
    type_sources_msg);
  rosidl_runtime_c__type_description__TypeSource__Sequence__destroy(
    type_sources_rt);
}

TEST(TestTypeDescriptionConversions, type_source_sequence_invalid_input) {
  EXPECT_TRUE(NULL == rcl_convert_type_source_sequence_msg_to_runtime(NULL));
  EXPECT_TRUE(NULL == rcl_convert_type_source_sequence_runtime_to_msg(NULL));
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
