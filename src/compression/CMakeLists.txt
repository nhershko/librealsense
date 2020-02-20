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
# "zlib-1.2.11/*.c"
# "jpeg-9c/*.c"
# "lz4/*.c"
)

# add_library(${PROJECT_NAME} SHARED ${COMPRESSION_SOURCES})
add_library(${PROJECT_NAME} STATIC ${COMPRESSION_SOURCES})

include_directories(${PROJECT_NAME}
    ../../common
    ../ipDevice_Common
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

if(WIN32)
    include_directories(${PROJECT_NAME}
        zlib-1.2.11/inc/
        turbo-jpeg/inc/
        lz4/inc/
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE ${DEPENDENCIES}
#        realsense2
        ${CMAKE_SOURCE_DIR}/src/compression/zlib-1.2.11/x86_64/zlibstaticd.lib
        ${CMAKE_SOURCE_DIR}/src/compression/turbo-jpeg/x86_64/jpeg-static.lib
        ${CMAKE_SOURCE_DIR}/src/compression/lz4/x86_64/lz4.lib
    )
else()
    find_package(PkgConfig)
    find_package(ZLIB REQUIRED)
    find_package(JPEG REQUIRED)

   #    target_link_libraries(${PROJECT_NAME}
    #        PRIVATE ${DEPENDENCIES}
    #        realsense2
    #    )

    #    find_package(ZLIB REQUIRED)
    #    target_link_libraries(${PROJECT_NAME}
    #        ${ZLIB_LIBRARIES}
    #    )

    #    find_package(JPEG REQUIRED)
    #    target_link_libraries(${PROJECT_NAME} 
    #        ${JPEG_LIBRARIES}
    #    )

##    find_package(TURBOJPEG REQUIRED)
##    target_link_libraries(${PROJECT_NAME} 
##        ${JPEG_LIBRARIES}
##    )
#
#    pkg_check_modules(TURBOJPEG_PKGCONF libturbojpeg)
#
#    find_path(TURBOJPEG_INCLUDE_DIRS
#        NAMES turbojpeg.h
#        PATHS ${TURBOJPEG_PKGCONF_INCLUDE_DIRS}
#    )
#
#    find_library(LZ4_LIBRARIES
#        NAMES turbojpeg
#        PATHS ${TURBOJPEG_PKGCONF_LIBRARY_DIRS}
#    )

#   find_package(LZ4 REQUIRED)
#   target_link_libraries(${PROJECT_NAME} 
#       ${LZ4_LIBRARIES}
#   )

    pkg_check_modules(LZ4_PKGCONF liblz4)

    find_path(LZ4_INCLUDE_DIRS
        NAMES lz4frame.h
        PATHS ${LZ4_PKGCONF_INCLUDE_DIRS}
    )

    find_library(LZ4_LIBRARIES
        NAMES lz4
        PATHS ${LZ4_PKGCONF_LIBRARY_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
         PRIVATE ${DEPENDENCIES}
         realsense2
         ${ZLIB_LIBRARIES}
         ${JPEG_LIBRARIES}
         ${LZ4_LIBRARIES}
    )

 endif()

if(BUILD_COMPRESSION)
    add_definitions(-DCOMPRESSION)
endif()

if(BUILD_IPDEV_STATS)
    add_definitions(-DSTATISTICS)
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Library")

#if(WIN32)
#    install(TARGETS ${PROJECT_NAME}
#        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#    )
#else()
#    install(TARGETS ${PROJECT_NAME}
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#    )
#endif()


install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
