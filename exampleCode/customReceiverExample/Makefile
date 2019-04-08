INCLUDES	=   -I . 
SRC_REC		=   dummyMain.cpp
LDFLAG_REC 	=	-L. -lSlsReceiver -L/usr/lib64/ -lpthread -lrt -L. -lzmq
DESTDIR		?=	../docs

all: dummyReceiver


dummyReceiver:$(SRC_REC)
	echo "creating receiver"
	g++ -o dummyReceiver $(SRC_REC) $(INCLUDES) $(LDFLAG_REC) -lm -lstdc++


clean:
	echo "cleaning"
	rm -rf dummyReceiver


