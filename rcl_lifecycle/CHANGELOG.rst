^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_lifecycle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
