
include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)
DOCDIR 		?= 	docs
SRCDIR		=	src
PROGS		= 	$(DESTDIR)/slsReceiver 


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS 

INCLUDES?= -Iinclude -IMySocketTCP -I../slsDetectorCalibration -I$(ASM) 

#-I$(SRCDIR)Interface

SRC_CLNT = $(SRCDIR)/MySocketTCP.cpp $(SRCDIR)/UDPInterface.cpp  $(SRCDIR)/UDPBaseImplementation.cpp $(SRCDIR)/UDPStandardImplementation.cpp $(SRCDIR)/slsReceiverTCPIPInterface.cpp $(SRCDIR)/slsReceiver.cpp $(SRCDIR)/slsReceiverUsers.cpp $(SRCDIR)/utilities.cpp 
MAIN_SRC = $(SRCDIR)/main.cpp

OBJS=$(SRC_CLNT:.cpp=.o) 


.PHONY: all intdoc  package eigerReceiver clean

all: lib $(SRC_CLNT) receiver

intdoc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config

%.o : %.cpp Makefile
ifeq ($(ROOTSLS),yes)
	$(CXX) -DROOTSLS -o $@ -c $< $(INCLUDES) $(DFLAGS)  $(ROOTFLAGS) -fPIC  $(EPICSFLAGS) -L/usr/lib64/ #$(FLAGS)
else	
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) -lpthread #$(FLAGS)
endif

lib:  $(OBJS) $(DESTDIR)/libSlsReceiver.so $(DESTDIR)/libSlsReceiver.a 

receiver: $(DESTDIR)/slsReceiver


$(DESTDIR)/libSlsReceiver.so: $(OBJS) 
	$(CXX)  -shared -Wl,-soname,libSlsReceiver.so -o libSlsReceiver.so  $(OBJS) -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -lpthread
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsReceiver.so $(DESTDIR)

$(DESTDIR)/libSlsReceiver.a: $(OBJS) 
	ar  rcs libSlsReceiver.a  $(OBJS) 
	mv libSlsReceiver.a $(DESTDIR)


$(DESTDIR)/slsReceiver: 	lib
	$(CXX)   -o $@  $(MAIN_SRC)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAGRXR) -fPIC 
#$(EIGERFLAGS)


# Stand-alone Mysocket tests
mysocket_test:
	g++ -c $(SRCDIR)/MySocketTCP.cpp -I include
	g++ -o rec MySocketTCP.o $(SRCDIR)/rec.cxx -I include
	g++ -o send MySocketTCP.o $(SRCDIR)/send.cxx -I include


clean:
	rm -rf $(OBJS)
	rm $(PROGS)
	rm $(DESTDIR)/libSlsReceiver.a   $(DESTDIR)/libSlsReceiver.so	
	cd 

#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsReceiver/slsReceiverUsers.h $(DESTDIR)


