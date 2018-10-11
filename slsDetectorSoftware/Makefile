include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT 

INCLUDES?= -I../slsSupportLib/include -IslsDetector  -ImultiSlsDetector  -I../slsSupportLib/include -IsharedMemory  I$(ASM)

#EPICSFLAGS=-D EPICS -I/usr/local/epics/base/include/ -I /usr/local/epics/base/include/os/Linux/ -L /usr/local/epics/base/lib/$(EPICS_HOST_ARCH) -Wl,-R/usr/local/epics/base/lib/$(EPICS_HOST_ARCH)  -lca -lCom 

LIBZMQDIR = ../slsSupportLib/include
LIBZMQ =  -L$(LIBZMQDIR) -Wl,-rpath=$(LIBZMQDIR) -lzmq

SRC_CLNT=  slsDetector/slsDetectorCommand.cpp slsDetector/slsDetector.cpp multiSlsDetector/multiSlsDetector.cpp ../slsSupportLib/include/ReceiverInterface.cpp slsDetector/slsDetectorUsers.cpp  sharedMemory/SharedMemory.cpp ../slsSupportLib/include/utilities.cpp
DEPSINCLUDES = ../slsSupportLib/include/ansi.h ../slsSupportLib/include/sls_detector_defs.h  ../slsSupportLib/include/sls_detector_funcs.h ../slsSupportLib/include/error_defs.h  slsDetector/detectorData.h   sharedMemory/SharedMemory.h ../slsSupportLib/include/sls_detector_exceptions.h ../slsSupportLib/include/versionAPI.h ../slsSupportLib/include/utilities.h     ../slsSupportLib/include/container_utils.h



$(info )
$(info #######################################)
$(info #   In slsDetectorSoftware Makefile   #)
$(info #######################################)
$(info )




OBJS = $(SRC_CLNT:%.cpp=%.o) 

.PHONY: all package clean

all: package $(SRC_CLNT)


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


