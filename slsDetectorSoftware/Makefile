CFLAGS= -DC_ONLY 
FLAGS=-DVERBOSE 
INCLUDES= -I commonFiles -I slsDetector -I MySocketTCP -I usersFunctions -I multiSlsDetector 
#-I eigerDetector -ImythenDetector -IgotthardDetector  
#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/SL5-x86/ -Wl,-R/usr/local/epics/base/lib/SL5-x86  -lca -lCom 


SRC_CLNT= slsDetector/slsDetector.cpp MySocketTCP/MySocketTCP.cxx usersFunctions/usersFunctions.c multiSlsDetector/multiSlsDetector.cpp 
#mythenDetector/mythenDetector.cpp eigerDetector/eigerDetector.cpp gotthardDetector/gotthardDetector.cpp 
SRC_MYTHEN_SVC = mythenDetectorServer/server.c mythenDetectorServer/server_funcs.c mythenDetectorServer/communication_funcs.c mythenDetectorServer/firmware_funcs.c mythenDetectorServer/mcb_funcs.c mythenDetectorServer/trimming_funcs.c 
#mythenDetectorServer/sharedmemory.c

all: package mythenServer doc picassoServer

doc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config



mythenServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(INCLUDES) -ImythenDetectorServer -DVIRTUAL -lm -D MCB_FUNCS -DC_ONLY -DVERBOSE
	mv a.out mythenServer


picassoServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(INCLUDES) -ImythenDetectorServer -D VIRTUAL -lm -DMCB_FUNCS -DPICASSOD -DC_ONLY
	mv a.out  picassoServer



package: $(SRC_CLNT)	
	$(CXX) -fPIC -g -o objs/multiSlsDetector.o -c -Wall multiSlsDetector/multiSlsDetector.cpp $(INCLUDES) $(FLAGS) 
	$(CXX) -fPIC -g -o objs/slsDetector.o -c -Wall slsDetector/slsDetector.cpp $(INCLUDES) $(FLAGS) 
#	$(CXX) -fPIC -g -o objs/eigerDetector.o -c -Wall eigerDetector/eigerDetector.cpp $(INCLUDES) $(FLAGS) 
#	$(CXX) -fPIC -g -o objs/mythenDetector.o -c -Wall mythenDetector/mythenDetector.cpp $(INCLUDES) $(FLAGS) 
#	$(CXX) -fPIC -g -o objs/gotthardDetector.o -c -Wall gotthardDetector/gotthardDetector.cpp $(INCLUDES) $(FLAGS) 
	$(CXX) -fPIC -g -o objs/usersFunctions.o -c -Wall usersFunctions/usersFunctions.c $(INCLUDES) $(FLAGS) $(EPICSFLAGS)
	$(CXX) -fPIC -g -o objs/MySocketTCP.o -c -Wall MySocketTCP/MySocketTCP.cpp $(INCLUDES) $(FLAGS) 
	$(CXX) -fPIC -g -o objs/multiSDetector.o -c -Wall multiSlsDetector/multiSlsDetector.cpp $(INCLUDES) $(FLAGS) 
	$(CXX) -shared -Wl,-soname,libSlsDetector.so.1 -o libSlsDetector.so.1.0.1  objs/slsDetector.o   objs/usersFunctions.o  objs/MySocketTCP.o  objs/multiSDetector.o -lc $(INCLUDES) $(FLAGS) $(EPICSFLAGS)
#objs/mythenDetector.o  objs/gotthardDetector.o 
	ln -sf libSlsDetector.so.1.0.1   libSlsDetector.so 
	ln -sf libSlsDetector.so.1  libSlsDetector.so 
	ar  rcs libSlsDetector.a      objs/slsDetector.o   objs/usersFunctions.o  objs/MySocketTCP.o objs/multiSlsDetector.o  
#objs/mythenDetector.o  objs/gotthardDetector.o  

clean:
	rm -rf libSlsDetector.so.1.0.1  libSlsDetector.so core objs/*  docs/*

#-------------------------------------------------------------------------------
lib:    package


# added install target, HBl
install_lib: lib
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P libSlsDetector.so.1.0.1  libSlsDetector.so $(DESTDIR)

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P ../slsDetectorSoftware/MySocketTCP/MySocketTCP.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/eigerDetector/eigerDetector.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/mythenDetector/mythenDetector.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/gotthardDetector/gotthardDetector.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/slsDetector/slsDetector.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/commonFiles/sls_detector_defs.h  $(DESTDIR)
	cp -P ../slsDetectorSoftware/usersFunctions/usersFunctions.h $(DESTDIR)

install_doc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -Pr docs/* $(DESTDIR)  