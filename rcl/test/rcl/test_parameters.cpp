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

#include <gtest/gtest.h>

#include "rosidl_generator_c/string_functions.h"
#include "rosidl_generator_c/primitives_array_functions.h"

#include "rcl_interfaces/msg/list_parameters_result__functions.h"

#include "rcl_interfaces/msg/parameter__functions.h"
#include "rcl_interfaces/msg/parameter_type__struct.h"
#include "rcl_interfaces/msg/parameter_value__functions.h"
#include "rcl_interfaces/msg/set_parameters_result__functions.h"

#include "rcl/error_handling.h"
#include "rcl/node.h"
#include "rcl/parameter.h"
#include "rcl/parameter_client.h"
#include "rcl/parameter_service.h"

#include "rcl/rcl.h"


#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

#define WAIT_TIME -1
// #define WAIT_TIME 1000000000
#define NUM_PARAMS (size_t)4


class CLASSNAME (TestParametersFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  rcl_node_t * node_ptr = nullptr;
  rcl_wait_set_t * wait_set = nullptr;
  rcl_parameter_service_t * parameter_service = nullptr;
  rcl_parameter_client_t * parameter_client = nullptr;

  void SetUp()
  {
    rcl_ret_t ret;
    ret = rcl_init(0, nullptr, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    this->node_ptr = new rcl_node_t;
    *this->node_ptr = rcl_get_zero_initialized_node();
    const char * name = "parameter_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    ret = rcl_node_init(this->node_ptr, name, "", &node_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    this->wait_set = new rcl_wait_set_t;
    *this->wait_set = rcl_get_zero_initialized_wait_set();
    ret = rcl_wait_set_init(wait_set, 0, 0, 0, 0, 0, rcl_get_default_allocator());
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    this->parameter_client = new rcl_parameter_client_t;
    *this->parameter_client = rcl_get_zero_initialized_parameter_client();
    rcl_parameter_client_options_t cs_options = rcl_parameter_client_get_default_options();
    ret = rcl_parameter_client_init(this->parameter_client, this->node_ptr, &cs_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

    this->parameter_service = new rcl_parameter_service_t;
    *this->parameter_service = rcl_get_zero_initialized_parameter_service();
    rcl_parameter_service_options_t ps_options = rcl_parameter_service_get_default_options();
    ret = rcl_parameter_service_init(this->parameter_service, this->node_ptr, &ps_options);
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }

  void TearDown()
  {
    rcl_ret_t ret;
    if (this->wait_set) {
      ret = rcl_wait_set_fini(this->wait_set);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      delete this->wait_set;
    }

    if (this->parameter_service) {
      ret = rcl_parameter_service_fini(this->parameter_service);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      delete this->parameter_service;
    }

    if (this->parameter_client) {
      ret = rcl_parameter_client_fini(this->parameter_client);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      delete this->parameter_client;
    }

    if (this->node_ptr) {
      ret = rcl_node_fini(this->node_ptr);
      EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
      delete this->node_ptr;
    }
    ret = rcl_shutdown();
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  }
};

// Helper function for filling in hardcoded test values
rcl_ret_t fill_parameter_array(rcl_interfaces__msg__Parameter__Array * parameters)
{
  size_t parameters_idx = 0;
  rcl_ret_t ret = rcl_parameter_set_bool(&parameters->data[parameters_idx++], "bool_param", true);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_parameter_set_integer(&parameters->data[parameters_idx++], "int_param", 123);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_parameter_set_double(&parameters->data[parameters_idx++], "float_param", 45.67);
  if (ret != RCL_RET_OK) {
    return ret;
  }

  ret = rcl_parameter_set_string(
    &parameters->data[parameters_idx++], "string_param", "hello world");
  // TODO(jacquelinekay) Bytes and other arrays of primitives need more helper functions
  /*
  uint8_t bytes[5] = "\1\2\3\4";
  ret = rcl_parameter_set_bytes(&parameters->data[parameters_idx++], "bytes_param", &(bytes[0]));
  */
  return ret;
}

void compare_parameter_array(const rcl_interfaces__msg__Parameter__Array * parameters)
{
  ASSERT_EQ(parameters->size, NUM_PARAMS);
  size_t parameters_idx = 0;

  EXPECT_EQ(strcmp(parameters->data[parameters_idx].name.data, "bool_param"), 0);
  EXPECT_TRUE(parameters->data[parameters_idx++].value.bool_value);

  EXPECT_EQ(strcmp(parameters->data[parameters_idx].name.data, "int_param"), 0);
  EXPECT_EQ(parameters->data[parameters_idx++].value.integer_value, 123);

  EXPECT_EQ(strcmp(parameters->data[parameters_idx].name.data, "float_param"), 0);
  EXPECT_EQ(parameters->data[parameters_idx++].value.double_value, 45.67);

  EXPECT_EQ(strcmp(parameters->data[parameters_idx].name.data, "string_param"), 0);
  EXPECT_EQ(strcmp(parameters->data[parameters_idx].value.string_value.data, "hello world"), 0);
}

void compare_parameter_array(const rcl_interfaces__msg__ParameterValue__Array * parameters)
{
  ASSERT_EQ(parameters->size, NUM_PARAMS);
  size_t parameters_idx = 0;

  EXPECT_TRUE(parameters->data[parameters_idx++].bool_value);
  EXPECT_EQ(parameters->data[parameters_idx++].integer_value, 123);
  EXPECT_EQ(parameters->data[parameters_idx++].double_value, 45.67);
  EXPECT_EQ(strcmp(parameters->data[parameters_idx].string_value.data, "hello world"), 0);
}

void compare_parameter_array(const rosidl_generator_c__String__Array * parameter_names)
{
  ASSERT_EQ(parameter_names->size, NUM_PARAMS);
  size_t parameters_idx = 0;

  EXPECT_EQ(strcmp(parameter_names->data[parameters_idx++].data, "bool_param"), 0);
  EXPECT_EQ(strcmp(parameter_names->data[parameters_idx++].data, "int_param"), 0);
  EXPECT_EQ(strcmp(parameter_names->data[parameters_idx++].data, "float_param"), 0);
  EXPECT_EQ(strcmp(parameter_names->data[parameters_idx++].data, "string_param"), 0);
}

rcl_ret_t fill_parameter_array(rcl_interfaces__msg__ParameterValue__Array * parameters)
{
  size_t parameters_idx = 0;
  rcl_ret_t ret = rcl_parameter_set_value_bool(&parameters->data[parameters_idx++], true);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_parameter_set_value_integer(&parameters->data[parameters_idx++], 123);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_parameter_set_value_double(&parameters->data[parameters_idx++], 45.67);
  if (ret != RCL_RET_OK) {
    return ret;
  }

  ret = rcl_parameter_set_value_string(&parameters->data[parameters_idx++], "hello world");
  /*
  uint8_t bytes[5] = "\1\2\3\4";
  ret = rcl_parameter_set_value_bytes(&parameters->data[parameters_idx++], &(bytes[0]));
  */
  return ret;
}

bool fill_parameter_names_array(rosidl_generator_c__String__Array * names)
{
  size_t parameters_idx = 0;
  if (!rosidl_generator_c__String__assign(&names->data[parameters_idx++], "bool_param")) {
    return false;
  }
  if (!rosidl_generator_c__String__assign(&names->data[parameters_idx++], "int_param")) {
    return false;
  }
  if (!rosidl_generator_c__String__assign(&names->data[parameters_idx++], "float_param")) {
    return false;
  }
  if (!rosidl_generator_c__String__assign(&names->data[parameters_idx++], "string_param")) {
    return false;
  }
  // rosidl_generator_c__String__assign(&names->data[parameters_idx++], "bytes_param");
  return true;
}

rcl_ret_t prepare_wait_set(
  rcl_wait_set_t * wait_set, rcl_parameter_service_t * parameter_service,
  rcl_parameter_client_t * parameter_client)
{
  rcl_ret_t ret = rcl_wait_set_clear_services(wait_set);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_clear_clients(wait_set);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_clear_subscriptions(wait_set);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_resize_services(wait_set, RCL_NUMBER_OF_PARAMETER_ACTIONS);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_resize_clients(wait_set, RCL_NUMBER_OF_PARAMETER_ACTIONS);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_resize_subscriptions(wait_set, 1);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_add_parameter_service(wait_set, parameter_service);
  if (ret != RCL_RET_OK) {
    return ret;
  }
  ret = rcl_wait_set_add_parameter_client(wait_set, parameter_client);
  return ret;
}

// TODO(jacquelinekay) Test un-setting parameters using set_parameters
TEST_F(CLASSNAME(TestParametersFixture, RMW_IMPLEMENTATION), test_set_parameters) {
  rmw_request_id_t request_header;
  rcl_param_action_t action = RCL_PARAMETER_ACTION_UNKNOWN;
  rcl_ret_t ret;

  rcl_interfaces__msg__Parameter__Array parameters;
  EXPECT_TRUE(rcl_interfaces__msg__Parameter__Array__init(&parameters, NUM_PARAMS));

  ret = fill_parameter_array(&parameters);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  int64_t seq_num;
  ret = rcl_parameter_client_send_set_request(this->parameter_client, &parameters, &seq_num);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  ret = rcl_parameter_service_get_pending_action(wait_set, parameter_service, &action);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  EXPECT_EQ(action, RCL_SET_PARAMETERS);

  rcl_interfaces__msg__Parameter__Array * parameters_req = rcl_parameter_service_take_set_request(
    this->parameter_service, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  // Validate the request
  compare_parameter_array(parameters_req);

  // For now we'll just set them all to be successful
  // Should SetParametersResult have a "name" field for the parameter key it describes?

  rcl_interfaces__msg__SetParametersResult__Array results;
  EXPECT_TRUE(rcl_interfaces__msg__SetParametersResult__Array__init(&results, NUM_PARAMS));

  for (size_t i = 0; i < NUM_PARAMS; ++i) {
    results.data[i].successful = true;
    rosidl_generator_c__String__assign(&results.data[i].reason, "success");
  }
  ret = rcl_parameter_service_send_set_response(
    this->parameter_service, &request_header, &results);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_parameter_client_get_pending_action(wait_set, parameter_client, &action);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  EXPECT_EQ(action, RCL_SET_PARAMETERS);

  rcl_interfaces__msg__SetParametersResult__Array * results_response =
    rcl_parameter_client_take_set_response(this->parameter_client, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  for (size_t i = 0; i < NUM_PARAMS; ++i) {
    EXPECT_TRUE(results_response->data[i].successful);
    EXPECT_EQ(strcmp(results.data[i].reason.data, "success"), 0);
  }

  rcl_interfaces__msg__Parameter__Array prior_state;
  EXPECT_TRUE(rcl_interfaces__msg__Parameter__Array__init(&prior_state, 3));

  // Bogus values for the previous state: one the same, one removed, one changed
  ret = rcl_parameter_set_integer(&prior_state.data[0], "int_param", 123);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_parameter_set_integer(&prior_state.data[1], "deleted", 24);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_parameter_set_double(&prior_state.data[2], "float_param", -45.67);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  rcl_interfaces__msg__ParameterEvent event;
  EXPECT_TRUE(rcl_interfaces__msg__ParameterEvent__init(&event));

  ret = rcl_parameter_convert_changes_to_event(&prior_state, parameters_req, &event);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  auto validate_event = [](const rcl_interfaces__msg__ParameterEvent & param_event) {
      EXPECT_EQ(strcmp(param_event.changed_parameters.data[0].name.data, "float_param"), 0);
      EXPECT_EQ(param_event.changed_parameters.data[0].value.double_value, 45.67);

      EXPECT_EQ(strcmp(param_event.deleted_parameters.data[0].name.data, "deleted"), 0);

      // Ordering of new parameters doesn't matter
      // New parameters
      EXPECT_EQ(strcmp(param_event.new_parameters.data[0].name.data, "bool_param"), 0);
      EXPECT_TRUE(param_event.new_parameters.data[0].value.bool_value);

      EXPECT_EQ(strcmp(param_event.new_parameters.data[1].name.data, "string_param"), 0);
      EXPECT_EQ(strcmp(param_event.new_parameters.data[1].value.string_value.data, "hello world"),
        0);
    };

  validate_event(event);

  ret = rcl_parameter_service_publish_event(this->parameter_service, &event);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  rcl_interfaces__msg__ParameterEvent event_response;
  EXPECT_TRUE(rcl_interfaces__msg__ParameterEvent__init(&event_response));
  ret = rcl_parameter_client_take_event(this->parameter_client, &event_response, nullptr);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  validate_event(event_response);
}

TEST_F(CLASSNAME(TestParametersFixture, RMW_IMPLEMENTATION), test_set_parameters_atomically) {
  rmw_request_id_t request_header;
  rcl_param_action_t action;
  rcl_ret_t ret;

  rcl_interfaces__msg__Parameter__Array parameters;
  EXPECT_TRUE(rcl_interfaces__msg__Parameter__Array__init(&parameters, NUM_PARAMS));

  rcl_interfaces__msg__SetParametersResult result;
  EXPECT_TRUE(rcl_interfaces__msg__SetParametersResult__init(&result));

  ret = fill_parameter_array(&parameters);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  int64_t seq_num;
  ret = rcl_parameter_client_send_set_atomically_request(
    this->parameter_client, &parameters, &seq_num);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_parameter_service_get_pending_action(this->wait_set, this->parameter_service, &action);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  EXPECT_EQ(action, RCL_SET_PARAMETERS_ATOMICALLY);

  rcl_interfaces__msg__Parameter__Array * parameters_req =
    rcl_parameter_service_take_set_atomically_request(this->parameter_service, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  compare_parameter_array(parameters_req);

  // For now we'll just set them all to be successful
  // Should SetParametersResult have a "name" field for the parameter key it describes?
  result.successful = true;
  rosidl_generator_c__String__assign(&result.reason, "Because reasons");
  ret = rcl_parameter_service_send_set_atomically_response(
    this->parameter_service, &request_header, &result);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  ret = rcl_parameter_client_get_pending_action(this->wait_set, this->parameter_client, &action);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;
  EXPECT_EQ(action, RCL_SET_PARAMETERS_ATOMICALLY);

  rcl_interfaces__msg__SetParametersResult * result_response =
    rcl_parameter_client_take_set_atomically_response(this->parameter_client, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe;

  EXPECT_TRUE(result_response->successful);
  EXPECT_EQ(strcmp(result_response->reason.data, "Because reasons"), 0);
}


TEST_F(CLASSNAME(TestParametersFixture, RMW_IMPLEMENTATION), test_get_parameters) {
  rmw_request_id_t request_header;
  rcl_ret_t ret;
  rcl_param_action_t action;
  (void) ret;

  rosidl_generator_c__String__Array parameter_names;
  EXPECT_TRUE(rosidl_generator_c__String__Array__init(&parameter_names, NUM_PARAMS));

  rcl_interfaces__msg__ParameterValue__Array parameter_values;
  EXPECT_TRUE(rcl_interfaces__msg__ParameterValue__Array__init(&parameter_values, NUM_PARAMS));

  EXPECT_TRUE(fill_parameter_names_array(&parameter_names)) << rcl_get_error_string_safe();

  int64_t seq_num;
  ret = rcl_parameter_client_send_get_request(
    this->parameter_client, &parameter_names, &seq_num);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_parameter_service_get_pending_action(this->wait_set, this->parameter_service, &action);
  EXPECT_EQ(action, RCL_GET_PARAMETERS);

  rosidl_generator_c__String__Array * request = rcl_parameter_service_take_get_request(
    this->parameter_service, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK);
  compare_parameter_array(request);

  // Assign some bogus values
  // In a real client library, these would be pulled from storage
  ret = fill_parameter_array(&parameter_values);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_parameter_service_send_get_response(
    this->parameter_service, &request_header, &parameter_values);
  EXPECT_EQ(ret, RCL_RET_OK);

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_parameter_client_get_pending_action(this->wait_set, this->parameter_client, &action);
  EXPECT_EQ(action, RCL_GET_PARAMETERS);

  // TODO(jacquelinekay): Should GetParameters_Response have a Parameter__Array subfield
  // instead of a ParameterValue__Array?
  rcl_interfaces__msg__ParameterValue__Array * response = rcl_parameter_client_take_get_response(
    this->parameter_client, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK);

  compare_parameter_array(response);
}


TEST_F(CLASSNAME(TestParametersFixture, RMW_IMPLEMENTATION), test_get_parameter_types) {
  rmw_request_id_t request_header;
  rcl_ret_t ret;
  rcl_param_action_t action;
  (void) ret;

  rosidl_generator_c__String__Array parameter_names;
  EXPECT_TRUE(rosidl_generator_c__String__Array__init(&parameter_names, NUM_PARAMS));

  rosidl_generator_c__uint8__Array parameter_types;
  EXPECT_TRUE(rosidl_generator_c__uint8__Array__init(&parameter_types, NUM_PARAMS));

  EXPECT_TRUE(fill_parameter_names_array(&parameter_names)) << rcl_get_error_string_safe();
  int64_t seq_num;
  ret = rcl_parameter_client_send_get_types_request(this->parameter_client, &parameter_names,
      &seq_num);

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_parameter_service_get_pending_action(this->wait_set, this->parameter_service, &action);
  EXPECT_EQ(action, RCL_GET_PARAMETER_TYPES);

  rosidl_generator_c__String__Array * request = rcl_parameter_service_take_get_types_request(
    this->parameter_service, &request_header);
  compare_parameter_array(request);

  {
    size_t parameters_idx = 0;
    parameter_types.data[parameters_idx++] = rcl_interfaces__msg__ParameterType__PARAMETER_BOOL;
    parameter_types.data[parameters_idx++] = rcl_interfaces__msg__ParameterType__PARAMETER_INTEGER;
    parameter_types.data[parameters_idx++] = rcl_interfaces__msg__ParameterType__PARAMETER_DOUBLE;
    parameter_types.data[parameters_idx++] = rcl_interfaces__msg__ParameterType__PARAMETER_STRING;
    parameter_types.data[parameters_idx++] = rcl_interfaces__msg__ParameterType__PARAMETER_BYTES;
  }
  ret = rcl_parameter_service_send_get_types_response(this->parameter_service, &request_header,
      &parameter_types);

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK);
  ret = rcl_parameter_client_get_pending_action(this->wait_set, this->parameter_client, &action);
  EXPECT_EQ(action, RCL_GET_PARAMETER_TYPES);

  rosidl_generator_c__uint8__Array * response = rcl_parameter_client_take_get_types_response(
    this->parameter_client, &request_header);

  {
    size_t parameters_idx = 0;
    EXPECT_EQ(response->data[parameters_idx++], rcl_interfaces__msg__ParameterType__PARAMETER_BOOL);
    EXPECT_EQ(response->data[parameters_idx++],
      rcl_interfaces__msg__ParameterType__PARAMETER_INTEGER);
    EXPECT_EQ(response->data[parameters_idx++],
      rcl_interfaces__msg__ParameterType__PARAMETER_DOUBLE);
    EXPECT_EQ(response->data[parameters_idx++],
      rcl_interfaces__msg__ParameterType__PARAMETER_STRING);
    // EXPECT_EQ(
    //   response.data[parameters_idx++], rcl_interfaces__msg__ParameterType__PARAMETER_BYTES);
  }
}

TEST_F(CLASSNAME(TestParametersFixture, RMW_IMPLEMENTATION), test_list_parameters) {
  rmw_request_id_t request_header;
  rcl_ret_t ret;
  rcl_param_action_t action;
  (void) ret;

  rcl_interfaces__msg__ListParametersResult list_result;
  EXPECT_TRUE(rcl_interfaces__msg__ListParametersResult__init(&list_result));
  EXPECT_TRUE(rosidl_generator_c__String__Array__init(&list_result.names, NUM_PARAMS));
  EXPECT_TRUE(rosidl_generator_c__String__Array__init(&list_result.prefixes, NUM_PARAMS));

  rosidl_generator_c__String__Array prefixes;
  EXPECT_TRUE(rosidl_generator_c__String__Array__init(&prefixes, 0));

  uint64_t depth = 0;
  int64_t seq_num;
  ret = rcl_parameter_client_send_list_request(this->parameter_client, &prefixes, depth, &seq_num);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  ret = rcl_parameter_service_get_pending_action(this->wait_set, this->parameter_service, &action);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(action, RCL_LIST_PARAMETERS);

  rosidl_generator_c__String__Array prefixes_req;
  uint64_t depth_req;
  EXPECT_TRUE(rosidl_generator_c__String__Array__init(&prefixes_req, 0));
  ret = rcl_parameter_service_take_list_request(this->parameter_service, &request_header,
      &prefixes_req, &depth_req);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  // put some names in
  EXPECT_TRUE(fill_parameter_names_array(&list_result.names)) << rcl_get_error_string_safe();
  ret = rcl_parameter_service_send_list_response(this->parameter_service, &request_header,
      &list_result);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  ret = prepare_wait_set(this->wait_set, this->parameter_service, this->parameter_client);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  ret = rcl_wait(this->wait_set, WAIT_TIME);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  ret = rcl_parameter_client_get_pending_action(this->wait_set, this->parameter_client, &action);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();
  EXPECT_EQ(action, RCL_LIST_PARAMETERS);

  rcl_interfaces__msg__ListParametersResult * result_response =
    rcl_parameter_client_take_list_response(this->parameter_client, &request_header);
  EXPECT_EQ(ret, RCL_RET_OK) << rcl_get_error_string_safe();

  compare_parameter_array(&result_response->names);
}
