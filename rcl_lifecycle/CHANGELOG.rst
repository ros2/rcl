^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_lifecycle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.6.0 (2018-11-16)
-----------
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
