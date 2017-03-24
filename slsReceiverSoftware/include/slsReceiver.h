#pragma once
/********************************************//**
 * @file slsReceiver.h
 * @short creates the UDP and TCP class objects
 ***********************************************/



#include "slsReceiverTCPIPInterface.h"
#include "UDPInterface.h"

#include "receiver_defs.h"
#include "MySocketTCP.h"
//#include "utilities.h"


/**
 *@short creates the UDP and TCP class objects
 */

class slsReceiver : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * creates the tcp interface and the udp class
	 * @param argc from command line
	 * @param argv from command line
	 * @param succecc socket creation was successfull
	 */
	slsReceiver(int argc, char *argv[], int &success);

	/**
	 * Destructor
	 */
	~slsReceiver();

	/**
	 * starts listening on the TCP port for client comminication
	 \return 0 for success or 1 for FAIL in creating TCP server
	 */
	int start();

	/**
	 * stops listening to the TCP & UDP port and exit receiver program
	 */
	void stop();

	/**
	 * Close File and exits receiver server
	 */
	void closeFile(int p);

	/**
	 * get get Receiver Version
	 \returns id
	 */
	int64_t getReceiverVersion();

	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is undefined at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	void registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg);

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg);

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
	void registerCallBackRawDataReady(void (*func)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
			char*, uint32_t, void*),void *arg);



 private:
	slsReceiverTCPIPInterface* tcpipInterface;
	UDPInterface* udp_interface;
};

