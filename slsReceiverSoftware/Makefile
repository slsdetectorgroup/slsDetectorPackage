include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR	 	?= 	$(DESTDIR)
DOCDIR 		?= 	docs
SRCDIR		=	src
TESTDIR		=	test
BUILDDIR	=	build
PROGS		= 	$(DESTDIR)/slsReceiver 


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS 

INCLUDES?= $(INCLUDESRXR) -I include/ -I ../slsDetectorCalibration

#-Iinclude -I../slsDetectorCalibration -I$(ASM) 

SRC_CLNT = MySocketTCP.cpp UDPInterface.cpp  UDPBaseImplementation.cpp UDPStandardImplementation.cpp slsReceiverTCPIPInterface.cpp slsReceiver.cpp slsReceiverUsers.cpp utilities.cpp 

ifeq ($(REST), yes)
	SRC_CLNT += UDPRESTImplementation.cpp
endif

MAIN_SRC = main.cpp

OBJS=$(SRC_CLNT:%.cpp=$(BUILDDIR)/%.o) 


$(info )
$(info #######################################)
$(info #    Compiling slsReceiverSoftware    #)
$(info #######################################)
$(info )


.PHONY: all intdoc  package eigerReceiver clean

all: builddir lib receiver

intdoc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config

$(BUILDDIR)/%.o : $(SRCDIR)/%.cpp Makefile
ifeq ($(ROOTSLS),yes)
	$(CXX) -DROOTSLS -o $@ -c $< $(INCLUDES) $(DFLAGS)  $(ROOTFLAGS) -fPIC  $(EPICSFLAGS) $(LDFLAGRXR) -L/usr/lib64/ $(FLAGS)
else	
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) $(LDFLAGRXR) -lpthread $(FLAGS)
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
	$(CXX)   -o $@  $(SRCDIR)/$(MAIN_SRC)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAGRXR) -fPIC 
#$(EIGERFLAGS)


# Stand-alone Mysocket tests
mysocket_test:
	g++ -o $(TESTDIR)/MySocketTCP.o -c $(SRCDIR)/MySocketTCP.cpp -I include
	g++ -o $(TESTDIR)/rec $(TESTDIR)/MySocketTCP.o $(TESTDIR)/rec.cxx -I include
	g++ -o $(TESTDIR)/send $(TESTDIR)/MySocketTCP.o $(TESTDIR)/send.cxx -I include


clean: buildclean
	make testclean
	if test -e $(DESTDIR)/libSlsReceiver.a; then rm $(DESTDIR)/libSlsReceiver.a;fi
	if test -e $(DESTDIR)/libSlsReceiver.so; then rm $(DESTDIR)/libSlsReceiver.so;fi
	if test -e $(PROGS); then rm $(PROGS);fi

builddir:
	if [ ! -d $(BUILDDIR) ]; then mkdir $(BUILDDIR); fi

buildclean:
	rm -rf $(OBJS)		

testclean:
	if [ -f $(TESTDIR)/rec ]; then \
		cd $(TESTDIR) && rm *.o rec send; \
	fi


#-------------------------------------------------------------------------------

install: package

install_inc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P slsReceiver/slsReceiverUsers.h $(DESTDIR)


