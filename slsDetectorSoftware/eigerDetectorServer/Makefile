CC			=	powerpc-4xx-softfloat-gcc
CCX			=	powerpc-4xx-softfloat-g++
CFLAGS		+=  -Wall -DDACS_INT   -DEIGERD  -DSLS_DETECTOR_FUNCTION_LIST  -DDACS_INT #-DSTOP_SERVER #-DVERBOSE #-DVIRTUAL
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	communication_funcs.c slsDetectorServer.c slsDetectorServer_funcs.c  slsDetectorFunctionList.c 
SRC_CLNT2   =   Eiger.cxx HardwareIO.cxx LocalLinkInterface.cxx Feb.cxx
OBJS 		= 	$(SRC_CLNT:.c=.o) 
OBJS2 		= 	$(SRC_CLNT2:.cpp=.o)


all: clean $(PROGS)

	
boot: $(OBJS) $(OBJS2)

$(PROGS): 
	echo $(OBJS) $(OBJS2)
	mkdir -p $(DESTDIR) 
	#if you use g++ for pc, uncomment swapData in SendData() and ReceiveData() in communication_funcs.c
	$(CCX)  -o $@  $(SRC_CLNT) $(SRC_CLNT2) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o
	