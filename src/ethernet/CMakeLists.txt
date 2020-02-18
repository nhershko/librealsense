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

file(GLOB IPDEV_SOURCES 
 "*.hh"
 "*.h"
 "*.cpp"
 "rtp_stream.cpp"
 "rtp_callback.cpp"
 "rtsp_client/*.cpp"
 "../ipDevice_Common/*.h"
 "../ipDevice_Common/*.cpp"
 ${LIVE}/groupsock/*.c* 
 ${LIVE}/liveMedia/*.c* 
 ${LIVE}/UsageEnvironment/*.c* 
 ${LIVE}/BasicUsageEnvironment/*.c* 
 
)

add_library(${PROJECT_NAME} SHARED ${IPDEV_SOURCES})

if(WIN32)
	#
	# Windows doesn't support POSIX ssize_t type 
	# by defaultso we use the following defines:
	#
	# #ifdef _WIN64
	# #  define ssize_t __int64
	# #else
	# #  define ssize_t long
	# #endif
	#
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		# 64-bit environment...
		message(STATUS "Target is 64 bits")
		add_definitions(-Dssize_t=__int64)
	else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		# 32-bit environment...
		message(STATUS "Target is 32 bits")
		add_definitions(-Dssize_t=long)
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
endif()

add_library(${PROJECT_NAME} SHARED ${ETHERNET_SOURCES})

include_directories(${PROJECT_NAME}
        ../../common
		../ipDevice_Common
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${DEPENDENCIES}
	realsense2
)

if(BUILD_COMPRESSION)
    add_definitions(-DCOMPRESSION)
	target_link_libraries(${PROJECT_NAME} 
		realsense2-compression
	)
endif()

if(IPDEV_STATS)
    add_definitions(-DSTATISTICS)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "rtsp_client/*.h")

target_include_directories(${PROJECT_NAME} PUBLIC ${LIVE}/groupsock/include ${LIVE}/liveMedia/include  ${LIVE}/UsageEnvironment/include ${LIVE}/BasicUsageEnvironment/include inc)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Library")

#install(TARGETS rs-software-device RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(WIN32)
	install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
else()
	install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
endif()
