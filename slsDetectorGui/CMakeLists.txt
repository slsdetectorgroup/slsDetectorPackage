set(CMAKE_AUTOMOC ON)

find_package(Qt4 REQUIRED)
find_package(Qwt 6.1 REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

set(SOURCES
    slsDetectorPlotting/src/SlsQt1DPlot.cpp
    slsDetectorPlotting/src/SlsQt1DZoomer.cpp
    slsDetectorPlotting/src/SlsQt2DHist.cpp
    slsDetectorPlotting/src/SlsQt2DPlot.cpp
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

qt4_wrap_ui(FORMS_H ${FORMS})

set(HEADERS
    slsDetectorPlotting/include/SlsQt1DPlot.h
    slsDetectorPlotting/include/SlsQt1DZoomer.h
    slsDetectorPlotting/include/SlsQt2DHist.h
    slsDetectorPlotting/include/SlsQt2DPlot.h
    slsDetectorPlotting/include/SlsQt2DZoomer.h
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
qt4_add_resources(RESOURCES_SRCS ${RESOURCES})


add_definitions(
    -DPRINT_LOG #-DVERBOSE 
)

add_executable(slsDetectorGui
    ${SOURCES}
    ${HEADERS}
    ${FORMS_H}
    ${RESOURCES_SRCS}
)

target_include_directories(slsDetectorGui PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/slsDetectorPlotting/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<BUILD_INTERFACE:${QT_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${QT_QTCORE_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${QT_QTGUI_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${QWT_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

target_link_libraries(slsDetectorGui PUBLIC
    slsProjectOptions 
    slsDetectorStatic
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QWT_LIBRARIES}
    PNG::PNG
    ZLIB::ZLIB
    Qt4::QtOpenGL
    Qt4::QtSvg
    expat
    
    PRIVATE
    slsProjectWarnings
)

set_target_properties(slsDetectorGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
if(SLS_LTO_AVAILABLE)
    set_property(TARGET slsDetectorGui PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

install(TARGETS slsDetectorGui
    # EXPORT "${TARGETS_EXPORT_NAME}" #do not export gui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

