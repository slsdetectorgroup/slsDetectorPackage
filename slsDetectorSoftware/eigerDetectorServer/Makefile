CC			=	powerpc-4xx-softfloat-gcc
CLAGS		+=  -Wall -DDACS_INT   -DEIGERD  -DSLS_DETECTOR_FUNCTION_LIST #-DVERBOSE #-DVIRTUAL
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.c slsDetectorServer_funcs.c communication_funcs.c slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.cpp=.o) 



all: clean $(PROGS)


3pc: clean pccompile $(PROGS)


#pccompile:
#CC := gcc
	
boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CC) $(SRC_CLNT) $(CLAGS) $(LDLIBS) -o $@   
	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
