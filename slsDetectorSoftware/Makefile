### LEO: NB slsDetectorSoftware cannot compile by itself, as it depends on slsDetectorCalibration (see INCLUDES)

include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)
DOCDIR 		?= 	docs


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT -DTHIS_PATH='"$(shell pwd)"' -DSLS_RECEIVER_FUNCTION_LIST $(shell root-config --cflags --glibs)   #-DALLFILE_DEBUG #-DMYROOT1 -DALLFILE   #-DMYROOT1 `root-config --cflags --glibs`

INCLUDES?= -IcommonFiles -IslsDetector -IMySocketTCP -IusersFunctions -ImultiSlsDetector -IslsDetectorUtils -IslsDetectorCommand -IslsDetectorAnalysis -IslsReceiverInterface -IslsReceiver -I../slsDetectorCalibration -I$(ASM)

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/$(EPICS_HOST_ARCH) -Wl,-R/usr/local/epics/base/lib/$(EPICS_HOST_ARCH)  -lca -lCom 

SRC_CLNT=slsDetectorAnalysis/fileIO.cpp MySocketTCP/MySocketTCP.cpp usersFunctions/usersFunctions.cpp slsDetector/slsDetectorUtils.cpp slsDetector/slsDetectorCommand.cpp slsDetectorAnalysis/angularConversion.cpp slsDetectorAnalysis/angularConversionStatic.cpp slsDetectorAnalysis/energyConversion.cpp slsDetector/slsDetectorActions.cpp slsDetectorAnalysis/postProcessing.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp slsDetector/slsDetectorUsers.cpp slsDetectorAnalysis/postProcessingFuncs.cpp slsReceiverInterface/receiverInterface.cpp slsReceiver/slsReceiverFunctionList.cpp  slsReceiver/slsReceiver_funcs.cpp  slsReceiver/slsReceiverUsers.cpp
# slsReceiver/eigerReceiver.cpp  



OBJS = $(SRC_CLNT:.cpp=.o) 
OBJS +=  slsReceiver/eigerReceiver.o  

### LEO: unused
#HEADERS = $(SRC_CLNT:.cpp=.h) commonFiles/sls_detector_defs.h slsDetectorAnalysis/detectorData.h slsDetector/slsDetectorBase.h multiSlsDetector/multiSlsDetectorCommand.h   slsDetectorAnalysis/enCalLogClass.h  slsDetectorAnalysis/angCalLogClass.h slsDetectorAnalysis/angleConversionConstant.h  usersFunctions/angleFunction.h slsReceiverInterface/receiverInterface.h slsDetector/svnInfoLib.h slsReceiver/circularFifo.h        slsReceiver/slsReceiver_funcs.h        slsReceiver/svnInfoReceiverTmp.h slsReceiver/receiver_defs.h       slsReceiver/slsReceiverFunctionList.h  slsReceiver/slsReceiverUsers.h slsReceiver/svnInfoReceiver.h #../slsDetectorCalibration/singlePhotonDetector.h  ../slsDetectorCalibration/moenchCommonMode.h ../slsDetectorCalibration/moench02ModuleData.h ../slsDetectorCalibration/slsReceiverData.h


.PHONY: all intdoc doc htmldoc package eigerReceiver clean

all: package $(SRC_CLNT)

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
ifeq ($(EIGERSLS),yes)
	$(CXX) -DEIGERSLS -o $@ -c $< $(INCLUDES) $(DFLAGS)  $(EIGERFLAGS) -fPIC  $(EPICSFLAGS) -L/usr/lib64/ #$(FLAGS)
else
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) -lpthread #$(FLAGS)
endif

# LEO: not satisfied by eigerReceiver 
package:  eigerReceiver $(OBJS) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a 

eigerReceiver:
	cd slsReceiver && make eigerReceiver

$(DESTDIR)/libSlsDetector.so: $(OBJS) 
	$(CXX)  -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -lpthread
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsDetector.so $(DESTDIR)

$(DESTDIR)/libSlsDetector.a: $(OBJS) 
	ar  rcs libSlsDetector.a  $(OBJS) 
	mv libSlsDetector.a $(DESTDIR)

clean:
	cd slsReceiver && make clean
	rm -rf  $(DESTDIR)/libSlsDetector.a   $(DESTDIR)/libSlsDetector.so core  docs/* slsDetectorUsersDocs $(OBJS)
	cd 

#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsDetector/slsDetectorUsers.h slsDetectorAnalysis/detectorData.h  $(DESTDIR) slsReceiver/slsReceiverUsers.h $(DESTDIR)


