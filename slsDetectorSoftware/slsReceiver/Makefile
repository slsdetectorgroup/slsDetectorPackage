CC			=	g++
CLAGS		+=  -DSLS_RECEIVER_FUNCTION_LIST -DGOTTHARDD 
LDLIBS		+= 	-lm  -lstdc++ -lpthread

INCLUDES	= 	-I ../MySocketTCP -I ../commonFiles -I .
SRC_CLNT	= 	slsReceiver.cpp ../MySocketTCP/MySocketTCP.cpp slsReceiver_funcs.cpp slsReceiverFunctionList.cpp

PROGS		= 	slsReceiver 
INSTDIR		= 	bin
INSTMODE	= 	0777

OBJS 		= 	$(SRC_CLNT:.cpp=.o) 


all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(INSTDIR) 
	gcc $(SRC_CLNT) $(INCLUDES) $(CLAGS) $(LDLIBS) -o $@   
	mv $(PROGS) $(INSTDIR)
	
	
clean:
	rm -rf $(PROGS)  *.o  $(INSTDIR)/*

