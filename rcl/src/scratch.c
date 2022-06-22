#include "rcl/rcl.h"
#include "rmw/rmw.h"

#include "rcl_interfaces/msg/service_event.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"

#include "test_msgs/srv/basic_types.h"

int main(int argc, char** argv)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  rcl_ret_t ret = rcl_init_options_init(&init_options, allocator);
  if (RCL_RET_OK != ret) {
    printf("failed to init options\n");
    return 1;
  }
  rcl_context_t context = rcl_get_zero_initialized_context();
  ret = rcl_init(argc, argv, &init_options, &context);
  if (RCL_RET_OK != ret) {
    printf("failed to init context\n");
    return 1;
  }
  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_options = rcl_node_get_default_options();
  ret = rcl_node_init(&node, "foo", "", &context, &node_options);
  if (RCL_RET_OK != ret) {
    printf("failed to init node\n");
    return 1;
  }
 
  // Create a service event publisher
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  const rosidl_message_type_support_t * service_event_typesupport =
    ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, ServiceEvent);
  if (!service_event_typesupport) {
    printf("failed to get service event typesupport\n");
    return 1;
  }
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  ret = rcl_publisher_init(
    &publisher, &node, service_event_typesupport, "service_events", &publisher_options);
  if (RCL_RET_OK != ret) {
    printf("failed to init publisher\n");
    return 1;
  }

  // Create a service request message and serialize it
  test_msgs__srv__BasicTypes_Request request_msg;
  test_msgs__srv__BasicTypes_Request__init(&request_msg);
  const rosidl_message_type_support_t * request_typesupport =
    ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, srv, BasicTypes_Request);
  if (!request_typesupport) {
    printf("failed to get request typesupport\n");
    return 1;
  }
  rcl_serialized_message_t serialized_msg = rmw_get_zero_initialized_serialized_message();
  ret = rmw_serialized_message_init(&serialized_msg, 0u, &allocator);
  if (RCL_RET_OK != ret) {
    printf("failed to init serialized message\n");
    return 1;
  }
  ret = rmw_serialize(&request_msg, request_typesupport, &serialized_msg);
  if (RCL_RET_OK != ret) {
    printf("failed to serialize message\n");
    return 1;
  }

  // Create and populate a service event message
  rcl_interfaces__msg__ServiceEvent msg;
  rcl_interfaces__msg__ServiceEvent__init(&msg);
  rosidl_runtime_c__String__assign(&msg.service_name, "my_service");
  rosidl_runtime_c__String__assign(&msg.request_type_name, "test_msgs/srv/BasicTypes_Request");
  rosidl_runtime_c__octet__Sequence__init(&msg.serialized_request, serialized_msg.buffer_length);
  memcpy(msg.serialized_request.data, serialized_msg.buffer, serialized_msg.buffer_length);

  // Publish the message
  for (int i = 0; i < 20; ++i) {
    ret = rcl_publish(&publisher, &msg, NULL);
    if (RCL_RET_OK != ret) {
      printf("failed to publish message\n");
    } else {
      printf("published message\n");
    }
    usleep(1000000);
  }
    
  ret = rmw_serialized_message_fini(&serialized_msg);
  test_msgs__srv__BasicTypes_Request__fini(&request_msg);
  rcl_interfaces__msg__ServiceEvent__fini(&msg);
  ret = rcl_publisher_fini(&publisher, &node);
  ret = rcl_node_fini(&node);
  ret = rcl_shutdown(&context);
  ret = rcl_context_fini(&context);

  return 0;
}
