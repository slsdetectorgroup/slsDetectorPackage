#include "slsDetectorServer_funcs.h"
#include "slsDetectorFunctionList.h"
#include "communication_funcs.h"
#include "logger.h"

#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

//defined in the detector specific Makefile
#ifdef GOTTHARDD
const enum detectorType myDetectorType = GOTTHARD;
#elif EIGERD
const enum detectorType myDetectorType = EIGER;
#elif JUNGFRAUD
const enum detectorType myDetectorType = JUNGFRAU;
#elif CHIPTESTBOARDD
const enum detectorType myDetectorType = CHIPTESTBOARD;
#elif MOENCHD
const enum detectorType myDetectorType = MOENCH;
#else
const enum detectorType myDetectorType = GENERIC;
#endif

// Global variables from communication_funcs
extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;
extern int isControlServer;
extern int ret;
extern int fnum;
extern char mess[MAX_STR_LENGTH];

// Variables that will be exported
int sockfd = 0;
int debugflag = 0;

// Local variables
int (*flist[NUM_DET_FUNCTIONS])(int);
#ifdef EIGERD
uint32_t dhcpipad = 0;
#endif

enum updateRet {NO_UPDATE, UPDATE};

/* initialization functions */

int printSocketReadError() {
	FILE_LOG(logERROR, ("Error reading from socket. Possible socket crash.\n"));
	return FAIL;
}


void init_detector() {
#ifdef VIRTUAL
	FILE_LOG(logINFO, ("This is a VIRTUAL detector\n"));
#endif
	if (isControlServer) {
	    basictests();
	    initControlServer();
#ifdef EIGERD
	    dhcpipad = getDetectorIP();
#endif
	}
	else initStopServer();
	strcpy(mess,"dummy message");
	strcpy(lastClientIP,"none");
	strcpy(thisClientIP,"none1");
	lockStatus=0;
}


int decode_function(int file_des) {
	ret = FAIL;

	int n = receiveData(file_des,&fnum,sizeof(fnum),INT32);
	if (n <= 0) {
		FILE_LOG(logDEBUG3, ("ERROR reading from socket n=%d, fnum=%d, file_des=%d, fname=%s\n",
				n, fnum, file_des, getFunctionName((enum detFuncs)fnum)));
		return FAIL;
	} else
		FILE_LOG(logDEBUG3, ("Received %d bytes\n", n ));

		if (fnum < 0 || fnum >= NUM_DET_FUNCTIONS) {
		FILE_LOG(logERROR, ("Unknown function enum %d\n", fnum));
		ret=(M_nofunc)(file_des);
	} else {
		FILE_LOG(logDEBUG1, (" calling function fnum=%d, (%s)\n",
				fnum,  getFunctionName((enum detFuncs)fnum)));
		ret = (*flist[fnum])(file_des);

		if (ret == FAIL) {
			FILE_LOG(logDEBUG1, ("Error executing the function = %d (%s)\n",
					fnum, getFunctionName((enum detFuncs)fnum)));
		} else FILE_LOG(logDEBUG1, ("Function (%s) executed %s\n",
				getFunctionName((enum detFuncs)fnum), getRetName()));
	}
	return ret;
}

const char* getRetName() {
	switch(ret) {
	case OK:			return "OK";
	case FAIL:			return "FAIL";
	case FORCE_UPDATE: 	return "FORCE_UPDATE";
	case GOODBYE:		return "GOODBYE";
	case REBOOT:		return "REBOOT";
	default:			return "unknown";
	}
}

const char* getTimerName(enum timerIndex ind) {
    switch (ind) {
    case FRAME_NUMBER:              return "frame_number";
    case ACQUISITION_TIME:          return "acquisition_time";
    case FRAME_PERIOD:              return "frame_period";
    case DELAY_AFTER_TRIGGER:       return "delay_after_trigger";
    case GATES_NUMBER:              return "gates_number";
    case CYCLES_NUMBER:             return "cycles_number";
    case ACTUAL_TIME:               return "actual_time";
    case MEASUREMENT_TIME:          return "measurement_time";
    case PROGRESS:                  return "progress";
    case MEASUREMENTS_NUMBER:       return "measurements_number";
    case FRAMES_FROM_START:         return "frames_from_start";
    case FRAMES_FROM_START_PG:      return "frames_from_start_pg";
    case SAMPLES:              		return "samples";
    case SUBFRAME_ACQUISITION_TIME: return "subframe_acquisition_time";
    case SUBFRAME_DEADTIME:         return "subframe_deadtime";
    case STORAGE_CELL_NUMBER:       return "storage_cell_number";
    case STORAGE_CELL_DELAY:		return "storage_cell_delay";
    default:                        return "unknown_timer";
    }
}


const char* getSpeedName(enum speedVariable ind) {
    switch (ind) {
    case CLOCK_DIVIDER:  return "clock_divider";
    case ADC_CLOCK:      return "adc_clock";
    case ADC_PHASE:      return "adc_phase";
    case ADC_PIPELINE:   return "adc_pipeline";
    case DBIT_CLOCK:     return "dbit_clock";
    case DBIT_PHASE:     return "dbit_phase";
    case DBIT_PIPELINE:  return "dbit_pipeline";
    case MAX_ADC_PHASE_SHIFT:  	return "max_adc_phase_shift";
    case MAX_DBIT_PHASE_SHIFT:  return "max_dbit_phase_shift";
    default:             return "unknown_speed";
    }
}

const char* getRunStateName(enum runStatus ind) {
    switch (ind) {
    case IDLE:  		return "idle";
    case ERROR:    		return "error";
    case WAITING:   	return "waiting";
    case RUN_FINISHED:  return "run_finished";
    case TRANSMITTING:  return "transmitting";
    case RUNNING:   	return "running";
    case STOPPED:     	return "stopped";
    default:            return "unknown";
    }
}

const char* getFunctionName(enum detFuncs func) {
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
	case F_SET_PATTERN_IO_CONTROL:			return "F_SET_PATTERN_IO_CONTROL";
	case F_SET_PATTERN_CLOCK_CONTROL:		return "F_SET_PATTERN_CLOCK_CONTROL";
	case F_SET_PATTERN_WORD:				return "F_SET_PATTERN_WORD";
	case F_SET_PATTERN_LOOP:				return "F_SET_PATTERN_LOOP";
	case F_SET_PATTERN_WAIT_ADDR:			return "F_SET_PATTERN_WAIT_ADDR";
	case F_SET_PATTERN_WAIT_TIME:			return "F_SET_PATTERN_WAIT_TIME";
	case F_SET_PATTERN_MASK:				return "F_SET_PATTERN_MASK";
	case F_GET_PATTERN_MASK:				return "F_GET_PATTERN_MASK";
	case F_SET_PATTERN_BIT_MASK:			return "F_SET_PATTERN_BIT_MASK";
	case F_GET_PATTERN_BIT_MASK:			return "F_GET_PATTERN_BIT_MASK";
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
    case F_DIGITAL_IO_DELAY:              	return "F_DIGITAL_IO_DELAY";
    case F_COPY_DET_SERVER:              	return "F_COPY_DET_SERVER";
    case F_REBOOT_CONTROLLER:              	return "F_REBOOT_CONTROLLER";

	default:								return "Unknown Function";
	}
}

void function_table() {
	flist[F_EXEC_COMMAND]						= &exec_command;
	flist[F_GET_DETECTOR_TYPE]					= &get_detector_type;
	flist[F_SET_EXTERNAL_SIGNAL_FLAG]			= &set_external_signal_flag;
	flist[F_SET_EXTERNAL_COMMUNICATION_MODE]	= &set_external_communication_mode;
	flist[F_GET_ID]								= &get_id;
	flist[F_DIGITAL_TEST]						= &digital_test;
	flist[F_SET_DAC]							= &set_dac;
	flist[F_GET_ADC]							= &get_adc;
	flist[F_WRITE_REGISTER]						= &write_register;
	flist[F_READ_REGISTER]						= &read_register;
	flist[F_SET_MODULE]							= &set_module;
	flist[F_GET_MODULE]							= &get_module;
	flist[F_SET_SETTINGS]						= &set_settings;
	flist[F_GET_THRESHOLD_ENERGY]				= &get_threshold_energy;
	flist[F_START_ACQUISITION]					= &start_acquisition;
	flist[F_STOP_ACQUISITION]					= &stop_acquisition;
	flist[F_START_READOUT]						= &start_readout;
	flist[F_GET_RUN_STATUS]						= &get_run_status;
	flist[F_START_AND_READ_ALL]					= &start_and_read_all;
	flist[F_READ_ALL]							= &read_all;
	flist[F_SET_TIMER]							= &set_timer;
	flist[F_GET_TIME_LEFT]						= &get_time_left;
	flist[F_SET_DYNAMIC_RANGE]					= &set_dynamic_range;
	flist[F_SET_READOUT_FLAGS]					= &set_readout_flags;
	flist[F_SET_ROI]							= &set_roi;
	flist[F_SET_SPEED]							= &set_speed;
	flist[F_EXIT_SERVER]						= &exit_server;
	flist[F_LOCK_SERVER]						= &lock_server;
	flist[F_GET_LAST_CLIENT_IP]					= &get_last_client_ip;
	flist[F_SET_PORT]							= &set_port;
	flist[F_UPDATE_CLIENT]						= &update_client;
	flist[F_CONFIGURE_MAC]						= &configure_mac;
	flist[F_LOAD_IMAGE]							= &load_image;
	flist[F_READ_COUNTER_BLOCK]					= &read_counter_block;
	flist[F_RESET_COUNTER_BLOCK]				= &reset_counter_block;
	flist[F_ENABLE_TEN_GIGA]					= &enable_ten_giga;
	flist[F_SET_ALL_TRIMBITS]					= &set_all_trimbits;
	flist[F_SET_PATTERN_IO_CONTROL]				= &set_pattern_io_control;
	flist[F_SET_PATTERN_CLOCK_CONTROL]			= &set_pattern_clock_control;
	flist[F_SET_PATTERN_WORD]					= &set_pattern_word;
	flist[F_SET_PATTERN_LOOP]					= &set_pattern_loop;
	flist[F_SET_PATTERN_WAIT_ADDR]				= &set_pattern_wait_addr;	
	flist[F_SET_PATTERN_WAIT_TIME]				= &set_pattern_wait_time;
	flist[F_SET_PATTERN_MASK]					= &set_pattern_mask;
	flist[F_GET_PATTERN_MASK]					= &get_pattern_mask;
	flist[F_SET_PATTERN_BIT_MASK]				= &set_pattern_bit_mask;
	flist[F_GET_PATTERN_BIT_MASK]				= &get_pattern_bit_mask;
	flist[F_WRITE_ADC_REG]						= &write_adc_register;
	flist[F_SET_COUNTER_BIT]					= &set_counter_bit;
	flist[F_PULSE_PIXEL]						= &pulse_pixel;
	flist[F_PULSE_PIXEL_AND_MOVE]				= &pulse_pixel_and_move;
	flist[F_PULSE_CHIP]							= &pulse_chip;
	flist[F_SET_RATE_CORRECT]					= &set_rate_correct;
	flist[F_GET_RATE_CORRECT]					= &get_rate_correct;
	flist[F_SET_NETWORK_PARAMETER]				= &set_network_parameter;
	flist[F_PROGRAM_FPGA]						= &program_fpga;
	flist[F_RESET_FPGA]							= &reset_fpga;
	flist[F_POWER_CHIP]							= &power_chip;
	flist[F_ACTIVATE]							= &set_activate;
	flist[F_PREPARE_ACQUISITION]				= &prepare_acquisition;
	flist[F_THRESHOLD_TEMP]                     = &threshold_temp;
	flist[F_TEMP_CONTROL]                       = &temp_control;
	flist[F_TEMP_EVENT]                         = &temp_event;
	flist[F_AUTO_COMP_DISABLE]                  = &auto_comp_disable;
	flist[F_STORAGE_CELL_START]                 = &storage_cell_start;
	flist[F_CHECK_VERSION]                 		= &check_version;
	flist[F_SOFTWARE_TRIGGER]                 	= &software_trigger;
	flist[F_LED]                 				= &led;
	flist[F_DIGITAL_IO_DELAY]                 	= &digital_io_delay;
	flist[F_COPY_DET_SERVER]                 	= &copy_detector_server;
	flist[F_REBOOT_CONTROLLER]                 	= &reboot_controller;

	// check
	if (NUM_DET_FUNCTIONS  >= RECEIVER_ENUM_START) {
		FILE_LOG(logERROR, ("The last detector function enum has reached its limit\nGoodbye!\n"));
		exit(EXIT_FAILURE);
	}

	int iloop = 0;
	for (iloop = 0; iloop < NUM_DET_FUNCTIONS ; ++iloop) {
		FILE_LOG(logDEBUG1, ("function fnum=%d, (%s)\n", iloop,
				getFunctionName((enum detFuncs)iloop)));
	}
}

void functionNotImplemented() {
	ret = FAIL;
	sprintf(mess, "Function (%s) is not implemented for this detector\n",
			getFunctionName((enum detFuncs)fnum));
	FILE_LOG(logERROR, (mess));
}

