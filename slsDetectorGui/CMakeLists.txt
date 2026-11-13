# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


# Concurrent is needed for the acquisition thread in qDrawPlot.cpp
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Gui Core Concurrent)

find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)


# The plotting widgets are a static library so that the gui, the tests and
# the demo share them.
set(PLOTTING_SOURCES
    slsDetectorPlotting/src/SlsQtPlotCommon.cpp
    slsDetectorPlotting/src/SlsQtFastImage.cpp
    slsDetectorPlotting/src/SlsQtScalePanel.cpp
    slsDetectorPlotting/src/SlsQt1DPlot.cpp
    slsDetectorPlotting/src/SlsQt2DHist.cpp
    slsDetectorPlotting/src/SlsQt2DPlot.cpp
)
set(PLOTTING_HEADERS
    slsDetectorPlotting/include/SlsQtPlotCommon.h
    slsDetectorPlotting/include/SlsQtFastImage.h
    slsDetectorPlotting/include/SlsQtScalePanel.h
    slsDetectorPlotting/include/SlsQt1DPlot.h
    slsDetectorPlotting/include/SlsQt2DHist.h
    slsDetectorPlotting/include/SlsQt2DPlot.h
)

add_library(slsDetectorPlotting STATIC
    ${PLOTTING_SOURCES}
    ${PLOTTING_HEADERS}
)
target_include_directories(slsDetectorPlotting PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/slsDetectorPlotting/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(slsDetectorPlotting
    PUBLIC
        slsProjectOptions
        slsSupportStatic
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
    PRIVATE
        slsProjectWarnings
)


set(SOURCES
    src/qDetectorMain.cpp
    src/qDrawPlot.cpp
    src/qCloneWidget.cpp
    src/qTabMeasurement.cpp
    src/qTabDataOutput.cpp
    src/qTabPlot.cpp
    src/qTabAdvanced.cpp
    src/qTabSettings.cpp
    src/qTabDebugging.cpp
    src/qTabDeveloper.cpp
    src/qTabMessages.cpp
    src/qDacWidget.cpp
)

set(FORMS
    forms/form_detectormain.ui
    forms/form_tab_measurement.ui
    forms/form_tab_dataoutput.ui
    forms/form_tab_plot.ui
    forms/form_tab_advanced.ui
    forms/form_tab_settings.ui
    forms/form_tab_debugging.ui
    forms/form_tab_messages.ui 
    forms/form_tab_developer.ui
    forms/form_dac.ui
    forms/form_plot.ui
    forms/form_cloneplot.ui
)

qt_wrap_ui(FORMS_H ${FORMS})

set(HEADERS
    include/qDefs.h
    include/qDetectorMain.h
    include/qDrawPlot.h
    include/qCloneWidget.h
    include/qTabMeasurement.h
    include/qTabDataOutput.h
    include/qTabPlot.h
    include/qTabAdvanced.h
    include/qTabSettings.h
    include/qTabDebugging.h
    include/qTabDeveloper.h
    include/qTabMessages.h
    include/qDacWidget.h
)
set(RESOURCES
    include/icons.qrc
)
qt_add_resources(RESOURCES_SRCS ${RESOURCES})


add_executable(slsDetectorGui
    ${SOURCES}
    ${HEADERS}
    ${FORMS_H}
    ${RESOURCES_SRCS}
)

target_compile_definitions(slsDetectorGui PRIVATE PRINT_LOG)

target_include_directories(slsDetectorGui PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

target_link_libraries(slsDetectorGui PUBLIC
    slsProjectOptions 
    slsDetectorStatic
    slsDetectorPlotting
    PNG::PNG
    ZLIB::ZLIB
    Qt6::Concurrent

    PRIVATE
    slsProjectWarnings
)

set_target_properties(slsDetectorGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    #Instruct CMake to run moc automatically when needed
    AUTOMOC ON
)
if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET slsDetectorGui PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

if(SLS_USE_TESTS)
    add_subdirectory(tests)
    add_subdirectory(demo)
endif()

sls_install_executable(slsDetectorGui COMPONENT gui)
