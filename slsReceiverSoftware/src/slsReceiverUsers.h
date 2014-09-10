#ifndef SLS_RECEIVER_USERS_H
#define SLS_RECEIVER_USERS_H

#include <stdio.h>
#include <stdint.h>


class slsReceiver;

  /** 
@short Class for implementing the SLS data receiver in the users application. Callbacks can be defined for processing and/or saving data
  */
/** 


 @libdoc slsReceiverUsers is a class that can be instantiated in the users software to receive the data from the detectors. Callbacks can be defined for processing and/or saving data


 ***********************************************/

class slsReceiverUsers {

public:
	/**
	 * Constructor
	 * reads config file, creates socket, assigns function table
	 * @param argc from command line
	 * @param argv from command line
	 * @param succecc socket creation was successfull
	 */
	slsReceiverUsers(int argc, char *argv[], int &success);


	/** Destructor */
	~slsReceiverUsers();

	/** Close File and exits receiver server */
	void closeFile(int p);

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

	@sort register calbback for starting the acquisition 
	 \param func  callback to be called when starting the acquisition. Its arguments are filepath filename fileindex data size
	   
	  \returns	   0 callback takes care of open,close,write file; 	   1 callback writes file, we have to open, close it; 2 we open, close, write file, callback does not do anything

	*/
	
	void registerCallBackStartAcquisition(int (*func)(char* filepath, char* filename,int fileindex, int datasize, void*),void *arg);


	/**	
	   @sort register callback for end of acquisition 
	  \param func end of acquisition callback. Argument nf is total frames caught
	  \returns nothing
	*/
	
	
	void registerCallBackAcquisitionFinished(void (*func)(int nf, void*),void *arg);
	


	/**
	   @sort register callback to be called when data are available (to process and/or save the data).
	   \param func raw data ready callback. arguments are framenum datapointer  datasize file descriptor guidatapointer (NULL, no data required)
	  \returns nothing
	*/
	
	void registerCallBackRawDataReady(void (*func)(int framenumber, char* datapointer, int datasize, FILE* filedescriptor, char* guidatapointer, void*),void *arg);

	// made static to close thread files with ctrl+c
	static slsReceiver* receiver;
};


#endif
