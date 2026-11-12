# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2026 Contributors to the SLS Detector Package
#
# QtTest suite for the Qt5/Qt6 plotting stack.
#
# Wired in from slsDetectorGui/CMakeLists.txt by an EXISTS guard; the plotting
# sources are compiled once into the slsDetectorPlotting static library there
# and shared by the GUI executable, this suite and the demo.
#
# Build:  cmake --build <build> --target plotting_test
# Run:    QT_QPA_PLATFORM=offscreen <build>/bin/plotting_test
#         (or "ctest -R plotting" when the project is configured with
#          -DSLS_USE_TESTS=ON)

if(SLS_GUI_QT_MAJOR STREQUAL "5")
    find_package(Qt5 5.9 REQUIRED COMPONENTS Test)
    set(SLS_QT_TEST Qt5::Test)
else()
    find_package(Qt6 6.2 REQUIRED COMPONENTS Test)
    set(SLS_QT_TEST Qt6::Test)
endif()

if(NOT TARGET slsDetectorPlotting)
    message(FATAL_ERROR
        "plotting_test needs the slsDetectorPlotting target defined in "
        "slsDetectorGui/CMakeLists.txt")
endif()

add_executable(plotting_test
    plotting_test.cpp
)
target_link_libraries(plotting_test PRIVATE
    slsDetectorPlotting
    ${SLS_QT_TEST}
)
set_target_properties(plotting_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(SLS_USE_TESTS)
    add_test(NAME plotting_test COMMAND plotting_test)
    set_tests_properties(plotting_test PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 300
    )
endif()
