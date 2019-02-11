INCLUDES	=   -I . 
SRC_TEST	= 	main.cpp
LIBDIR		= 	../../bin
ZLIBDIR		= 	../../slsReceiverSoftware/include

LBITS := $(shell getconf LONG_BIT)
SERVER_TYPE = "64 bits compile server detected..."
LDFLAG_DET 	=	-I. -L$(LIBDIR) -L$(ZLIBDIR) -lSlsReceiver -lSlsDetector -L/usr/lib64/ -pthread -lrt -L. -lzmq

all: manual-acq 
	
manual-acq:$(SRC_TEST)
	@echo "------------------------------------------------------------"
	@echo "creating test software"
	@echo $(SERVER_TYPE)
	@echo "------------------------------------------------------------"

	mkdir -p bin
	g++ -o bin/manual-acq $(SRC_TEST) $(INCLUDES) $(LDFLAG_DET) -lm -lstdc++
	cp bin/manual-acq ../../bin
	
clean:
	@echo "------------------------------------------------------------"
	@echo "cleaning test software"
	@echo "------------------------------------------------------------"
	rm -rf bin/manual-acq
