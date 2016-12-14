cmake_minimum_required(VERSION 3.5)

project(rcl_lifecycle)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra")
endif()

find_package(ament_cmake REQUIRED)
find_package(rcl REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation_cmake REQUIRED)
find_package(lifecycle_msgs REQUIRED)

include_directories(include)

set(rcl_lifecycle_sources
  src/com_interface.c
  src/default_state_machine.c
  src/rcl_lifecycle.c
  src/transition_map.c)
set_source_files_properties(
  ${rcl_lifecycle_sources}
  PROPERTIES language "C")

macro(targets)
  get_rcl_information("${rmw_implementation}" "rcl${target_suffix}")

  ### C-Library depending only on RCL
  add_library(
    rcl_lifecycle${target_suffix}
    SHARED
    ${rcl_lifecycle_sources})

  ament_target_dependencies(rcl_lifecycle${target_suffix}
    "rcl${target_suffix}"
    "lifecycle_msgs")

  # Causes the visibility macros to use dllexport rather than dllimport,
  # which is appropriate when building the dll but not consuming it.
  target_compile_definitions(rcl_lifecycle${target_suffix} PRIVATE "RCL_LIFECYCLE_BUILDING_DLL")

  install(TARGETS rcl_lifecycle${target_suffix}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
endmacro()

call_for_each_rmw_implementation(targets GENERATE_DEFAULT)

if(BUILD_TESTING)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++14")
  add_compile_options(-Wall -Wextra -Wpedantic)
  endif()

  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  find_package(rcl REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # Gtests
  ament_add_gtest(test_default_state_machine
    test/test_default_state_machine.cpp
  )
  if(TARGET test_default_state_machine)
    target_include_directories(test_default_state_machine PUBLIC
      ${rcl_INCLUDE_DIRS}
      ${rcl_lifecycle_INCLUDE_DIRS}
    )
    target_link_libraries(test_default_state_machine ${PROJECT_NAME})
  endif()
  ament_add_gtest(test_multiple_instances
    test/test_multiple_instances.cpp
  )
  if(TARGET test_multiple_instances)
    target_include_directories(test_multiple_instances PUBLIC
      ${rcl_INCLUDE_DIRS}
      ${rcl_lifecycle_INCLUDE_DIRS}
    )
    target_link_libraries(test_multiple_instances ${PROJECT_NAME})
  endif()
endif()

ament_export_dependencies(ament_cmake)
ament_export_dependencies(lifecycle_msgs)
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package(
  CONFIG_EXTRAS rcl_lifecycle-extras.cmake)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})

install(
  DIRECTORY include/
  DESTINATION include)
