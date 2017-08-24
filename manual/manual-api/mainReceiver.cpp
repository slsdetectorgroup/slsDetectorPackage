/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include <iostream>
#include <string.h>
#include <signal.h>	//SIGINT
#include <cstdlib>		//system

//#include "utilities.h"
//#include "logger.h"

#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait

#include <string>
using namespace std;

#define NUM_RECEIVERS 	1
#define START_TCP_PORT	1954
#define PRINT_IN_COLOR(c,f, ...) 	printf ("\033[%dm" f RESET, 30 + c+1, ##__VA_ARGS__)


pid_t childPid[NUM_RECEIVERS];
bool keeprunning;
int numrunning;



void sigChildExitedHandler(int sig) {
  pid_t pid = wait(NULL);
  bprintf(GRAY, "\nChild Process Pid %d exited.\n", pid);
  numrunning--;
}


void sigInterruptHandler(int p){
	keeprunning = false;
}

int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
  bprintf(BLUE, "#### StartAcq:  filepath:%s  filename:%s fileindex:%llu  datasize:%u ####\n",
	 filepath, filename, fileindex, datasize);

  bprintf(BLUE, "--StartAcq: returning 0\n");
  return 0;
}


void AcquisitionFinished(uint64_t frames, void*p){
  bprintf(BLUE, "#### AcquisitionFinished: frames:%llu ####\n",frames);
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



int main(int argc, char *argv[]) {

	// set default child process pid values
	for (int i = 0; i < NUM_RECEIVERS; ++i)
		childPid[i] = -1;

	keeprunning = true;
	numrunning = 0;

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SIGINT\n");
	}

	// wait for all the SIGCHILD signals
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=sigChildExitedHandler;	// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGCHLD, &asa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SICHILD\n");
	}


	int narg= 3;

	for (int i = 0; i < NUM_RECEIVERS; ++i) {

		childPid[i] = fork();

		// fork failed
		if (childPid[i] < 0) {
			bprintf(RED,"fork() failed. Killing all the receiver objects\n");
			raise(SIGINT);
		}

		// child process
		else if (childPid[i] == 0) {
			bprintf(BLUE,"Starting Receiver %d with pid %ld\n", i, (long)getpid());

			char temp[10];
			sprintf(temp,"%d",START_TCP_PORT + i);
			char* args[] = {(char*)"ignored", (char*)"--rx_tcpport", temp};
			int ret = slsReceiverDefs::OK;
			slsReceiverUsers *receiver = new slsReceiverUsers(narg, args, ret);
			if(ret==slsReceiverDefs::FAIL){
				delete receiver;
				exit(EXIT_FAILURE);
			}

			//register callbacks
			//remember to set file write enable to 0 (using the client) if we should not write files and
			//you will write data using the callbacks

			/**
			 * Call back for start acquisition
			 * callback arguments are
			 * filepath
			 * filename
			 * fileindex
			 * datasize
			 *
			 * return value is insignificant at the moment
			 * we write depending on file write enable
			 * users get data to write depending on call backs registered
			 */
			bprintf(BLUE, "Registering 	StartAcq()\n");
			receiver->registerCallBackStartAcquisition(StartAcq, NULL);

			/**
			 * Call back for acquisition finished
			 * callback argument is
			 * total frames caught
			 */
			bprintf(BLUE, "Registering 	AcquisitionFinished()\n");
			receiver->registerCallBackAcquisitionFinished(AcquisitionFinished, NULL);

			/**
			 * Call back for raw data
			 * args to raw data ready callback are
			 * frameNumber is the frame number
			 * expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
			 * packetNumber is the packet number
			 * bunchId is the bunch id from beamline
			 * timestamp is the time stamp with 10 MHz clock
			 * modId is the unique module id (unique even for left, right, top, bottom)
			 * xCoord is the x coordinate in the complete detector system
			 * yCoord is the y coordinate in the complete detector system
			 * zCoord is the z coordinate in the complete detector system
			 * debug is for debugging purposes
			 * roundRNumber is the round robin set number
			 * detType is the detector type see :: detectorType
			 * version is the version number of this structure format
			 * dataPointer is the pointer to the data
			 * dataSize in bytes is the size of the data in bytes
			 */
			bprintf(BLUE, "Registering     GetData() \n");
			receiver->registerCallBackRawDataReady(GetData,NULL);


			//start tcp server thread
			if (receiver->start() == slsReceiverDefs::FAIL){
				delete receiver;
				exit(EXIT_FAILURE);
			}

			while(keeprunning)
				usleep(1 * 1000 * 1000);
			delete receiver;
			exit(EXIT_SUCCESS);
		}

		// parent process
		else
			numrunning++;

	}

	cout << "Ready ... " << endl;
	bprintf(GRAY, "\n[ Press \'Ctrl+c\' to exit ]\n");

	// wait for all child processes to exit
	while(numrunning)
		usleep(1 * 1000 * 1000);
	cout << "Goodbye!" << endl;
	return 0;
}

