CC			=	powerpc-4xx-softfloat-gcc
CCX			=	powerpc-4xx-softfloat-g++
CFLAGS		+=  -Wall -DDACS_INT  -DEIGERD  -DSLS_DETECTOR_FUNCTION_LIST  -DDACS_INT -DSTOP_SERVER  #-DVERBOSE #-DVIRTUAL -DPCCOMPILE -DMARTIN
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777


SRC_CLNT	=	communication_funcs.c slsDetectorServer.c slsDetectorServer_funcs.c  slsDetectorFunctionList.c FebControl.c Beb.c HardwareIO.c LocalLinkInterface.c Feb.c  FebInterface.c 
#SRC_CLNT2   =	FebServer.cxx FebControl.cxx FebInterface.cxx LocalLinkInterface.cxx HardwareIO.cxx
#SRC_CLNT3   =	BebServer.cxx Beb.cxx LocalLinkInterface.cxx HardwareIO.cxx

OBJS 		= 	$(SRC_CLNT:.c=.o) 
 

all: clean $(PROGS) #feb_debug beb_debug

	
boot: $(OBJS) 

$(PROGS): 
	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC)  	-o $@  $(SRC_CLNT) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	
feb_debug:$(SRC_CLNT2)
	$(CCX)  -o feb_debug  $(SRC_CLNT2) -I.
	mv feb_debug $(DESTDIR)
	
beb_debug:$(SRC_CLNT3)
	$(CCX)  -o beb_debug  $(SRC_CLNT3) -I.
	mv beb_debug $(DESTDIR)	

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o
	
