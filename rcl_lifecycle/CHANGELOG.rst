^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_lifecycle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.7.10 (2021-05-21)
-------------------

0.7.9 (2020-07-10)
------------------

0.7.8 (2019-12-10)
------------------

0.7.7 (2019-09-20)
------------------
* reset error message before setting a new one, embed the original one (`#501 <https://github.com/ros2/rcl/issues/501>`_) (`#505 <https://github.com/ros2/rcl/issues/505>`_)
* Contributors: Zachary Michaels

0.7.6 (2019-08-01)
------------------

0.7.5 (2019-06-12)
------------------

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
