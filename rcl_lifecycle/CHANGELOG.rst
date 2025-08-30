^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_lifecycle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

10.2.3 (2025-07-29)
-------------------
* Fix Cmake deprecation (`#1249 <https://github.com/ros2/rcl/issues/1249>`_)
* Contributors: mosfet80

10.2.2 (2025-06-23)
-------------------

10.2.1 (2025-05-30)
-------------------
* introduce rcl_lifecycle_get_transition_label_by_id(). (`#1229 <https://github.com/ros2/rcl/issues/1229>`_)
* Contributors: Tomoya Fujita

10.2.0 (2025-04-25)
-------------------

10.1.0 (2025-04-04)
-------------------
* add rcl_print_transition_map. (`#1217 <https://github.com/ros2/rcl/issues/1217>`_)
* Enable test isolation in rcl_lifecycle (`#1216 <https://github.com/ros2/rcl/issues/1216>`_)
* Contributors: Scott K Logan, Tomoya Fujita

10.0.2 (2025-01-31)
-------------------
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
* Contributors: Chris Lalancette

10.0.1 (2024-11-20)
-------------------

10.0.0 (2024-10-03)
-------------------
* Fix NULL allocator and racy condition. (`#1188 <https://github.com/ros2/rcl/issues/1188>`_)
* Fix typo in rcl_lifecycle_com_interface_t doc (`#1174 <https://github.com/ros2/rcl/issues/1174>`_)
* Contributors: Christophe Bedard, Tomoya Fujita

9.4.1 (2024-07-29)
------------------
* Fix a memory leak in test_rcl_lifecycle. (`#1173 <https://github.com/ros2/rcl/issues/1173>`_)
  This one came about probably as a result of a bad merge.
  But essentially we were forcing the srv_change_state
  com_interface to be nullptr, but forgetting to save off
  the old pointer early enough.  Thus, we could never restore
  the old one before we went to "fini", and the memory would
  be leaked.  Fix this by remembering the impl pointer earlier.
* Contributors: Chris Lalancette

9.4.0 (2024-06-17)
------------------

9.3.0 (2024-04-26)
------------------
* Fixed warnings - strict-prototypes (`#1148 <https://github.com/ros2/rcl/issues/1148>`_)
* Contributors: Alejandro Hernández Cordero

9.2.1 (2024-04-16)
------------------
* Generate version header using ament_generate_version_header(..) (`#1141 <https://github.com/ros2/rcl/issues/1141>`_)
* Contributors: G.A. vd. Hoorn

9.2.0 (2024-03-28)
------------------
* Update quality declaration documents (`#1131 <https://github.com/ros2/rcl/issues/1131>`_)
* Contributors: Christophe Bedard

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

6.3.0 (2023-06-12)
------------------
* Use TRACETOOLS\_ prefix for tracepoint-related macros (`#1058 <https://github.com/ros2/rcl/issues/1058>`_)
* Contributors: Christophe Bedard

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
* [rolling] Update maintainers - 2022-11-07 (`#1017 <https://github.com/ros2/rcl/issues/1017>`_)
* Contributors: Audrow Nash

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
* Install includes it include/${PROJECT_NAME} (`#959 <https://github.com/ros2/rcl/issues/959>`_)
* Contributors: Shane Loretz

5.0.1 (2022-01-14)
------------------
* [rcl_lifecycle] Do not share transition event message between nodes (`#956 <https://github.com/ros2/rcl/issues/956>`_)
* Contributors: Ivan Santiago Paunovic

5.0.0 (2021-12-14)
------------------
* Update maintainers to Ivan Paunovic and William Woodall (`#952 <https://github.com/ros2/rcl/issues/952>`_)
* Fix up documentation build for rcl_lifecycle when using rosdoc2 (`#938 <https://github.com/ros2/rcl/issues/938>`_)
* Contributors: Audrow Nash, Michel Hidalgo

4.0.0 (2021-09-16)
------------------

3.2.0 (2021-09-02)
------------------
* Rename variable to fix name shadowing warning (`#929 <https://github.com/ros2/rcl/issues/929>`_)
* Contributors: Alberto Soragna

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
* make rcl_lifecycle_com_interface optional in lifecycle nodes (`#882 <https://github.com/ros2/rcl/issues/882>`_)
* Contributors: Karsten Knese

