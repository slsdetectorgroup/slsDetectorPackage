# do not change below this line

INCS='-I$(LIBRARYDIR)/commonFiles -I$(LIBRARYDIR)/slsDetector -I$(LIBRARYDIR)/MySocketTCP -I$(LIBRARYDIR)/usersFunctions -I$(LIBRARYDIR)/multiSlsDetector -I$(LIBRARYDIR)/slsDetectorUtils -I$(LIBRARYDIR)/slsDetectorCommand -I$(LIBRARYDIR)/slsDetectorAnalysis -I$(LIBRARYDIR)/slsReceiverInterface -I$(ASM)'

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


LDFLAG:='-L$(LIBDIR) -lSlsDetector'

#FLAGS=-DVERBOSE
ASM=$(shell echo "/lib/modules/`uname -r`/build/include")



INCLUDES='-I. -I$(LIBRARYDIR)/commonFiles -I$(LIBRARYDIR)/slsDetector -I$(LIBRARYDIR)/MySocketTCP -I$(LIBRARYDIR)/usersFunctions -I$(LIBRARYDIR)/multiSlsDetector -I$(LIBRARYDIR)/slsDetectorUtils -I$(LIBRARYDIR)/slsDetectorCommand -I$(LIBRARYDIR)/slsDetectorAnalysis -I$(LIBRARYDIR)/slsReceiverInterface -I$(ASM)'



all: lib  textclient slsReceiver gui 

nonstatic: lib  slsDetectorClient slsReceiver slsDetectorGUI

lib:
	cd $(LIBRARYDIR) && $(MAKE) FLAGS=$(FLAGS) DESTDIR=$(LIBDIR) INCLUDES=$(INCLUDES)

stextclient: slsDetectorClient_static

slsDetectorClient_static: lib
	cd  $(CLIENTDIR) && $(MAKE) static_clients FLAGS=$(FLAGS) LDFLAG=$(LDFLAG) DESTDIR=$(BINDIR) LIBDIR=$(LIBDIR) INCLUDES=$(INCLUDES)


textclient:
	cd  $(CLIENTDIR) && $(MAKE) FLAGS=$(FLAGS) DESTDIR=$(BINDIR)  LIBDIR=$(LIBDIR) LIBS=$(LDFLAG) INCLUDES=$(INCLUDES)

slsReceiver: lib
	cd  $(RECEIVERDIR) && $(MAKE)  FLAGS=$(FLAGS) DESTDIR=$(BINDIR) LIBDIR=$(LIBDIR)  LIBS=$(LDFLAG) INCLUDES=$(INCLUDES)


receiver: slsReceiver

slsDetectorGUI: lib
	echo $(LDFLAG)
	cd  $(GUIDIR) && $(MAKE)  FLAGS=$(FLAGS) LDFLAG='-L$(LIBDIR) -lSlsDetector' DESTDIR=$(BINDIR) LIBDIR=$(LIBDIR) INCLUDES=$(INCLUDES)

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
#	cd $(MANDIR) && $(MAKE) clean
#	cd $(DOCDIR) && rm -rf * 

install_lib: 
	cd $(LIBRARYDIR) && $(MAKE) install DESTDIR=$(LIBDIR) INCLUDES=$(INCLUDES)
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR)



install_client: textclient slsReceiver

install_gui: gui

confinstall:
	make conf;\
	make install

install_lib: 
	make lib;\
	make textclient; \
	make slsReceiver; \
	make doc; \
	make htmldoc; \
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR);

install: 
	make install_lib; \
	make gui; \
	make calWiz; \
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR);


conf:
	set -e; \
	. ./configure; \
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
	@echo "make all 		compile library,  text clients, data reciever"
	@echo "make lib 		compile library"
	@echo "make client		compile the slsDetectorClient dynamically linking the libraries"
	@echo "make sclient 		compile slsDetectorClient statically linking the libraries"
	@echo "make receiver		compile the slsReciever dynamically linking the libraries"
	@echo "make gui			compile slsDetectorGUI - requires a working Qt4 and Qwt installation"
	@echo "make calWiz 		compile the calibration wizards - requires a working Root installation"
	@echo "make doc			compile pdf documentation"
	@echo "make htmldoc		compile html (and pdf) documentation"
	@echo "make install_lib         installs the libraries, the text clients, the documentation and the includes for the API"
	@echo "make install             installs all software, including the gui, the cal wizards and the includes for the API"
	@echo "make confinstall         installs all software, including the gui, the cal wizards and the includes for the API, prompting for the install paths"
	@echo "make clean              	remove object files and executables"
	@echo "make help               	lists possible targets"
	@echo "make tar                 makes a compressed tar of the software package"
	@echo ""
	@echo ""
	@echo "Variables -  to change them run <source configure> :"
	@echo "INSTALLROOT=<yourdir>:    installation root di	r, default $PWD"
	@echo "BINDIR=<yourbin>:         binary installation dir below INSTALLROOT, default bin"
	@echo "LIBDIR=<yourlib>:         library installation dir below INSTALLROOT, default lib"
	@echo "INCDIR=<yourincludes>:    header installation dir below INSTALLROOT, default include"
	@echo "DOCDIR=<yourdoc>:         documentation installation dir below INSTALLROOT, default doc"
