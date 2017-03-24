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
	
void slsReceiverUsers::registerCallBackRawDataReady(void (*func)(uint64_t frameNumber, uint32_t expLength, uint32_t packetNumber, uint64_t bunchId, uint64_t timestamp,
		uint16_t modId, uint16_t xCoord, uint16_t yCoord, uint16_t zCoord, uint32_t debug, uint16_t roundRNumber, uint8_t detType, uint8_t version,
		char* datapointer, uint32_t datasize, void*), void *arg){
	receiver->registerCallBackRawDataReady(func,arg);
}

