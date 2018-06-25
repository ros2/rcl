^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl
^^^^^^^^^^^^^^^^^^^^^^^^^

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
