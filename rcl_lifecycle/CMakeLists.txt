cmake_minimum_required(VERSION 3.5)

project(rcl_lifecycle)

find_package(ament_cmake REQUIRED)

find_package(lifecycle_msgs REQUIRED)
find_package(rcl REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)

include_directories(include)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options("-Wall -Wextra -Wpedantic")
endif()

set(rcl_lifecycle_sources
  src/com_interface.c
  src/default_state_machine.c
  src/rcl_lifecycle.c
  src/transition_map.c
)
set_source_files_properties(
  ${rcl_lifecycle_sources}
  PROPERTIES language "C")

### C-Library depending only on RCL
add_library(
  rcl_lifecycle
  SHARED
  ${rcl_lifecycle_sources})

ament_target_dependencies(rcl_lifecycle
  "lifecycle_msgs"
  "rcl"
  "rcutils"
)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(rcl_lifecycle PRIVATE "RCL_LIFECYCLE_BUILDING_DLL")

install(TARGETS rcl_lifecycle
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
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
ament_package()

install(
  DIRECTORY include/
  DESTINATION include)
