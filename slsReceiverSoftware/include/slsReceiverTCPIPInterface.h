#pragma once
/********************************************//**
 * @file slsReceiverTCPIPInterface.h
 * @short interface between receiver and client
 ***********************************************/


#include "sls_receiver_defs.h"
#include "receiver_defs.h"
#include "MySocketTCP.h"
#include "UDPInterface.h"



/**
 *@short interface between receiver and client
 */

class slsReceiverTCPIPInterface : private virtual slsReceiverDefs {
	
 public:

	/** Destructor */
	virtual ~slsReceiverTCPIPInterface();

	/**
	 * Constructor
	 * reads config file, creates socket, assigns function table
	 * @param succecc socket creation was successfull
	 * @param rbase pointer to the receiver base
	 * @param pn port number (defaults to default port number)
	 */

  slsReceiverTCPIPInterface(int &success, UDPInterface* rbase, int pn=-1);

	/**
	 * Sets the port number to listen to. 
	 Take care that the client must know to whcih port it has to listen to, so normally it is better to use a fixes port from the instatiation or change it from the client.
	 @param pn port number (-1 only get)
	 \returns actual port number
	*/
	int setPortNumber(int pn=-1);
	
	/**
	 * Starts listening on the TCP port for client comminication
	 \returns OK or FAIL
	 */
	int start();

	/** stop listening on the TCP & UDP port for client comminication */
	void stop();


	/** gets version */
	int64_t getReceiverVersion();

	//***callback functions***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is insignificant at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	void registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg);

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg);

	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * frameNumber is the frame number
	 * expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
	 * packetNumber is the packet number
	 * bunchId is the bunch id from beamline
	 * timestamp is the time stamp with 10 MHz clock
	 * modId is the unique module id (unique even for left, right, top, bottom)
	 * xCoord is the x coordinate in the complete detector system
	 * yCoord is the y coordinate in the complete detector system
	 * zCoord is the z coordinate in the complete detector system
	 * debug is for debugging purposes
	 * roundRNumber is the round robin set number
	 * detType is the detector type see :: detectorType
	 * version is the version number of this structure format
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes
	 */
	void registerCallBackRawDataReady(void (*func)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
			char*, uint32_t, void*),void *arg);

 private:

	/**
	 * Static function - Thread started which is a TCP server
	 * Called by start()
	 * @param this_pointer pointer to this object
	 */
	static void* startTCPServerThread(void *this_pointer);


	/**
	 * Thread started which is a TCP server
	 * Called by start()
	 */
	void startTCPServer();

	/** retuns function name with function index */
	const char* getFunctionName(enum recFuncs func);

	/** assigns functions to the fnum enum */
	int function_table();

	/** Decodes Function */
	int decode_function();

	/** print socket read error */
	int printSocketReadError();

	/** receiver object is null */
	void invalidReceiverObject();

	/** receiver already locked */
	void receiverlocked();

	/** receiver not idle */
	void receiverNotIdle();

	/** function not implemented for specific detector */
	void functionNotImplemented();

	/** Unrecognized Function */
	int M_nofunc();



	/** Execute command */
	int	exec_command();

	/** Exit Receiver Server */
	int	exit_server();

	/** Locks Receiver */
	int	lock_receiver();

	/** Get Last Client IP*/
	int	get_last_client_ip();

	/** Set port */
	int set_port();

	/** Updates Client if different clients connect */
	int	update_client();

	/** Sends the updated parameters to client */
	int send_update();

	/** get version, calls get_version */
	int get_id();

	/** Set detector type */
	int set_detector_type();

	/** set detector hostname  */
	int set_detector_hostname();

	/** set short frame */
	int set_short_frame();

	/** Set up UDP Details */
	int setup_udp();

	/** set acquisition period, frame number etc */
	int set_timer();

	/** set dynamic range  */
	int set_dynamic_range();

	/** Sets the receiver to send every nth frame to gui, or only upon gui request */
	int set_read_frequency();

	/** Gets receiver status */
	int	get_status();

	/** Start Receiver - starts listening to udp packets from detector */
	int start_receiver();

	/** Stop Receiver - stops listening to udp packets from detector*/
	int stop_receiver();

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished */
	int start_readout();

	/** Reads Frame/ buffer */
	int	read_frame();

	/** Set File path */
	int set_file_dir();

	/** Set File name without frame index, file index and extension */
	int set_file_name();

	/** Set File index */
	int set_file_index();

	/** Set Frame index */
	int set_frame_index();

	/** Gets frame index for each acquisition */
	int	get_frame_index();

	/** Gets Total Frames Caught */
	int	get_frames_caught();

	/** Resets Total Frames Caught */
	int	reset_frames_caught();

	/** Enable File Write*/
	int enable_file_write();

	/** enable compression */
	int enable_compression();

	/** enable overwrite  */
	int enable_overwrite();

	/** enable 10Gbe */
	int enable_tengiga();

	/** set fifo depth */
	int set_fifo_depth();

	/** activate/ deactivate */
	int set_activate();

	/* Set the data stream enable */
	int set_data_stream_enable();

	/** Sets the timer between frames streamed by receiver when frequency is set to 0 */
	int set_read_receiver_timer();

	/** enable flipped data */
	int set_flipped_data();

	/** set file format */
	int set_file_format();

	/** set position id */
	int set_detector_posid();

	/** set multi detector size */
	int set_multi_detector_size();

	/** set streaming port */
	int set_streaming_port();



	/** detector type */
	detectorType myDetectorType;

	/** slsReceiverBase object */
	UDPInterface *receiverBase;

	/** Function List */
	int (slsReceiverTCPIPInterface::*flist[NUM_REC_FUNCTIONS])();

	/** Message */
	char mess[MAX_STR_LENGTH];

	/** success/failure */
	int ret;

	/** function index */
	int fnum;

	/** Lock Status if server locked to a client */
	int lockStatus;

	/** kill tcp server thread */
	int killTCPServerThread;

	/** thread for TCP server */
	pthread_t   TCPServer_thread;

	/** port number */
	int portNumber;

	//***callback parameters***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is insignificant at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	int (*startAcquisitionCallBack)(char*, char*, uint64_t, uint32_t, void*);
	void *pStartAcquisition;

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void (*acquisitionFinishedCallBack)(uint64_t, void*);
	void *pAcquisitionFinished;


	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * frameNumber is the frame number
	 * expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
	 * packetNumber is the packet number
	 * bunchId is the bunch id from beamline
	 * timestamp is the time stamp with 10 MHz clock
	 * modId is the unique module id (unique even for left, right, top, bottom)
	 * xCoord is the x coordinate in the complete detector system
	 * yCoord is the y coordinate in the complete detector system
	 * zCoord is the z coordinate in the complete detector system
	 * debug is for debugging purposes
	 * roundRNumber is the round robin set number
	 * detType is the detector type see :: detectorType
	 * version is the version number of this structure format
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes
	 */
	void (*rawDataReadyCallBack)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
			char*, uint32_t, void*);
	void *pRawDataReady;



protected:
	/** Socket */
	MySocketTCP* mySock;
};
