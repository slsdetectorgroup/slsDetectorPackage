current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/sls/
md5_dir = ../../slsSupportLib/opensslMd5/

BLACKFIN_CC = bfin-uclinux-gcc
CROSS		= powerpc-4xx-softfloat-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -std=gnu99 -DEIGERD  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(current_dir)  -I$(md5_dir)#-DDEBUG1 #-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm -lrt -pthread
PROGS		= 	eigerDetectorServer 
DESTDIR		= 	bin
INSTMODE	= 	0777

SRCS 		= 	slsDetectorFunctionList.c HardwareIO.c LocalLinkInterface.c  FebInterface.c FebControl.c Beb.c 
SRCS		+=	$(main_src)communication_funcs.c $(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs_UDP.c $(main_src)common.c $(main_src)/sharedMemory.c $(md5_dir)md5.c

OBJS 		= 	$(SRCS:.c=.o) 

all: clean $(PROGS) #hv9m_blackfin_server 
version: clean versioning $(PROGS) #hv9m_blackfin_server 
	
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
	cp config_eiger.txt $(DESTDIR)
	cp detid.txt $(DESTDIR)
	

hv9m_blackfin_server:9mhvserial_bf.c
	$(BLACKFIN_CC) -o hv9m_blackfin_server 9mhvserial_bf.c -Wall #-DVERBOSE
	mv hv9m_blackfin_server $(DESTDIR)	
	rm hv9m_blackfin_server.gdb  $(main_server)*.o

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o $(DESTDIR)/hv9m_blackfin_server  $(main_server)*.o
	
	
