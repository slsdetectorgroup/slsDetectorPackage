  /********************************************//**
 * @file slsReceiver_funcs.h
 * @short interface between receiver and client
 ***********************************************/
#ifndef RECEIVER_H
#define RECEIVER_H

#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "MySocketTCP.h"

class slsReceiverFunctionList;


/**
  *@short interface between receiver and client
  */

class slsReceiverFuncs : private virtual slsDetectorDefs {

public:
	/**
	 * Constructor
	 * reads config file, creates socket, assigns function table
	 * @param mySocket tcp socket connecting receiver and client
	 * @param fname name of config file
	 * @param success if socket creation was successfull
	 * @param shortfname true if short file name required
	 */
	slsReceiverFuncs(MySocketTCP *&mySocket,string const fname,int &success, bool shortfname);

	/** Destructor */
	virtual ~slsReceiverFuncs(){};

	/** assigns functions to the fnum enum */
	int function_table();

	/** Decodes Function */
	int decode_function();

	/** Unrecognized Function */
	int M_nofunc();

	/** Close File */
	static void closeFile(int p);

	/** Set File name without frame index, file index and extension */
	int set_file_name();

	/** Set File path */
	int set_file_dir();

	/** Set up UDP Details */
	int setup_udp();

	/** Set File index */
	int set_file_index();

	/** Start Receiver - starts listening to udp packets from detector */
	int start_receiver();

	/** Stop Receiver - stops listening to udp packets from detector*/
	int stop_receiver();

	/** Gets receiver status */
	int	get_status();

	/** Gets Total Frames Caught */
	int	get_frames_caught();

	/** Gets frame index for each acquisition */
	int	get_frame_index();

	/** Resets Total Frames Caught */
	int	reset_frames_caught();

	/** Reads Frame/ buffer */
	int	read_frame();

	/** Enable File Write*/
	int enable_file_write();

	/** Get Version */
	int get_version();

	//General Functions
	/** Locks Receiver */
	int	lock_receiver();

	/** Set port */
	int set_port();

	/** Get Last Client IP*/
	int	get_last_client_ip();

	/** Updates Client if different clients connect */
	int	update_client();

	/** Sends the updated parameters to client */
	int send_update();

	/** Exit Receiver Server */
	int	exit_server();

	/** Execute command */
	int	exec_command();



//private:

	/** Socket */
	MySocketTCP*& socket;
private:
	/** slsReceiverFunctionList object */
	slsReceiverFunctionList *slsReceiverList;

	/** Number of functions */
	static const int numberOfFunctions = 256;

	/** Function List */
	int (slsReceiverFuncs::*flist[numberOfFunctions])();

	/** Message */
	char mess[MAX_STR_LENGTH];

	/** success/failure */
	int ret;

	/** Lock Status if server locked to a client */
	int lockStatus;

	static int file_des;
	static int socketDescriptor;

};


#endif
