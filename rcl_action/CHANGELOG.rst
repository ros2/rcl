^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_action
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

10.2.4 (2025-09-30)
-------------------
* add rcl_action_goal_handle_is_abortable(). (`#1257 <https://github.com/ros2/rcl/issues/1257>`_)
* Contributors: Tomoya Fujita

10.2.3 (2025-07-29)
-------------------
* Fix Cmake deprecation (`#1249 <https://github.com/ros2/rcl/issues/1249>`_)
* Contributors: mosfet80

10.2.2 (2025-06-23)
-------------------

10.2.1 (2025-05-30)
-------------------

10.2.0 (2025-04-25)
-------------------

10.1.0 (2025-04-04)
-------------------
* Set envars to run tests with rmw_zenoh_cpp with multicast discovery (`#1218 <https://github.com/ros2/rcl/issues/1218>`_)
* No need to add public symbol visibility macros in implementation. (`#1213 <https://github.com/ros2/rcl/issues/1213>`_)
* fix 'rcl_action_server_configure_action_introspection': inconsistent dll linkage. (`#1212 <https://github.com/ros2/rcl/issues/1212>`_)
* Add new interfaces to enable intropsection for action (`#1207 <https://github.com/ros2/rcl/issues/1207>`_)
* Contributors: Barry Xu, Tomoya Fujita, yadunund

10.0.2 (2025-01-31)
-------------------
* fix(rcl_action): Allow to pass the timer to action during initialization (`#1201 <https://github.com/ros2/rcl/issues/1201>`_)
  * fix(timer): Use impl pointer in jump callback
  The interface description does not explicitly state that a
  rcl_timer_t may not be copied around. Therefore users may do this.
  By using a known never changing pointer in the callbacks, we avoid
  segfaults, even if the 'user' decides to copy the rcl_timer_t around.
* Added remapping resolution for action names (`#1170 <https://github.com/ros2/rcl/issues/1170>`_)
  * Added remapping resolution for action names
  * Fix cpplint/uncrustify
  * Simplified returned error codes in case name resolution failes.
  * Renamed action_name field to remapped_action_name.
  * Removed unnecessary resolved_action_name stack variable
  * Added tests for action name remapping.
  * Add tests for action name remapping using local arguments
  ---------
* Clean up error handling in many rcl{_action,_lifecycle} codepaths (`#1202 <https://github.com/ros2/rcl/issues/1202>`_)
  * Shorten the delay in test_action_server setup.
  Instead of waiting 250ms between setting up 10 goals
  (for at least 2.5 seconds), just wait 100ms which reduces
  this to 1 second.
  * Small style cleanups in test_action_server.cpp
  * Reset the error in rcl_node_type_cache_register_type().
  That is, if rcutils_hash_map_set() fails, it sets its
  own error, so overriding it with our own will cause a
  warning to print.  Make sure to clear it before setting
  our own.
  * Only unregister a clock jump callback if we have installed it.
  This avoids a warning on cleanup in rcl_timer_init2.
  * Record the return value from rcl_node_type_cache_register_type.
  Otherwise, in a failure situation we set the error but we
  actually return RCL_RET_OK to the upper layers, which is
  odd.
  * Get rid of completely unnecessary return value translation.
  This generated code was translating an RCL error to an
  RCL error, which doesn't make much sense.  Just remove
  the duplicate code.
  * Use the rcl_timer_init2 functionality to start the timer disabled.
  Rather than starting it enabled, and then immediately
  canceling it.
  * Don't overwrite the error from rcl_action_goal_handle_get_info()
  It already sets the error, so rcl_action_server_goal_exists()
  should not set it again.
  * Reset errors before setting new ones when checking action validity
  That way we avoid an ugly warning in the error paths.
  * Move the copying of the options earlier in rcl_subscription_init.
  That way when we go to cleanup in the "fail" case, the
  options actually exist and are valid.  This avoids an
  ugly warning during cleanup.
  * Make sure to set the error on failure of rcl_action_get\_##_service_name
  This makes it match the generated code for the action_client.
  * Reset the errors during RCUTILS_FAULT_INJECTION testing.
  That way subsequent failures won't print out ugly error
  strings.
  * Make sure to return errors in _rcl_parse_resource_match .
  That is, if rcl_lexer_lookahead2_expect() returns an error,
  we should pass that along to higher layers rather than
  just ignoring it.
  * Don't overwrite error by rcl_validate_enclave_name.
  It leads to ugly warnings.
  * Add acomment that rmw_validate_namespace_with_size sets the error
  * Make sure to reset error in rcl_node_type_cache_init.
  Otherwise we get a warning about overwriting the error
  from rcutils_hash_map_init.
  * Conditionally set error message in rcl_publisher_is_valid.
  Only when rcl_context_is_valid doesn't set the error.
  * Don't overwrite error from rcl_node_get_logger_name.
  It already sets the error in the failure case.
  * Make sure to reset errors when testing network flow endpoints.
  That's because some of the RMW implementations may not support
  this feature, and thus set errors.
  * Make sure to reset errors in rcl_expand_topic_name.
  That way we can set more useful errors for the upper
  layers.
  * Cleanup wait.c error handling.
  In particular, make sure to not overwrite errors as we
  get into error-handling paths, which should clean up
  warnings we get.
  * Make sure to reset errors in rcl_lifecycle tests.
  That way we won't get ugly "overwritten" warnings on
  subsequent tests.
  ---------
