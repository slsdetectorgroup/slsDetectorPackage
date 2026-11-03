# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#
# jf_online_display_gui -- single multithreaded program merging
#   jf_event_finder_cli / jf_event_finder_gui  (ZMQ gate)
#   onlinedisp_zmq                             (Qt5 + ROOT display)
#
# Standalone build (this directory):
#   cmake -S . -B build && cmake --build build -j
#   ./build/jf_online_display_gui                   (normal)
#   ./build/jf_online_display_gui_JFSTRX            (strip-xel)
#
# The Makefile in this directory builds the same two programs; this file exists
# so the program can be built like the rest of the package.
#
# Targets
#   jf_online_display_gui         normal geometry
#   jf_online_display_gui_JFSTRX  same sources with -DJFSTRX (strip-xel)
#   gate_contig_test              gate unit test (JF_BUILD_TESTS=ON, default)
#   jf_feeder                     ZMQ test feeder (JF_BUILD_TESTS=ON, default)
#
# The two variants must never share object files: the geometry and the struct
# layouts differ, so they are compiled twice (JFSTRX/common object libraries).

cmake_minimum_required(VERSION 3.16)
project(jf_online_display_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

option(JF_BUILD_TESTS "build the gate unit test and the ZMQ feeder" ON)

# ---------------------------------------------------------------------------
# paths relative to this directory (same layout as the Makefile)
#   PKG = ../../..   package root
#   CAL = ../..      slsDetectorCalibration
# ---------------------------------------------------------------------------
get_filename_component(PKG "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
get_filename_component(CAL "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# ---------------------------------------------------------------------------
# dependencies
# ---------------------------------------------------------------------------
find_package(Qt5 5.9 COMPONENTS Widgets Gui Core REQUIRED)
# online_jf.h includes Qt headers, and every translation unit of this program
# sees them (the core/root/h5 objects too), so the include paths are added to the
# flag list rather than left to the link interface.
set(JF_QT_INCLUDES
    ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS})
find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Graf Gpad Thread)

# HDF5: CMake's own module finds both C and C++ (the C++ headers are not in the
# default include path on every distro).
find_package(HDF5 REQUIRED COMPONENTS C CXX)

# libzmq: prefer the prebuilt static library from the package build tree (that is
# what the Makefile links), then a system library as a fallback.
set(JF_ZMQ_STATIC "${PKG}/build/_deps/libzmq-build/lib/libzmq.a"
    CACHE FILEPATH "static libzmq to link")
if(EXISTS "${JF_ZMQ_STATIC}")
    set(JF_ZMQ_LIBRARY "${JF_ZMQ_STATIC}")
    set(JF_ZMQ_INCLUDE "${PKG}/build/_deps/libzmq-src/include")
    find_package(Threads REQUIRED) # the static build needs pthread/sodium
    set(JF_ZMQ_EXTRA Threads::Threads sodium)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZMQ REQUIRED IMPORTED_TARGET libzmq)
    set(JF_ZMQ_LIBRARY PkgConfig::ZMQ)
    set(JF_ZMQ_INCLUDE "")
    set(JF_ZMQ_EXTRA "")
endif()

# ---------------------------------------------------------------------------
# includes / flags shared by every source of this program
# ---------------------------------------------------------------------------
set(JF_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CAL}/dataStructures"
    "${CAL}/interpolations"
    "${CAL}/tiffio/include"
    "${PKG}/slsSupportLib/include"
    "${PKG}/slsReceiverSoftware/include"
    "${PKG}/libs/rapidjson"
    "${JF_ZMQ_INCLUDE}"
    ${JF_QT_INCLUDES}
    ${HDF5_INCLUDE_DIRS}
    ${ROOT_INCLUDE_DIRS}
)

# The strip-xel decoder header is included by the -DJFSTRX build only; listing it
# as a dependency makes both build systems rebuild when it changes.
set(JF_STRIXEL_HDR "${CAL}/dataStructures/jungfrauLGADStrixelsData_new.h")

set(JF_SOURCES
    online_jf_core.cpp
    online_jf_root.cpp
    online_jf_h5.cpp
)
set(JF_GUI_SOURCE online_jf_gui.cpp)

# Everything except the definition of JFSTRX: the two object libraries below
# differ only in that definition (and therefore in their compiled geometry).
set(JF_COMMON_WARN -Wall -O3 -pthread)

foreach(variant IN ITEMS "" "JFSTRX")
    if(variant STREQUAL "")
        set(objlib jf_common)
        set(app jf_online_display_gui)
    else()
        set(objlib jf_common_jfstrx)
        set(app jf_online_display_gui_JFSTRX)
    endif()

    add_library(${objlib} OBJECT ${JF_SOURCES})
    target_include_directories(${objlib} PRIVATE ${JF_INCLUDES})
    target_compile_options(${objlib} PRIVATE ${JF_COMMON_WARN})
    if(NOT variant STREQUAL "")
        target_compile_definitions(${objlib} PRIVATE ${variant})
    endif()
    set_source_files_properties(${JF_SOURCES} PROPERTIES
        OBJECT_DEPENDS "${JF_STRIXEL_HDR}")

    add_executable(${app} ${JF_GUI_SOURCE} $<TARGET_OBJECTS:${objlib}>)
    target_include_directories(${app} PRIVATE ${JF_INCLUDES})
    target_compile_options(${app} PRIVATE ${JF_COMMON_WARN})
    if(NOT variant STREQUAL "")
        target_compile_definitions(${app} PRIVATE ${variant})
    endif()
    target_link_libraries(${app} PRIVATE
        Qt5::Widgets Qt5::Gui Qt5::Core
        ${ROOT_LIBRARIES}
        ${JF_ZMQ_LIBRARY}
        ${JF_ZMQ_EXTRA}
        ${HDF5_LIBRARIES}
        pthread rt m
    )
endforeach()

# ---------------------------------------------------------------------------
# tests / helpers
# ---------------------------------------------------------------------------
if(JF_BUILD_TESTS)
    enable_testing()
    add_test(NAME gate_contig COMMAND gate_contig_test)
    # gate decision logic in isolation (no Qt, no ROOT usage at run time)
    add_executable(gate_contig_test
        test/gate_contig_test.cpp
        $<TARGET_OBJECTS:jf_common>)
    target_include_directories(gate_contig_test PRIVATE ${JF_INCLUDES})
    target_compile_options(gate_contig_test PRIVATE ${JF_COMMON_WARN})
    target_link_libraries(gate_contig_test PRIVATE
        Qt5::Widgets Qt5::Gui Qt5::Core
        ${ROOT_LIBRARIES}
        ${JF_ZMQ_LIBRARY}
        ${JF_ZMQ_EXTRA}
        ${HDF5_LIBRARIES}
        pthread rt m
    )

    # stand-in slsReceiver: single- or dual-interface ZMQ publisher
    add_executable(jf_feeder test/jf_feeder.cpp)
    target_include_directories(jf_feeder PRIVATE
        "${PKG}/libs/rapidjson" "${JF_ZMQ_INCLUDE}")
    target_compile_options(jf_feeder PRIVATE ${JF_COMMON_WARN})
    target_link_libraries(jf_feeder PRIVATE ${JF_ZMQ_LIBRARY} ${JF_ZMQ_EXTRA})
endif()
