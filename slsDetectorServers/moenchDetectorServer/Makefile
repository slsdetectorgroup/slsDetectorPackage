# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
current_dir = $(shell pwd)
main_inc = ../slsDetectorServer/include/
main_src = ../slsDetectorServer/src/
support_lib = ../../slsSupportLib/include/
md5_dir = ../../slsSupportLib/src/

CROSS  		= bfin-uclinux-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall -std=gnu99 -DMOENCHD  -DSTOP_SERVER -I$(main_inc) -I$(support_lib) -I$(current_dir)  #-DVERBOSEI #-DVERBOSE 
LDLIBS		+= 	-lm -lrt -pthread
PROGS		= 	moenchDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS	=	slsDetectorFunctionList.c 
SRCS	+=	$(main_src)slsDetectorServer.c $(main_src)slsDetectorServer_funcs.c $(main_src)communication_funcs.c $(main_src)blackfin.c $(main_src)common.c $(main_src)commonServerFunctions.c  $(main_src)AD9257.c $(main_src)ALTERA_PLL.c $(main_src)LTC2620.c $(main_src)MAX1932.c $(main_src)programViaBlackfin.c $(main_src)/sharedMemory.c $(md5_dir)md5.c

OBJS 	= 	$(SRCS:.c=.o) 
 
all: clean $(PROGS)
version: clean versioning $(PROGS)

boot: $(OBJS) 

version_name=APIMOENCH
version_path=slsDetectorServers/moenchDetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; python updateAPIVersion.py $(version_name) $(version_path);  tput sgr0;`


$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)  $(LDLIBS)  
	mv $(PROGS) $(DESTDIR)
	cp detid_moench.txt $(DESTDIR)
	rm *.gdb
	rm $(main_src)*.o $(md5_dir)*.o

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o $(md5_dir)*.o
	

	