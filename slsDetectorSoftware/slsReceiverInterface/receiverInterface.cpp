#include "receiverInterface.h"
#include "sls_detector_defs.h"


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
   set or read the acquisition timers
   enum timerIndex {
   FRAME_NUMBER,
   ACQUISITION_TIME,
   FRAME_PERIOD,
   DELAY_AFTER_TRIGGER,
   GATES_NUMBER,
   PROBES_NUMBER
   CYCLES_NUMBER,
   GATE_INTEGRATED_TIME
   }
		 */
		/*
int64_t receiverInterface::setTimer(timerIndex index, int64_t t){


  int fnum=F_SET_TIMER;
  int64_t retval;
  uint64_t ut;
  char mess[100];
  int ret=OK;
  int n=0;

  if (index!=MEASUREMENTS_NUMBER) {


#ifdef VERBOSE
    std::cout<< "Setting timer  "<< index << " to " <<  t << "ns" << std::endl;
#endif
    ut=t;
    if (thisDetector->onlineFlag==ONLINE_FLAG) {
      if (controlSocket) {
	if  (controlSocket->Connect()>=0) {
	  controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	  controlSocket->SendDataOnly(&index,sizeof(index));
	  n=controlSocket->SendDataOnly(&t,sizeof(t));
	  controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	  if (ret==slsDetectorDefs::FAIL) {
	    controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	    std::cout<< "Detector returned error: " << mess << std::endl;
	  } else {
	    controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	    thisDetector->timerValue[index]=retval;
	  }
	  controlSocket->Disconnect();
	  if (ret==FORCE_UPDATE) {
	    updateDetector();
#ifdef VERBOSE
	    std::cout<< "Updated!" << std::endl;
#endif

	  }
	}
      }
    } else {
      //std::cout<< "offline " << std::endl;
      if (t>=0)
	thisDetector->timerValue[index]=t;
    }
  } else {
    if (t>=0)
      thisDetector->timerValue[index]=t;
  }
#ifdef VERBOSE
  std::cout<< "Timer " << index << " set to  "<< thisDetector->timerValue[index] << "ns"  << std::endl;
#endif
  if (index==PROBES_NUMBER) {
    setDynamicRange();
    //cout << "Changing probes: data size = " << thisDetector->dataBytes <<endl;
  }

  // set progress
  if ((index==FRAME_NUMBER) || (index==CYCLES_NUMBER)) {

    setTotalProgress();


  }

  return thisDetector->timerValue[index];

};

		 */






		/*



int64_t receiverInterface::getTimeLeft(timerIndex index){


  int fnum=F_GET_TIME_LEFT;
  int64_t retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Getting  timer  "<< index <<  std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
      if  (stopSocket->Connect()>=0) {
	stopSocket->SendDataOnly(&fnum,sizeof(fnum));
	stopSocket->SendDataOnly(&index,sizeof(index));
	stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL) {
	  stopSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
	}
	stopSocket->Disconnect();
      }
    }
  }
#ifdef VERBOSE
  std::cout<< "Time left is  "<< retval << std::endl;
#endif
  return retval;

};







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


