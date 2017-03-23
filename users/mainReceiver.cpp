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

#include <string>
using namespace std;

#define NUM_RECEIVERS 	2
#define START_TCP_PORT	1954
#define PRINT_IN_COLOR(c,f, ...) 	printf ("\033[%dm" f RESET, 30 + c+1, ##__VA_ARGS__)

slsReceiverUsers *receivers[NUM_RECEIVERS];

void deleteReceiver(slsReceiverUsers* r[]){
	for (int i = 0; i < NUM_RECEIVERS; ++i)
	if (r[i]) {
		delete r[i];
		r[i] = 0;
	}
}

void closeFile(int p){
	deleteReceiver(receivers);
}




int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
  printf("--StartAcq:  filepath:%s  filename:%s fileindex:%llu  datasize:%u\n",
	 filepath, filename, fileindex, datasize);

  printf("--StartAcq: returning 0\n");
  return 0;
}


void AcquisitionFinished(uint64_t frames, void*p){
  printf("AcquisitionFinished: frames:%llu \n",frames);

}


void GetData(uint64_t frameNumber, uint32_t expLength, uint32_t packetNumber, uint64_t bunchId, uint64_t timestamp,
		uint16_t modId, uint16_t xCoord, uint16_t yCoord, uint16_t zCoord, uint32_t debug, uint16_t roundRNumber, uint8_t detType, uint8_t version,
		char* datapointer, uint32_t datasize, void* p){

	PRINT_IN_COLOR (xCoord,
			"%d GetData: \n"
			"frameNumber: %llu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %llu\t\ttimestamp: %llu\t\tmodId: %u\t\t"
			"xCoord: %u\t\tyCoord: %u\t\tzCoord: %u\t\tdebug: %u\t\troundRNumber: %u\t\tdetType: %u\t\t"
			"version: %u\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
			xCoord, frameNumber, expLength, packetNumber, bunchId, timestamp, modId,
			xCoord, yCoord, zCoord, debug, roundRNumber, detType, version,
			((uint8_t)(*((uint8_t*)(datapointer)))), datasize);

}



int main(int argc, char *argv[]) {

	//Catch signal SIGINT to close files properly
	signal(SIGINT,closeFile);

	int ret = slsReceiverDefs::OK;
	int narg= 3;

	for (int i = 0; i < NUM_RECEIVERS; ++i) {

		char temp[10];
		sprintf(temp,"%d",START_TCP_PORT + i);
		char* args[] = {(char*)"ignored", (char*)"--rx_tcpport", temp};

		cprintf(BLUE,"Starting Receiver %d\n", i);
		receivers[i] = new slsReceiverUsers(narg, args, ret);
		if(ret==slsReceiverDefs::FAIL){
			deleteReceiver(receivers);
			return -1;
		}

		//register callbacks

		/**
		 * Call back for start acquisition
		 * callback arguments are
		 * filepath
		 * filename
		 * fileindex
		 * datasize
		 *
		 * return value is
		 * 0 callback takes care of open,close,wrie file
		 * 1 callback writes file, we have to open, close it
		 * 2 we open, close, write file, callback does not do anything
		 */
		printf("Registering 	StartAcq()\n");
		receivers[i]->registerCallBackStartAcquisition(StartAcq, NULL);

		/**
		 * Call back for acquisition finished
		 * callback argument is
		 * total frames caught
		 */
		printf("Registering 	AcquisitionFinished()\n");
		receivers[i]->registerCallBackAcquisitionFinished(AcquisitionFinished, NULL);

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
		 * fileDescriptor is the file descriptor
		 */
		printf("Registering     GetData() \n");
		receivers[i]->registerCallBackRawDataReady(GetData,NULL);


		//start tcp server thread
		ret = receivers[i]->start();
		if(ret == slsReceiverDefs::FAIL){
			for (int i = 0; i < i; ++i)
				receivers[i]->stop();
			deleteReceiver(receivers);
			return -1;
		}
	}


	FILE_LOG(logDEBUG1) << "DONE!" << endl;
	cprintf( BLUE, "Type \'q\' to exit\n");
	string str;
	cin>>str;
	//wait and look for an exit keyword
	while(str.find("q") == string::npos)
		cin>>str;
	//stop tcp server thread, stop udp socket
	for (int i = 0; i < NUM_RECEIVERS; ++i) {
		cprintf(BLUE,"Stopping Receiver %d\n",i);
		receivers[i]->stop();
	}


	deleteReceiver(receivers);



	return 0;
}

