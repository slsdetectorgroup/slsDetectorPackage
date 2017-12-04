/*
 * qClient.h
 *
 *  Created on: Feb 27, 2013
 *      Author: Dhanya Maliakal
 */
#ifndef QCLIENT_H
#define QCLIENT_H


/** Qt Project Class Headers */
/** Project Class Headers */
class MySocketTCP;
#include "sls_detector_defs.h"
/** C++ Include Headers */
#include <stdlib.h>
#include <string>
using namespace std;

/**
 *@short Sets up the gui server
 */
class qClient: public virtual slsDetectorDefs{


public:
	/** \short The constructor*/
	qClient(char* hostname);
	/** Destructor	 */
	virtual ~qClient();

	/**Execute command*/
	int executeLine(int narg, char *args[]);

private:
	/** Print list of commands */
	string printCommands();

	/** Start Acquisition
	 * @param blocking true if its a blocking acquistion
	 */
	int startAcquisition(bool blocking = false);

	/** Stops Acquisition */
	int stopAcquisition();

	/** Gets run status */
	string getStatus();

	/** Exits Server */
	int exitServer();

	/** client socket */
	MySocketTCP *mySocket;

	/** client socket */
	MySocketTCP *myStopSocket;

	char mess[MAX_STR_LENGTH];

};



#endif /* QCLIENT_H */
