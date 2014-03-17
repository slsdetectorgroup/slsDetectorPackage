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
// #define EIGER_DEBUG

/* macro for debug output */
#ifdef EIGER_DEBUG
#define DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#else
#define DEBUG(x)
#endif

class EigerReceiverImplementation: public EigerReceiver {

public:
	void initialize(const char *detectorHostName) {
	 DEBUG("initialize() with: detectorHostName= " << detectorHostName << ".");
	}

	char *getDetectorHostname() {
		const std::string name = "some_host_name";
	    char *c = new char[name.length()];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        DEBUG("getDetectorHostname(): Return " << c << ".");
	    return(c);
	}

	char *getFileName() {
		const std::string name = "some_file_name";
	    char *c = new char[name.length()];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        DEBUG("getFileName(): Return " << c);
	    return(c);
	}

	char *getFilePath() {
		std::string name = "some_path";
	    char *c = new char[name.length()];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        DEBUG("getFilePath(): Return " << c);
	    return(c);
	}

	int getDynamicRange() {
		DEBUG("getDynamicRange(): Return 16.");
		return(16);
	}

	int getScanTag() {
		DEBUG("getScanTag(): return 4711.");
		return(4711);
	}

	int getNumberOfFrames() {
		DEBUG("getNumberOfFrames(): return 42.");
		return(42);
	}

	char * setFileName(const char c[]) {
		DEBUG("setFileName() called with " << c <<".");
		return(this->getFileName());
	}

	char * setFilePath(const char c[]) {
		DEBUG("setFilePath() called with" << c << ".");
		return(this->getFilePath());
	}

	int getEnableFileWrite() {
		DEBUG("getEnableFileWrite() returns 1.");
		return(1);
	}
	int setDynamicRange (const int dr) {
		DEBUG("setDynamicRange() called with " << dr << '.');
		return(this->getDynamicRange());
	}

	int setScanTag (const int tag) {
		DEBUG("setScanTag() called with " << tag);
		return(this->getScanTag());
	}

	int setNumberOfFrames (const int fnum) {
		DEBUG("setNumberOfFrames() called with " << fnum);
		return(this->getNumberOfFrames());
	}

	int setEnableFileWrite(const int i) {
		DEBUG("enableFileWrite() called with " << i);
		return(0);
	}

	int startReceiver(char message[]) {
		DEBUG("startReceiver(): return 0.");
		message = NULL;
		return(0);
	}

	int stopReceiver() {
		DEBUG("stopReceiver(): return 0.");
		return(0);
	}

	void abort() {
		DEBUG("abort(): return 0.");
	}

};

EigerReceiver *EigerReceiver::create(void) {
	DEBUG("create(): Return new EigerReceiverImplementation instance.");
	return new EigerReceiverImplementation();
}




