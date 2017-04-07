CC			=	g++ 
CLAGS		+=  -DVERBOSE #VERYBOSE
LDLIBS		+= 	-lm  -lstdc++ -pthread 

LDIR 		=	../../slsDetectorSoftware
RDIR 		= 	../../slsReceiverSoftware

INCLUDES	= 	-I $(LDIR)/commonFiles  -I $(LDIR)/slsDetector -I ../include -I $(RDIR)/include
SRC_CLNT	= 	qClient.cpp $(RDIR)/src/MySocketTCP.cpp

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
	rm -rf $(DESTDIR)/$(PROGS)  *.o  $(FINALDIR)/$(PROGS)





