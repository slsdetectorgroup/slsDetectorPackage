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
int dataBytes = 10;
#ifdef EIGERD
uint32_t dhcpipad = 0;
#endif
#ifdef GOTTHARDD
int digitalTestBit = 0;
#endif



/* initialization functions */

int printSocketReadError() {
	FILE_LOG(logERROR, ("Error reading from socket. Possible socket crash.\n"));
	return FAIL;
}


void basictests() {
	checkFirmwareCompatibility();
}


void init_detector() {
#ifdef VIRTUAL
	FILE_LOG(logINFO, ("This is a VIRTUAL detector\n"));
#endif
	if (isControlServer) {
	    basictests();
#ifdef JUNGFRAUD
	    if (debugflag != PROGRAMMING_MODE)
#endif
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

	// jungfrau in programming mode
#ifdef JUNGFRAUD
	if ((debugflag == PROGRAMMING_MODE) &&
			(fnum != F_PROGRAM_FPGA) &&
			(fnum != F_GET_DETECTOR_TYPE) &&
			(fnum != F_RESET_FPGA) &&
			(fnum != F_UPDATE_CLIENT) &&
			(fnum != F_CHECK_VERSION)) {
		ret = (M_nofuncMode)(file_des);
	}
	else
#endif
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
		} else FILE_LOG(logDEBUG1, ("Function (%s) executed ok\n",
				getFunctionName((enum detFuncs)fnum)));
	}
	return ret;
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
	flist[F_SET_CTB_PATTERN]					= &set_ctb_pattern;
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

void validate(int arg, int retval, char* modename, int hex) {
	if (ret == OK && arg != -1 && retval != arg) {
		ret = FAIL;
		if (hex)
			sprintf(mess, "Could not %s. Set 0x%x, but read 0x%x\n",
				modename, arg, retval);
		else
			sprintf(mess, "Could not %s. Set %d, but read %d\n",
				modename, arg, retval);
		FILE_LOG(logERROR,(mess));
	}
}

void validate64(int64_t arg, int64_t retval, char* modename, int hex) {
	if (ret == OK && arg != -1 && retval != arg) {
		ret = FAIL;
		if (hex)
			sprintf(mess, "Could not %s. Set 0x%llx, but read 0x%llx\n",
				modename, (long long unsigned int)arg, (long long unsigned int)retval);
		else
			sprintf(mess, "Could not %s. Set %lld, but read %lld\n",
				modename, (long long unsigned int)arg, (long long unsigned int)retval);
		FILE_LOG(logERROR,(mess));
	}
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
	return Server_SendResult(file_des, OTHER, 0, NULL, 0);
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
	return Server_SendResult(file_des, OTHER, 0, NULL, 0);
}




int exec_command(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	if (receiveData(file_des, cmd, MAX_STR_LENGTH, OTHER) < 0)
		return printSocketReadError();
	FILE_LOG(logINFO, ("Executing command (%s)\n", cmd));

	// set
	if (Server_VerifyLock() == OK) {
		FILE* sysFile = popen(cmd, "r");
		const size_t tempsize = 256;
		char temp[tempsize];
		memset(temp, 0, tempsize);
		while(fgets(temp, tempsize, sysFile) != NULL) {
			// size left excludes terminating character
			size_t sizeleft = MAX_STR_LENGTH - strlen(retval) - 1;
			// more than the command
			if (tempsize > sizeleft) {
				strncat(retval, temp, sizeleft);
				break;
			}
			strncat(retval, temp, tempsize);
			memset(temp, 0, tempsize);
		}
		pclose(sysFile);
		FILE_LOG(logINFO, ("Result of cmd (%s):\n%s\n", cmd, retval));
	}
	return Server_SendResult(file_des, OTHER, 0, retval, sizeof(retval));
}




int get_detector_type(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum detectorType retval = myDetectorType;
	FILE_LOG(logDEBUG1,("Returning detector type %d\n", retval));
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	// set
	if ((flag != GET_EXTERNAL_SIGNAL_FLAG) && (Server_VerifyLock() == OK)) {
		setExtSignal(signalindex, flag);
	}
	// get
	retval = getExtSignal(signalindex);
	validate((int)flag, (int)retval, "set external signal flag", 1);
	FILE_LOG(logDEBUG1, ("External Signal Flag: %d\n", retval));
}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
		case GATE_FIX_NUMBER:
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
	validate((int)arg, (int)retval, "set timing mode", 0);
	FILE_LOG(logDEBUG1, ("Timing Mode: %d\n",retval));
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
#if defined(EIGERD) || defined(JUNGFRAUD)
	case SOFTWARE_FIRMWARE_API_VERSION:
