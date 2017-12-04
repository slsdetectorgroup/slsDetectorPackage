CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
#ASM=$(shell echo "/lib/modules/`uname -r`/build/include")

#INCLUDES= -I../slsDetectorSoftware/commonFiles -I../slsDetectorSoftware/slsDetector -I ../slsDetectorSoftware/MySocketTCP  -I../slsDetectorSoftware/usersFunctions -I../slsDetectorSoftware/multiSlsDetector -I../slsDetectorSoftware/slsDetectorAnalysis -I../slsDetectorSoftware/slsReceiverInterface  -I$(ASM)
#LDFLAG= 



INCLUDES?= -I../commonFiles -I../slsDetector -I ../MySocketTCP  -I../usersFunctions -I../multiSlsDetector -I../slsDetectorAnalysis -I../slsReceiverInterface -I$(shell echo "/lib/modules/`uname -r`/build/include")


LIBDIR?=../
LIBS?= -L$(LIBDIR) -lSlsDetector 

LDFLAG=   -L/usr/lib64/ -lpthread


DESTDIR ?= bin
DOCDIR ?= $(PWD)/docs

BIN=$(DESTDIR)

SRC_CLNT=sls_detector_client.cpp

clients: $(DESTDIR)/sls_detector_put $(DESTDIR)/sls_detector_get $(DESTDIR)/sls_detector_acquire $(DESTDIR)/sls_detector_help
	echo $(LIBS)

static_clients: $(DESTDIR)/ssls_detector_put $(DESTDIR)/ssls_detector_get $(DESTDIR)/ssls_detector_acquire $(DESTDIR)/ssls_detector_help

all: clients
	echo $(LIBS)

nonstatic: clients



lib:
	cd ../ && $(MAKE) DESTDIR=$(LIBDIR) 



$(DESTDIR)/ssls_detector_put: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/ssls_detector_put $(SRC_CLNT)   $(FLAGS)    $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 

$(DESTDIR)/ssls_detector_get: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/ssls_detector_get $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)

$(DESTDIR)/ssls_detector_acquire: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/ssls_detector_acquire $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)

$(DESTDIR)/ssls_detector_help: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/ssls_detector_help $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)



$(DESTDIR)/sls_detector_put: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_put $(SRC_CLNT)   $(FLAGS)    $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))

$(DESTDIR)/sls_detector_get: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_get $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))

$(DESTDIR)/sls_detector_acquire: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_acquire $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))

$(DESTDIR)/sls_detector_help: $(SRC_CLNT) lib
	echo $(LIBS)
	echo  $(LDFLAG) 
	mkdir -p $(BIN)
	$(CXX) -o $(BIN)/sls_detector_help $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))


doc: 
	cd manual && make DESTDIR=$(DOCDIR)

htmldoc: 
	cd manual && make html DESTDIR=$(DOCDIR)

clean:
#	cd manual && make clean
	rm  -rf $(BIN)/sls_detector_put $(BIN)/sls_detector_get $(BIN)/sls_detector_acquire  $(BIN)/sls_detector_help 

install: clients