2.6.0 (2021-03-18)
------------------

2.5.2 (2021-02-05)
------------------

2.5.1 (2021-01-25)
------------------

2.5.0 (2020-12-08)
------------------
* Update QDs to QL 1 (`#866 <https://github.com/ros2/rcl/issues/866>`_)
* Update QL (`#858 <https://github.com/ros2/rcl/issues/858>`_)
* Make sure to always check return values (`#840 <https://github.com/ros2/rcl/issues/840>`_)
* Update tracetools QL and add to rcl_lifecycle's QD (`#845 <https://github.com/ros2/rcl/issues/845>`_)
* Add compiler warnings (`#830 <https://github.com/ros2/rcl/issues/830>`_)
* Contributors: Alejandro Hernández Cordero, Audrow Nash, Chris Lalancette, Christophe Bedard, Stephen Brawner

2.4.0 (2020-10-19)
------------------
* Make sure to check the return value of rcl APIs. (`#838 <https://github.com/ros2/rcl/issues/838>`_)
* Contributors: Chris Lalancette

2.3.0 (2020-10-19)
------------------
* Add lifecycle node state transition instrumentation (`#804 <https://github.com/ros2/rcl/issues/804>`_)
* Update maintainers (`#825 <https://github.com/ros2/rcl/issues/825>`_)
* Improve error messages in rcl_lifecycle (`#742 <https://github.com/ros2/rcl/issues/742>`_)
* Fix test_rcl_lifecycle (`#788 <https://github.com/ros2/rcl/issues/788>`_)
* Contributors: Christophe Bedard, Ivan Santiago Paunovic, Lei Liu, brawner

2.2.0 (2020-09-02)
------------------
* Add fault injection macros and unit tests to rcl_lifecycle (`#731 <https://github.com/ros2/rcl/issues/731>`_)
* Remove std::cout line from test_rcl_lifecycle.cpp (`#773 <https://github.com/ros2/rcl/issues/773>`_)
* Set transition_map->states/transition size to 0 on fini (`#729 <https://github.com/ros2/rcl/issues/729>`_)
* Contributors: brawner

2.1.0 (2020-07-22)
------------------
* Topic fix rcl lifecycle test issue (`#715 <https://github.com/ros2/rcl/issues/715>`_)
* Removed doxygen warnings (`#712 <https://github.com/ros2/rcl/issues/712>`_)
* Contributors: Alejandro Hernández Cordero, Barry Xu

2.0.0 (2020-07-09)
------------------
* Update quality declaration and coverage (`#674 <https://github.com/ros2/rcl/issues/674>`_)
* Contributors: Alejandro Hernández Cordero

1.2.0 (2020-06-18)
------------------

1.1.5 (2020-06-03)
------------------

1.1.4 (2020-06-02)
------------------

1.1.3 (2020-06-01)
------------------
* Add Security Vulnerability Policy pointing to REP-2006 (`#661 <https://github.com/ros2/rcl/issues/661>`_)
* Contributors: Chris Lalancette

1.1.2 (2020-05-28)
------------------
* Allow transition start and goal states to be null (`#662 <https://github.com/ros2/rcl/issues/662>`_)
* Contributors: Karsten Knese

1.1.1 (2020-05-26)
------------------
* Increase rcl_lifecycle test coverage and add more safety checks (`#649 <https://github.com/ros2/rcl/issues/649>`_)
* Contributors: Stephen Brawner

1.1.0 (2020-05-22)
------------------
* Update Quality Declaration for 1.0 (`#647 <https://github.com/ros2/rcl/issues/647>`_)
* Contributors: brawner

1.0.0 (2020-05-12)
------------------

0.9.1 (2020-05-08)
------------------
* Included features (`#644 <https://github.com/ros2/rcl/issues/644>`_)
* Quality Declarations for rcl_action, rcl_lifecycle, yaml_parser (`#641 <https://github.com/ros2/rcl/issues/641>`_)
* Contributors: Alejandro Hernández Cordero, Stephen Brawner

