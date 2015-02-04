/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include <iostream>
#include <string.h>

#include "utilities.h"
#include "logger.h"

using namespace std;


int main(int argc, char *argv[]) {
	int ret = slsReceiverDefs::OK;

	slsReceiverUsers *user = new slsReceiverUsers(argc, argv, ret);

	if(ret==slsReceiverDefs::FAIL)
		return -1;


	//register callbacks


	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   datasize

	   return value is 
	   0 raw data ready callback takes care of open,close,write file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything


	   registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg);
	 */

	//receiver->registerCallBackStartAcquisition(func,arg);


	/**
	  callback argument is
	  total farmes caught
	  registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	 */


	//receiver->registerCallBackAcquisitionFinished(func,arg);



	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)

	  NEVER DELETE THE DATA POINTER
	  REMEMBER THAT THE CALLBACK IS BLOCKING

	  registerCallBackRawDataReady(void (*func)(int, char*, FILE*, char*, void*),void *arg);

	 */

	//receiver->registerCallBackRawDataReady(func,arg);



	//start tcp server thread
	if(user->start() == slsReceiverDefs::OK){
		cout << "DONE!" << endl;
		string str;
		cin>>str;
		//wait and look for an exit keyword
		while(str.find("exit") == string::npos)
			cin>>str;
		//stop tcp server thread, stop udp socket
		user->stop();
	}

	delete user;
	cout << "Goodbye!" << endl;
	return 0;
}

