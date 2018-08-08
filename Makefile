# do not change below this line#

# Include common definitions
include Makefile.include

INSTALLROOT	?=	$(PWD)
BINDIR	?=	$(INSTALLROOT)/bin
DOCDIR	?=	$(INSTALLROOT)/manual/docs
LIBDIR	?=	$(INSTALLROOT)/bin
INCDIR	?=	$(INSTALLROOT)/include
DETAILDOC	?=	$(INSTALLROOT)/docs
 
WD				=	$(shell pwd)
LIBRARYDIR		=	$(WD)/slsDetectorSoftware
LIBRARYRXRDIR 		= 	$(WD)/slsReceiverSoftware
CLIENTDIR		=	$(LIBRARYDIR)/slsDetectorClient
GUIDIR			=	$(WD)/slsDetectorGui
RECEIVERDIR		=	$(LIBRARYRXRDIR)
CALWIZDIR		=	$(WD)/calibrationWizards
MANDIR			=	$(WD)/manual
CALIBDIR		=	$(WD)/slsDetectorCalibration
MANAPIDIR		=	$(MANDIR)/manual-api

TABSPACE		:=	"\t"

 
INCLUDES=-I. -I$(LIBRARYDIR)/commonFiles -I$(LIBRARYDIR)/slsDetector -I$(LIBRARYDIR)/usersFunctions -I$(LIBRARYDIR)/multiSlsDetector -I$(LIBRARYDIR)/slsDetectorUtils -I$(LIBRARYDIR)/slsDetectorCommand -I$(LIBRARYDIR)/slsDetectorAnalysis -I$(LIBRARYDIR)/slsReceiverInterface  -I$(LIBRARYRXRDIR)/include -I$(LIBRARYDIR)/threadFiles -I$(LIBRARYDIR)/sharedMemory -I$(ASM)

INCLUDESRXR += -I. -I$(LIBRARYRXRDIR)/include -I$(CALIBDIR) -I$(ASM) 
#LIBFLAGRXR += 

