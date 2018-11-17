^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl
^^^^^^^^^^^^^^^^^^^^^^^^^

0.6.0 (2018-11-16)
------------------
* Updated to expand node_secure_root using local_namespace (`#300 <https://github.com/ros2/rcl/issues/300>`_)
* Moved stdatomic helper to rcutils (`#324 <https://github.com/ros2/rcl/issues/324>`_)
* Added subfolder argument to the ROSIDL_GET_SRV_TYPE_SUPPORT macro (`#322 <https://github.com/ros2/rcl/issues/322>`_)
* Updated to use new error handling API from rcutils (`#314 <https://github.com/ros2/rcl/issues/314>`_)
* Fixed minor documentation issues (`#305 <https://github.com/ros2/rcl/issues/305>`_)
* Added macro semicolons (`#303 <https://github.com/ros2/rcl/issues/303>`_)
* Added Rcl timer with ros time (`#286 <https://github.com/ros2/rcl/issues/286>`_)
* Updated to ensure that timer period is non-negative (`#295 <https://github.com/ros2/rcl/issues/295>`_)
* Fixed calculation of next timer call (`#291 <https://github.com/ros2/rcl/issues/291>`_)
* Updated to null deallocated jump callbacks (`#294 <https://github.com/ros2/rcl/issues/294>`_)
* Included namespaces in get_node_names. (`#287 <https://github.com/ros2/rcl/issues/287>`_)
* Fixed documentation issues (`#288 <https://github.com/ros2/rcl/issues/288>`_)
* Updated to check if pointers are null before calling memset (`#290 <https://github.com/ros2/rcl/issues/290>`_)
* Added multiple time jump callbacks to clock (`#284 <https://github.com/ros2/rcl/issues/284>`_)
* Consolidated wait set functions (`#285 <https://github.com/ros2/rcl/issues/285>`_)
  * Consolidate functions to clear wait set
  Added rcl_wait_set_clear()
  Added rcl_wait_set_resize()
  Removed
  rcl_wait_set_clear_subscriptions()
  rcl_wait_set_clear_guard_conditions()
  rcl_wait_set_clear_clients()
  rcl_wait_set_clear_services()
  rcl_wait_set_clear_timers()
  rcl_wait_set_resize_subscriptions()
  rcl_wait_set_resize_guard_conditions()
  rcl_wait_set_resize_timers()
  rcl_wait_set_resize_clients()
  rcl_wait_set_resize_services()
* ROS clock storage initially set to zero (`#283 <https://github.com/ros2/rcl/issues/283>`_)
* Fixed issue with deallocation of parameter_files (`#279 <https://github.com/ros2/rcl/issues/279>`_)
* Update to initialize memory before sending a message (`#277 <https://github.com/ros2/rcl/issues/277>`_)
* Set error message when clock type is not ROS_TIME (`#275 <https://github.com/ros2/rcl/issues/275>`_)
* Copy allocator passed in to clock init (`#274 <https://github.com/ros2/rcl/issues/274>`_)
* Update to initialize timer with clock (`#272 <https://github.com/ros2/rcl/issues/272>`_)
* Updated to use test_msgs instead of std_msgs in tests (`#270 <https://github.com/ros2/rcl/issues/270>`_)
* Added regression test for node:__ns remapping (`#263 <https://github.com/ros2/rcl/issues/263>`_)
* Updated to support Uncrustify 0.67 (`#266 <https://github.com/ros2/rcl/issues/266>`_)
* Contributors: Chris Lalancette, Chris Ye, Dirk Thomas, Jacob Perron, Michael Carroll, Mikael Arguedas, Ruffin, Shane Loretz, William Woodall, dhood

