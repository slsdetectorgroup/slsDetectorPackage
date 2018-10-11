#pragma once
/************************************************
 * @file sls_detector_funcs.h
 * @short functions indices to call on server (detector/receiver)
 ***********************************************/
/**
 *@short functions indices to call on server (detector/receiver)
 */


enum detFuncs{
	F_EXEC_COMMAND=0, /**< command is executed */
	F_GET_DETECTOR_TYPE, /**< return detector type */
	F_SET_EXTERNAL_SIGNAL_FLAG, /**< set/get flag for external signal */
	F_SET_EXTERNAL_COMMUNICATION_MODE, /**< set/get  external communication mode (obsolete) */
	F_GET_ID, /**< get detector id of version */
	F_DIGITAL_TEST, /**< digital test of the detector */
	F_SET_DAC, /**< set DAC value */
	F_GET_ADC, /**< get ADC value */
	F_WRITE_REGISTER, /**< write to register */
	F_READ_REGISTER, /**< read register */
	F_SET_MODULE, /**< initialize module */
	F_GET_MODULE, /**< get module status */
	F_SET_SETTINGS, /**< set detector settings */
	F_GET_THRESHOLD_ENERGY, /**< get detector threshold (in eV) */
	F_START_ACQUISITION, /**< start acquisition */
	F_STOP_ACQUISITION, /**< stop acquisition */
	F_START_READOUT, /**< start readout */
	F_GET_RUN_STATUS, /**< get acquisition status */
	F_START_AND_READ_ALL, /**< start acquisition and read all frames*/
	F_READ_ALL, /**< read alla frames */
	F_SET_TIMER, /**< set/get timer value */
	F_GET_TIME_LEFT, /**< get current value of the timer (time left) */
	F_SET_DYNAMIC_RANGE, /**< set/get detector dynamic range */
	F_SET_READOUT_FLAGS, /**< set/get readout flags */
	F_SET_ROI, /**< set/get region of interest */
	F_SET_SPEED, /**< set/get readout speed parameters */
	F_EXIT_SERVER, /**< turn off detector server */
	F_LOCK_SERVER, /**< Locks/Unlocks server communication to the given client */
	F_GET_LAST_CLIENT_IP, /**< returns the IP of the client last connected to the detector */
	F_SET_PORT, /**< Changes communication port of the server */
	F_UPDATE_CLIENT, /**< Returns all the important parameters to update the shared memory of the client */
	F_CONFIGURE_MAC, /**< Configures MAC for Gotthard readout */
	F_LOAD_IMAGE, /**< Loads Dark/Gain image to the Gotthard detector */
	F_READ_COUNTER_BLOCK, /**< reads the counter block memory for gotthard */
	F_RESET_COUNTER_BLOCK, /**< resets the counter block memory for gotthard */
	F_CALIBRATE_PEDESTAL, /**< starts acquistion, calibrates pedestal and write back to fpga */
	F_ENABLE_TEN_GIGA, /**< enable 10Gbe */
	F_SET_ALL_TRIMBITS, /** < set all trimbits to this value */
	F_SET_CTB_PATTERN, /** < loads a pattern in the CTB */
	F_WRITE_ADC_REG, /** < writes an ADC register */
	F_SET_COUNTER_BIT, /** < set/reset counter bit in detector for eiger */
	F_PULSE_PIXEL,/** < pulse pixel n number of times in eiger at (x,y)  */
	F_PULSE_PIXEL_AND_MOVE,/** < pulse pixel n number of times and move relatively by x and y */
	F_PULSE_CHIP, /** < pulse chip n number of times */
	F_SET_RATE_CORRECT,/** < set/reset rate correction tau */
	F_GET_RATE_CORRECT,/** < get rate correction tau */
	F_SET_NETWORK_PARAMETER,/**< set network parameters such as transmission delay, flow control */
	F_PROGRAM_FPGA,/**< program FPGA */
	F_RESET_FPGA, /**< reset FPGA */
	F_POWER_CHIP, /**< power chip */
	F_ACTIVATE,/** < activate */
	F_PREPARE_ACQUISITION,/** < prepare acquisition */
	F_THRESHOLD_TEMP, /** < set threshold temperature */
	F_TEMP_CONTROL, /** < set temperature control */
	F_TEMP_EVENT, /** < set temperature event */
	F_AUTO_COMP_DISABLE, /** < auto comp disable mode */
	F_STORAGE_CELL_START, /** < storage cell start */
	F_CHECK_VERSION,/** < check version compatibility */
	F_SOFTWARE_TRIGGER,/** < software trigger */
	NUM_DET_FUNCTIONS,

