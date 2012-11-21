# do not change below this line


WD=$(shell pwd)
LIBRARYDIR=$(WD)/slsDetectorSoftware
CLIENTDIR=$(WD)/slsDetectorClient
GUIDIR=$(WD)/slsDetectorGuiOriginal
RECEIVERDIR=$(LIBRARYDIR)/slsReceiver
CALWIZDIR=$(WD)/calibrationWizards
MANDIR=$(WD)/manual

INSTALLROOT?=$(PWD)
BINDIR?=$(INSTALLROOT)/bin
DOCDIR?=$(INSTALLROOT)/docs
LIBDIR?=$(INSTALLROOT)/bin
INCDIR?=$(INSTALLROOT)/include


LDFLAG='-L$(LIBDIR) -lSlsDetector'

#FLAGS=-DVERBOSE
ASM=$(shell echo "/lib/modules/`uname -r`/build/include")



INCLUDES='-I. -I$(LIBRARYDIR)/commonFiles -I$(LIBRARYDIR)/slsDetector -I$(LIBRARYDIR)/MySocketTCP -I$(LIBRARYDIR)/usersFunctions -I$(LIBRARYDIR)/multiSlsDetector -I$(LIBRARYDIR)/slsDetectorUtils -I$(LIBRARYDIR)/slsDetectorCommand -I$(LIBRARYDIR)/slsDetectorAnalysis -I$(LIBRARYDIR)/slsReceiverInterface -I$(ASM)'



all: lib  slsDetectorClient slsReceiver gui 

nonstatic: lib  slsDetectorClient slsReceiver slsDetectorGUI

lib:
	echo "compile lib"
	cd $(LIBRARYDIR) && $(MAKE) FLAGS=$(FLAGS) DESTDIR=$(LIBDIR) INCLUDES='-I$(LIBRARYDIR)/commonFiles -I$(LIBRARYDIR)/slsDetector -I$(LIBRARYDIR)/MySocketTCP -I$(LIBRARYDIR)/usersFunctions -I$(LIBRARYDIR)/multiSlsDetector -I$(LIBRARYDIR)/slsDetectorUtils -I$(LIBRARYDIR)/slsDetectorCommand -I$(LIBRARYDIR)/slsDetectorAnalysis -I$(LIBRARYDIR)/slsReceiverInterface -I$(ASM)'

slsDetectorClient_static: lib
	cd  $(CLIENTDIR) && $(MAKE)  FLAGS=$(FLAGS) LDFLAG=$(LDFLAG) DESTDIR=$(DESTDIR) LIBDIR=$(LIBDIR) INCLUDES=$(INCLUDES)


slsDetectorClient: lib
	echo "compile client"
	cd  $(CLIENTDIR) && $(MAKE) FLAGS=$(FLAGS) DESTDIR=$(BINDIR)  LIBDIR=$(LIBDIR) LIBS=$(LDFLAG) INCLUDES=$(INCLUDES)

slsReceiver: lib
	echo "compile receiver"
	cd  $(RECEIVERDIR) && $(MAKE)  FLAGS=$(FLAGS) DESTDIR=$(BINDIR) LIBDIR=$(LIBDIR)  LIBS=$(LDFLAG) INCLUDES=$(INCLUDES)


slsDetectorGUI: lib
	echo $(LDFLAG)
	cd  $(GUIDIR) && $(MAKE)  FLAGS=$(FLAGS) LDFLAG=$(LDFLAG) DESTDIR=$(BINDIR) LIBDIR=$(LIBDIR) INCLUDES=$(INCLUDES)

calWiz: 
	cd  $(CALWIZDIR) && $(MAKE)  FLAGS=$(FLAGS)  LDFLAG=$(LDFLAG) DESTDIR=$(BINDIR) INCLUDES=$(INCLUDES)



gui: slsDetectorGUI







doc:
	$(shell test -d $(DOCDIR) || mkdir -p $(DOCDIR))
	$(shell test -d $(DOCDIR)/pdf || mkdir -p $(DOCDIR)/pdf)
	cd $(LIBRARYDIR) && make doc DOCDIR=$(DOCDIR)
	cd $(CLIENTDIR) && make doc DOCDIR=$(DOCDIR)
	cd $(GUIDIR)  && make doc DOCDIR=$(DOCDIR)
	cd $(CALWIZDIR) && make doc DESTDIR=$(DOCDIR)
	cd $(MANDIR) && make DESTDIR=$(DOCDIR)

