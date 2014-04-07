include ../../Makefile.include

DESTDIR		?=	../../bin
LIBDIR		?=	$(DESTDIR)
PROGS		= 	$(DESTDIR)/slsReceiver 


CFLAGS		+=  	-DSLS_RECEIVER_FUNCTION_LIST -O3 
CPPFLAGS	= 	${CFLAGS}   # for MAC
LDFLAG 		?=	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -lpthread
LDFLAG 		+=	-lm  -lstdc++ 



INCLUDES	?= 	-I ../MySocketTCP -I ../commonFiles -I ../../slsDetectorCalibration -I . 
SRC_CLNT	= 	slsReceiver.cpp 
#../MySocketTCP/MySocketTCP.cpp  slsReceiver_funcs.cpp UHRIXCallback.cpp slsReceiverFunctionList.cpp slsReceiverUsers.cpp 


INSTMODE	= 	0777
OBJS 		= 	$(SRC_CLNT:.cpp=.o)


.PHONY: all receiver clean static_receiver boot eigerReceiver lib

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


$(DESTDIR)/slsReceiver: eigerReceiver	lib
	$(CXX)   -o $@  $(SRC_CLNT)  $(FLAGS)  $(INCLUDES) $(CLAGS) $(LIBS) $(LDFLAG) -fPIC
#$(EIGERFLAGS) 	


ifeq ($(EIGERSLS), yes)
eigerReceiver: 
	$(CXX) $(FLAGS) $(CFLAGS) -fPIC -c -o eigerReceiverTest.o eigerReceiverTest.cpp $(EIGERFLAGS)
	$(CXX) $(FLAGS) $(CFLAGS) -fPIC -c -o eigerReceiver.o eigerReceiver.cpp $(EIGERFLAGS)
	$(CXX)  eigerReceiverTest.o eigerReceiver.o   -o eigerReceiverTest $(EIGERFLAGS)
else
eigerReceiver: eigerReceiverDummy.cpp
	echo "Compiling dummy EigerReceiver"
	$(CXX) $(FLAGS) $(CFLAGS) -fPIC -c -o eigerReceiver.o eigerReceiverDummy.cpp 
endif

lib:
	cd ../ && $(MAKE) DESTDIR=../bin LIBDIR=../bin

clean:
	rm -rf $(PROGS)  *.o  eigerReceiverTest
	rm -rf  $(DESTDIR)/libSlsDetector.a   $(DESTDIR)/libSlsDetector.so core
	


