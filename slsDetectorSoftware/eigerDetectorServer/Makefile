CC			=	powerpc-4xx-softfloat-gcc
BLACKFIN_CC = 	bfin-uclinux-gcc
CFLAGS		+=  -Wall -DDACS_INT  -DEIGERD  -DSLS_DETECTOR_FUNCTION_LIST  -DDACS_INT -DSTOP_SERVER #-DVERBOSEI #-DVERBOSE #-DVIRTUAL -DPCCOMPILE -DMARTIN
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777


SRC_CLNT	=	communication_funcs.c slsDetectorServer.c slsDetectorServer_funcs.c  slsDetectorFunctionList.c FebControl.c Beb.c HardwareIO.c LocalLinkInterface.c Feb.c  FebInterface.c 
OBJS 		= 	$(SRC_CLNT:.c=.o) 
 

all: clean $(PROGS) #hv9m_blackfin_server 

	
boot: $(OBJS) 

$(PROGS): 
	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC)  	-o $@  $(SRC_CLNT) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	

hv9m_blackfin_server:9mhvserial_bf.c
	$(BLACKFIN_CC) -o hv9m_blackfin_server 9mhvserial_bf.c -Wall #-DVERBOSE
	mv hv9m_blackfin_server $(DESTDIR)	
	rm hv9m_blackfin_server.gdb

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o $(DESTDIR)/hv9m_blackfin_server 
	
