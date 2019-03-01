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
	F_LED,	/** < switch on/off led */
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
	F_GET_ADDITIONAL_JSON_HEADER,/** < get additional json header */
	F_RECEIVER_UDP_SOCK_BUF_SIZE, /** < UDP socket buffer size */
	F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE, /** < real UDP socket buffer size */
	F_SET_RECEIVER_FRAMES_PER_FILE, /** < receiver frames per file */
	F_RECEIVER_CHECK_VERSION, /** < check receiver version compatibility */
	F_RECEIVER_DISCARD_POLICY, /** < frames discard policy */
	F_RECEIVER_PADDING_ENABLE, /** < partial frames padding enable */
	F_RECEIVER_DEACTIVATED_PADDING_ENABLE, /** < deactivated receiver padding enable */
	F_RECEIVER_SET_READOUT_FLAGS, /**< set/get receiver readout flags */
	NUM_REC_FUNCTIONS
};

#ifdef __cplusplus
static const char* getFunctionNameFromEnum(enum detFuncs func) {
	switch (func) {
	case F_EXEC_COMMAND:					return "F_EXEC_COMMAND";
	case F_GET_DETECTOR_TYPE:				return "F_GET_DETECTOR_TYPE";
	case F_SET_EXTERNAL_SIGNAL_FLAG:		return "F_SET_EXTERNAL_SIGNAL_FLAG";
	case F_SET_EXTERNAL_COMMUNICATION_MODE:	return "F_SET_EXTERNAL_COMMUNICATION_MODE";
	case F_GET_ID:							return "F_GET_ID";
	case F_DIGITAL_TEST:					return "F_DIGITAL_TEST";
	case F_SET_DAC:							return "F_SET_DAC";
	case F_GET_ADC:							return "F_GET_ADC";
	case F_WRITE_REGISTER:					return "F_WRITE_REGISTER";
	case F_READ_REGISTER:					return "F_READ_REGISTER";
	case F_SET_MODULE:						return "F_SET_MODULE";
	case F_GET_MODULE:						return "F_GET_MODULE";
	case F_SET_SETTINGS:					return "F_SET_SETTINGS";
	case F_GET_THRESHOLD_ENERGY:			return "F_GET_THRESHOLD_ENERGY";
	case F_START_ACQUISITION:				return "F_START_ACQUISITION";
	case F_STOP_ACQUISITION:				return "F_STOP_ACQUISITION";
	case F_START_READOUT:					return "F_START_READOUT";
	case F_GET_RUN_STATUS:					return "F_GET_RUN_STATUS";
	case F_START_AND_READ_ALL:				return "F_START_AND_READ_ALL";
	case F_READ_ALL:						return "F_READ_ALL";
	case F_SET_TIMER:						return "F_SET_TIMER";
	case F_GET_TIME_LEFT:					return "F_GET_TIME_LEFT";
	case F_SET_DYNAMIC_RANGE:				return "F_SET_DYNAMIC_RANGE";
	case F_SET_READOUT_FLAGS:				return "F_SET_READOUT_FLAGS";
	case F_SET_ROI:							return "F_SET_ROI";
	case F_SET_SPEED:						return "F_SET_SPEED";
	case F_EXIT_SERVER:						return "F_EXIT_SERVER";
	case F_LOCK_SERVER:						return "F_LOCK_SERVER";
	case F_GET_LAST_CLIENT_IP:				return "F_GET_LAST_CLIENT_IP";
	case F_SET_PORT:						return "F_SET_PORT";
	case F_UPDATE_CLIENT:					return "F_UPDATE_CLIENT";
	case F_CONFIGURE_MAC:					return "F_CONFIGURE_MAC";
	case F_LOAD_IMAGE:						return "F_LOAD_IMAGE";
	case F_READ_COUNTER_BLOCK:				return "F_READ_COUNTER_BLOCK";
	case F_RESET_COUNTER_BLOCK:				return "F_RESET_COUNTER_BLOCK";
	case F_ENABLE_TEN_GIGA:					return "F_ENABLE_TEN_GIGA";
	case F_SET_ALL_TRIMBITS:				return "F_SET_ALL_TRIMBITS";
	case F_SET_CTB_PATTERN:					return "F_SET_CTB_PATTERN";
	case F_WRITE_ADC_REG:					return "F_WRITE_ADC_REG";
	case F_SET_COUNTER_BIT:					return "F_SET_COUNTER_BIT";
	case F_PULSE_PIXEL:						return "F_PULSE_PIXEL";
	case F_PULSE_PIXEL_AND_MOVE:			return "F_PULSE_PIXEL_AND_MOVE";
	case F_PULSE_CHIP:						return "F_PULSE_CHIP";
	case F_SET_RATE_CORRECT:				return "F_SET_RATE_CORRECT";
	case F_GET_RATE_CORRECT:				return "F_GET_RATE_CORRECT";
	case F_SET_NETWORK_PARAMETER:			return "F_SET_NETWORK_PARAMETER";
	case F_PROGRAM_FPGA:					return "F_PROGRAM_FPGA";
	case F_RESET_FPGA:						return "F_RESET_FPGA";
	case F_POWER_CHIP:						return "F_POWER_CHIP";
	case F_ACTIVATE:						return "F_ACTIVATE";
	case F_PREPARE_ACQUISITION:				return "F_PREPARE_ACQUISITION";
	case F_THRESHOLD_TEMP:                  return "F_THRESHOLD_TEMP";
	case F_TEMP_CONTROL:                    return "F_TEMP_CONTROL";
	case F_TEMP_EVENT:                      return "F_TEMP_EVENT";
    case F_AUTO_COMP_DISABLE:               return "F_AUTO_COMP_DISABLE";
    case F_STORAGE_CELL_START:              return "F_STORAGE_CELL_START";
    case F_CHECK_VERSION:              		return "F_CHECK_VERSION";
    case F_SOFTWARE_TRIGGER:              	return "F_SOFTWARE_TRIGGER";
    case F_LED:              				return "F_LED";

    case NUM_DET_FUNCTIONS:              	return "NUM_DET_FUNCTIONS";
    case RECEIVER_ENUM_START:				return "RECEIVER_ENUM_START";

	case F_EXEC_RECEIVER_COMMAND:			return "F_EXEC_RECEIVER_COMMAND";
	case F_EXIT_RECEIVER: 					return "F_EXIT_RECEIVER";
	case F_LOCK_RECEIVER: 					return "F_LOCK_RECEIVER";
	case F_GET_LAST_RECEIVER_CLIENT_IP: 	return "F_GET_LAST_RECEIVER_CLIENT_IP";
	case F_SET_RECEIVER_PORT: 				return "F_SET_RECEIVER_PORT";
	case F_UPDATE_RECEIVER_CLIENT: 			return "F_UPDATE_RECEIVER_CLIENT";
	case F_GET_RECEIVER_ID: 				return "F_GET_RECEIVER_ID";
	case F_GET_RECEIVER_TYPE: 				return "F_GET_RECEIVER_TYPE";
	case F_SEND_RECEIVER_DETHOSTNAME:		return "F_SEND_RECEIVER_DETHOSTNAME";
	case F_RECEIVER_SET_ROI: 				return "F_RECEIVER_SET_ROI";
	case F_SETUP_RECEIVER_UDP:				return "F_SETUP_RECEIVER_UDP";
	case F_SET_RECEIVER_TIMER:  			return "F_SET_RECEIVER_TIMER";
	case F_SET_RECEIVER_DYNAMIC_RANGE:  	return "F_SET_RECEIVER_DYNAMIC_RANGE";
	case F_RECEIVER_STREAMING_FREQUENCY:	return "F_RECEIVER_STREAMING_FREQUENCY";
	case F_GET_RECEIVER_STATUS:				return "F_GET_RECEIVER_STATUS";
	case F_START_RECEIVER:					return "F_START_RECEIVER";
	case F_STOP_RECEIVER:					return "F_STOP_RECEIVER";
	case F_SET_RECEIVER_FILE_PATH: 			return "F_SET_RECEIVER_FILE_PATH";
	case F_SET_RECEIVER_FILE_NAME: 			return "F_SET_RECEIVER_FILE_NAME";
	case F_SET_RECEIVER_FILE_INDEX: 		return "F_SET_RECEIVER_FILE_INDEX";
	case F_GET_RECEIVER_FRAME_INDEX:		return "F_GET_RECEIVER_FRAME_INDEX";
	case F_GET_RECEIVER_FRAMES_CAUGHT:		return "F_GET_RECEIVER_FRAMES_CAUGHT";
	case F_RESET_RECEIVER_FRAMES_CAUGHT:	return "F_RESET_RECEIVER_FRAMES_CAUGHT";
	case F_ENABLE_RECEIVER_FILE_WRITE:		return "F_ENABLE_RECEIVER_FILE_WRITE";
	case F_ENABLE_RECEIVER_OVERWRITE:		return "F_ENABLE_RECEIVER_OVERWRITE";
	case F_ENABLE_RECEIVER_TEN_GIGA:		return "F_ENABLE_RECEIVER_TEN_GIGA";
	case F_SET_RECEIVER_FIFO_DEPTH:			return "F_SET_RECEIVER_FIFO_DEPTH";
	case F_RECEIVER_ACTIVATE:				return "F_RECEIVER_ACTIVATE";
	case F_STREAM_DATA_FROM_RECEIVER:		return "F_STREAM_DATA_FROM_RECEIVER";
	case F_RECEIVER_STREAMING_TIMER:		return "F_RECEIVER_STREAMING_TIMER";
	case F_SET_FLIPPED_DATA_RECEIVER:		return "F_SET_FLIPPED_DATA_RECEIVER";
	case F_SET_RECEIVER_FILE_FORMAT:		return "F_SET_RECEIVER_FILE_FORMAT";
	case F_SEND_RECEIVER_DETPOSID:			return "F_SEND_RECEIVER_DETPOSID";
	case F_SEND_RECEIVER_MULTIDETSIZE:  	return "F_SEND_RECEIVER_MULTIDETSIZE";
	case F_SET_RECEIVER_STREAMING_PORT: 	return "F_SET_RECEIVER_STREAMING_PORT";
	case F_RECEIVER_STREAMING_SRC_IP: 		return "F_RECEIVER_STREAMING_SRC_IP";
	case F_SET_RECEIVER_SILENT_MODE:		return "F_SET_RECEIVER_SILENT_MODE";
	case F_ENABLE_GAPPIXELS_IN_RECEIVER:	return "F_ENABLE_GAPPIXELS_IN_RECEIVER";
	case F_RESTREAM_STOP_FROM_RECEIVER:		return "F_RESTREAM_STOP_FROM_RECEIVER";
    case F_ADDITIONAL_JSON_HEADER:      	return "F_ADDITIONAL_JSON_HEADER";
    case F_GET_ADDITIONAL_JSON_HEADER:      return "F_GET_ADDITIONAL_JSON_HEADER";
    case F_RECEIVER_UDP_SOCK_BUF_SIZE:  	return "F_RECEIVER_UDP_SOCK_BUF_SIZE";
    case F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE: return "F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE";
    case F_SET_RECEIVER_FRAMES_PER_FILE:	return "F_SET_RECEIVER_FRAMES_PER_FILE";
    case F_RECEIVER_CHECK_VERSION:			return "F_RECEIVER_CHECK_VERSION";
    case F_RECEIVER_DISCARD_POLICY:			return "F_RECEIVER_DISCARD_POLICY";
    case F_RECEIVER_PADDING_ENABLE:			return "F_RECEIVER_PADDING_ENABLE";
    case F_RECEIVER_DEACTIVATED_PADDING_ENABLE: return "F_RECEIVER_DEACTIVATED_PADDING_ENABLE";
    case F_RECEIVER_SET_READOUT_FLAGS: 		return "F_RECEIVER_SET_READOUT_FLAGS";
    case NUM_REC_FUNCTIONS: 				return "NUM_REC_FUNCTIONS";
	default:								return "Unknown Function";
	}
};
#endif

