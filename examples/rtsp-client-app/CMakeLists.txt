# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseEthernetSoftwareDevice)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(myClient myClient.cpp)

    include_directories(myClient
	../../src/ethernet
    )

    set_property(TARGET myClient PROPERTY CXX_STANDARD 11)
    target_link_libraries(myClient ${DEPENDENCIES})
    target_link_libraries(myClient realsense2-ethernet)
    include_directories(myClient ../ ../../third-party/tclap/include ../../third-party/ ../../common/res)
    set_target_properties (myClient PROPERTIES FOLDER "Examples")
    install(TARGETS myClient RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
