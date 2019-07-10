#pragma once
/** 
   @internal
   function indexes to call on the server
   All set functions with argument -1 work as get, when possible 
 */

#define REC_FUNC_START_INDEX 128

enum recFuncs{
	//General functions
	F_EXEC_RECEIVER_COMMAND=REC_FUNC_START_INDEX, 		/**< command is executed */
	F_EXIT_RECEIVER, 				/**< turn off receiver server */
	F_LOCK_RECEIVER, 				/**< Locks/Unlocks server communication to the given client */
	F_GET_LAST_RECEIVER_CLIENT_IP,  /**< returns the IP of the client last connected to the receiver */
	F_SET_RECEIVER_PORT, 			/**< Changes communication port of the receiver */
	F_UPDATE_RECEIVER_CLIENT, 		/**< Returns all the important parameters to update the shared memory of the client */

	// Identification
	F_GET_RECEIVER_ID, 				/**< get receiver id of version */
	F_GET_RECEIVER_TYPE, 			/**< return receiver type */
	F_SEND_RECEIVER_DETHOSTNAME,	/**< set detector hostname to receiver */

	//network functions
	F_RECEIVER_SET_ROI, 			/**< Sets receiver ROI */
	F_SETUP_RECEIVER_UDP,			/**< sets the receiver udp connection and returns receiver mac address */

	//Acquisition setup functions
	F_SET_RECEIVER_TIMER,  			/**< set/get timer value */
	F_SET_RECEIVER_DYNAMIC_RANGE,  	/**< set/get detector dynamic range */
	F_READ_RECEIVER_FREQUENCY, 		/**< sets the frequency of receiver sending frames to gui */

	// Acquisition functions
	F_GET_RECEIVER_STATUS,			/**< gets the status of receiver listening mode */
	F_START_RECEIVER,				/**< starts the receiver listening mode */
	F_STOP_RECEIVER,				/**< stops the receiver listening mode */
	F_START_RECEIVER_READOUT, 		/**< acquisition has stopped. start remaining readout in receiver */

	//file functions
	F_SET_RECEIVER_FILE_PATH, 		/**< sets receiver file directory */
	F_SET_RECEIVER_FILE_NAME, 		/**< sets receiver file name */
	F_SET_RECEIVER_FILE_INDEX, 		/**< sets receiver file index */
	F_GET_RECEIVER_FRAME_INDEX,		/**< gets the receiver frame index */
	F_GET_RECEIVER_FRAMES_CAUGHT,	/**< gets the number of frames caught by receiver */
	F_RESET_RECEIVER_FRAMES_CAUGHT, /**< resets the frames caught by receiver */
	F_ENABLE_RECEIVER_FILE_WRITE,	/**< sets the receiver file write */
	F_ENABLE_RECEIVER_COMPRESSION,	/**< enable compression in receiver */
	F_ENABLE_RECEIVER_OVERWRITE,		/**< set overwrite flag in receiver */

	F_ENABLE_RECEIVER_TEN_GIGA,		/**< enable 10Gbe in receiver */
	F_SET_RECEIVER_FIFO_DEPTH,		/**< set receiver fifo depth */

	F_RECEIVER_ACTIVATE,			/** < activate/deactivate readout */
	F_STREAM_DATA_FROM_RECEIVER,		/**< stream data from receiver to client */
	F_READ_RECEIVER_TIMER,			/** < sets the timer between each data stream in receiver */
	F_SET_FLIPPED_DATA_RECEIVER,		/** < sets the enable to flip data across x/y axis (bottom/top) */
	F_SET_RECEIVER_FILE_FORMAT,		/** < sets the receiver file format */

	F_SEND_RECEIVER_DETPOSID,		/** < sets the detector position id in the reveiver */
	F_SEND_RECEIVER_MULTIDETSIZE,    /** < sets the multi detector size to the receiver */
	F_SET_RECEIVER_STREAMING_PORT, 	/** < sets the receiver streaming port */
	F_RECEIVER_STREAMING_SRC_IP,	/** < sets the receiver streaming source IP */
	F_SET_RECEIVER_SILENT_MODE,		/** < sets the receiver silent mode */
	F_ENABLE_GAPPIXELS_IN_RECEIVER,	/** < sets gap pixels in the receiver */
	F_RESTREAM_STOP_FROM_RECEIVER,	/** < restream stop from receiver */
	F_ADDITIONAL_JSON_HEADER,       /** < additional json header */
	F_RECEIVER_UDP_SOCK_BUF_SIZE,   /** < UDP socket buffer size */
    F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE,   /** < real UDP socket buffer size */
	F_SET_RECEIVER_FRAMES_PER_FILE, /** < receiver frames per file */
	F_RECEIVER_CHECK_VERSION,		/** < check receiver version compatibility */
	F_RECEIVER_DISCARD_POLICY,		/** < frames discard policy */
	F_RECEIVER_PADDING_ENABLE,		/** < partial frames padding enable */
	F_RECEIVER_DEACTIVATED_PADDING_ENABLE, /** < deactivated receiver padding enable */
	F_RECEIVER_QUAD,
	/* Always append functions hereafter!!! */


	/* Always append functions before!!! */
	NUM_REC_FUNCTIONS
};

