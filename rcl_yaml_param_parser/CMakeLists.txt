cmake_minimum_required(VERSION 3.5)

project(rcl_yaml_param_parser)

find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)
find_package(rcl REQUIRED)
find_package(libyaml_vendor REQUIRED)
find_package(yaml REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

include_directories(include ${yaml_INCLUDE_DIRS} ${rcutils_INCLUDE_DIRS} ${rcl_INCLUDE_DIRS})

set(rcl_yaml_parser_sources
  src/parser.c
)

add_library(
  ${PROJECT_NAME}
  ${rcl_yaml_parser_sources})
ament_target_dependencies(${PROJECT_NAME} "yaml" "rcutils" "rcl")

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  macro(rcutils_custom_add_gtest target)
    ament_add_gtest(${target} ${ARGN})
  endmacro()

  # Gtests
  rcutils_custom_add_gtest(test_parse_yaml
    test/test_parse_yaml.cpp
  )
  if(TARGET test_parse_yaml)
    target_link_libraries(test_parse_yaml ${PROJECT_NAME})
  endif()

endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()
