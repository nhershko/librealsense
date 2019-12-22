# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseToolsRealSenseServer)

include_directories(${CMAKE_BINARY_DIR})

include(../../common/CMakeLists.txt)

#set(RS_SERVER_SOURCES
#        rs-server.cpp
#	RsSource.cpp
#)

set(LIVE ../../third-party/live/)

file(GLOB RS_SERVER_SOURCES
 "*.h"
 "*.cpp"
 "rtsp_client/*.cpp"
 ${LIVE}/groupsock/*.c*
 ${LIVE}/liveMedia/*.c*
 ${LIVE}/UsageEnvironment/*.c*
 ${LIVE}/BasicUsageEnvironment/*.c*
)


# config-ui
if(WIN32)
    message(SEND_ERROR "rs-server supports for Linux only")
else()
    add_executable(rs-server ${RS_SERVER_SOURCES})
    include_directories(rs-server 
        ../../include/librealsense2
	#/usr/include/BasicUsageEnvironment
	#/usr/include/UsageEnvironment
	#/usr/include/groupsock
	#/usr/include/liveMedia
    )
endif()

set_property(TARGET rs-server PROPERTY CXX_STANDARD 11)

find_package(ZLIB REQUIRED)

target_link_libraries(rs-server ${DEPENDENCIES} realsense2 ${ZLIB_LIBRARIES})

set_target_properties (rs-server PROPERTIES
    FOLDER Tools
)

install(
    TARGETS

    rs-server

    RUNTIME DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)
