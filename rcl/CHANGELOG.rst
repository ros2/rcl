^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcl
^^^^^^^^^^^^^^^^^^^^^^^^^

1.1.12 (2022-01-31)
-------------------
* Add setter and getter for domain_id in rcl_init_options_t (`#678 <https://github.com/ros2/rcl/issues/678>`_) (`#946 <https://github.com/ros2/rcl/issues/946>`_)
* Fix test_info_by_topic flaky (`#859 <https://github.com/ros2/rcl/issues/859>`_) (`#944 <https://github.com/ros2/rcl/issues/944>`_)
* Contributors: Jacob Perron, Tomoya.Fujita

1.1.11 (2021-04-14)
-------------------
* Update quality declaration links (re: `ros2/docs.ros2.org#52 <https://github.com/ros2/docs.ros2.org/issues/52>`_) (`#910 <https://github.com/ros2/rcl/issues/910>`_)
* Contributors: Simon Honigmann

1.1.10 (2020-12-09)
-------------------
* Update QD to QL 1 (`#867 <https://github.com/ros2/rcl/issues/867>`_)
* Update QD (`#843 <https://github.com/ros2/rcl/issues/843>`_)
* Complete rcl package's logging API test coverage (`#747 <https://github.com/ros2/rcl/issues/747>`_)
* Contributors: Alejandro Hernández Cordero, Christophe Bedard, Jorge Perez, Michel Hidalgo, Stephen Brawner