* Contributors: Chris Lalancette, Janosch Machowinski, Justus Braun

10.0.1 (2024-11-20)
-------------------

10.0.0 (2024-10-03)
-------------------
* Cleanup test_graph.cpp. (`#1193 <https://github.com/ros2/rcl/issues/1193>`_)
* Expect a minimum of two nodes to be alive in test_graph (`#1192 <https://github.com/ros2/rcl/issues/1192>`_)
* escalate RCL_RET_ACTION_xxx to 40XX. (`#1191 <https://github.com/ros2/rcl/issues/1191>`_)
* Fix NULL allocator and racy condition. (`#1188 <https://github.com/ros2/rcl/issues/1188>`_)
* Increased timeouts (`#1181 <https://github.com/ros2/rcl/issues/1181>`_)
* Change the starting time of the goal expiration timeout (`#1121 <https://github.com/ros2/rcl/issues/1121>`_)
* Contributors: Alejandro Hernández Cordero, Barry Xu, Chris Lalancette, Tomoya Fujita, Yadu

9.4.1 (2024-07-29)
------------------
* Increase the test_action_interaction timeouts. (`#1172 <https://github.com/ros2/rcl/issues/1172>`_)
  While I can't reproduce the problem locally, I suspect that
  waiting only 1 second for the entities to become ready isn't
  enough in all cases, particularly on Windows, with Connext,
  and when we are running in parallel with other tests.
  Thus, increase the timeout for the rcl_wait() in all of the
  test_action_interaction tests, which should hopefully be
  enough to make this always pass.
* Stop compiling rcl_action tests multiple times. (`#1165 <https://github.com/ros2/rcl/issues/1165>`_)
  We don't need to compile the tests once for each RMW;
  we can just compile it once and then use the RMW_IMPLEMENTATION
  environment variable to run the tests on the different RMWs.
  This speeds up compilation.
* Contributors: Chris Lalancette

9.4.0 (2024-06-17)
------------------

9.3.0 (2024-04-26)
------------------

9.2.1 (2024-04-16)
------------------
* Generate version header using ament_generate_version_header(..) (`#1141 <https://github.com/ros2/rcl/issues/1141>`_)
* Contributors: G.A. vd. Hoorn

9.2.0 (2024-03-28)
------------------
* add RCL_RET_TIMEOUT to action service response. (`#1138 <https://github.com/ros2/rcl/issues/1138>`_)
  * add RCL_RET_TIMEOUT to action service response.
  * address review comment.
  ---------
* Update quality declaration documents (`#1131 <https://github.com/ros2/rcl/issues/1131>`_)
* Contributors: Christophe Bedard, Tomoya Fujita

9.1.0 (2024-01-24)
------------------

9.0.0 (2023-12-26)
------------------

8.0.0 (2023-11-06)
------------------

7.3.0 (2023-10-09)
------------------

7.2.0 (2023-10-04)
------------------
* Remove most remaining uses of ament_target_dependencies. (`#1102 <https://github.com/ros2/rcl/issues/1102>`_)
* Contributors: Chris Lalancette

7.1.1 (2023-09-07)
------------------

7.1.0 (2023-08-21)
------------------

7.0.0 (2023-07-11)
------------------
* Add `~/get_type_description` service (rep2011) (`#1052 <https://github.com/ros2/rcl/issues/1052>`_)
* Modifies timers API to select autostart state (`#1004 <https://github.com/ros2/rcl/issues/1004>`_)
* Contributors: Eloy Briceno, Hans-Joachim Krauch

6.3.0 (2023-06-12)
------------------

6.2.0 (2023-06-07)
------------------

6.1.1 (2023-05-11)
------------------

6.1.0 (2023-04-28)
------------------

6.0.1 (2023-04-18)
------------------

6.0.0 (2023-04-12)
------------------
* doc update, ROS message accessibility depends on RMW implementation. (`#1043 <https://github.com/ros2/rcl/issues/1043>`_)
* Contributors: Tomoya Fujita

5.9.0 (2023-03-01)
------------------

5.8.0 (2023-02-23)
------------------

5.7.0 (2023-02-13)
------------------
* Update rcl to C++17. (`#1031 <https://github.com/ros2/rcl/issues/1031>`_)
* Contributors: Chris Lalancette

