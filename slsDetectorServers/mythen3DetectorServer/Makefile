current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/

CROSS  		= nios2-buildroot-linux-gnu-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -DMYTHEN3D  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(current_dir) -DDEBUG1 #-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm
PROGS		= 	mythen3DetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS	=	slsDetectorFunctionList.c 
SRCS	+=	$(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs.c $(main_src)nios.c $(main_src)DAC6571.c $(main_src)common.c $(main_src)LTC2620_Driver.c

OBJS 	= 	$(SRCS:.c=.o) 
 
#all: clean $(PROGS)
all: clean versioning $(PROGS)

boot: $(OBJS) 

version_name=APIMYTHEN3
version_path=slsDetectorServers/mythen3DetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)  $(LDLIBS)  
	mv $(PROGS) $(DESTDIR)
	rm $(main_src)*.o

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o
	

	