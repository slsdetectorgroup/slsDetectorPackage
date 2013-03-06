CC			=	g++ 
CLAGS		+=  -DVERBOSE #VERYBOSE
LDLIBS		+= 	-lm  -lstdc++ -lpthread 

LDIR 		=	../../slsDetectorSoftware
INCLUDES	= 	-I ../../slsDetectorSoftware/commonFiles  -I ../../slsDetectorSoftware/MySocketTCP -I ../../slsDetectorSoftware/slsDetector -I ../include
SRC_CLNT	= 	qClient.cpp ../../slsDetectorSoftware/MySocketTCP/MySocketTCP.cpp

PROGS		= 	gui_client 
DESTDIR		?= 	bin
INSTMODE	= 	0777

FINALDIR    = ../../bin

OBJS 		= 	$(SRC_CLNT:.cpp=.o) 


all: clean $(PROGS)

boot: $(OBJS)

$(PROGS):
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	gcc $(SRC_CLNT) $(INCLUDES) $(FLAGS) $(LDLIBS) -o $@   
	mv $(PROGS) $(FINALDIR)

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o 
	rm -rf $(FINALDIR)/$(PROGS)





