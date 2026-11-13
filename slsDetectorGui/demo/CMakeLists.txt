# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2026 Contributors to the SLS Detector Package

# Shows the plotting widgets with synthetic frames, no detector needed.
# Also renders screenshots and benchmarks offscreen, see plot_demo --help

add_executable(plot_demo
    plot_demo.cpp
)
target_link_libraries(plot_demo PRIVATE slsDetectorPlotting)
set_target_properties(plot_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
