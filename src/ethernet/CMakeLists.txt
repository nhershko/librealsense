# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.9.0)

project(realsense2-ethernet VERSION 1.0.0 LANGUAGES CXX C)
# Save the command line compile commands in the build output

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

 set(DEPENDENCIES ${DEPENDENCIES} realsense2)

 file(GLOB SOURCES *.cpp)

add_library(${PROJECT_NAME} SHARED  ethernet-device.cpp ethernet-device.h)

#target_sources(${PROJECT_NAME} ethernet-device.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
#target_link_libraries(${PROJECT_NAME} ${DEPENDENCIES})
target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${DEPENDENCIES}
	realsense2
)

#include_directories(${PROJECT_NAME}  ../../common ../../third-party ../../third-party/tclap/include)
set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Library")

#install(TARGETS rs-software-device RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
