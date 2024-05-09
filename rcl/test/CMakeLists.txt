find_package(ament_cmake_gtest REQUIRED)

find_package(launch_testing_ament_cmake REQUIRED)
find_package(mimick_vendor REQUIRED)
find_package(osrf_testing_tools_cpp REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw_implementation_cmake REQUIRED)
find_package(rosidl_runtime_cpp REQUIRED)
find_package(test_msgs REQUIRED)

get_target_property(memory_tools_ld_preload_env_var
  osrf_testing_tools_cpp::memory_tools LIBRARY_PRELOAD_ENVIRONMENT_VARIABLE)

include(cmake/rcl_add_custom_executable.cmake)
include(cmake/rcl_add_custom_gtest.cmake)
include(cmake/rcl_add_custom_launch_test.cmake)

set(extra_lib_dirs "${rcl_lib_dir}")
add_definitions(-DTEST_RESOURCES_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(DISTRIBUTION "Unknown")
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  # If we are on Linux, look for /etc/os-release, which has "key=value" items.
  # Parse those items, looking for a key named "id" (case-insensitive), and
  # if we find it, set DISTRIBUTION to that value.  That gives us some idea
  # of which Linux distribution we are on.
  if(EXISTS "/etc/os-release")
    file(STRINGS "/etc/os-release" OS_RELEASE)
    foreach(line ${OS_RELEASE})
      string(REGEX REPLACE "^(.*)=.*" "\\1" key "${line}")
      string(TOLOWER "${key}" key)
      if("${key}" STREQUAL "id")
        string(REGEX REPLACE "^.*=(.*)" "\\1" DISTRIBUTION "${line}")
        string(TOLOWER "${DISTRIBUTION}" DISTRIBUTION)
        break()
      endif()
    endforeach()
  endif()
endif()

# finding gtest once in the highest scope
# prevents finding it repeatedly in each local scope
ament_find_gtest()

ament_add_gtest_executable(test_client
  rcl/test_client.cpp
)
target_link_libraries(test_client ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_timer
  rcl/test_timer.cpp
)
target_link_libraries(test_timer ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_context
  rcl/test_context.cpp
)
target_link_libraries(test_context ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_get_node_names
  rcl/test_get_node_names.cpp
)
target_link_libraries(test_get_node_names ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_graph
  rcl/test_graph.cpp
)
target_link_libraries(test_graph ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_info_by_topic
  rcl/test_info_by_topic.cpp
)
target_link_libraries(test_info_by_topic ${PROJECT_NAME} wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_count_matched
  rcl/test_count_matched.cpp
)
target_link_libraries(test_count_matched ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_get_actual_qos
  rcl/test_get_actual_qos.cpp
)
target_link_libraries(test_get_actual_qos ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})


ament_add_gtest_executable(test_init
  rcl/test_init.cpp
)
target_link_libraries(test_init ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_node
    rcl/test_node.cpp
)
target_link_libraries(test_node ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_remap
  rcl/test_remap.cpp
)
target_include_directories(test_remap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/)
target_link_libraries(test_remap ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_remap_integration
  rcl/test_remap_integration.cpp
)
target_link_libraries(test_remap_integration ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_guard_condition
  rcl/test_guard_condition.cpp
)
target_link_libraries(test_guard_condition ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_publisher
  rcl/test_publisher.cpp
)
target_include_directories(test_publisher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/)
target_link_libraries(test_publisher ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_publisher_wait_all_ack
  rcl/test_publisher_wait_all_ack.cpp
)
target_link_libraries(test_publisher_wait_all_ack ${PROJECT_NAME} mimick wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools
        rcutils::rcutils rosidl_runtime_c::rosidl_runtime_c ${test_msgs_TARGETS})

ament_add_gtest_executable(test_events
  rcl/test_events.cpp
)
target_include_directories(test_events PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/)
target_link_libraries(test_events ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_wait
  rcl/test_wait.cpp
)
target_link_libraries(test_wait ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools)

ament_add_gtest_executable(test_logging_rosout
  rcl/test_logging_rosout.cpp
)
target_link_libraries(test_logging_rosout ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${rcl_interfaces_TARGETS})

ament_add_gtest_executable(test_namespace
  rcl/test_namespace.cpp
)
target_link_libraries(test_namespace ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_rmw_impl_id_check_func
  rcl/test_rmw_impl_id_check_func.cpp
)
target_link_libraries(test_rmw_impl_id_check_func ${PROJECT_NAME} mimick)

ament_add_gtest_executable(test_network_flow_endpoints
  rcl/test_network_flow_endpoints.cpp
)
target_link_libraries(test_network_flow_endpoints ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_service_event_publisher
  rcl/test_service_event_publisher.cpp
)
target_include_directories(test_service_event_publisher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/)
target_link_libraries(test_service_event_publisher
  ${PROJECT_NAME} mimick wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_type_description_conversions
  rcl/test_type_description_conversions.cpp
)
target_link_libraries(test_type_description_conversions
  ${PROJECT_NAME} rosidl_runtime_c::rosidl_runtime_c ${test_msgs_TARGETS})

