cmake_minimum_required(VERSION 3.5)

project(rcl_action)

find_package(ament_cmake_ros REQUIRED)

# find_package(action_msgs REQUIRED)
find_package(rcl REQUIRED)
find_package(rcutils REQUIRED)
# find_package(rmw REQUIRED)

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
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# set(rcl_action_sources
#   src/action_server.c
#   src/action_client.c
#   src/action_goal.c
#   src/transition_map.c
# )
# set_source_files_properties(
#   ${rcl_action_sources}
#   PROPERTIES language "C"
# )

### C-Library depending only on RCL
# add_library(
#   ${PROJECT_NAME}
#   ${rcl_action_sources}
# )

# specific order: dependents before dependencies
# ament_target_dependencies(${PROJECT_NAME}
#   "rcl"
#   "action_msgs"
#   "rosidl_generator_c"
#   "rcutils"
# )

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
# target_compile_definitions(${PROJECT_NAME} PRIVATE "RCL_ACTION_BUILDING_DLL")

# install(TARGETS ${PROJECT_NAME}
#   ARCHIVE DESTINATION lib
#   LIBRARY DESTINATION lib
#   RUNTIME DESTINATION bin
# )

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # Gtests
  # ament_add_gtest(test_action_server
  #   test/test_action_server.cpp
  # )
  # if(TARGET test_action_server)
  #   target_include_directories(test_action_server PUBLIC
  #     ${rcl_INCLUDE_DIRS}
  #   )
  #   target_link_libraries(test_action_server ${PROJECT_NAME})
  # endif()
  # ament_add_gtest(test_action_client
  #   test/test_action_client.cpp
  # )
  # if(TARGET test_action_client)
  #   target_include_directories(test_action_client PUBLIC
  #     ${rcl_INCLUDE_DIRS}
  #   )
  #   target_link_libraries(test_action_client ${PROJECT_NAME})
  # endif()
endif()

# specific order: dependents before dependencies
ament_export_include_directories(include)
# ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(ament_cmake)
ament_export_dependencies(rcl)
# ament_export_dependencies(action_msgs)
ament_export_dependencies(rcutils)
ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)
