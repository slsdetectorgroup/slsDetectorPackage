CC			=	g++#powerpc-4xx-softfloat-g++
CLAGS		+=  -Wall -DVIRTUAL  -DDACS_INT  -DSLS_DETECTOR_FUNCTION_LIST -DEIGER
LDLIBS		+= 	-lm  

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.c slsDetectorServer_funcs.c communication_funcs.c slsDetector_firmware.c slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.cpp=.o) 



all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CC) $(SRC_CLNT) $(CLAGS) $(LDLIBS) -o $@   
	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
