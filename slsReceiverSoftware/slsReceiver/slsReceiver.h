/********************************************//**
 * @file slsReceiver.h
 * @short creates the UDP and TCP class objects
 ***********************************************/
#ifndef SLS_RECEIVER_H
#define SLS_RECEIVER_H


#include "slsReceiverTCPIPInterface.h"
#include "UDPInterface.h"
//#include "UDPBaseImplementation.h"

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
	 @sort register calbback for starting the acquisition
	 @param func  callback to be called when starting the acquisition. Its arguments are filepath filename fileindex data size
	 \returns	   0 callback takes care of open,close,write file; 	   1 callback writes file, we have to open, close it; 2 we open, close, write file, callback does not do anything
	 */
	void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg);


	/**
	  callback argument is
	  toatal farmes caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);

	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	 */
	void registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg);


 private:
	slsReceiverTCPIPInterface* tcpipInterface;
	UDPInterface* udp_interface;
};


#endif
