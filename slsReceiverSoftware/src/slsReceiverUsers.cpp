#include "container_utils.h" // For sls::make_unique<>

#include "slsReceiverUsers.h"

slsReceiverUsers::slsReceiverUsers(int argc, char *argv[], int &success) {
	// catch the exception here to limit it to within the library (for current version)
	try {
		receiver = sls::make_unique<slsReceiver>(argc, argv);
		success = slsDetectorDefs::OK;
	} catch (...) {
		success = slsDetectorDefs::FAIL;
	}
}

slsReceiverUsers::slsReceiverUsers(int tcpip_port_no) {
	receiver = sls::make_unique<slsReceiver>(tcpip_port_no);
}

int slsReceiverUsers::start() {
	return receiver->start();
}

void slsReceiverUsers::stop() {
	receiver->stop();
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

void slsReceiverUsers::registerCallBackRawDataReady(void (*func)(char* header,
		char* datapointer, uint32_t datasize, void*), void *arg){
	receiver->registerCallBackRawDataReady(func,arg);
}

void slsReceiverUsers::registerCallBackRawDataModifyReady(void (*func)(char* header,
		char* datapointer, uint32_t& revDatasize, void*), void *arg){
	receiver->registerCallBackRawDataModifyReady(func,arg);
}
