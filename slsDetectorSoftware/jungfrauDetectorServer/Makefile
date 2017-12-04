# $Id: Makefile,v 1.1.1.1 2006/02/04 03:35:01 freza Exp $
# first compile
# make cris-axis-linux-gnu


CROSS  = bfin-uclinux-
CC    = $(CROSS)gcc

CFLAGS += -Wall -DJUNGFRAUD -DMCB_FUNCS -DDACS_INT  -DDEBUG -DV1 #-DVERBOSE #-DVERYVERBOSE  #-DVIRTUAL 


PROGS= 	 jungfrauDetectorServer
INSTDIR= 	/tftpboot
INSTMODE= 	0777



BINS = testlib_sharedlibc
SRCS = server.c server_funcs.c communication_funcs.c firmware_funcs.c mcb_funcs.c
OBJS = $(SRCS:%.c=%.o) 



all:	clean $(PROGS) 

test: 	clean jungfrauADCTEst

boot: $(OBJS)

jungfrauDetectorServer: $(OBJS)
	echo $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS_$@) $(LDFLAGS_$@)


jungfrauADCTEst: $(OBJS)
	echo $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS_$@) $(LDFLAGS_$@) -DTESTADC 




install:	$(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) $(PROGS) $(INSTDIR)


romfs:
	$(ROMFSINST)	/bin/$(PROGS) 

clean:
	rm -rf $(PROGS)  *.o *.gdb