htmldoc:
	make doc
	$(shell test -d $(DOCDIR) || mkdir -p $(DOCDIR))
	$(shell test -d $(DOCDIR)/html || mkdir -p $(DOCDIR)/html)
	cd $(LIBRARYDIR) && make htmldoc DOCDIR=$(DOCDIR)
	cd $(CLIENTDIR) && make htmldoc DOCDIR=$(DOCDIR)
	cd $(GUIDIR)  && make htmldoc DOCDIR=$(DOCDIR)
	cd $(CALWIZDIR) && make htmldoc DESTDIR=$(DOCDIR)
	cd $(MANDIR) && make html DESTDIR=$(DOCDIR)

clean:
	cd $(BINDIR) && rm -rf sls_detector_* slsDetectorGui slsReceiver angularCalibrationWizard energyCalibrationWizard 
	cd $(LIBDIR) && rm -rf libSlsDetector.so libSlsDetector.a
	cd $(LIBRARYDIR) && $(MAKE) clean
	cd $(CLIENTDIR) && $(MAKE) clean
	cd $(GUIDIR) && $(MAKE) clean
	cd $(RECEIVERDIR) && $(MAKE) clean	
	cd  $(CALWIZDIR) && $(MAKE) clean	
	cd $(MANDIR) && $(MAKE) clean
	cd $(DOCDIR) && rm -rf * 

install_lib:
	cd $(LIBRARYDIR) && $(MAKE) install DESTDIR=$(LIBDIR)
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR)



install_client:
	cd $(CLIENTDIR) && $(MAKE) install DESTDIR=$(BINDIR)



install:
	set -e; \
	. ./configure; \
	make conf;\
	make install_lib;\
	make install_client ; \
	make install_gui; \
	make install_calwiz; \
	make install_doc; \
	make install_htmldoc; \


conf:
	@echo "INSTALLROOT is $(INSTALLROOT)"
	@echo "BINDIR is $(BINDIR)"
	@echo "LIBDIR is $(LIBDIR)"
	@echo "INCDIR is $(INCDIR)"
	@echo "DOCDIR is $(DOCDIR)"

tar:
	make clean
	cd .. && tar czf newMythenSoftware.tgz newMythenSoftware

help:
	@echo "Targets:"
	@echo "make all 		compile library, and text client"
	@echo "make lib 		compile library"
	@echo "make slsDetectorClient 	compile slsDetectorClient"
	@echo "make nonstatic		compile the slsDetectorClient dynamically linking the libraries"
	@echo "make slsDetectorGUI 	compile slsDetectorGUI - requires a working Qt4 and Qwt installation"
	@echo "make calWiz 		compile the calibration wizards - requires a working root installation"
	@echo "make doc			compile pdf documentation"
	@echo "make htmldoc			compile html documentation"
	@echo ""
	@echo "conf 			list the install variables"
	@echo "make install_client     	install slsDetectorClient"
	@echo "make install_lib       	install detector library and include files"
	@echo "make install            	install library, include files, slsDetectorClient"
	@echo "make install_libdoc    	install library documentaion"
	@echo "make install_clientdoc  	install mythenClient documentation"
	@echo "make install_doc        	install all documentation"
	@echo "make clean              	remove object files and executables"
	@echo "make help               	lists possible targets"
	@echo ""
	@echo "Variables -  to change them run <source configure> :"
	@echo "INSTALLROOT=<yourdir>:    installation root dir, default /usr/local"
	@echo "BINDIR=<yourbin>:          binary installation dir below INSTALLROOT, default bin"
	@echo "LIBDIR=<yourlib>:          library installation dir below INSTALLROOT, default lib"
	@echo "INCDIR=<yourincludes>:     header installation dir below INSTALLROOT, default include/slsdetector"
	@echo "DOCDIR=<yourdoc>:          documentation installation dir below INSTALLROOT, default share/doc"
