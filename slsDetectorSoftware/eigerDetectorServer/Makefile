CC			=	powerpc-4xx-softfloat-g++
CLAGS		+=  -Wall  -DDACS_INT  -DSLS_DETECTOR_FUNCTION_LIST -DEIGERD #-DVIRTUAL  
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.cpp
SRC_CLNTC	=   slsDetectorServer_funcs.c communication_funcs.c slsDetector_firmware.c slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.cpp=.o) $(SRC_CLNTC:%.c=%.o) 



all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CC) $(CLAGS) $(SRC_CLNT) $(SRC_CLNTC) $(LDLIBS) -o $@   
	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
