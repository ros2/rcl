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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
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
endmacro()

call_for_each_rmw_implementation(target GENERATE_DEFAULT)

ament_export_dependencies(rcl_interfaces)
ament_export_dependencies(rmw)
ament_export_dependencies(rmw_implementation)
ament_export_dependencies(rosidl_generator_c)

ament_export_include_directories(include)

ament_export_libraries(${PROJECT_NAME})

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)

  set(extra_test_libraries)
  set(extra_memory_tools_env PHONY=)  # Use a phony env var so there is always at least one.
  ament_find_gtest()  # For GTEST_LIBRARIES
  if(APPLE)
    add_library(${PROJECT_NAME}_memory_tools_interpose SHARED test/memory_tools_osx_interpose.cpp)
    target_link_libraries(${PROJECT_NAME}_memory_tools_interpose ${GTEST_LIBRARIES})
    set_target_properties(${PROJECT_NAME}_memory_tools_interpose
      PROPERTIES COMPILE_FLAGS "-std=c++11")
    list(APPEND extra_test_libraries ${PROJECT_NAME}_memory_tools_interpose)
    list(APPEND extra_memory_tools_env
      DYLD_INSERT_LIBRARIES=$<TARGET_FILE:${PROJECT_NAME}_memory_tools_interpose>)
  endif()
  add_library(${PROJECT_NAME}_memory_tools SHARED test/memory_tools.cpp)
  if(NOT WIN32)
    set_target_properties(${PROJECT_NAME}_memory_tools PROPERTIES COMPILE_FLAGS "-std=c++11")
  endif()
  if(UNIX AND NOT APPLE)
    list(APPEND extra_test_libraries dl)
    list(APPEND extra_memory_tools_env DL_PRELOAD=$<TARGET_FILE:${PROJECT_NAME}_memory_tools>)
  endif()
  target_link_libraries(${PROJECT_NAME}_memory_tools ${extra_test_libraries})
  target_compile_definitions(${PROJECT_NAME}_memory_tools
    PRIVATE "RCL_MEMORY_TOOLS_BUILDING_DLL")
  list(APPEND extra_test_libraries ${PROJECT_NAME}_memory_tools)

  if(NOT WIN32)
    ament_add_gtest(test_memory_tools test/test_memory_tools.cpp ENV ${extra_memory_tools_env})
    if(TARGET test_memory_tools)
      target_include_directories(test_memory_tools PUBLIC
        ${rcl_interfaces_INCLUDE_DIRS}
        ${rmw_INCLUDE_DIRS}
      )
      if(NOT WIN32)
        set_target_properties(test_memory_tools PROPERTIES COMPILE_FLAGS "-std=c++11")
      endif()
      target_link_libraries(test_memory_tools ${PROJECT_NAME} ${extra_test_libraries})
    endif()
  endif()

  ament_add_gtest(test_allocator test/rcl/test_allocator.cpp ENV ${extra_memory_tools_env})
  if(TARGET test_allocator)
    target_include_directories(test_allocator PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_allocator PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_allocator ${PROJECT_NAME} ${extra_test_libraries})
  endif()

  ament_add_gtest(test_time test/rcl/test_time.cpp ENV ${extra_memory_tools_env})
  if(TARGET test_time)
    target_include_directories(test_time PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_time PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_time ${PROJECT_NAME} ${extra_test_libraries})
  endif()

  ament_add_gtest(test_common
    test/rcl/test_common.cpp
    ENV
      ${extra_memory_tools_env}
      EMPTY_TEST=
      NORMAL_TEST=foo
  )
  if(TARGET test_common)
    target_include_directories(test_common PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_common PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_common ${PROJECT_NAME} ${extra_test_libraries})
  endif()

  ament_add_gtest(test_rcl test/rcl/test_rcl.cpp ENV ${extra_memory_tools_env})
  if(TARGET test_rcl)
    target_include_directories(test_rcl PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_rcl PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_rcl ${PROJECT_NAME} ${extra_test_libraries})
  endif()

  ament_add_gtest(test_node test/rcl/test_node.cpp ENV ${extra_memory_tools_env})
  if(TARGET test_node)
    target_include_directories(test_node PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_node PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_node ${PROJECT_NAME} ${extra_test_libraries})
  endif()
endif()

ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)
