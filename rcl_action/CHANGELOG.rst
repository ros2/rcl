^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_action
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.7.10 (2021-05-21)
-------------------
* Fix rcl_action test_graph (`#504 <https://github.com/ros2/rcl/issues/504>`_) (`#874 <https://github.com/ros2/rcl/issues/874>`_)
* Contributors: Ivan Pauno, Jacob Perron

0.7.9 (2020-07-10)
------------------
* Fixed doxygen warnings. (`#702 <https://github.com/ros2/rcl/issues/702>`_)
* Contributors: Alejandro Hernández Cordero

0.7.8 (2019-12-10)
------------------

0.7.7 (2019-09-20)
------------------

0.7.6 (2019-08-01)
------------------

0.7.5 (2019-06-12)
------------------

0.7.4 (2019-05-29)
------------------
* rcl_action - user friendly error messages for invalid transitions (`#448 <https://github.com/ros2/rcl/issues/448>`_)
* Contributors: Siddharth Kucheria

0.7.3 (2019-05-20)
------------------
* Fixed memory leaks in ``rcl_action`` unit tests (`#442 <https://github.com/ros2/rcl/issues/442>`_)
* Contributors: Prajakta Gokhale

0.7.2 (2019-05-08)
------------------
* Update graph test for change to rmw names and types struct (`#407 <https://github.com/ros2/rcl/issues/407>`_)
* New interfaces and their implementations for QoS features (`#408 <https://github.com/ros2/rcl/issues/408>`_)
* Add return code to CancelGoal service response (`#422 <https://github.com/ros2/rcl/issues/422>`_)
* Rmw preallocate (`#428 <https://github.com/ros2/rcl/issues/428>`_)
* Contributors: Jacob Perron, M. M, Michael Carroll

0.7.1 (2019-04-29)
------------------
* Renamed action state transitions (`#409 <https://github.com/ros2/rcl/issues/409>`_)
* Updated initialization of rmw_qos_profile_t struct instances. (`#416 <https://github.com/ros2/rcl/issues/416>`_)
* Contributors: Jacob Perron, M. M

0.7.0 (2019-04-14)
------------------
* Added Action graph API (`#411 <https://github.com/ros2/rcl/issues/411>`_)
* Updated to use ament_target_dependencies where possible. (`#400 <https://github.com/ros2/rcl/issues/400>`_)
* Fixed typo in Doxyfile. (`#398 <https://github.com/ros2/rcl/issues/398>`_)
* Updated tests to use separated action types. (`#340 <https://github.com/ros2/rcl/issues/340>`_)
* Fixed minor documentation issues. (`#397 <https://github.com/ros2/rcl/issues/397>`_)
* Set symbol visibility to hidden for rcl. (`#391 <https://github.com/ros2/rcl/issues/391>`_)
* Fixed rcl_action documentation. (`#380 <https://github.com/ros2/rcl/issues/380>`_)
* Removed now unused test executable . (`#382 <https://github.com/ros2/rcl/issues/382>`_)
* Removed unused action server option 'clock_type'. (`#382 <https://github.com/ros2/rcl/issues/382>`_)
* Set error message when there is an invalid goal transition. (`#382 <https://github.com/ros2/rcl/issues/382>`_)
* Updated to pass context to wait set, and fini rmw context (`#373 <https://github.com/ros2/rcl/issues/373>`_)
* Contributors: Dirk Thomas, Jacob Perron, Sachin Suresh Bhat, William Woodall, ivanpauno

0.6.4 (2019-01-11)
------------------
* Added parentheses around use of zerouuid macro (`#371 <https://github.com/ros2/rcl/issues/371>`_)
* Fixed logic that moves goal handles when one expires (`#360 <https://github.com/ros2/rcl/issues/360>`_)
* Updated to avoid timer period being set to 0 (`#359 <https://github.com/ros2/rcl/issues/359>`_)
* Contributors: Jacob Perron, Shane Loretz

0.6.3 (2018-12-13)
------------------

0.6.2 (2018-12-13)
------------------
* [rcl_action] Bugfix: arithmetic error
* Contributors: Jacob Perron

0.6.1 (2018-12-07)
------------------
* Added wait_for_action_server() for action clients (`#349 <https://github.com/ros2/rcl/issues/349>`_)
* Updated to adapt to action implicit changes (`#353 <https://github.com/ros2/rcl/issues/353>`_)
* Added action interaction tests (`#352 <https://github.com/ros2/rcl/issues/352>`_)
* Enabled test_action_communication to compile against available rmw. (`#351 <https://github.com/ros2/rcl/issues/351>`_)
* Changed UUID type in action msgs (`#338 <https://github.com/ros2/rcl/issues/338>`_)
* Added rcl_action_server_is_valid_except_context (`#348 <https://github.com/ros2/rcl/issues/348>`_)
* Updated to fini even if node context is invalid and reset error (`#346 <https://github.com/ros2/rcl/issues/346>`_)
* Added timer to action server to check expired goals + asan fixes (`#343 <https://github.com/ros2/rcl/issues/343>`_)
* Increased timeout for rcl_wait in action tests (`#344 <https://github.com/ros2/rcl/issues/344>`_)
* Refactored init to not be global (`#336 <https://github.com/ros2/rcl/issues/336>`_)
* Completes integration tests for action client/server (`#331 <https://github.com/ros2/rcl/issues/331>`_)
* Updated rcl_action_expire_goals() to output list of expired goals. (`#342 <https://github.com/ros2/rcl/issues/342>`_)
* Updated process_cancel_request to no longer change goal state (`#341 <https://github.com/ros2/rcl/issues/341>`_)
* Add action server implementation (`#323 <https://github.com/ros2/rcl/issues/323>`_)
* Contributors: Alexis Pojomovsky, Jacob Perron, Michel Hidalgo, Shane Loretz, William Woodall

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
