CROSS  = bfin-uclinux-
CC    = $(CROSS)gcc
#CC			=	gcc
CLAGS		+=  -Wall -DVIRTUAL  -DDACS_INT  -DGENERICD # -DSLS_DETECTOR_FUNCTION_LIST 
LDLIBS		+= 	-lm  
INCS          = -I. -I../../slsReceiverSoftware/include/
PROGS		= 	mythen3Server
#DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.c slsDetectorServer_funcs.c communication_funcs.c  slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.c=.o) 



all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
#	mkdir -p $(DESTDIR) 
	$(CC) $(SRC_CLNT) $(CLAGS) $(LDLIBS) $(INCS) -o $@   
#	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