1.1.9 (2020-11-03)
------------------
* increase timeouts in test_services fixtures for Connext (`#745 <https://github.com/ros2/rcl/issues/745>`_)
* Add a semicolon to RCUTILS_LOGGING_AUTOINIT. (`#816 <https://github.com/ros2/rcl/issues/816>`_)
* Zero initialize events an size_of_events members of rcl_wait_set_t (`#841 <https://github.com/ros2/rcl/issues/841>`_)
* Return OK when finalizing zero-initialized contexts (`#842 <https://github.com/ros2/rcl/issues/842>`_)
* Make sure to check the return value of rcl APIs. (`#838 <https://github.com/ros2/rcl/issues/838>`_)
* Fix memory leak because of mock test (`#800 <https://github.com/ros2/rcl/issues/800>`_)
* Fix that not to deallocate event impl in some failure case (`#790 <https://github.com/ros2/rcl/issues/790>`_)
* calling fini functions to avoid memory leak (`#791 <https://github.com/ros2/rcl/issues/791>`_)
* Bump rcl arguments' API test coverage. (`#777 <https://github.com/ros2/rcl/issues/777>`_)
* Fix rcl arguments' API memory leaks and bugs. (`#778 <https://github.com/ros2/rcl/issues/778>`_)
* Add coverage tests wait module (`#769 <https://github.com/ros2/rcl/issues/769>`_)
* Fix wait allocation cleanup (`#770 <https://github.com/ros2/rcl/issues/770>`_)
* Bump test coverage. (`#764 <https://github.com/ros2/rcl/issues/764>`_)
* Check rcutils_strdup() outcome immediately. (`#768 <https://github.com/ros2/rcl/issues/768>`_)
* Cleanup rcl_get_secure_root() implementation. (`#762 <https://github.com/ros2/rcl/issues/762>`_)
* Add fault injection macros to rcl functions (`#727 <https://github.com/ros2/rcl/issues/727>`_)
* Yield rcl_context_fini() error codes. (`#763 <https://github.com/ros2/rcl/issues/763>`_)
* Do not invalidate context before successful shutdown. (`#761 <https://github.com/ros2/rcl/issues/761>`_)
* Zero initialize guard condition on failed init. (`#760 <https://github.com/ros2/rcl/issues/760>`_)
* Adding tests to arguments.c (`#752 <https://github.com/ros2/rcl/issues/752>`_)
* Extend rcl_expand_topic_name() API test coverage. (`#758 <https://github.com/ros2/rcl/issues/758>`_)
* Add coverage tests 94% service.c (`#756 <https://github.com/ros2/rcl/issues/756>`_)
* Clean up rcl_expand_topic_name() implementation. (`#757 <https://github.com/ros2/rcl/issues/757>`_)
* Added path_to_fail to mocking_utils in rcl
* Complete rcl enclave validation API coverage. (`#751 <https://github.com/ros2/rcl/issues/751>`_)
* Fix allocation arguments copy (`#748 <https://github.com/ros2/rcl/issues/748>`_)
* Fix rcl package's logging API error specs and handling. (`#746 <https://github.com/ros2/rcl/issues/746>`_)
* Fix bug error handling get_param_files (`#743 <https://github.com/ros2/rcl/issues/743>`_)
* Complete subscription API test coverage (`#734 <https://github.com/ros2/rcl/issues/734>`_)
* Add deallocate calls to free strdup allocated memory (`#737 <https://github.com/ros2/rcl/issues/737>`_)
* Add missing calls to rcl_convert_rmw_ret_to_rcl_ret (`#738 <https://github.com/ros2/rcl/issues/738>`_)
* Add mock tests, publisher 95% coverage (`#732 <https://github.com/ros2/rcl/issues/732>`_)
* Reformat rmw_impl_id_check to call a testable function (`#725 <https://github.com/ros2/rcl/issues/725>`_)
* Make sure to call rcl_arguments_fini at the end of the test.
* Add remap needed null check (`#711 <https://github.com/ros2/rcl/issues/711>`_)
* Make public ini/fini rosout publisher (`#704 <https://github.com/ros2/rcl/issues/704>`_)
* Move rcl_remap_copy to public header (`#709 <https://github.com/ros2/rcl/issues/709>`_)
* Add coverage tests for `rcl` (`#703 <https://github.com/ros2/rcl/issues/703>`_)
* Add bad arguments tests for coverage (`#698 <https://github.com/ros2/rcl/issues/698>`_)
* Improve error checking and handling in subscription APIs. (`#739 <https://github.com/ros2/rcl/issues/739>`_)
* Fix memory leak in rcl_subscription_init()/rcl_publisher_init() (`#794 <https://github.com/ros2/rcl/issues/794>`_, `#834 <https://github.com/ros2/rcl/issues/834>`_) (`#832 <https://github.com/ros2/rcl/issues/832>`_)
* Improve rcl init test coverage. (`#684 <https://github.com/ros2/rcl/issues/684>`_)
* Remove unused check context.c (`#691 <https://github.com/ros2/rcl/issues/691>`_)
* Improve subscription coverage (`#681 <https://github.com/ros2/rcl/issues/681>`_)
* Improve rcl timer test coverage. (`#680 <https://github.com/ros2/rcl/issues/680>`_)
* Improve wait sets test coverage. (`#683 <https://github.com/ros2/rcl/issues/683>`_)
* Minor fixes to rcl clock implementation. (`#688 <https://github.com/ros2/rcl/issues/688>`_)
* Improve clock test coverage. (`#685 <https://github.com/ros2/rcl/issues/685>`_)
* Improve enclave validation test coverage. (`#682 <https://github.com/ros2/rcl/issues/682>`_)
* Contributors: Chen Lihui, Chris Lalancette, Dirk Thomas, Jacob Perron, Jorge Perez, Michel Hidalgo, ahcorde, brawner

1.1.8 (2020-10-07)
------------------
* Tweaks to client.c and subscription.c for cleaner init/fini (`#728 <https://github.com/ros2/rcl/issues/728>`_) (`#822 <https://github.com/ros2/rcl/issues/822>`_)
* Contributors: Stephen Brawner

1.1.7 (2020-08-03)
------------------
* Removed doxygen warnings (`#712 <https://github.com/ros2/rcl/issues/712>`_) (`#724 <https://github.com/ros2/rcl/issues/724>`_)
* Set domain id to 0 if it is RMW_DEFAULT_DOMAIN_ID (`#719 <https://github.com/ros2/rcl/issues/719>`_)
* Contributors: Alejandro Hernández Cordero, Ivan Santiago Paunovic

