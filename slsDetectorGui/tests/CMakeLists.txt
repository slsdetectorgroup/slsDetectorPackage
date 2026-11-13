# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2026 Contributors to the SLS Detector Package

# QtTest suite for the plotting widgets. Needs no display:
#   QT_QPA_PLATFORM=offscreen bin/plotting_test

find_package(Qt6 6.2 REQUIRED COMPONENTS Test)

add_executable(plotting_test
    plotting_test.cpp
)
target_link_libraries(plotting_test PRIVATE
    slsDetectorPlotting
    Qt6::Test
)
set_target_properties(plotting_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_test(NAME plotting_test COMMAND plotting_test)
set_tests_properties(plotting_test PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
