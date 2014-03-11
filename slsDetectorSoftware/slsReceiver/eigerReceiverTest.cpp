/*
 * eigerReceiverTest.cpp

 *
 *  Created on: Mar 11, 2014
 *      Author: billich
 */

#include <iostream>
#include <string>
#include "eigerReceiver.h"

int main(int argc, char *argv[]){

	const char *name = "detectors_host_name";
	std::string prefix = "main: ";
	std::cout <<prefix<< "start EigerReceiver tests\n";

	EigerReceiver *receiver = EigerReceiver::create();
	receiver->initialize(name);

	char *c1 = receiver->getFileName();
	std::cout <<prefix<< "got file name " << c1 << '\n';
	delete[] c1;

	char *c2 = receiver->getFilePath();
	std::cout <<prefix<< "got path name " << c2 << '\n';
	delete[]c2;

	int range = receiver->getDynamicRange();
	std::cout <<prefix<< "got dynamic range " << range << ".\n";

	int tag = receiver->getScanTag();
	std::cout <<prefix<< "got scan tag " << tag << ".\n";

	char *c3 = receiver->setFileName( "some_other_name");
	std::cout <<prefix<< "got file name  " << c3 << " after setting to <some_other_name>\n";
	delete[] c3;

	char *c4 = receiver->setFilePath( "some_other_path");
	std::cout <<prefix<< "got file path  " << c4 << " after setting to <some_other_path>\n";
	delete[] c4;

	range = receiver->setDynamicRange(8);
	std::cout <<prefix<< "got dynamic range "  << range << " after setting it to 8.\n";

	int n = receiver->setNumberOfFrames(11);
	int w = receiver->setEnableFileWrite(1);

	char *c5;
	receiver->startReceiver(c5);
	receiver->stopReceiver();
	receiver->abort();
	receiver->getEnableFileWrite();
	char *c6 = receiver->getDetectorHostname();
	delete[] c6;
}




