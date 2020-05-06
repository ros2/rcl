`rcl_action` provides a pure C implementation of the ROS concept of an action.

It builds on top of the implementation of topics and services in `rcl`.

`rcl_action` consists of functions and structs for the following ROS action entities:

 - Action client
   - [rcl_action/action_client.h](include/rcl_action/action_client.h)
 - Action server
   - [rcl_action/action_server.h](include/rcl_action/action_server.h)
 - Goal handle
   - [rcl_action/goal_handle.h](include/rcl_action/goal_handle.h)
 - Goal state machine
   - [rcl_action/goal_state_machine.h](include/rcl_action/goal_state_machine.h)

It also has some machinery that is necessary to wait on and act on these entities:

- Wait sets for waiting on actions clients and action servers to be ready
 - [rcl_action/wait.h](include/rcl_action/wait.h)

Some useful abstractions and utilities:

- Return codes and other types
 - [rcl_action/types.h](include/rcl_action/types.h)
