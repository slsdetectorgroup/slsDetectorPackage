CC			=	g++
#CC  		= 	bfin-uclinux-
SRC			=	slsADCReader.cpp
INCLUDES	=	-I ../../slsReceiverSoftware/include -I ../commonFiles
CFLAGS 		=	-Wall -lm -lstdc++


all: clean slsADCReader


slsADCReader: $(SRC)
	echo "creating adc reader"
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)


clean:
	echo "cleaning"
	rm -rf *.o

	

