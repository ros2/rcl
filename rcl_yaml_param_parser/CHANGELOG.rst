^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_yaml_param_parser
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.7.10 (2021-05-21)
-------------------

0.7.9 (2020-07-10)
------------------
* Added rcl yaml param parser Doxyfile. (`#701 <https://github.com/ros2/rcl/issues/701>`_)
* Contributors: Alejandro Hernández Cordero

0.7.8 (2019-12-10)
------------------
* Avoid C4703 error on UWP. (`#282 <https://github.com/ros2/rcl/issues/282>`_) (`#536 <https://github.com/ros2/rcl/issues/536>`_)
* Contributors: Sean Kelly

0.7.7 (2019-09-20)
------------------
* Increase MAX_STRING_SIZE (`#487 <https://github.com/ros2/rcl/issues/487>`_) (`#503 <https://github.com/ros2/rcl/issues/503>`_)
* Contributors: Zachary Michaels, Hyunseok Yang

0.7.6 (2019-08-01)
------------------
* Accept quoted int or float values as strings. (`#474 <https://github.com/ros2/rcl/issues/474>`_)
* Contributors: ivanpauno

0.7.5 (2019-06-12)
------------------
* Fix memory corruption when maximum number of parameters is exceeded (`#456 <https://github.com/ros2/rcl/issues/456>`_)
* Contributors: Jacob Perron

0.7.4 (2019-05-29)
------------------
* Allow empty strings if they are quoted. (`#450 <https://github.com/ros2/rcl/issues/450>`_)
* Contributors: Ralf Anton Beier

0.7.3 (2019-05-20)
------------------

0.7.2 (2019-05-08)
------------------

0.7.1 (2019-04-29)
------------------

0.7.0 (2019-04-14)
------------------
* Corrected bool reading from yaml files. (`#415 <https://github.com/ros2/rcl/issues/415>`_)
* Added launch along with launch_testing as test dependencies. (`#393 <https://github.com/ros2/rcl/issues/393>`_)
* Set symbol visibility to hidden for rcl. (`#391 <https://github.com/ros2/rcl/issues/391>`_)
* Contributors: Michel Hidalgo, Sachin Suresh Bhat, ivanpauno

0.6.4 (2019-01-11)
------------------

0.6.3 (2018-12-13)
------------------

0.6.2 (2018-12-13)
------------------

0.6.1 (2018-12-07)
------------------
* No changes.

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
