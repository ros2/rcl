cmake_minimum_required(VERSION 2.8.3)

project(rcl)

find_package(ament_cmake REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation REQUIRED)
find_package(rmw_implementation_cmake REQUIRED)
find_package(rosidl_generator_c REQUIRED)

include_directories(include)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif()

set(${PROJECT_NAME}_sources
  src/rcl/allocator.c
  src/rcl/client.c
  src/rcl/common.c
  src/rcl/guard_condition.c
  src/rcl/node.c
  src/rcl/publisher.c
  src/rcl/rcl.c
  src/rcl/service.c
  src/rcl/subscription.c
  src/rcl/wait.c
  src/rcl/time.c
  src/rcl/timer.c
)

macro(target)
  add_library(${PROJECT_NAME}${target_suffix} SHARED ${${PROJECT_NAME}_sources})
  ament_target_dependencies(${PROJECT_NAME}${target_suffix}
    "rcl_interfaces"
    "rmw"
    "rosidl_generator_c"
    "${rmw_implementation}"
  )

  # Causes the visibility macros to use dllexport rather than dllimport,
  # which is appropriate when building the dll but not consuming it.
  target_compile_definitions(${PROJECT_NAME}${target_suffix} PRIVATE "RCL_BUILDING_DLL")

  install(
    TARGETS ${PROJECT_NAME}${target_suffix}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

  # rcl_lib_dir is passed as APPEND_LIBRARY_DIRS for each ament_add_gtest call so
  # the librcl that they link against is on the library path.
  # This is especially important on Windows.
  # This is overwritten each loop, but which one it points to doesn't really matter.
  set(rcl_lib_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}${target_suffix}>")
endmacro()

call_for_each_rmw_implementation(target GENERATE_DEFAULT)

ament_export_dependencies(ament_cmake)
ament_export_dependencies(rcl_interfaces)
ament_export_dependencies(rmw)
ament_export_dependencies(rmw_implementation)
ament_export_dependencies(rosidl_generator_c)

ament_export_include_directories(include)

ament_export_libraries(${PROJECT_NAME})

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  add_subdirectory(test)
endif()

ament_package(
  CONFIG_EXTRAS rcl-extras.cmake
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)