$(info )
$(info #######################################)
$(info #   In slsDetectorsPackage Makefile   #)
$(info #######################################)
$(info )


.PHONY: all nonstatic static lib libreceiver textclient receiver gui stextclient sreceiver

#all: lib textclient  receiver gui 
all: textclient  receiver gui 

nonstatic: lib libreceiver textclient receiver  gui 

static: lib  libreceiver stextclient sreceiver gui 


lib:
	cd $(LIBRARYDIR) && $(MAKE) FLAGS='$(FLAGS)' DESTDIR='$(LIBDIR)' LIBRARYDIR='$(LIBRARYDIR)' LIBS='$(LDFLAGDET)' INCLUDES='$(INCLUDES)' LIBDIR='$(LIBDIR)'

libreceiver:
	cd $(LIBRARYRXRDIR) && $(MAKE) FLAGS='$(FLAGS)' DESTDIR='$(LIBDIR)' LIBS='$(LDFLAGRXR)' INCLUDES='$(INCLUDESRXR)' LIBDIR='$(LIBDIR)'


stextclient: slsDetectorClient_static

slsDetectorClient: textclient

slsDetectorClient_static: #lib
	cd  $(CLIENTDIR) && $(MAKE) static_clients FLAGS='$(FLAGS)' DESTDIR='$(BINDIR)' LIBRARYDIR='$(LIBRARYDIR)' LIBS='$(LDFLAGDET)' INCLUDES='$(INCLUDES)' LIBDIR='$(LIBDIR)'
	@echo ""
	@echo "#######################################"
	@echo "# Back in slsDetectorPackage Makefile #"
	@echo "#######################################"
	@echo ""
		
textclient: #lib
	cd  $(CLIENTDIR) && $(MAKE) FLAGS='$(FLAGS)' DESTDIR='$(BINDIR)'  LIBRARYDIR='$(LIBRARYDIR)' LIBS='$(LDFLAGDET)' INCLUDES='$(INCLUDES)' LIBDIR='$(LIBDIR)'
	@echo ""
	@echo "#######################################"
	@echo "# Back in slsDetectorPackage Makefile #"
	@echo "#######################################"
	@echo ""
	
slsReceiver: receiver

slsReceiver_static: receiver

receiver: #libreceiver
#	cd  $(RECEIVERDIR) && $(MAKE) receiver FLAGS='$(FLAGS)' DESTDIR='$(BINDIR)'  LIBS='$(LDFLAGRXR)' INCLUDES='$(INCLUDESRXR)' LIBDIR='$(LIBDIR)'
	cd  $(RECEIVERDIR) && $(MAKE) FLAGS='$(FLAGS)' DESTDIR='$(BINDIR)'  LIBS='$(LDFLAGRXR)' INCLUDES='$(INCLUDESRXR)' LIBDIR='$(LIBDIR)'
	cd  $(MANAPIDIR) && $(MAKE) slsMultiReceiver
	@echo ""
	@echo "#######################################"
	@echo "# Back in slsDetectorPackage Makefile #"
	@echo "#######################################"
	@echo ""
		
sreceiver: #libreceiver
	cd  $(RECEIVERDIR) && $(MAKE)  static_receiver FLAGS='$(FLAGS)' DESTDIR='$(BINDIR)'  LIBS='$(LDFLAGRXR)' INCLUDES='$(INCLUDESRXR)' LIBDIR='$(LIBDIR)'
	@echo ""
	@echo "#######################################"
	@echo "# Back in slsDetectorPackage Makefile #"
	@echo "#######################################"
	@echo ""

slsDetectorGUI: #lib
	cd  $(GUIDIR) && $(MAKE) DESTDIR='$(BINDIR)' LIBRARYDIR='$(LIBRARYDIR)' INCLUDES='$(INCLUDES)' LDFLAGDET='$(LDFLAGDETONLY)' LIBDIR='$(LIBDIR)'
	@echo ""
	@echo "#######################################"
	@echo "# Back in slsDetectorPackage Makefile #"
	@echo "#######################################"
	@echo ""
	
calWiz: 
	cd  $(CALWIZDIR) && $(MAKE)  DESTDIR=$(BINDIR) #FLAGS=$(FLAGS)  LDFLAGDET=$(LDFLAGDET) INCLUDES=$(INCLUDES)



gui: slsDetectorGUI


doc:
	$(shell test -d $(DOCDIR) || mkdir -p $(DOCDIR))
	cd manual && make all DESTDIR=$(DOCDIR)

htmldoc:
	make doc
	$(shell test -d $(DOCDIR) || mkdir -p $(DOCDIR))
	cd manual && make html DESTDIR=$(DOCDIR)
	
detaildoc: createdocs docspdf docshtml removedocs

createdocs: $(LIBRARYDIR)/doxy.config
	doxygen $(LIBRARYDIR)/doxy.config	

docspdf: 
	cd slsDetectorPackageDocs/latex && make 
	$(shell test -d $(DETAILDOC) || mkdir -p $(DETAILDOC))
	$(shell test -d $(DETAILDOC)/pdf || mkdir -p $(DETAILDOC)/pdf)
	mv slsDetectorPackageDocs/latex/refman.pdf $(DETAILDOC)/pdf/slsDetectorPackageDocs.pdf

docshtml: 
	$(shell test -d $(DETAILDOC) || mkdir -p $(DETAILDOC))
	$(shell test -d $(DETAILDOC)/html || mkdir -p $(DETAILDOC)/html)
	$(shell test -d $(DETAILDOC)/html/slsDetectorPackageDocs && rm -r $(DETAILDOC)/html/slsDetectorPackageDocs)
	mv slsDetectorPackageDocs/html $(DETAILDOC)/html/slsDetectorPackageDocs
	
removedocs:
	rm -rf 	slsDetectorPackageDocs;


clean:
	cd $(BINDIR) && rm -rf sls_detector_* slsDetectorGui slsReceiver angularCalibrationWizard energyCalibrationWizard 
	cd $(LIBDIR) && rm -rf libSlsDetector.so libSlsDetector.a libSlsReceiver.so libSlsReceiver.a 
	cd $(LIBRARYDIR) && $(MAKE) clean 
	cd $(LIBRARYRXRDIR) && $(MAKE) clean 
	cd $(CLIENTDIR) && $(MAKE) clean
	cd $(GUIDIR) && $(MAKE) clean
	cd $(CALWIZDIR) && $(MAKE) clean
	cd manual && $(MAKE) clean
	cd $(DOCDIR) && rm -rf * 
	rm -rf 	slsDetectorPackageDocs;
	rm -rf $(DETAILDOC)
	rm -rf $(MANAPIDIR)/slsMultiReceiver


#install_lib: 
#	cd $(LIBRARYDIR) && $(MAKE) install DESTDIR=$(LIBDIR) INCLUDES=$(INCLUDES)
#	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR)

mythen_virtual:
	cd $(LIBRARYDIR) && $(MAKE) mythenVirtualServer DESTDIR=$(BINDIR)


gotthard_virtual:
	cd $(LIBRARYDIR) && $(MAKE) gotthardVirtualServer DESTDIR=$(BINDIR)


install_client: textclient slsReceiver

install_gui: gui

confinstall:
	make conf;\
	make install

install_lib: 
	make lib;\
	make libreceiver; \
	make textclient; \
	make slsReceiver; \
	make doc; \
	make htmldoc; \
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR); \
	cd $(LIBRARYRXRDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR);

install: 
	make install_lib; \
	make gui; \
	make calWiz; \
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR);\
	cd $(LIBRARYRXRDIR) && $(MAKE) install_inc DESTDIR=$(INCDIR);

conf:
	set -e; \
	. ./configure; \
	@echo "INSTALLROOT is $(INSTALLROOT)"
	@echo "BINDIR is $(BINDIR)"
	@echo "LIBDIR is $(LIBDIR)"
	@echo "INCDIR is $(INCDIR)"
	@echo "DOCDIR is $(DOCDIR)"


help:
	@echo "Targets:"
	@echo "make all 		compile library,  text clients, data reciever"
	@echo "make lib 		compile library"
	@echo "make libreceiver 		compile receiver library"
	@echo "make textclient		compile the slsDetectorClient dynamically linking the libraries"
	@echo "make stextclient 		compile slsDetectorClient statically linking the libraries"
	@echo "make receiver		compile the slsReciever dynamically linking the libraries"
	@echo "make sreceiver		compile the slsReciever statically linking the libraries"
	@echo "make gui			compile slsDetectorGUI - requires a working Qt4 and Qwt installation"
	@echo "make calWiz 		compile the calibration wizards - requires a working Root installation"
	@echo "make doc			compile pdf documentation"
	@echo "make htmldoc		compile html (and pdf) documentation"
	@echo "make install_lib         installs the libraries, the text clients, the documentation and the includes for the API"
	@echo "make install             installs all software, including the gui, the cal wizards and the includes for the API"
	@echo "make confinstall         installs all software, including the gui, the cal wizards and the includes for the API, prompting for the install paths"
	@echo "make clean              	remove object files and executables"
	@echo "make help               	lists possible targets"
	@echo ""
	@echo ""
	@echo "Makefile variables"
	@echo "DEBUG=1,2		 set debug level to 1 (VERBOSE) or 2 (VERYVERBOSE)"
	@echo ""
	@echo ""
	@echo "Variables -  to change them run <source configure> :"
	@echo "INSTALLROOT=<yourdir>:    installation root di	r, default $PWD"
	@echo "BINDIR=<yourbin>:         binary installation dir below INSTALLROOT, default bin"
	@echo "LIBDIR=<yourlib>:         library installation dir below INSTALLROOT, default lib"
	@echo "INCDIR=<yourincludes>:    header installation dir below INSTALLROOT, default include"
	@echo "DOCDIR=<yourdoc>:         documentation installation dir below INSTALLROOT, default doc"
