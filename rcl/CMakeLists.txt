cmake_minimum_required(VERSION 3.5)

project(rcl)

find_package(ament_cmake_ros REQUIRED)

find_package(rcl_interfaces REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation REQUIRED)
find_package(rosidl_generator_c REQUIRED)

include_directories(include)

# Default to C11
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra)
endif()

set(${PROJECT_NAME}_sources
  src/rcl/arguments.c
  src/rcl/client.c
  src/rcl/common.c
  src/rcl/context.c
  src/rcl/expand_topic_name.c
  src/rcl/graph.c
  src/rcl/guard_condition.c
  src/rcl/init.c
  src/rcl/init_options.c
  src/rcl/lexer.c
  src/rcl/lexer_lookahead.c
  src/rcl/node.c
  src/rcl/publisher.c
  src/rcl/remap.c
  src/rcl/rmw_implementation_identifier_check.c
  src/rcl/service.c
  src/rcl/subscription.c
  src/rcl/time.c
  src/rcl/timer.c
  src/rcl/validate_topic_name.c
  src/rcl/wait.c
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})
# specific order: dependents before dependencies
ament_target_dependencies(${PROJECT_NAME}
  "rcl_interfaces"
  "rmw_implementation"
  "rmw"
  "rcutils"
  "rosidl_generator_c"
)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "RCL_BUILDING_DLL")

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# rcl_lib_dir is passed as APPEND_LIBRARY_DIRS for each ament_add_gtest call so
# the librcl that they link against is on the library path.
# This is especially important on Windows.
# This is overwritten each loop, but which one it points to doesn't really matter.
set(rcl_lib_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# specific order: dependents before dependencies
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})

ament_export_dependencies(ament_cmake)
ament_export_dependencies(rcl_interfaces)
ament_export_dependencies(rmw_implementation)
ament_export_dependencies(rmw)
ament_export_dependencies(rcutils)
ament_export_dependencies(rosidl_generator_c)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  add_subdirectory(test)
endif()

ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)
