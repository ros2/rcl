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

#include <chrono>
#include <string>
#include <thread>

#include "rcl/service.h"

#include "rcl/rcl.h"

#include "example_interfaces/srv/add_two_ints.h"

#include "../memory_tools/memory_tools.hpp"
#include "../scope_exit.hpp"
#include "rcl/error_handling.h"

bool
wait_for_service_to_be_ready(
  rcl_service_t * service,
  size_t max_tries,
  int64_t period_ms)
{
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  rcl_ret_t ret = rcl_wait_set_init(&wait_set, 0, 0, 0, 0, 1, rcl_get_default_allocator());
  if (ret != RCL_RET_OK) {
    fprintf(stderr, "Error in wait set init: %s\n", rcl_get_error_string_safe());
    return false;
  }
  auto wait_set_exit = make_scope_exit([&wait_set]() {
    if (rcl_wait_set_fini(&wait_set) != RCL_RET_OK) {
      fprintf(stderr, "Error in wait set fini: %s\n", rcl_get_error_string_safe());
      throw std::runtime_error("error waiting for service to be ready");
    }
  });
  size_t iteration = 0;
  do {
    ++iteration;
    if (rcl_wait_set_clear_services(&wait_set) != RCL_RET_OK) {
      fprintf(stderr, "Error in wait_set_clear_services: %s\n", rcl_get_error_string_safe());
      return false;
    }
    if (rcl_wait_set_add_service(&wait_set, service) != RCL_RET_OK) {
      fprintf(stderr, "Error in wait_set_add_service: %s\n", rcl_get_error_string_safe());
      return false;
    }
    ret = rcl_wait(&wait_set, RCL_MS_TO_NS(period_ms));
    if (ret == RCL_RET_TIMEOUT) {
      continue;
    }
    if (ret != RCL_RET_OK) {
      fprintf(stderr, "Error in wait: %s\n", rcl_get_error_string_safe());
      return false;
    }
    for (size_t i = 0; i < wait_set.size_of_services; ++i) {
      if (wait_set.services[i] && wait_set.services[i] == service) {
        return true;
      }
    }
  } while (iteration < max_tries);
  return false;
}

int main(int argc, char ** argv)
{
  int main_ret = 0;
  {
    if (rcl_init(argc, argv, rcl_get_default_allocator()) != RCL_RET_OK) {
      fprintf(stderr, "Error in rcl init: %s\n", rcl_get_error_string_safe());
      return -1;
    }
    rcl_node_t node = rcl_get_zero_initialized_node();
    const char * name = "service_fixture_node";
    rcl_node_options_t node_options = rcl_node_get_default_options();
    if (rcl_node_init(&node, name, "", &node_options) != RCL_RET_OK) {
      fprintf(stderr, "Error in node init: %s\n", rcl_get_error_string_safe());
      return -1;
    }
    auto node_exit = make_scope_exit([&main_ret, &node]() {
      if (rcl_node_fini(&node) != RCL_RET_OK) {
        fprintf(stderr, "Error in node fini: %s\n", rcl_get_error_string_safe());
        main_ret = -1;
      }
    });

    const rosidl_service_type_support_t * ts = ROSIDL_GET_SRV_TYPE_SUPPORT(
      example_interfaces, AddTwoInts);
    const char * topic = "add_two_ints";

    rcl_service_t service = rcl_get_zero_initialized_service();
    rcl_service_options_t service_options = rcl_service_get_default_options();
    rcl_ret_t ret = rcl_service_init(&service, &node, ts, topic, &service_options);
    if (ret != RCL_RET_OK) {
      fprintf(stderr, "Error in service init: %s\n", rcl_get_error_string_safe());
      return -1;
    }

    auto service_exit = make_scope_exit([&main_ret, &service, &node]() {
      if (rcl_service_fini(&service, &node)) {
        fprintf(stderr, "Error in service fini: %s\n", rcl_get_error_string_safe());
        main_ret = -1;
      }
    });

    // Initialize a response.
    example_interfaces__srv__AddTwoInts_Response service_response;
    example_interfaces__srv__AddTwoInts_Response__init(&service_response);
    auto response_exit = make_scope_exit([&service_response]() {
      example_interfaces__srv__AddTwoInts_Response__fini(&service_response);
    });

    // Block until a client request comes in.

    if (!wait_for_service_to_be_ready(&service, 1000, 100)) {
      fprintf(stderr, "Service never became ready\n");
      return -1;
    }

    // Take the pending request.
    example_interfaces__srv__AddTwoInts_Request service_request;
    example_interfaces__srv__AddTwoInts_Request__init(&service_request);
    auto request_exit = make_scope_exit([&service_request]() {
      example_interfaces__srv__AddTwoInts_Request__fini(&service_request);
    });
    rmw_request_id_t header;
    // TODO(jacquelinekay) May have to check for timeout error codes
    if (rcl_take_request(&service, &header, &service_request) != RCL_RET_OK) {
      fprintf(stderr, "Error in take_request: %s\n", rcl_get_error_string_safe());
      return -1;
    }

    // Sum the request and send the response.
    service_response.sum = service_request.a + service_request.b;
    if (rcl_send_response(&service, &header, &service_response) != RCL_RET_OK) {
      fprintf(stderr, "Error in send_response: %s\n", rcl_get_error_string_safe());
      return -1;
    }
    // Our scope exits should take care of fini for everything
    // stick around until launch gives us a signal to exit
    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  return main_ret;
}
