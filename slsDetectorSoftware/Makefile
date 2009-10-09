CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
INCLUDES= -I commonFiles -I slsDetector -I MySocketTCP -I eigerDetector -ImythenDetector  -I usersFunctions


#SRC_H=  slsDetector/slsDetector.h mythenDetector/mythenDetector.h eigerDetector/eigerDetector.h MySocketTCP/MySocketTCP.h usersFunctions/usersFunctions.h commonFiles/sls_detector_defs.h 

#mythenDetectorServer/communication_funcs.h mythenDetectorServer/firmware_funcs.h mythenDetectorServer/mcb_funcs.h mythenDetectorServer/registers.h mythenDetectorServer/server_defs.h mythenDetectorServer/server_funcs.h mythenDetectorServer/trimming_funcs.h


SRC_CLNT= slsDetector/slsDetector.cpp MySocketTCP/MySocketTCP.cxx usersFunctions/usersFunctions.c mythenDetector/mythenDetector.cpp eigerDetector/eigerDetector.cpp

SRC_MYTHEN_SVC = mythenDetectorServer/server.c mythenDetectorServer/server_funcs.c commonFiles/communication_funcs.c mythenDetectorServer/firmware_funcs.c mythenDetectorServer/mcb_funcs.c mythenDetectorServer/trimming_funcs.c mythenDetectorServer/sharedmemory.c

all: package mythenServer doc

doc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config




mythenServer:  $(SRC_MYTHEN_SVC) 
	gcc  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(INCLUDES) -ImythenDetectorServer -D VIRTUAL -lm -D MCB_FUNCS
	mv a.out bin/mythenDetectorServer



package: $(SRC_CLNT)
	gcc -fPIC -g -o objs/slsDetector.o -c -Wall slsDetector/slsDetector.cpp $(INCLUDES)
	gcc -fPIC -g -o objs/eigerDetector.o -c -Wall eigerDetector/eigerDetector.cpp $(INCLUDES)
	gcc -fPIC -g -o objs/usersFunctions.o -c -Wall usersFunctions/usersFunctions.c $(INCLUDES)
	gcc -fPIC -g -o objs/MySocketTCP.o -c -Wall MySocketTCP/MySocketTCP.cxx $(INCLUDES)
	gcc -shared -Wl,-soname,libSlsDetector.so.1 -o bin/libSlsDetector.so.1.0.1 objs/slsDetector.o  objs/eigerDetector.o  objs/usersFunctions.o  objs/MySocketTCP.o  -lc $(INCLUDES)

clean:
	rm bin/* core objs/* 
