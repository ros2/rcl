^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_yaml_param_parser
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.6.0 (2018-11-16)
------------------
* Updated to use new error handling API from rcutils (`#314 <https://github.com/ros2/rcl/issues/314>`_)
* Fixed FQN=//node_name when ns is / (`#299 <https://github.com/ros2/rcl/issues/299>`_)
* Fixed documentation issues (`#288 <https://github.com/ros2/rcl/issues/288>`_)
* Fixed to deallocate ret_val to avoid memory leak (`#278 <https://github.com/ros2/rcl/issues/278>`_)
* Contributors: Chris Ye, William Woodall, dhood

0.5.0 (2018-06-25)
------------------
* Added functions to parse YAML parameter files. (`#235 <https://github.com/ros2/rcl/issues/235>`_)
* Contributors: Shane Loretz, William Woodall, anup-pem, dhood
