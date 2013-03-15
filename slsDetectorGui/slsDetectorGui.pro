
QT_INSTALL_PREFIX=$(QTDIR)
QMAKE_UIC = $(QTDIR)/bin/uic
QMAKE_MOC =  $(QTDIR)/bin/moc
QMAKE_RCC =  $(QTDIR)/bin/rcc
QMAKE_INCDIR_QT = $(QTDIR)/include/

QMAKE_LIBS_QT = 	-L$(QTDIR)/lib 
QMAKE_LIBS = 	-L$(QTDIR)/lib 



DESTDIR  			?= 		bin
MOC_DIR   			= 		mocs
OBJECTS_DIR 		= 		objs
UI_HEADERS_DIR		= 		forms/include
SLSDETLIB ?=../slsDetectorSoftware               
RESOURCES   		+=  	icons.qrc

CONFIG				+=		debug no_include_pwd

QMAKE_CXXFLAGS_WARN_ON = 	-w 
QMAKE_CFLAGS_WARN_ON   = 	-w

DEFINES 			+= 		VERBOSE DACS_INT PRINT_LOG THIS_PATH=\\\"$$PWD\\\" #VERYVERBOSE 
							

target.path 		+= 		$(DESTDIR)
documentation.path 	= 		/$(DOCPATH)
documentation.files = 		docs/*
INSTALLS			+= 		target
INSTALLS 			+= 		documentation
QMAKE_CLEAN 		+= 		docs/*/* 
							

#LIBS				+=		-Wl,-Bstatic -L../slsDetectorSoftware -lSlsDetector  -Wl,-Bdynamic\
#							-L$(QWTDIR)/lib  -lqwt -L$(QWT3D)/lib 
 
LIBS				=	-L$(QWTDIR)/lib	  -lqwt -L$(QWT3D)/lib  

DEPENDPATH  		+=		\
							slsDetectorPlotting/include\
							include\
							forms/include

INCLUDEPATH 		= 	\	 
							$(QWTDIR)/include\
							$(QWTDIR) \
                              $(QWTDIR)/src\
                              $(QWT3D)/include\
							slsDetectorPlotting/include\
							include\
							forms/include\
                                                        $(INCLUDES)

#                                                        $(shell echo "/lib/modules/`uname -r`/build/include") \
#							../slsDetectorSoftware/commonFiles\
#							../slsDetectorSoftware/MySocketTCP\
#							../slsDetectorSoftware/slsReceiverInterface\
#							../slsDetectorSoftware/slsDetector\
#							../slsDetectorSoftware/slsDetectorAnalysis\
#							../slsDetectorSoftware/multiSlsDetector\
#							../slsDetectorSoftware/usersFunctions  

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
							src/qScanWidget.cpp\
							src/qTabAdvanced.cpp\
							src/qTabSettings.cpp\
							src/qTabDebugging.cpp\
							src/qTabDeveloper.cpp\
							src/qTabMessages.cpp\
							src/qServer.cpp

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
							include/qDebugStream.h\
							include/qDetectorMain.h\
							include/qDrawPlot.h\
							include/qCloneWidget.h\
							include/qTabMeasurement.h\
							include/qTabDataOutput.h\
							include/qTabPlot.h\
							include/qTabActions.h\
							include/qActionsWidget.h\
							include/qScanWidget.h\
							include/qTabAdvanced.h\
							include/qTabSettings.h\
							include/qTabDebugging.h\
							include/qTabDeveloper.h\
							include/qTabMessages.h\
							include/svnInfoGui.h\
							../slsDetectorSoftware/commonFiles/sls_detector_defs.h\
							include/qServer.h


FORMS = 					\
							forms/form_detectormain.ui\
							forms/form_tab_measurement.ui\
							forms/form_tab_dataoutput.ui\
							forms/form_tab_plot.ui\
#							forms/form_tab_actions.ui\
							forms/form_tab_advanced.ui\
							forms/form_tab_settings.ui\
							forms/form_tab_debugging.ui\
#							forms/form_tab_developer.ui\
#							forms/form_tab_messages.ui
							forms/form_action.ui\
							forms/form_scan.ui
