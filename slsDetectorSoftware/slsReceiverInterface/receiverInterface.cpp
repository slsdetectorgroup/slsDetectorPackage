#include "receiverInterface.h"


#include  <sys/types.h>
#include  <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bitset>
#include <cstdlib>
#include <iostream>



receiverInterface::receiverInterface(MySocketTCP *socket):dataSocket(socket){}



receiverInterface::~receiverInterface(){
	delete dataSocket;
}



int receiverInterface::sendString(int fnum, char retval[], char arg[]){
	int ret = slsDetectorDefs::FAIL;
	char mess[100] = "";

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->SendDataOnly(arg,MAX_STR_LENGTH);
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==slsDetectorDefs::FAIL){
				dataSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
			}
			dataSocket->ReceiveDataOnly(retval,MAX_STR_LENGTH);
		}
		dataSocket->Disconnect();
	}
	return ret;
}



int receiverInterface::sendUDPDetails(int fnum, char retval[], char arg[2][MAX_STR_LENGTH]){
	char args[2][MAX_STR_LENGTH];
	int ret = slsDetectorDefs::FAIL;
	char mess[100] = "";

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->SendDataOnly(arg,sizeof(args));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==slsDetectorDefs::FAIL){
				dataSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
			}
			else
				dataSocket->ReceiveDataOnly(retval,MAX_STR_LENGTH);
		}
		dataSocket->Disconnect();
	}
	return ret;
}


int receiverInterface::sendInt(int fnum, int &retval, int arg){
	int ret = slsDetectorDefs::FAIL;
	char mess[100] = "";

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->SendDataOnly(&arg,sizeof(arg));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==slsDetectorDefs::FAIL){
				dataSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
			}
			dataSocket->ReceiveDataOnly(&retval,sizeof(retval));
		}
		dataSocket->Disconnect();
	}
	return ret;
}



int receiverInterface::getInt(int fnum, int &retval){
	int ret = slsDetectorDefs::FAIL;

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			dataSocket->ReceiveDataOnly(&retval,sizeof(retval));
		}
		dataSocket->Disconnect();
	}
	return ret;
}



int receiverInterface::getInt(int fnum, int64_t &retval){
	int ret = slsDetectorDefs::FAIL;

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			dataSocket->ReceiveDataOnly(&retval,sizeof(retval));
		}
		dataSocket->Disconnect();
	}
	return ret;
}


int receiverInterface::getLastClientIP(int fnum, char retval[]){
	int ret = slsDetectorDefs::FAIL;

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			dataSocket->ReceiveDataOnly(retval,sizeof(retval));
			dataSocket->Disconnect();
		}
	}
	return ret;
}



int receiverInterface::executeFunction(int fnum){

	int ret = slsDetectorDefs::FAIL;
	char mess[100] = "";

	if (dataSocket) {
		if  (dataSocket->Connect()>=0) {
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==slsDetectorDefs::FAIL){
				dataSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
			}
			dataSocket->Disconnect();
		}
	}
	return ret;
}


/*

int receiverInterface::exitServer(){

  int retval;
  int fnum=F_EXIT_SERVER;

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      controlSocket->Connect();
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      controlSocket->Disconnect();
    }
  }
  if (retval!=OK) {
    std::cout<< std::endl;
    std::cout<< "Shutting down the server" << std::endl;
    std::cout<< std::endl;
  }
  return retval;

};

		 */


