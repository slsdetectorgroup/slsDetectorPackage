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

	/** Destructor */
	virtual ~slsReceiverTCPIPInterface();

	/** Close all threaded Files and exit */
	void closeFile(int p);

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

	/** assigns functions to the fnum enum */
	int function_table();

	/** Decodes Function */
	int decode_function();

	/** Unrecognized Function */
	int M_nofunc();

	/** Set detector type */
	int set_detector_type();

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

	/** propix specific read frame */
	int propix_read_frame();

	/** moench specific read frame */
	int moench_read_frame();

	/** eiger specific read frame */
	int eiger_read_frame();

	/** jungfrau specific read frame */
	int jungfrau_read_frame();

	/** Sets the receiver to send every nth frame to gui, or only upon gui request */
	int set_read_frequency();

	/** Sets the timer between frames streamed by receiver when frequency is set to 0 */
	int set_read_receiver_timer();

	/* Set the data stream enable */
	int set_data_stream_enable();

	/** Enable File Write*/
	int enable_file_write();

	/** get version, calls get_version */
	int get_id();

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished */
	int start_readout();

	/** set acquisition period, frame number etc */
	int set_timer();

	/** enable compression */
	int enable_compression();

	/** set detector hostname  */
	int set_detector_hostname();

	/** set dynamic range  */
	int set_dynamic_range();

	/** enable overwrite  */
	int enable_overwrite();

	/** enable 10Gbe */
	int enable_tengiga();

	/** set fifo depth */
	int set_fifo_depth();

	/** activate/ deactivate */
	int set_activate();

	/** enable flipped data */
	int set_flipped_data();

	/** set file format */
	int set_file_format();

	/** set position id */
	int set_detector_posid();

	/** set multi detector size */
	int set_multi_detector_size();

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

	/** slsReceiverBase object */
	UDPInterface *receiverBase;

	/** Number of functions */
	static const int numberOfFunctions = 256;

	/** Function List */
	int (slsReceiverTCPIPInterface::*flist[numberOfFunctions])();

	/** Message */
	char mess[MAX_STR_LENGTH];

	/** success/failure */
	int ret;

	/** Lock Status if server locked to a client */
	int lockStatus;

	/** kill tcp server thread */
	int killTCPServerThread;

	/** thread for TCP server */
	pthread_t   TCPServer_thread;

	/** size of one frame*/
	int tenGigaEnable;

	/** port number */
	int portNumber;

	/** Receiver not setup error message */
	char SET_RECEIVER_ERR_MESSAGE[MAX_STR_LENGTH];


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