#endif
	case DETECTOR_SERIAL_NUMBER:
	case DETECTOR_FIRMWARE_VERSION:
	case DETECTOR_SOFTWARE_VERSION:
		retval = getDetectorId(arg);
		FILE_LOG(logDEBUG1, ("Id(%d): %lld\n", retval));
		break;
	default:
		modeNotImplemented("ID Index", (int)arg);
		break;
	}
	return Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));
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
#ifdef GOTTHARDD
		case DIGITAL_BIT_TEST:
			FILE_LOG(logDEBUG1, ("Setting digital test bit: %d\n", ival));
			if (ival >= 0)
				digitalTestBit = (ival > 0) ? 1 : 0;
			retval = digitalTestBit;
			break;
#endif
		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_BUS_TEST:
			retval = detectorTest(mode);
			FILE_LOG(logDEBUG1, ("Digital Test (%d): %d\n", mode, retval));
			break;
		default:
			modeNotImplemented("Digital Test Mode", (int)mode);
			break;
		}
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}






int set_dac(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[3] = {-1, -1, -1};
    int retval[2] = {-1, -1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	enum dacIndex ind = args[0];
	int mV = args[1];
	int val = args[2];
    enum DACINDEX serverDacIndex = 0;

    // check if dac exists for this detector
#ifdef JUNGFRAUD
    if ((ind != HV_NEW) && (ind >= NDAC_OLDBOARD)) {	//for compatibility with old board
    	modeNotImplemented("Dac Index", (int)ind);
    } else
    	serverDacIndex = ind;
#else
    switch (ind) {
#ifdef GOTTHARDD
    case G_VREF_DS :
    case G_VCASCN_PB:
    case G_VCASCP_PB:
    case G_VOUT_CM:
    case G_VCASC_OUT:
    case G_VIN_CM:
    case G_VREF_COMP:
    case G_IB_TESTC:
    case HV_POT:
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
    case HV_NEW:
    case IO_DELAY:
        break;
#endif
    default:
    	modeNotImplemented("Dac Index", (int)ind);
        break;
    }
#endif

    // index exists
    if (ret == OK) {

        FILE_LOG(logDEBUG1, ("Setting DAC %d to %d %s\n", serverDacIndex, val,
        		(mV ? "mV" : "dac units")));

    	// set & get
    	if ((val == -1) || ((val != -1) && (Server_VerifyLock() == OK))) {
    		switch(ind) {

    		// io delay
#ifdef EIGERD
    		case IO_DELAY:
    			retval[0] = setIODelay(val);
    			FILE_LOG(logDEBUG1, ("IODelay: %d\n", retval[0]));
    			break;
#endif

    		// high voltage
    		case HV_POT:
    		case HV_NEW:
    			retval[0] = setHighVoltage(val);
    			FILE_LOG(logDEBUG1, ("High Voltage: %d\n", retval[0]));
#ifdef EIGERD
    			if ((retval[0] != SLAVE_HIGH_VOLTAGE_READ_VAL) && (retval[0] < 0)) {
    				ret = FAIL;
    				if (retval[0] == -1)
    					sprintf(mess, "Setting high voltage failed. Bad value %d. "
    							"The range is from 0 to 200 V.\n",val);
    				else if (retval[0] == -2)
    					strcpy(mess, "Setting high voltage failed. "
    							"Serial/i2c communication failed.\n");
    				else if (retval[0] == -3)
    					strcpy(mess, "Getting high voltage failed. "
    							"Serial/i2c communication failed.\n");
    				FILE_LOG(logERROR,(mess));
    			}
#endif
    			break;

                // dacs
    			default:
                    setDAC(serverDacIndex, val, mV, retval);
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
                    	int temp = 0;
                        if (mV)
                            temp = retval[1];
                        else
                            temp = retval[0];
                        if ((abs(temp-val) <= 5) || val == -1) {
                            ret = OK;
                        } else {
                            ret = FAIL;
                            sprintf(mess,"Setting dac %d : wrote %d but read %d\n", serverDacIndex, val, temp);
                            FILE_LOG(logERROR,(mess));
                        }
                    }
                    FILE_LOG(logDEBUG1, ("Dac (%d): %d dac units and %d mV\n", serverDacIndex, retval[0], retval[1]));
    				break;
    		}
    	}
    }
    return Server_SendResult(file_des, INT32, 1, retval, sizeof(retval));
}






