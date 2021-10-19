# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
current_dir = $(shell pwd)

CROSS  		= nios2-buildroot-linux-gnu-
CC    		= $(CROSS)gcc
CFLAGS		+=  -Wall #-DDEBUG 
PROGS		= 	stripd 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS	=	stripd.c 

OBJS 	= 	$(SRCS:.c=.o) 
 
all: clean $(PROGS)
version: clean versioning $(PROGS)

boot: $(OBJS) 

$(PROGS): $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)  $(LDLIBS)  
	mv $(PROGS) $(DESTDIR)
	rm $(main_src)*.o	

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o *.gdb $(main_src)*.o
	

	
