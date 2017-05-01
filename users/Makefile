WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -pthread
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -pthread
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/include
INCLUDESDET		=	-I $(LIBRARYRXRDIR)/include -I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis



all: receivers users

receivers:
	@echo $(WD)
	g++ mainReceiver.cpp   $(INCLUDESRXR) $(LDFLAGRXR) -o userReceiver

users:
	@echo $(WD)
	g++ mainClient.cpp   $(INCLUDESDET) $(LDFLAGDET) -o userClient 


clean:
	rm userReceiver userClient

