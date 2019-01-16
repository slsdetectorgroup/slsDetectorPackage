/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
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

/** Define Colors to print data call back in different colors for different recievers */
/*
#define PRINT_IN_COLOR(c,f, ...) 	printf ("\033[%dm" f RESET, 30 + c+1, ##__VA_ARGS__)

int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
  FILE_LOG(logINFO) << "#### StartAcq:  "
		  "filepath: " << filepath << "filename: " << filename <<
		  "fileindex: " << fileindex << "datasize: " << datasize << " ####";
  FILE_LOG(logINFO) << "--StartAcq: returning 0";
  return 0;
}


void AcquisitionFinished(uint64_t frames, void*p){
	 FILE_LOG(logINFO) << "#### AcquisitionFinished: frames:" << frames << " ####";
}


void GetData(char* metadata, char* datapointer, uint32_t datasize, void* p){
	slsDetectorDefs::sls_receiver_header* header = (slsDetectorDefs::sls_receiver_header*)metadata;
	slsDetectorDefs::sls_detector_header detectorHeader = header->detHeader;

	PRINT_IN_COLOR (detectorHeader.modId?detectorHeader.modId:detectorHeader.row,
			"#### %d GetData: ####\n"
			"frameNumber: %lu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %lu"
			"\t\ttimestamp: %lu\t\tmodId: %u\t\t"
			"row: %u\t\tcolumn: %u\t\treserved: %u\t\tdebug: %u"
			"\t\troundRNumber: %u\t\tdetType: %u\t\tversion: %u"
			//"\t\tpacketsMask:%s"
			"\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
			detectorHeader.row, (long unsigned int)detectorHeader.frameNumber,
			detectorHeader.expLength, detectorHeader.packetNumber, (long unsigned int)detectorHeader.bunchId,
			(long unsigned int)detectorHeader.timestamp, detectorHeader.modId,
			detectorHeader.row, detectorHeader.column, detectorHeader.reserved,
			detectorHeader.debug, detectorHeader.roundRNumber,
			detectorHeader.detType, detectorHeader.version,
			//header->packetsMask.to_string().c_str(),
            ((uint8_t)(*((uint8_t*)(datapointer)))), datasize);
}
*/


int main(int argc, char *argv[]) {

	keeprunning = true;
	FILE_LOG(logINFOBLUE) << "Created [ Tid: " << syscall(SYS_gettid) << " ]";

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, nullptr) == -1) {
		FILE_LOG(logERROR) << "Could not set handler function for SIGINT";
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
		FILE_LOG(logERROR) << "Could not set handler function for SIGPIPE";
	}


	int ret = slsDetectorDefs::OK;
	slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
	if(ret==slsDetectorDefs::FAIL){
		delete receiver;
		FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
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
	if (receiver->start() == slsDetectorDefs::FAIL){
		delete receiver;
		FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
		exit(EXIT_FAILURE);
	}

	FILE_LOG(logINFO) << "Ready ... ";
	FILE_LOG(logINFO) << "[ Press \'Ctrl+c\' to exit ]";
	while(keeprunning)
		pause();

	delete receiver;
	FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
	FILE_LOG(logINFO) << "Exiting Receiver";
	return 0;
}

