cmake_minimum_required(VERSION 3.5)

project(rcl)

find_package(ament_cmake_ros REQUIRED)
find_package(ament_cmake_gen_version_h REQUIRED)

find_package(libyaml_vendor REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rcl_logging_interface REQUIRED)
find_package(rcl_yaml_param_parser REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation REQUIRED)
find_package(rosidl_runtime_c REQUIRED)
find_package(service_msgs REQUIRED)
find_package(tracetools REQUIRED)
find_package(type_description_interfaces REQUIRED)
find_package(yaml REQUIRED)

include(cmake/rcl_set_symbol_visibility_hidden.cmake)
include(cmake/get_default_rcl_logging_implementation.cmake)
get_default_rcl_logging_implementation(RCL_LOGGING_IMPL)

# Default to C11
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(${PROJECT_NAME}_sources
  src/rcl/arguments.c
  src/rcl/client.c
  src/rcl/common.c
  src/rcl/context.c
  src/rcl/discovery_options.c
  src/rcl/domain_id.c
  src/rcl/dynamic_message_type_support.c
  src/rcl/event.c
  src/rcl/expand_topic_name.c
  src/rcl/graph.c
  src/rcl/guard_condition.c
  src/rcl/init.c
  src/rcl/init_options.c
  src/rcl/lexer.c
  src/rcl/lexer_lookahead.c
  src/rcl/localhost.c
  src/rcl/logging_rosout.c
  src/rcl/logging.c
  src/rcl/log_level.c
  src/rcl/network_flow_endpoints.c
  src/rcl/node.c
  src/rcl/node_options.c
  src/rcl/node_type_cache.c
  src/rcl/publisher.c
  src/rcl/remap.c
  src/rcl/node_resolve_name.c
  src/rcl/rmw_implementation_identifier_check.c
  src/rcl/security.c
  src/rcl/service.c
  src/rcl/service_event_publisher.c
  src/rcl/subscription.c
  src/rcl/time.c
  src/rcl/timer.c
  src/rcl/type_hash.c
  src/rcl/type_description_conversions.c
  src/rcl/validate_enclave_name.c
  src/rcl/validate_topic_name.c
  src/rcl/wait.c
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")
target_link_libraries(${PROJECT_NAME} PUBLIC
  # TODO(clalancette): rcl_interfaces should be PRIVATE, but downstream depends on it for now
  ${rcl_interfaces_TARGETS}
  # TODO(clalancette): rcl_logging_interface should be PRIVATE, but downstream depends on it for now
  rcl_logging_interface::rcl_logging_interface
  rcl_yaml_param_parser::rcl_yaml_param_parser
  rcutils::rcutils
  rmw::rmw
  # TODO(clalancette): rmw_implementation should be PRIVATE, but downstream depends on it for now
  rmw_implementation::rmw_implementation
  rosidl_runtime_c::rosidl_runtime_c
  ${type_description_interfaces_TARGETS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${RCL_LOGGING_IMPL}::${RCL_LOGGING_IMPL}
  ${service_msgs_TARGETS}
  tracetools::tracetools
  yaml
)

# Allow configuring the default discovery range
if(DEFINED RCL_DEFAULT_DISCOVERY_RANGE)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  "RCL_DEFAULT_DISCOVERY_RANGE=${RCL_DEFAULT_DISCOVERY_RANGE}")
endif()

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "RCL_BUILDING_DLL")
rcl_set_symbol_visibility_hidden(${PROJECT_NAME} LANGUAGE "C")

if(BUILD_TESTING AND NOT RCUTILS_DISABLE_FAULT_INJECTION)
  target_compile_definitions(${PROJECT_NAME} PUBLIC RCUTILS_ENABLE_FAULT_INJECTION)
endif()

install(
  TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# rcl_lib_dir is passed as APPEND_LIBRARY_DIRS for each ament_add_gtest call so
# the librcl that they link against is on the library path.
# This is especially important on Windows.
# This is overwritten each loop, but which one it points to doesn't really matter.
set(rcl_lib_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Export old-style CMake variables
ament_export_include_directories("include/${PROJECT_NAME}")
ament_export_libraries(${PROJECT_NAME})
# Export modern CMake targets
ament_export_targets(${PROJECT_NAME})

ament_export_dependencies(ament_cmake)
# TODO(clalancette): rcl_interfaces shouldn't be exported, but downstream depends on it for now
ament_export_dependencies(rcl_interfaces)
# TODO(clalancette): rcl_logging_interface shouldn't be exported, but downstream depends on it for now
ament_export_dependencies(rcl_logging_interface)
ament_export_dependencies(rcl_yaml_param_parser)
ament_export_dependencies(rcutils)
ament_export_dependencies(rmw)
# TODO(clalancette): rmw_implementation shouldn't be exported, but downstream depends on it for now
ament_export_dependencies(rmw_implementation)
ament_export_dependencies(rosidl_runtime_c)
ament_export_dependencies(type_description_interfaces)
ament_generate_version_header(${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  add_subdirectory(test)
endif()

ament_package(CONFIG_EXTRAS "rcl-extras.cmake")

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)
