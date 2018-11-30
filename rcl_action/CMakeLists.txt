cmake_minimum_required(VERSION 3.5)

project(rcl_action)

find_package(ament_cmake_ros REQUIRED)

find_package(rosidl_generator_c REQUIRED)
find_package(action_msgs REQUIRED)
find_package(rcl REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)

include_directories(
  include
  ${action_msgs_INCLUDE_DIRS}
  ${rcl_INCLUDE_DIRS}
)

# Default to C11
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(test_compile_headers
  src/test_compile_headers.c
)

set(rcl_action_sources
  src/${PROJECT_NAME}/action_client.c
  src/${PROJECT_NAME}/action_server.c
  src/${PROJECT_NAME}/goal_handle.c
  src/${PROJECT_NAME}/goal_state_machine.c
  src/${PROJECT_NAME}/names.c
  src/${PROJECT_NAME}/types.c
)

set_source_files_properties(
  ${rcl_action_sources}
  PROPERTIES language "C"
)

add_library(${PROJECT_NAME}
  ${rcl_action_sources}
)

ament_target_dependencies(${PROJECT_NAME}
  "action_msgs"
  "rcl"
  "rcutils"
  "rmw"
  "rosidl_generator_c"
)
# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "RCL_ACTION_BUILDING_DLL")

install(
  DIRECTORY include/
  DESTINATION include
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  find_package(osrf_testing_tools_cpp REQUIRED)
  find_package(test_msgs REQUIRED)
  ament_lint_auto_find_test_dependencies()
  ament_find_gtest()
  # Gtests
  ament_add_gtest(test_action_client
    test/rcl_action/test_action_client.cpp
  )
  if(TARGET test_action_client)
    target_include_directories(test_action_client PUBLIC
      include
      ${osrf_testing_tools_cpp_INCLUDE_DIRS}
    )
    target_link_libraries(test_action_client
      ${PROJECT_NAME}
    )
    ament_target_dependencies(test_action_client "rcl" "test_msgs")
  endif()
  ament_add_gtest(test_action_communication
    test/rcl_action/test_action_communication.cpp
  )
  if(TARGET test_action_communication)
    target_include_directories(test_action_communication PUBLIC
      include
      ${rcl_INCLUDE_DIRS}
    )
    target_link_libraries(test_action_communication
      ${PROJECT_NAME}
    )
    ament_target_dependencies(test_action_communication "test_msgs")
  endif()
  ament_add_gtest(test_action_server
    test/rcl_action/test_action_server.cpp
  )
  if(TARGET test_action_server)
    target_include_directories(test_action_server PUBLIC
      include
      ${rcl_INCLUDE_DIRS}
    )
    target_link_libraries(test_action_server
      ${PROJECT_NAME}
    )
    ament_target_dependencies(test_action_server "test_msgs")
  endif()
  ament_add_gtest(test_goal_handle
    test/rcl_action/test_goal_handle.cpp
  )
  if(TARGET test_goal_handle)
    target_include_directories(test_goal_handle PUBLIC
      include
      ${rcl_INCLUDE_DIRS}
    )
    target_link_libraries(test_goal_handle
      ${PROJECT_NAME}
    )
  endif()
  ament_add_gtest(test_goal_state_machine
    test/rcl_action/test_goal_state_machine.cpp
  )
  if(TARGET test_goal_state_machine)
    target_include_directories(test_goal_state_machine PUBLIC
      ${rcl_INCLUDE_DIRS}
    )
    target_link_libraries(test_goal_state_machine
      ${PROJECT_NAME}
    )
  endif()
  ament_add_gtest(test_types
    test/rcl_action/test_types.cpp
  )
  if(TARGET test_types)
    target_include_directories(test_types PUBLIC
      include
      ${rcl_INCLUDE_DIRS}
    )
    target_link_libraries(test_types
      ${PROJECT_NAME}
    )
  endif()
  ament_add_gtest(test_names
    test/rcl_action/test_names.cpp
  )
  if(TARGET test_names)
    target_include_directories(test_names PUBLIC
      ${rcl_INCLUDE_DIRS}
    )
    target_link_libraries(test_names
      ${PROJECT_NAME}
    )
  endif()
endif()

# specific order: dependents before dependencies
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(ament_cmake)
ament_export_dependencies(rcl)
ament_export_dependencies(rcutils)
ament_export_dependencies(rmw)
ament_export_dependencies(action_msgs)
ament_export_dependencies(rosidl_generator_c)
ament_package()
