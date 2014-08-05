/*
 * eigerReceiverTest.cpp

 *
 *  Created on: Mar 11, 2014
 *      Author: billich
 */

#include <iostream>
#include <string>
#include "eigerReceiver.h"

using namespace std;

int main(int argc, char *argv[]){

	const char *name = "detectors_host_name";
	const char *empty = "";
	std::string prefix = "main: ";
	cout <<prefix<< "start EigerReceiver tests" << endl;


	EigerReceiver *receiver = EigerReceiver::create();

	int status = receiver->getStatus();
	char *c0 = receiver->getDetectorHostname();
	if (c0 == NULL) {
 		cout <<prefix<< "getDetectorHostname() returned NULL, as expected before initialization." << endl;
	}
	delete[] c0;

	cout <<prefix<< "initialize 4 times - only the second should work" << endl;
	receiver->initialize(empty);
	status = receiver->getStatus();
	receiver->initialize(name);
	status = receiver->getStatus();
	receiver->initialize(name);
	status = receiver->getStatus();
	receiver->initialize((char *)NULL);

	cout << endl;

	status = receiver->getStatus();
	char *c6 = receiver->getDetectorHostname();
	cout <<prefix<< "got detector hostname " << c6 << " after initialization;" <<endl<<endl;
	delete[] c6;

	cout <<prefix<< "try get*() methods before set*() - expect default values" <<endl;
	char *c1 = receiver->getFileName();
	cout <<prefix<< "got file name <" << c1 <<">." << endl;
	delete[] c1;

	char *c2 = receiver->getFilePath();
	cout <<prefix<< "got path name <" << c2 <<">." << endl;
	delete[]c2;

	int range = receiver->getDynamicRange();
	cout <<prefix<< "got dynamic range " << range << endl;

	int tag = receiver->getScanTag();
	cout <<prefix<< "got scan tag " << tag << endl;
	cout << endl;

	char *c3 = receiver->setFileName( "some_other_name");
	cout <<prefix<< "got file name  <" << c3 << "> after setting to <some_other_name>" << endl << endl;
	delete[] c3;

	char *c4 = receiver->setFilePath( "some_other_path");
	cout <<prefix<< "got file path  <" << c4 << "> after setting to <some_other_path>" << endl << endl;
	delete[] c4;

	range = receiver->setDynamicRange(8);
	cout <<prefix<< "got dynamic range "  << range << " after setting it to 8." << endl << endl;

	tag = receiver->setScanTag(99);
	cout << "got scan tag " << tag << " after setting to 99." << endl << endl;

	int n = receiver->setNumberOfFrames(11);
	cout << "got number of frames " << n << " after setting to 11." << endl << endl;

	int w = receiver->setEnableFileWrite(1);
	cout << "got enable file write " << w  << " after setting to 1." << endl << endl;

	char *c5;
	status = receiver->getStatus();
	receiver->startReceiver(c5);
	status = receiver->getStatus();
	receiver->stopReceiver();
	status = receiver->getStatus();
	receiver->abort();
	status = receiver->getStatus();

}




