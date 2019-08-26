current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/

CROSS  		= bfin-uclinux-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -DMOENCHD  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(current_dir)#-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm  
PROGS		= 	moenchDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS 		= 	slsDetectorFunctionList.c 
SRCS		+=	$(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs.c $(main_src)blackfin.c $(main_src)common.c $(main_src)commonServerFunctions.c $(main_src)communication_funcs_UDP.c $(main_src)UDPPacketHeaderGenerator.c $(main_src)AD9257.c $(main_src)ALTERA_PLL.c $(main_src)LTC2620.c $(main_src)MAX1932.c $(main_src)programfpga.c

OBJS 		= 	$(SRCS:.c=.o) 

all: clean versioning $(PROGS)

boot: $(OBJS) 

version_name=APIMOENCH
version_path=slsDetectorServers/moenchDetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	rm *.gdb
	rm $(main_src)*.o

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o
	

	