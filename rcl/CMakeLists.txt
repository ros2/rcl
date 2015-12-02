cmake_minimum_required(VERSION 2.8.3)

project(rcl)

find_package(ament_cmake REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rmw REQUIRED)
find_package(rosidl_generator_c REQUIRED)

ament_export_dependencies(rmw rosidl_generator_c)
ament_export_include_directories(include)

include_directories(include)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11 -Wall -Wextra")
endif()

set(${PROJECT_NAME}_sources
  src/rcl/allocator.c
  src/rcl/common.c
  src/rcl/guard_condition.c
  src/rcl/node.c
  src/rcl/publisher.c
  src/rcl/rcl.c
  src/rcl/subscription.c
  src/rcl/wait.c
)
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_sources})
ament_target_dependencies(${PROJECT_NAME}
  "rcl_interfaces"
  "rmw"
  "rosidl_generator_c"
)

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)
