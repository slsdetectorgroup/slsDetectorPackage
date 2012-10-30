CFLAGS+= 	-Wall -DC_ONLY  -DSLS_RECEIVER_FUNCTION_LIST -DGOTTHARDD #-DVERBOSE
LDLIBS+= 	-lm  -lstdc++ -lpthread
PROGS= 		slsReceiver 
INSTDIR= 	bin
INSTMODE= 	0777

SRC_CLNT = 	slsReceiver.c communication_funcs.c slsReceiverFunctionList.c slsReceiver_funcs.c 
OBJS = 		$(SRC_CLNT:.c=.o) 


all:	clean $(PROGS) 

boot: $(OBJS)

#doc: $(SRC_CLNT)
#	doxygen doxy.config

#%.o : %.c %.h
#	$(CXX) -Wall -o $@ -c $< $(INCLUDES) $(CFLAGS) $(FLAGS) $(LDLIBS)  -fPIC 



$(PROGS): 
	echo $(OBJS)
	mkdir -p $(INSTDIR) 
	$(CC) $(SRC_CLNT) $(LDFLAGS) $(LDLIBS) $(CFLAGS) -o $@   
	mv $(PROGS) $(INSTDIR)


clean:
	rm -rf $(PROGS)  *.o  $(INSTDIR)/*





