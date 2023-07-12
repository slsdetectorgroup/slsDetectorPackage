# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
set(SOURCES
    src/string_utils.cpp
    src/file_utils.cpp
    src/ClientSocket.cpp
    src/DataSocket.cpp
    src/ServerSocket.cpp
    src/ServerInterface.cpp
    src/ToString.cpp
    src/network_utils.cpp
    src/ZmqSocket.cpp
    src/UdpRxSocket.cpp
    src/sls_detector_exceptions.cpp
    src/md5_helper.cpp
    src/Version.cpp
)

# Header files to install as a part of the library
set(PUBLICHEADERS
    include/sls/sls_detector_defs.h
    include/sls/sls_detector_exceptions.h
    include/sls/container_utils.h
    include/sls/string_utils.h
    include/sls/network_utils.h
    include/sls/ToString.h
    include/sls/TypeTraits.h
    include/sls/TimeHelper.h
)

# Additional headers to be installed if SLS_DEVEL_HEADERS
# is specified as an option. Aim is to give the developer
# access to utilities in the support library while
# at the same time offer a minimal install
if(SLS_DEVEL_HEADERS)
    set(PUBLICHEADERS
        ${PUBLICHEADERS}
        include/sls/ansi.h
        include/sls/logger.h
        include/sls/file_utils.h
        include/sls/sls_detector_funcs.h
        include/sls/ClientSocket.h
        include/sls/DataSocket.h
        include/sls/ServerSocket.h
        include/sls/ServerInterface.h
        include/sls/Timer.h
        include/sls/StaticVector.h
        include/sls/UdpRxSocket.h
        include/sls/versionAPI.h
        include/sls/ZmqSocket.h
        include/sls/bit_utils.h
        include/sls/md5.h
        include/sls/md5_helper.h
        include/sls/Version.h
    )
endif()


# Library for md5 c code that we are using (and potentially other c code)
# Maybe this should be broken out into it's own folder etc.
add_library(md5sls STATIC 
    src/md5.c
)

target_include_directories(md5sls 
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)


# Create an object library to avoid building the library twice
add_library(slsSupportObject OBJECT
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(slsSupportObject 
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
    ${SLS_INTERNAL_RAPIDJSON_DIR}
)

message(STATUS "RAPID: ${SLS_INTERNAL_RAPIDJSON_DIR}")

target_link_libraries(slsSupportObject 
  PUBLIC
    slsProjectOptions 
    
  PRIVATE
    slsProjectWarnings
    md5sls     
    "$<BUILD_INTERFACE:libzmq-static>"
)

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)

#List of targets to support adding removing targets as config
set(SUPPORT_LIBRARY_TARGETS slsSupportObject)

# Add shared library version of the support lib
if(SLS_BUILD_SHARED_LIBRARIES)
    add_library(slsSupportShared SHARED $<TARGET_OBJECTS:slsSupportObject>)
    target_link_libraries(slsSupportShared PUBLIC slsSupportObject)

    set_target_properties(slsSupportShared PROPERTIES
        VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}
        SOVERSION ${PACKAGE_VERSION_MAJOR}
        LIBRARY_OUTPUT_NAME SlsSupport
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER "${PUBLICHEADERS}"
    )
    list(APPEND SUPPORT_LIBRARY_TARGETS slsSupportShared)
endif(SLS_BUILD_SHARED_LIBRARIES)

# Add static version of the support lib
add_library(slsSupportStatic STATIC $<TARGET_OBJECTS:slsSupportObject>)
target_link_libraries(slsSupportStatic PUBLIC slsSupportObject)

set_target_properties(slsSupportStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsSupportStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)
list(APPEND SUPPORT_LIBRARY_TARGETS slsSupportStatic)

if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET ${SUPPORT_LIBRARY_TARGETS} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()


list(APPEND SUPPORT_LIBRARY_TARGETS md5sls)

install(TARGETS ${SUPPORT_LIBRARY_TARGETS}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
)


