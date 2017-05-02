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

DFLAGS= -g -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS #-DVERBOSE

INCLUDES?= $(INCLUDESRXR) -I include/ -I ../slsDetectorCalibration

##############################################################
# ZMQ specific: in this case, you need ZMQ libraries already included in this package
###########################################################
LIBZMQDIR = include
LIBZMQ =  -L$(LIBZMQDIR) -Wl,-rpath=$(LIBZMQDIR) -lzmq

#-Iinclude -I../slsDetectorCalibration -I$(ASM) 

SRC_CLNT = MySocketTCP.cpp  ThreadObject.cpp Listener.cpp DataProcessor.cpp DataStreamer.cpp Fifo.cpp File.cpp BinaryFile.cpp UDPInterface.cpp  UDPBaseImplementation.cpp UDPStandardImplementation.cpp slsReceiverTCPIPInterface.cpp slsReceiver.cpp slsReceiverUsers.cpp utilities.cpp
ifeq ($(HDF5),yes)
	SRC_CLNT += HDF5File.cpp HDF5FileStatic.cpp
endif

ifeq ($(REST), yes)
	SRC_CLNT += UDPRESTImplementation.cpp
endif

MAIN_SRC = main.cpp

DUMMY_MAIN_SRC = dummyMain.cpp

OBJS=$(SRC_CLNT:%.cpp=$(BUILDDIR)/%.o) 


$(info )
$(info #######################################)
$(info #    Compiling slsReceiverSoftware    #)
$(info #######################################)
$(info )


.PHONY: all intdoc  package eigerReceiver clean

all: builddir lib receiver

dummy: $(DESTDIR)/dummyReceiver

intdoc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config

$(BUILDDIR)/%.o : $(SRCDIR)/%.cpp Makefile
ifeq ($(ROOTSLS),yes)
	$(CXX) -DROOTSLS -o $@ -c $< $(INCLUDES) $(DFLAGS)  $(ROOTFLAGS) -fPIC  $(EPICSFLAGS) $(LDFLAGRXR) -L/usr/lib64/ $(FLAGS) 
else	
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) $(LDFLAGRXR) -pthread $(FLAGS) $(LIBZMQ)  -lrt 
endif

lib:  $(OBJS) $(DESTDIR)/libSlsReceiver.so $(DESTDIR)/libSlsReceiver.a 

receiver: $(DESTDIR)/slsReceiver


$(DESTDIR)/libSlsReceiver.so: $(OBJS) 
	$(CXX)  -shared -Wl,-soname,libSlsReceiver.so -o libSlsReceiver.so  $(OBJS) -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -pthread $(LIBZMQ) -lrt 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsReceiver.so $(DESTDIR)

$(DESTDIR)/libSlsReceiver.a: $(OBJS) 
	ar  rcs libSlsReceiver.a  $(OBJS) 
	mv libSlsReceiver.a $(DESTDIR)


$(DESTDIR)/slsReceiver: 	lib
	$(CXX)   -o $@  $(SRCDIR)/$(MAIN_SRC)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAGRXR) -fPIC  $(LIBZMQ) -lrt 
#$(EIGERFLAGS)


$(DESTDIR)/dummyReceiver: 	lib
	$(CXX)   -o $@  $(SRCDIR)/$(DUMMY_MAIN_SRC)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAGRXR) -fPIC $(LIBZMQ)  -lrt 
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


