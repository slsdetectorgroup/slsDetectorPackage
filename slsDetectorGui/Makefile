# LEO: Won't compile without libSlsDetector
  
include ../Makefile.include

$(info )
$(info ##################################)
$(info #    In slsDetectorGui Makefile  #)
$(info ##################################)
$(info )


DESTDIR		?=	../bin
LIBDIR		?=  ../bin
PROG		= 	$(DESTDIR)/slsDetectorGui
DOCDIR		?=	docs

LIBRARYDIR		?=	../slsDetectorSoftware
LIBRARYRXRDIR 	?= 	../slsReceiverSoftware
INCLUDES	?= 	$(LIBRARYDIR)/commonFiles -I$(LIBRARYRXRDIR)/MySocketTCP  -I$(LIBRARYDIR)/slsReceiverInterface   -I$(LIBRARYDIR)/slsDetector  -I$(LIBRARYDIR)/slsDetectorAnalysis  -I$(LIBRARYDIR)/multiSlsDetector   -I$(LIBRARYDIR)/usersFunctions  -I$(LIBRARYRXRDIR)/includes

LDFLAGDET	?=	 -L$(LIBDIR)  -Wl,-rpath=$(LIBDIR) -lSlsDetector 



#ifeq ( $(EIGERSLS), yes)  	
#	LDFLAG += $(EIGERFLAGS)
#else ifeq ( $(ROOTSLS), yes)  	
#	LDFLAG += $(ROOTFLAGS)
#endif

.PHONY: all lib clean mm doc htmldoc guiclient


all: lib Makefile.gui $(PROG) guiclient

lib:  
	cd $(LIBRARYDIR) && $(MAKE) DESTDIR=$(DESTDIR) 
	@echo ""
	@echo "#######################################"
	@echo "#   Back in slsDetectorGui Makefile   #"
	@echo "#######################################"
	@echo ""


clean: 
	if test -e Makefile.gui; then $(MAKE) -f Makefile.gui clean; $(MAKE) -f Makefile.gui mocclean; 	rm Makefile.gui; rm $(DESTDIR)/slsDetectorGui; else 	$(MAKE) Makefile.gui; 	$(MAKE) -f Makefile.gui clean; $(MAKE) -f Makefile.gui mocclean;  rm $(DESTDIR)/slsDetectorGui; fi
	cd client &&  $(MAKE) clean
#	cd manual && make clean


Makefile.gui: mm

mm: 
	qmake -set QT_INSTALL_PREFIX $(QTDIR) && qmake -o Makefile.gui INCLUDES='$(INCLUDES)' DESTDIR=$(DESTDIR) SLSDETLIB=$(DESTDIR) SUBLIBS='$(LDFLAGDET)'  LIBDIR='$(LIBDIR)'

doc:
	cd manual && $(MAKE) DESTDIR=$(DOCDIR)

htmldoc:
	cd manual && $(MAKE) html DESTDIR=$(DOCDIR)

$(PROG): Makefile $(DIR) src/*.cpp  include/*.h slsDetectorPlotting/include/*.h slsDetectorPlotting/src/*.cxx # forms/include/*.h
	$(MAKE) -f Makefile.gui SLSDETLIB=$(DESTDIR) DESTDIR=$(DESTDIR) SUBLIBS='$(LDFLAGDET)' INCLUDES='$(INCLUDES)' LIBDIR='$(LIBDIR)'

guiclient:
#	echo $(WD)
	cd client && $(MAKE) DESTDIR=$(DESTDIR)
	
