CC			=	g++ 
CLAGS		+=  -DVERBOSE #VERYBOSE
LDLIBS		+= 	-lm  -lstdc++ -pthread 

LDIR 		=	../../slsDetectorSoftware
RDIR 		= 	../../slsReceiverSoftware

INCLUDES	= 	-I $(LDIR)/commonFiles  -I $(LDIR)/slsDetector -I ../include -I $(RDIR)/include
SRC_CLNT	= 	qClient.cpp $(RDIR)/src/MySocketTCP.cpp
DEPSINCLUDES=   $(RDIR)/include/MySocketTCP.h $(LDIR)/slsDetector/slsDetectorBase.h  $(LDIR)/commonFiles/sls_detector_defs.h


all:	client

client: $(OBJS) $(DESTDIR)/gui_client

OBJS = $(SRC_CLNT:%.cpp=%.o) 


$(DESTDIR)/gui_client :  $(SRC_CLNT) $(DEPSINCLUDES) Makefile
	$(CXX) -o $@ -c $< $(INCLUDES) $(FLAGS)  $(LDLIBS) 



clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o  





