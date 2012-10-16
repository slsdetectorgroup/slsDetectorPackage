CFLAGS= -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -DDACS_INT

INCLUDES= -IcommonFiles -IslsDetector -IMySocketTCP -IusersFunctions -ImultiSlsDetector -IslsDetectorUtils -IslsDetectorCommand -IslsDetectorAnalysis

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/SL5-x86/ -Wl,-R/usr/local/epics/base/lib/SL5-x86  -lca -lCom 
CC=g++


SRC_CLNT= slsDetectorAnalysis/fileIO.cpp MySocketTCP/MySocketTCP.cpp usersFunctions/usersFunctions.cpp slsDetector/slsDetectorUtils.cpp slsDetector/slsDetectorCommand.cpp slsDetectorAnalysis/angularConversion.cpp slsDetectorAnalysis/angularConversionStatic.cpp slsDetectorAnalysis/energyConversion.cpp slsDetector/slsDetectorActions.cpp slsDetectorAnalysis/postProcessing.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp slsDetector/slsDetectorUsers.cpp slsDetectorAnalysis/postProcessingFuncs.cpp 

OBJS = $(SRC_CLNT:.cpp=.o) 

HEADERS = $(SRC_CLNT:.cpp=.h) commonFiles/sls_detector_defs.h slsDetectorAnalysis/detectorData.h slsDetector/slsDetectorBase.h multiSlsDetector/multiSlsDetectorCommand.h   slsDetectorAnalysis/enCalLogClass.h  slsDetectorAnalysis/angCalLogClass.h slsDetectorAnalysis/angleConversionConstant.h  usersFunctions/angleFunction.h



SRC_MYTHEN_SVC = mythenDetectorServer/server.c mythenDetectorServer/server_funcs.c mythenDetectorServer/communication_funcs.c mythenDetectorServer/firmware_funcs.c mythenDetectorServer/mcb_funcs.c mythenDetectorServer/trimming_funcs.c 

all: package $(SRC_CLNT)
	echo "compiling all"

doc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config



mythenServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDES) -ImythenDetectorServer -DVIRTUAL -lm -D MCB_FUNCS -DC_ONLY -DVERBOSE
	mv a.out mythenServer


picassoServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDES) -ImythenDetectorServer -D VIRTUAL -lm -DMCB_FUNCS -DPICASSOD -DC_ONLY
	mv a.out  picassoServer




%.o : %.cpp %.h
	$(CXX) -Wall -o $@ -c $< $(INCLUDES) $(DFLAGS) $(FLAGS) -fPIC  $(EPICSFLAGS)


package: $(OBJS)
	$(CXX)  -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)
	ar  rcs libSlsDetector.a  $(OBJS)

clean:
	rm -rf libSlsDetector.a  libSlsDetector.so core  docs/* $(OBJS)


#-------------------------------------------------------------------------------
lib:    package


# added install target, HBl
install_lib: lib
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P libSlsDetector.so $(DESTDIR)

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P $(HEADERS) $(DESTDIR)


install_doc: doc
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -Pr docs/* $(DESTDIR)  
