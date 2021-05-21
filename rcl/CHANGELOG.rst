^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl
^^^^^^^^^^^^^^^^^^^^^^^^^

0.7.10 (2021-05-21)
-------------------

0.7.9 (2020-07-10)
------------------
* Fixed doxygen warnings. (`#702 <https://github.com/ros2/rcl/issues/702>`_)
* Allow get_node_names to return result in any order. (`#592 <https://github.com/ros2/rcl/issues/592>`_)
* Don't check history depth if RMW_QOS_POLICY_HISTORY_KEEP_ALL. (`#595 <https://github.com/ros2/rcl/issues/595>`_)
* Contributors: Alejandro Hernández Cordero, Dan Rose

0.7.8 (2019-12-10)
------------------
* Set allocator before goto fail. (`#540 <https://github.com/ros2/rcl/issues/540>`_)
* Contributors: Borja Outerelo

0.7.7 (2019-09-20)
------------------

0.7.6 (2019-08-01)
------------------

0.7.5 (2019-06-12)
------------------

0.7.4 (2019-05-29)
------------------
* Fix tests now that FastRTPS correctly reports that liveliness is not supported (`#452 <https://github.com/ros2/rcl/issues/452>`_)
* In test_events, wait for discovery to be complete bidirectionally before moving on (`#451 <https://github.com/ros2/rcl/issues/451>`_)
* fix leak in test_service (`#447 <https://github.com/ros2/rcl/issues/447>`_)
* fix leak in test_guard_condition (`#446 <https://github.com/ros2/rcl/issues/446>`_)
* fix leak in test_get_actual_qos (`#445 <https://github.com/ros2/rcl/issues/445>`_)
* fix leak in test_expand_topic_name (`#444 <https://github.com/ros2/rcl/issues/444>`_)
* Contributors: Abby Xu, Emerson Knapp

0.7.3 (2019-05-20)
------------------
* Fixed memory leak in ``test_client`` (`#443 <https://github.com/ros2/rcl/issues/443>`_)
* Fixed memory leaks in ``test_wait.cpp`` (`#439 <https://github.com/ros2/rcl/issues/439>`_)
* Fixed memory leak in ``test_context`` (`#441 <https://github.com/ros2/rcl/issues/441>`_)
* Fixed memory leak in ``test_init`` (`#440 <https://github.com/ros2/rcl/issues/440>`_)
* Enabled rcl ``test_events`` unit tests on macOS (`#433 <https://github.com/ros2/rcl/issues/433>`_)
* Enabled deadline tests for FastRTPS (`#438 <https://github.com/ros2/rcl/issues/438>`_)
* Corrected use of ``launch_testing.assert.assertExitCodes`` (`#437 <https://github.com/ros2/rcl/issues/437>`_)
* Reverted "Changes the default 3rd party logger from rcl_logging_noop to… (`#436 <https://github.com/ros2/rcl/issues/436>`_)
* Fixed memory leaks in ``test_security_directory`` (`#420 <https://github.com/ros2/rcl/issues/420>`_)
* Fixed a memory leak in rcl context fini (`#434 <https://github.com/ros2/rcl/issues/434>`_)
* Contributors: Abby Xu, Cameron Evans, Chris Lalancette, Dirk Thomas, M. M, ivanpauno

0.7.2 (2019-05-08)
------------------
* Changes the default 3rd party logger from rcl_logging_noop to rcl_logging_log4cxx (`#425 <https://github.com/ros2/rcl/issues/425>`_)
* fix leak in node.c (`#424 <https://github.com/ros2/rcl/issues/424>`_)
* Add new RCL_RET_UNSUPPORTED (`#432 <https://github.com/ros2/rcl/issues/432>`_)
* New interfaces and their implementations for QoS features (`#408 <https://github.com/ros2/rcl/issues/408>`_)
* Add an allocator to the external logging initialization. (`#430 <https://github.com/ros2/rcl/issues/430>`_)
* fix buffer overflow in test_security_dir (`#423 <https://github.com/ros2/rcl/issues/423>`_)
* Rmw preallocate (`#428 <https://github.com/ros2/rcl/issues/428>`_)
* Use new test interface definitions (`#427 <https://github.com/ros2/rcl/pull/427>`_)
* Migrate launch tests to new launch_testing features & API (`#405 <https://github.com/ros2/rcl/issues/405>`_)
* Fix argument passed to logging macros (`#421 <https://github.com/ros2/rcl/issues/421>`_)
* Make sure to initialize the bool field. (`#426 <https://github.com/ros2/rcl/issues/426>`_)
* Contributors: Abby Xu, Chris Lalancette, Emerson Knapp, Jacob Perron, M. M, Michael Carroll, Michel Hidalgo, Nick Burek, Thomas Moulard

0.7.1 (2019-04-29)
------------------
* Replaced reinterperet_cast with static_cast. (`#410 <https://github.com/ros2/rcl/issues/410>`_)
* Fixed leak in __wait_set_clean_up. (`#418 <https://github.com/ros2/rcl/issues/418>`_)
* Updated initialization of rmw_qos_profile_t struct instances. (`#416 <https://github.com/ros2/rcl/issues/416>`_)
* Contributors: Dirk Thomas, M. M, jhdcs

