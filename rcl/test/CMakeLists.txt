find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_nose REQUIRED)

find_package(example_interfaces REQUIRED)
find_package(std_msgs REQUIRED)

find_package(rmw_implementation_cmake REQUIRED)

include(cmake/rcl_add_custom_executable.cmake)
include(cmake/rcl_add_custom_gtest.cmake)
include(cmake/rcl_add_custom_launch_test.cmake)

set(extra_test_libraries)
set(extra_test_env)
set(extra_lib_dirs "${rcl_lib_dir}")

# This subdirectory extends both extra_test_libraries, extra_test_env, and extra_lib_dirs.
add_subdirectory(memory_tools)

macro(test_target)
  find_package(${rmw_implementation} REQUIRED)
  test_target_function()
endmacro()

function(test_target_function)
  message(STATUS "Creating tests for '${rmw_implementation}'")
  list(APPEND extra_test_env RMW_IMPLEMENTATION=${rmw_implementation})

  # Gtests

  rcl_add_custom_gtest(test_client${target_suffix}
    SRCS rcl/test_client.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  rcl_add_custom_gtest(test_time${target_suffix}
    SRCS rcl/test_time.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_common${target_suffix}
    SRCS rcl/test_common.cpp
    ENV
      ${extra_test_env}
      EMPTY_TEST=
      NORMAL_TEST=foo
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_get_node_names${target_suffix}
    SRCS rcl/test_get_node_names.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  set(SKIP_TEST "")
  # TODO(wjwwood): remove this when the graph API works properly for connext dynamic
  if(
    rmw_implementation STREQUAL "rmw_connext_dynamic_cpp"
  )
    message(STATUS "Skipping test_graph${target_suffix} test.")
    set(SKIP_TEST "SKIP_TEST")
  endif()
  rcl_add_custom_gtest(test_graph${target_suffix}
    SRCS rcl/test_graph.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces" "std_msgs"
    ${SKIP_TEST}
  )

  rcl_add_custom_gtest(test_rcl${target_suffix}
    SRCS rcl/test_rcl.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_node${target_suffix}
    SRCS rcl/test_node.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_guard_condition${target_suffix}
    SRCS rcl/test_guard_condition.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_publisher${target_suffix}
    SRCS rcl/test_publisher.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "std_msgs"
  )

  rcl_add_custom_gtest(test_service${target_suffix}
    SRCS rcl/test_service.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  rcl_add_custom_gtest(test_subscription${target_suffix}
    SRCS rcl/test_subscription.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "std_msgs"
  )

  rcl_add_custom_gtest(test_wait${target_suffix}
    SRCS rcl/test_wait.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_namespace${target_suffix}
    SRCS test_namespace.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  # Launch tests

  rcl_add_custom_executable(service_fixture${target_suffix}
    SRCS rcl/service_fixture.cpp
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  rcl_add_custom_executable(client_fixture${target_suffix}
    SRCS rcl/client_fixture.cpp
    LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  # TODO remove this once connext declares libdl dependency in libnddscore again
  # See https://github.com/ros2/rcl/issues/52
  if(rmw_implementation STREQUAL "rmw_connext_cpp")
    connext_workaround(client_fixture${target_suffix})
    connext_workaround(service_fixture${target_suffix})
  endif()

  rcl_add_custom_launch_test(test_services
    service_fixture
    client_fixture
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    TIMEOUT 15
  )
endfunction()

macro(connext_workaround target)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    get_target_property(_current_link_flags ${target} LINK_FLAGS)
    set(_link_flags "-Wl,--no-as-needed")
    if(_current_link_flags)
      set(_link_flags ${_current_link_flags} ${_link_flags})
    endif()
    set_target_properties(${target} PROPERTIES LINK_FLAGS "${_link_flags}")
  endif()
endmacro()

call_for_each_rmw_implementation(test_target)

rcl_add_custom_gtest(test_validate_topic_name
  SRCS rcl/test_validate_topic_name.cpp
  ENV ${extra_test_env}
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
)

rcl_add_custom_gtest(test_expand_topic_name
  SRCS rcl/test_expand_topic_name.cpp
  ENV ${extra_test_env}
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
)

rcl_add_custom_gtest(test_timer${target_suffix}
  SRCS rcl/test_timer.cpp
  ENV ${extra_test_env}
  APPEND_LIBRARY_DIRS ${extra_lib_dirs}
  LIBRARIES ${PROJECT_NAME} ${extra_test_libraries}
)
