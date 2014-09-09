
include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)
DOCDIR 		?= 	docs


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS 

INCLUDES?= -I. -Iincludes -IMySocketTCP -IslsReceiver -IslsDetectorCalibration -I$(ASM) 

#-IslsReceiverInterface

SRC_CLNT = MySocketTCP/MySocketTCP.cpp slsReceiver/UDPInterface.cpp  slsReceiver/UDPBaseImplementation.cpp slsReceiver/UDPStandardImplementation.cpp slsReceiver/slsReceiverTCPIPInterface.cpp slsReceiver/slsReceiver.cpp slsReceiver/slsReceiverUsers.cpp slsReceiver/utilities.cpp 
#slsReceiverInterface/receiverInterface.cpp
#slsReceiver/slsReceiverUDPFunctions.cpp
#slsReceiver/utilities.cpp   includes/logger.h
#ifeq ($(REST),yes)
#	SRC_CLNT += slsReceiver/UDPRESTImplementation.cpp
#	INCLUDES += $(RESTINCLUDE)
#endif

OBJS=$(SRC_CLNT:.cpp=.o) 
#OBJS = $(OBJS1:.h=.o) 
#OBJS += slsReceiver/logger.o
#OBJS +=  slsReceiver/eigerReceiver.o  



.PHONY: all intdoc  package eigerReceiver clean

all: package $(SRC_CLNT)

intdoc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config

%.o : %.cpp Makefile
ifeq ($(ROOTSLS),yes)
	echo "with root"
	$(CXX) -DROOTSLS -o $@ -c $< $(INCLUDES) $(DFLAGS)  $(ROOTFLAGS) -fPIC  $(EPICSFLAGS) -L/usr/lib64/ #$(FLAGS)
else	
	echo "without root"
	echo $(REST)
	echo $(INCLUDES)
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) -lpthread #$(FLAGS)
endif

# LEO: not satisfied by eigerReceiver 
package:  $(OBJS) $(DESTDIR)/libSlsReceiver.so $(DESTDIR)/libSlsReceiver.a 

#slsReceiver/logger.o:
#	$(CXX) -o $@ -c includes/logger.h $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) -lpthread #$(FLAGS)

#eigerReceiver:
#	echo "src client:" $(SRC_CLNT)
#	cd slsReceiver && make eigerReceiver

$(DESTDIR)/libSlsReceiver.so: $(OBJS) 
	$(CXX)  -shared -Wl,-soname,libSlsReceiver.so -o libSlsReceiver.so  $(OBJS) -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -lpthread
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsReceiver.so $(DESTDIR)

$(DESTDIR)/libSlsReceiver.a: $(OBJS) 
	ar  rcs libSlsReceiver.a  $(OBJS) 
	mv libSlsReceiver.a $(DESTDIR)

clean:
	echo rm -rf $(OBJS)
	rm -rf $(OBJS)
	cd slsReceiver && make clean
	cd 

#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsReceiver/slsReceiverUsers.h $(DESTDIR)


