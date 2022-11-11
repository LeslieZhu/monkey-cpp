cmake_minimum_required(VERSION 3.14)
project(waiicpp)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)

find_package(GTest)
include_directories(${GTEST_INCLUDE_DIRS})


add_executable(
  hello_test
  test/hello_test.cpp
)

target_link_libraries(
  hello_test
  ${GTEST_BOTH_LIBRARIES}
)