0.9.0 (2020-04-29)
------------------
* Added rcl_lifecycle Doxyfile (`#633 <https://github.com/ros2/rcl/issues/633>`_)
* Export targets in a addition to include directories / libraries (`#635 <https://github.com/ros2/rcl/issues/635>`_)
* Added documentation (`#622 <https://github.com/ros2/rcl/issues/622>`_)
* Fixed argument name in rcl_lifecycle.h (`#626 <https://github.com/ros2/rcl/issues/626>`_)
* Rename rosidl_generator_c namespace to rosidl_runtime_c (`#616 <https://github.com/ros2/rcl/issues/616>`_)
* Changed rosidl_generator_c/cpp to rosidl_runtime_c/cpp (`#588 <https://github.com/ros2/rcl/issues/588>`_)
* Removed rmw_implementation from package.xml (`#575 <https://github.com/ros2/rcl/issues/575>`_)
* Code style only: wrap after open parenthesis if not in one line (`#565 <https://github.com/ros2/rcl/issues/565>`_)
* Free valid_transitions for all states (`#537 <https://github.com/ros2/rcl/issues/537>`_)
* Contributors: Alejandro Hernández Cordero, Dirk Thomas, Víctor Mayoral Vilches

0.8.3 (2019-11-08)
------------------

0.8.2 (2019-10-23)
------------------

0.8.1 (2019-10-08)
------------------

0.8.0 (2019-09-26)
------------------
* reset error message before setting a new one, embed the original one (`#501 <https://github.com/ros2/rcl/issues/501>`_)
* Contributors: Dirk Thomas

0.7.4 (2019-05-29)
------------------

0.7.3 (2019-05-20)
------------------

0.7.2 (2019-05-08)
------------------
* Rmw preallocate (`#428 <https://github.com/ros2/rcl/issues/428>`_)
* Contributors: Michael Carroll

0.7.1 (2019-04-29)
------------------

0.7.0 (2019-04-14)
------------------
* Updated to use ament_target_dependencies where possible. (`#400 <https://github.com/ros2/rcl/issues/400>`_)
* Set symbol visibility to hidden for rcl. (`#391 <https://github.com/ros2/rcl/issues/391>`_)
* Contributors: Sachin Suresh Bhat, ivanpauno

0.6.4 (2019-01-11)
------------------

0.6.3 (2018-12-13)
------------------

0.6.2 (2018-12-13)
------------------

0.6.1 (2018-12-07)
------------------
* Refactored init to not be global (`#336 <https://github.com/ros2/rcl/issues/336>`_)
* Contributors: William Woodall

0.6.0 (2018-11-16)
------------------
* Updated use new error handling API from rcutils (`#314 <https://github.com/ros2/rcl/issues/314>`_)
* Deleted TRANSITION_SHUTDOWN (`#313 <https://github.com/ros2/rcl/issues/313>`_)
* Refactored lifecycle (`#298 <https://github.com/ros2/rcl/issues/298>`_)
  * no static initialization of states anymore
  * make transition labels more descriptive
  * introduce labeled keys
  * define default transition keys
  * fix memory management
  * introduce service for transition graph
  * export transition keys
  * remove keys, transition id unique, label ambiguous
  * semicolon for macro call
* Added macro semicolons (`#303 <https://github.com/ros2/rcl/issues/303>`_)
* Fixed naming of configure_error transition (`#292 <https://github.com/ros2/rcl/issues/292>`_)
* Removed use of uninitialized CMake var (`#268 <https://github.com/ros2/rcl/issues/268>`_)
* Fixed rosidl dependencies (`#265 <https://github.com/ros2/rcl/issues/265>`_)
  * [rcl_lifecycle] remove rosidl deps as this package doesnt generate any messages
  * depend on rosidl_generator_c
* Contributors: Chris Lalancette, Dirk Thomas, Karsten Knese, Mikael Arguedas, William Woodall

0.5.0 (2018-06-25)
------------------
* Updated code to use private substitution (``~``) in lifecycle topics and services (`#260 <https://github.com/ros2/rcl/issues/260>`_)
  * use ~/<topic> rather than manually constructing topics/services
  * use check argument for null macros
* Fixed potential segmentation fault due to nullptr dereference (`#202 <https://github.com/ros2/rcl/issues/202>`_)
  * Signed-off-by: Ethan Gao <ethan.gao@linux.intel.com>
* Contributors: Dirk Thomas, Ethan Gao, Michael Carroll, William Woodall
