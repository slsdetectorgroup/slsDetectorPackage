/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
#include "slsReceiverUsers.h"


#include <signal.h>	//SIGINT
#include <cstdlib>	//EXIT

#include <iostream>
using namespace std;


void closeFile(int p){
  cout<<"close file in receiver"<<endl;
  slsReceiverUsers::closeFile(p);
  exit(0);
}


int StartAcq(char* filepath, char*filename,int fileindex, int datasize, void*p){
  printf("--StartAcq:  filepath:%s  filename:%s fileindex:%d  datasize:%d\n",
	 filepath, filename, fileindex, datasize);

  printf("--StartAcq: returning 0\n");
  return 0;
}


void AcquisitionFinished(int frames, void*p){
  printf("AcquisitionFinished: frames:%d why ?\n",frames);

}


void GetData(int framenum, char* datapointer, FILE* descriptor, char* gui, void* p){
  printf("GetData: framenum: %d(%d)\n", framenum, *(int*)datapointer);

}



int main(int argc, char *argv[]) {
  int ret = slsDetectorDefs::OK;
  
  slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
  
  if(ret==slsDetectorDefs::FAIL)
    return -1;
  
  
  //Catch signal SIGINT to close files properly
  signal(SIGINT,closeFile);
  
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




  receiver->start();
  
  
  return 0;
}

