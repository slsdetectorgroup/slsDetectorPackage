#include "slsReceiverUsers.h"
#include "slsReceiver_funcs.h"

slsReceiverFuncs* slsReceiverUsers::receiver(NULL);

slsReceiverUsers::slsReceiverUsers(int argc, char *argv[], int &success) {
	slsReceiverUsers::receiver=new slsReceiverFuncs(argc, argv, success);
}

slsReceiverUsers::~slsReceiverUsers() {
  delete slsReceiverUsers::receiver;
}

void slsReceiverUsers::start() {
	slsReceiverUsers::receiver->start();
}


void slsReceiverUsers::closeFile(int p) {
	slsReceiverUsers::receiver->closeFile(p);
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


