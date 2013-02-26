CC			=	g++ 
	CLAGS		+=  -DSLS_RECEIVER_FUNCTION_LIST -DGOTTHARDD  #-DTESTWRITE
LDLIBS		+= 	-lm  -lstdc++ -lpthread 

INCLUDES	= 	-I ../MySocketTCP -I ../commonFiles -I ../usersFunctions -I ../slsDetectorAnalysis -I .
SRC_CLNT	= 	slsReceiver.cpp ../MySocketTCP/MySocketTCP.cpp slsReceiver_funcs.cpp slsReceiverFunctionList.cpp ../usersFunctions/usersFunctions.cpp

PROGS		= 	slsReceiver 
DESTDIR		?= 	bin
INSTMODE	= 	0777

OBJS 		= 	$(SRC_CLNT:.cpp=.o) 


all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	gcc $(SRC_CLNT) $(INCLUDES) $(CLAGS) $(LDLIBS) -o $@   
	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