0.7.0 (2019-04-14)
------------------
* Added more test cases for graph API + fix bug. (`#404 <https://github.com/ros2/rcl/issues/404>`_)
* Fixed missing include. (`#413 <https://github.com/ros2/rcl/issues/413>`_)
* Updated to use pedantic. (`#412 <https://github.com/ros2/rcl/issues/412>`_)
* Added function to get publisher actual qos settings. (`#406 <https://github.com/ros2/rcl/issues/406>`_)
* Refactored graph API docs. (`#401 <https://github.com/ros2/rcl/issues/401>`_)
* Updated to use ament_target_dependencies where possible. (`#400 <https://github.com/ros2/rcl/issues/400>`_)
* Fixed regression around fully qualified node name. (`#402 <https://github.com/ros2/rcl/issues/402>`_)
* Added function rcl_names_and_types_init. (`#403 <https://github.com/ros2/rcl/issues/403>`_)
* Fixed uninitialize sequence number of client. (`#395 <https://github.com/ros2/rcl/issues/395>`_)
* Added launch along with launch_testing as test dependencies. (`#393 <https://github.com/ros2/rcl/issues/393>`_)
* Set symbol visibility to hidden for rcl. (`#391 <https://github.com/ros2/rcl/issues/391>`_)
* Updated to split test_token to avoid compiler note. (`#392 <https://github.com/ros2/rcl/issues/392>`_)
* Dropped legacy launch API usage. (`#387 <https://github.com/ros2/rcl/issues/387>`_)
* Improved security directory lookup. (`#332 <https://github.com/ros2/rcl/issues/332>`_)
* Enforce non-null argv values on rcl_init(). (`#388 <https://github.com/ros2/rcl/issues/388>`_)
* Removed incorrect argument documentation. (`#361 <https://github.com/ros2/rcl/issues/361>`_)
* Changed error to warning for multiple loggers. (`#384 <https://github.com/ros2/rcl/issues/384>`_)
* Added rcl_node_get_fully_qualified_name. (`#255 <https://github.com/ros2/rcl/issues/255>`_)
* Updated rcl_remap_t to use the PIMPL pattern. (`#377 <https://github.com/ros2/rcl/issues/377>`_)
* Fixed documentation typo. (`#376 <https://github.com/ros2/rcl/issues/376>`_)
* Removed test circumvention now that a bug is fixed in rmw_opensplice. (`#368 <https://github.com/ros2/rcl/issues/368>`_)
* Updated to pass context to wait set, and fini rmw context. (`#373 <https://github.com/ros2/rcl/issues/373>`_)
* Updated to publish logs to Rosout. (`#350 <https://github.com/ros2/rcl/issues/350>`_)
* Contributors: AAlon, Dirk Thomas, Jacob Perron, M. M, Michael Carroll, Michel Hidalgo, Mikael Arguedas, Nick Burek, RARvolt, Ross Desmond, Sachin Suresh Bhat, Shane Loretz, William Woodall, ivanpauno

0.6.4 (2019-01-11)
------------------
* Added method for accessing rmw_context from rcl_context (`#372 <https://github.com/ros2/rcl/issues/372>`_)
* Added guard against bad allocation when calling rcl_arguments_copy() (`#367 <https://github.com/ros2/rcl/issues/367>`_)
* Updated to ensure that context instance id storage is aligned correctly (`#365 <https://github.com/ros2/rcl/issues/365>`_)
* Fixed error from uncrustify v0.68 (`#364 <https://github.com/ros2/rcl/issues/364>`_)
* Contributors: Jacob Perron, William Woodall, sgvandijk

0.6.3 (2018-12-13)
------------------
* Set rmw_wait timeout using ros timers too (`#357 <https://github.com/ros2/rcl/issues/357>`_)
* Contributors: Shane Loretz

0.6.2 (2018-12-13)
------------------
* Updated docs about possibility of rcl_take not taking (`#356 <https://github.com/ros2/rcl/issues/356>`_)
* Bugfix: ensure NULL timeout is passed to rmw_wait() when min_timeout is not set
  Otherwise, there is a risk of integer overflow (e.g. in rmw_fastrtps) and rmw_wait() will wake immediately.
* Contributors: Jacob Perron, William Woodall

0.6.1 (2018-12-07)
------------------
* Added new cli parameters for configuring the logging. (`#327 <https://github.com/ros2/rcl/issues/327>`_)
* Added node graph api to rcl. (`#333 <https://github.com/ros2/rcl/issues/333>`_)
* Fixed compiler warning in clang (`#345 <https://github.com/ros2/rcl/issues/345>`_)
* Refactored init to not be global (`#336 <https://github.com/ros2/rcl/issues/336>`_)
* Methods to retrieve matched counts on pub/sub. (`#326 <https://github.com/ros2/rcl/issues/326>`_)
* Updated to output index in container when adding an entity to a wait set. (`#335 <https://github.com/ros2/rcl/issues/335>`_)
* Contributors: Jacob Perron, Michael Carroll, Nick Burek, Ross Desmond, William Woodall

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
