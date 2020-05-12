^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_lifecycle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
