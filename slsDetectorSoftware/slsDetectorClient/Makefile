include ../../Makefile.include

CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
#ASM=$(shell echo "/lib/modules/`uname -r`/build/include")

INCLUDES?= -I../commonFiles -I../slsDetector -I ../MySocketTCP  -I../usersFunctions -I../multiSlsDetector -I../slsDetectorAnalysis -I../slsReceiverInterface -I ../../slsReceiverSoftware/include -I$(shell echo "/lib/modules/`uname -r`/build/include")
DESTDIR ?= ../../bin
BIN=$(DESTDIR)
LIBS?= -L$(DESTDIR) -lSlsDetector 
LIBRARYDIR?=../
LDFLAG=   -L/usr/lib64/ -pthread
DOCDIR ?= $(PWD)/docs
SRC_CLNT=sls_detector_client.cpp


$(info )
$(info #######################################)
$(info #    In slsDetectorclient Makefile    #)
$(info #######################################)
$(info )

all: clients

nonstatic: clients

clients: builddir lib $(DESTDIR)/sls_detector_put $(DESTDIR)/sls_detector_get $(DESTDIR)/sls_detector_acquire $(DESTDIR)/sls_detector_help

static_clients: builddir lib $(DESTDIR)/ssls_detector_put $(DESTDIR)/ssls_detector_get $(DESTDIR)/ssls_detector_acquire $(DESTDIR)/ssls_detector_help

builddir:
#	echo $(LIBS)
#	echo  $(LDFLAG) 
	mkdir -p $(BIN)

lib: 
#	echo $(LIBDIR) 
	cd $(LIBRARYDIR) && $(MAKE) DESTDIR=$(DESTDIR)  TABSPACE='$(TABSPACE)'
	@echo ""
	@echo "#######################################"
	@echo "#  Back in slsDetectorClient Makefile #"
	@echo "#######################################"
	@echo ""

$(DESTDIR)/ssls_detector_put: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a 
	$(call colorecho,"#######################################")
	$(call colorecho,"#     Compiling ssls_detector_put     #")
	$(call colorecho,"#######################################")
	$(CXX) -static -o $(BIN)/ssls_detector_put $(SRC_CLNT)   $(FLAGS)    $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 

$(DESTDIR)/ssls_detector_get: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#     Compiling ssls_detector_get     #")
	$(call colorecho,"#######################################")
	$(CXX) -static -o $(BIN)/ssls_detector_get $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)

$(DESTDIR)/ssls_detector_acquire: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#   Compiling ssls_detector_acquire   #")
	$(call colorecho,"#######################################")
	$(CXX) -static -o $(BIN)/ssls_detector_acquire $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)

$(DESTDIR)/ssls_detector_help: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#     Compiling ssls_detector_help    #")
	$(call colorecho,"#######################################")
	$(CXX) -static -o $(BIN)/ssls_detector_help $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)



$(DESTDIR)/sls_detector_put: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#      Compiling sls_detector_put     #")
	$(call colorecho,"#######################################")
	$(CXX) -o $(BIN)/sls_detector_put $(SRC_CLNT)   $(FLAGS)    $(INCLUDES) -DPUT $(LIBS) $(LDFLAG) 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))

$(DESTDIR)/sls_detector_get: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#      Compiling sls_detector_get     #")
	$(call colorecho,"#######################################")
	$(CXX) -o $(BIN)/sls_detector_get $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DGET  $(LIBS) $(LDFLAG)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))

$(DESTDIR)/sls_detector_acquire: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#    Compiling sls_detector_acquire   #")
	$(call colorecho,"#######################################")
	$(CXX) -o $(BIN)/sls_detector_acquire $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DREADOUT  $(LIBS) $(LDFLAG)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))

$(DESTDIR)/sls_detector_help: $(SRC_CLNT) $(DESTDIR)/libSlsDetector.so $(DESTDIR)/libSlsDetector.a
	$(call colorecho,"#######################################")
	$(call colorecho,"#      Compiling sls_detector_help    #")
	$(call colorecho,"#######################################")
	$(CXX) -o $(BIN)/sls_detector_help $(SRC_CLNT)   $(FLAGS)  $(INCLUDES) -DHELP  $(LIBS) $(LDFLAG)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))


doc: 
	cd manual && make DESTDIR=$(DOCDIR)

htmldoc: 
	cd manual && make html DESTDIR=$(DOCDIR)

clean:
#	cd manual && make clean
	rm  -rf $(BIN)/sls_detector_put $(BIN)/sls_detector_get $(BIN)/sls_detector_acquire  $(BIN)/sls_detector_help $(BIN)/gui_client

install: clients