1.1.6 (2020-07-07)
------------------
* Keep domain id if ROS_DOMAIN_ID is invalid (`#689 <https://github.com/ros2/rcl/issues/689>`_) (`#694 <https://github.com/ros2/rcl/issues/694>`_)
* Use RCL_RET\_* codes only (`#686 <https://github.com/ros2/rcl/issues/686>`_) (`#693 <https://github.com/ros2/rcl/issues/693>`_)
* Add check for invalid output in rcl_node_options_copy (`#671 <https://github.com/ros2/rcl/issues/671>`_)
* Add tests for rcl package (`#668 <https://github.com/ros2/rcl/issues/668>`_)
* Fixed doxygen warnings (`#677 <https://github.com/ros2/rcl/issues/677>`_) (`#696 <https://github.com/ros2/rcl/issues/696>`_)
* Print RCL_LOCALHOST_ENV_VAR if error happens via rcutils_get_env (`#672 <https://github.com/ros2/rcl/issues/672>`_)
* Contributors: Alejandro Hernández Cordero, Jorge Perez, Michel Hidalgo, tomoya

1.1.5 (2020-06-03)
------------------
* Fix conversions between rmw_localhost_only_t and bool (`#670 <https://github.com/ros2/rcl/issues/670>`_)
* Contributors: Jorge Perez

1.1.4 (2020-06-02)
------------------
* Ensure rcl_publisher_init() fails safely (`#667 <https://github.com/ros2/rcl/issues/667>`_)
* Contributors: Michel Hidalgo

1.1.3 (2020-06-01)
------------------
* Add Security Vulnerability Policy pointing to REP-2006 (`#661 <https://github.com/ros2/rcl/issues/661>`_)
* Add tests to publisher and init modules of rcl (`#657 <https://github.com/ros2/rcl/issues/657>`_)
* Contributors: Chris Lalancette, Jorge Perez

1.1.2 (2020-05-28)
------------------
* Improve docblocks (`#659 <https://github.com/ros2/rcl/issues/659>`_)
* Contributors: Alejandro Hernández Cordero

1.1.1 (2020-05-26)
------------------

1.1.0 (2020-05-22)
------------------
* Expose rcl default logging output handler (`#660 <https://github.com/ros2/rcl/issues/660>`_)
* Remove deprecated functions (`#658 <https://github.com/ros2/rcl/issues/658>`_)
* Warn about unused return value for set_logger_level (`#652 <https://github.com/ros2/rcl/issues/652>`_)
* Mark cyclonedds test_service test as flakey (`#648 <https://github.com/ros2/rcl/issues/648>`_)
* Convert sleep_for into appropriate logic in tests(`#631 <https://github.com/ros2/rcl/issues/631>`_)
* Reduce timeouts in tests(`#613 <https://github.com/ros2/rcl/issues/613>`_)
* Add tests for time.c and timer.c (`#599 <https://github.com/ros2/rcl/issues/599>`_)
* Update Quality Declaration for 1.0 (`#647 <https://github.com/ros2/rcl/issues/647>`_)
* Contributors: Barry Xu, Dirk Thomas, Ivan Santiago Paunovic, Jorge Perez, Tully Foote, brawner

1.0.0 (2020-05-12)
------------------
* Remove MANUAL_BY_NODE liveliness API (`#645 <https://github.com/ros2/rcl/issues/645>`_)
* Make test_two_timers* more reliable (`#640 <https://github.com/ros2/rcl/issues/640>`_)
* Contributors: Ivan Santiago Paunovic

0.9.1 (2020-05-08)
------------------
* Included features (`#644 <https://github.com/ros2/rcl/issues/644>`_)
* Current state Quality Declaration (`#639 <https://github.com/ros2/rcl/issues/639>`_)
* Initialize service timestamps to 0 and test. (`#642 <https://github.com/ros2/rcl/issues/642>`_)
* Contributors: Alejandro Hernández Cordero, Ingo Lütkebohle, Jorge Perez

