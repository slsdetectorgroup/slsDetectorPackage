/********************************************//**
 * @file slsReceiver.h
 * @short creates the UDP and TCP class objects
 ***********************************************/
#ifndef SLS_RECEIVER_H
#define SLS_RECEIVER_H


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
	 * return value is
	 * 0 callback takes care of open,close,wrie file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything
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
	 * index
	 * frame number
	 * timestamp/ bunch id
	 * exposure length/ sub frame number
	 * datapointer
	 * datasize in bytes
	 * file descriptor
	 */
	void registerCallBackRawDataReady(void (*func)(int, uint64_t, uint64_t, uint64_t, char*, uint32_t, FILE*, void*),void *arg);


 private:
	slsReceiverTCPIPInterface* tcpipInterface;
	UDPInterface* udp_interface;
};


#endif