void modeNotImplemented(char* modename, int mode) {
	ret = FAIL;
	sprintf(mess, "%s (%d) is not implemented for this detector\n", modename, mode);
	FILE_LOG(logERROR,(mess));
}

void validate(int arg, int retval, char* modename, enum numberMode nummode) {
	if (ret == OK && arg != -1 && retval != arg) {
		ret = FAIL;
		if (nummode == HEX)
			sprintf(mess, "Could not %s. Set 0x%x, but read 0x%x\n",
				modename, arg, retval);
		else
			sprintf(mess, "Could not %s. Set %d, but read %d\n",
				modename, arg, retval);
		FILE_LOG(logERROR,(mess));
	}
}

void validate64(int64_t arg, int64_t retval, char* modename, enum numberMode nummode) {
	if (ret == OK && arg != -1 && retval != arg) {
		ret = FAIL;
		if (nummode == HEX)
			sprintf(mess, "Could not %s. Set 0x%llx, but read 0x%llx\n",
				modename, (long long unsigned int)arg, (long long unsigned int)retval);
		else
			sprintf(mess, "Could not %s. Set %lld, but read %lld\n",
				modename, (long long unsigned int)arg, (long long unsigned int)retval);
		FILE_LOG(logERROR,(mess));
	}
}

int executeCommand(char* command, char* result, enum TLogLevel level) {
	const size_t tempsize = 256;
	char temp[tempsize];
	memset(temp, 0, tempsize);
	memset(result, 0, MAX_STR_LENGTH);

	FILE_LOG(level, ("Executing command:\n[%s]\n", command));
	strcat(command, " 2>&1");

	fflush(stdout);
	FILE* sysFile = popen(command, "r");
	while(fgets(temp, tempsize, sysFile) != NULL) {
		// size left excludes terminating character
		size_t sizeleft = MAX_STR_LENGTH - strlen(result) - 1;
		// more than the command
		if (tempsize > sizeleft) {
			strncat(result, temp, sizeleft);
			break;
		}
		strncat(result, temp, tempsize);
		memset(temp, 0, tempsize);
	}
	int sucess = pclose(sysFile);
	if (strlen(result)) {
		if (sucess) {
			sucess = FAIL;
			FILE_LOG(logERROR, ("%s\n", result));
		} else {
			FILE_LOG(level, ("Result:\n[%s]\n", result));
		}
	}
	return sucess;
}

int  M_nofunc(int file_des) {
	ret = FAIL;
	memset(mess, 0, sizeof(mess));

	// to receive any arguments
	int n = 1;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	sprintf(mess,"Unrecognized Function enum %d. Please do not proceed.\n", fnum);
	FILE_LOG(logERROR, (mess));
	return Server_SendResult(file_des, OTHER, NO_UPDATE, NULL, 0);
}


// Jungfrau program mode
int  M_nofuncMode(int file_des) {
	ret = FAIL;
	memset(mess, 0, sizeof(mess));

	// to receive any arguments
	int n = 1;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	sprintf(mess,"This Function %s cannot be executed as the "
			"On-board detector server in update mode.\n"
			"Restart detector server in normal mode (without any arguments) to continue.\n",
			getFunctionName((enum detFuncs)fnum));
	FILE_LOG(logERROR, (mess));
	return Server_SendResult(file_des, OTHER, NO_UPDATE, NULL, 0);
}




int exec_command(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	if (receiveData(file_des, cmd, MAX_STR_LENGTH, OTHER) < 0)
		return printSocketReadError();

	// set
	if (Server_VerifyLock() == OK) {
		ret = executeCommand(cmd, retval, logINFO);
	}
	return Server_SendResult(file_des, OTHER, NO_UPDATE, retval, sizeof(retval));
}




int get_detector_type(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum detectorType retval = myDetectorType;
	FILE_LOG(logDEBUG1,("Returning detector type %d\n", retval));
	return Server_SendResult(file_des, INT32, NO_UPDATE, &retval, sizeof(retval));
}





int set_external_signal_flag(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1,-1};
	enum externalSignalFlag retval= GET_EXTERNAL_SIGNAL_FLAG;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();

	int signalindex = args[0];
	enum externalSignalFlag flag = args[1];
	FILE_LOG(logDEBUG1, ("Setting external signal %d to flag %d\n", signalindex, flag));

#ifndef GOTTHARDD
	functionNotImplemented();
