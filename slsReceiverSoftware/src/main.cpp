/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include <iostream>
#include <string.h>
#include  <signal.h>	//SIGINT

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

/*
int startAcquisitionCallBack(char* filePath, char* fileName, int fileIndex, int bufferSize, void* context) {
  cout << "#### startAcquisitionCallBack ####" << endl;
  cout << "* filePath: " << filePath << endl;
  cout << "* fileName: " << fileName << endl;
  cout << "* fileIndex: " << fileIndex << endl;
  cout << "* bufferSize: " << bufferSize << endl;
  return 1;
}

void acquisitionFinishedCallBack(int totalFramesCaught, void* context) {
  cout << "#### acquisitionFinishedCallBack ####" << endl;
  cout << "* totalFramesCaught: " << totalFramesCaught << endl;
}

void rawDataReadyCallBack(int currFrameNum, char* dataPointer, int dataSize, FILE* file, char* guiDataPointer, void* context) {
  cout << "#### rawDataReadyCallBack ####" << endl;
  cout << "* currFrameNum: " << currFrameNum << endl;
  cout << "* dataSize: " << dataSize << endl;
}
*/

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
	//receiver->registerCallBackStartAcquisition(startAcquisitionCallBack,NULL);


	/**
	  callback argument is
	  total farmes caught
	  registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	 */
	//receiver->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,NULL);


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
	//receiver->registerCallBackRawDataReady(rawDataReadyCallBack,NULL);



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

	deleteReceiver(receiver);
	cout << "Goodbye!" << endl;
	return 0;
}

