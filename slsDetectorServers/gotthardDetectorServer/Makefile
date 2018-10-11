CROSS  = bfin-uclinux-
CC    = $(CROSS)gcc

CFLAGS += -Wall  -DGOTTHARDD   -DDEBUG   # -DVERBOSE #-DVERYVERBOSE  #-DVIRTUAL

PROGS= 		gotthardDetectorServer 
INSTDIR= 	/tftpboot
INSTMODE= 	0777


BINS = testlib_sharedlibc
SRCS = server.c firmware_funcs.c  server_funcs.c communication_funcs.c
OBJS = $(SRCS:%.c=%.o)


all:	clean versioning $(PROGS) 

boot: $(OBJS)

versioning: 
	@echo `tput setaf 6;	./updateGitVersion.sh; tput sgr0;`
	
$(PROGS): $(OBJS) 
	echo $(OBJS)
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS_$@) $(LDFLAGS_$@) 
	rm gotthardDetectorServer.gdb


install:	$(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) $(PROGS) $(INSTDIR)


romfs:
	$(ROMFSINST)	/bin/$(PROGS) 

clean:
	rm -rf $(PROGS)  *.o *.gdb





