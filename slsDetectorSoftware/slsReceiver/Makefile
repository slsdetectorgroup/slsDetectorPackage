CC			=	g++ 
CFLAGS		+=  -DSLS_RECEIVER_FUNCTION_LIST -O3 
CPPFLAGS    = ${CFLAGS}   # for MAC
LDFLAG=   -L/usr/lib64/ -lpthread -lm  -lstdc++ 

POCODIR = /afs/psi.ch/user/s/sala/public/poco-slp_6.4-64bit
JSONBOXDIR = /afs/psi.ch/user/s/sala/public/JsonBox-slp_6.4-64bit
EIGERFLAGS = -L $(JSONBOXDIR) -L $(POCODIR)/lib -Wl,-rpath=$(POCODIR)/lib -I $(POCODIR)/include -I $(JSONBOXDIR)/include 

#LDLIBS		+= 	-lm  -lstdc++ -lpthread 
LIBS?= -L$(LIBDIR) -lSlsDetector 

INCLUDES	?= 	-I ../MySocketTCP -I ../commonFiles -I  -I ../../../slsDetectorCalibration -I . 
SRC_CLNT	= 	slsReceiver.cpp 
#../MySocketTCP/MySocketTCP.cpp  slsReceiver_funcs.cpp UHRIXCallback.cpp slsReceiverFunctionList.cpp slsReceiverUsers.cpp 

DESTDIR		?= 	bin
INSTMODE	= 	0777

OBJS 		= 	$(SRC_CLNT:.cpp=.o) 

PROGS		= 	$(DESTDIR)/slsReceiver 

all: receiver

receiver: $(DESTDIR)/slsReceiver

static_receiver: $(DESTDIR)/sslsReceiver


boot: $(OBJS)

$(DESTDIR)/sslsReceiver: lib
	echo $(OBJS)
	echo $(LDFLAG)
	echo $(LIBS)
	mkdir -p $(DESTDIR) 
	$(CXX) -static  -o $@  $(SRC_CLNT)  $(FLAGS)  $(INCLUDES) $(CLAGS)  $(LIBS) $(LDFLAG)


$(DESTDIR)/slsReceiver: lib
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CXX)   -o $@  $(SRC_CLNT)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAG) 	

eigerReceiver:
	$(CXX) $(FLAGS) $(CFLAGS) $(EIGERFLAGS) -c -o eigerReceiverTest.o eigerReceiverTest.cpp -lPocoNet -lPocoFoundation -lJsonBox
	$(CXX) $(FLAGS) $(CFLAGS) $(EIGERFLAGS) -c -o eigerReceiver.o eigerReceiver.cpp -lPocoNet -lPocoFoundation -lJsonBox
	$(CXX) $(EIGERFLAGS) eigerReceiverTest.o eigerReceiver.o   -o eigerReceiverTest -lPocoNet -lPocoFoundation -lJsonBox

lib:
	cd ../ && $(MAKE) DESTDIR=$(LIBDIR) 

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o  eigerReceiverTest

# EigerDataReceiver
eigerReceiverTest.o: eigerReceiver.h
eigerReceiver.o    : eigerReceiver.h
eigerReceiverTest: eigerReceiver.o eigerReceiverTest.o
#EOF

