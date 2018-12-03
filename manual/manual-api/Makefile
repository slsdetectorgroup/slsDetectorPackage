PKGDIR		=	../..
LIBDIR		=   $(PKGDIR)/bin
INCLUDES	=   -I . -I$(PKGDIR)/slsReceiverSoftware/include -I$(PKGDIR)/slsDetectorSoftware/slsDetectorAnalysis -I$(LIBDIR) -I$(PKGDIR)/slsDetectorSoftware/slsDetector 
SRC_DET		= 	mainClient.cpp 	
SRC_REC		=   mainReceiver.cpp
ZMQLIBDIR	= 	$(PKGDIR)/slsReceiverSoftware/include
LDFLAG_DET 	=	-I. -L$(LIBDIR) -Wl,-rpath=$(LIBDIR) -lSlsDetector -L/usr/lib64/ -pthread -lrt -L$(ZMQLIBDIR) -Wl,-rpath=$(ZMQLIBDIR) -lzmq
LDFLAG_REC 	=	-I. -L$(LIBDIR) -Wl,-rpath=$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -pthread -lrt -L$(ZMQLIBDIR) -Wl,-rpath=$(ZMQLIBDIR) -lzmq
DESTDIR		?=	../docs

HDF5 		?=	no
HDF5_DIR 	?=	/opt/hdf5v1.10.0

ifeq ($(HDF5),yes)
	LDFLAG_REC += -L$(HDF5_DIR)/lib  -Wl,-rpath=$(HDF5_DIR)/lib -lhdf5 -lhdf5_cpp -lsz -lz -DHDF5C 
endif

all: docs detUser slsMultiReceiver 
#all: docs

docs: createdocs docspdf docshtml removedocs

createdocs: slsDetectorUsers.doxy mainClient.cpp mainReceiver.cpp
	doxygen slsDetectorUsers.doxy	

docspdf: 
	cd slsDetectorUsersDocs/latex && make 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p $(DESTDIR)/pdf)
	mv slsDetectorUsersDocs/latex/refman.pdf $(DESTDIR)/pdf/slsDetectorUsersDocs.pdf

docshtml: 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/slsDetectorUsersDocs && rm -r $(DESTDIR)/html/slsDetectorUsersDocs)
	mv slsDetectorUsersDocs/html $(DESTDIR)/html/slsDetectorUsersDocs
	
removedocs:
	rm -rf 	slsDetectorUsersDocs;
	
detUser:$(SRC_DET)
	echo "creating client"
	mkdir -p bin
	g++ -o bin/detUser $(SRC_DET) $(INCLUDES) $(LDFLAG_DET) -lm -lstdc++

slsMultiReceiver:$(SRC_REC)
	echo "creating receiver"
	echo $LDFLAG_REC
	mkdir -p bin
	g++ -o bin/slsMultiReceiver $(SRC_REC) $(INCLUDES) $(LDFLAG_REC) -lm -lstdc++
	cp bin/slsMultiReceiver $(LIBDIR)
	
clean:
	echo "cleaning for manual-api"
	rm -rf bin/detUser bin/slsMultiReceiver bin/detReceiver slsDetectorUsersDocs
	rm -rf slsDetectorUsersDocs
	rm -rf $(DESTDIR)/html/slsDetectorUsersDocs
	rm -rf $(DESTDIR)/pdf/slsDetectorUsersDocs.pdf
	rm -rf $(LIBDIR)/slsMultiReceiver
	


