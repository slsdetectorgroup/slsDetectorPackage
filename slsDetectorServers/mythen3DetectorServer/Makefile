# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/
det_lib = ../../slsDetectorSoftware/include/sls/
md5_dir = ../../slsSupportLib/src/

CROSS  		= nios2-buildroot-linux-gnu-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -std=gnu99 -DMYTHEN3D  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(det_lib) -I$(current_dir)  #-DDEBUG1 #-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm -lrt -pthread
PROGS		= 	mythen3DetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS	=	slsDetectorFunctionList.c 
SRCS	+=	$(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs.c $(main_src)nios.c $(main_src)DAC6571.c $(main_src)common.c $(main_src)LTC2620_Driver.c $(main_src)ALTERA_PLL_CYCLONE10.c $(main_src)/programViaNios.c $(main_src)/sharedMemory.c $(main_src)/loadPattern.c mythen3.c $(md5_dir)md5.c

OBJS 	= 	$(SRCS:.c=.o) 
 
all: clean $(PROGS)
version: clean versioning $(PROGS)

boot: $(OBJS) 

version_name=APIMYTHEN3
version_path=slsDetectorServers/mythen3DetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; python updateAPIVersion.py $(version_name) $(version_path);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)  $(LDLIBS)  
	mv $(PROGS) $(DESTDIR)
	cp DefaultPattern_mythen3.txt $(DESTDIR)
	cp detid_mythen3.txt $(DESTDIR)
	rm $(main_src)*.o $(md5_dir)*.o

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o $(md5_dir)*.o
	

	
