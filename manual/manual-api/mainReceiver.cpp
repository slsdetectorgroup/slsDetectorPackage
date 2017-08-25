/**
 \file mainReceiver.cpp

This file is an example of how to implement the slsReceiverUsers class
You can compile it linking it to the slsReceiver library

g++ mainReceiver.cpp -L lib -lSlsReceiver -L/usr/lib64/ -L lib2 -lzmq  -pthread -lrt -lm -lstdc++

where,

lib is the location of lSlsReceiver.so

lib2 is the location of the libzmq.a.
[ libzmq.a is required only when using data call backs and enabling data streaming from receiver to client.
It is linked in manual/manual-api from slsReceiverSoftware/include ]

 */

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

/* Define Number of receivers */
#define NUM_RECEIVERS 	1
/** Define TCP Port of First Receiver, others are incremented by 1 */
#define START_TCP_PORT	1954
/** Define Colors to print data call back in different colors for different recievers */
#define PRINT_IN_COLOR(c,f, ...) 	printf ("\033[%dm" f RESET, 30 + c+1, ##__VA_ARGS__)

/* List of process IDs of all child receiver processes */
pid_t childPid[NUM_RECEIVERS];
/** Variable is true to continue running, set to false upon interrupt */
bool keeprunning;
/** Variable indicating number of child processes running */
int numrunning;


/**
 * Child Exit Signal Interrupt Handler
 * When a child process exits, this function is called,
 * blocks until all child processes exit &
 * decreases the variable indicating number of running processes
 * @param sig signal enum
 */
void sigChildExitedHandler(int sig) {
	pid_t pid = wait(NULL);
	bprintf(GRAY, "\nChild Process Pid %d exited.\n", pid);
	numrunning--;
}

/**
 * Control+C Interrupt Handler
 * Sets the variable keeprunning to false, to let all the processes know to exit properly
 */
void sigInterruptHandler(int p){
	keeprunning = false;
}

/**
 * Start Acquisition Call back
 * slsReceiver writes data if file write enabled.
 * Users get data to write using call back if registerCallBackRawDataReady is registered.
 * @param filepath file path
 * @param filename file name
 * @param fileindex file index
 * @param datasize data size in bytes
 * @param p pointer to object
 * \returns ignored
 */
int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
	bprintf(BLUE, "#### StartAcq:  filepath:%s  filename:%s fileindex:%llu  datasize:%u ####\n",
			filepath, filename, fileindex, datasize);

	bprintf(BLUE, "--StartAcq: returning 0\n");
	return 0;
}

/**
 * Acquisition Finished Call back
 * @param frames Number of frames caught
 * @param p pointer to object
 */
void AcquisitionFinished(uint64_t frames, void*p){
	bprintf(BLUE, "#### AcquisitionFinished: frames:%llu ####\n",frames);
}

/**
 * Get Receiver Data Call back
 * Prints in different colors(for each receiver process) the different headers for each image call back.
 * @param frameNumber frame number
 * @param expLength real time exposure length (in 100ns) or sub frame number (Eiger 32 bit mode only)
 * @param packetNumber number of packets caught for this frame
 * @param bunchId bunch id from beamline
 * @param timestamp time stamp  in 10MHz clock (not implemented for most)
 * @param modId module id	(not implemented for most)
 * @param xCoord x coordinates (detector id in 1D)
 * @param yCoord y coordinates (not implemented)
 * @param zCoord z coordinates (not implemented)
 * @param debug debug values if any
 * @param roundRNumber (not implemented)
 * @param detType detector type see :: detectorType
 * @param version version of standard header (structure format)
 * @param datapointer pointer to data
 * @param datasize data size in bytes
 * @param p pointer to object
 */
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


/**
 * Example of main program using the slsReceiverUsers class
 *
 * - Defines in file for:
 *  	- Default Number of receivers is 1
 *  	- Default Start TCP port is 1954
 */
int main(int argc, char *argv[]) {

	/**	- set default values: child process pid values to -1, keeprunning to true, numrunning to 0 */
	for (int i = 0; i < NUM_RECEIVERS; ++i)
		childPid[i] = -1;
	keeprunning = true;
	numrunning = 0;

	/** - Catch signal SIGINT to close files and call destructors properly */
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SIGINT\n");
	}

	/** - wait for all the SIGCHILD signals and decrease numrunningeach time a child process exits*/
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=sigChildExitedHandler;	// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGCHLD, &asa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SICHILD\n");
	}


	/** - loop over number of receivers */
	for (int i = 0; i < NUM_RECEIVERS; ++i) {

		/**	- fork process to create child process */
		childPid[i] = fork();

		/**	- if fork failed, raise SIGINT and kill all receiver objects */
		if (childPid[i] < 0) {
			bprintf(RED,"fork() failed. Killing all the receiver objects\n");
			raise(SIGINT);
		}

		/**	- if child process */
		else if (childPid[i] == 0) {
			bprintf(BLUE,"Starting Receiver %d with pid %ld\n", i, (long)getpid());

			char temp[10];
			sprintf(temp,"%d",START_TCP_PORT + i);
			char* args[] = {(char*)"ignored", (char*)"--rx_tcpport", temp};
			int ret = slsReceiverDefs::OK;
			/**	-  create slsReceiverUsers object with appropriate arguments
			  (START_TCP_PORT incrementing by 1 */
			slsReceiverUsers *receiver = new slsReceiverUsers(3, args, ret);
			if(ret==slsReceiverDefs::FAIL){
				delete receiver;
				exit(EXIT_FAILURE);
			}

			/**	- register callbacks. remember to set file write enable to 0 (using the client)
			  if we should not write files and you will write data using the callbacks */

			/** - Call back for start acquisition */
			bprintf(BLUE, "Registering 	StartAcq()\n");
			receiver->registerCallBackStartAcquisition(StartAcq, NULL);

			/** - Call back for acquisition finished */
			bprintf(BLUE, "Registering 	AcquisitionFinished()\n");
			receiver->registerCallBackAcquisitionFinished(AcquisitionFinished, NULL);

			/* 	- Call back for raw data */
			bprintf(BLUE, "Registering     GetData() \n");
			receiver->registerCallBackRawDataReady(GetData,NULL);


			/**	- start tcp server thread */
			if (receiver->start() == slsReceiverDefs::FAIL){
				delete receiver;
				exit(EXIT_FAILURE);
			}

			/**	- as long as keeprunning is true, usleep for a second */
			while(keeprunning)
				usleep(1 * 1000 * 1000);
			/**	- interrupt caught, delete slsReceiverUsers object and exit */
			delete receiver;
			exit(EXIT_SUCCESS);
		}

		/**		- parent process, increment number of running processes */
		else
			numrunning++;

	}

	/** - Print Ready and Instructions how to exit */
	cout << "Ready ... " << endl;
	bprintf(GRAY, "\n[ Press \'Ctrl+c\' to exit ]\n");

	/** - Parent process waits for all child processes to exit by sleeping till numrunning is 0 */
	while(numrunning)
		usleep(1 * 1000 * 1000);
	cout << "Goodbye!" << endl;
	return 0;
}

