/*
 * eigerReceiver.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: billich
 */

#include <iostream>
#include <string>
#include "eigerReceiver.h"


/* uncomment next line to enable debug output */
#define EIGER_DEBUG

/* macro for debug output http://stackoverflow.com/a/14256296 */
#ifdef EIGER_DEBUG
#define DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#else
#define DEBUG(x)
#endif


using namespace std;

struct EigerReceiverInitializationConfiguration {

	string detectorHostname;
};

struct EigerReceiverScanConfiguration {

	string fileName;
	string filePath;
	int dynamicRange;
	int scanTag;
	int numberOfFrames;
	bool doFileWrite;
	bool doFileOverWrite;

	EigerReceiverScanConfiguration():
			dynamicRange(-1),
			scanTag(-1),
			numberOfFrames(-1),
			doFileWrite(false),
			doFileOverWrite(false){};
};

class EigerReceiverImplementation: public EigerReceiver {

public:

	EigerReceiverImplementation() : isInitialized(false), status(slsDetectorDefs::ERROR) {};

	void initialize(const char *detectorHostname) {

	 string name;
	 if (detectorHostname != NULL) {
		 name = detectorHostname;
	 }

	 if (name.empty()) {
		 DEBUG("initialize(): can't initialize with empty string or NULL for detectorHostname");
	 } else if (isInitialized == true) {
		 DEBUG("initialize(): already initialized, can't initialize several times");
	 } else {
	   DEBUG("initialize(): initialize() with: detectorHostName=" << name << ".");
	   init_config.detectorHostname = name;
	   isInitialized = true;
	   status = slsDetectorDefs::IDLE;
	 }

	 //REST call - hardcoded
         RestHelper rest ;
         rest.init("localhost",8080);
         std::string answer;
         std::cout << "---- REST test 1: true, string "<< std::endl;
         int code = rest.get_json("status", &answer);
         std::cout << "Answer: " <<  answer << std::endl;

         std::cout << "---- REST test 2: 404, string "<< std::endl;
         code = rest.get_json("statuss", &answer);
         if (code != 0){
           //throw -1;
           std::cout << "I SHOULD THROW AN EXCEPTION!!!" << std::endl;
         }

         std::cout << "---- REST test 3: true, json object "<< std::endl;
         JsonBox::Value json_value;
         code = rest.get_json("status", &json_value);
         std::cout << "JSON " << json_value["status"] << std::endl;

	 answer = "";
	 std::cout << "---- REST test 4: POST, string "<< std::endl;
         code = rest.post_json("recipes/cassoela", &answer);
	 std::cout << "POST answer: " << answer << std::endl;
	 if (code != 0){
           //throw -1;
           std::cout << "I SHOULD THROW AN EXCEPTION!!!" << std::endl;
         }

	 RestHelper rest2 ;
         rest2.init("reallyfake",8080);
         std::cout << "---- REST test 4: host not found, json object "<< std::endl;
         JsonBox::Value json_value2;
         code = rest2.get_json("status", &json_value2);
         if (code != 0){
           //throw -1;
           std::cout << "I SHOULD THROW AN EXCEPTION!!!" << std::endl;
         }

        }


	char *getDetectorHostname() const {
		string name = init_config.detectorHostname;
		if (name.empty()) {
			DEBUG("getDetectorHostname(): Return NULL");
			return(NULL);
		}
	    char *c = new char[name.length()+1];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        DEBUG("getDetectorHostname(): Return " << c << ".");
	    return(c);
	}

	char *getFileName() const {
		string name = scan_config.fileName;

	    char *c = new char[name.length()+1];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        DEBUG("getFileName(): Return " << c);
	    return(c);
	}

	char *getFilePath() const {
		string name = scan_config.filePath;

	    char *c = new char[name.length()+1];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        DEBUG("getFilePath(): Return " << c);
	    return(c);
	}

	int getDynamicRange() const {
		DEBUG("getDynamicRange(): Return " << scan_config.dynamicRange);
		return(scan_config.dynamicRange);
	}

	int getScanTag() const {
		DEBUG("getScanTag(): returns " << scan_config.scanTag);
		return(scan_config.scanTag);
	}

	int getNumberOfFrames() const {
		DEBUG("getNumberOfFrames(): return " << scan_config.numberOfFrames);
		return(scan_config.numberOfFrames);
	}

	int getEnableFileWrite() const {
		DEBUG("getEnableFileWrite() returns " << scan_config.doFileWrite);
		return(scan_config.doFileWrite);
	}

	int getEnableOverwrite() const {
		DEBUG("getEnableOverwrite() returns " << scan_config.doFileOverWrite);
		return(scan_config.doFileOverWrite);
	}

	slsDetectorDefs::runStatus getStatus() const {
		DEBUG("getStatus(): return " <<status);
		return(status);
	}

	char *setFileName(const char c[]) {
		DEBUG("setFileName() called with " << c <<".");
		scan_config.fileName = c;
		return(this->getFileName());
	}

	char *setFilePath(const char c[]) {
		DEBUG("setFilePath() called with " << c << ".");
		scan_config.filePath = c;
		return(this->getFilePath());
	}

	int setDynamicRange (const int dr) {
		DEBUG("setDynamicRange() called with " << dr << '.');
		scan_config.dynamicRange = dr;
		return(getDynamicRange());
	}

	int setScanTag (const int tag) {
		DEBUG("setScanTag() called with " << tag);
		scan_config.scanTag = tag;
		return(getScanTag());
	}

	int setNumberOfFrames (const int fnum) {
		DEBUG("setNumberOfFrames() called with " << fnum);
		scan_config.numberOfFrames = fnum;
		return(getNumberOfFrames());
	}

	int setEnableFileWrite(const int i) {
		DEBUG("enableFileWrite() called with " << i);
		scan_config.doFileWrite = i;
		return(getEnableFileWrite());
	}

	int setEnableOverwrite(const int i) {
		DEBUG("setEnableOverwrite() called with " << i);
		scan_config.doFileOverWrite = i;
		return(getEnableOverwrite());
	}

	int startReceiver(char message[]) {
		DEBUG("startReceiver(): return 0.");
		status = slsDetectorDefs::RUNNING;
		message = NULL;
		return(0);
	}

	int stopReceiver() {
		DEBUG("stopReceiver(): return 0.");
		status = slsDetectorDefs::IDLE;
		return(0);
	}

	void abort() {
		DEBUG("abort(): return 0.");
		status = slsDetectorDefs::IDLE;
	}

private:
	EigerReceiverScanConfiguration scan_config;
	EigerReceiverInitializationConfiguration init_config;
	bool isInitialized;
	slsDetectorDefs::runStatus status;
};

EigerReceiver *EigerReceiver::create(void) {
	DEBUG("create(): Return new EigerReceiverImplementation instance.");
	return new EigerReceiverImplementation();
}




