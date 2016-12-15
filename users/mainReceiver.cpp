/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include <iostream>
#include <string.h>
#include  <signal.h>	//SIGINT
#include <cstdlib>		//system

#include "utilities.h"
#include "logger.h"
using namespace std;

slsReceiverUsers *receiver;

void deleteReceiver(slsReceiverUsers* r){
	if(r){delete r;r=0;}
}

void closeFile(int p){
	deleteReceiver(receiver);
}




int StartAcq(char* filepath, char*filename,int fileindex, int datasize, void*p){
  printf("--StartAcq:  filepath:%s  filename:%s fileindex:%d  datasize:%d\n",
	 filepath, filename, fileindex, datasize);

  printf("--StartAcq: returning 0\n");
  return 0;
}


void AcquisitionFinished(int frames, void*p){
  printf("AcquisitionFinished: frames:%d \n",frames);

}


void GetData(int framenum, char* datapointer, int datasize, FILE* descriptor, char* gui, void* p){
  printf("GetData: framenum: %d(%d)\n", framenum, *(int*)datapointer);

}



int main(int argc, char *argv[]) {

	//Catch signal SIGINT to close files properly
	signal(SIGINT,closeFile);

	int ret = slsReceiverDefs::OK;
	receiver = new slsReceiverUsers(argc, argv, ret);

	if(ret==slsReceiverDefs::FAIL){
		deleteReceiver(receiver);
		return -1;
	}
  


  //register callbacks 


	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   datasize
	   
	   return value is 
	   0 raw data ready callback takes care of open,close,write file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything

	   registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg);
	*/
  	//receiver->registerCallBackStartAcquisition(func,arg);

  	  printf("Registering 	StartAcq()\n");
  	receiver->registerCallBackStartAcquisition(StartAcq, NULL);
	


	/**
	  callback argument is
	  total farmes caught
	  registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	*/
	//receiver->registerCallBackAcquisitionFinished(func,arg);

  	  printf("Registering 	AcquisitionFinished()\n");
  	receiver->registerCallBackAcquisitionFinished(AcquisitionFinished, NULL);


	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)
	  
	  NEVER DELETE THE DATA POINTER
	  REMEMBER THAT THE CALLBACK IS BLOCKING

	  registerCallBackRawDataReady(void (*func)(int, char*, FILE*, char*, void*),void *arg);

	*/
	//receiver->registerCallBackRawDataReady(func,arg);

  	  printf("Registering     GetData() \n");
  	receiver->registerCallBackRawDataReady(GetData,NULL);


  	  /* start receiver to listen for commands from the client (and data from detectors when expected */
  	receiver->start();

	//start tcp server thread
	if(receiver->start() == slsReceiverDefs::OK){
		FILE_LOG(logDEBUG1) << "DONE!" << endl;
		string str;
		cin>>str;
		//wait and look for an exit keyword
		while(str.find("exit") == string::npos)
			cin>>str;
		//stop tcp server thread, stop udp socket
		receiver->stop();
	}
  
  
  return 0;
}