ament_add_gtest_executable(test_node_type_cache
  rcl/test_node_type_cache.cpp
)
target_link_libraries(test_node_type_cache ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS})

ament_add_gtest_executable(test_get_type_description_service
  rcl/test_get_type_description_service.cpp
)
target_include_directories(test_get_type_description_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/)
target_link_libraries(test_get_type_description_service
  ${PROJECT_NAME} mimick wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools ${type_description_interfaces_TARGETS})

function(test_target)
  message(STATUS "Creating tests for '${rmw_implementation}'")
  set(rmw_implementation_env_var RMW_IMPLEMENTATION=${rmw_implementation})

  # Gtests
  ament_add_gtest_test(test_client
    TEST_NAME test_client${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_timer
    TEST_NAME test_timer${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_context
    TEST_NAME test_context${target_suffix}
    ENV ${rmw_implementation_env_var} ${memory_tools_ld_preload_env_var}
  )

  ament_add_gtest_test(test_get_node_names
    TEST_NAME test_get_node_names${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_graph
    TEST_NAME test_graph${target_suffix}
    ENV ${rmw_implementation_env_var}
    TIMEOUT 120
  )

  ament_add_gtest_test(test_info_by_topic
    TEST_NAME test_info_by_topic${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_count_matched
    TEST_NAME test_count_matched${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_get_actual_qos
    TEST_NAME test_get_actual_qos${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_init
    TEST_NAME test_init${target_suffix}
    ENV ${rmw_implementation_env_var} ${memory_tools_ld_preload_env_var}
  )

  # RHEL does not support mimick's inject_on_return functionality, which
  # causes test_node to segfault when it runs.  Since RHEL is not a Tier 1
  # platform, just disable the test since we are covering it elsewhere.
  if(
    "${DISTRIBUTION}" STREQUAL "\"centos\"" OR
    "${DISTRIBUTION}" STREQUAL "\"almalinux\"")
    set(gtest_filter_env_var "GTEST_FILTER=-TestNodeFixture.test_rcl_node_init_with_internal_errors")
  else()
    set(gtest_filter_env_var "")
  endif()
  ament_add_gtest_test(test_node
    TEST_NAME test_node${target_suffix}
    ENV ${rmw_implementation_env_var} ${memory_tools_ld_preload_env_var} ${gtest_filter_env_var}
    TIMEOUT 240  # Large timeout to wait for fault injection tests
  )

  ament_add_gtest_test(test_remap
    TEST_NAME test_remap${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_remap_integration
    TEST_NAME test_remap_integration${target_suffix}
    ENV ${rmw_implementation_env_var}
    TIMEOUT 200
  )

  ament_add_gtest_test(test_guard_condition
    TEST_NAME test_guard_condition${target_suffix}
    ENV ${rmw_implementation_env_var} ${memory_tools_ld_preload_env_var}
  )

  ament_add_gtest_test(test_publisher
    TEST_NAME test_publisher${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_publisher_wait_all_ack
    TEST_NAME test_publisher_wait_all_ack${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  rcl_add_custom_gtest(test_service${target_suffix}
    SRCS rcl/test_service.cpp
    ENV ${rmw_implementation_env_var}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} mimick wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS}
  )

  rcl_add_custom_gtest(test_subscription${target_suffix}
    SRCS rcl/test_subscription.cpp
    ENV ${rmw_implementation_env_var}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} mimick wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools rosidl_runtime_cpp::rosidl_runtime_cpp
      ${test_msgs_TARGETS}
    TIMEOUT 120
  )
  # TODO(asorbini) Enable message timestamp tests for rmw_connextdds on Windows
  # once clock incompatibilities are resolved.
  if(rmw_implementation STREQUAL "rmw_connextdds" AND WIN32)
    message(STATUS "Disabling message timestamp test for ${rmw_implementation}")
  else()
    message(STATUS "Enabling message timestamp test for ${rmw_implementation}")
    target_compile_definitions(test_subscription${target_suffix}
      PRIVATE "RMW_TIMESTAMPS_SUPPORTED=1")
    target_compile_definitions(test_service${target_suffix}
      PRIVATE "RMW_TIMESTAMPS_SUPPORTED=1")
  endif()

  ament_add_gtest_test(test_events
    TEST_NAME test_events${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_wait
    TEST_NAME test_wait${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_logging_rosout
    TEST_NAME test_logging_rosout${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_namespace
    TEST_NAME test_namespace${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_rmw_impl_id_check_func
    TEST_NAME test_rmw_impl_id_check_func${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_network_flow_endpoints
    TEST_NAME test_network_flow_endpoints${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_service_event_publisher
    TEST_NAME test_service_event_publisher${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_type_description_conversions
    TEST_NAME test_type_description_conversions${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_node_type_cache
    TEST_NAME test_node_type_cache${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  ament_add_gtest_test(test_get_type_description_service
    TEST_NAME test_get_type_description_service${target_suffix}
    ENV ${rmw_implementation_env_var}
  )

  # Launch tests

  rcl_add_custom_launch_test(test_services
    service_fixture
    client_fixture
    ENV ${rmw_implementation_env_var}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    TIMEOUT 15
  )

  set(SKIP_TEST "")
  if(WIN32)
    # TODO(dhood): launch does not set the return code correctly for these tests on Windows.
    # See https://github.com/ros2/launch/issues/66
    set(SKIP_TEST "SKIP_TEST")
  endif()
  set(TEST_RMW_IMPL_ID_CHECK_EXECUTABLE_NAME "$<TARGET_FILE:test_rmw_impl_id_check_exe>")
  configure_file(
    rcl/test_rmw_impl_id_check.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_rmw_impl_id_check${target_suffix}.py.configure
    @ONLY
  )
  file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test/test_rmw_impl_id_check${target_suffix}_$<CONFIG>.py"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/test_rmw_impl_id_check${target_suffix}.py.configure"
  )
  add_launch_test(
    "${CMAKE_CURRENT_BINARY_DIR}/test/test_rmw_impl_id_check${target_suffix}_$<CONFIG>.py"
    TARGET test_rmw_impl_id_check${target_suffix}
    APPEND_LIBRARY_DIRS "${extra_lib_dirs}"
    ${SKIP_TEST}
  )
  if(TEST test_rmw_impl_id_check${target_suffix})
    set_tests_properties(
      test_rmw_impl_id_check${target_suffix}
      PROPERTIES DEPENDS test_rmw_impl_id_check_exe
    )
  endif()

endfunction()

# Build simple executable for using in the test_rmw_impl_id_check
add_executable(test_rmw_impl_id_check_exe
  rcl/test_rmw_impl_id_check_exe.cpp)
target_link_libraries(test_rmw_impl_id_check_exe ${PROJECT_NAME})

# This file is used by many tests, so build it just once
add_library(wait_for_entity_helpers STATIC rcl/wait_for_entity_helpers.cpp)
target_link_libraries(wait_for_entity_helpers PUBLIC ${PROJECT_NAME})
target_link_libraries(wait_for_entity_helpers PRIVATE
  osrf_testing_tools_cpp::memory_tools
  rcutils::rcutils)

# Launch test executables

rcl_add_custom_executable(service_fixture
  SRCS rcl/service_fixture.cpp
  LIBRARIES ${PROJECT_NAME} wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS} rcutils::rcutils
)

rcl_add_custom_executable(client_fixture
  SRCS rcl/client_fixture.cpp
  LIBRARIES ${PROJECT_NAME} wait_for_entity_helpers osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS}
)

call_for_each_rmw_implementation(test_target)

rcl_add_custom_gtest(test_arguments
  SRCS rcl/test_arguments.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/
  LIBRARIES ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_time
  SRCS rcl/test_time.cpp
  ENV ${memory_tools_ld_preload_env_var}
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_lexer
  SRCS rcl/test_lexer.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME}
)

rcl_add_custom_gtest(test_lexer_lookahead
  SRCS rcl/test_lexer_lookahead.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_validate_enclave_name
  SRCS rcl/test_validate_enclave_name.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} mimick
)

rcl_add_custom_gtest(test_discovery_options
  SRCS rcl/test_discovery_options.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_domain_id
  SRCS rcl/test_domain_id.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} mimick
)

rcl_add_custom_gtest(test_localhost
  SRCS rcl/test_localhost.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME}
)

rcl_add_custom_gtest(test_logging
  SRCS rcl/test_logging.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} mimick ${rcl_interfaces_TARGETS}
    rcl_logging_interface::rcl_logging_interface ${RCL_LOGGING_IMPL}::${RCL_LOGGING_IMPL} osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_validate_topic_name
  SRCS rcl/test_validate_topic_name.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME}
)

rcl_add_custom_gtest(test_expand_topic_name
  SRCS rcl/test_expand_topic_name.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} mimick
)

rcl_add_custom_gtest(test_security
  SRCS rcl/test_security.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_common
  SRCS rcl/test_common.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/common.c
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../src/rcl/
  LIBRARIES ${PROJECT_NAME}
)

rcl_add_custom_gtest(test_log_level
  SRCS rcl/test_log_level.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} mimick osrf_testing_tools_cpp::memory_tools
)

rcl_add_custom_gtest(test_subscription_content_filter_options
  SRCS rcl/test_subscription_content_filter_options.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools ${test_msgs_TARGETS}
)

rcl_add_custom_gtest(test_type_hash
  SRCS rcl/test_type_hash.cpp
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME}
)
