#include "slsReceiverUsers.h"
#include "slsReceiver.h"

slsReceiverUsers::slsReceiverUsers(int argc, char *argv[], int &success) {
	receiver=new slsReceiver(argc, argv, success);
}

slsReceiverUsers::~slsReceiverUsers() {
  delete receiver;
}

int slsReceiverUsers::start() {
	return receiver->start();
}

void slsReceiverUsers::stop() {
	receiver->stop();
}


void slsReceiverUsers::closeFile(int p) {
	receiver->closeFile(p);
}

int64_t slsReceiverUsers::getReceiverVersion(){
	return receiver->getReceiverVersion();
}


void slsReceiverUsers::registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg){
	receiver->registerCallBackStartAcquisition(func,arg);
}

  

void slsReceiverUsers::registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg){
	receiver->registerCallBackAcquisitionFinished(func,arg);
}
	

void slsReceiverUsers::registerCallBackRawDataReady(void (*func)(int, uint64_t, uint64_t, uint64_t, char*, uint32_t, FILE*, void*), void *arg){
	receiver->registerCallBackRawDataReady(func,arg);
}


