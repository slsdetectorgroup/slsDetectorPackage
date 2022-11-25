# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
cmake_minimum_required(VERSION 3.12)
project(slsDetectorPackage)
set(PROJECT_VERSION 6.1.2)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

cmake_policy(SET CMP0074 NEW)
include(cmake/project_version.cmake)

#functions to add compiler flags
include(cmake/SlsAddFlag.cmake)

# Include additional modules that are used unconditionally
include(GNUInstallDirs)

# If conda build, always set lib dir to 'lib'
if($ENV{CONDA_BUILD})
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Set lower / upper case project names
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)

# Set targets export name (used by slsDetectorPackage and dependencies)
set(TARGETS_EXPORT_NAME "${PROJECT_NAME_LOWER}-targets")
#set(namespace "${PROJECT_NAME}::")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


# Check if project is being used directly or via add_subdirectory
set(SLS_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SLS_MASTER_PROJECT ON)
endif()

option(SLS_USE_HDF5 "HDF5 File format" OFF)
option(SLS_BUILD_SHARED_LIBRARIES "Build shared libaries" ON)
option(SLS_USE_TEXTCLIENT "Text Client" ON)
option(SLS_USE_DETECTOR "Detector libs" ON)
option(SLS_USE_RECEIVER "Receiver" ON)
option(SLS_USE_RECEIVER_BINARIES "Receiver binaries" ON)
option(SLS_USE_GUI "GUI" OFF)
option(SLS_USE_SIMULATOR "Simulator" OFF)
option(SLS_USE_TESTS "TESTS" OFF)
option(SLS_USE_INTEGRATION_TESTS "Integration Tests" OFF)
option(SLS_USE_SANITIZER "Sanitizers for debugging" OFF)
option(SLS_USE_PYTHON "Python bindings" OFF)
option(SLS_USE_CTBGUI "ctb GUI" OFF)
option(SLS_BUILD_DOCS "docs" OFF)
option(SLS_BUILD_EXAMPLES "examples" OFF)
option(SLS_TUNE_LOCAL "tune to local machine" OFF)
option(SLS_DEVEL_HEADERS "install headers for devel" OFF)
option(SLS_USE_MOENCH "compile zmq and post processing for Moench" OFF)


set(ClangFormat_EXCLUDE_PATTERNS    "build/" 
                                    "libs/" 
                                    "slsDetectorCalibration/" 
                                    "ctbGui/" 
                                    "manual/"
                                    "python/"
                                    "sample/"
                                    ${CMAKE_BINARY_DIR})
find_package(ClangFormat)




set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()


#Enable LTO if available
include(CheckIPOSupported)
check_ipo_supported(RESULT SLS_LTO_AVAILABLE)
if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    message(STATUS "Building with link time optimization")
else()
    message(STATUS "Building without link time optimization")
endif()


#Add two fake libraries to manage options
add_library(slsProjectOptions INTERFACE)
add_library(slsProjectWarnings INTERFACE)
target_compile_features(slsProjectOptions INTERFACE cxx_std_11)
target_compile_options(slsProjectWarnings INTERFACE 
                                            -Wall
                                            -Wextra
                                            -Wno-unused-parameter
                                            # -Wold-style-cast
                                            -Wnon-virtual-dtor
                                            -Woverloaded-virtual
                                            -Wdouble-promotion
                                            -Wformat=2
                                            -Wredundant-decls
                                            # -Wconversion
                                            -Wvla
                                            -Wdouble-promotion
                                            -Werror=return-type
                                 )

#Settings for C code
add_library(slsProjectCSettings INTERFACE)
target_compile_options(slsProjectCSettings INTERFACE 
                                            -std=gnu99 #fixed
                                            -Wall
                                            -Wextra
                                            -Wno-unused-parameter
                                            -Wdouble-promotion
                                            -Wformat=2
                                            -Wredundant-decls
                                            -Wdouble-promotion
                                            -Werror=return-type
                                )


