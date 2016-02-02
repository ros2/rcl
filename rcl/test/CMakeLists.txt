find_package(ament_cmake_gtest REQUIRED)
find_package(std_msgs REQUIRED)

set(extra_test_libraries)
set(extra_memory_tools_env PHONY=)  # Use a phony env var so there is always at least one.
ament_find_gtest()  # For GTEST_LIBRARIES
if(APPLE)
  add_library(${PROJECT_NAME}_memory_tools_interpose SHARED memory_tools_osx_interpose.cpp)
  target_link_libraries(${PROJECT_NAME}_memory_tools_interpose ${GTEST_LIBRARIES})
  set_target_properties(${PROJECT_NAME}_memory_tools_interpose
    PROPERTIES COMPILE_FLAGS "-std=c++11")
  list(APPEND extra_test_libraries ${PROJECT_NAME}_memory_tools_interpose)
  list(APPEND extra_memory_tools_env
    DYLD_INSERT_LIBRARIES=$<TARGET_FILE:${PROJECT_NAME}_memory_tools_interpose>)
endif()
add_library(${PROJECT_NAME}_memory_tools SHARED memory_tools.cpp)
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
  ament_add_gtest(test_memory_tools test_memory_tools.cpp
    ENV ${extra_memory_tools_env} APPEND_LIBRARY_DIRS ${rcl_lib_dir})
  if(TARGET test_memory_tools)
    target_include_directories(test_memory_tools PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_memory_tools PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_memory_tools ${extra_test_libraries})
  endif()
endif()

macro(test_target)
  ament_add_gtest(test_allocator${target_suffix} rcl/test_allocator.cpp
    ENV ${extra_memory_tools_env} APPEND_LIBRARY_DIRS ${rcl_lib_dir})
  if(TARGET test_allocator${target_suffix})
    target_include_directories(test_allocator${target_suffix} PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_allocator${target_suffix} PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_allocator${target_suffix}
      ${PROJECT_NAME}${target_suffix} ${extra_test_libraries})
    ament_target_dependencies(test_allocator${target_suffix} rmw_implementation)
  endif()

  ament_add_gtest(test_time${target_suffix} rcl/test_time.cpp
    ENV ${extra_memory_tools_env} APPEND_LIBRARY_DIRS ${rcl_lib_dir})
  if(TARGET test_time${target_suffix})
    target_include_directories(test_time${target_suffix} PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_time${target_suffix} PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_time${target_suffix}
      ${PROJECT_NAME}${target_suffix} ${extra_test_libraries})
    ament_target_dependencies(test_time${target_suffix} rmw_implementation)
  endif()

  ament_add_gtest(test_common${target_suffix}
    rcl/test_common.cpp
    ENV
      ${extra_memory_tools_env}
      EMPTY_TEST=
      NORMAL_TEST=foo
    APPEND_LIBRARY_DIRS
      ${rcl_lib_dir}
  )
  if(TARGET test_common${target_suffix})
    target_include_directories(test_common${target_suffix} PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_common${target_suffix} PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_common${target_suffix}
      ${PROJECT_NAME}${target_suffix} ${extra_test_libraries})
    ament_target_dependencies(test_common${target_suffix} rmw_implementation)
  endif()

  ament_add_gtest(test_rcl${target_suffix} rcl/test_rcl.cpp
    ENV ${extra_memory_tools_env} APPEND_LIBRARY_DIRS ${rcl_lib_dir})
  if(TARGET test_rcl${target_suffix})
    target_include_directories(test_rcl${target_suffix} PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_rcl${target_suffix} PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_rcl${target_suffix}
      ${PROJECT_NAME}${target_suffix} ${extra_test_libraries})
    ament_target_dependencies(test_rcl${target_suffix} rmw_implementation)
  endif()

  ament_add_gtest(test_node${target_suffix} rcl/test_node.cpp
    ENV ${extra_memory_tools_env} APPEND_LIBRARY_DIRS ${rcl_lib_dir})
  if(TARGET test_node${target_suffix})
    target_include_directories(test_node${target_suffix} PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_node${target_suffix} PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_node${target_suffix}
      ${PROJECT_NAME}${target_suffix} ${extra_test_libraries})
    ament_target_dependencies(test_node${target_suffix} rmw_implementation)
  endif()

  ament_add_gtest(test_publisher${target_suffix} rcl/test_publisher.cpp
    ENV ${extra_memory_tools_env} APPEND_LIBRARY_DIRS ${rcl_lib_dir})
  if(TARGET test_publisher${target_suffix})
    target_include_directories(test_publisher${target_suffix} PUBLIC
      ${rcl_interfaces_INCLUDE_DIRS}
      ${rmw_INCLUDE_DIRS}
      ${std_msgs_INCLUDE_DIRS}
    )
    if(NOT WIN32)
      set_target_properties(test_publisher${target_suffix} PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
    target_link_libraries(test_publisher${target_suffix}
      ${PROJECT_NAME}${target_suffix} ${extra_test_libraries} ${std_msgs_LIBRARIES})
    ament_target_dependencies(test_publisher${target_suffix} rmw_implementation std_msgs)
  endif()
endmacro()

call_for_each_rmw_implementation(test_target)
