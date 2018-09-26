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
#include <unistd.h> 	//usleep
#include <syscall.h>


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


void GetData(char* metadata, char* datapointer, uint32_t datasize, void* p){
	slsReceiverDefs::sls_receiver_header* header = (slsReceiverDefs::sls_receiver_header*)metadata;
	slsReceiverDefs::sls_detector_header detectorHeader = header->detHeader;

	PRINT_IN_COLOR (detectorHeader.modId?detectorHeader.modId:detectorHeader.row,
			"#### %d GetData: ####\n"
			"frameNumber: %llu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %llu"
			"\t\ttimestamp: %llu\t\tmodId: %u\t\t"
			"xCrow%u\t\tcolumn: %u\t\tcolumn: %u\t\tdebug: %u"
			"\t\troundRNumber: %u\t\tdetType: %u\t\tversion: %u"
			//"\t\tpacketsMask:%s"
			"\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
			detectorHeader.row, detectorHeader.frameNumber,
			detectorHeader.expLength, detectorHeader.packetNumber, detectorHeader.bunchId,
			detectorHeader.timestamp, detectorHeader.modId,
			detectorHeader.row, detectorHeader.column, detectorHeader.column,
			detectorHeader.debug, detectorHeader.roundRNumber,
			detectorHeader.detType, detectorHeader.version,
			//header->packetsMask.to_string().c_str(),
            ((uint8_t)(*((uint8_t*)(datapointer)))), datasize);
}
*/


int main(int argc, char *argv[]) {

	keeprunning = true;
	cprintf(BLUE,"Created [ Tid: %ld ]\n", (long)syscall(SYS_gettid));

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGINT\n");
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGPIPE\n");
	}


	int ret = slsReceiverDefs::OK;
	slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
	if(ret==slsReceiverDefs::FAIL){
		delete receiver;
		cprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
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
	  sls_receiver_header frame metadata
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)
	  NEVER DELETE THE DATA POINTER
	  REMEMBER THAT THE CALLBACK IS BLOCKING
	  registerCallBackRawDataReady(void (*func)(char*, char*, uint32_t, void*),void *arg);
	 */
	//receiver->registerCallBackRawDataReady(rawDataReadyCallBack,NULL);



	//start tcp server thread
	if (receiver->start() == slsReceiverDefs::FAIL){
		delete receiver;
		cprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
	}

	FILE_LOG(logINFO) << "Ready ... ";
	cprintf(RESET, "\n[ Press \'Ctrl+c\' to exit ]\n");
	while(keeprunning)
		pause();

	delete receiver;
	cprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
	FILE_LOG(logINFO) << "Goodbye!";
	return 0;
}

