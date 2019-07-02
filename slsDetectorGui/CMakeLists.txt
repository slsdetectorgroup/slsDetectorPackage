set(CMAKE_AUTOMOC ON)

set(SOURCES
    slsDetectorPlotting/src/SlsQt1DPlot.cxx
    slsDetectorPlotting/src/SlsQt1DZoomer.cxx
    slsDetectorPlotting/src/SlsQt2DHist.cxx
    slsDetectorPlotting/src/SlsQt2DPlot.cxx
    slsDetectorPlotting/src/SlsQt2DPlotLayout.cxx
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
)

set(FORMS
    forms/form_detectormain.ui
    forms/form_tab_measurement.ui
    forms/form_tab_dataoutput.ui
    forms/form_tab_plot.ui
    forms/form_tab_advanced.ui
    forms/form_tab_settings.ui
    forms/form_tab_debugging.ui
)

qt4_wrap_ui(FORMS_H ${FORMS})

set(HEADERS
    slsDetectorPlotting/include/SlsQt1DPlot.h
    slsDetectorPlotting/include/SlsQt1DZoomer.h
    slsDetectorPlotting/include/SlsQt2DHist.h
    slsDetectorPlotting/include/SlsQt2DPlot.h
    slsDetectorPlotting/include/SlsQt2DPlotLayout.h
    slsDetectorPlotting/include/SlsQt2DZoomer.h
    include/qDefs.h
    include/qDebugStream.h
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
    ../slsDetectorSoftware/include
    ../slsSupportLib/include/versionAPI.h
    ../slsSupportLib/include/ServerSocket.h
    ../slsSupportLib/include/ServerInterface2.h
)
set(RESOURCES
    include/icons.qrc
)
qt4_add_resources(RESOURCES_SRCS ${RESOURCES})

include_directories(
    include
    slsDetectorPlotting/include
    ../slsSupportLib/include  
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QWT_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(
    -DPRINT_LOG #-DVERBOSE 
)

add_executable(slsDetectorGui
    ${SOURCES}
    ${HEADERS}
    ${FORMS_H}
    ${RESOURCES_SRCS}
)

target_link_libraries(slsDetectorGui PUBLIC
    slsProjectOptions
    slsProjectWarnings
    slsSupportLib
    slsDetectorShared
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QWT_LIBRARIES}
    ${ZeroMQ_LIBRARIES}
    rt
    png
    z
    Qt4::QtOpenGL
    Qt4::QtSvg
)

set_target_properties(slsDetectorGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS slsDetectorGui DESTINATION bin)