#Testing for minimum version for compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
        message(FATAL_ERROR "Clang version must be at least 3.2!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
            message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
            target_compile_options(slsProjectWarnings INTERFACE 
                                             -Wno-missing-field-initializers)
    endif()
endif()

# Add or disable warnings depending on if the compiler supports them 
# The function checks internally and sets HAS_warning-name
sls_enable_cxx_warning("-Wnull-dereference")
sls_enable_cxx_warning("-Wduplicated-cond")
sls_disable_cxx_warning("-Wclass-memaccess")
sls_disable_c_warning("-Wstringop-truncation")


if(SLS_USE_SANITIZER)
    target_compile_options(slsProjectOptions INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_libraries(slsProjectOptions INTERFACE -fsanitize=address,undefined)
    # target_compile_options(slsProjectOptions INTERFACE -fsanitize=thread)
    # target_link_libraries(slsProjectOptions INTERFACE -fsanitize=thread)
endif()

if(SLS_TUNE_LOCAL)
    target_compile_options(slsProjectOptions INTERFACE -mtune=native -march=native)
endif()


#rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/rapidjson>  
)

# Install fake the libraries
install(TARGETS slsProjectOptions slsProjectWarnings rapidjson
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH $ORIGIN)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)


set(ZeroMQ_HINT "" CACHE STRING "Hint where ZeroMQ could be found")
#Adapted from: https://github.com/zeromq/cppzmq/
if (NOT TARGET libzmq)
    if(ZeroMQ_HINT)
        message(STATUS "Looking for ZeroMQ in: ${ZeroMQ_HINT}")
        find_package(ZeroMQ 4 
            NO_DEFAULT_PATH
            HINTS ${ZeroMQ_DIR}
        )
    else()
        find_package(ZeroMQ 4 QUIET)
    endif()
    
  # libzmq autotools install: fallback to pkg-config
  if(NOT ZeroMQ_FOUND)
    message(STATUS "CMake libzmq package not found, trying again with pkg-config (normal install of zeromq)")
    list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/libzmq-pkg-config)
    find_package(ZeroMQ 4 REQUIRED)
  endif()

  # TODO "REQUIRED" above should already cause a fatal failure if not found, but this doesn't seem to work
  if(NOT ZeroMQ_FOUND)
    message(FATAL_ERROR "ZeroMQ was not found, neither as a CMake package nor via pkg-config")
  endif()

  if (ZeroMQ_FOUND AND NOT TARGET libzmq)
    message(FATAL_ERROR "ZeroMQ version not supported!")
  endif()
endif()

if (SLS_USE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


# Common functionallity to detector and receiver
add_subdirectory(slsSupportLib)

if (SLS_USE_DETECTOR OR SLS_USE_TEXTCLIENT)
    add_subdirectory(slsDetectorSoftware)
endif ()

if (SLS_USE_RECEIVER)
    add_subdirectory(slsReceiverSoftware) 
endif (SLS_USE_RECEIVER)

if (SLS_USE_GUI)   
    add_subdirectory(slsDetectorGui)
endif (SLS_USE_GUI)

if (SLS_USE_SIMULATOR)
    add_subdirectory(slsDetectorServers)
endif (SLS_USE_SIMULATOR)

if (SLS_USE_INTEGRATION_TESTS)
    add_subdirectory(integrationTests)
endif (SLS_USE_INTEGRATION_TESTS)

if (SLS_USE_PYTHON)
    find_package (Python 3.6 COMPONENTS Interpreter Development)
    add_subdirectory(libs/pybind11)
    add_subdirectory(python)
endif(SLS_USE_PYTHON)

if (SLS_USE_CTBGUI)
    add_subdirectory(ctbGui)
endif(SLS_USE_CTBGUI)

configure_file( .clang-tidy  
    ${CMAKE_BINARY_DIR}/.clang-tidy  
)

if (SLS_BUILD_EXAMPLES)
    add_subdirectory(sample)
endif(SLS_BUILD_EXAMPLES)

if(SLS_BUILD_DOCS)
    add_subdirectory(docs)
endif(SLS_BUILD_DOCS)

if(SLS_USE_MOENCH)
    add_subdirectory(slsDetectorCalibration/moenchExecutables)
endif(SLS_USE_MOENCH)

if(SLS_MASTER_PROJECT)
    set(CMAKE_INSTALL_DIR "share/cmake/${PROJECT_NAME}")
    set(PROJECT_LIBRARIES slsSupportShared slsDetectorShared slsReceiverShared)
    include(cmake/package_config.cmake)
endif()