5.6.0 (2022-12-05)
------------------
* Reduce result_timeout to 10 seconds. (`#1012 <https://github.com/ros2/rcl/issues/1012>`_)
* [rolling] Update maintainers - 2022-11-07 (`#1017 <https://github.com/ros2/rcl/issues/1017>`_)
* Contributors: Audrow Nash, Chris Lalancette

5.5.0 (2022-11-02)
------------------

5.4.1 (2022-09-13)
------------------

5.4.0 (2022-04-29)
------------------

5.3.1 (2022-04-26)
------------------

5.3.0 (2022-04-05)
------------------

5.2.1 (2022-03-31)
------------------

5.2.0 (2022-03-24)
------------------

5.1.0 (2022-03-01)
------------------
* Add Events Executor (`#839 <https://github.com/ros2/rcl/issues/839>`_)
* Install includes it include/${PROJECT_NAME} (`#959 <https://github.com/ros2/rcl/issues/959>`_)
* Contributors: Shane Loretz, iRobot ROS

5.0.1 (2022-01-14)
------------------

5.0.0 (2021-12-14)
------------------
* Fix up documentation build for rcl_action when using rosdoc2 (`#937 <https://github.com/ros2/rcl/issues/937>`_)
* Contributors: Michel Hidalgo

4.0.0 (2021-09-16)
------------------
* Fix expired goals capacity of action server (`#931 <https://github.com/ros2/rcl/issues/931>`_)
* Contributors: spiralray

3.2.0 (2021-09-02)
------------------
* Wait for action server in rcl_action comm tests. (`#919 <https://github.com/ros2/rcl/issues/919>`_)
* Contributors: Michel Hidalgo

3.1.2 (2021-04-26)
------------------

3.1.1 (2021-04-12)
------------------

3.1.0 (2021-04-06)
------------------
* updating quality declaration links (re: `ros2/docs.ros2.org#52 <https://github.com/ros2/docs.ros2.org/issues/52>`_) (`#909 <https://github.com/ros2/rcl/issues/909>`_)
* Contributors: shonigmann

3.0.1 (2021-03-25)
------------------

3.0.0 (2021-03-23)
------------------

2.6.0 (2021-03-18)
------------------
* Don't expect RCL_RET_TIMEOUT to set an error string (`#900 <https://github.com/ros2/rcl/issues/900>`_)
* Add support for rmw_connextdds (`#895 <https://github.com/ros2/rcl/issues/895>`_)
* Contributors: Andrea Sorbini

2.5.2 (2021-02-05)
------------------
* Avoid setting error message twice. (`#887 <https://github.com/ros2/rcl/issues/887>`_)
* Contributors: Chen Lihui

2.5.1 (2021-01-25)
------------------

2.5.0 (2020-12-08)
------------------
* Address various clang static analysis fixes (`#864 <https://github.com/ros2/rcl/issues/864>`_)
* Update QDs to QL 1 (`#866 <https://github.com/ros2/rcl/issues/866>`_)
* Update QL (`#858 <https://github.com/ros2/rcl/issues/858>`_)
* Make sure to always check return values (`#840 <https://github.com/ros2/rcl/issues/840>`_)
* Update deprecated gtest macros (`#818 <https://github.com/ros2/rcl/issues/818>`_)
* Contributors: Alejandro Hernández Cordero, Audrow Nash, Chris Lalancette, Stephen Brawner

2.4.0 (2020-10-19)
------------------
* Make sure to check the return value of rcl APIs. (`#838 <https://github.com/ros2/rcl/issues/838>`_)
* Contributors: Chris Lalancette

2.3.0 (2020-10-19)
------------------
* Update maintainers (`#825 <https://github.com/ros2/rcl/issues/825>`_)
* Store reference to rcl_clock_t instead of copy (`#797 <https://github.com/ros2/rcl/issues/797>`_)
* Use valid clock in case of issue in rcl_timer_init (`#795 <https://github.com/ros2/rcl/issues/795>`_)
* Contributors: Ivan Santiago Paunovic, Shane Loretz, brawner

2.2.0 (2020-09-02)
------------------
* Add fault injection macros and unit tests to rcl_action (`#730 <https://github.com/ros2/rcl/issues/730>`_)
* Change some EXPECT_EQ to ASSERT_EQ in test_action_server. (`#759 <https://github.com/ros2/rcl/issues/759>`_)
* Contributors: Chris Lalancette, brawner

2.1.0 (2020-07-22)
------------------
* Removed doxygen warnings (`#712 <https://github.com/ros2/rcl/issues/712>`_)
* Address issue 716 by zero initializing pointers and freeing memory (`#717 <https://github.com/ros2/rcl/issues/717>`_)
* Contributors: Alejandro Hernández Cordero, brawner

2.0.0 (2020-07-09)
------------------
* Update quality declaration and coverage (`#674 <https://github.com/ros2/rcl/issues/674>`_)
* Contributors: Alejandro Hernández Cordero

1.2.0 (2020-06-18)
------------------
* Fixed doxygen warnings (`#677 <https://github.com/ros2/rcl/issues/677>`_)
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
