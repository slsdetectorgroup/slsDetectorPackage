#include "slsReceiverUsers.h"
#include "slsReceiver.h"

slsReceiver* slsReceiverUsers::receiver(NULL);

slsReceiverUsers::slsReceiverUsers(int argc, char *argv[], int &success) {
	slsReceiverUsers::receiver=new slsReceiver(argc, argv, success);
}

slsReceiverUsers::~slsReceiverUsers() {
  delete slsReceiverUsers::receiver;
}

int slsReceiverUsers::start() {
	return slsReceiverUsers::receiver->start();
}

void slsReceiverUsers::stop() {
	slsReceiverUsers::receiver->stop();
}


void slsReceiverUsers::closeFile(int p) {
	slsReceiverUsers::receiver->closeFile(p);
}

int64_t slsReceiverUsers::getReceiverVersion(){
	slsReceiverUsers::receiver->getReceiverVersion();
}


void slsReceiverUsers::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
	slsReceiverUsers::receiver->registerCallBackStartAcquisition(func,arg);
}

  

void slsReceiverUsers::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
	slsReceiverUsers::receiver->registerCallBackAcquisitionFinished(func,arg);
}
	

void slsReceiverUsers::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
	slsReceiverUsers::receiver->registerCallBackRawDataReady(func,arg);
}


