INCLUDES	=   -I . 
SRC_DET		= 	mainClient.cpp 	
SRC_REC		=   mainReceiver.cpp
LIBDIR		=   .
LDFLAG_DET 	=	-I. -L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -pthread -lrt -L. -lzmq
LDFLAG_REC 	=	-I. -L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -pthread -lrt -L. -lzmq
DESTDIR		?=	../docs


all: docs detUser detReceiver 
#all: docs

docs: createdocs docspdf docshtml removedocs

createdocs: slsDetectorUsers.doxy slsDetectorUsers.h detectorData.h slsReceiverUsers.h mainClient.cpp mainReceiver.cpp
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
	g++ -o detUser $(SRC_DET) $(INCLUDES) $(LDFLAG_DET) -lm -lstdc++

detReceiver:$(SRC_REC)
	echo "creating receiver"
	g++ -o detReceiver $(SRC_REC) $(INCLUDES) $(LDFLAG_REC) -lm -lstdc++
	
clean:
	echo "cleaning for manual-api"
	rm -rf detUser detReceiver slsDetectorUsersDocs
	rm -rf slsDetectorUsersDocs
	rm -rf $(DESTDIR)/html/slsDetectorUsersDocs
	rm -rf $(DESTDIR)/pdf/slsDetectorUsersDocs.pdf
	


