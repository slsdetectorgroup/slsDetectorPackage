
SRC=jctbDict.cpp jctbMain.cpp jctbDacs.cpp jctbDac.cpp  jctbPattern.cpp  jctbSignals.cpp jctbSignal.cpp jctbAdcs.cpp jctbAcquisition.cpp
INCS=jctbSignals.h jctbSignal.h jctbPattern.h jctbDacs.h jctbDac.h jctbMain.h jctbAdcs.h jctbAcquisition.h
LINKDEF=jctbLinkDef.h 

INCDIR=-I../slsReceiverSoftware/include/ -I../slsDetectorSoftware/multiSlsDetector/ -I../slsDetectorSoftware/slsDetector -I../slsDetectorSoftware/commonFiles -I../slsDetectorSoftware/slsDetectorAnalysis -I../slsDetectorSoftware/slsDetectorUsers -I../slsDetectorSoftware/usersFunctions
LDFLAG=-L../bin -lSlsDetector -L/usr/lib64/ -lpthread

MAIN=JCTBGui.cpp

DESTDIR?=../bin

all: $(DESTDIR)/JCTBGui

doc:
	cd manual && make DESTDIR=$(DESTDIR)

htmldoc:
	cd manual && make html DESTDIR=$(DESTDIR)

jctbDict.cpp: $(INCS)  $(LINKDEF)	
	rootcint -f jctbDict.cpp -c $(INCS) $(LINKDEF)	

$(DESTDIR)/JCTBGui: $(SRC) $(MAIN)  $(LINKDEF)	$(INCS)
	g++ -DMYROOT `root-config --cflags --glibs` -lMinuit $(LDFLAG) -o JCTBGui $(INCDIR) $(SRC)  $(MAIN)
	mv JCTBGui $(DESTDIR)	

clean:
	rm -f JCTBGui *.o jctbDict.*
