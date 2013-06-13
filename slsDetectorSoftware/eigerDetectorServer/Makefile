CC			=	powerpc-4xx-softfloat-gcc
CCX			=	powerpc-4xx-softfloat-g++
CLAGS		+=  -Wall -DDACS_INT   -DEIGERD  -DSLS_DETECTOR_FUNCTION_LIST #-DVERBOSE #-DVIRTUAL
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.c slsDetectorServer_funcs.c communication_funcs.c slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.cpp=.o) 



all: clean $(PROGS)

	
boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
#	$(CC) $(SRC_CLNT) $(CLAGS) $(LDLIBS) -o $@   
	$(CC)  -c   $(SRC_CLNT) $(CLAGS) $(LDLIBS)
	$(CCX) -o $@    slsDetectorServer.o slsDetectorServer_funcs.o communication_funcs.o slsDetectorFunctionList.o 
	mv $(PROGS) $(DESTDIR)

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o
	