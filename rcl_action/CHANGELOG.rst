^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_action
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.1.13 (2022-02-04)
-------------------
* Fix expired goals capacity of action server (`#931 <https://github.com/ros2/rcl/issues/931>`_) (`#957 <https://github.com/ros2/rcl/issues/957>`_)
* Contributors: spiralray

1.1.12 (2022-01-31)
-------------------

1.1.11 (2021-04-14)
-------------------
* Update quality declaration links (re: `ros2/docs.ros2.org#52 <https://github.com/ros2/docs.ros2.org/issues/52>`_) (`#910 <https://github.com/ros2/rcl/issues/910>`_)
* Contributors: Simon Honigmann

1.1.10 (2020-12-09)
-------------------
* rcl_action: address various clang static analysis fixes (`#864 <https://github.com/ros2/rcl/issues/864>`_) (`#875 <https://github.com/ros2/rcl/issues/875>`_)
* Update build.ros2.org links (`#868 <https://github.com/ros2/rcl/issues/868>`_)
* Update QD to QL 1 (`#867 <https://github.com/ros2/rcl/issues/867>`_)
* Update QD (`#843 <https://github.com/ros2/rcl/issues/843>`_)
* Contributors: Alejandro Hernández Cordero, Christophe Bedard, Jorge Perez, Stephen Brawner

1.1.9 (2020-11-03)
------------------
* Make sure to always check return values. (`#840 <https://github.com/ros2/rcl/issues/840>`_)
* Make sure to check the return value of rcl APIs. (`#838 <https://github.com/ros2/rcl/issues/838>`_)
* Add fault injection macros and unit tests to rcl_action (`#730 <https://github.com/ros2/rcl/issues/730>`_)
* Contributors: Chris Lalancette, brawner

1.1.8 (2020-10-07)
------------------
* Fix action client test failure on Windows by zero initializing pointers and freeing memory (`#717 <https://github.com/ros2/rcl/issues/717>`_) (`#820 <https://github.com/ros2/rcl/issues/820>`_)
* Use valid clock in case of issue in rcl_timer_init (`#795 <https://github.com/ros2/rcl/issues/795>`_) Store reference to rcl_clock_t instead of copy (`#797 <https://github.com/ros2/rcl/issues/797>`_) (`#805 <https://github.com/ros2/rcl/issues/805>`_)
* Contributors: Shane Loretz, Stephen Brawner

1.1.7 (2020-08-03)
------------------
* Removed doxygen warnings (`#712 <https://github.com/ros2/rcl/issues/712>`_) (`#724 <https://github.com/ros2/rcl/issues/724>`_)
* Contributors: Alejandro Hernández Cordero

1.1.6 (2020-07-07)
------------------
* Fixed doxygen warnings (`#677 <https://github.com/ros2/rcl/issues/677>`_) (`#696 <https://github.com/ros2/rcl/issues/696>`_)
* Contributors: Alejandro Hernández Cordero

1.1.5 (2020-06-03)
------------------

1.1.4 (2020-06-02)
------------------

1.1.3 (2020-06-01)
------------------
* Add Security Vulnerability Policy pointing to REP-2006 (`#661 <https://github.com/ros2/rcl/issues/661>`_)
* Address unused parameter warnings (`#666 <https://github.com/ros2/rcl/issues/666>`_)
* Increase test coverage of rcl_action (`#663 <https://github.com/ros2/rcl/issues/663>`_)
* Contributors: Chris Lalancette, Stephen Brawner

1.1.2 (2020-05-28)
------------------

1.1.1 (2020-05-26)
------------------

1.1.0 (2020-05-22)
------------------
* Update Quality Declaration for 1.0 (`#647 <https://github.com/ros2/rcl/issues/647>`_)
* Contributors: Stephen Brawner

1.0.0 (2020-05-12)
------------------

0.9.1 (2020-05-08)
------------------
* Included features (`#644 <https://github.com/ros2/rcl/issues/644>`_)
* Quality Declarations for rcl_action, rcl_lifecycle, yaml_parser (`#641 <https://github.com/ros2/rcl/issues/641>`_)
* Contributors: Alejandro Hernández Cordero, Stephen Brawner

0.9.0 (2020-04-29)
------------------
* Export targets in a addition to include directories / libraries (`#632 <https://github.com/ros2/rcl/issues/632>`_)
* Rename rosidl_generator_c namespace to rosidl_runtime_c (`#616 <https://github.com/ros2/rcl/issues/616>`_)
* Rename rosidl_generator_cpp namespace to rosidl_runtime_cpp (`#615 <https://github.com/ros2/rcl/issues/615>`_)
* Changed rosidl_generator_c/cpp to rosidl_runtime_c/cpp (`#588 <https://github.com/ros2/rcl/issues/588>`_)
* Changed build_depend and build_depend_export dependencies to depend (`#577 <https://github.com/ros2/rcl/issues/577>`_)
* Code style only: wrap after open parenthesis if not in one line (`#565 <https://github.com/ros2/rcl/issues/565>`_)
* Check if action status publisher is ready (`#541 <https://github.com/ros2/rcl/issues/541>`_)
* Contributors: Alejandro Hernández Cordero, Dirk Thomas, Tomoya Fujita

0.8.3 (2019-11-08)
------------------

0.8.2 (2019-10-23)
------------------
* Correct action server documentation (`#519 <https://github.com/ros2/rcl/issues/519>`_)
* Add mechanism to pass rmw impl specific payloads during pub/sub creation (`#513 <https://github.com/ros2/rcl/issues/513>`_)
* Contributors: Jacob Perron, William Woodall

0.8.1 (2019-10-08)
------------------

0.8.0 (2019-09-26)
------------------
* Fix rcl_action test_graph (`#504 <https://github.com/ros2/rcl/issues/504>`_)
* remove unused CMake code (`#475 <https://github.com/ros2/rcl/issues/475>`_)
* Contributors: Mikael Arguedas, ivanpauno

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
