CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
INCLUDES= -IcommonFiles -IslsDetector -IMySocketTCP -IusersFunctions -ImultiSlsDetector -IslsDetectorUtils -IslsDetectorCommand

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/SL5-x86/ -Wl,-R/usr/local/epics/base/lib/SL5-x86  -lca -lCom 
CC=g++


SRC_CLNT= slsDetector/slsDetector.cpp MySocketTCP/MySocketTCP.cpp usersFunctions/usersFunctions.cpp multiSlsDetector/multiSlsDetector.cpp  slsDetector/slsDetectorUtils.cpp slsDetector/slsDetectorCommand.cpp 

OBJS = $(SRC_CLNT:.cpp=.o) 


SRC_MYTHEN_SVC = mythenDetectorServer/server.c mythenDetectorServer/server_funcs.c mythenDetectorServer/communication_funcs.c mythenDetectorServer/firmware_funcs.c mythenDetectorServer/mcb_funcs.c mythenDetectorServer/trimming_funcs.c 

all: package $(SRC_CLNT)
	echo "compiling all"

doc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config



mythenServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(INCLUDES) -ImythenDetectorServer -DVIRTUAL -lm -D MCB_FUNCS -DC_ONLY -DVERBOSE
	mv a.out mythenServer


picassoServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(INCLUDES) -ImythenDetectorServer -D VIRTUAL -lm -DMCB_FUNCS -DPICASSOD -DC_ONLY
	mv a.out  picassoServer




%.o : %.cpp %.h
	$(CXX) -Wall -o $@ -c $< $(INCLUDES) $(FLAGS)  $(EPICSFLAGS)


package: $(OBJS)
	$(CXX) -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(FLAGS) $(EPICSFLAGS)
	ar  rcs libSlsDetector.a  $(OBJS)

clean:
	rm -rf libSlsDetector.a  libSlsDetector.so core  docs/* $(OBJS)


#-------------------------------------------------------------------------------
lib:    package


# added install target, HBl
install_lib: lib
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P libSlsDetector.so.1.0.1  libSlsDetector.so libSlsDetector.so.1  $(DESTDIR)

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P ../slsDetectorSoftware/MySocketTCP/MySocketTCP.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/slsDetector/slsDetectorBase.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/slsDetector/slsDetector.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/slsDetector/slsDetectorUtils.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/slsDetector/slsDetectorCommand.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/multiSlsDetector/multiSlsDetector.h $(DESTDIR)
	cp -P ../slsDetectorSoftware/commonFiles/sls_detector_defs.h  $(DESTDIR)
	cp -P ../slsDetectorSoftware/usersFunctions/usersFunctions.h $(DESTDIR)

install_doc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -Pr docs/* $(DESTDIR)  