#else
	if (signalindex > 0)
	    modeNotImplemented("Signal index", signalindex);
	else {
	    // set
	    if ((flag != GET_EXTERNAL_SIGNAL_FLAG) && (Server_VerifyLock() == OK)) {
	        setExtSignal(flag);
	    }
	    // get
	    retval = getExtSignal();
	    validate((int)flag, (int)retval, "set external signal flag", DEC);
	    FILE_LOG(logDEBUG1, ("External Signal Flag: %d\n", retval));
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int set_external_communication_mode(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum externalCommunicationMode arg = GET_EXTERNAL_COMMUNICATION_MODE;
	enum externalCommunicationMode retval = GET_EXTERNAL_COMMUNICATION_MODE;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting external communication mode to %d\n", arg));

	// set
	if ((arg != GET_EXTERNAL_COMMUNICATION_MODE) && (Server_VerifyLock() == OK)) {
		switch (arg) {
		case AUTO_TIMING:
		case TRIGGER_EXPOSURE:
#ifdef EIGERD
		case GATED:
		case BURST_TRIGGER:
#endif
			setTiming(arg);
			break;
		default:
			modeNotImplemented("Timing mode", (int)arg);
			break;
		}
	}
	// get
	retval = getTiming();
	validate((int)arg, (int)retval, "set timing mode", DEC);
	FILE_LOG(logDEBUG1, ("Timing Mode: %d\n",retval));
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int get_id(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum idMode arg = 0;
	int64_t retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Getting Id %d\n", arg));

	// get
	switch (arg) {
#ifndef GOTTHARDD
	case SOFTWARE_FIRMWARE_API_VERSION:
	case DETECTOR_SERIAL_NUMBER:
#endif
	case DETECTOR_FIRMWARE_VERSION:
	case DETECTOR_SOFTWARE_VERSION:
		retval = getDetectorId(arg);
		FILE_LOG(logDEBUG1, ("Id(%d): %lld\n", retval));
		break;
	default:
		modeNotImplemented("ID Index", (int)arg);
		break;
	}
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}





int digital_test(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1, -1};
	int retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	enum digitalTestMode mode = args[0];
#ifdef GOTTHARDD
	int ival = args[1];
	FILE_LOG(logDEBUG1, ("Digital test, mode = %d, ival:%d\n", mode, ival));
#else
	FILE_LOG(logDEBUG1, ("Digital test, mode = %d\n", mode));
#endif

#ifdef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		switch (mode) {

		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_BUS_TEST:
#ifdef GOTTHARDD
        case DIGITAL_BIT_TEST:
            retval = detectorTest(mode, ival);
            break;
#else
            retval = detectorTest(mode);
#endif
			FILE_LOG(logDEBUG1, ("Digital Test (%d): %d\n", mode, retval));
			break;
		default:
			modeNotImplemented("Digital Test Mode", (int)mode);
			break;
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int set_dac(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[3] = {-1, -1, -1};
    int retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	enum dacIndex ind = args[0];
	int mV = args[1];
	int val = args[2];
    enum DACINDEX serverDacIndex = 0;

    // check if dac exists for this detector
    switch (ind) {
#ifdef GOTTHARDD
    case G_VREF_DS :
        serverDacIndex = VREF_DS;
        break;
    case G_VCASCN_PB:
        serverDacIndex = VCASCN_PB;
        break;
    case G_VCASCP_PB:
        serverDacIndex = VCASCP_PB;
        break;
    case G_VOUT_CM:
        serverDacIndex = VOUT_CM;
        break;
    case G_VCASC_OUT:
        serverDacIndex = VCASC_OUT;
        break;
    case G_VIN_CM:
        serverDacIndex = VIN_CM;
        break;
    case G_VREF_COMP:
        serverDacIndex = VREF_COMP;
        break;
    case G_IB_TESTC:
        serverDacIndex = IB_TESTC;
        break;
    case HIGH_VOLTAGE:
        break;
#elif EIGERD
    case TRIMBIT_SIZE:
        serverDacIndex = VTR;
        break;
    case THRESHOLD:
        serverDacIndex = VTHRESHOLD;
        break;
    case E_SvP:
        serverDacIndex = SVP;
        break;
    case E_SvN:
        serverDacIndex = SVN;
        break;
    case E_Vtr:
        serverDacIndex = VTR;
        break;
    case E_Vrf:
        serverDacIndex = VRF;
        break;
    case E_Vrs:
        serverDacIndex = VRS;
        break;
    case E_Vtgstv:
        serverDacIndex = VTGSTV;
        break;
    case E_Vcmp_ll:
        serverDacIndex = VCMP_LL;
        break;
    case E_Vcmp_lr:
        serverDacIndex = VCMP_LR;
        break;
    case E_cal:
        serverDacIndex = CAL;
        break;
    case E_Vcmp_rl:
        serverDacIndex = VCMP_RL;
        break;
    case E_Vcmp_rr:
        serverDacIndex = VCMP_RR;
        break;
    case E_rxb_rb:
        serverDacIndex = RXB_RB;
        break;
    case E_rxb_lb:
        serverDacIndex = RXB_LB;
        break;
    case E_Vcp:
        serverDacIndex = VCP;
        break;
    case E_Vcn:
        serverDacIndex = VCN;
        break;
    case E_Vis:
        serverDacIndex = VIS;
        break;
    case HIGH_VOLTAGE:
    case IO_DELAY:
        break;
#elif CHIPTESTBOARDD
    case ADC_VPP:
    case HIGH_VOLTAGE:
        break;
    case V_POWER_A:
        serverDacIndex = D_PWR_A;
        break;
    case V_POWER_B:
        serverDacIndex = D_PWR_B;
        break;
    case V_POWER_C:
        serverDacIndex = D_PWR_C;
        break;
    case V_POWER_D:
        serverDacIndex = D_PWR_D;
        break;
    case V_POWER_IO:
        serverDacIndex = D_PWR_IO;
        break;
    case V_POWER_CHIP:
        serverDacIndex = D_PWR_CHIP;
        break;
    case V_LIMIT:
        break;
#elif MOENCHD
    case ADC_VPP:
    case HIGH_VOLTAGE:
    case V_LIMIT:
        break;
#endif
    default:
#ifdef JUNGFRAUD
        if ((ind == HIGH_VOLTAGE) || (ind < NDAC_OLDBOARD)) {  //for compatibility with old board
            serverDacIndex = ind;
            break;
        }
#elif CHIPTESTBOARDD
        if (ind < NDAC_ONLY) {
            serverDacIndex = ind;
            break;
        }
#elif MOENCHD
        if (ind < NDAC) {
            serverDacIndex = ind;
            break;
        }
#endif
        modeNotImplemented("Dac Index", (int)ind);
        break;
    }

    // index exists
    if (ret == OK) {

        FILE_LOG(logDEBUG1, ("Setting DAC %d to %d %s\n", serverDacIndex, val,
        		(mV ? "mV" : "dac units")));

    	// set & get
    	if ((val == -1) || (Server_VerifyLock() == OK)) {
    		switch(ind) {

    		// adc vpp
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
    		case ADC_VPP:
    		if (val < 0 || val > AD9257_GetMaxValidVref())  {
    		    ret = FAIL;
                sprintf(mess,"Could not set dac. Adc Vpp value should be between 0 and %d\n", AD9257_GetMaxValidVref());
                FILE_LOG(logERROR,(mess));
    		} else {
    		    AD9257_SetVrefVoltage(val);
    		    retval = val; // cannot read
    		}
    		break;
#endif

    		// io delay
#ifdef EIGERD
    		case IO_DELAY:
    			retval = setIODelay(val);
    			FILE_LOG(logDEBUG1, ("IODelay: %d\n", retval));
    		    validate(val, retval, "set iodelay", DEC);
    			break;
#endif

    		// high voltage
    		case HIGH_VOLTAGE:
    			retval = setHighVoltage(val);
    			FILE_LOG(logDEBUG1, ("High Voltage: %d\n", retval));
#if defined(JUNGFRAUD) || defined (CHIPTESTBOARDD) || defined(MOENCHD)
    			validate(val, retval, "set high voltage", DEC);
#endif
#ifdef GOTTHARDD
    			if (retval == -1) {
    			    ret = FAIL;
    			    strcpy(mess,"Invalid Voltage. Valid values are 0, 90, 110, 120, 150, 180, 200\n");
    			    FILE_LOG(logERROR,(mess));
    			} else
    			    validate(val, retval, "set high voltage", DEC);
#elif EIGERD
    			if ((retval != SLAVE_HIGH_VOLTAGE_READ_VAL) && (retval < 0)) {
    				ret = FAIL;
    				if (retval == -1)
    					sprintf(mess, "Setting high voltage failed. Bad value %d. "
    							"The range is from 0 to 200 V.\n",val);
    				else if (retval == -2)
    					strcpy(mess, "Setting high voltage failed. "
    							"Serial/i2c communication failed.\n");
    				else if (retval == -3)
    					strcpy(mess, "Getting high voltage failed. "
    							"Serial/i2c communication failed.\n");
    				FILE_LOG(logERROR,(mess));
    			}
#endif
    			break;

    			// power, vlimit
#ifdef CHIPTESTBOARDD
            case V_POWER_A:
            case V_POWER_B:
            case V_POWER_C:
            case V_POWER_D:
            case V_POWER_IO:
                if (val != -1) {
                    if (!mV) {
                        ret = FAIL;
                        sprintf(mess,"Could not set power. Power regulator %d should be in mV and not dac units.\n", ind);
                        FILE_LOG(logERROR,(mess));
                    } else if (checkVLimitCompliant(val) == FAIL) {
                        ret = FAIL;
                        sprintf(mess,"Could not set power. Power regulator %d exceeds voltage limit %d.\n", ind, getVLimit());
                        FILE_LOG(logERROR,(mess));
                    } else if (!isPowerValid(serverDacIndex, val)) {
                        ret = FAIL;
                        sprintf(mess,"Could not set power. Power regulator %d should be between %d and %d mV\n",
                                ind, (serverDacIndex == D_PWR_IO ? VIO_MIN_MV : POWER_RGLTR_MIN), (VCHIP_MAX_MV - VCHIP_POWER_INCRMNT));
                        FILE_LOG(logERROR,(mess));
                    } else {
                        setPower(serverDacIndex, val);

                    }
                }
                retval = getPower(serverDacIndex);
                FILE_LOG(logDEBUG1, ("Power regulator(%d): %d\n", ind, retval));
                validate(val, retval, "set power regulator", DEC);
    			break;


            case V_POWER_CHIP:
                if (val >= 0) {
                    ret = FAIL;
                    sprintf(mess,"Can not set Vchip. Can only be set automatically in the background (+200mV from highest power regulator voltage).\n");
                    FILE_LOG(logERROR,(mess));
                    /* restrict users from setting vchip
                    if (!mV) {
                        ret = FAIL;
                        sprintf(mess,"Could not set Vchip. Should be in mV and not dac units.\n");
                        FILE_LOG(logERROR,(mess));
                    } else if (!isVchipValid(val)) {
                        ret = FAIL;
                        sprintf(mess,"Could not set Vchip. Should be between %d and %d mV\n", VCHIP_MIN_MV, VCHIP_MAX_MV);
                        FILE_LOG(logERROR,(mess));
                    } else {
                        setVchip(val);
                    }
                    */
                }
                retval = getVchip();
                FILE_LOG(logDEBUG1, ("Vchip: %d\n", retval));
                if (ret == OK && val != -1 && val != -100 && retval != val) {
                    ret = FAIL;
                    sprintf(mess, "Could not set vchip. Set %d, but read %d\n", val, retval);
                    FILE_LOG(logERROR,(mess));
                }
                break;
#endif

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
            case V_LIMIT:
                if (val >= 0) {
                    if (!mV) {
                        ret = FAIL;
                        strcpy(mess,"Could not set power. VLimit should be in mV and not dac units.\n");
                        FILE_LOG(logERROR,(mess));
                    } else {
                        setVLimit(val);
                    }
                }
                retval = getVLimit();
                FILE_LOG(logDEBUG1, ("VLimit: %d\n", retval));
                validate(val, retval, "set vlimit", DEC);
                break;
#endif

                // dacs
    			default:
    			    if (mV && val > DAC_MAX_MV) {
                        ret = FAIL;
                        sprintf(mess,"Could not set dac %d to value %d. Allowed limits (0 - %d mV).\n", ind, val, DAC_MAX_MV);
                        FILE_LOG(logERROR,(mess));
    			    } else if (!mV && val > getMaxDacSteps() ) {
                        ret = FAIL;
                        sprintf(mess,"Could not set dac %d to value %d. Allowed limits (0 - %d dac units).\n", ind, val, getMaxDacSteps());
                        FILE_LOG(logERROR,(mess));
    			    } else {
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
    			        if ((val != -1 && mV && checkVLimitCompliant(val) == FAIL) ||
    			                (val != -1 && !mV && checkVLimitDacCompliant(val) == FAIL)) {
                            ret = FAIL;
                            sprintf(mess,"Could not set dac %d to value %d. "
                                    "Exceeds voltage limit %d.\n",
                                    ind, (mV ? val : dacToVoltage(val)), getVLimit());
                            FILE_LOG(logERROR,(mess));
    			        } else
#endif
    			        setDAC(serverDacIndex, val, mV);
    			        retval = getDAC(serverDacIndex, mV);
    			    }
#ifdef EIGERD
                    if (val != -1) {
                        //changing dac changes settings to undefined
                        switch(serverDacIndex) {
                        case VCMP_LL:
                        case VCMP_LR:
                        case VCMP_RL:
                        case VCMP_RR:
                        case VRF:
                        case VCP:
                            setSettings(UNDEFINED);
                            FILE_LOG(logERROR, ("Settings has been changed "
                            		"to undefined (changed specific dacs)\n"));
                            break;
                        default:
                            break;
                        }
                    }
#endif
                    //check
                    if (ret == OK) {
                        if ((abs(retval - val) <= 5) || val == -1) {
                            ret = OK;
                        } else {
                            ret = FAIL;
                            sprintf(mess,"Setting dac %d : wrote %d but read %d\n", serverDacIndex, val, retval);
                            FILE_LOG(logERROR,(mess));
                        }
                    }
                    FILE_LOG(logDEBUG1, ("Dac (%d): %d %s\n\n", serverDacIndex, retval, (mV ? "mV" : "dac units")));
    				break;
    		}
    	}
    }
    return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int get_adc(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum dacIndex ind = 0;
	int retval = -1;

	if (receiveData(file_des, &ind, sizeof(ind), INT32) < 0)
		return printSocketReadError();

#ifdef MOENCHD
    functionNotImplemented();
#else
	enum ADCINDEX serverAdcIndex = 0;

	// get
	switch (ind) {
#if defined(GOTTHARDD) || defined(JUNGFRAUD)
	case TEMPERATURE_FPGA:
		serverAdcIndex = TEMP_FPGA;
		break;
	case TEMPERATURE_ADC:
		serverAdcIndex = TEMP_ADC;
		break;
#elif EIGERD
	case TEMPERATURE_FPGAEXT:
		serverAdcIndex = TEMP_FPGAEXT;
		break;
	case TEMPERATURE_10GE:
		serverAdcIndex = TEMP_10GE;
		break;
	case TEMPERATURE_DCDC:
		serverAdcIndex = TEMP_DCDC;
		break;
	case TEMPERATURE_SODL:
		serverAdcIndex = TEMP_SODL;
		break;
	case TEMPERATURE_SODR:
		serverAdcIndex = TEMP_SODR;
		break;
	case TEMPERATURE_FPGA:
		serverAdcIndex = TEMP_FPGA;
		break;
	case TEMPERATURE_FPGA2:
		serverAdcIndex = TEMP_FPGAFEBL;
		break;
	case TEMPERATURE_FPGA3:
		serverAdcIndex = TEMP_FPGAFEBR;
		break;
#elif CHIPTESTBOARDD
    case V_POWER_A:
        serverAdcIndex = V_PWR_A;
        break;
    case V_POWER_B:
        serverAdcIndex = V_PWR_B;
        break;
    case V_POWER_C:
        serverAdcIndex = V_PWR_C;
        break;
    case V_POWER_D:
        serverAdcIndex = V_PWR_D;
        break;
    case V_POWER_IO:
        serverAdcIndex = V_PWR_IO;
        break;
    case I_POWER_A:
        serverAdcIndex = I_PWR_A;
        break;
    case I_POWER_B:
        serverAdcIndex = I_PWR_B;
        break;
    case I_POWER_C:
        serverAdcIndex = I_PWR_C;
        break;
    case I_POWER_D:
        serverAdcIndex = I_PWR_D;
        break;
    case I_POWER_IO:
        serverAdcIndex = I_PWR_IO;
        break;
#endif
	default:
#ifdef CHIPTESTBOARDD
        if (ind >= SLOW_ADC0 && ind <= SLOW_ADC_TEMP) {
            serverAdcIndex = ind;
            break;
        }
#endif
		modeNotImplemented("Adc Index", (int)ind);
		break;
	}

	// valid index
	if (ret == OK) {
		FILE_LOG(logDEBUG1, ("Getting ADC %d\n", serverAdcIndex));
		retval = getADC(serverAdcIndex);
		FILE_LOG(logDEBUG1, ("ADC(%d): %d\n", serverAdcIndex, retval));
	}
#endif

	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int write_register(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t args[2] = {-1, -1};
    uint32_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    uint32_t addr = args[0];
    uint32_t val = args[1];
    FILE_LOG(logDEBUG1, ("Writing to register 0x%x, data 0x%x\n", addr, val));

    // only set
    if (Server_VerifyLock() == OK) {
#ifdef GOTTHARDD
        retval = writeRegister16And32(addr, val);
#else
        retval = writeRegister(addr, val);
#endif
        // validate
        if (retval != val) {
            ret = FAIL;
            sprintf(mess,"Could not write to register 0x%x. Wrote 0x%x but read 0x%x\n", addr, val, retval);
            FILE_LOG(logERROR,(mess));
        }
        FILE_LOG(logDEBUG1, ("Write register (0x%x): 0x%x\n", retval));
    }
    return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int read_register(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint32_t addr = -1;
	uint32_t retval = -1;

	if (receiveData(file_des, &addr, sizeof(addr), INT32) < 0)
		return printSocketReadError();

	FILE_LOG(logDEBUG1, ("Reading from register 0x%x\n", addr));

	// get
#ifdef GOTTHARDD
	retval = readRegister16And32(addr);
#else
	retval = readRegister(addr);
#endif
	FILE_LOG(logINFO, ("Read register (0x%x): 0x%x\n", addr, retval));

	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int set_module(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum detectorSettings retval = -1;

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
    functionNotImplemented();
#else

	sls_detector_module module;
	int *myDac = NULL;
	int *myChan = NULL;
	module.dacs = NULL;
	module.chanregs = NULL;

	// allocate to receive arguments
	// allocate dacs
	myDac = (int*)malloc(getNumberOfDACs() * sizeof(int));
	// error
	if (getNumberOfDACs() > 0 && myDac == NULL) {
		ret = FAIL;
		sprintf(mess, "Could not allocate dacs\n");
		FILE_LOG(logERROR,(mess));
	} else
		module.dacs = myDac;

#ifdef EIGERD
	// allocate chans
	if (ret == OK) {
		myChan = (int*)malloc(getTotalNumberOfChannels() * sizeof(int));
		if (getTotalNumberOfChannels() > 0 && myChan == NULL) {
			ret = FAIL;
			sprintf(mess,"Could not allocate chans\n");
			FILE_LOG(logERROR,(mess));
		} else
			module.chanregs = myChan;
	}
#endif
	// receive arguments
	if (ret == OK) {
		module.nchip = getNumberOfChips();
		module.nchan = getTotalNumberOfChannels();
		module.ndac = getNumberOfDACs();
		int ts = receiveModule(file_des, &module);
		if (ts < 0) {
			if (myChan != NULL) free(myChan);
			if (myDac != NULL) 	free(myDac);
			return printSocketReadError();
		}
		FILE_LOG(logDEBUG1, ("module register is %d, nchan %d, nchip %d, "
				"ndac %d, iodelay %d, tau %d, eV %d\n",
				module.reg, module.nchan, module.nchip,
				module.ndac, module.iodelay, module.tau, module.eV));
		// should at least have a dac
		if (ts <= sizeof(sls_detector_module)) {
			ret = FAIL;
			sprintf(mess, "Cannot set module. Received incorrect number of dacs or channels\n");
			FILE_LOG(logERROR,(mess));
		}
	}

	// receive all arguments
	if (ret == FAIL) {
		int n = 1;
		while (n > 0)
			n = receiveData(file_des, mess, MAX_STR_LENGTH, OTHER);
	}

	// only set
	else if (Server_VerifyLock() == OK) {
		// check index
		switch (module.reg) {
#ifdef EIGERD
		case STANDARD:
		case HIGHGAIN:
		case LOWGAIN:
		case VERYHIGHGAIN:
		case VERYLOWGAIN:
#elif JUNGFRAUD
		case GET_SETTINGS:
		case DYNAMICGAIN:
		case DYNAMICHG0:
		case FIXGAIN1:
		case FIXGAIN2:
		case FORCESWITCHG1:
		case FORCESWITCHG2:
#elif GOTTHARDD
		case GET_SETTINGS:
		case DYNAMICGAIN:
		case HIGHGAIN:
		case LOWGAIN:
		case MEDIUMGAIN:
		case VERYHIGHGAIN:
#endif
			break;
		default:
			modeNotImplemented("Settings", (int)module.reg);
			break;
		}

		ret = setModule(module, mess);
		retval = getSettings();
		validate(module.reg, (int)retval, "set module (settings)", DEC);
		FILE_LOG(logDEBUG1, ("Settings: %d\n", retval));
	}
	if (myChan != NULL) free(myChan);
	if (myDac != NULL) 	free(myDac);
#endif

	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int get_module(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	sls_detector_module module;
	int *myDac = NULL;
	int *myChan = NULL;
	module.dacs = NULL;
	module.chanregs = NULL;

	// allocate to send arguments
	// allocate dacs
	myDac = (int*)malloc(getNumberOfDACs() * sizeof(int));
	// error
	if (getNumberOfDACs() > 0 && myDac == NULL) {
		ret = FAIL;
		sprintf(mess, "Could not allocate dacs\n");
		FILE_LOG(logERROR,(mess));
	} else
		module.dacs = myDac;

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
    functionNotImplemented();
#endif

#ifdef EIGERD
    // allocate chans
	if (ret == OK) {
		myChan = (int*)malloc(getTotalNumberOfChannels() * sizeof(int));
		if (getTotalNumberOfChannels() > 0 && myChan == NULL) {
			ret = FAIL;
			sprintf(mess,"Could not allocate chans\n");
			FILE_LOG(logERROR,(mess));
		} else
			module.chanregs=myChan;
	}
#endif

	// get module
	if (ret == OK) {
		module.nchip = getNumberOfChips();
		module.nchan = getTotalNumberOfChannels();
		module.ndac = getNumberOfDACs();

		// only get
		FILE_LOG(logDEBUG1, ("Getting module\n"));
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
		getModule(&module);
#endif
		FILE_LOG(logDEBUG1, ("Getting module. Settings:%d\n", module.reg));
	}

	Server_SendResult(file_des, INT32, UPDATE, NULL, 0);

	// send module, 0 is to receive partially (without trimbits etc)
	if (ret != FAIL) {
		ret = sendModule(file_des, &module);
	}
	if (myChan != NULL)	free(myChan);
	if (myDac != NULL) 	free(myDac);
	return ret;
}






int set_settings(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum detectorSettings isett = GET_SETTINGS;
	enum detectorSettings retval = GET_SETTINGS;

	if (receiveData(file_des, &isett, sizeof(isett), INT32) < 0)
		return printSocketReadError();

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
    functionNotImplemented();
#else
	FILE_LOG(logDEBUG1, ("Setting settings %d\n", isett));

	//set & get
	if ((isett == GET_SETTINGS) || (Server_VerifyLock() == OK)) {

		// check index
		switch(isett) {
		case GET_SETTINGS:
#ifdef JUNGFRAUD
		case DYNAMICGAIN:
		case DYNAMICHG0:
		case FIXGAIN1:
		case FIXGAIN2:
		case FORCESWITCHG1:
		case FORCESWITCHG2:
#elif GOTTHARDD
		case DYNAMICGAIN:
		case HIGHGAIN:
		case LOWGAIN:
		case MEDIUMGAIN:
		case VERYHIGHGAIN:
#endif
			break;
		default:
			if (myDetectorType == EIGER) {
				ret = FAIL;
				sprintf(mess, "Cannot set settings via SET_SETTINGS, use SET_MODULE\n");
				FILE_LOG(logERROR,(mess));
			} else
				modeNotImplemented("Settings Index", (int)isett);
			break;
		}

		// if index is okay, set & get
		if (ret == OK) {
			retval = setSettings(isett);
			FILE_LOG(logDEBUG1, ("Settings: %d\n", retval));
			validate((int)isett, (int)retval, "set settings", DEC);
#if defined(JUNGFRAUD) || defined (GOTTHARDD)
			if (ret == OK && isett >= 0) {
				ret = setDefaultDacs();
				if (ret == FAIL) {
					strcpy(mess,"Could change settings, but could not set to default dacs\n");
					FILE_LOG(logERROR,(mess));
				}
			}
#endif
		}
	}
#endif

	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int get_threshold_energy(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	FILE_LOG(logDEBUG1, ("Getting Threshold energy\n"));
#ifndef EIGERD
	functionNotImplemented();
#else
	// only get
	retval = getThresholdEnergy();
	FILE_LOG(logDEBUG1, ("Threshold energy: %d eV\n", retval));
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int start_acquisition(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Starting Acquisition\n"));
	// only set
	if (Server_VerifyLock() == OK) {
		ret = startStateMachine();
		if (ret == FAIL) {
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
			sprintf(mess, "Could not start acquisition. Could not create udp socket in server. Check rx_udpip & rx_udpport.\n");
#else
			sprintf(mess, "Could not start acquisition\n");
#endif
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG2, ("Starting Acquisition ret: %d\n", ret));
	}
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}



int stop_acquisition(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Stopping Acquisition\n"));
	// only set
	if (Server_VerifyLock() == OK) {
		ret = stopStateMachine();
		if (ret == FAIL) {
			sprintf(mess, "Could not stop acquisition\n");
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("Stopping Acquisition ret: %d\n", ret));
	}
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}





int start_readout(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Starting readout\n"));
#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = startReadOut();
		if (ret == FAIL) {
			sprintf(mess, "Could not start readout\n");
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("Starting readout ret: %d\n", ret));
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}






int get_run_status(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus retval = ERROR;

	FILE_LOG(logDEBUG1, ("Getting status\n"));
	// only get
	retval = getRunStatus();
	FILE_LOG(logDEBUG1, ("Status: %d\n", retval));
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int start_and_read_all(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Starting Acquisition and read all frames\n"));
	// start state machine
	FILE_LOG(logDEBUG1, ("Starting Acquisition\n"));
	// only set
	if (Server_VerifyLock() == OK) {
		ret = startStateMachine();
		if (ret == FAIL) {
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
			sprintf(mess, "Could not start acquisition. Could not create udp socket in server. Check rx_udpip & rx_udpport.\n");
#else
			sprintf(mess, "Could not start acquisition\n");
#endif
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG2, ("Starting Acquisition ret: %d\n", ret));

	}

	// lock or acquisition start error
	if (ret == FAIL)
		return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);

	// read all (again validate lock, but should pass and not fail)
	return read_all(file_des);
}




int read_all(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Reading all frames\n"));
	// only set
	if (Server_VerifyLock() == OK) {
		readFrame(&ret, mess);
	}
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}






int set_timer(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t args[2] = {-1,-1};
	int64_t retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT64) < 0)
		return printSocketReadError();
	enum timerIndex ind = (int)args[0];
	int64_t tns = args[1];
	char timerName[50] = {0};
	strcpy(timerName, getTimerName(ind));
#ifdef EIGERD
	int64_t subexptime = 0;
#endif
	FILE_LOG(logDEBUG1, ("Setting timer %s(%d) to %lld ns\n", timerName, (int)ind, tns));

	// set & get
	if ((tns == -1) || (Server_VerifyLock() == OK)) {

	    // check index
	    switch (ind) {
	    case FRAME_NUMBER:
	    case ACQUISITION_TIME:
	    case FRAME_PERIOD:
	    case CYCLES_NUMBER:
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
	    case SAMPLES:
#endif
#ifndef EIGERD
	    case DELAY_AFTER_TRIGGER:
#endif
	        retval = setTimer(ind, tns);
	        break;
#ifdef JUNGFRAUD
	    case STORAGE_CELL_NUMBER:
	        if (tns > MAX_STORAGE_CELL_VAL) {
	            ret = FAIL;
	            strcpy(mess,"Max Storage cell number should not exceed 15\n");
	            FILE_LOG(logERROR,(mess));
	            break;
	        }
	        retval = setTimer(ind,tns);
	        break;

	    case STORAGE_CELL_DELAY:
	        if (tns > MAX_STORAGE_CELL_DLY_NS_VAL) {
	            ret = FAIL;
	            sprintf(mess,"Max Storage cell delay value should not exceed %d ns\n", MAX_STORAGE_CELL_DLY_NS_VAL);
	            FILE_LOG(logERROR,(mess));
	            break;
	        }
	        retval = setTimer(ind,tns);
	        break;
#endif
#ifdef EIGERD
	    case SUBFRAME_ACQUISITION_TIME:
	        if (tns > ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) ) {
	            ret = FAIL;
	            strcpy(mess,"Sub Frame exposure time should not exceed 5.368 seconds\n");
	            FILE_LOG(logERROR,(mess));
	            break;
	        }
	        retval = setTimer(ind,tns);
	        break;
	    case SUBFRAME_DEADTIME:
	        subexptime = setTimer(SUBFRAME_ACQUISITION_TIME, -1);
	        if ((tns + subexptime) > ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) ) {
	            ret = FAIL;
	            sprintf(mess,"Sub Frame Period should not exceed 5.368 seconds. "
	                    "So sub frame dead time should not exceed %lfu seconds "
	                    "(subexptime = %lf seconds)\n",
	                    ((((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) - subexptime)/1E9),
	                    (subexptime/1E9));
	            FILE_LOG(logERROR,(mess));
	            break;
	        }
	        retval = setTimer(ind,tns);
	        break;
#endif
	    default:
	        modeNotImplemented(timerName, (int)ind);
	        break;
	    }

	    // validate
	    if (ret != FAIL) {
	        char vtimerName[50] = {0};
	        sprintf(vtimerName, "set %s", timerName);
#ifdef EIGERD
	        validate64(tns, retval, vtimerName, DEC); // copied to server, not read from detector register
#else
	        switch(ind) {
	        case FRAME_NUMBER:
	        case CYCLES_NUMBER:
	        case STORAGE_CELL_NUMBER:
	            validate64(tns, retval, vtimerName, DEC); // no conversion, so all good
	            break;
	        case SAMPLES:
	            if (retval == -1) {
	                ret = FAIL;
	                retval = setTimer(ind, -1);
	                sprintf(mess, "Could not set samples to %lld. Could not allocate RAM\n",
	                        (long long unsigned int)tns);
	                FILE_LOG(logERROR,(mess));
	            } else
	                validate64(tns, retval, vtimerName, DEC); // no conversion, so all good
	        case ACQUISITION_TIME:
	        case FRAME_PERIOD:
	        case DELAY_AFTER_TRIGGER:
	        case SUBFRAME_ACQUISITION_TIME:
	        case SUBFRAME_DEADTIME:
	        case STORAGE_CELL_DELAY:
	            // losing precision due to conversion to clock (also gotthard master delay is different)
	            if (validateTimer(ind, tns, retval) == FAIL) {
	                ret = FAIL;
	                sprintf(mess, "Could not %s. Set %lld, but read %lld\n", vtimerName,
	                        (long long unsigned int)tns, (long long unsigned int)retval);
	                FILE_LOG(logERROR,(mess));
	            }
	            break;

	        default:
	            break;
	        }
#endif
	    }
	}
	if (ret != FAIL) {
		FILE_LOG(logDEBUG1, ("Timer index %d: %lld\n", ind, retval));
	}
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}







int get_time_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum timerIndex ind = -1;
    int64_t retval = -1;

    if (receiveData(file_des, &ind, sizeof(ind), INT32) < 0)
        return printSocketReadError();
    FILE_LOG(logDEBUG1, ("Getting timer left index %d\n", ind));

    // only get
    // check index
#ifdef JUNGFRAUD
    if (ind == DELAY_AFTER_TRIGGER) {
        ret = FAIL;
        sprintf(mess,"Timer Left Index (%d) is not implemented for this release.\n", (int)ind);
        FILE_LOG(logERROR,(mess));
    }
#endif
    if (ret == OK) {
        switch(ind) {
#ifdef EIGERD
        case MEASURED_PERIOD:
        case MEASURED_SUBPERIOD:
#elif JUNGFRAUD
        case FRAMES_FROM_START:
        case FRAMES_FROM_START_PG:
        case ACTUAL_TIME:
        case MEASUREMENT_TIME:
        case FRAME_NUMBER:
        case FRAME_PERIOD:
        case DELAY_AFTER_TRIGGER:
        case CYCLES_NUMBER:
#elif GOTTHARDD
        case ACQUISITION_TIME:
        case FRAME_NUMBER:
        case FRAME_PERIOD:
        case DELAY_AFTER_TRIGGER:
        case CYCLES_NUMBER:
#elif CHIPTESTBOARDD
        case FRAMES_FROM_START:
        case FRAMES_FROM_START_PG:
        case ACTUAL_TIME:
        case MEASUREMENT_TIME:
        case FRAME_NUMBER:
        case FRAME_PERIOD:
        case DELAY_AFTER_TRIGGER:
        case CYCLES_NUMBER:
#elif MOENCHD
        case FRAMES_FROM_START:
        case FRAMES_FROM_START_PG:
        case ACTUAL_TIME:
        case MEASUREMENT_TIME:
        case FRAME_NUMBER:
        case FRAME_PERIOD:
        case DELAY_AFTER_TRIGGER:
        case CYCLES_NUMBER:
#endif
            retval = getTimeLeft(ind);
            FILE_LOG(logDEBUG1, ("Timer left index %d: %lld\n", ind, retval));
            break;
        default:
            modeNotImplemented("Timer left index", (int)ind);
            break;
        }
    }
    return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}






int set_dynamic_range(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int dr = -1;
	int retval = -1;

	if (receiveData(file_des, &dr, sizeof(dr), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting dr to %d\n", dr));

	// set & get
	if ((dr == -1) || (Server_VerifyLock() == OK)) {
		// check dr
		switch(dr) {
		case -1:
		case 16:
#ifdef EIGERD
		case 4:	case 8:	case 32:
#endif
			retval = setDynamicRange(dr);
			FILE_LOG(logDEBUG1, ("Dynamic range: %d\n", retval));
			validate(dr, retval, "set dynamic range", DEC);
			break;
		default:
			modeNotImplemented("Dynamic range", dr);
			break;
		}
	}
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int set_readout_flags(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum readOutFlags arg = GET_READOUT_FLAGS;
	enum readOutFlags retval = GET_READOUT_FLAGS;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting readout flags to %d\n", arg));

#if defined(JUNGFRAUD) || defined(GOTTHARDD) || defined(MOENCHD)
	functionNotImplemented();
#else
	// set & get
	if ((arg == GET_READOUT_FLAGS) || (Server_VerifyLock() == OK)) {

		switch(arg) {
		case GET_READOUT_FLAGS:
#ifdef EIGERD
		case STORE_IN_RAM:
		case CONTINOUS_RO:
		case PARALLEL:
		case NONPARALLEL:
		case SAFE:
		case SHOW_OVERFLOW:
		case NOOVERFLOW:
#elif CHIPTESTBOARDD
		case NORMAL_READOUT:
		case DIGITAL_ONLY:
		case ANALOG_AND_DIGITAL:
#endif
			retval = setReadOutFlags(arg);
			FILE_LOG(logDEBUG1, ("Read out flags: 0x%x\n", retval));
			validate((int)arg, (int)(retval & arg), "set readout flag", HEX);
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
            if (retval == -2) {
                ret = FAIL;
                sprintf(mess, "Readout Flags failed. Cannot allocate RAM\n");
                FILE_LOG(logERROR,(mess));
            }
#endif
			break;
		default:
			modeNotImplemented("Read out flag index", (int)arg);
			break;
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int set_roi(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int narg = -1;
	ROI arg[MAX_ROIS];
	int nretval = -1;
	ROI* retval = NULL;

	// receive number of ROIs
	if (receiveData(file_des, &narg, sizeof(narg), INT32) < 0)
		return printSocketReadError();
	// receive ROIs
	{
		int iloop = 0;
		for (iloop = 0; iloop < narg; ++iloop) {
			if (receiveData(file_des, &arg[iloop].xmin, sizeof(int), INT32) < 0)
				return printSocketReadError();
			if (receiveData(file_des, &arg[iloop].xmax, sizeof(int), INT32) < 0)
				return printSocketReadError();
			if (receiveData(file_des, &arg[iloop].ymin, sizeof(int), INT32) < 0)
				return printSocketReadError();
			if (receiveData(file_des, &arg[iloop].ymax, sizeof(int), INT32) < 0)
				return printSocketReadError();
		}
	}
	FILE_LOG(logDEBUG1, ("Set ROI (narg:%d)\n", narg));
	{
		int iloop = 0;
		for (iloop = 0; iloop < narg; ++iloop) {
			FILE_LOG(logDEBUG1, ("%d: %d\t%d\t%d\t%d\n",
					arg[iloop].xmin, arg[iloop].xmax, arg[iloop].ymin, arg[iloop].ymax));
		}
	}

#if defined(JUNGFRAUD) || defined(EIGERD)
	functionNotImplemented();
#else
	// set & get
	if ((narg == GET_READOUT_FLAGS) || (Server_VerifyLock() == OK)) {
	    if (myDetectorType == GOTTHARD && narg > 1) {
	        ret = FAIL;
            strcpy(mess,"Can not set more than one ROI per module.\n");
            FILE_LOG(logERROR,(mess));
	    } else {
	        retval = setROI(narg, arg, &nretval, &ret);
	        if (ret == FAIL) {
	            if (nretval == -1) // chip test board
	                sprintf(mess,"Could not set ROI. Max ROI level (100) reached!\n");
	            else if (nretval == -2)
	                sprintf(mess, "Could not set ROI. Could not allocate RAM\n");
	            else
	                sprintf(mess,"Could not set all roi. "
	                        "Set %d rois, but read %d rois\n", narg, nretval);
	            FILE_LOG(logERROR,(mess));
	        }
	        FILE_LOG(logDEBUG1, ("nRois: %d\n", nretval));
	    }
	}
#endif

	Server_SendResult(file_des, INT32, UPDATE, NULL, 0);

	if (ret != FAIL) {
		//retvalsize could be swapped during sendData
		int nretval1 = nretval;
		sendData(file_des, &nretval1, sizeof(nretval1), INT32);
		int iloop = 0;
		for(iloop = 0; iloop < nretval; ++iloop) {
			sendData(file_des, &retval[iloop].xmin, sizeof(int), INT32);
			sendData(file_des, &retval[iloop].xmax, sizeof(int), INT32);
			sendData(file_des, &retval[iloop].ymin, sizeof(int), INT32);
			sendData(file_des, &retval[iloop].ymax, sizeof(int), INT32);
		}
	}
	return ret;
}





int set_speed(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[3] = {-1, -1, -1};
	int retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();

	enum speedVariable ind = args[0];
	int val = args[1];
	int mode = args[2];

    char speedName[20] = {0};
    strcpy(speedName, getSpeedName(ind));
    FILE_LOG(logDEBUG1, ("Setting speed index %s (speedVariable %d) to %d (mode: %d)\n", speedName, ind, val, mode));

    // check index
    switch(ind) {
#ifdef JUNGFRAUD
        case ADC_PHASE:
        case CLOCK_DIVIDER:
        case MAX_ADC_PHASE_SHIFT:
#elif CHIPTESTBOARDD
        case ADC_PHASE:
        case DBIT_PHASE:
        case MAX_ADC_PHASE_SHIFT:
        case MAX_DBIT_PHASE_SHIFT:
        case ADC_CLOCK:
        case DBIT_CLOCK:
        case CLOCK_DIVIDER:
        case ADC_PIPELINE:
        case DBIT_PIPELINE:
#elif MOENCHD
        case ADC_PHASE:
        case DBIT_PHASE:
        case MAX_ADC_PHASE_SHIFT:
        case MAX_DBIT_PHASE_SHIFT:
        case ADC_CLOCK:
        case DBIT_CLOCK:
        case CLOCK_DIVIDER:
        case ADC_PIPELINE:
        case DBIT_PIPELINE:
#elif GOTTHARDD
        case ADC_PHASE:
#elif EIGERD
        case CLOCK_DIVIDER:
#endif
            break;
        default:
            modeNotImplemented(speedName, (int)ind);
            break;
    }
#if (!defined(CHIPTESTBOARDD)) && (!defined(MOENCHD)) && (!defined(JUNGFRAUD))
    if (ret == OK && mode == 1) {
		ret = FAIL;
		strcpy(mess, "deg is not defined for this detector.\n");
		FILE_LOG(logERROR,(mess));
    }
#endif

    if (ret == OK) {
    	// set
    	if ((val != -1) && (Server_VerifyLock() == OK)) {
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(JUNGFRAUD)
    		setSpeed(ind, val, mode);
#else
    		setSpeed(ind, val);
#endif
    	}
    	// get
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(JUNGFRAUD)
    	retval = getSpeed(ind, mode);
#else
    	retval = getSpeed(ind);
#endif
    	FILE_LOG(logDEBUG1, ("%s: %d (mode:%d)\n", speedName, retval, mode));
    	// validate
    	char validateName[20] = {0};
    	sprintf(validateName, "set %s", speedName);
#ifndef GOTTHARDD
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(JUNGFRAUD)
    	if ((ind == ADC_PHASE || ind == DBIT_PHASE) && mode == 1) {
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
    		ret = validatePhaseinDegrees(ind, val, retval);
#else
    		ret = validatePhaseinDegrees(val, retval);
#endif
    		if (ret == FAIL) {
    			sprintf(mess, "Could not set %s. Set %d, got %d\n", validateName, val, retval);
    			FILE_LOG(logERROR,(mess));
    		}
    	} else
    		validate(val, retval, validateName, DEC);
#else
    	validate(val, retval, validateName, DEC);
#endif
#endif
    }

	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int exit_server(int file_des) {
	FILE_LOG(logINFORED, ("Closing Server\n"));
	ret = OK;
	memset(mess, 0, sizeof(mess));
	Server_SendResult(file_des, INT32, NO_UPDATE, NULL, 0);
	return GOODBYE;
}




int lock_server(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int lock = 0;

	if (receiveData(file_des, &lock, sizeof(lock), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Locking Server to %d\n", lock));

	// set
	if (lock >= 0) {
		if (!lockStatus || // if it was unlocked, anyone can lock
				(!strcmp(lastClientIP, thisClientIP)) || // if it was locked, need same ip
				(!strcmp(lastClientIP,"none"))) { // if it was locked, must be by "none"
			lockStatus = lock;
			if (lock) {
				FILE_LOG(logINFO, ("Server lock to %s\n", lastClientIP));
			} else {
				FILE_LOG(logINFO, ("Server unlocked\n"));
			}
			strcpy(lastClientIP, thisClientIP);
		}   else {
			Server_LockedError();
		}
	}
	int retval = lockStatus;
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int get_last_client_ip(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	return Server_SendResult(file_des, OTHER, UPDATE, lastClientIP, sizeof(lastClientIP));
}




int set_port(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int p_number = -1;
	char oldLastClientIP[INET_ADDRSTRLEN] = {0};

	if (receiveData(file_des, &p_number, sizeof(p_number), INT32) < 0)
		return printSocketReadError();

	// set only
	int sd = -1;
	if ((Server_VerifyLock() == OK)) {
		// port number too low
		 if (p_number < 1024) {
			ret = FAIL;
			sprintf(mess,"%s port Number (%d) too low\n",
					(isControlServer ? "control":"stop"), p_number);
			FILE_LOG(logERROR,(mess));
		} else {
			FILE_LOG(logINFO, ("Setting %s port to %d\n",
					(isControlServer ? "control":"stop"), p_number));
			strcpy(oldLastClientIP, lastClientIP);
			sd = bindSocket(p_number);
		}
	}

	Server_SendResult(file_des, INT32, UPDATE, &p_number, sizeof(p_number));
	// delete old socket
	if (ret != FAIL) {
		closeConnection(file_des);
		exitServer(sockfd);
		sockfd = sd;
		strcpy(lastClientIP, oldLastClientIP);
	}
	return ret;
}




int update_client(int file_des) {
	ret = FORCE_UPDATE;
	memset(mess, 0, sizeof(mess));
	Server_SendResult(file_des, INT32, NO_UPDATE, NULL, 0);
	return send_update(file_des);
}




int send_update(int file_des) {
    ret = OK;
	int n = 0;
	int i32 = -1;
	int64_t i64 = -1;

	n = sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
	if (n < 0) return printSocketReadError();

	// dr
	i32 = setDynamicRange(GET_FLAG);
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();

	// databytes
	i32 = calculateDataBytes();
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();

	// settings
#if defined(EIGERD) || defined(JUNGFRAUD) || defined(GOTTHARDD)
	i32 = (int)getSettings();
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();
#endif

	// threshold energy
#ifdef EIGERD
	i32 = getThresholdEnergy(GET_FLAG);
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();
#endif

	// #frames
	i64 = setTimer(FRAME_NUMBER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	// exptime
	i64 = setTimer(ACQUISITION_TIME,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	// subexptime, subdeadtime
#ifdef EIGERD
	i64 = setTimer(SUBFRAME_ACQUISITION_TIME,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	i64 = setTimer(SUBFRAME_DEADTIME,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();
#endif

	// period
	i64 = setTimer(FRAME_PERIOD,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	// delay
#ifndef EIGERD
	i64 = setTimer(DELAY_AFTER_TRIGGER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();
#endif

	// #storage cell, storage_cell_delay
#ifdef JUNGFRAUD
	i64 = setTimer(STORAGE_CELL_NUMBER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	i64 = setTimer(STORAGE_CELL_DELAY,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();
#endif

	// #cycles
	i64 = setTimer(CYCLES_NUMBER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	// readout flags
#if defined(EIGERD) || defined(CHIPTESTBOARDD)
    i32 = setReadOutFlags(GET_READOUT_FLAGS);
    n = sendData(file_des,&i32,sizeof(i32),INT32);
    if (n < 0) return printSocketReadError();
#endif

    // #samples, roi
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(GOTTHARDD)
    i64 = setTimer(SAMPLES,GET_FLAG);
    n = sendData(file_des,&i64,sizeof(i64),INT64);
    if (n < 0) return printSocketReadError();

    // roi
    ROI* retval = NULL;
    ROI arg[1];
    int ret = OK, nretval = 0;
    retval = setROI(-1, arg, &nretval, &ret);
	//retvalsize could be swapped during sendData
	int nretval1 = nretval;
	sendData(file_des, &nretval1, sizeof(nretval1), INT32);
	int iloop = 0;
	for(iloop = 0; iloop < nretval; ++iloop) {
		sendData(file_des, &retval[iloop].xmin, sizeof(int), INT32);
		sendData(file_des, &retval[iloop].xmax, sizeof(int), INT32);
		sendData(file_des, &retval[iloop].ymin, sizeof(int), INT32);
		sendData(file_des, &retval[iloop].ymax, sizeof(int), INT32);
	}


#endif

	if (lockStatus == 0) {
		strcpy(lastClientIP,thisClientIP);
	}

	return ret;
}






int configure_mac(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
    const size_t array_size = 50;
    const size_t n_args = 14;
    const size_t n_retvals = 2;
    char args[n_args][array_size];
    char retvals[n_retvals][array_size];

	memset(args, 0, sizeof(args));
	memset(retvals, 0, sizeof(retvals));

	if (receiveData(file_des, args, sizeof(args), OTHER) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("\n Configuring MAC\n"));

	// dest port
	uint32_t dstPort = 0;
	sscanf(args[0], "%x", 	&dstPort);
	FILE_LOG(logDEBUG1, ("Dst Port: %x\n", dstPort));
	// dest ip
	uint32_t dstIp = 0;
	sscanf(args[1], "%x", 	&dstIp);
	FILE_LOG(logDEBUG1, ("Dst Ip Addr: %d.%d.%d.%d = 0x%x \n",
			(dstIp >> 24) & 0xff, (dstIp >> 16) & 0xff, (dstIp >> 8) & 0xff, (dstIp) & 0xff,
			dstIp));
	// dest mac
	uint64_t dstMac = 0;
#ifdef VIRTUAL
	sscanf(args[2], "%lx", 	&dstMac);
#else
	sscanf(args[2], "%llx", 	&dstMac);
#endif
	FILE_LOG(logDEBUG1, ("Dst Mac Addr: (0x) "));
	{
		int iloop = 5;
		for (iloop = 5; iloop >= 0; --iloop) {
			printf ("%x", (unsigned int)(((dstMac >> (8 * iloop)) & 0xFF)));
			if (iloop > 0) {
				printf(":");
			}
		}
	}
	FILE_LOG(logDEBUG1, (" = %llx\n", dstMac));
	// source ip
	uint32_t srcIp = 0;
	sscanf(args[3], "%x",	&srcIp);
	FILE_LOG(logDEBUG1, ("Src Ip Addr: %d.%d.%d.%d = 0x%x \n",
			(srcIp >> 24) & 0xff, (srcIp >> 16) & 0xff, (srcIp >> 8) & 0xff, (srcIp) & 0xff,
			srcIp));
	// source mac
	uint64_t srcMac = 0;
#ifdef VIRTUAL
	sscanf(args[4], "%lx",	&srcMac);
#else
	sscanf(args[4], "%llx",	&srcMac);
#endif
	FILE_LOG(logDEBUG1, ("Src Mac Addr: (0x) "));
	{
		int iloop = 5;
		for (iloop = 5; iloop >= 0; --iloop) {
			printf("%x", (unsigned int)(((srcMac >> (8 * iloop)) & 0xFF)));
			if (iloop > 0) {
				printf(":");
			}
		}
	}
	FILE_LOG(logDEBUG1, (" = %llx\n", srcMac));

#if defined(JUNGFRAUD) || defined(EIGERD)
	// source port 2
	uint32_t dstPort2 = 0;
	sscanf(args[5], "%x", 	&dstPort2);
	FILE_LOG(logDEBUG1, ("Dst Port2: %x\n", dstPort2));
#endif
#ifdef JUNGFRAUD
	// dest ip2
	uint32_t dstIp2 = 0;
	sscanf(args[6], "%x", 	&dstIp2);
	FILE_LOG(logDEBUG1, ("Dst Ip Addr: %d.%d.%d.%d = 0x%x \n",
			(dstIp2 >> 24) & 0xff, (dstIp2 >> 16) & 0xff, (dstIp2 >> 8) & 0xff, (dstIp2) & 0xff,
			dstIp2));
	// dest mac2
	uint64_t dstMac2 = 0;
#ifdef VIRTUAL
	sscanf(args[7], "%lx", 	&dstMac2);
#else
	sscanf(args[7], "%llx", 	&dstMac2);
#endif
	FILE_LOG(logDEBUG1, ("Dst Mac Addr: (0x) "));
	{
		int iloop = 5;
		for (iloop = 5; iloop >= 0; --iloop) {
			printf ("%x", (unsigned int)(((dstMac2 >> (8 * iloop)) & 0xFF)));
			if (iloop > 0) {
				printf(":");
			}
		}
	}
	FILE_LOG(logDEBUG1, (" = %llx\n", dstMac2));
	// source ip2
	uint32_t srcIp2 = 0;
	sscanf(args[8], "%x",	&srcIp2);
	FILE_LOG(logDEBUG1, ("Src Ip Addr: %d.%d.%d.%d = 0x%x \n",
			(srcIp2 >> 24) & 0xff, (srcIp2 >> 16) & 0xff, (srcIp2 >> 8) & 0xff, (srcIp2) & 0xff,
			srcIp2));
	// source mac2
	uint64_t srcMac2 = 0;
#ifdef VIRTUAL
	sscanf(args[9], "%lx",	&srcMac2);
#else
	sscanf(args[9], "%llx",	&srcMac2);
#endif
	FILE_LOG(logDEBUG1, ("Src Mac Addr: (0x) "));
	{
		int iloop = 5;
		for (iloop = 5; iloop >= 0; --iloop) {
			printf("%x", (unsigned int)(((srcMac2 >> (8 * iloop)) & 0xFF)));
			if (iloop > 0) {
				printf(":");
			}
		}
	}
	FILE_LOG(logDEBUG1, (" = %llx\n", srcMac2));

	// number of interfaces
	int numInterfaces = 0;
	sscanf(args[10], "%d",	&numInterfaces);
	int selInterface = 1;
	sscanf(args[11], "%d",	&selInterface);

#endif
#if defined(JUNGFRAUD) || defined(EIGERD)
	int pos[2] = {0, 0};
	sscanf(args[12], "%x", 	&pos[0]);
	sscanf(args[13], "%x", 	&pos[1]);
	FILE_LOG(logDEBUG1, ("Position: [%d, %d]\n", pos[0], pos[1]));
#endif


	// set only
	if ((Server_VerifyLock() == OK)) {

		// stop detector if it was running
		enum runStatus status = getRunStatus();
		if (status != IDLE && status != RUN_FINISHED && status != STOPPED) {
			if (status == RUNNING)
				stopStateMachine();
#ifndef EIGERD
		    cleanFifos();
#endif
		    status = getRunStatus();
		    if (status != IDLE && status != RUN_FINISHED && status != STOPPED) {
		    	ret = FAIL;
		    	sprintf(mess, "Cannot configure mac when detector is not idle. Detector at %s state\n", getRunStateName(status));
		    	FILE_LOG(logERROR,(mess));
		    }
		}

		if (ret == OK) {
#ifdef EIGERD
			// change mac to hardware mac
			if (srcMac != getDetectorMAC()) {
				FILE_LOG(logWARNING, ("actual detector mac address %llx does not match "
						"the one from client %llx\n",
						(long long unsigned int)getDetectorMAC(),
						(long long unsigned int)srcMac));
				srcMac = getDetectorMAC();
				FILE_LOG(logWARNING,("matched detectormac to the hardware mac now\n"));
			}

			// always remember the ip sent from the client (could be for 10g(if not dhcp))
			if (srcIp != getDetectorIP())
				dhcpipad = srcIp;

			//only for 1Gbe, change ip to hardware ip
			if (!enableTenGigabitEthernet(-1)) {
				FILE_LOG(logWARNING, ("Using DHCP IP for Configuring MAC\n"));
				srcIp = getDetectorIP();
			}
			// 10 gbe (use ip given from client)
			else
				srcIp = dhcpipad;
			ret = configureMAC(dstIp, dstMac, srcMac, srcIp, dstPort, dstPort2);
#elif JUNGFRAUD
			ret = configureMAC(numInterfaces, selInterface, dstIp, dstMac, srcMac, srcIp, dstPort, dstIp2, dstMac2, srcMac2, srcIp2, dstPort2);
#else
			ret = configureMAC(dstIp, dstMac, srcMac, srcIp, dstPort);
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
            if (ret != OK) {
            	if (ret == FAIL)
            		sprintf(mess,"Could not configure mac because of incorrect udp 1G destination IP and port\n");
            	else if (ret == -1)
            		sprintf(mess, "Could not allocate RAM\n");
            	FILE_LOG(logERROR,(mess));
            }
#else
			if (ret == FAIL) {
				sprintf(mess,"Configure Mac failed\n");
				FILE_LOG(logERROR,(mess));
			}
#endif
			else {
			    FILE_LOG(logINFO, ("\tConfigure MAC successful\n"));
			}
#if defined(EIGERD) || defined (JUNGFRAUD)
			if (ret != FAIL) {
				ret = setDetectorPosition(pos);
				if (ret == FAIL) {
					sprintf(mess, "Could not set detector position\n");
					FILE_LOG(logERROR,(mess));
				}
			}
#endif
			// set retval vals
			if (ret != FAIL) {
				sprintf(retvals[0],"%llx", (long long unsigned int)srcMac);
				sprintf(retvals[1],"%x", srcIp);
			}
		}
	}

	return Server_SendResult(file_des, OTHER, UPDATE, retvals, sizeof(retvals));
}





int load_image(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1, -1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();

	enum imageType index = args[0];
	int numChannels = args[1];
	short int imageVals[numChannels];
	memset(imageVals, 0, numChannels * sizeof(short int));
	if (numChannels > 0) {
		if (receiveData(file_des, imageVals, numChannels * sizeof(short int), OTHER) < 0) {
			return printSocketReadError();
		}
	}
	FILE_LOG(logDEBUG1, ("Loading %s image (ind:%d)\n", (index == DARK_IMAGE) ? "dark" :
			((index == GAIN_IMAGE) ? "gain" : "unknown"), index));

#ifndef GOTTHARDD
	functionNotImplemented();
#else

	// set only
	if (Server_VerifyLock() == OK) {
		switch (index) {
		case DARK_IMAGE :
		case GAIN_IMAGE :
		    // size of image does not match expected size
		    if (numChannels != (calculateDataBytes()/sizeof(short int))) {
		        ret = FAIL;
                sprintf(mess, "Could not load image. "
                        "Number of Channels do not match. Expected %d, got %d\n",
                        calculateDataBytes(), numChannels);
                FILE_LOG(logERROR,(mess));
		    } else
		        loadImage(index, imageVals);
			break;
		default:
			modeNotImplemented("Image index", (int)index);
			break;
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}






int read_counter_block(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1, -1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	int startACQ = args[0];
    int numChannels = args[1];
    short int retval[numChannels];
    memset(retval, 0, numChannels * sizeof(short int));
	FILE_LOG(logDEBUG1, ("Read counter block with start acq bit: %d\n", startACQ));

#ifndef GOTTHARDD
	functionNotImplemented();
#else

	// only set
	if (Server_VerifyLock() == OK) {
	    // size of image does not match expected size
	    if (numChannels != (calculateDataBytes()/sizeof(short int))) {
	        ret = FAIL;
	        sprintf(mess, "Could not load image. "
	                "Number of Channels do not match. Expected %d, got %d\n",
	                calculateDataBytes(), numChannels);
	        FILE_LOG(logERROR,(mess));
	    } else {
	        ret = readCounterBlock(startACQ, retval);
	        if (ret == FAIL) {
	            strcpy(mess, "Could not read counter block\n");
	            FILE_LOG(logERROR,(mess));
	        }
	    }
	}
#endif
	return Server_SendResult(file_des, OTHER, UPDATE, retval, numChannels * sizeof(short int));
}





int reset_counter_block(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int startACQ = -1;

	if (receiveData(file_des, &startACQ, sizeof(startACQ), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Reset counter block with start acq bit: %d\n", startACQ));

#ifndef GOTTHARDD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = resetCounterBlock(startACQ);
		if (ret == FAIL) {
			strcpy(mess, "Could not reset counter block\n");
			FILE_LOG(logERROR, (mess));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}





int enable_ten_giga(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Enable/ Disable 10GbE : %d\n", arg));

#if defined(JUNGFRAUD) || defined(GOTTHARDD)
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = enableTenGigabitEthernet(arg);
		FILE_LOG(logDEBUG1, ("10GbE: %d\n", retval));
		validate(arg, retval, "enable/disable 10GbE", DEC);
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int set_all_trimbits(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Set all trmbits to %d\n", arg));

#ifndef EIGERD
	functionNotImplemented();
#else

	// set
	if (arg >= 0 && Server_VerifyLock() == OK) {
		ret = setAllTrimbits(arg);
		//changes settings to undefined
		setSettings(UNDEFINED);
		FILE_LOG(logERROR, ("Settings has been changed to undefined (change all trimbits)\n"));
	}
	// get
	retval = getAllTrimbits();
	FILE_LOG(logDEBUG1, ("All trimbits: %d\n", retval));
	validate(arg, retval, "set all trimbits", DEC);
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int set_pattern_io_control(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
	uint64_t arg = -1;
	uint64_t retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
		return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
    functionNotImplemented();
#else
	FILE_LOG(logDEBUG1, ("Setting Pattern IO Control to 0x%llx\n", (long long int)arg));
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = writePatternIOControl(arg);
		FILE_LOG(logDEBUG1, ("Pattern IO Control retval: 0x%llx\n", (long long int) retval));
		validate64(arg, retval, "Pattern IO Control", HEX);
	}
#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
} 






int set_pattern_clock_control(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
	uint64_t arg = -1;
	uint64_t retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
		return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
    functionNotImplemented();
#else
	FILE_LOG(logDEBUG1, ("Setting Pattern Clock Control to 0x%llx\n", (long long int)arg));
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = writePatternClkControl(arg);
		FILE_LOG(logDEBUG1, ("Pattern Clock Control retval: 0x%llx\n", (long long int) retval));
		validate64(arg, retval, "Pattern Clock Control", HEX);
	}
#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}






int set_pattern_word(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t args[2] = {-1, -1};
    uint64_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT64) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
    functionNotImplemented();
#else
	int addr = (int)args[0];
	uint64_t word = args[1];
	FILE_LOG(logDEBUG1, ("Setting Pattern Word (addr:0x%x, word:0x%llx\n", addr, (long long int)word));
	if (Server_VerifyLock() == OK) {
		// valid address
		if (addr > MAX_PATTERN_LENGTH) {
			ret = FAIL;
			sprintf(mess, "Cannot set Pattern (Word, addr:0x%x). Addr must be <= 0x%x\n",
					addr, MAX_PATTERN_LENGTH);
			FILE_LOG(logERROR, (mess));
        } else {
			retval = writePatternWord(addr, word);
			FILE_LOG(logDEBUG1, ("Pattern Word retval: 0x%llx\n", (long long int) retval));
			// no validation (cannot read as it will execute the pattern)
		}
	}
#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}






int set_pattern_loop(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[4] = {-1, -1, -1, -1};
    int retvals[3] = {-1, -1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    int loopLevel = args[0];
	int startAddr = args[1];
	int stopAddr = args[2];
	int numLoops = args[3];
	FILE_LOG(logDEBUG1, ("Setting Pattern loops (loopLevel:%d startAddr:0x%x stopAddr:0x%x numLoops:%d)\n", loopLevel, startAddr, stopAddr, numLoops));
	if ((startAddr == -1) || (stopAddr == -1) || (numLoops == -1) || (Server_VerifyLock() == OK)) {
		// valid loop level
		 if (loopLevel < -1 || loopLevel > 2) { // loop level of -1 : complete pattern
			ret = FAIL;
			sprintf(mess, "Cannot set Pattern loops. Level %d should be between -1 and 2\n",loopLevel);
			FILE_LOG(logERROR, (mess));
        } 
		// valid addr for loop level 0-2
		else if (startAddr > MAX_PATTERN_LENGTH  || stopAddr > MAX_PATTERN_LENGTH ) {
            ret = FAIL;
            sprintf(mess, "Cannot set Pattern loops. Address (start addr:0x%x and stop addr:0x%x) "
			"should be less than 0x%x\n", startAddr, stopAddr, MAX_PATTERN_LENGTH);
			FILE_LOG(logERROR, (mess));
        }
		
		else {
			setPatternLoop(loopLevel, &startAddr, &stopAddr, &numLoops);
			FILE_LOG(logDEBUG1, ("Pattern Loops retval: (start:0x%x, stop:0x%x, nloop:%d)\n", startAddr, stopAddr, numLoops));
			retvals[0] = startAddr;
			retvals[1] = stopAddr;
			retvals[2] = numLoops;
			validate(args[1], startAddr, "Pattern loops' start address", HEX);
			validate(args[2], stopAddr, "Pattern loops' stop address", HEX);
			validate(args[3], numLoops, "Pattern loops' number of loops", DEC);
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, retvals, sizeof(retvals));
}






int set_pattern_wait_addr(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = { -1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    int loopLevel = args[0];
	int addr = args[1];
	FILE_LOG(logDEBUG1, ("Setting Pattern wait address (loopLevel:%d addr:0x%x)\n", loopLevel, addr));
	if ((addr == -1) || (Server_VerifyLock() == OK)) {
		// valid loop level 0-2
		 if (loopLevel < 0 || loopLevel > 2) { 
			ret = FAIL;
			sprintf(mess, "Cannot set Pattern wait address. Level %d should be between 0 and 2\n",loopLevel);
			FILE_LOG(logERROR, (mess));
        } 
		// valid addr 
		else if (addr > MAX_PATTERN_LENGTH) {
            ret = FAIL;
            sprintf(mess, "Cannot set Pattern wait address. Address (0x%x) should be less than 0x%x\n", addr, MAX_PATTERN_LENGTH);
			FILE_LOG(logERROR, (mess));
        }
		else {
			retval = setPatternWaitAddress(loopLevel, addr);
			FILE_LOG(logDEBUG1, ("Pattern wait address retval: 0x%x\n", retval));
			validate(addr, retval, "Pattern wait address", HEX);
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int set_pattern_wait_time(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t args[2] = { -1, -1};
    uint64_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    int loopLevel = (int)args[0];
	uint64_t timeval = args[1];
	FILE_LOG(logDEBUG1, ("Setting Pattern wait time (loopLevel:%d timeval:0x%llx)\n", loopLevel, (long long int)timeval));
	if ((timeval == -1) || (Server_VerifyLock() == OK)) {
		// valid loop level 0-2
		 if (loopLevel < 0 || loopLevel > 2) { 
			ret = FAIL;
			sprintf(mess, "Cannot set Pattern wait time. Level %d should be between 0 and 2\n",loopLevel);
			FILE_LOG(logERROR, (mess));
        } 
		else {
			retval = setPatternWaitTime(loopLevel, timeval);
			FILE_LOG(logDEBUG1, ("Pattern wait time retval: 0x%llx\n", (long long int)retval));
			validate64(timeval, retval, "Pattern wait time", HEX);
		}
	}
#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}




int set_pattern_mask(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint64_t arg = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Set Pattern Mask to %d\n", arg));

#if (!defined(MOENCHD)) && (!defined(CHIPTESTBOARDD))
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		setPatternMask(arg);
		uint64_t retval64 = getPatternMask();
		FILE_LOG(logDEBUG1, ("Pattern mask: 0x%llx\n", (long long unsigned int) retval64));
		validate64(arg, retval64, "Set Pattern Mask", HEX);
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}

int get_pattern_mask(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint64_t retval64 = -1;

	FILE_LOG(logDEBUG1, ("Get Pattern Mask\n"));

#if (!defined(MOENCHD)) && (!defined(CHIPTESTBOARDD))
	functionNotImplemented();
#else
	// only get
	retval64 = getPatternMask();
	FILE_LOG(logDEBUG1, ("Get Pattern mask: 0x%llx\n", (long long unsigned int) retval64));

#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval64, sizeof(retval64));
}

int set_pattern_bit_mask(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint64_t arg = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Set Pattern Bit Mask to %d\n", arg));

#if (!defined(MOENCHD)) && (!defined(CHIPTESTBOARDD))
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		setPatternBitMask(arg);
		uint64_t retval64 = getPatternBitMask();
		FILE_LOG(logDEBUG1, ("Pattern bit mask: 0x%llx\n", (long long unsigned int) retval64));
		validate64(arg, retval64, "Set Pattern Bit Mask", HEX);
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}

int get_pattern_bit_mask(int file_des){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint64_t retval64 = -1;

	FILE_LOG(logDEBUG1, ("Get Pattern Bit Mask\n"));

#if (!defined(MOENCHD)) && (!defined(CHIPTESTBOARDD))
	functionNotImplemented();
#else
	// only get
	retval64 = getPatternBitMask();
	FILE_LOG(logDEBUG1, ("Get Pattern Bitmask: 0x%llx\n", (long long unsigned int) retval64));

#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval64, sizeof(retval64));
}

int write_adc_register(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint32_t args[2] = {-1, -1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	uint32_t addr = args[0];
	uint32_t val = args[1];
	FILE_LOG(logDEBUG1, ("Writing 0x%x to ADC Register 0x%x\n", val, addr));

#ifdef EIGERD
	functionNotImplemented();
#else
#ifndef VIRTUAL
	// only set
	if (Server_VerifyLock() == OK) {
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
		AD9257_Set(addr, val);
#elif GOTTHARDD
	if (getBoardRevision() == 1) {
	    AD9252_Set(addr, val);
	} else {
	    AD9257_Set(addr, val);
	}
#endif
	}
#endif
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}




int set_counter_bit(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Set counter bit with value: %d\n", arg));

#ifndef EIGERD
	functionNotImplemented();
#else

	// set
	if (arg >= 0 && Server_VerifyLock() == OK) {
		setCounterBit(arg);
	}
	// get
	retval = setCounterBit(-1);
	FILE_LOG(logDEBUG1, ("Set counter bit retval: %d\n", retval));
	validate(arg, retval, "set counter bit", DEC);
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int pulse_pixel(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[3] = {-1,-1,-1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Pulse pixel, n: %d, x: %d, y: %d\n", args[0], args[1], args[2]));

#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = pulsePixel(args[0], args[1], args[2]);
		if (ret == FAIL) {
			strcpy(mess, "Could not pulse pixel\n");
			FILE_LOG(logERROR,(mess));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}




int pulse_pixel_and_move(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[3] = {-1,-1,-1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Pulse pixel and move, n: %d, x: %d, y: %d\n",
			args[0], args[1], args[2]));

#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = pulsePixelNMove(args[0], args[1], args[2]);
		if (ret == FAIL) {
			strcpy(mess, "Could not pulse pixel and move\n");
			FILE_LOG(logERROR,(mess));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}






int pulse_chip(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Pulse chip: %d\n", arg));

#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = pulseChip(arg);
		if (ret == FAIL) {
			strcpy(mess, "Could not pulse chip\n");
			FILE_LOG(logERROR,(mess));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}





int set_rate_correct(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t tau_ns = -1;

	if (receiveData(file_des, &tau_ns, sizeof(tau_ns), INT64) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Set rate correct with tau %lld\n", (long long int)tau_ns));

#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {

		int dr = setDynamicRange(-1);

		// switching on in wrong bit mode
		if ((tau_ns != 0) && (dr != 32) && (dr != 16)) {
			ret = FAIL;
			strcpy(mess,"Rate correction Deactivated, must be in 32 or 16 bit mode\n");
			FILE_LOG(logERROR,(mess));
		}

		// switching on in right mode
		else {
			if (tau_ns < 0)
				tau_ns = getDefaultSettingsTau_in_nsec();
			else if (tau_ns > 0) {
				//changing tau to a user defined value changes settings to undefined
				setSettings(UNDEFINED);
				FILE_LOG(logERROR, ("Settings has been changed to undefined (tau changed)\n"));
			}
			int64_t retval = setRateCorrection(tau_ns);
			validate64(tau_ns, retval, "set rate correction", DEC);
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}





int get_rate_correct(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t retval = -1;

	FILE_LOG(logDEBUG1, ("Getting rate correction\n"));
#ifndef EIGERD
	functionNotImplemented();
#else
	retval = getCurrentTau();
	FILE_LOG(logDEBUG1, ("Tau: %lld\n", (long long int)retval));
#endif
	return Server_SendResult(file_des, INT64, UPDATE, &retval, sizeof(retval));
}





int set_network_parameter(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1,-1};
	int retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	enum networkParameter mode = args[0];
	int value = args[1];
	FILE_LOG(logDEBUG1, ("Set network parameter index %d to %d\n", mode, value));

#if defined(GOTTHARDD) || defined (CHIPTESTBOARDD) || defined(MOENCHD)
	functionNotImplemented();
#else
    enum NETWORKINDEX serverIndex = 0;

	// set & get
	if ((value == -1) || (Server_VerifyLock() == OK)) {
		// check index
		switch (mode) {
#ifdef EIGERD
        case FLOW_CONTROL_10G:
        	serverIndex = FLOWCTRL_10G;
            break;
		case DETECTOR_TXN_DELAY_LEFT:
			serverIndex = TXN_LEFT;
			break;
		case DETECTOR_TXN_DELAY_RIGHT:
			serverIndex = TXN_RIGHT;
			break;
#endif
		case DETECTOR_TXN_DELAY_FRAME:
			serverIndex = TXN_FRAME;
#ifdef JUNGFRAUD
			if (value > MAX_TIMESLOT_VAL)	{
			    ret = FAIL;
			    sprintf(mess,"Transmission delay %d should be in range: 0 - %d\n",
			    		value, MAX_TIMESLOT_VAL);
			    FILE_LOG(logERROR, (mess));
			}
#endif
			break;
		default:
			modeNotImplemented("Image index", (int)serverIndex);
			break;
		}
		// valid index
		if (ret == OK) {
			retval = setNetworkParameter(serverIndex, value);
			FILE_LOG(logDEBUG1, ("Network Parameter index %d: %d\n", serverIndex, retval));
			validate(value, retval, "set network parameter", DEC);
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}






int program_fpga(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

#if defined(EIGERD) || defined(GOTTHARDD)
	//to receive any arguments
	int n = 1;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	functionNotImplemented();
#else
#ifndef VIRTUAL
	// only set
	if (Server_VerifyLock() == OK) {

		FILE_LOG(logINFOBLUE, ("Programming FPGA...\n"));

		size_t filesize = 0;
		size_t totalsize = 0;
		size_t unitprogramsize = 0;
		char* fpgasrc = NULL;
		FILE* fp = NULL;

		// filesize
		if (receiveData(file_des,&filesize,sizeof(filesize),INT32) < 0)
			return printSocketReadError();
		totalsize = filesize;
		FILE_LOG(logDEBUG1, ("Total program size is: %d\n", totalsize));


		// opening file pointer to flash and telling FPGA to not touch flash
		if (startWritingFPGAprogram(&fp) != OK) {
			ret = FAIL;
			sprintf(mess,"Could not write to flash. Error at startup.\n");
			FILE_LOG(logERROR,(mess));
		}
		Server_SendResult(file_des, INT32, NO_UPDATE, NULL, 0);


		//erasing flash
		if (ret != FAIL) {
			eraseFlash();
			fpgasrc = (char*)malloc(MAX_FPGAPROGRAMSIZE);
		}


		//writing to flash part by part
		while(ret != FAIL && filesize) {

			unitprogramsize = MAX_FPGAPROGRAMSIZE;  //2mb
			if (unitprogramsize > filesize) //less than 2mb
				unitprogramsize = filesize;
			FILE_LOG(logDEBUG1, ("unit size to receive is:%d\nfilesize:%d\n", unitprogramsize, filesize));

			//receive part of program
			if (receiveData(file_des,fpgasrc,unitprogramsize,OTHER) < 0)
				return printSocketReadError();

			if (!(unitprogramsize - filesize)) {
				fpgasrc[unitprogramsize] = '\0';
				filesize -= unitprogramsize;
				unitprogramsize++;
			} else
				filesize -= unitprogramsize;

			// write part to flash
			ret = writeFPGAProgram(fpgasrc, unitprogramsize, fp);
			Server_SendResult(file_des, INT32, NO_UPDATE, NULL, 0);
			if (ret == FAIL) {
				FILE_LOG(logERROR, ("Failure: Breaking out of program receiving\n"));
			} else {
				//print progress
				FILE_LOG(logINFO, ("Writing to Flash:%d%%\r",
						(int) (((double)(totalsize-filesize)/totalsize)*100) ));
				fflush(stdout);
			}
		}
		printf("\n");
		if (ret == OK) {
			FILE_LOG(logINFO, ("Done copying program\n"));
		}

		// closing file pointer to flash and informing FPGA
		stopWritingFPGAprogram(fp);

		//free resources
		if (fpgasrc != NULL)
			free(fpgasrc);
		if (fp != NULL)
			fclose(fp);

		FILE_LOG(logINFO, ("Completed program fpga command with %s\n", (ret == OK ? "success" : "fail")));
	}
#endif
#endif
	return ret;
}





int reset_fpga(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Reset FPGA\n"));
#if defined(EIGERD) || defined(GOTTHARDD)
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		if (isControlServer) {
			basictests();	// mapping of control server at least
			initControlServer();
		}
		else initStopServer(); //remapping of stop server
		ret = FORCE_UPDATE;
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}



int power_chip(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Powering chip to %d\n", arg));

#if (!defined(JUNGFRAUD)) && (!defined(MOENCHD))
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = powerChip(arg);
		FILE_LOG(logDEBUG1, ("Power chip: %d\n", retval));
		validate(arg, retval, "power on/off chip", DEC);
#ifdef JUNGFRAUD
		// narrow down error when powering on
		if (ret == FAIL && arg > 0) {
			if (setTemperatureEvent(-1) == 1)
			    sprintf(mess,"Powering chip failed due to over-temperature event. "
			    		"Clear event & power chip again. Set %d, read %d \n", arg, retval);
			FILE_LOG(logERROR, (mess));
		}
#endif
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int set_activate(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting activate mode to %d\n", arg));

#ifndef EIGERD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = activate(arg);
		FILE_LOG(logDEBUG1, ("Activate: %d\n", retval));
		validate(arg, retval, "set activate", DEC);
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int prepare_acquisition(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Preparing Acquisition\n"));
#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = prepareAcquisition();
		if (ret == FAIL) {
			strcpy(mess, "Could not prepare acquisition\n");
			FILE_LOG(logERROR, (mess));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}




// stop server
int threshold_temp(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting threshold temperature to %d\n", arg));

#ifndef JUNGFRAUD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
	    if (arg > MAX_THRESHOLD_TEMP_VAL)   {
	        ret = FAIL;
	        sprintf(mess,"Threshold Temp %d should be in range: 0 - %d\n",
	        		arg, MAX_THRESHOLD_TEMP_VAL);
	        FILE_LOG(logERROR, (mess));
	    }
		// valid temp
	    else {
			retval = setThresholdTemperature(arg);
			FILE_LOG(logDEBUG1, ("Threshold temperature: %d\n", retval));
			validate(arg, retval, "set threshold temperature", DEC);
	    }
	}
#endif
    return Server_SendResult(file_des, INT32, NO_UPDATE, &retval, sizeof(retval));
}


// stop server
int temp_control(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting temperature control to %d\n", arg));

#ifndef JUNGFRAUD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = setTemperatureControl(arg);
		FILE_LOG(logDEBUG1, ("Temperature control: %d\n", retval));
		validate(arg, retval, "set temperature control", DEC);
	}
#endif
	return Server_SendResult(file_des, INT32, NO_UPDATE, &retval, sizeof(retval));
}



// stop server
int temp_event(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting temperature event to %d\n", arg));

#ifndef JUNGFRAUD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = setTemperatureEvent(arg);
		FILE_LOG(logDEBUG1, ("Temperature event: %d\n", retval));
		validate(arg, retval, "set temperature event", DEC);
	}
#endif
    return Server_SendResult(file_des, INT32, NO_UPDATE, &retval, sizeof(retval));
}





int auto_comp_disable(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting  Auto comp disable to %d\n", arg));

#ifndef JUNGFRAUD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || (Server_VerifyLock() == OK)) {
		retval = autoCompDisable(arg);
		FILE_LOG(logDEBUG1, ("Auto comp disable: %d\n", retval));
		validate(arg, retval, "set auto comp disable", DEC);
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}





int storage_cell_start(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    FILE_LOG(logDEBUG1, ("Setting Storage cell start to %d\n", arg));

#ifndef JUNGFRAUD
    functionNotImplemented();
#else
    // set & get
    if ((arg == -1) || (Server_VerifyLock() == OK)) {
        if (arg > MAX_STORAGE_CELL_VAL) {
            ret = FAIL;
            strcpy(mess,"Max Storage cell number should not exceed 15\n");
            FILE_LOG(logERROR, (mess));
        } else {
            retval = selectStoragecellStart(arg);
            FILE_LOG(logDEBUG1, ("Storage cell start: %d\n", retval));
            validate(arg, retval, "set storage cell start", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int check_version(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t arg = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
		return printSocketReadError();

	// check software- firmware compatibility and basic tests
	if (isControlServer) {
		FILE_LOG(logDEBUG1, ("Checking software-firmware compatibility and basic test result\n"));

		// check if firmware check is done
		if (!isFirmwareCheckDone()) {
			usleep(3 * 1000 * 1000);
			if (!isFirmwareCheckDone()) {
				ret = FAIL;
				strcpy(mess,"Firmware Software Compatibility Check (Server Initialization) "
						"still not done done in server. Unexpected.\n");
				FILE_LOG(logERROR,(mess));
			}
		}
		// check firmware check result
		if (ret == OK) {
			char* firmware_message = NULL;
			if (getFirmwareCheckResult(&firmware_message) == FAIL) {
				ret = FAIL;
				strcpy(mess, firmware_message);
				FILE_LOG(logERROR,(mess));
			}
		}
	}

	if (ret == OK) {
		FILE_LOG(logDEBUG1, ("Checking versioning compatibility with value 0x%llx\n",arg));

		int64_t client_requiredVersion = arg;
		int64_t det_apiVersion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);
		int64_t det_version = getDetectorId(DETECTOR_SOFTWARE_VERSION);

		// old client
		if (det_apiVersion > client_requiredVersion) {
			ret = FAIL;
			sprintf(mess,"Client's detector SW API version: (0x%llx). "
					"Detector's SW API Version: (0x%llx). "
					"Incompatible, update client!\n",
					(long long int)client_requiredVersion, (long long int)det_apiVersion);
			FILE_LOG(logERROR,(mess));
		}

		// old software
		else if (client_requiredVersion > det_version) {
			ret = FAIL;
			sprintf(mess,"Detector SW Version: (0x%llx). "
					"Client's detector SW API Version: (0x%llx). "
					"Incompatible, update detector software!\n",
					(long long int)det_version, (long long int)client_requiredVersion);
			FILE_LOG(logERROR,(mess));
		}
	}
	return Server_SendResult(file_des, INT32, NO_UPDATE, NULL, 0);
}




int software_trigger(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Software Trigger\n"));
#ifndef EIGERD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		ret = softwareTrigger();
		if (ret == FAIL) {
			sprintf(mess, "Could not send software trigger\n");
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("Software trigger successful\n"));
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}


int led(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    FILE_LOG(logDEBUG1, ("Setting led enable to %d\n", arg));

#if (!defined(MOENCHD)) && (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // set & get
    if ((arg == -1) || (Server_VerifyLock() == OK)) {
    	retval = setLEDEnable(arg);
    	FILE_LOG(logDEBUG1, ("LED Enable: %d\n", retval));
    	validate(arg, retval, "LED Enable", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, UPDATE, &retval, sizeof(retval));
}




int digital_io_delay(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint64_t args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT64) < 0)
        return printSocketReadError();
    FILE_LOG(logDEBUG1, ("Digital IO Delay, pinMask: 0x%llx, delay:%d ps\n", args[0], (int)args[1]));

#if (!defined(MOENCHD)) && (!defined(CHIPTESTBOARDD))
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		int delay = (int)args[1];
		if (delay < 0 || delay > DIGITAL_IO_DELAY_MAXIMUM_PS) {
			ret = FAIL;
			sprintf(mess, "Could not set digital IO delay. Delay maximum is %d ps\n", DIGITAL_IO_DELAY_MAXIMUM_PS);
			FILE_LOG(logERROR,(mess));
		} else {
			setDigitalIODelay(args[0], delay);
			FILE_LOG(logDEBUG1, ("Digital IO Delay successful\n"));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, UPDATE, NULL, 0);
}

int copy_detector_server(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char args[2][MAX_STR_LENGTH];
	char retvals[MAX_STR_LENGTH] = {0};

    memset(args, 0, sizeof(args));
    memset(retvals, 0, sizeof(retvals));

    if (receiveData(file_des, args, sizeof(args), OTHER) < 0)
        return printSocketReadError();

#ifdef EIGERD
    functionNotImplemented();
#else

    // only set
    if (Server_VerifyLock() == OK) {
        char* sname = args[0];
        char* hostname = args[1];
        FILE_LOG(logINFOBLUE, ("Copying server %s from host %s\n", sname, hostname));

        char cmd[MAX_STR_LENGTH];
        memset(cmd, 0, MAX_STR_LENGTH);

        // copy server
        sprintf(cmd, "tftp %s -r %s -g", hostname, sname);
        int success = executeCommand(cmd, retvals, logDEBUG1);
        if (success == FAIL) {
        	ret = FAIL;
        	strcpy(mess, retvals);
        	//FILE_LOG(logERROR, (mess)); already printed in executecommand
        }

        // success
        else {
        	FILE_LOG(logINFO, ("Server copied successfully\n"));
        	// give permissions
        	sprintf(cmd, "chmod 777 %s", sname);
        	executeCommand(cmd, retvals, logDEBUG1);

        	// edit /etc/inittab
        	// find line numbers in /etc/inittab where DetectorServer
        	strcpy(cmd, "sed -n '/DetectorServer/=' /etc/inittab");
        	executeCommand(cmd, retvals, logDEBUG1);
        	while (strlen(retvals)) {
        		// get first linen number
        		int lineNumber = atoi(retvals);
        		// delete that line
        		sprintf(cmd, "sed -i \'%dd\' /etc/inittab", lineNumber);
        		executeCommand(cmd, retvals, logDEBUG1);
        		// find line numbers again
        		strcpy(cmd, "sed -n '/DetectorServer/=' /etc/inittab");
        		executeCommand(cmd, retvals, logDEBUG1);
        	}
        	FILE_LOG(logINFO, ("Deleted all lines containing DetectorServer in /etc/inittab\n"));

        	// append line
        	sprintf(cmd, "echo \"ttyS0::respawn:/./%s\" >> /etc/inittab", sname);
        	executeCommand(cmd, retvals, logDEBUG1);

        	FILE_LOG(logINFO, ("/etc/inittab modified to have %s\n", sname));
        }
    }
#endif
    return Server_SendResult(file_des, OTHER, NO_UPDATE, retvals, sizeof(retvals));
}


int reboot_controller(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
#ifdef EIGERD
	functionNotImplemented();
	return ret;
#else
	FILE_LOG(logINFORED, ("Rebooting controller\n"));
	return REBOOT;
#endif
}
