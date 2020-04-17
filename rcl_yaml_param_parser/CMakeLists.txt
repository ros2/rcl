cmake_minimum_required(VERSION 3.5)

project(rcl_yaml_param_parser)

find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)
find_package(libyaml_vendor REQUIRED)
find_package(yaml REQUIRED)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(rcl_yaml_parser_sources
  src/parser.c
)

add_library(
  ${PROJECT_NAME}
  ${rcl_yaml_parser_sources})
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")
ament_target_dependencies(${PROJECT_NAME} "yaml" "rcutils")

# Set the visibility to hidden by default if possible
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  # Set the visibility of symbols to hidden by default for gcc and clang
  # (this is already the default on Windows)
  set_target_properties(${PROJECT_NAME}
    PROPERTIES
    COMPILE_FLAGS "-fvisibility=hidden"
  )
endif()

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "RCL_YAML_PARAM_PARSER_BUILDING_DLL")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  find_package(osrf_testing_tools_cpp REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # Gtests
  ament_add_gtest(test_parse_yaml
    test/test_parse_yaml.cpp
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  if(TARGET test_parse_yaml)
    target_link_libraries(test_parse_yaml ${PROJECT_NAME})
    target_include_directories(test_parse_yaml
      PRIVATE ${osrf_testing_tools_cpp_INCLUDE_DIRS})
  endif()

endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(include)
install(
  DIRECTORY include/
  DESTINATION include
)
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})
ament_package()
