# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2026 Contributors to the SLS Detector Package
#
# Standalone Qt Widgets demo / benchmark for the Qt6-only plotting stack.
# Wired in from slsDetectorGui/CMakeLists.txt by an EXISTS guard; links the
# shared slsDetectorPlotting static library built by the GUI CMake file.
#
# Build: cmake --build <build> --target plot_demo
# Run:   QT_QPA_PLATFORM=offscreen <build>/bin/plot_demo --frames 120

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Gui Core)

if(NOT TARGET slsDetectorPlotting)
    message(FATAL_ERROR
        "plot_demo needs the slsDetectorPlotting target defined in "
        "slsDetectorGui/CMakeLists.txt")
endif()

add_executable(plot_demo
    plot_demo.cpp
)
target_link_libraries(plot_demo PRIVATE slsDetectorPlotting)
set_target_properties(plot_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
