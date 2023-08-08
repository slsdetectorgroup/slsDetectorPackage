# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/
det_lib = ../../slsDetectorSoftware/include/sls/
md5_dir = ../../slsSupportLib/src/

#CROSS  		= bfin-uclinux-
CROSS  		= 
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -std=gnu99 -DXILINX_CHIPTESTBOARDD  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(det_lib) -I$(current_dir) #-DDEBUG1 #-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm -lrt -pthread
PROGS		= 	xilinx_ctbDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS 		= 	slsDetectorFunctionList.c  
SRCS		+=	$(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs.c $(main_src)arm.c $(main_src)common.c $(main_src)/sharedMemory.c $(md5_dir)md5.c

OBJS 		= 	$(SRCS:.c=.o) 
 
all: clean $(PROGS)
version: clean versioning $(PROGS)

boot: $(OBJS) 

version_branch=$(API_BRANCH)
version_name=APIXILINXCTB
version_path=slsDetectorServers/xilinx_ctbDetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path) $(version_branch);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	rm $(main_src)*.o $(md5_dir)*.o
clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o $(md5_dir)*.o
	

	