0.9.0 (2020-04-29)
------------------
* Fix std::string construction in test (`#636 <https://github.com/ros2/rcl/issues/636>`_)
* Add basic functionality tests for validate_enclave_name and subscription (`#624 <https://github.com/ros2/rcl/issues/624>`_)
* Save allocator for RCL_CLOCK_UNINITIALIZED clock (`#623 <https://github.com/ros2/rcl/issues/623>`_)
* Implement service info structure with timestamps (`#627 <https://github.com/ros2/rcl/issues/627>`_)
* Add support for taking a sequence of messages (`#614 <https://github.com/ros2/rcl/issues/614>`_)
* Message info with timestamps support in rcl (`#619 <https://github.com/ros2/rcl/issues/619>`_)
* Don't call ``rcl_logging_configure/rcl_logging_fini`` in ``rcl_init/rcl_shutdown`` (`#579 <https://github.com/ros2/rcl/issues/579>`_)
* Export targets in a addition to include directories / libraries (`#629 <https://github.com/ros2/rcl/issues/629>`_)
* Document rcl_pub/etc_fini() must come before rcl_node_fini() (`#625 <https://github.com/ros2/rcl/issues/625>`_)
* Update security environment variables (`#617 <https://github.com/ros2/rcl/issues/617>`_)
* Add visibility to rcl_timer_get_allocator (`#610 <https://github.com/ros2/rcl/issues/610>`_)
* Fix test_publisher memory leaks reported by asan (`#567 <https://github.com/ros2/rcl/issues/567>`_)
* security-context -> enclave (`#612 <https://github.com/ros2/rcl/issues/612>`_)
* Rename rosidl_generator_c namespace to rosidl_runtime_c (`#616 <https://github.com/ros2/rcl/issues/616>`_)
* Rename rosidl_generator_cpp namespace to rosidl_runtime_cpp (`#615 <https://github.com/ros2/rcl/issues/615>`_)
* Fix security directory lookup for '/' security contexts (`#609 <https://github.com/ros2/rcl/issues/609>`_)
* Changed rosidl_generator_c/cpp to rosidl_runtime_c/cpp (`#588 <https://github.com/ros2/rcl/issues/588>`_)
* Remove deprecated CLI rules (`#603 <https://github.com/ros2/rcl/issues/603>`_)
* Use keystore root as security root directory, and not contexts folder (`#607 <https://github.com/ros2/rcl/issues/607>`_)
* Remove tinydir_vendor dependency (`#608 <https://github.com/ros2/rcl/issues/608>`_)
* Add missing allocator check for NULL (`#606 <https://github.com/ros2/rcl/issues/606>`_)
* Change naming style for private functions (`#597 <https://github.com/ros2/rcl/issues/597>`_)
* Switch to one Participant per Context (`#515 <https://github.com/ros2/rcl/issues/515>`_)
* Support for ON_REQUESTED_INCOMPATIBLE_QOS and ON_OFFERED_INCOMPATIBLE_QOS events (`#535 <https://github.com/ros2/rcl/issues/535>`_)
* Small typo fix (`#604 <https://github.com/ros2/rcl/issues/604>`_)
* Update docstring with new possible return code (`#600 <https://github.com/ros2/rcl/issues/600>`_)
* Add missing node destruction (`#601 <https://github.com/ros2/rcl/issues/601>`_)
* Test that nodes are returned with correct multiplicity (`#598 <https://github.com/ros2/rcl/issues/598>`_)
* Trigger guard condition when timer is reset (`#589 <https://github.com/ros2/rcl/issues/589>`_)
* Clock API improvements (`#580 <https://github.com/ros2/rcl/issues/580>`_)
* Fix memory leak in rcl_arguments (`#564 <https://github.com/ros2/rcl/issues/564>`_)
* Don't check history depth if RMW_QOS_POLICY_HISTORY_KEEP_ALL (`#593 <https://github.com/ros2/rcl/issues/593>`_)
* Fix alloc-dealloc-mismatch(new->free) in test_info_by_topic (`#469 <https://github.com/ros2/rcl/issues/469>`_) (`#569 <https://github.com/ros2/rcl/issues/569>`_)
* Use 10sec lifespan in rosout publisher qos (`#587 <https://github.com/ros2/rcl/issues/587>`_)
* Document clock types (`#578 <https://github.com/ros2/rcl/issues/578>`_)
* Make rosout publisher transient local with a depth of 1000 (`#582 <https://github.com/ros2/rcl/issues/582>`_)
* Enable TestInfoByTopicFixture unit tests for other rmw_implementations (`#583 <https://github.com/ros2/rcl/issues/583>`_)
* Fix memory leak in test_subscription_nominal (`#469 <https://github.com/ros2/rcl/issues/469>`_) (`#562 <https://github.com/ros2/rcl/issues/562>`_)
* Update rmw_topic_endpoint_info_array usage (`#576 <https://github.com/ros2/rcl/issues/576>`_)
* Add rcl versions of rmw_topic_endpoint_info* types (`#558 <https://github.com/ros2/rcl/issues/558>`_)
* Enable test for rcl_get_subscriptions_info_by_topic / rcl_get_publishers_info_by_topic for Cyclone (`#572 <https://github.com/ros2/rcl/issues/572>`_)
* Fixed missing initialization and fixed qos checking in test (`#571 <https://github.com/ros2/rcl/issues/571>`_)
* Fix test_count_matched memory leaks reported by asan `#567 <https://github.com/ros2/rcl/issues/567>`_ (`#568 <https://github.com/ros2/rcl/issues/568>`_)
* Code style only: wrap after open parenthesis if not in one line (`#565 <https://github.com/ros2/rcl/issues/565>`_)
* Fix return type of rcl_publisher_get_subscription_count() (`#559 <https://github.com/ros2/rcl/issues/559>`_)
* Fix doc strings (`#557 <https://github.com/ros2/rcl/issues/557>`_)
* Implement functions to get publisher and subcription informations like QoS policies from topic name (`#511 <https://github.com/ros2/rcl/issues/511>`_)
* Use absolute topic name for ``rosout`` (`#549 <https://github.com/ros2/rcl/issues/549>`_)
* Set allocator before goto fail (`#546 <https://github.com/ros2/rcl/issues/546>`_)
* Add public facing API for validating rcl_wait_set_t (`#538 <https://github.com/ros2/rcl/issues/538>`_)
* Add flag to enable/disable rosout logging in each node individually. (`#532 <https://github.com/ros2/rcl/issues/532>`_)
* Treat __name the same as __node (`#494 <https://github.com/ros2/rcl/issues/494>`_)
* Contributors: Alejandro Hernández Cordero, Barry Xu, Chris Lalancette, Dan Rose, Dennis Potman, Dirk Thomas, DongheeYe, Ingo Lütkebohle, Ivan Santiago Paunovic, Jacob Perron, Jaison Titus, Jorge Perez, Miaofei Mei, Michael Carroll, Michel Hidalgo, Mikael Arguedas, P. J. Reed, Ruffin, Shane Loretz, William Woodall, y-okumura-isp

