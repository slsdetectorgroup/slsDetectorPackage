current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/sls/
md5_dir = ../../slsSupportLib/opensslMd5/

CROSS  		= nios2-buildroot-linux-gnu-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -std=gnu99 -DGOTTHARD2D  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(current_dir)  -I$(md5_dir) #-DDEBUG1 #-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm -lrt -pthread
PROGS		= 	gotthard2DetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS	=	slsDetectorFunctionList.c 
SRCS	+=	$(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs.c $(main_src)nios.c $(main_src)common.c $(main_src)DAC6571.c $(main_src)LTC2620_Driver.c $(main_src)ALTERA_PLL_CYCLONE10.c $(main_src)ASIC_Driver.c $(main_src)/programFpgaNios.c $(main_src)/sharedMemory.c $(md5_dir)md5.c

OBJS 	= 	$(SRCS:.c=.o) 
 
all: clean $(PROGS)
version: clean versioning $(PROGS)

boot: $(OBJS) 

version_name=APIGOTTHARD2
version_path=slsDetectorServers/gotthard2DetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)  $(LDLIBS)  
	mv $(PROGS) $(DESTDIR)
	cp config_gotthard2.txt $(DESTDIR)
	cp detid_gotthard2.txt $(DESTDIR)
	rm $(main_src)*.o	

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o
	

	