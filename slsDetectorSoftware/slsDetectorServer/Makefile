CC			=	gcc
CLAGS		+=  -Wall -DVIRTUAL  -DDACS_INT  -DGENERICD # -DSLS_DETECTOR_FUNCTION_LIST 
LDLIBS		+= 	-lm  

PROGS		= 	genericDetectorServer 
DESTDIR		?= 	bin
INSTMODE	= 	0777

SRC_CLNT	=	slsDetectorServer.c slsDetectorServer_funcs.c communication_funcs.c  slsDetectorFunctionList.c 
OBJS 		= 	$(SRC_CLNT:.cpp=.o) 



all: clean $(PROGS)

boot: $(OBJS)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CC) $(SRC_CLNT) $(CLAGS) $(LDLIBS) -o $@   
	mv $(PROGS) $(DESTDIR)


clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
