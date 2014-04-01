CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT -DTHIS_PATH='"$(shell pwd)"' -DSLS_RECEIVER_FUNCTION_LIST $(shell root-config --cflags --glibs)   #-DALLFILE_DEBUG #-DMYROOT1 -DALLFILE   #-DMYROOT1 `root-config --cflags --glibs`

#ASM=$(shell echo "/lib/modules/`uname -r`/build/include")

INCLUDES?= -IcommonFiles -IslsDetector -IMySocketTCP -IusersFunctions -ImultiSlsDetector -IslsDetectorUtils -IslsDetectorCommand -IslsDetectorAnalysis -IslsReceiverInterface -IslsReceiver -I../slsDetectorCalibration -I$(ASM)

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/$(EPICS_HOST_ARCH) -Wl,-R/usr/local/epics/base/lib/$(EPICS_HOST_ARCH)  -lca -lCom 

CC=g++


SRC_CLNT= slsDetectorAnalysis/fileIO.cpp MySocketTCP/MySocketTCP.cpp usersFunctions/usersFunctions.cpp slsDetector/slsDetectorUtils.cpp slsDetector/slsDetectorCommand.cpp slsDetectorAnalysis/angularConversion.cpp slsDetectorAnalysis/angularConversionStatic.cpp slsDetectorAnalysis/energyConversion.cpp slsDetector/slsDetectorActions.cpp slsDetectorAnalysis/postProcessing.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp slsDetector/slsDetectorUsers.cpp slsDetectorAnalysis/postProcessingFuncs.cpp slsReceiverInterface/receiverInterface.cpp slsReceiver/slsReceiverFunctionList.cpp  slsReceiver/slsReceiver_funcs.cpp  slsReceiver/slsReceiverUsers.cpp slsReceiver/eigerReceiver.cpp  

OBJS = $(SRC_CLNT:.cpp=.o) 

HEADERS = $(SRC_CLNT:.cpp=.h) commonFiles/sls_detector_defs.h slsDetectorAnalysis/detectorData.h slsDetector/slsDetectorBase.h multiSlsDetector/multiSlsDetectorCommand.h   slsDetectorAnalysis/enCalLogClass.h  slsDetectorAnalysis/angCalLogClass.h slsDetectorAnalysis/angleConversionConstant.h  usersFunctions/angleFunction.h slsReceiverInterface/receiverInterface.h slsDetector/svnInfoLib.h slsReceiver/circularFifo.h        slsReceiver/slsReceiver_funcs.h        slsReceiver/svnInfoReceiverTmp.h slsReceiver/receiver_defs.h       slsReceiver/slsReceiverFunctionList.h  slsReceiver/slsReceiverUsers.h slsReceiver/svnInfoReceiver.h #../slsDetectorCalibration/singlePhotonDetector.h  ../slsDetectorCalibration/moenchCommonMode.h ../slsDetectorCalibration/moench02ModuleData.h ../slsDetectorCalibration/slsReceiverData.h

#POCODIR = /afs/psi.ch/user/s/sala/public/poco-slp_6.4-32bit
#JSONBOXDIR = /afs/psi.ch/user/s/sala/public/JsonBox-slp_6.4-32bit
POCODIR ?= /afs/psi.ch/user/s/sala/public/poco-slp_5.7-32bit
JSONBOXDIR ?= /afs/psi.ch/user/s/sala/public/JsonBox-slp_5.7-32bit
#POCODIR ?= /home/sala/Programs/poco-ubuntu_13.10-64bit
#JSONBOXDIR ?= /home/sala/Programs/JsonBox-ubuntu_13.10-64bit
EIGERFLAGS ?= -L $(JSONBOXDIR) -L $(POCODIR)/lib -Wl,-rpath=$(POCODIR)/lib -I $(POCODIR)/include -I $(JSONBOXDIR)/include 

DESTDIR = bin
DOCDIR ?= docs


all: package $(SRC_CLNT)
	echo "-----------compiling all"

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


mythenVirtualServer:  $(SRC_MYTHEN_SVC) 
	cd mythenDetectorServer && make -f Makefile.virtual DESTDIR=$(DESTDIR) 

gotthardVirtualServer:  $(SRC_MYTHEN_SVC) 
	cd gotthardDetectorServer && make -f Makefile.virtual DESTDIR=$(DESTDIR) 



%.o : %.cpp %.h Makefile
	echo "------------- compiling" $@
	$(CXX) -Wall -o $@ -c $< $(INCLUDES) $(DFLAGS) $(FLAGS) $(EIGERFLAGS) -fPIC  $(EPICSFLAGS) -L/usr/lib64/

package: eigerReceiver $(OBJS) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a 

eigerReceiver:
	cd slsReceiver && make eigerReceiver

$(DESTDIR)/libSlsDetector.so: $(OBJS) 
	echo "------------- Moving"
	$(CXX)  -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64/
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsDetector.so $(DESTDIR)

$(DESTDIR)/libSlsDetector.a: $(OBJS) 
	ar  rcs libSlsDetector.a  $(OBJS) 
	mv libSlsDetector.a $(DESTDIR)

clean:
	rm -rf $(DESTDIR)/libSlsDetector.a  $(DESTDIR)/libSlsDetector.so core  docs/* slsDetectorUsersDocs $(OBJS)


#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsDetector/slsDetectorUsers.h slsDetectorAnalysis/detectorData.h  $(DESTDIR) slsReceiver/slsReceiverUsers.h $(DESTDIR)


