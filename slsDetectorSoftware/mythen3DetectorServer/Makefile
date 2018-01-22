CROSS  = bfin-uclinux-
CC    = $(CROSS)gcc
#CC			=	gcc
CLAGS		+=  -Wall -DVIRTUAL  -DDACS_INT  -DMYTHEN3D -DVERBOSE   -DSLS_DETECTOR_FUNCTION_LIST #-DGENERICD
LDLIBS		+= 	-lm  
INCS          = -I. -I../../slsReceiverSoftware/include/ -I../commonFiles
PROGS		= 	mythen3Server
#DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.c mythen3Server_funcs.c ../commonFiles/communication_funcs.c  mythen3FunctionList.c 
OBJS 		= 	$(SRC_CLNT:.c=.o) 



all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
#	mkdir -p $(DESTDIR) 
	$(CC) $(SRC_CLNT) $(CLAGS) $(LDLIBS) $(INCS) -o $@   
#	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(PROGS)  *.o 