0.8.3 (2019-11-08)
------------------
* Support CLI parameter overrides using dots instead of slashes. (`#530 <https://github.com/ros2/rcl/issues/530>`_)
  Signed-off-by: Michel Hidalgo <michel@ekumenlabs.com>
* Contributors: Michel Hidalgo

0.8.2 (2019-10-23)
------------------
* Remove the prototype from rcl_impl_getenv. (`#525 <https://github.com/ros2/rcl/issues/525>`_)
* Use return_loaned_message_from (`#523 <https://github.com/ros2/rcl/issues/523>`_)
* Avoid ready_fn and self.proc_info (`#522 <https://github.com/ros2/rcl/issues/522>`_)
* Add localhost option to node creation (`#520 <https://github.com/ros2/rcl/issues/520>`_)
* Add initial instrumentation (`#473 <https://github.com/ros2/rcl/issues/473>`_)
* Zero copy api (`#506 <https://github.com/ros2/rcl/issues/506>`_)
* Don't create rosout publisher instance unless required. (`#514 <https://github.com/ros2/rcl/issues/514>`_)
* Handle zero non-ROS specific args properly in rcl_remove_ros_arguments (`#518 <https://github.com/ros2/rcl/issues/518>`_)
* Update rcl_node_init docstring (`#517 <https://github.com/ros2/rcl/issues/517>`_)
* Remove vestigial references to rcl_ok() (`#516 <https://github.com/ros2/rcl/issues/516>`_)
* Add mechanism to pass rmw impl specific payloads during pub/sub creation (`#513 <https://github.com/ros2/rcl/issues/513>`_)
* Contributors: Brian Marchi, Chris Lalancette, Ingo Lütkebohle, Jacob Perron, Karsten Knese, Michel Hidalgo, Peter Baughman, William Woodall, tomoya

