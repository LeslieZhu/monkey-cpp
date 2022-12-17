cmake_minimum_required(VERSION 3.14)
project(waiicpp)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

MESSAGE("    Build type is ${CMAKE_BUILD_TYPE} [use -DCMAKE_BUILD_TYPE=Release or -DCMAKE_BUILD_TYPE=Debug on the cmake command to configure the build type]")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(${PROJECT_SOURCE_DIR})

find_package(GTest)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(gflags REQUIRED)

add_executable(monkey
  main/monkey.cpp
)

target_link_libraries(monkey
)

add_executable(fibonacci
  benchmark/fibonacci.cpp
)

target_link_libraries(fibonacci /usr/local/lib/libgflags.a)

add_executable(test_monkey
  test/main.cpp
)

target_link_libraries(test_monkey
  ${GTEST_BOTH_LIBRARIES}
)
