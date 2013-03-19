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

	/** Send to Gui Server */
	int sendToGuiServer(int fnum);

	/** Gets run status */
	string getStatus();

	/** client socket */
	MySocketTCP *mySocket;

};



#endif /* QCLIENT_H */