int get_adc(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum dacIndex ind = 0;
	int retval = -1;

	if (receiveData(file_des, &ind, sizeof(ind), INT32) < 0)
		return printSocketReadError();
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
#endif
	default:
		modeNotImplemented("Adc Index", (int)ind);
		break;
	}

	// valid index
	if (ret == OK) {
		FILE_LOG(logDEBUG1, ("Getting ADC %d\n", serverAdcIndex));
		retval = getADC(serverAdcIndex);
		FILE_LOG(logDEBUG1, ("ADC(%d): %d\n", retval));
	}
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
		retval = writeRegister(addr, val);
		// validate
		if (retval != val) {
			ret = FAIL;
			sprintf(mess,"Could not write to register 0x%x. Wrote 0x%x but read 0x%x\n", addr, val, retval);
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("Write register (0x%x): 0x%x\n", retval));
	}
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	retval = readRegister(addr);
	FILE_LOG(logDEBUG1, ("Read register (0x%x): 0x%x\n", retval));

	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}




int set_module(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum detectorSettings retval = -1;

	sls_detector_module module;
	int *myDac = NULL;
	int *myAdc = NULL;
	int *myChan = NULL;
	module.dacs = NULL;
	module.adcs = NULL;
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

	// allocate adcs
	if (ret == OK) {
		myAdc = (int*)malloc(getNumberOfADCs() * sizeof(int));
		// error
		if (getNumberOfADCs() > 0 && myAdc == NULL) {
			ret = FAIL;
			sprintf(mess,"Could not allocate adcs\n");
			FILE_LOG(logERROR,(mess));
		} else
			module.adcs = myAdc;
	}

	// allocate chans
#ifdef EIGERD
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
		module.nadc = getNumberOfADCs();
		int ts = receiveModule(file_des, &module);
		if (ts < 0) {
			if (myChan != NULL) free(myChan);
			if (myDac != NULL) 	free(myDac);
			if (myAdc != NULL) 	free(myAdc);
			return printSocketReadError();
		}
		FILE_LOG(logDEBUG1, ("module register is %d, nchan %d, nchip %d, "
				"ndac %d, nadc %d, iodelay %d, tau %d, eV %d\n",
				module.reg, module.nchan, module.nchip,
				module.ndac,  module.nadc, module.iodelay, module.tau, module.eV));
		// should at least have a dac
		if (ts <= sizeof(sls_detector_module)) {
			ret = FAIL;
			sprintf(mess, "Cannot set module. Received incorrect number of dacs or adcs or channels\n");
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
			modeNotImplemented("Settings", (int)module.reg);
			break;
		}

		ret = setModule(module, mess);
		retval = getSettings();
		validate(module.reg, (int)retval, "set module (settings)", 0);
		FILE_LOG(logDEBUG1, ("Settings: %d\n", retval));
	}
	if (myChan != NULL) free(myChan);
	if (myDac != NULL) 	free(myDac);
	if (myAdc != NULL) 	free(myAdc);
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}




int get_module(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	sls_detector_module module;
	int *myDac = NULL;
	int *myAdc = NULL;
	int *myChan = NULL;
	module.dacs = NULL;
	module.adcs = NULL;
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

	// allocate adcs
	if (ret == OK) {
		myAdc = (int*)malloc(getNumberOfADCs() * sizeof(int));
		// error
		if (getNumberOfADCs() > 0 && myAdc == NULL) {
			ret = FAIL;
			sprintf(mess,"Could not allocate adcs\n");
			FILE_LOG(logERROR,(mess));
		} else
			module.adcs=myAdc;
	}

	// allocate chans
#ifdef EIGERD
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
		module.nadc = getNumberOfADCs();

		// only get
		FILE_LOG(logDEBUG1, ("Getting module\n"));
		getModule(&module);
		FILE_LOG(logDEBUG1, ("Getting module. Settings:%d\n", module.reg));
	}

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	// send module, 0 is to receive partially (without trimbits etc)
	if (ret != FAIL) {
		ret = sendModule(file_des, &module);
	}
	if (myChan != NULL)	free(myChan);
	if (myDac != NULL) 	free(myDac);
	if (myAdc != NULL) 	free(myAdc);
	return ret;
}






