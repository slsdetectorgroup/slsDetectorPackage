
PROG= $(DESTDIR)/slsDetectorGui
DESTDIR?=bin
DOCDIR?=docs

LIBDIR?=../bin
LIBRARYDIR?=../slsDetectorSoftware
INCLUDES?= $(LIBRARYDIR)/commonFiles -I$(LIBRARYDIR)/MySocketTCP  -I$(LIBRARYDIR)/slsReceiverInterface   -I$(LIBRARYDIR)/slsDetector  -I$(LIBRARYDIR)/slsDetectorAnalysis  -I$(LIBRARYDIR)/multiSlsDetector   -I$(LIBRARYDIR)/usersFunctions  
LDFLAG?=-L$(LIBRARYDIR)/bin -lSlsDetector 

all: $(PROG) Makefile.gui guiclient

clean: 
	if test -e Makefile.gui; then make -f Makefile.gui clean; make -f Makefile.gui mocclean; 	rm Makefile.gui; else 	make Makefile.gui; 	make -f Makefile.gui clean; make -f Makefile.gui mocclean;  fi
	cd client &&  $(MAKE) clean
#	cd manual && make clean


Makefile.gui: mm

mm:
	qmake -set QT_INSTALL_PREFIX $(QTDIR) && qmake -o Makefile.gui INCLUDES='$(INCLUDES)' DESTDIR=$(DESTDIR) SLSDETLIB=$(LIBDIR) SUBLIBS='$(LDFLAG)' INCLUDES='$(INCLUDES)' 

doc:
	cd manual && make DESTDIR=$(DOCDIR)

htmldoc:
	cd manual && make html DESTDIR=$(DOCDIR)

$(PROG): Makefile.gui $(DIR)
	echo $(LDFLAG)
	make -f Makefile.gui SLSDETLIB=$(LIBDIR) DESTDIR=$(DESTDIR) SUBLIBS='$(LDFLAG)' INCLUDES='$(INCLUDES)'


guiclient:
	echo $(WD)
	cd client && $(MAKE) DESTDIR=$(DESTDIR)
