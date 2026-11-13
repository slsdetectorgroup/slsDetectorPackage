# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


# Qt 5 or 6 (Widgets/Gui/Core). Concurrent is needed for the acquisition
# thread in qDrawPlot.cpp. No Qwt.  SLS_GUI_QT_MAJOR selects the Qt major
# version; Qt 5 is needed on older distributions such as RHEL 7.
if(SLS_GUI_QT_MAJOR STREQUAL "6")
    find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Gui Core Concurrent)
    set(SLS_QT_LIBS Qt6::Widgets Qt6::Gui Qt6::Core)
    set(SLS_QT_CONCURRENT Qt6::Concurrent)
elseif(SLS_GUI_QT_MAJOR STREQUAL "5")
    find_package(Qt5 5.9 REQUIRED COMPONENTS Widgets Gui Core Concurrent)
    set(SLS_QT_LIBS Qt5::Widgets Qt5::Gui Qt5::Core)
    set(SLS_QT_CONCURRENT Qt5::Concurrent)
else()
    message(FATAL_ERROR "SLS_GUI_QT_MAJOR must be 5 or 6 (got ${SLS_GUI_QT_MAJOR})")
endif()

find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)


# The Qt-only plotting stack is built once as a static library and linked by
# the GUI executable as well as by the plotting demo and the QtTest suite.
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
set_target_properties(slsDetectorPlotting PROPERTIES
    # plotting widgets are Q_OBJECT classes; moc them in the library
    AUTOMOC ON
    AUTOUIC OFF
)
target_include_directories(slsDetectorPlotting PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/slsDetectorPlotting/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(slsDetectorPlotting
    PUBLIC
        slsProjectOptions
        # sls/ansi.h, sls/sls_detector_defs.h and the logger symbols
        slsSupportStatic
        ${SLS_QT_LIBS}
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
    src/qVersionResolve.cpp
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

if(SLS_GUI_QT_MAJOR STREQUAL "6")
    qt6_wrap_ui(FORMS_H ${FORMS})
else()
    qt5_wrap_ui(FORMS_H ${FORMS})
endif()

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
    include/qVersionResolve.h
)
set(RESOURCES
    include/icons.qrc
)
if(SLS_GUI_QT_MAJOR STREQUAL "6")
    qt6_add_resources(RESOURCES_SRCS ${RESOURCES})
else()
    qt5_add_resources(RESOURCES_SRCS ${RESOURCES})
endif()


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
    ${SLS_QT_LIBS}
    ${SLS_QT_CONCURRENT}
    
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

# Demo application and QtTest based tests for the plotting widgets.
# These are provided by the verification task; wire them in when present so
# they do not have to touch this file. Both link slsDetectorPlotting.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/demo/CMakeLists.txt")
    add_subdirectory(demo)
endif()

sls_install_executable(slsDetectorGui COMPONENT gui)
