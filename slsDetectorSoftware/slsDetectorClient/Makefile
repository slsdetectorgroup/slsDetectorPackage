CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
INCLUDES= -I../slsDetectorSoftware/commonFiles -I../slsDetectorSoftware/slsDetector -I ../slsDetectorSoftware/MySocketTCP  -I../slsDetectorSoftware/usersFunctions -I../slsDetectorSoftware/multiSlsDetector
#LIBS=-L../TMythenDetector/bin/ -L$(QTDIR)/lib
LIBS= -L../slsDetectorSoftware/
LDFLAG= -lSlsDetector -lpthread

BIN=bin

SRC_CLNT=sls_detector_client.cpp

clients: sls_detector_put sls_detector_get sls_detector_acquire sls_detector_help

all:  lib clients

lib:
	cd ../slsDetectorSoftware && $(MAKE) lib



sls_detector_put: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_put $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 

sls_detector_get: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_get $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)

sls_detector_acquire: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_acquire $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)

sls_detector_help: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/sls_detector_help $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)


clean:
	rm  -rf $(BIN)/sls_detector_put sls_detector_get sls_detector_acquire  sls_detector_help





install_sls_detector: all
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P $(BIN)/sls_detector_put $(DESTDIR)
	cp -P $(BIN)/sls_detector_get $(DESTDIR)
	cp -P $(BIN)/sls_detector_acquire $(DESTDIR)
	cp -P $(BIN)/sls_detector_help $(DESTDIR)



