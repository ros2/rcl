^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl_yaml_param_parser
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.0.0 (2020-05-12)
------------------

0.9.1 (2020-05-08)
------------------
* Included features (`#644 <https://github.com/ros2/rcl/issues/644>`_)
* Quality Declarations for rcl_action, rcl_lifecycle, yaml_parser (`#641 <https://github.com/ros2/rcl/issues/641>`_)
* Contributors: Alejandro Hernández Cordero, brawner

0.9.0 (2020-04-29)
------------------
* Added rcl yaml param parser doxyfile (`#634 <https://github.com/ros2/rcl/issues/634>`_)
* Fixed rcl_yaml_param_parser package description (`#637 <https://github.com/ros2/rcl/issues/637>`_)
* Fix usage to not expose underlying yaml (`#630 <https://github.com/ros2/rcl/issues/630>`_)
* Export targets in a addition to include directories / libraries (`#621 <https://github.com/ros2/rcl/issues/621>`_)
* Remove usage of undefined CMake variable (`#620 <https://github.com/ros2/rcl/issues/620>`_)
* Fix memory leaks (`#564 <https://github.com/ros2/rcl/issues/564>`_)
* Code style only: wrap after open parenthesis if not in one line (`#565 <https://github.com/ros2/rcl/issues/565>`_)
* Contributors: Alejandro Hernández Cordero, Dirk Thomas, y-okumura-isp

0.8.3 (2019-11-08)
------------------

0.8.2 (2019-10-23)
------------------
* Specify test working directory (`#529 <https://github.com/ros2/rcl/issues/529>`_)
* Remove the maximum string size. (`#524 <https://github.com/ros2/rcl/issues/524>`_)
* Contributors: Chris Lalancette, Dan Rose

0.8.1 (2019-10-08)
------------------

0.8.0 (2019-09-26)
------------------
* Enable incremental parameter yaml file parsing. (`#507 <https://github.com/ros2/rcl/issues/507>`_)
* Support parameter overrides and remap rules flags on command line (`#483 <https://github.com/ros2/rcl/issues/483>`_)
* Increase MAX_STRING_SIZE (`#487 <https://github.com/ros2/rcl/issues/487>`_)
* include actual size in error message (`#490 <https://github.com/ros2/rcl/issues/490>`_)
* Avoid C4703 error on UWP (`#282 <https://github.com/ros2/rcl/issues/282>`_)
* [YAML Parser] Support parameter value parsing (`#471 <https://github.com/ros2/rcl/issues/471>`_)
* [YAML Parser] Depend on rcutils only (`#470 <https://github.com/ros2/rcl/issues/470>`_)
* Accept quoted int or float values as strings (`#464 <https://github.com/ros2/rcl/issues/464>`_)
* Fix memory corruption when maximum number of parameters is exceeded (`#456 <https://github.com/ros2/rcl/issues/456>`_)
* Contributors: Dirk Thomas, Esteve Fernandez, Jacob Perron, Michel Hidalgo, hyunseok-yang, ivanpauno

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
