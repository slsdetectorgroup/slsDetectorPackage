include ../../Makefile.include

DESTDIR		?=	../../bin
LIBDIR		?=	$(DESTDIR)
PROGS		= 	$(DESTDIR)/slsReceiver 


CFLAGS		+=  	-DSLS_RECEIVER_UDP_FUNCTIONS -O3 
CPPFLAGS	= 	${CFLAGS}   # for MAC


LDFLAGRXR 		?=	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -lpthread
LDFLAGRXR 		+=	-lm  -lstdc++ 


INCLUDES	?= 	-I ../MySocketTCP  -I ../slsDetectorCalibration -I ../includes -I eigerReceiver -I . -I ../slsDetectorSoftware/commonFiles
SRC_CLNT	= 	main.cpp 


INSTMODE	= 	0777
OBJS 		= 	$(SRC_CLNT:.cpp=.o)


.PHONY: all receiver clean static_receiver boot eigerReceiver lib

all: receiver

receiver: $(DESTDIR)/slsReceiver

static_receiver: $(DESTDIR)/sslsReceiver

boot: $(OBJS)

$(DESTDIR)/sslsReceiver: lib
	echo $(OBJS)
	echo $(LDFLAGRXR)
	echo $(LIBS)
	mkdir -p $(DESTDIR) 
	$(CXX) -static  -o $@  $(SRC_CLNT)  $(FLAGS)  $(INCLUDES) $(CLAGS)  $(LIBS) $(LDFLAGRXR)


$(DESTDIR)/slsReceiver: eigerReceiver	lib
	$(CXX)   -o $@  $(SRC_CLNT)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAGRXR) -fPIC 
#$(EIGERFLAGS) 	


ifeq ($(EIGERSLS), yes)
eigerReceiver: 
	$(CXX) $(FLAGS) $(CFLAGS) $(INCLUDES) -fPIC -c -o eigerReceiverTest.o eigerReceiver/eigerReceiverTest.cpp $(EIGERFLAGS)
	$(CXX) $(FLAGS) $(CFLAGS) $(INCLUDES) -fPIC -c -o eigerReceiver.o eigerReceiver/eigerReceiver.cpp $(EIGERFLAGS)
	$(CXX)  eigerReceiverTest.o eigerReceiver.o   -o eigerReceiver/eigerReceiverTest $(EIGERFLAGS)
else ifeq ($(ROOTSLS), yes)
eigerReceiver: eigerReceiver/eigerReceiverDummy.cpp 
	echo "Compiling with root"
	$(CXX) $(FLAGS) $(CFLAGS) $(INCLUDES) -fPIC -c -o eigerReceiver.o eigerReceiver/eigerReceiverDummy.cpp $(ROOTFLAGS)
else
eigerReceiver: eigerReceiver/eigerReceiverDummy.cpp
	$(CXX) $(FLAGS) $(CFLAGS) $(INCLUDES) -fPIC -c -o eigerReceiver.o eigerReceiver/eigerReceiverDummy.cpp 	
endif

lib:
	cd ../ && $(MAKE) DESTDIR=../bin LIBDIR=../bin

clean:
	rm -rf $(PROGS)  *.o  eigerReceiverTest $(DESTDIR)/libSlsReceiver.a   $(DESTDIR)/libSlsReceiver.so core



