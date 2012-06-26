DESTDIR  			= 		bin
MOC_DIR   			= 		mocs
OBJECTS_DIR 		= 		objs
UI_HEADERS_DIR		= 		forms/include

RESOURCES   		+=  	icons.qrc


DEFINES 			+= 		VERBOSE
							

target.path 		+= 		$(DESTDIR)
documentation.path 	= 		/$(DOCPATH)
documentation.files = 		docs/*
INSTALLS			+= 		target
INSTALLS 			+= 		documentation
QMAKE_CLEAN 		+= 		docs/*/* \
							$(DESTDIR)* \
							forms/include/*
							

LIBS				+=		-Wl,-Bstatic -L../slsDetectorSoftware -lSlsDetector  -Wl,-Bdynamic\
							-L/usr/local/qwt-5.2.3-svn/lib  -lqwt

DEPENDPATH  		+=		\
							slsDetectorPlotting/include\
							include\
							forms/include


INCLUDEPATH 		+= 		\
							/usr/local/qwt-5.2.3-svn/include\
							slsDetectorPlotting/include\
							include\
							forms/include\
							../slsDetectorSoftware/commonFiles\
							../slsDetectorSoftware/MySocketTCP\
							../slsDetectorSoftware/slsDetector\
							../slsDetectorSoftware/slsDetectorAnalysis\
							../slsDetectorSoftware/multiSlsDetector\
							../slsDetectorSoftware/usersFunctions  

SOURCES 			= 		\
							slsDetectorPlotting/src/SlsQt1DPlot.cxx\
							slsDetectorPlotting/src/SlsQt1DZoomer.cxx\
							slsDetectorPlotting/src/SlsQt2DHist.cxx\
							slsDetectorPlotting/src/SlsQt2DPlot.cxx\
							slsDetectorPlotting/src/SlsQt2DPlotLayout.cxx\
							slsDetectorPlotting/src/SlsQtNumberEntry.cxx\
							src/qDetectorMain.cpp\
							src/qDrawPlot.cpp\
							src/qCloneWidget.cpp\	
							src/qTabMeasurement.cpp\
							src/qTabDataOutput.cpp\
							src/qTabPlot.cpp\
							src/qTabActions.cpp\
							src/qActionsWidget.cpp\
							src/qTabAdvanced.cpp\
							src/qTabSettings.cpp\
							src/qTabDebugging.cpp\
							src/qTabDeveloper.cpp

HEADERS 			=  		\
							slsDetectorPlotting/include/SlsQt1DPlot.h\
							slsDetectorPlotting/include/SlsQt1DZoomer.h\
							slsDetectorPlotting/include/SlsQt2DHist.h\
							slsDetectorPlotting/include/SlsQt2DPlot.h\
							slsDetectorPlotting/include/SlsQt2DPlotLayout.h\
							slsDetectorPlotting/include/SlsQt2DZoomer.h\
							slsDetectorPlotting/include/SlsQtValidators.h\
							slsDetectorPlotting/include/SlsQtNumberEntry.h\
							include/qDefs.h\
							include/qDetectorMain.h\
							include/qDrawPlot.h\
							include/qCloneWidget.h\
							include/qTabMeasurement.h\
							include/qTabDataOutput.h\
							include/qTabPlot.h\
							include/qTabActions.h\
							include/qActionsWidget.h\
							include/qTabAdvanced.h\
							include/qTabSettings.h\
							include/qTabDebugging.h\
							include/qTabDeveloper.h\
							../slsDetectorSoftware/commonFiles/sls_detector_defs.h


FORMS = 					\
							forms/form_detectormain.ui\
							forms/form_tab_measurement.ui\
							forms/form_tab_dataoutput.ui\
							forms/form_tab_plot.ui\
#							forms/form_tab_actions.ui\
							forms/form_tab_advanced.ui\
							forms/form_tab_settings.ui\
							forms/form_tab_debugging.ui\
							forms/form_tab_developer.ui
