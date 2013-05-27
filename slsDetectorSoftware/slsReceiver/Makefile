CC			=	g++ 
CLAGS		+=  -DSLS_RECEIVER_FUNCTION_LIST  
LDFLAG=   -L/usr/lib64/ -lpthread -lm  -lstdc++ 


#LDLIBS		+= 	-lm  -lstdc++ -lpthread 
LIBS?= -L$(LIBDIR) -lSlsDetector 

INCLUDES	?= 	-I ../MySocketTCP -I ../commonFiles -I -I ../slsDetectorAnalysis -I .
SRC_CLNT	= 	slsReceiver.cpp 
#../MySocketTCP/MySocketTCP.cpp slsReceiver_funcs.cpp UHRIXCallback.cpp slsReceiverFunctionList.cpp slsReceiverUsers.cpp 

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



lib:
	cd ../ && $(MAKE) DESTDIR=$(LIBDIR) 

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
