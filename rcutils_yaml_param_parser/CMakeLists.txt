cmake_minimum_required(VERSION 3.5)

project(rcutils_yaml_param_parser)

find_package(ament_cmake_ros REQUIRED)
find_package(yaml REQUIRED)
find_package(rcutils REQUIRED)

include_directories(include ${yaml_INCLUDE_DIRS} ${rcutils_INCLUDE_DIRS})

set(rcutils_yaml_sources
  src/parser.c
)

add_library(
  ${PROJECT_NAME}
  ${rcutils_yaml_sources})
ament_target_dependencies(${PROJECT_NAME} "yaml" "rcutils")

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  find_package(ament_cmake_gmock REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_cmake_pytest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  set(extra_test_libraries)
  set(extra_test_env)
  set(extra_lib_dirs)

  set(SKIP_TEST_IF_WIN32 "")
  if(WIN32)  # (memory tools doesn't do anything on Windows)
    set(SKIP_TEST_IF_WIN32 "SKIP_TEST")
  endif()

  macro(rcutils_custom_add_gtest target)
    ament_add_gtest(${target} ${ARGN})
  endmacro()

  macro(rcutils_custom_add_gmock target)
    ament_add_gmock(${target} ${ARGN})
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
