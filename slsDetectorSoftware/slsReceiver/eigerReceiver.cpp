/*
 * eigerReceiver.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: billich
 */

#include <iostream>
#include <string>
#include "eigerReceiver.h"

class EigerReceiverImplementation: public EigerReceiver {

public:
	void initialize(const char *detectorHostName) {
	 std::cout << "initialize() with: detectorHostName= " << detectorHostName << ".\n";
	}

	char *getDetectorHostname() {
		const std::string name = "some_host_name";
	    char *c = new char[name.length()];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        std::cout << "getDetectorHostname(): Return " << c << ".\n";
	    return(c);
	}

	char *getFileName() {
		const std::string name = "some_file_name";
	    char *c = new char[name.length()];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        std::cout << "getFileName(): Return " << c << ".\n";
	    return(c);
	}

	char *getFilePath() {
		std::string name = "some_path";
	    char *c = new char[name.length()];
	    name.copy(c, name.length());
	    c[name.length()] = '\0';
        std::cout << "getFilePath(): Return " << c << ".\n";
	    return(c);
	}

	int getDynamicRange() {
		std::cout << "getDynamicRange(): Return 16.\n";
		return(16);
	}

	int getScanTag() {
		std::cout << "getScanTag(): return 4711.\n";
		return(4711);
	}

	int getNumberOfFrames() {
		std::cout << "getNumberOfFrames(): return 42.\n";
		return(42);
	}

	char * setFileName(const char c[]) {
		std::cout << "setFileName() called with " << c <<".\n";
		return(this->getFileName());
	}

	char * setFilePath(const char c[]) {
		std::cout << "setFilePath() called with" << c << ".\n";
		return(this->getFilePath());
	}

	int getEnableFileWrite() {
		std::cout << "getEnableFileWrite() returns 1.\n";
		return(1);
	}
	int setDynamicRange (const int dr) {
		std::cout << "setDynamicRange() called with " << dr << '.' << '\n';
		return(this->getDynamicRange());
	}

	int setScanTag (const int tag) {
		std::cout << "setScanTag() called with " << tag << ".\n";
		return(this->getScanTag());
	}

	int setNumberOfFrames (const int fnum) {
		std::cout << "setNumberOfFrames() called with " << fnum << ".\n";
		return(this->getNumberOfFrames());
	}

	int setEnableFileWrite(const int i) {
		std::cout << "enableFileWrite() called with " << i << ".\n";
		return(0);
	}

	int startReceiver(char message[]) {
		std::cout << "startReceiver(): return 0.\n";
		message = NULL;
		return(0);
	}

	int stopReceiver() {
		std::cout << "stopReceiver(): return 0.\n";
		return(0);
	}

	void abort() {
		std::cout << "abort(): return 0.\n";
	}

};

EigerReceiver *EigerReceiver::create(void) {
	std::cout << "create(): Return new EigerReceiverImplementation instance.\n";
	return new EigerReceiverImplementation();
}