0.8.1 (2019-10-08)
------------------
* Switch the default logging implementation to spdlog.
* Contributors: Chris Lalancette

0.8.0 (2019-09-26)
------------------
* Delete rcl_impl_getenv, replaced by rcutils_get_env (`#502 <https://github.com/ros2/rcl/issues/502>`_)
* Parse CLI parameters and YAML files (`#508 <https://github.com/ros2/rcl/issues/508>`_)
* Add specific return code for non existent node (`#492 <https://github.com/ros2/rcl/issues/492>`_)
* Add node name and namespace validation to graph functions (`#499 <https://github.com/ros2/rcl/issues/499>`_)
* Bring back deprecated CLI arguments (`#496 <https://github.com/ros2/rcl/issues/496>`_)
* Polish rcl arguments implementation (`#497 <https://github.com/ros2/rcl/issues/497>`_)
* Uncoment some test_graph test cases after fix in rmw_fastrtps (`ros2/rmw_fastrtps#316 <https://github.com/ros2/rmw_fastrtps/issues/316>`_) (`#498 <https://github.com/ros2/rcl/issues/498>`_)
* Promote special CLI rules to flags (`#495 <https://github.com/ros2/rcl/issues/495>`_)
* Fail fast on invalid ROS arguments (`#493 <https://github.com/ros2/rcl/issues/493>`_)
* Enforce -r/--remap flags. (`#491 <https://github.com/ros2/rcl/issues/491>`_)
* Support parameter overrides and remap rules flags on command line (`#483 <https://github.com/ros2/rcl/issues/483>`_)
* Allow get_node_names to return result in any order (`#488 <https://github.com/ros2/rcl/issues/488>`_)
* rosout init and fini marked as RCL_PUBLIC (`#479 <https://github.com/ros2/rcl/issues/479>`_)
* included header in logging_rosout.c (`#478 <https://github.com/ros2/rcl/issues/478>`_)
* Migrate to '--ros-args ... [--]'-based ROS args extraction (`#477 <https://github.com/ros2/rcl/issues/477>`_)
* Improve security error messages  (`#480 <https://github.com/ros2/rcl/issues/480>`_)
* Add function for getting clients by node (`#459 <https://github.com/ros2/rcl/issues/459>`_)
* Remove special case check for manual_by_node for rmw_fastrtps (`#467 <https://github.com/ros2/rcl/issues/467>`_)
* Fix memory leak of 56 bytes in test_graph
* Change tests to try MANUAL_BY_TOPIC liveliness for FastRTPS (`#465 <https://github.com/ros2/rcl/issues/465>`_)
* Implement get_actual_qos() for subscriptions (`#455 <https://github.com/ros2/rcl/issues/455>`_)
* Log warning when remapping to an invalid node name (`#454 <https://github.com/ros2/rcl/issues/454>`_)
* Use size_t printf format for size_t variable (`#453 <https://github.com/ros2/rcl/issues/453>`_)
* Contributors: Alberto Soragna, Emerson Knapp, Jacob Perron, M. M, Michel Hidalgo, Mikael Arguedas, Víctor Mayoral Vilches, eboasson, ivanpauno

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
