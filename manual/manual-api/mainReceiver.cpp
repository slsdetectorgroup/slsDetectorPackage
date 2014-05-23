/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
*/

/**
\file mainReceiver.cpp


This file is an example of how to implement the slsDetectorUsers class 
You can compile it linking it to the slsDetector library

gcc mainReceiver.cpp -L lib -l SlsDetector -lm -lpthread

where lib is the location of libSlsDetector.so

*/

#include "slsReceiverUsers.h"


#include <iostream>
using namespace std;


int main(int argc, char *argv[]) {
  int ret = 0;


  /*
      Instantiate the slsReceieverUsers class
      The port number is passed as an argument
*/
  slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
  
  /*
      return if could not open TCP socket for interfacing to client 
  */
  if(ret==1)
    return -1;
  

  /*register callbacks */


	/*
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


	/*
	  callback argument is
	  total farmes caught
	  registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	*/
	
	
	//receiver->registerCallBackAcquisitionFinished(func,arg);
	


	/*
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


  /* start receiver to listen for commands from the client (and data from detectors when expected */

  receiver->start();
  
  
  return 0;
}

