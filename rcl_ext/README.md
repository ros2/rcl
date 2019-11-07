# The rcl_ext package

The package rcl_ext is a small [ROS 2](http://www.ros2.org/) package for providing a thin API layer on top of RCL to create nodes, publishers, subscribers and timers with a one-liner.

It it similar to rclc API, however does not provide a spin() funktion for the node to receive messages. This is intentional because the executor concept shall be implemented a different package, e.g. rcl-executor.
Also, it does not create additional data structures, like rclc_node_t, rclc_publisher_t, rclc_subscriber_t etc. in the package rclc, but uses the rcl data structures instead. This light-waight and easy-to-use interface is intended for using ROS2 on micro-controllers without the rclcpp or rclpy. 

API:
- rcl_ext_init()
- rcl_ext_init_fini()
- rcl_ext_create_node()
- rcl_ext_node_fini()
- rcl_ext_create_publisher()
- rcl_ext_publisher_fini()
- rcl_ext_create_subscription()
- rcl_ext_subscription_fini()
- rcl_ext_create_timer()
- rcl_ext_timer_fini()


A complete code example with the `rcl-ext` and `rcl-executor` package is provided in the package [rcl_ext_examples](../rcl_ext_examples)