int set_settings(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum detectorSettings isett = GET_SETTINGS;
	enum detectorSettings retval = GET_SETTINGS;

	if (receiveData(file_des, &isett, sizeof(isett), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting settings %d\n", isett));

	//set & get
	if ((isett == GET_SETTINGS) || ((isett != GET_SETTINGS) && (Server_VerifyLock() == OK))) {

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
			validate((int)isett, (int)retval, "set settings", 0);
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
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}






int start_acquisition(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Starting Acquisition\n"));
	// only set
	if (Server_VerifyLock() == OK) {
		ret = startStateMachine();
		if (ret == FAIL) {
			sprintf(mess, "Could not start acquisition\n");
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("Starting Acquisition ret: %d\n", ret));
	}
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
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
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
}





int start_readout(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Starting readout\n"));
#ifdef JUNGFRAUD
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
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
}






int get_run_status(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus retval = ERROR;

	FILE_LOG(logDEBUG1, ("Getting status\n"));
	// only get
	retval = getRunStatus();
	FILE_LOG(logDEBUG1, ("Status: %d\n", retval));
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}





int start_and_read_all(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Starting Acquisition and read all frames\n"));
	// start state machine
	FILE_LOG(logDEBUG1, ("Stopping Acquisition\n"));
	// only set
	if (Server_VerifyLock() == OK) {
		ret = startStateMachine();
		if (ret == FAIL) {
			sprintf(mess, "Could not start acquisition\n");
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("Starting Acquisition ret: %d\n", ret));

	}

	// lock or acquisition start error
	if (ret == FAIL)
		return Server_SendResult(file_des, INT32, 1 , NULL, 0);

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
	return Server_SendResult(file_des, INT32, 1 , NULL, 0);
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
#ifdef EIGERD
	int64_t subexptime = 0;
#endif
	FILE_LOG(logDEBUG1, ("Setting timer index %d to %lld ns\n", ind, tns));

	// set & get
	if ((tns == -1) || ((tns != -1) && (Server_VerifyLock() == OK))) {

		// check index
		switch (ind) {
		case FRAME_NUMBER:
		case ACQUISITION_TIME:
		case FRAME_PERIOD:
		case CYCLES_NUMBER:
#ifdef JUNGFRAUD
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
			modeNotImplemented("Timer index", (int)ind);
			break;
		}
		// validate
		validate64(tns, retval, "set timer", 0);
	}
	if (ret != FAIL) {
		FILE_LOG(logDEBUG1, ("Timer index %d: %lld\n", ind, retval));
	}
	return Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));
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
#elif GOTTHARDD
        case GATES_NUMBER:
#endif
#if defined(GOTTHARDD) || defined(JUNGFRAUD)
        case FRAME_NUMBER:
        case ACQUISITION_TIME:
        case FRAME_PERIOD:
        case DELAY_AFTER_TRIGGER:
        case CYCLES_NUMBER:
        case PROGRESS:
        case ACTUAL_TIME:
        case MEASUREMENT_TIME:
#endif
            retval = getTimeLeft(ind);
            FILE_LOG(logDEBUG1, ("Timer left index %d: %lld\n", ind, retval));
            break;
        default:
            modeNotImplemented("Timer left index", (int)ind);
            break;
        }
    }
    return Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));
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
	if ((dr == -1) || ((dr != -1) && (Server_VerifyLock() == OK))) {

#ifdef EIGERD
		int old_dr = setDynamicRange(-1);
#endif
		// check dr
		switch(dr) {
		case -1:
		case 16:
#ifdef EIGERD
		case 4:	case 8:	case 32:
#endif
			retval = setDynamicRange(dr);
			FILE_LOG(logDEBUG1, ("Dynamic range: %d\n", retval));
			validate(dr, retval, "set dynamic range", 0);
			if (dr >= 0)
				dataBytes=calculateDataBytes();
			break;
		default:
			modeNotImplemented("Dynamic range", dr);
			break;
		}

#ifdef EIGERD
		if (dr != -1 && getRateCorrectionEnable()) {
			// 4 or 8 bit, switch off rate corr (only if dr worked)
			if ((dr != 32) && (dr != 16) && (ret == OK)) {
				setRateCorrection(0);
				ret = FAIL;
				strcpy(mess,"Switching off Rate Correction. Must be in 32 or 16 bit mode\n");
				FILE_LOG(logERROR,(mess));
			}

			// 16, 32 (diff from old value), set new tau in rate table(-1), if it didnt work, give error
			else if ((dr == 16 || dr == 32) && (old_dr != dr)) {
				setRateCorrection(-1); //tau_ns will not be -1 here
				// it didnt work
				if (!getRateCorrectionEnable()) {
					ret = FAIL;
					strcpy(mess,"Deactivating Rate Correction. Could not set it.\n");
					FILE_LOG(logERROR,(mess));
				}
			}
		}
#endif
	}
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}






