  /********************************************//**
 * @file slsReceiverUsers.h
 * @short API for implementing the SLS data receiver in the users application. Callbacks can be defined for processing and/or saving data
 * 
 ***********************************************/
#ifndef SLS_RECEIVER_USERS_H
#define SLS_RECEIVER_USERS_H

#include "slsReceiver_funcs.h"

class slsReceiverFuncs;



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

	/** Close File */
	static void closeFile(int p);

	/** starts listening on the TCP port for client comminication */
	void start();

	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   data size
	   
	   return value is 
	   0 callback takes care of open,close,wrie file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything

	*/
	
	void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg);


	/**
	  callback argument is
	  total frames caught

	*/
	
	
	int registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	


	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	
	int registerCallBackRawDataReady(void (*func)(int, char*, FILE*, char*, void*),void *arg);


 private:

	slsReceiverFuncs *receiver;

};


#endif
