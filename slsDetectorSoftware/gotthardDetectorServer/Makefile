# $Id: Makefile,v 1.1.1.1 2006/02/04 03:35:01 freza Exp $
# first compile
# make cris-axis-linux-gnu


CROSS  = bfin-uclinux-
CC    = $(CROSS)gcc

CFLAGS += -Wall  -DMCB_FUNCS  -DVERBOSE #-DVERY_VERBOSE #-DVIRTUAL

#INCLUDES= /usr/src/kernels/2.6.18-238.12.1.el5-i686/include
#/home/l_maliakal_d/bfin/blackfin-linux-dist/linux-2.6.x/include
#/usr/include/asm/page.h


PROGS= 		gotthardDetectorServer 

INSTDIR= 	/tftpboot
INSTMODE= 	0777



BINS = testlib_sharedlibc
SRCS = server.c server_funcs.c communication_funcs.c firmware_funcs.c mcb_funcs.c trimming_funcs.c  sharedmemory.c 
OBJS = $(SRCS:%.c=%.o) 


#LDLIBS+= 	-lm 



all:	clean $(PROGS) 

boot: $(OBJS)

$(PROGS): $(OBJS)
	echo $(OBJS)
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS_$@) $(LDFLAGS_$@)


install:	$(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) $(PROGS) $(INSTDIR)


#CFLAGS+= 	-Wall -DC_ONLY -DMCB_FUNCS 
#-DVERBOSE 
#-DVERYVERBOSE 
#-Werror


clean:
	rm -rf $(PROGS)  *.o






