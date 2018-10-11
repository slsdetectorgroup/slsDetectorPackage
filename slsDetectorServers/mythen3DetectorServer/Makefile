CROSS  = bfin-uclinux-
CC    = $(CROSS)gcc
#CC			=	gcc
CLAGS		+=  -Wall -DMYTHEN3D -DSLS_DETECTOR_FUNCTION_LIST -DDACS_INT -DSTOP_SERVER #-DVERBOSEI #-DVERBOSE
LDLIBS		+= 	-lm   -lstdc++ 


PROGS		= 	mythen3DetectorServer
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	communication_funcs.c slsDetectorServer.c slsDetectorServer_funcs.c slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.c=.o) 



all: clean versioning $(PROGS)

boot: $(OBJS)

versioning: 
	@echo `tput setaf 6;	./updateGitVersion.sh; tput sgr0;`

$(PROGS): $(OBJS)
#	echo $(OBJS) 
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	rm *.gdb

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o