int set_readout_flags(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum readOutFlags arg = GET_READOUT_FLAGS;
	enum readOutFlags retval = GET_READOUT_FLAGS;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Setting readout flags to %d\n", arg));

#ifndef EIGERD
	functionNotImplemented();
#else
	// set & get
	if ((arg == GET_READOUT_FLAGS) || ((arg != GET_READOUT_FLAGS) && (Server_VerifyLock() == OK))) {

		switch(arg) {
		case STORE_IN_RAM:
		case CONTINOUS_RO:
		case PARALLEL:
		case NONPARALLEL:
		case SAFE:
		case SHOW_OVERFLOW:
		case NOOVERFLOW:
			retval = setReadOutFlags(arg);
			FILE_LOG(logDEBUG1, ("Read out flags: 0x%x\n", retval));
			validate((int)arg, (int)(retval & arg), "set readout flag", 1);
			break;
		default:
			modeNotImplemented("Read out flag index", (int)arg);
			break;
		}
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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

#ifndef GOTTHARDD
	functionNotImplemented();
#else
	// set & get
	if ((arg == GET_READOUT_FLAGS) || ((arg != GET_READOUT_FLAGS) && (Server_VerifyLock() == OK))) {
		retval = setROI(narg, arg, &nretval, &ret);
		if (ret == FAIL) {
			sprintf(mess,"Could not set all roi. "
					"Set %d rois, but read %d rois\n", narg, nretval);
			FILE_LOG(logERROR,(mess));
		}
		FILE_LOG(logDEBUG1, ("nRois: %d\n", nretval));
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

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
	int args[2] = {-1,-1};
	int retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	enum speedVariable ind = args[0];
	int val = args[1];
	FILE_LOG(logDEBUG1, ("Setting speed index %d to %d\n", ind, val));

	// set & get
	if ((val == -1) || ((val != -1) && (Server_VerifyLock() == OK))) {
		// check index
		switch(ind) {
#ifdef JUNGFRAUD
		case ADC_PHASE:
			retval = adcPhase(val);
			FILE_LOG(logDEBUG1, ("ADc Phase: %d\n", retval));
			if (val != 100000) {
				validate(val, retval, "set adc phase ", 0);
			}
			break;
#endif
		case CLOCK_DIVIDER:
			retval = setSpeed(val);
			FILE_LOG(logDEBUG1, ("Clock: %d\n", retval));
			validate(val, retval, "set clock ", 0);
			break;
		default:
			modeNotImplemented("Speed index", (int)ind);
			break;
		}
	}
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}






int exit_server(int file_des) {
	cprintf(BG_RED, "Closing Server\n");
	ret = OK;
	memset(mess, 0, sizeof(mess));
	Server_SendResult(file_des, INT32, 0, NULL, 0);
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
			strcpy(lastClientIP, thisClientIP);
		}   else {
			Server_LockedError();
		}
	}
	return Server_SendResult(file_des, INT32, 1, &lockStatus, sizeof(lockStatus));
}




int get_last_client_ip(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	return Server_SendResult(file_des, INT32, 1, lastClientIP, sizeof(lastClientIP));
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

	Server_SendResult(file_des, INT32, 1, &p_number, sizeof(p_number));
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
#ifdef JUNGFRAUD
	if (debugflag == PROGRAMMING_MODE) {
	    ret = OK;
	}
#endif
	Server_SendResult(file_des, INT32, 0, NULL, 0);
	if (ret == OK)
	    return ret;
	return send_update(file_des);
}




