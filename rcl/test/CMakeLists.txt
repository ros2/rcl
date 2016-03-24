find_package(ament_cmake_gtest REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(std_msgs REQUIRED)

include(rcl_add_custom_gtest.cmake)

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
  # TODO(wjwwood): remove these when C type support is supported by typesupport_introspection.
  #                after removing, we can also recombine the function and macro
  if("${rmw_implementation} " STREQUAL "rmw_fastrtps_cpp ")
    message(STATUS "Skipping tests for '${rmw_implementation}'")
    return()
  endif()

  message(STATUS "Creating tests for '${rmw_implementation}'")

  rcl_add_custom_gtest(test_allocator${target_suffix}
    SRCS rcl/test_allocator.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_client${target_suffix}
    SRCS rcl/test_client.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  rcl_add_custom_gtest(test_time${target_suffix}
    SRCS rcl/test_time.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_common${target_suffix}
    SRCS rcl/test_common.cpp
    ENV
      ${extra_test_env}
      EMPTY_TEST=
      NORMAL_TEST=foo
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_rcl${target_suffix}
    SRCS rcl/test_rcl.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_node${target_suffix}
    SRCS rcl/test_node.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation}
  )

  rcl_add_custom_gtest(test_publisher${target_suffix}
    SRCS rcl/test_publisher.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "std_msgs"
  )

  rcl_add_custom_gtest(test_service${target_suffix}
    SRCS rcl/test_service.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "example_interfaces"
  )

  rcl_add_custom_gtest(test_subscription${target_suffix}
    SRCS rcl/test_subscription.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    LIBRARIES ${PROJECT_NAME}${target_suffix} ${extra_test_libraries}
    AMENT_DEPENDENCIES ${rmw_implementation} "std_msgs"
  )
endfunction()

call_for_each_rmw_implementation(test_target)
