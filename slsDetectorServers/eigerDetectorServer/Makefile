current_dir = $(shell pwd)
main_server = ../slsDetectorServer/
support_lib = ../../slsSupportLib/include/

BLACKFIN_CC = bfin-uclinux-gcc
CC    		= powerpc-4xx-softfloat-gcc
CFLAGS		+=  -Wall -DEIGERD  -DSTOP_SERVER -I$(main_server) -I$(support_lib) -I$(current_dir)#-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm  
PROGS		= 	eigerDetectorServer 
DESTDIR		= 	bin
INSTMODE	= 	0777

SRCS 		= 	$(main_server)communication_funcs.c $(main_server)slsDetectorServer.c $(main_server)slsDetectorServer_funcs.c  slsDetectorFunctionList.c HardwareIO.c LocalLinkInterface.c  FebInterface.c FebControl.c Beb.c 
OBJS 		= 	$(SRCS:.c=.o) 

all: clean $(PROGS) #hv9m_blackfin_server 
#all: clean   versioning  $(PROGS) #hv9m_blackfin_server 

	
boot: $(OBJS) 

version_name=APIEIGER
version_path=slsDetectorServers/eigerDetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path);  tput sgr0;`


$(PROGS):  $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC)  	-o $@  $^ $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	

hv9m_blackfin_server:9mhvserial_bf.c
	$(BLACKFIN_CC) -o hv9m_blackfin_server 9mhvserial_bf.c -Wall #-DVERBOSE
	mv hv9m_blackfin_server $(DESTDIR)	
	rm hv9m_blackfin_server.gdb  $(main_server)*.o

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o $(DESTDIR)/hv9m_blackfin_server  $(main_server)*.o
	
	
