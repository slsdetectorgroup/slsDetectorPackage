/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include <iostream>
#include <string.h>
#include <signal.h>	//SIGINT
#include <cstdlib>		//system

#include "utilities.h"
#include "logger.h"

#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <syscall.h>
using namespace std;


bool keeprunning;

void sigInterruptHandler(int p){
	keeprunning = false;
}

/*
int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
  printf("#### StartAcq:  filepath:%s  filename:%s fileindex:%llu  datasize:%u ####\n",
	 filepath, filename, fileindex, datasize);

  cprintf(BLUE, "--StartAcq: returning 0\n");
  return 0;
}


void AcquisitionFinished(uint64_t frames, void*p){
  cprintf(BLUE, "#### AcquisitionFinished: frames:%llu ####\n",frames);
}


void GetData(uint64_t frameNumber, uint32_t expLength, uint32_t packetNumber, uint64_t bunchId, uint64_t timestamp,
		uint16_t modId, uint16_t xCoord, uint16_t yCoord, uint16_t zCoord, uint32_t debug, uint16_t roundRNumber, uint8_t detType, uint8_t version,
		char* datapointer, uint32_t datasize, void* p){

	PRINT_IN_COLOR (xCoord,
			"#### %d GetData: ####\n"
			"frameNumber: %llu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %llu\t\ttimestamp: %llu\t\tmodId: %u\t\t"
			"xCoord: %u\t\tyCoord: %u\t\tzCoord: %u\t\tdebug: %u\t\troundRNumber: %u\t\tdetType: %u\t\t"
			"version: %u\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
			xCoord, frameNumber, expLength, packetNumber, bunchId, timestamp, modId,
			xCoord, yCoord, zCoord, debug, roundRNumber, detType, version,
			((uint8_t)(*((uint8_t*)(datapointer)))), datasize);

}
*/


int main(int argc, char *argv[]) {

	keeprunning = true;
	bprintf(BLUE,"Created [ Tid: %ld ]\n", (long)syscall(SYS_gettid));

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SIGINT\n");
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SIGCHILD\n");
	}


	int ret = slsReceiverDefs::OK;
	slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
	if(ret==slsReceiverDefs::FAIL){
		delete receiver;
		bprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
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
	if (receiver->start() == slsReceiverDefs::FAIL){
		delete receiver;
		bprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
	}

	FILE_LOG(logINFO) << "Ready ... ";
	bprintf(GRAY, "\n[ Press \'Ctrl+c\' to exit ]\n");
	while(keeprunning)
		usleep(5 * 1000 * 1000);

	delete receiver;
	bprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
	FILE_LOG(logINFO) << "Goodbye!";
	return 0;
}

