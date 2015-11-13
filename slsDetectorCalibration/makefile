CC=gcc
CFLAGS=  -g -c -W  -lstdc++

ROOTINCLUDE=$(ROOTSYS)/include

energyCalibration.o: energyCalibration.cpp
	$(CC) $(CFLAGS) energyCalibration.cpp -I $(ROOTINCLUDE) 

clean:
	rm -f energyCalibration.o