current_dir = $(shell pwd)
main_server = ../slsDetectorServer/
support_lib = ../../slsSupportLib/include/

CROSS  		= bfin-uclinux-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -DMOENCHD  -DSTOP_SERVER -I$(main_server) -I$(support_lib) -I$(current_dir)#-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm  
PROGS		= 	moenchDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS 		= 	$(main_server)communication_funcs.c $(main_server)slsDetectorServer.c $(main_server)slsDetectorServer_funcs.c  slsDetectorFunctionList.c 
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

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_server)*.o
	

	