CFLAGS= -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -DDACS_INT -DTHIS_PATH='"$(shell pwd)"'

#ASM=$(shell echo "/lib/modules/`uname -r`/build/include")

#INCLUDES= -IcommonFiles -IslsDetector -IMySocketTCP -IusersFunctions -ImultiSlsDetector -IslsDetectorUtils -IslsDetectorCommand -IslsDetectorAnalysis -IslsReceiverInterface -I$(ASM)

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/SL5-x86/ -Wl,-R/usr/local/epics/base/lib/SL5-x86  -lca -lCom 
CC=g++


SRC_CLNT= slsDetectorAnalysis/fileIO.cpp MySocketTCP/MySocketTCP.cpp usersFunctions/usersFunctions.cpp slsDetector/slsDetectorUtils.cpp slsDetector/slsDetectorCommand.cpp slsDetectorAnalysis/angularConversion.cpp slsDetectorAnalysis/angularConversionStatic.cpp slsDetectorAnalysis/energyConversion.cpp slsDetector/slsDetectorActions.cpp slsDetectorAnalysis/postProcessing.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp slsDetector/slsDetectorUsers.cpp slsDetectorAnalysis/postProcessingFuncs.cpp slsReceiverInterface/receiverInterface.cpp

OBJS = $(SRC_CLNT:.cpp=.o) 

HEADERS = $(SRC_CLNT:.cpp=.h) commonFiles/sls_detector_defs.h slsDetectorAnalysis/detectorData.h slsDetector/slsDetectorBase.h multiSlsDetector/multiSlsDetectorCommand.h   slsDetectorAnalysis/enCalLogClass.h  slsDetectorAnalysis/angCalLogClass.h slsDetectorAnalysis/angleConversionConstant.h  usersFunctions/angleFunction.h slsReceiverInterface/receiverInterface.h slsDetector/svnInfoLib.h



SRC_MYTHEN_SVC = mythenDetectorServer/server.c mythenDetectorServer/server_funcs.c mythenDetectorServer/communication_funcs.c mythenDetectorServer/firmware_funcs.c mythenDetectorServer/mcb_funcs.c mythenDetectorServer/trimming_funcs.c 



DESTDIR ?= bin
DOCDIR ?= docs



all: package $(SRC_CLNT)
	echo "compiling all"

intdoc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config

doc: $(DOCDIR)/pdf/slsDetectorUsersDocs.pdf

$(DOCDIR)/pdf/slsDetectorUsersDocs.pdf: slsDetectorUsersDocs
	cd slsDetectorUsersDocs/latex && make 
	$(shell test -d $(DOCDIR) || mkdir -p $(DOCDIR))
	$(shell test -d $(DOCDIR)/pdf || mkdir -p $(DOCDIR)/pdf)
	cp slsDetectorUsersDocs/latex/refman.pdf $(DOCDIR)/pdf/slsDetectorUsersDocs.pdf


htmldoc: $(DOCDIR)/html/slsDetectorUsersDocs

$(DOCDIR)/html/slsDetectorUsersDocs: slsDetectorUsersDocs
	$(shell test -d $(DOCDIR) || mkdir -p $(DOCDIR))
	$(shell test -d $(DOCDIR)/html || mkdir -p $(DOCDIR)/html)
	$(shell test -d $(DOCDIR)/html/slsDetectorUsersDocs && rm -r $(DOCDIR)/html/slsDetectorUsersDocs)
	cp -r slsDetectorUsersDocs/html $(DOCDIR)/html/slsDetectorUsersDocs

slsDetectorUsersDocs: slsDetectorUsers.doxy slsDetector/slsDetectorUsers.h slsDetector/slsDetectorUsers.cpp slsDetectorAnalysis/detectorData.h
	doxygen slsDetectorUsers.doxy


mythenServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDES) -ImythenDetectorServer -DVIRTUAL -lm -D MCB_FUNCS -DC_ONLY -DVERBOSE
	mv a.out mythenServer


picassoServer:  $(SRC_MYTHEN_SVC) 
	$(CC)  $(SRC_MYTHEN_SVC)  $(CFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDES) -ImythenDetectorServer -D VIRTUAL -lm -DMCB_FUNCS -DPICASSOD -DC_ONLY
	mv a.out  picassoServer




%.o : %.cpp %.h Makefile
	$(CXX) -Wall -o $@ -c $< $(INCLUDES) $(DFLAGS) $(FLAGS)  -fPIC  $(EPICSFLAGS) -L/usr/lib64/




package: $(OBJS) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a 

$(DESTDIR)/libSlsDetector.so: $(OBJS) 
	$(CXX)  -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64/
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsDetector.so $(DESTDIR)

$(DESTDIR)/libSlsDetector.a: $(OBJS) 
	ar  rcs libSlsDetector.a  $(OBJS) 
	mv libSlsDetector.a $(DESTDIR)

clean:
	rm -rf libSlsDetector.a  libSlsDetector.so core  docs/* slsDetectorUsersDocs $(OBJS)


#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsDetector/slsDetectorUsers.h slsDetectorAnalysis/detectorData.h  $(DESTDIR)


