CFLAGS= -DC_ONLY 
#FLAGS=-DVERBOSE 
INCLUDES= -I../slsDetectorSoftware/commonFiles -I../slsDetectorSoftware/slsDetector -I ../slsDetectorSoftware/MySocketTCP  -I../slsDetectorSoftware/usersFunctions
#-I../slsDetectorSoftware/mythenDetector -I../slsDetectorSoftware/gotthardDetector  
#LIBS=-L../TMythenDetector/bin/ -L$(QTDIR)/lib
LIBS= -L$(QTDIR)/lib -L../slsDetectorSoftware/
LDFLAG=-lSlsDetector -lpthread

MYTHEN = mythen_put mythen_get mythen_acquire
PICASSO = picasso_put picasso_get picasso_acquire
GOTTHARD = gotthard_put gotthard_get gotthard_acquire

BIN=bin

SRC_CLNT=sls_detector_client.cpp

gotthard: lib $(GOTTHARD)
mythen: lib $(MYTHEN)
picasso: lib $(PICASSO)

lib:
#	cd ../TMythenDetector && $(MAKE)
	cd ../slsDetectorSoftware && $(MAKE) lib

all: mythen gotthard picasso 




mythen_put: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/mythen_put $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DPUT -DMYTHEN_DET $(LIBS) $(LDFLAG) 

mythen_get: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/mythen_get $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DGET  -DMYTHEN_DET $(LIBS) $(LDFLAG)

mythen_acquire: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/mythen_acquire $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DREADOUT  -DMYTHEN_DET $(LIBS) $(LDFLAG)


mythen_clean:
	rm  -rf $(BIN)/$(MYTHEN)









gotthard_put: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/gotthard_put $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DPUT -DGOTTHARD_DET $(LIBS) $(LDFLAG) 

gotthard_get: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/gotthard_get $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DGET  -DGOTTHARD_DET $(LIBS) $(LDFLAG)

gotthard_acquire: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/gotthard_acquire $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DREADOUT  -DGOTTHARD_DET $(LIBS) $(LDFLAG)



gotthard_clean:
	rm  -rf $(BIN)/$(GOTTHARD)









picasso_put: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/picasso_put $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DPUT -DPICASSOD  -DMYTHEN_DET $(LIBS) $(LDFLAG)

picasso_get: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/picasso_get $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DGET -DPICASSOD  -DMYTHEN_DET $(LIBS) $(LDFLAG)

picasso_acquire: $(SRC_CLNT)
	mkdir -p $(BIN)
	$(CXX) -static -o $(BIN)/picasso_acquire $(SRC_CLNT)   $(FLAGS) -lpthread $(INCLUDES) -DREADOUT -DGET -DPICASSOD  -DMYTHEN_DET $(LIBS) $(LDFLAG)



picasso_clean:
	rm  -rf $(BIN)/$(PICASSO)













clean:	picasso_clean mythen_clean gotthard_clean
	

install_mythen: mythen
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P $(BIN)/mythen_put $(DESTDIR)
	cp -P $(BIN)/mythen_get $(DESTDIR)
	cp -P $(BIN)/mythen_acquire $(DESTDIR)

install_gotthard: gotthard
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P $(BIN)/gotthard_put $(DESTDIR)
	cp -P $(BIN)/gotthard_get $(DESTDIR)
	cp -P $(BIN)/gotthard_acquire $(DESTDIR)

install_picasso:  picasso
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P $(BIN)/picassoPut $(DESTDIR)
	cp -P $(BIN)/picassoGet $(DESTDIR)
	cp -P $(BIN)/picassoData $(DESTDIR)

install_doc:
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	cp -P docs/* $(DESTDIR)

install: install_mythen install_picasso install_gotthard
	



