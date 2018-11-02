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
	 * Starts up a Receiver server. Reads configuration file, options, and
	 * assembles a Receiver using TCP and UDP detector interfaces
	 * throws an exception in case of failure
	 * @param argc from command line
	 * @param argv from command line
	 */
	slsReceiver(int argc, char *argv[]);

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
	 * sls_receiver_header frame metadata
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes.
	 */
	void registerCallBackRawDataReady(void (*func)(char* ,
			char*, uint32_t, void*),void *arg);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes.
     * Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void registerCallBackRawDataModifyReady(void (*func)(char* ,
            char*, uint32_t &,void*),void *arg);



 private:
	slsReceiverTCPIPInterface* tcpipInterface;
};

