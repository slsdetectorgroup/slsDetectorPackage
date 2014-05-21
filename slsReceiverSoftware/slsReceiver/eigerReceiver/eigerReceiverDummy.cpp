/*
 * eigerReceiver.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: billich
 */

#include <iostream>
#include <string>
#include "eigerReceiver.h"


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

	EigerReceiverImplementation(){};

        ~EigerReceiverImplementation(){};

	void initialize(const char *detectorHostname) {}

	char *getDetectorHostname() const { return (char*)"";}

	char *getFileName() const {return (char*)"";}

	char *getFilePath() const {return (char*)"";}

	int getDynamicRange() const { return 0;}

	int getScanTag() const {return 0;}

	int getNumberOfFrames() const {return 0;}

	int getEnableFileWrite() const {return 0;}

	int getEnableOverwrite() const {return 0;}

	slsReceiverDefs::runStatus getStatus() const { return slsReceiverDefs::IDLE;}

	char *setFileName(const char c[]) {return (char*)"";}

	char *setFilePath(const char c[]) {return (char*)"";}

	int setDynamicRange (const int dr) {return 0;}

	int setScanTag (const int tag) {return 0;}

	int setNumberOfFrames (const int fnum) {return 0;}

	int setEnableFileWrite(const int i) {return 0;}

	int setEnableOverwrite(const int i) {return 0;}

	int startReceiver(char message[]) {return 0;}

	int stopReceiver() {return 0;}

	void abort() {}

private:
	EigerReceiverScanConfiguration scan_config;
	EigerReceiverInitializationConfiguration init_config;
	bool isInitialized;
	slsReceiverDefs::runStatus status;
	
};

EigerReceiver *EigerReceiver::create(void) {
	return new EigerReceiverImplementation();
}




