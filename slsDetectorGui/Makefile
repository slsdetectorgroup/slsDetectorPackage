PROG= bin/slsDetectorGui
DIR= $(shell ls src/* slsDetectorPlotting/include/* slsDetectorPlotting/src/* include/* forms/include/*)


all: $(PROG)


clean: 
	if test -e Makefile.gui; then make -f Makefile.gui clean; make -f Makefile.gui mocclean; rm Makefile.gui; else 	make Makefile.gui; make -f Makefile.gui clean; make -f Makefile.gui mocclean;  fi



Makefile.gui: slsDetectorGui.pro
	qmake -o Makefile.gui

$(PROG): Makefile.gui $(DIR)
	make -f Makefile.gui

