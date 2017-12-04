# LEO: Won't compile without libSlsDetector
  
include ../Makefile.include

$(info )
$(info ##################################)
$(info #    Compiling slsDetectorGui    #)
$(info ##################################)
$(info )



PROG		= 	$(DESTDIR)/slsDetectorGui

DESTDIR		?=	../bin
LIBDIR		?=	$(DESTDIR)
DOCDIR		?=	docs

LIBRARYDIR		?=	../slsDetectorSoftware
LIBRARYRXRDIR 	?= 	../slsReceiverSoftware
INCLUDES	?= 	$(LIBRARYDIR)/commonFiles -I$(LIBRARYRXRDIR)/MySocketTCP  -I$(LIBRARYDIR)/slsReceiverInterface   -I$(LIBRARYDIR)/slsDetector  -I$(LIBRARYDIR)/slsDetectorAnalysis  -I$(LIBRARYDIR)/multiSlsDetector   -I$(LIBRARYDIR)/usersFunctions  -I$(LIBRARYRXRDIR)/includes

#ifeq ( $(EIGERSLS), yes)  	
#	LDFLAG += $(EIGERFLAGS)
#else ifeq ( $(ROOTSLS), yes)  	
#	LDFLAG += $(ROOTFLAGS)
#endif

.PHONY: all lib clean mm doc htmldoc guiclient


all: lib $(PROG) Makefile.gui guiclient

lib:
	cd ../ && $(MAKE) lib

clean: 
	if test -e Makefile.gui; then $(MAKE) -f Makefile.gui clean; $(MAKE) -f Makefile.gui mocclean; 	rm Makefile.gui; else 	$(MAKE) Makefile.gui; 	$(MAKE) -f Makefile.gui clean; $(MAKE) -f Makefile.gui mocclean;  fi
	cd client &&  $(MAKE) clean
#	cd manual && make clean


Makefile.gui: mm

mm:
	qmake -set QT_INSTALL_PREFIX $(QTDIR) && qmake -o Makefile.gui INCLUDES='$(INCLUDES)' DESTDIR=$(DESTDIR) SLSDETLIB=$(LIBDIR) SUBLIBS='$(LDFLAGDET)'

doc:
	cd manual && $(MAKE) DESTDIR=$(DOCDIR)

htmldoc:
	cd manual && $(MAKE) html DESTDIR=$(DOCDIR)

$(PROG): Makefile.gui $(DIR)
	$(MAKE) -f Makefile.gui SLSDETLIB=$(LIBDIR) DESTDIR=$(DESTDIR) SUBLIBS='$(LDFLAGDET)' INCLUDES='$(INCLUDES)'

guiclient:
	echo $(WD)
	cd client && $(MAKE) DESTDIR=$(DESTDIR)
