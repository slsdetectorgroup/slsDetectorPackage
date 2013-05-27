#include "slsReceiverUsers.h"
#include "slsReceiver_funcs.h"

slsReceiverUsers::slsReceiverUsers(int argc, char *argv[], int &success) {
  receiver=new slsReceiverFuncs(argc, argv, success);
}

slsReceiverUsers::~slsReceiverUsers() {
  delete receiver;
}

void slsReceiverUsers::start() {
  receiver->start();
}


void slsReceiverUsers::closeFile(int p) {
  slsReceiverFuncs::closeFile(0);
}


void slsReceiverUsers::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
  receiver->registerCallBackStartAcquisition(func,arg);
}

  

void slsReceiverUsers::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
  receiver->registerCallBackAcquisitionFinished(func,arg);
}
	

void slsReceiverUsers::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
  receiver->registerCallBackRawDataReady(func,arg);
}


