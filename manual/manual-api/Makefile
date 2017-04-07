INCLUDES	=   -I . 
SRC_DET		= 	mainClient.cpp 	
SRC_REC		=   mainReceiver.cpp
LIBDIR		=   ../../bin
LDFLAG_DET 	=	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -pthread
LDFLAG_REC 	=	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -pthread
DESTDIR		?=	../docs


#all: detUser detReceiver 
all: docs

docs:
	doxygen slsDetectorUsers.doxy

detUser:$(SRC_DET)
	echo "creating client"
	g++ -o detUser $(SRC_DET) $(INCLUDES) $(LDFLAG_DET) -lm -lstdc++

detReceiver:$(SRC_REC)
	echo "creating receiver"
	g++ -o detReceiver $(SRC_REC) $(INCLUDES) $(LDFLAG_REC) -lm -lstdc++
	
clean:
	echo "cleaning"
	rm -rf detUser detReceiver slsDetectorUsersDocs


