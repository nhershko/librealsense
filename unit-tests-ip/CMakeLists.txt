#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseUnitTestsIp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(DEPENDENCIES realsense2)

################################
# GTest
################################
## enable_testing()
set(gtest_SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/third-party/googletest-1.10.x/googletest/)

file(GLOB GTEST_SOURCES 
 ${gtest_SOURCE_DIR}/src/*.h*
 ${gtest_SOURCE_DIR}/src/*.c*
)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

################################
# Unit Tests
################################
set (unit_tests_ip_sources
    unit-tests-ip.cpp
    ${gtest_SOURCE_DIR}/src/gtest-all.cc
)
# Add test cpp file
add_executable(ip-test ${unit_tests_ip_sources})
target_link_libraries(ip-test ${DEPENDENCIES})
target_link_libraries(ip-test realsense2-ethernet)
