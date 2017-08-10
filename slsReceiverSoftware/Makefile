include ../Makefile.include

DESTDIR		?=	../bin
LIBDIR		?=	../bin
DOCDIR 		?= 	docs
SRCDIR		=	src
INCDIR		=   include
TESTDIR		=	test
BUILDDIR	=	build
PROGS		= 	$(DESTDIR)/slsReceiver 


CFLAGS= -g -DC_ONLY -fPIC
#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

DFLAGS= -g -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS #-DVERBOSE

INCLUDES?= $(INCLUDESRXR) -I include/ 

##############################################################
# ZMQ specific: in this case, you need ZMQ libraries already included in this package
###########################################################
LIBZMQDIR = include
LIBZMQ =  -L$(LIBZMQDIR) -Wl,-rpath=$(LIBZMQDIR) -lzmq



SRC_CLNT = MySocketTCP.cpp  ThreadObject.cpp Listener.cpp DataProcessor.cpp DataStreamer.cpp Fifo.cpp File.cpp BinaryFile.cpp UDPInterface.cpp  UDPBaseImplementation.cpp UDPStandardImplementation.cpp slsReceiverTCPIPInterface.cpp slsReceiver.cpp slsReceiverUsers.cpp utilities.cpp
DEPSINCLUDES = $(INCDIR)/ansi.h $(INCDIR)/sls_receiver_defs.h $(INCDIR)/sls_receiver_funcs.h $(INCDIR)/GeneralData.h  $(INCDIR)/circularFifo.h $(INCDIR)/genericSocket.h  $(INCDIR)/logger.h $(INCDIR)/receiver_defs.h $(INCDIR)/UDPInterface.h $(INCDIR)/utilities.h $(INCDIR)/ZmqSocket.h $(INCDIR)/BinaryFileStatic.h $(INCDIR)/HDF5FileStatic.h


ifeq ($(HDF5),yes)
	SRC_CLNT += HDF5File.cpp HDF5FileStatic.cpp
endif

ifeq ($(REST), yes)
	SRC_CLNT += UDPRESTImplementation.cpp
endif

MAIN_SRC = main.cpp


OBJS=$(SRC_CLNT:%.cpp=$(BUILDDIR)/%.o) 

$(info )
$(info #######################################)
$(info #   In slsReceiverSoftware Makefile   #)
$(info #######################################)
$(info )

ifeq ($(REST), yes)
$(info )
$(info !#####################################!)
$(info !    PLEASE ENSURE THAT VARIABLES     !)
$(info !       POCODIR and JSONBOXDIR        !)
$(info !        ARE PROPERLY SETUP!!!        !)
$(info !#####################################!)
$(info )
endif

.PHONY: all intdoc  package eigerReceiver clean

all: builddir lib receiver

dummy: $(DESTDIR)/dummyReceiver

intdoc: $(SRC_H) $(SRC_CLNT) 
	doxygen doxy.config

$(BUILDDIR)/%.o : $(SRCDIR)/%.cpp $(INCDIR)/%.h $(DEPSINCLUDES) Makefile
ifeq ($(ROOTSLS),yes)
	$(CXX) -DROOTSLS -o $@ -c $< $(INCLUDES) $(DFLAGS)  $(ROOTFLAGS) -fPIC  $(EPICSFLAGS) $(LDFLAGRXR) -L/usr/lib64/ $(FLAGS) 
else	
#	$(call colorecho,`./updateGitVersion.sh`)
	$(CXX) -o $@ -c $< $(INCLUDES) $(DFLAGS) -fPIC  $(EPICSFLAGS) $(LDFLAGRXR) -pthread $(FLAGS) $(LIBZMQ)  -lrt 
endif

versioning: 
	$(call colorecho,`./updateGitVersion.sh`)

lib:  versioning $(OBJS) $(DESTDIR)/libSlsReceiver.so $(DESTDIR)/libSlsReceiver.a 

receiver: $(DESTDIR)/slsReceiver

$(DESTDIR)/libSlsReceiver.so: $(OBJS)  
	$(call colorecho,"#######################################")
	$(call colorecho,"#    Compiling slsReceiver Library    #")
	$(call colorecho,"#######################################")
	$(CXX)  -shared -Wl,-soname,libSlsReceiver.so -o libSlsReceiver.so  $(OBJS) -lc $(INCLUDES) $(DFLAGS) $(FLAGS) $(EPICSFLAGS)  -L/usr/lib64 -pthread $(LIBZMQ) -lrt 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	mv libSlsReceiver.so $(DESTDIR)

$(DESTDIR)/libSlsReceiver.a: $(OBJS) 
	ar  rcs libSlsReceiver.a  $(OBJS) 
	mv libSlsReceiver.a $(DESTDIR)


$(DESTDIR)/slsReceiver: 	 $(SRCDIR)/$(MAIN_SRC) $(DESTDIR)/libSlsReceiver.so $(DESTDIR)/libSlsReceiver.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#        Compiling slsReceiver        #")
	$(call colorecho,"#######################################")
	$(CXX)   -o $@  $(SRCDIR)/$(MAIN_SRC)  $(FLAGS)  $(INCLUDES) $(CFLAGS) $(LIBS) $(LDFLAGRXR) -fPIC  $(LIBZMQ) -lrt 
#$(EIGERFLAGS)


$(DESTDIR)/dummyReceiver: 	$(SRCDIR)/$(DUMMY_MAIN_SRC) $(DESTDIR)/libSlsReceiver.so $(DESTDIR)/libSlsReceiver.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#     Compiling Dummy slsReceiver     #")
	$(call colorecho,"#######################################")
	$(CXX)   -o $@  $(SRCDIR)/$(DUMMY_MAIN_SRC)  $(FLAGS)  $(INCLUDES) $(CFLAGS) $(LIBS) $(LDFLAGRXR) -fPIC $(LIBZMQ)  -lrt 
#$(EIGERFLAGS)


clean: buildclean
	make testclean
	if test -e $(DESTDIR)/libSlsReceiver.a; then rm $(DESTDIR)/libSlsReceiver.a;fi
	if test -e $(DESTDIR)/libSlsReceiver.so; then rm $(DESTDIR)/libSlsReceiver.so;fi
	if test -e $(PROGS); then rm $(PROGS);fi

builddir:
	mkdir -p $(BUILDDIR)
#	if [ ! -d $(BUILDDIR) ]; then mkdir $(BUILDDIR); fi

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