int send_update(int file_des) {
    ret = OK;
	int n = 0;
	int i32 = -1;
	int64_t i64 = -1;

	n = sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
	if (n < 0) return printSocketReadError();

	i32 = setDynamicRange(GET_FLAG);
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();

	dataBytes = calculateDataBytes();
	n = sendData(file_des,&dataBytes,sizeof(dataBytes),INT32);
	if (n < 0) return printSocketReadError();

	i32 = (int)setSettings(GET_SETTINGS);
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();

#ifdef EIGERD
	i32 = getThresholdEnergy(GET_FLAG);
	n = sendData(file_des,&i32,sizeof(i32),INT32);
	if (n < 0) return printSocketReadError();
#endif

	i64 = setTimer(FRAME_NUMBER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	i64 = setTimer(ACQUISITION_TIME,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

#ifdef EIGERD
	i64 = setTimer(SUBFRAME_ACQUISITION_TIME,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	i64 = setTimer(SUBFRAME_DEADTIME,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();
#endif

	i64 = setTimer(FRAME_PERIOD,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

#ifndef EIGERD
	i64 = setTimer(DELAY_AFTER_TRIGGER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();
#endif

#if !defined(EIGERD) && !defined(JUNGFRAUD)
	i64 = setTimer(GATES_NUMBER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();
#endif

	i64 = setTimer(CYCLES_NUMBER,GET_FLAG);
	n = sendData(file_des,&i64,sizeof(i64),INT64);
	if (n < 0) return printSocketReadError();

	if (lockStatus == 0) {
		strcpy(lastClientIP,thisClientIP);
	}

	return ret;
}






int configure_mac(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char args[9][50];
	memset(args, 0, sizeof(args));
	char retvals[3][50];
	memset(retvals, 0, sizeof(retvals));
	int iretval = -1;

	if (receiveData(file_des, args, sizeof(args), OTHER) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("\n Configuring UDP Destination\n"));
#ifdef GOTTHARDD
	FILE_LOG(logDEBUG1, ("Digital Test Bit %d\t", digitalTestBit);
#endif
	uint32_t dstIp = 0;
	sscanf(args[0], "%x", 	&dstIp);
	FILE_LOG(logDEBUG1, ("Dst Ip Addr: %d.%d.%d.%d = 0x%x \n",
			(dstIp >> 24) & 0xff, (dstIp >> 16) & 0xff, (dstIp >> 8) & 0xff, (dstIp) & 0xff,
			dstIp));
	uint64_t dstMac = 0;
#ifdef VIRTUAL
	sscanf(args[1], "%lx", 	&dstMac);
#else
	sscanf(args[1], "%llx", 	&dstMac);
#endif
	FILE_LOG(logDEBUG1, ("Dst Mac Addr: (0x) "));
	{
		int iloop = 5;
		for (iloop = 5; iloop >= 0; --iloop) {
			FILE_LOG(logDEBUG1, ("%x", (unsigned int)(((dstMac >> (8 * iloop)) & 0xFF))));
			if (iloop > 0) {
				FILE_LOG(logDEBUG1, (":"));
			}
		}
	}
	FILE_LOG(logDEBUG1, (" = %llx\n", dstMac));
	uint32_t dstPort = 0;
	sscanf(args[2], "%x", 	&dstPort);
	FILE_LOG(logDEBUG1, ("Dst Port: %x\n", dstPort));
	uint32_t dstPort2 = 0;
	sscanf(args[5], "%x", 	&dstPort2);
	FILE_LOG(logDEBUG1, ("Dst Port2: %x\n", dstPort2));
	uint64_t srcMac = 0;
#ifdef VIRTUAL
	sscanf(args[3], "%lx",	&srcMac);
#else
	sscanf(args[3], "%llx",	&srcMac);
#endif
	FILE_LOG(logDEBUG1, ("Src Mac Addr: (0x) "));
	{
		int iloop = 5;
		for (iloop = 5; iloop >= 0; --iloop) {
			FILE_LOG(logDEBUG1, ("%x", (unsigned int)(((srcMac >> (8 * iloop)) & 0xFF))));
			if (iloop > 0) {
				FILE_LOG(logDEBUG1, (":"));
			}
		}
	}
	FILE_LOG(logDEBUG1, (" = %llx\n", srcMac));
	uint32_t srcIp = 0;
	sscanf(args[4], "%x",	&srcIp);
	FILE_LOG(logDEBUG1, ("Src Ip Addr: %d.%d.%d.%d = 0x%x \n",
			(srcIp >> 24) & 0xff, (srcIp >> 16) & 0xff, (srcIp >> 8) & 0xff, (srcIp) & 0xff,
			srcIp));
#if defined(JUNGFRAUD) || defined(EIGERD)
	int pos[3] = {0, 0, 0};
	sscanf(args[6], "%x", 	&pos[0]);
	sscanf(args[7], "%x", 	&pos[1]);
	sscanf(args[8], "%x", 	&pos[2]);
	FILE_LOG(logDEBUG1, ("Position: [%d, %d, %d]\n", pos[0], pos[1], pos[2]));
#endif

	// set only
	if ((Server_VerifyLock() == OK)) {

		// stop detector if it was running
		if (getRunStatus() != IDLE) {
			ret = FAIL;
			sprintf(mess, "Cannot configure mac when detector is not idle\n");
			FILE_LOG(logERROR,(mess));
		}

		else {
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

#endif
#ifdef GOTTHARDD
			iretval = configureMAC(dstIp, dstMac, srcMac, srcIp, dstPort, dstPort2, digitalTestBit);
			FILE_LOG(logDEBUG1, ("Configure mac retval: %d\n", iretval));
			FILE_LOG(logINFO, ("Configure MAC successful: %d\n", iretval));
#else
			iretval = configureMAC(dstIp, dstMac, srcMac, srcIp, dstPort, dstPort2);
			FILE_LOG(logDEBUG1, ("Configure mac retval: %d\n", iretval));

			if (iretval == -1) {
				ret = FAIL;
				sprintf(mess,"Configure Mac failed\n");
				FILE_LOG(logERROR,(mess));
			} else {
				FILE_LOG(logINFO, ("Configure MAC successful\n"));
				ret = setDetectorPosition(pos);
				if (ret == FAIL) {
					sprintf(mess, "Could not set detector position\n");
					FILE_LOG(logERROR,(mess));
				}
			}
#endif
			// set retval vals
			if (ret != FAIL) {
				sprintf(retvals[0],"%x", iretval);
				sprintf(retvals[1],"%llx", (long long unsigned int)srcMac);
				sprintf(retvals[2],"%x", srcIp);
			}
		}
	}

	return Server_SendResult(file_des, OTHER, 1, retvals, sizeof(retvals));
}





int load_image(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1, -1};

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();
	enum imageType index = args[0];
	char* ImageVals = NULL;
	if (args[1] > 0) {
		ImageVals = (char*)malloc(args[1]);
		if (receiveData(file_des, ImageVals, args[1], OTHER) < 0) {
			free(ImageVals);
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
			ret = loadImage(index, ImageVals);
			FILE_LOG(logDEBUG1, ("Loading image ret: %d\n", ret));
			if (ret == FAIL) {
				sprintf(mess, "Could not load image\n");
				FILE_LOG(logERROR,(mess));
			}
			break;
		default:
			modeNotImplemented("Image index", (int)index);
			break;
		}
	}
#endif
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
}






int read_counter_block(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int startACQ = -1;
	char retval[dataBytes];
	memset(retval, 0, dataBytes);

	if (receiveData(file_des, &startACQ, sizeof(startACQ), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Read counter block with start acq bit: %d\n", startACQ));

#ifndef GOTTHARDD
	functionNotImplemented();
#else

	// only set
	if (Server_VerifyLock() == OK) {
		ret = readCounterBlock(startACQ, retval);
		if (ret == FAIL) {
			strcpy(mess, "Could not read counter block\n");
			FILE_LOG(logERROR,(mess));
		}
	}
#endif
	return Server_SendResult(file_des, OTHER, 1, retval, dataBytes);
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
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
}





int enable_ten_giga(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Enable/ Disable 10GbE : %d\n", arg));

#ifndef EIGERD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
		retval = enableTenGigabitEthernet(arg);
		FILE_LOG(logDEBUG1, ("10GbE: %d\n", retval));
		validate(arg, retval, "enable/disable 10GbE", 0);
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	validate(arg, retval, "set all trimbits", 0);
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}




int set_ctb_pattern(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	functionNotImplemented();
    return Server_SendResult(file_des, INT32, 1, NULL, 0);
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

#ifndef JUNGFRAUD
	functionNotImplemented();
#else
#ifndef VIRTUAL
	// only set
	if (Server_VerifyLock() == OK)
		setAdc(addr, val);
#endif
#endif
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
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
	validate(arg, retval, "set counter bit", 0);
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
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
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
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
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
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
			validate64(tau_ns, retval, "set rate correction", 0);
		}
	}
#endif
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
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
	return Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));
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
	enum NETWORKINDEX serverIndex = 0;
	FILE_LOG(logDEBUG1, ("Set network parameter index %d to %d\n", mode, value));

#ifdef GOTTHARDD
	functionNotImplemented();
#else
	// set & get
	if ((value == -1) || ((value != -1) && (Server_VerifyLock() == OK))) {
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
			validate(value, retval, "set network parameter", 0);
		}
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
}






int program_fpga(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

#ifndef JUNGFRAUD
	//to receive any arguments
	int n = 1;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	functionNotImplemented();
#else
#ifndef VIRTUAL
	// only set
	if (Server_VerifyLock() == OK) {

		// not in programming mode
		if (debugflag != PROGRAMMING_MODE) {
			//to receive any arguments
			int n = 1;
			while (n > 0)
				n = receiveData(file_des, mess, MAX_STR_LENGTH, OTHER);
			ret = FAIL;
			sprintf(mess,"FPGA cannot be programmed in this mode. "
					"Restart on-board detector server with -update for programming mode.\n");
			FILE_LOG(logERROR,(mess));
		}

		else {
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

			//---------------- first ret ----------------
			Server_SendResult(file_des, INT32, 0, NULL, 0);

			if (ret != FAIL) {
				//erasing flash
				eraseFlash();
				fpgasrc = (char*)malloc(MAX_FPGAPROGRAMSIZE);
			}

			//writing to flash part by part
			while(ret != FAIL && filesize) {

				unitprogramsize = MAX_FPGAPROGRAMSIZE;  //2mb
				if (unitprogramsize > filesize) //less than 2mb
					unitprogramsize = filesize;
				FILE_LOG(logDEBUG1, ("unit size to receive is:%d\n"
						"filesize:%d\n", unitprogramsize, filesize));


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

				//---------------- middle rets ----------------
				Server_SendResult(file_des, INT32, 0, NULL, 0);



				if (ret == FAIL) {
					cprintf(RED,"Failure: Breaking out of program receiving\n");
				} else {
					//print progress
					FILE_LOG(logINFO, ("Writing to Flash:%d%%\r",
							(int) (((double)(totalsize-filesize)/totalsize)*100) ));
					fflush(stdout);
				}
			}
			printf("\n");
			FILE_LOG(logINFO, ("Done copying program\n"));

			// closing file pointer to flash and informing FPGA
			stopWritingFPGAprogram(fp);

			//free resources
			if (fpgasrc != NULL)
				free(fpgasrc);
			if (fp != NULL)
				fclose(fp);

			FILE_LOG(logDEBUG1, ("Done with program receiving command\n"));

			if (isControlServer) {
				basictests(debugflag);
				initControlServer();
			}
		}
	}
#endif
#endif
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
}





int reset_fpga(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	FILE_LOG(logDEBUG1, ("Reset FPGA\n"));
#ifndef JUNGFRAUD
	functionNotImplemented();
#else
	// only set
	if (Server_VerifyLock() == OK) {
		if (isControlServer) {
			basictests(debugflag);	// mapping of control server at least
			if (debugflag != PROGRAMMING_MODE)
				initControlServer();
		}
		else initStopServer(); //remapping of stop server
		ret = FORCE_UPDATE;
	}
#endif
	return Server_SendResult(file_des, INT32, 1, NULL, 0);
}



int power_chip(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();
	FILE_LOG(logDEBUG1, ("Powering chip to %d\n", arg));

#ifndef JUNGFRAUD
	functionNotImplemented();
#else
	// set & get
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
		retval = powerChip(arg);
		FILE_LOG(logDEBUG1, ("Power chip: %d\n", retval));
		validate(arg, retval, "power on/off chip", 0);
		// narrow down error when powering on
		if (ret == FAIL && arg > 0) {
			if (setTemperatureEvent(-1) == 1)
			    sprintf(mess,"Powering chip failed due to over-temperature event. "
			    		"Clear event & power chip again. Set %d, read %d \n", arg, retval);
			FILE_LOG(logERROR, (mess));
		}
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
		retval = activate(arg);
		FILE_LOG(logDEBUG1, ("Activate: %d\n", retval));
		validate(arg, retval, "set activate", 0);
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
	return Server_SendResult(file_des, INT32, 1 , NULL, 0);
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
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
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
			validate(arg, retval, "set threshold temperature", 0);
	    }
	}
#endif
    return Server_SendResult(file_des, INT32, 0, &retval, sizeof(retval));
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
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
		retval = setTemperatureControl(arg);
		FILE_LOG(logDEBUG1, ("Temperature control: %d\n", retval));
		validate(arg, retval, "set temperature control", 0);
	}
#endif
	return Server_SendResult(file_des, INT32, 0, &retval, sizeof(retval));
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
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
		retval = setTemperatureEvent(arg);
		FILE_LOG(logDEBUG1, ("Temperature event: %d\n", retval));
		validate(arg, retval, "set temperature event", 0);
	}
#endif
    return Server_SendResult(file_des, INT32, 0, &retval, sizeof(retval));
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
	if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
		retval = autoCompDisable(arg);
		FILE_LOG(logDEBUG1, ("Auto comp disable: %d\n", retval));
		validate(arg, retval, "set auto comp disable", 0);
	}
#endif
	return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
    if ((arg == -1) || ((arg != -1) && (Server_VerifyLock() == OK))) {
        if (arg > MAX_STORAGE_CELL_VAL) {
            ret = FAIL;
            strcpy(mess,"Max Storage cell number should not exceed 15\n");
            FILE_LOG(logERROR, (mess));
        } else {
            retval = selectStoragecellStart(arg);
            FILE_LOG(logDEBUG1, ("Storage cell start: %d\n", retval));
            validate(arg, retval, "set storage cell start", 0);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));
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
		FILE_LOG(logDEBUG1, ("Checking versioning compatibility with value %d\n",arg));

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
	return Server_SendResult(file_des, INT32, 1 , NULL, 0);
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
		FILE_LOG(logDEBUG1, ("Software trigger ret: %d\n", ret));
	}
#endif
	return Server_SendResult(file_des, INT32, 1 , NULL, 0);
}

