CC			=	g++ 
CLAGS		+=  -DVERBOSE #VERYBOSE
LDLIBS		+= 	-lm  -lstdc++ -pthread 

DDIR		=	../../bin
INCLUDES	= 	-I ../../slsSupportLib/include -I ../include 
SRC_CLNT	= 	qClient.cpp 
DEPSINCLUDES=   qClient.h ../../slsSupportLib/include/MySocketTCP.h  ../../slsSupportLib/include/sls_detector_defs.h


all:	client

client: $(OBJS) $(DDIR)/gui_client

OBJS = $(SRC_CLNT:%.cpp=%.o) 


$(DDIR)/gui_client :  $(SRC_CLNT) $(DEPSINCLUDES) Makefile
	$(CXX) -o $@ -c $< $(INCLUDES) $(FLAGS)  $(LDLIBS) 



clean:
	rm -rf $(DDIR)/gui_client  *.o  





