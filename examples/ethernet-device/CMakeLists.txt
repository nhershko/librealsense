# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseEthernetSoftwareDevice)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(rs-ethernet-device rs-ethernet-device.cpp)

    include_directories(rs-ethernet-device
        ../../common
        /usr/include/BasicUsageEnvironment
        /usr/include/UsageEnvironment
        /usr/include/groupsock
        /usr/include/liveMedia
    )

    set_property(TARGET rs-ethernet-device PROPERTY CXX_STANDARD 11)
    target_link_libraries(rs-ethernet-device ${DEPENDENCIES})
    target_link_libraries(rs-ethernet-device realsense2-ethernet)
    include_directories(rs-ethernet-device ../ ../../third-party/tclap/include ../../third-party/ ../../common/res)
    set_target_properties (rs-ethernet-device PROPERTIES FOLDER "Examples")
    install(TARGETS rs-ethernet-device RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
