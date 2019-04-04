#pragma once

class MySocketTCP;
#include "sls_detector_defs.h"

#include <stdlib.h>
#include <string>


/**
 *@short Sets up the gui server
 */
class qClient: public virtual slsDetectorDefs{


public:
	/** 
	 * The constructor
	 */
	qClient(char* hostname);
	/** 
	 * Destructor	 
	 */
	virtual ~qClient();

	/**
	 * Execute command
	 */
	int executeLine(int narg, char *args[]);

private:
	/** 
	 * Print list of commands
	 */
	std::string printCommands();

	/** 
	 * Start Acquisition
	 * @param blocking true if its a blocking acquistion
	 */
	int startAcquisition(bool blocking = false);

	/** 
	 * Stops Acquisition 
	 */
	int stopAcquisition();

	/** 
	 * Gets run status 
	 */
	std::string getStatus();

	/** 
	 * Exits Server 
	 */
	int exitServer();

	/** client socket */
	MySocketTCP *mySocket;

	/** client socket */
	MySocketTCP *myStopSocket;

	/** error message */
	char mess[MAX_STR_LENGTH];

};
