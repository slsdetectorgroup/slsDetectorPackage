/** 
   @internal
   function indexes to call on the server
   All set functions with argument -1 work as get, when possible 
 */
#ifndef SLS_RECEIVER_FUNCS_H
#define SLS_RECEIVER_FUNCS_H

enum {
	//General functions
	F_EXEC_RECEIVER_COMMAND=128, 		/**< command is executed */
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
	F_RECEIVER_SHORT_FRAME, 		/**< Sets receiver to receive short frames */
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
	F_READ_RECEIVER_FRAME,  		/**< read one frame to gui*/

	//file functions
	F_SET_RECEIVER_FILE_PATH, 		/**< sets receiver file directory */
	F_SET_RECEIVER_FILE_NAME, 		/**< sets receiver file name */
	F_SET_RECEIVER_FILE_INDEX, 		/**< sets receiver file index */
	F_SET_RECEIVER_FRAME_INDEX,		/**< sets the receiver frame index */
	F_GET_RECEIVER_FRAME_INDEX,		/**< gets the receiver frame index */
	F_GET_RECEIVER_FRAMES_CAUGHT,	/**< gets the number of frames caught by receiver */
	F_RESET_RECEIVER_FRAMES_CAUGHT, /**< resets the frames caught by receiver */
	F_ENABLE_RECEIVER_FILE_WRITE,	/**< sets the receiver file write */
	F_ENABLE_RECEIVER_COMPRESSION,	/**< enable compression in receiver */
	F_ENABLE_RECEIVER_OVERWRITE,		/**< set overwrite flag in receiver */

	F_ENABLE_RECEIVER_TEN_GIGA,		/**< enable 10Gbe in receiver */
	F_SET_RECEIVER_FIFO_DEPTH		/**< set receiver fifo depth */

	/* Always append functions hereafter!!! */
};

#endif
/** @endinternal */
