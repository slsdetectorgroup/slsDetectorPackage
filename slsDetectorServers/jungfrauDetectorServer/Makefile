current_dir = $(shell pwd)
main_server = ../slsDetectorServer/
support_lib = ../../slsSupportLib/include/

CROSS  		= bfin-uclinux-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -DJUNGFRAUD  -DSTOP_SERVER -I$(main_server) -I$(support_lib) -I$(current_dir)#-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm
PROGS		= 	jungfrauDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	$(main_server)communication_funcs.c $(main_server)slsDetectorServer.c $(main_server)slsDetectorServer_funcs.c  slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.c=.o) 
 
all: clean versioning $(PROGS)

boot: $(OBJS) 

version_name=APIJUNGFRAU
version_path=slsDetectorServers/jungfrauDetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)  $(LDLIBS)  
	mv $(PROGS) $(DESTDIR)
	rm *.gdb

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb
	

	