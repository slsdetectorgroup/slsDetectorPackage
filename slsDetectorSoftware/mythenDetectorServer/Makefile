# $Id: Makefile,v 1.1.1.1 2006/02/04 03:35:01 freza Exp $
# first compile
# make cris-axis-linux-gnu

AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/Rules.axis

PROGS= 		mythenDetectorServer 

INSTDIR= 	$(prefix)/bin
INSTMODE= 	0777

SRCS= 		 server.c server_funcs.c communication_funcs.c firmware_funcs.c mcb_funcs.c trimming_funcs.c  sharedmemory.c
OBJS= 	$(SRCS:%.c=%.o) 

VFLAGS= 
#-DVERBOSE 
#-DVERYVERBOSE
CFLAGS+= 	-Wall -DC_ONLY -DMCB_FUNCS -DDACS_INT $(VFLAGS)
#-Werror 

LDLIBS+= 	-lm

mythen:	clean $(PROGS)
all:	$(PROGS) 

boot: $(OBJS)

$(PROGS): $(OBJS)
	echo $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(CFLAGS) -o $@

install:	$(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) $(PROGS) $(INSTDIR)

clean:
	rm -rf $(PROGS) $(PICASSO) *.o

depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS) 2>/dev/null
