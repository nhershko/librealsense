# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.9.0)

project(realsense2-ethernet VERSION 1.0.0 LANGUAGES CXX C)
# Save the command line compile commands in the build output

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(DEPENDENCIES ${DEPENDENCIES} realsense2)

# set(LIVE third-party/live555)
set(LIVE ../../third-party/live/)

file(GLOB ETHERNET_SOURCES 
 "*.hh"
 "*.h"
 "*.cpp"
 "rtp_stream.cpp"
 "rtp_callback.cpp"
 "rtsp_client/*.cpp"
 ${LIVE}/groupsock/*.c* 
 ${LIVE}/liveMedia/*.c* 
 ${LIVE}/UsageEnvironment/*.c* 
 ${LIVE}/BasicUsageEnvironment/*.c* 
)

add_library(${PROJECT_NAME} SHARED ${ETHERNET_SOURCES})

include_directories(${PROJECT_NAME}
        ../../common
	#/usr/include/BasicUsageEnvironment
	#/usr/include/UsageEnvironment
	#/usr/include/groupsock
	#/usr/include/liveMedia
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# target_link_libraries(${PROJECT_NAME} ${DEPENDENCIES})

find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${DEPENDENCIES}
	realsense2
	#BasicUsageEnvironment
	#UsageEnvironment
	#groupsock
	#liveMedia
	${ZLIB_LIBRARIES}
	${JPEG_LIBRARIES}
)

if(COMPRESSION)
    add_definitions(-DCOMPRESSION)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "rtsp_client/*.h")

target_include_directories(${PROJECT_NAME} PUBLIC ${LIVE}/groupsock/include ${LIVE}/liveMedia/include  ${LIVE}/UsageEnvironment/include ${LIVE}/BasicUsageEnvironment/include inc)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Library")

#install(TARGETS rs-software-device RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
