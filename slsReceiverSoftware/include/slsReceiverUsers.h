#pragma once

#include <stdio.h>
#include <stdint.h>


class slsReceiver;

  /** 
@short Class for implementing the SLS data receiver in the users application. Callbacks can be defined for processing and/or saving data

 slsReceiverUsers is a class that can be instantiated in the users software to receive the data from the detectors. Callbacks can be defined for processing and/or saving data
 ***********************************************/

class slsReceiverUsers {

public:
	/**
	 * Constructor
	 * reads config file, creates socket, assigns function table
	 * @param argc from command line
	 * @param argv from command line
	 * @param success socket creation was successfull
	 */
	slsReceiverUsers(int argc, char *argv[], int &success);


	/** Destructor */
	~slsReceiverUsers();

	/**
	 * starts listening on the TCP port for client comminication
	 \return 0 for success or 1 for FAIL in creating TCP server
	 */
	int start();

	/** stops listening to the TCP & UDP port and exit receiver program*/
	void stop();

	/**
	 get get Receiver Version
	 \returns id
	 */
	int64_t getReceiverVersion();

	/**

	@short register calbback for starting the acquisition
	 \param func  callback to be called when starting the acquisition. Its arguments are  filepath, filename, fileindex, datasize
	 \param arg argument
	 \return value is insignificant at the moment, we write depending on file write enable, users get data to write depending on call backs registered
	*/
	void registerCallBackStartAcquisition(int (*func)(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*),void *arg);


	/**	
	   @short register callback for end of acquisition
	  \param func end of acquisition callback. Argument nf is total frames caught
	  \param arg argument
	  \returns nothing
	*/
	void registerCallBackAcquisitionFinished(void (*func)(uint64_t nf, void*),void *arg);
	


	/**
	   @short register callback to be called when data are available (to process and/or save the data).
	   \param func raw data ready callback. arguments are sls_receiver_header, dataPointer, dataSize
	   \param arg argument
	   \returns nothing
	 */
	void registerCallBackRawDataReady(void (*func)(char* header,
			char* datapointer, uint32_t datasize, void*),void *arg);

	
    /**
       @short register callback to be called when data are available (to process and/or save the data).
       \param func raw data ready callback. arguments are  sls_receiver_header, dataPointer, revDatasize is the reference of data size in bytes. Can be modified to the new size to be written/streamed. (only smaller value).
       \param arg argument
       \returns nothing
     */
    void registerCallBackRawDataModifyReady(void (*func)(char* header,
            char* datapointer, uint32_t &revDatasize, void*),void *arg);

	//receiver object
	slsReceiver* receiver;
};