0.5.0 (2018-06-25)
------------------
* Updated code to only use ``rcutils_allocator_t`` and not use system memory functions directly. (`#261 <https://github.com/ros2/rcl/issues/261>`_)
* Changed code to use ``rcutils_format_string()`` rather than ``malloc`` and ``rcutils_snprintf()`` (`#240 <https://github.com/ros2/rcl/issues/240>`_)
* Added functions for dealing with serialized messages. (`#170 <https://github.com/ros2/rcl/issues/170>`_)
* Updated to use ``test_msgs`` instead of ``example_interfaces``. (`#259 <https://github.com/ros2/rcl/issues/259>`_)
* Added regression test for the Connext specific 'wrong type writer' error. (`#257 <https://github.com/ros2/rcl/issues/257>`_)
* Added the ability to set the default logger level from command line. (`#256 <https://github.com/ros2/rcl/issues/256>`_)
* Refactored the ``memory_tools`` testing API to ``osrf_testing_tools_cpp`` (`#238 <https://github.com/ros2/rcl/issues/238>`_)
* Added support for passing YAML parameter files via the command line arguments.  (`#253 <https://github.com/ros2/rcl/issues/253>`_)
* Migrated existing uses of ``launch`` to use the same API in it's new API ``launch.legacy``. (`#250 <https://github.com/ros2/rcl/issues/250>`_)
* Added a printed warning if non-FQN namespace remapping is passed. (`#248 <https://github.com/ros2/rcl/issues/248>`_)
* Made some changes toward MISRA C compliance. (`#229 <https://github.com/ros2/rcl/issues/229>`_)
* Changed ``rcl_node_init()`` so that it now copies node options passed into it (`#231 <https://github.com/ros2/rcl/issues/231>`_)
* Fixed some memory leaks in ``test_arguments`` (`#230 <https://github.com/ros2/rcl/issues/230>`_)
* Extended static remapping feature with support for the url scheme (`#227 <https://github.com/ros2/rcl/issues/227>`_)
* Made a change to force ``rcl_arguments_t`` to be zero initialized. (`#225 <https://github.com/ros2/rcl/issues/225>`_)
* Updated documentation for ``rmw_get_node_names()`` to mention the potential for null values (`#214 <https://github.com/ros2/rcl/issues/214>`_)
* Fix an issue with signed time difference. (`#224 <https://github.com/ros2/rcl/issues/224>`_)
* Changed library export order to fix static linking (`#216 <https://github.com/ros2/rcl/issues/216>`_)
* Implemented static remapping over command line arguments (`#217 <https://github.com/ros2/rcl/issues/217>`_ and `#221 <https://github.com/ros2/rcl/issues/221>`_)
* Added a sized validation function for the topic name as ``rcl_validate_topic_name_with_size()`` (`#220 <https://github.com/ros2/rcl/issues/220>`_)
* Added a logger name and stored it in the rcl node structure (`#212 <https://github.com/ros2/rcl/issues/212>`_)
* Changed ``rcutils_time_point_value_t`` type from ``uint64_t`` to ``int64_t`` (`#208 <https://github.com/ros2/rcl/issues/208>`_)
* Fixed a potential bug by resetting the ``RMWCount`` when using the ``DEALLOC`` macro on rmw storage of a wait set (`#209 <https://github.com/ros2/rcl/issues/209>`_ and `#211 <https://github.com/ros2/rcl/issues/211>`_)
  * Signed-off-by: jwang <jing.j.wang@intel.com>
* Fixed a potential bug by resetting ``wait_set`` type index in the ``SET_RESIZE`` macro (`#207 <https://github.com/ros2/rcl/issues/207>`_)
  * Signed-off-by: jwang <jing.j.wang@intel.com>
* Removed a slash behind ``SET_CLEAR`` MACRO (`#206 <https://github.com/ros2/rcl/issues/206>`_)
  * Signed-off-by: jwang <jing.j.wang@intel.com>
* Changed rmw result validation string to not ever return nullptr (`#193 <https://github.com/ros2/rcl/issues/193>`_)
  * Signed-off-by: Ethan Gao <ethan.gao@linux.intel.com>
* Clarified that ``rcl_take_response()`` populates the ``request_header`` (`#205 <https://github.com/ros2/rcl/issues/205>`_)
* Removed a now obsolete connext workaround (`#203 <https://github.com/ros2/rcl/issues/203>`_)
* Fixed a potential segmentation fault due to a nullptr dereference (`#202 <https://github.com/ros2/rcl/issues/202>`_)
  * Signed-off-by: Ethan Gao <ethan.gao@linux.intel.com>
* Contributors: Dirk Thomas, Ethan Gao, Karsten Knese, Michael Carroll, Mikael Arguedas, Shane Loretz, William Woodall, dhood, jwang11, serge-nikulin
