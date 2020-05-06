`rcl` consists of functions and structs (pure C) organized into ROS concepts:

- Nodes
 - [rcl/node.h](include/rcl/node.h)
- Publisher
 - [rcl/publisher.h](include/rcl/publisher.h)
- Subscription
 - [rcl/subscription.h](include/rcl/subscription.h)
- Service Client
 - [rcl/client.h](include/rcl/client.h)
- Service Server
 - [rcl/service.h](include/rcl/service.h)
- Timer
 - [rcl/timer.h](include/rcl/timer.h)

There are some functions for working with "Topics" and "Services":

- A function to validate a topic or service name (not necessarily fully qualified):
 - rcl_validate_topic_name()
 - [rcl/validate_topic_name.h](include/rcl/validate_topic_name.h)
- A function to expand a topic or service name to a fully qualified name:
 - rcl_expand_topic_name()
 - [rcl/expand_topic_name.h](include/rcl/expand_topic_name.h)

It also has some machinery that is necessary to wait on and act on these concepts:

- Initialization and shutdown management
 - [rcl/init.h](include/rcl/init.h)
- Wait sets for waiting on messages/service requests and responses/timers to be ready
 - [rcl/wait.h](include/rcl/wait.h)
- Guard conditions for waking up wait sets asynchronously
 - [rcl/guard_condition.h](include/rcl/guard_condition.h)
- Functions for introspecting and getting notified of changes of the ROS graph
 - [rcl/graph.h](include/rcl/graph.h)

Further still there are some useful abstractions and utilities:

- Allocator concept, which can be used to control allocation in `rcl_*` functions
 - [rcl/allocator.h](include/rcl/allocator.h)
- Concept of ROS Time and access to steady and system wall time
 - [rcl/time.h](include/rcl/time.h)
- Error handling functionality (C style)
 - [rcl/error_handling.h](include/rcl/error_handling.h)
- Macros
 - [rcl/macros.h](include/rcl/macros.h)
- Return code types
 - [rcl/types.h](include/rcl/types.h)
- Macros for controlling symbol visibility on the library
 - [rcl/visibility_control.h](include/rcl/visibility_control.h)
