set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

find_package(ament_cmake_gtest REQUIRED)
find_package(rcl REQUIRED)

# Gtests
ament_add_gtest(test_default_state_machine
  test_default_state_machine.cpp
)
if(TARGET test_default_state_machine)
  target_include_directories(test_default_state_machine PUBLIC
    ${rcl_INCLUDE_DIRS}
    ${rcl_lifecycle_INCLUDE_DIRS}
  )
  target_link_libraries(test_default_state_machine ${PROJECT_NAME})
endif()
