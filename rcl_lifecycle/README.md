`rcl_lifecycle` provides a pure C implementation of the ROS concept of lifecycle. It builds on top of the implementation of topics and services in `rcl`.

`rcl_lifecycle` consists of functions and structs for the following ROS lifecycle entities:

 - Lifecycle states
 - Lifecycle transitions
 - Lifecycle state machine
 - Lifecycle triggers

Some useful abstractions:

Return codes and other types [rcl_lifecycle/data_types.h](include/rcl_lifecycle/data_types.h)
