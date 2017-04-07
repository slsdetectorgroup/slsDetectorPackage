include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)
DOCDIR 		?= 	docs


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT 

INCLUDES?= -IcommonFiles -IslsDetector -I../slsReceiverSoftware/MySocketTCP -IusersFunctions -ImultiSlsDetector -IslsDetectorUtils -IslsDetectorCommand -IslsDetectorAnalysis -IslsReceiverInterface -I../slsReceiverSoftware/include -IthreadFiles -I$(ASM)

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/$(EPICS_HOST_ARCH) -Wl,-R/usr/local/epics/base/lib/$(EPICS_HOST_ARCH)  -lca -lCom 


SRC_CLNT=slsDetectorAnalysis/fileIO.cpp  usersFunctions/usersFunctions.cpp slsDetector/slsDetectorUtils.cpp slsDetector/slsDetectorCommand.cpp slsDetectorAnalysis/angularConversion.cpp slsDetectorAnalysis/angularConversionStatic.cpp slsDetectorAnalysis/energyConversion.cpp slsDetector/slsDetectorActions.cpp slsDetectorAnalysis/postProcessing.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp slsDetectorAnalysis/postProcessingFuncs.cpp slsReceiverInterface/receiverInterface.cpp slsDetector/slsDetectorUsers.cpp threadFiles/CondVar.cpp  threadFiles/Mutex.cpp threadFiles/ThreadPool.cpp   #../slsReceiverSoftware/MySocketTCP/MySocketTCP.cpp 


LIBZMQDIR = ../slsReceiverSoftware/include
LIBZMQ =  -L$(LIBZMQDIR) -Wl,-rpath=$(LIBZMQDIR) -lzmq

$(info )
$(info #######################################)
$(info #    Compiling slsDetectorSoftware    #)
$(info #######################################)
$(info )




OBJS = $(SRC_CLNT:.cpp=.o) 

.PHONY: all intdoc doc htmldoc package clean

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
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS)  -fPIC  $(EPICSFLAGS) -pthread  -lrt $(LIBZMQ) $(FLAGS)


package:  $(OBJS) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a 


$(DESTDIR)/libSlsDetector.so: $(OBJS) 
	$(CXX)  -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(DFLAGS)  $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -pthread -lrt $(LIBZMQ)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsDetector.so $(DESTDIR)

$(DESTDIR)/libSlsDetector.a: $(OBJS) 
	ar  rcs libSlsDetector.a  $(OBJS) 
	mv libSlsDetector.a $(DESTDIR)

clean:
	rm -rf  $(DESTDIR)/libSlsDetector.a   $(DESTDIR)/libSlsDetector.so core  docs/* slsDetectorUsersDocs $(OBJS)
	cd 

#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsDetector/slsDetectorUsers.h slsDetectorAnalysis/detectorData.h  $(DESTDIR)


