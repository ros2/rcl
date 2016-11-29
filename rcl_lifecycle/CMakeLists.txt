cmake_minimum_required(VERSION 3.5)

project(rcl_lifecycle)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra")
endif()

find_package(ament_cmake REQUIRED)
find_package(rcl REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(lifecycle_msgs REQUIRED)

include_directories(
  include)

set(rcl_lifecycle_sources
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
    "std_msgs"
    "lifecycle_msgs")

  install(TARGETS rcl_lifecycle${target_suffix}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
endmacro()

call_for_each_rmw_implementation(targets GENERATE_DEFAULT)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()

install(
  DIRECTORY include/
  DESTINATION include)
