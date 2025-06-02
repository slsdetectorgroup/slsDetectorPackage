# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
cmake_minimum_required(VERSION 3.15)
project(slsDetectorPackage)

# Read VERSION file into project version
set(VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
file(READ "${VERSION_FILE}" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" PROJECT_VERSION_STRING)
set(PROJECT_VERSION ${PROJECT_VERSION_STRING})

# Pass it to the compiler
add_compile_definitions(SLS_DET_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

cmake_policy(SET CMP0074 NEW)

if (${CMAKE_VERSION} VERSION_GREATER "3.24")
    cmake_policy(SET CMP0135 NEW) #Fetch content download timestamp
endif()
include(cmake/project_version.cmake)
include(cmake/SlsAddFlag.cmake)



# Using FetchContent to get libzmq
include(FetchContent)
option(SLS_FETCH_ZMQ_FROM_GITHUB "Fetch zmq from github" OFF)
option(SLS_FETCH_PYBIND11_FROM_GITHUB "Fetch pybind11 from github" OFF)



# Allow FetchContent_Populate to be called with a single argument
# otherwise deprecated warning is issued
# Note: From cmake 3.28 we can pass EXCLUDE_FROM_ALL to FetchContent_Declare
# and avoid direct use of Populate
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.30")
    cmake_policy(SET CMP0169 OLD) 
endif()

# Patch libzmq to set minimum cmake version to 3.15 to avoid warnings
# with newer cmake versions
# Patch is applied in the FetchContent_Declare
set(SLS_LIBZMQ_VERSION "4.3.4")


if(SLS_FETCH_ZMQ_FROM_GITHUB)
    # Opt in to pull down a zmq version from github instead of
    # using the bundled version
    FetchContent_Declare(
        libzmq
        GIT_REPOSITORY https://github.com/zeromq/libzmq.git
        GIT_TAG        v${SLS_LIBZMQ_VERSION}
        PATCH_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/libs/libzmq/libzmq_cmake_version.patch
        UPDATE_DISCONNECTED 1
    )
else()
    # Standard behaviour use libzmq included in this repo (libs/libzmq)
    FetchContent_Declare(
        libzmq
        URL ${CMAKE_CURRENT_SOURCE_DIR}/libs/libzmq/libzmq-${SLS_LIBZMQ_VERSION}.tar.gz
        URL_HASH MD5=cc20b769ac10afa352e5ed2769bb23b3
        PATCH_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/libs/libzmq/libzmq_cmake_version.patch
        UPDATE_DISCONNECTED 1
    )
endif()

# Disable unwanted options from libzmq
set(BUILD_TESTS OFF CACHE BOOL "Switch off libzmq test build")
set(BUILD_SHARED OFF CACHE BOOL "Switch off libzmq shared libs")
set(WITH_PERF_TOOL OFF CACHE BOOL "")
set(ENABLE_CPACK OFF CACHE BOOL "")
set(ENABLE_CLANG OFF CACHE BOOL "")
set(ENABLE_CURVE OFF CACHE BOOL "")
set(ENABLE_DRAFTS OFF CACHE BOOL "")
set(ENABLE_PRECOMPILED OFF CACHE BOOL "")
set(WITH_DOC OFF CACHE BOOL "")
set(WITH_DOCS OFF CACHE BOOL "")

 

# Using GetProperties and Populate to be able to exclude zmq
# from install (not possible with FetchContent_MakeAvailable(libzmq))
FetchContent_GetProperties(libzmq)
if(NOT libzmq_POPULATED)
  FetchContent_Populate(libzmq)
  add_subdirectory(${libzmq_SOURCE_DIR} ${libzmq_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()



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
set(namespace "sls::")

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
option(SLS_INSTALL_PYTHONEXT "Install the python extension in the install tree under CMAKE_INSTALL_PREFIX/python/" OFF)
option(SLS_USE_CTBGUI "ctb GUI" OFF)
option(SLS_BUILD_DOCS "docs" OFF)
option(SLS_BUILD_EXAMPLES "examples" OFF)
option(SLS_TUNE_LOCAL "tune to local machine" OFF)
option(SLS_DEVEL_HEADERS "install headers for devel" OFF)
option(SLS_USE_MOENCH "compile zmq and post processing for Moench" OFF)
option(SLS_USE_JUNGFRAU "compile post processing for Jungfrau" OFF)

#Convenience option to switch off defaults when building Moench binaries only
option(SLS_BUILD_ONLY_MOENCH "compile only Moench" OFF)
if(SLS_BUILD_ONLY_MOENCH)
    message(STATUS "Build MOENCH binaries only!")
    set(SLS_BUILD_SHARED_LIBRARIES OFF CACHE BOOL "Disabled for MOENCH_ONLY" FORCE)
    set(SLS_USE_TEXTCLIENT OFF CACHE BOOL "Disabled for MOENCH_ONLY" FORCE)
    set(SLS_USE_DETECTOR OFF CACHE BOOL "Disabled for MOENCH_ONLY" FORCE)
    set(SLS_USE_RECEIVER OFF CACHE BOOL "Disabled for MOENCH_ONLY" FORCE)
    set(SLS_USE_RECEIVER_BINARIES OFF CACHE BOOL "Disabled for MOENCH_ONLY" FORCE)
    set(SLS_USE_MOENCH ON CACHE BOOL "Enable" FORCE)
endif()

#Convenience option to switch off defaults when building Jungfrau binaries only
option(SLS_BUILD_ONLY_JUNGFRAU "compile only Jungfrau" OFF)
if(SLS_BUILD_ONLY_JUNGFRAU)
    message(STATUS "Build JUNGFRAU binaries only!")
    set(SLS_BUILD_SHARED_LIBRARIES OFF CACHE BOOL "Disabled for JUNGFRAU_ONLY" FORCE)
    set(SLS_USE_TEXTCLIENT OFF CACHE BOOL "Disabled for JUNGFRAU_ONLY" FORCE)
    set(SLS_USE_DETECTOR OFF CACHE BOOL "Disabled for JUNGFRAU_ONLY" FORCE)
    set(SLS_USE_RECEIVER OFF CACHE BOOL "Disabled for JUNGFRAU_ONLY" FORCE)
    set(SLS_USE_RECEIVER_BINARIES OFF CACHE BOOL "Disabled for JUNGFRAU_ONLY" FORCE)
    set(SLS_USE_JUNGFRAU ON CACHE BOOL "Enable" FORCE)
endif()


option(SLS_EXT_BUILD "external build of part of the project" OFF)
if(SLS_EXT_BUILD)
    message(STATUS "External build using already installed libraries")
    set(SLS_BUILD_SHARED_LIBRARIES OFF CACHE BOOL "Should already exist" FORCE)
    set(SLS_USE_TEXTCLIENT OFF CACHE BOOL "Should already exist" FORCE)
    set(SLS_USE_DETECTOR OFF CACHE BOOL "Should already exist" FORCE)
    set(SLS_USE_RECEIVER OFF CACHE BOOL "Should already exist" FORCE)
    set(SLS_USE_RECEIVER_BINARIES OFF CACHE BOOL "Should already exist" FORCE)
    set(SLS_MASTER_PROJECT OFF CACHE BOOL "No master proj in case of extbuild" FORCE) 
endif()

#Maybe have an option guarding this?
set(SLS_INTERNAL_RAPIDJSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/rapidjson)

set(SLS_INTERNAL_QWT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/qwt-6.1.5)

set(ClangFormat_EXCLUDE_PATTERNS    "build/" 
                                    "libs/" 
                                    "slsDetectorCalibration/" 
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


if(SLS_EXT_BUILD)
    # Find ourself in case of external build
    find_package(slsDetectorPackage ${PROJECT_VERSION} REQUIRED)
endif()



# slsProjectOptions and slsProjectWarnings are used
# to control options for the libraries
if(NOT TARGET slsProjectOptions)
    add_library(slsProjectOptions INTERFACE)
   target_compile_features(slsProjectOptions INTERFACE cxx_std_11)
endif()

if (NOT TARGET slsProjectWarnings)
    add_library(slsProjectWarnings INTERFACE)
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
    # Add or disable warnings depending on if the compiler supports them 
    # The function checks internally and sets HAS_warning-name
    sls_enable_cxx_warning("-Wnull-dereference")
    sls_enable_cxx_warning("-Wduplicated-cond")
    sls_disable_cxx_warning("-Wclass-memaccess")

    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            target_compile_options(slsProjectWarnings INTERFACE 
                                                -Wno-missing-field-initializers)
    endif()


endif()


if (NOT TARGET slsProjectCSettings)
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
                                                -Wno-format-overflow 
                                                -Wno-format-truncation
                                    )
    sls_disable_c_warning("-Wstringop-truncation")
endif()


if(SLS_USE_SANITIZER)
    target_compile_options(slsProjectOptions INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_libraries(slsProjectOptions INTERFACE -fsanitize=address,undefined)
    #target_compile_options(slsProjectOptions INTERFACE -fsanitize=thread -fno-omit-frame-pointer)
    #target_link_libraries(slsProjectOptions INTERFACE -fsanitize=thread)
endif()


if(SLS_TUNE_LOCAL)
    target_compile_options(slsProjectOptions INTERFACE -mtune=native -march=native)
endif()


if(SLS_MASTER_PROJECT)
install(TARGETS slsProjectOptions slsProjectWarnings 
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH $ORIGIN)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

if (SLS_USE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


if(NOT SLS_EXT_BUILD)
    add_subdirectory(slsSupportLib)
endif()

if (SLS_USE_DETECTOR OR SLS_USE_TEXTCLIENT)
    add_subdirectory(slsDetectorSoftware)
endif ()

if (SLS_USE_RECEIVER)
    add_subdirectory(slsReceiverSoftware) 
endif (SLS_USE_RECEIVER)

if (SLS_USE_GUI)   
    add_subdirectory(libs/qwt)  
    add_subdirectory(slsDetectorGui)
endif (SLS_USE_GUI)

if (SLS_USE_SIMULATOR)
    add_subdirectory(slsDetectorServers)
endif (SLS_USE_SIMULATOR)

if (SLS_USE_INTEGRATION_TESTS)
    add_subdirectory(integrationTests)
endif (SLS_USE_INTEGRATION_TESTS)

if (SLS_USE_PYTHON)
    find_package (Python 3.8 COMPONENTS Interpreter Development)
    if(SLS_FETCH_PYBIND11_FROM_GITHUB)
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11
            GIT_TAG        v2.13.6
        )

    else()
    # https://github.com/pybind/pybind11/releases
        FetchContent_Declare(
            pybind11
            URL ${CMAKE_CURRENT_SOURCE_DIR}/libs/pybind11/v2.13.6.tar.gz
            URL_HASH MD5=a04dead9c83edae6d84e2e343da7feeb
        )
    endif()
    FetchContent_MakeAvailable(pybind11)

    add_subdirectory(python)
endif(SLS_USE_PYTHON)

if (SLS_USE_CTBGUI)
    add_subdirectory(pyctbgui)
endif(SLS_USE_CTBGUI)

# Workaround for file note being copied to build directory
# when issuing a python -m build
# TODO! Proper fix
if(EXISTS ".clang-tidy")
    configure_file(.clang-tidy  
        ${CMAKE_BINARY_DIR}/.clang-tidy  
    )
endif()


if (SLS_BUILD_EXAMPLES)
    add_subdirectory(sample)
endif(SLS_BUILD_EXAMPLES)

if(SLS_BUILD_DOCS)
    add_subdirectory(docs)
endif(SLS_BUILD_DOCS)

if(SLS_USE_MOENCH)
    add_subdirectory(slsDetectorCalibration/tiffio)
    add_subdirectory(slsDetectorCalibration/moenchExecutables)
endif(SLS_USE_MOENCH)

if(SLS_USE_JUNGFRAU)
    add_subdirectory(slsDetectorCalibration/tiffio)
    add_subdirectory(slsDetectorCalibration/jungfrauExecutables)
endif(SLS_USE_JUNGFRAU)

if(SLS_MASTER_PROJECT)
    set(CMAKE_INSTALL_DIR "share/cmake/${PROJECT_NAME}")
    set(PROJECT_LIBRARIES slsSupportShared slsDetectorShared slsReceiverShared)
    include(cmake/package_config.cmake)
endif()