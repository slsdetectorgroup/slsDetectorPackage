/********************************************//**
 * @file slsReceiver.cpp
 * @short creates the UDP and TCP class objects
 ***********************************************/

#include "slsReceiver.h"
#include "slsReceiverUDPFunctions.h"
#include "eigerReceiver.h"


slsReceiver::slsReceiver(int argc, char *argv[], int &success){
	//creating base receiver
	cout << "SLS Receiver" << endl;
	receiverBase = new slsReceiverUDPFunctions();

	//tcp ip interface
	tcpipInterface = new slsReceiverTCPIPInterface(argc,argv,success,receiverBase);
}


slsReceiver::~slsReceiver() {if(receiverBase) delete receiverBase; if(tcpipInterface) delete tcpipInterface;}


int slsReceiver::start() {
	return tcpipInterface->start();
}


void slsReceiver::stop() {
	tcpipInterface->stop();
}


void slsReceiver::closeFile(int p) {
	tcpipInterface->closeFile(p);
}



int64_t slsReceiver::getReceiverVersion(){
	tcpipInterface->getReceiverVersion();
}


void slsReceiver::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
  //tcpipInterface
	  receiverBase->registerCallBackStartAcquisition(func,arg);
}



void slsReceiver::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
  //tcpipInterface
	  receiverBase->registerCallBackAcquisitionFinished(func,arg);
}


void slsReceiver::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
  //tcpipInterface
	  receiverBase->registerCallBackRawDataReady(func,arg);
}


