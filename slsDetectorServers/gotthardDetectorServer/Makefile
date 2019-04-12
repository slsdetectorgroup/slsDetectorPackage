CROSS  		= 	bfin-uclinux-
CC    		= 	$(CROSS)gcc
CFLAGS 		+=	-Wall  -DGOTTHARDD  -DSTOP_SERVER  # -DVERBOSE 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	gotthardDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRCS 		= 	communication_funcs.c slsDetectorServer.c slsDetectorServer_funcs.c  slsDetectorFunctionList.c 
OBJS 		= 	$(SRCS:%.c=%.o)


all: clean versioning $(PROGS) 

boot: $(OBJS)

version_name=APIGOTTHARD
version_path=slsDetectorServers/gotthardDetectorServer
versioning: 
	cd ../../ && echo $(PWD) && echo `tput setaf 6; ./updateAPIVersion.sh $(version_name) $(version_path);  tput sgr0;`

	
$(PROGS): $(OBJS) 
#	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CC) -o $@ $^ $(CFLAGS)   $(LDLIBS)
	mv $(PROGS) $(DESTDIR)
	rm *.gdb

clean:
	rm -rf $(DESTDIR)/$(PROGS) *.o *.gdb





