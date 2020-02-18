# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
cmake_minimum_required(VERSION 3.9.0)

project(realsense2-compression VERSION 1.0.0 LANGUAGES CXX C)
# Save the command line compile commands in the build output

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(DEPENDENCIES ${DEPENDENCIES} realsense2)


file(GLOB COMPRESSION_SOURCES 
 "*.hh"
 "*.h"
 "*.cpp"
  "../ipDevice_Common/*.h"
 "../ipDevice_Common/*.cpp"
)

add_library(${PROJECT_NAME} SHARED ${COMPRESSION_SOURCES})

include_directories(${PROJECT_NAME}
        ../../common
        ../ipDevice_Common
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

target_link_libraries(${PROJECT_NAME} 
  PRIVATE ${DEPENDENCIES}
	realsense2
)

find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} 
	${JPEG_LIBRARIES}
)

find_package(JPEG REQUIRED)
target_link_libraries(${PROJECT_NAME} 
	${JPEG_LIBRARIES}
)

find_package(LZ4 REQUIRED)
target_link_libraries(${PROJECT_NAME} 
	${LZ4_LIBRARIES}
)

#
#find_package(PkgConfig)
#
#pkg_check_modules(LZ4_PKGCONF liblz4)
#
#find_path(LZ4_INCLUDE_DIRS
#  NAMES lz4frame.h
#  PATHS ${LZ4_PKGCONF_INCLUDE_DIRS}
#)
#
#find_library(LZ4_LIBRARIES
#  NAMES lz4
#  PATHS ${LZ4_PKGCONF_LIBRARY_DIRS}
#)
#

if(BUILD_COMPRESSION)
    add_definitions(-DCOMPRESSION)
endif()

if(BUILD_IPDEV_STATS)
    add_definitions(-DSTATISTICS)
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Library")

#install(TARGETS rs-software-device RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
