
PROG= $(DESTDIR)/slsDetectorGui
#DIR= $(shell ls src/* slsDetectorPlotting/include/* slsDetectorPlotting/src/* include/ forms/*)
DESTDIR?=bin
DOCDIR?=docs
LIBDIR?=../slsDetectorSoftware


all: $(PROG) Makefile.gui

clean: 
	if test -e Makefile.gui; then make -f Makefile.gui clean; make -f Makefile.gui mocclean; rm Makefile.gui; else 	make Makefile.gui; make -f Makefile.gui clean; make -f Makefile.gui mocclean;  fi
	cd manual && make clean



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

