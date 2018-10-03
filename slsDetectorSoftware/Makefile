include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT 

INCLUDES?= -I../commonFiles -IslsDetector -I../slsReceiverSoftware/MySocketTCP  -ImultiSlsDetector  -IslsReceiverInterface -I../slsReceiverSoftware/include -IthreadFiles -IsharedMemory -I$(ASM)

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/$(EPICS_HOST_ARCH) -Wl,-R/usr/local/epics/base/lib/$(EPICS_HOST_ARCH)  -lca -lCom 

LIBZMQDIR = ../commonFiles
LIBZMQ =  -L$(LIBZMQDIR) -Wl,-rpath=$(LIBZMQDIR) -lzmq

SRC_CLNT=  slsDetector/slsDetectorCommand.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp slsReceiverInterface/receiverInterface.cpp slsDetector/slsDetectorUsers.cpp threadFiles/CondVar.cpp  threadFiles/Mutex.cpp threadFiles/ThreadPool.cpp sharedMemory/SharedMemory.cpp ../commonFiles/utilities.cpp#../slsReceiverSoftware/MySocketTCP/MySocketTCP.cpp 
DEPSINCLUDES = ../commonFiles/sls_receiver_defs.h ../commonFiles/sls_receiver_funcs.h ../commonFiles/ansi.h ../commonFiles/sls_detector_defs.h  ../commonFiles/sls_detector_funcs.h ../commonFiles/error_defs.h slsDetector/slsDetectorBase.h slsDetector/detectorData.h  threadFiles/Global.h threadFiles/Task.h sharedMemory/SharedMemory.h ../commonFiles/sls_receiver_exceptions.h ../commonFiles/versionAPI.h ../commonFiles/utilities.h



$(info )
$(info #######################################)
$(info #   In slsDetectorSoftware Makefile   #)
$(info #######################################)
$(info )




OBJS = $(SRC_CLNT:%.cpp=%.o) 

.PHONY: all package clean

all: package $(SRC_CLNT)

mythenVirtualServer:  $(SRC_MYTHEN_SVC) 
	cd mythenDetectorServer && make -f Makefile.virtual DESTDIR=$(DESTDIR) 

gotthardVirtualServer:  $(SRC_MYTHEN_SVC) 
	cd gotthardDetectorServer && make -f Makefile.virtual DESTDIR=$(DESTDIR) 


%.o : %.cpp %.h $(DEPSINCLUDES) Makefile 
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS)  -fPIC  $(EPICSFLAGS) -pthread  -lrt $(LIBZMQ) $(FLAGS)


package: versioning $(OBJS) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a 

versioning: 
	$(call colorecho,`./updateGitVersion.sh`)

$(DESTDIR)/libSlsDetector.so: $(OBJS) 
	$(call colorecho,"#######################################")
	$(call colorecho,"#    Compiling slsDetector Library    #")
	$(call colorecho,"#######################################")
	$(CXX)  -shared -Wl,-soname,libSlsDetector.so -o libSlsDetector.so  $(OBJS)  -lc $(INCLUDES) $(DFLAGS)  $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -pthread -lrt $(LIBZMQ)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsDetector.so $(DESTDIR)

$(DESTDIR)/libSlsDetector.a: $(OBJS) 
	ar  rcs libSlsDetector.a  $(OBJS) 
	mv libSlsDetector.a $(DESTDIR)

clean:
	rm -rf  $(DESTDIR)/libSlsDetector.a   $(DESTDIR)/libSlsDetector.so core  $(OBJS)
	cd 
	
	
	

#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsDetector/slsDetectorUsers.h slsDetector/detectorData.h  $(DESTDIR)


