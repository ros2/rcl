^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_action
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.6.0 (2018-11-16)
------------------
* Made rcl_action_get\_*_name() functions check for empty action names. `#329 <https://github.com/ros2/rcl/issues/329>`_
* Implemented Action client `#319 <https://github.com/ros2/rcl/issues/319>`_
* Added function to check if goal can be transitioned to CANCELING (`#325 <https://github.com/ros2/rcl/issues/325>`_)
* Implement goal handle (`#320 <https://github.com/ros2/rcl/issues/320>`_)
* Update to use new error handling API from rcutils (`#314 <https://github.com/ros2/rcl/issues/314>`_)
* Add action services and topics name getters `#317 <https://github.com/ros2/rcl/issues/317>`_
* Implement init/fini functions for types (`#312 <https://github.com/ros2/rcl/issues/312>`_)
* Refactor goal state machine implementation and add unit tests (`#311 <https://github.com/ros2/rcl/issues/311>`_)
* Add missing visibilty control definitions (`#315 <https://github.com/ros2/rcl/issues/315>`_)
* Add rcl_action package and headers (`#307 <https://github.com/ros2/rcl/issues/307>`_)
* Contributors: Jacob Perron, Michel Hidalgo, William Woodall
