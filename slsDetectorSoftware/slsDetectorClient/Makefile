CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
ASM=$(shell echo "/lib/modules/`uname -r`/build/include")

INCLUDES= -I../slsDetectorSoftware/commonFiles -I../slsDetectorSoftware/slsDetector -I ../slsDetectorSoftware/MySocketTCP  -I../slsDetectorSoftware/usersFunctions -I../slsDetectorSoftware/multiSlsDetector -I../slsDetectorSoftware/slsDetectorAnalysis -I../slsDetectorSoftware/slsReceiverInterface  -I$(ASM)
LIBS= -L../slsDetectorSoftware/ -lSlsDetector  -L/usr/lib64/ -lpthread
#LDFLAG= 



INCLUDES= -I../slsDetectorSoftware/commonFiles -I../slsDetectorSoftware/slsDetector -I ../slsDetectorSoftware/MySocketTCP  -I../slsDetectorSoftware/usersFunctions -I../slsDetectorSoftware/multiSlsDetector -I../slsDetectorSoftware/slsDetectorAnalysis -I../slsDetectorSoftware/slsReceiverInterface -I$(shell echo "/lib/modules/`uname -r`/build/include")

LIBS= -L../slsDetectorSoftware/ -lSlsDetector -lpthread


BIN=bin

SRC_CLNT=sls_detector_client.cpp

static_clients: static_sls_detector_put static_sls_detector_get static_sls_detector_acquire static_sls_detector_help

clients: sls_detector_put sls_detector_get sls_detector_acquire sls_detector_help

all:  lib static_clients

nonstatic: lib clients

lib:
	cd ../slsDetectorSoftware && $(MAKE) lib



static_sls_detector_put: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_put $(SRC_CLNT)   $(FLAGS)    $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 

static_sls_detector_get: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_get $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)

static_sls_detector_acquire: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_acquire $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)

static_sls_detector_help: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_help $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)



sls_detector_put: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_put $(SRC_CLNT)   $(FLAGS)    $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 

sls_detector_get: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_get $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)

sls_detector_acquire: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_acquire $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)

sls_detector_help: $(SRC_CLNT) lib
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_help $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)


clean:
	rm  -rf $(BIN)/sls_detector_put sls_detector_get sls_detector_acquire  sls_detector_help




install: all
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P $(BIN)/sls_detector_put $(DESTDIR)
	cp -P $(BIN)/sls_detector_get $(DESTDIR)
	cp -P $(BIN)/sls_detector_acquire $(DESTDIR)
	cp -P $(BIN)/sls_detector_help $(DESTDIR)



