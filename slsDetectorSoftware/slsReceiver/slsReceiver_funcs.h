  /********************************************//**
 * @file slsReceiver_funcs.h
 * @short interface between receiver and client
 ***********************************************/
#ifndef RECEIVER_H
#define RECEIVER_H

#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "MySocketTCP.h"
#include "slsReceiverFunctionList.h"



/**
  *@short interface between receiver and client
  */

class slsReceiverFuncs : private virtual slsDetectorDefs {

public:
	/**
	 * Constructor
	 * reads config file, creates socket, assigns function table
	 * @param argc from command line
	 * @param argv from command line
	 * @param succecc socket creation was successfull
	 */
  slsReceiverFuncs(int argc, char *argv[], int &success);

  /** starts listening on the TCP port for client comminication */
  
	void start();

	/** Destructor */
	virtual ~slsReceiverFuncs();

	/** Close File */
	static void closeFile(int p);


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
	
	void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){slsReceiverList->registerCallBackStartAcquisition(func,arg);};;


	/**
	  callback argument is
	  toatal farmes caught

	*/
	
	
	void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){slsReceiverList->registerCallBackAcquisitionFinished(func,arg);};
	


	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	
	void registerCallBackRawDataReady(void (*func)(int, char*, FILE*, char*, void*),void *arg){slsReceiverList->registerCallBackRawDataReady(func,arg);};


 private:
	/** assigns functions to the fnum enum */
	int function_table();

	/** Decodes Function */
	int decode_function();

	/** Unrecognized Function */
	int M_nofunc();

	/** Set File name without frame index, file index and extension */
	int set_file_name();

	/** Set File path */
	int set_file_dir();

	/** Set up UDP Details */
	int setup_udp();

	/** Set File index */
	int set_file_index();

	/** Set Frame index */
	int set_frame_index();

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

	/** set short frame */
	int set_short_frame();

	/** Reads Frame/ buffer */
	int	read_frame();

	/** gotthard specific read frame */
	int gotthard_read_frame();

	/** moench specific read frame */
	int moench_read_frame();

	/** Sets the receiver to send every nth frame to gui, or only upon gui request */
	int set_read_frequency();

	/** Reads every nth frame, sends them to gui without closing socket */
	int read_all();

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
	/** detector type */
	detectorType myDetectorType;

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

	/** Short frame */
	int shortFrame;

	/** Packets per frame */
	int packetsPerFrame;

	/** temporary variable to debug moench receiver with gotthard module */
	int withGotthard;

	static int file_des;
	static int socketDescriptor;

//private:
 protected:
	/** Socket */
	MySocketTCP* socket;
};


#endif