	RECEIVER_ENUM_START = 128, /**< detector function should not exceed this (detector server should not compile anyway) */
	F_EXEC_RECEIVER_COMMAND,/**< command is executed */
	F_EXIT_RECEIVER,/**< turn off receiver server */
	F_LOCK_RECEIVER,/**< Locks/Unlocks server communication to the given client */
	F_GET_LAST_RECEIVER_CLIENT_IP,/**< returns the IP of the client last connected to the receiver */
	F_SET_RECEIVER_PORT, /**< Changes communication port of the receiver */
	F_UPDATE_RECEIVER_CLIENT, /**< Returns all the important parameters to update the shared memory of the client */
	F_GET_RECEIVER_ID, /**< get receiver id of version */
	F_GET_RECEIVER_TYPE, /**< return receiver type */
	F_SEND_RECEIVER_DETHOSTNAME, /**< set detector hostname to receiver */
	F_RECEIVER_SET_ROI, /**< Sets receiver ROI */
	F_SETUP_RECEIVER_UDP, /**< sets the receiver udp connection and returns receiver mac address */
	F_SET_RECEIVER_TIMER, /**< set/get timer value */
	F_SET_RECEIVER_DYNAMIC_RANGE, /**< set/get detector dynamic range */
	F_RECEIVER_STREAMING_FREQUENCY, /**< sets the frequency of receiver sending frames to gui */
	F_GET_RECEIVER_STATUS, /**< gets the status of receiver listening mode */
	F_START_RECEIVER, /**< starts the receiver listening mode */
	F_STOP_RECEIVER, /**< stops the receiver listening mode */
	F_SET_RECEIVER_FILE_PATH, /**< sets receiver file directory */
	F_SET_RECEIVER_FILE_NAME, /**< sets receiver file name */
	F_SET_RECEIVER_FILE_INDEX, /**< sets receiver file index */
	F_GET_RECEIVER_FRAME_INDEX, /**< gets the receiver frame index */
	F_GET_RECEIVER_FRAMES_CAUGHT, /**< gets the number of frames caught by receiver */
	F_RESET_RECEIVER_FRAMES_CAUGHT, /**< resets the frames caught by receiver */
	F_ENABLE_RECEIVER_FILE_WRITE, /**< sets the receiver file write */
	F_ENABLE_RECEIVER_OVERWRITE, /**< set overwrite flag in receiver */
	F_ENABLE_RECEIVER_TEN_GIGA, /**< enable 10Gbe in receiver */
	F_SET_RECEIVER_FIFO_DEPTH, /**< set receiver fifo depth */
	F_RECEIVER_ACTIVATE, /** < activate/deactivate readout */
	F_STREAM_DATA_FROM_RECEIVER, /**< stream data from receiver to client */
	F_RECEIVER_STREAMING_TIMER, /** < sets the timer between each data stream in receiver */
	F_SET_FLIPPED_DATA_RECEIVER, /** < sets the enable to flip data across x/y axis (bottom/top) */
	F_SET_RECEIVER_FILE_FORMAT, /** < sets the receiver file format */
	F_SEND_RECEIVER_DETPOSID, /** < sets the detector position id in the reveiver */
	F_SEND_RECEIVER_MULTIDETSIZE, /** < sets the multi detector size to the receiver */
	F_SET_RECEIVER_STREAMING_PORT, /** < sets the receiver streaming port */
	F_RECEIVER_STREAMING_SRC_IP, /** < sets the receiver streaming source IP */
	F_SET_RECEIVER_SILENT_MODE, /** < sets the receiver silent mode */
	F_ENABLE_GAPPIXELS_IN_RECEIVER, /** < sets gap pixels in the receiver */
	F_RESTREAM_STOP_FROM_RECEIVER, /** < restream stop from receiver */
	F_ADDITIONAL_JSON_HEADER, /** < additional json header */
	F_RECEIVER_UDP_SOCK_BUF_SIZE, /** < UDP socket buffer size */
	F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE, /** < real UDP socket buffer size */
	F_SET_RECEIVER_FRAMES_PER_FILE, /** < receiver frames per file */
	F_RECEIVER_CHECK_VERSION, /** < check receiver version compatibility */
	F_RECEIVER_DISCARD_POLICY, /** < frames discard policy */
	F_RECEIVER_PADDING_ENABLE, /** < partial frames padding enable */
	F_RECEIVER_DEACTIVATED_PADDING_ENABLE, /** < deactivated receiver padding enable */
	NUM_REC_FUNCTIONS
};

