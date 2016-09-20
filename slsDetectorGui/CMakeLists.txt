
set(CMAKE_AUTOMOC ON)

set(SOURCES
    slsDetectorPlotting/src/SlsQt1DPlot.cxx
    slsDetectorPlotting/src/SlsQt1DZoomer.cxx
    slsDetectorPlotting/src/SlsQt2DHist.cxx
    slsDetectorPlotting/src/SlsQt2DPlot.cxx
    slsDetectorPlotting/src/SlsQt2DPlotLayout.cxx
    slsDetectorPlotting/src/SlsQtNumberEntry.cxx
    src/qDetectorMain.cpp
    src/qDrawPlot.cpp
    src/qCloneWidget.cpp
    src/qTabMeasurement.cpp
    src/qTabDataOutput.cpp
    src/qTabPlot.cpp
    src/qTabActions.cpp
    src/qActionsWidget.cpp
    src/qScanWidget.cpp
    src/qTabAdvanced.cpp
    src/qTabSettings.cpp
    src/qTabDebugging.cpp
    src/qTabDeveloper.cpp
    src/qTabMessages.cpp
    src/qServer.cpp
)

set(FORMS
    forms/form_detectormain.ui
    forms/form_tab_measurement.ui
    forms/form_tab_dataoutput.ui
    forms/form_tab_plot.ui
    forms/form_tab_advanced.ui
    forms/form_tab_settings.ui
    forms/form_tab_debugging.ui
    forms/form_action.ui
    forms/form_scan.ui
)

qt4_wrap_ui(FORMS_H ${FORMS})

set(HEADERS
    slsDetectorPlotting/include/SlsQt1DPlot.h
    slsDetectorPlotting/include/SlsQt1DZoomer.h
    slsDetectorPlotting/include/SlsQt2DHist.h
    slsDetectorPlotting/include/SlsQt2DPlot.h
    slsDetectorPlotting/include/SlsQt2DPlotLayout.h
    slsDetectorPlotting/include/SlsQt2DZoomer.h
    slsDetectorPlotting/include/SlsQtValidators.h
    slsDetectorPlotting/include/SlsQtNumberEntry.h
    include/qDefs.h
    include/qDebugStream.h
    include/qDetectorMain.h
    include/qDrawPlot.h
    include/qCloneWidget.h
    include/qTabMeasurement.h
    include/qTabDataOutput.h
    include/qTabPlot.h
    include/qTabActions.h
    include/qActionsWidget.h
    include/qScanWidget.h
    include/qTabAdvanced.h
    include/qTabSettings.h
    include/qTabDebugging.h
    include/qTabDeveloper.h
    include/qTabMessages.h
    include/gitInfoGui.h
    include/qServer.h
)
set(RESOURCES
    include/icons.qrc
)
qt4_add_resources(RESOURCES_SRCS ${RESOURCES})

include_directories(
    include
    slsDetectorPlotting/include
    ../slsDetectorSoftware/commonFiles 
    ../slsDetectorSoftware/slsReceiverInterface   
    ../slsDetectorSoftware/slsDetector  
    ../slsDetectorSoftware/slsDetectorAnalysis  
    ../slsDetectorSoftware/multiSlsDetector   
    ../slsDetectorSoftware/usersFunctions  
    ../slsReceiverSoftware/include
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QWT_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(
    -DDACS_INT -DVERBOSE -DPRINT_LOG
)

add_executable(slsDetectorGui
    ${SOURCES}
    ${HEADERS}
    ${FORMS_H}
    ${RESOURCES_SRCS}
)

set_target_properties(slsDetectorGui PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_library(zmq STATIC IMPORTED )

set_target_properties(zmq PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../slsReceiverSoftware/include/libzmq.a
)

target_link_libraries(slsDetectorGui 
    slsDetectorShared
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QWT_LIBRARIES}
    pthread
    zmq
    rt
)

add_executable(gui_client
    client/qClient.h
    client/qClient.cpp
)
set_target_properties(gui_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
