#include "slsDetectorServer_funcs.h"
#include "slsDetectorFunctionList.h"
#include "communication_funcs.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

//defined in the detector specific Makefile
#ifdef GOTTHARDD
const enum detectorType myDetectorType=GOTTHARD;
#elif EIGERD
const enum detectorType myDetectorType=EIGER;
#elif JUNGFRAUD
const enum detectorType myDetectorType=JUNGFRAU;
#elif MYTHEN3D
const enum detectorType myDetectorType=MYTHEN3;
#else
const enum detectorType myDetectorType=GENERIC;
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
#ifdef GOTTHARD
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
	    if (debugflag == PROGRAMMING_MODE)
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
		FILE_LOG(logDEBUG5, ("ERROR reading from socket n=%d, fnum=%d, file_des=%d, fname=%s\n",
				n, fnum, file_des, getFunctionName((enum detFuncs)fnum)));
		return FAIL;
	} else
		FILE_LOG(logDEBUG5, ("Received %d bytes\n", n ));

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
		FILE_LOG(logDEBUG5, (" calling function fnum=%d, (%s)\n",
				fnum,  getFunctionName((enum detFuncs)fnum)));
		ret=(*flist[fnum])(file_des);

		if (ret == FAIL) {
			FILE_LOG(logDEBUG5, ("Error executing the function = %d (%s)\n",
					fnum, getFunctionName((enum detFuncs)fnum)));
		}
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
	case F_CALIBRATE_PEDESTAL:				return "F_CALIBRATE_PEDESTAL";
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
	flist[F_CALIBRATE_PEDESTAL]					= &calibrate_pedestal;
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
		FILE_LOG(logDEBUG5, ("function fnum=%d, (%s) located at 0x%x\n", iloop,
				getFunctionName((enum detFuncs)iloop), (unsigned int)flist[iloop]));
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
	FILE_LOG(logWARNING,(mess));
}

void validate(int arg, int retval, char* modename, int hex) {
	if (ret == OK && arg != -1 && retval != arg) {
		ret = FAIL;
		if (hex)
			sprintf(mess, "Could not set %s. Set 0x%x, but got 0x%x\n",
				modename, arg, retval);
		else
			sprintf(mess, "Could not set %s. Set %d, but got %d\n",
				modename, arg, retval);
		FILE_LOG(logERROR,(mess));
	}
}


int  M_nofunc(int file_des){
	ret = FAIL;
	memset(mess, 0, sizeof(mess));

	// to receive any arguments
	int n = 0;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	strcpy(mess,"Unrecognized Function. Please do not proceed.\n");
	FILE_LOG(logERROR, (mess));

	Server_SendResult(file_des, OTHER, 0, NULL, 0);
	return ret;
}


// Jungfrau program mode
int  M_nofuncMode(int file_des){
	ret = FAIL;
	memset(mess, 0, sizeof(mess));

	// to receive any arguments
	int n = 0;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	sprintf(mess,"This Function %s cannot be executed as the "
			"On-board detector server in update mode.\n"
			"Restart detector server in normal mode (without any arguments) to continue.\n",
			getFunctionName((enum detFuncs)fnum));
	FILE_LOG(logERROR, (mess));

	Server_SendResult(file_des, OTHER, 0, NULL, 0);
	return ret;
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
	if (Server_VerifyLock() != FAIL) {
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

	Server_SendResult(file_des, OTHER, 0, retval, sizeof(retval));

	return ret;
}




int get_detector_type(int file_des) {
	ret = OK;
	enum detectorType retval = myDetectorType;
	FILE_LOG(logDEBUG5,("Returning detector type %d\n", retval));

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}





int set_external_signal_flag(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1,-1};
	enum externalSignalFlag retval= GET_EXTERNAL_SIGNAL_FLAG;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();

#ifndef GOTTHARDD
	functionNotImplemented();
#else
	int signalindex = args[0];
	enum externalSignalFlag flag = args[1];

	FILE_LOG(logDEBUG5, ("Setting external signal %d to flag %d\n", signalindex, flag));
	// set
	if ((flag != GET_EXTERNAL_SIGNAL_FLAG) && (Server_VerifyLock() != FAIL)) {
		setExtSignal(signalindex, flag);
	}
	// get
	retval = getExtSignal(signalindex);
	validate((int)flag, (int)retval, "external signal flag", 1);
	FILE_LOG(logDEBUG5, ("External Signal Flag: %d\n", retval));
}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int set_external_communication_mode(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum externalCommunicationMode arg = GET_EXTERNAL_COMMUNICATION_MODE;
	enum externalCommunicationMode retval = GET_EXTERNAL_COMMUNICATION_MODE;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();

	FILE_LOG(logDEBUG5, ("Setting external communication mode to %d\n", arg));

	// set
	if ((arg != GET_EXTERNAL_COMMUNICATION_MODE) && (Server_VerifyLock() != FAIL)) {
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
	validate((int)arg, (int)retval, "timing mode", 0);
	FILE_LOG(logDEBUG5, ("Timing Mode: %d\n",retval));

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int get_id(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum idMode arg = 0;
	int64_t retval = -1;

	if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
		return printSocketReadError();

	FILE_LOG(logDEBUG5, ("Getting Id %d\n", arg));

	// get
	switch (arg) {
#if defined(EIGERD) || defined(JUNGFRAUD)
	case SOFTWARE_FIRMWARE_API_VERSION:
#endif
	case DETECTOR_SERIAL_NUMBER:
	case DETECTOR_FIRMWARE_VERSION:
	case DETECTOR_SOFTWARE_VERSION:
		retval = getDetectorId(arg);
		FILE_LOG(logDEBUG5, ("Id(%d): %lld\n", retval));
		break;
	default:
		modeNotImplemented("ID Index", (int)arg);
		break;
	}

	Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));

	return ret;
}





int digital_test(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {-1, -1};
	int retval = -1;

	if (receiveData(file_des, args, sizeof(args), INT32) < 0)
		return printSocketReadError();

#ifdef EIGERD
	functionNotImplemented();
#else
	enum digitalTestMode mode = args[0];
	int ival = args[1];

	FILE_LOG(logDEBUG5, ("Digital test, mode = %d\n", mode));

	// set
	if (Server_VerifyLock() != FAIL) {
		switch (mode) {
#ifdef GOTTHARD:
		case DIGITAL_BIT_TEST:
			FILE_LOG(logDEBUG5, ("Setting digital test bit: %d\n", ival));
			if (ival >= 0)
				digitalTestBit = (ival > 0) ? 1 : 0;
			retval = digitalTestBit;
			break;
#endif
		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_BUS_TEST:
			retval = detectorTest(mode);
			FILE_LOG(logDEBUG5, ("Digital Test (%d): %d\n", mode, retval));
			break;
		default:
			modeNotImplemented("Digital Test Mode", (int)mode);
			break;
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
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
    }else
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
#elif MYTHEN3D
    case M_vIpre:
    	serverDacIndex = vIpre;
        break;
    case M_vIbias:
    	serverDacIndex = vIbias;
        break;
    case PREAMP:
    	serverDacIndex = Vrf;
        break;
    case SHAPER1:
    	serverDacIndex = VrfSh;
        break;
    case M_vIinSh:
    	serverDacIndex = vIinSh;
        break;
    case M_VdcSh:
    	serverDacIndex = VdcSh;
        break;
    case M_Vth2:
    	serverDacIndex = Vth2;
        break;
    case M_VPL:
    	serverDacIndex = VPL;
        break;
    case THRESHOLD:
    	serverDacIndex = Vth1;
        break;
    case M_Vth3:
    	serverDacIndex = Vth3;
        break;
    case TRIMBIT_SIZE:
    	serverDacIndex = Vtrim;
        break;
    case M_casSh:
    	serverDacIndex = casSh;
        break;
    case M_cas:
    	serverDacIndex = cas;
        break;
    case M_vIbiasSh:
    	serverDacIndex = vIbiasSh;
        break;
    case M_vIcin:
    	serverDacIndex = vIcin;
        break;
    case CALIBRATION_PULSE: // !!! pulse height + 1400 DACu
    	serverDacIndex = VPH;
        break;
    case M_vIpreOut:
    	serverDacIndex = vIpreOut;
        break;
    case V_POWER_A:
    	serverDacIndex = V_A;
        break;
    case V_POWER_B:
    	serverDacIndex = V_B;
        break;
    case V_POWER_IO:
    	serverDacIndex = V_IO;
        break;
    case V_POWER_CHIP:
    	serverDacIndex = V_CHIP;
        break;
    case V_LIMIT:
    	serverDacIndex = V_LIM;
        break;
#endif
    default:
    	modeNotImplemented("Dac Index", (int)ind);
        break;
    }
#endif

    // index exists
    if (ret == OK) {

        FILE_LOG(logDEBUG5, ("Setting DAC %d to %d %s\n", serverDacIndex, val,
        		(mV ? "mV" : "dac units")));

    	// set & get
    	if ((val == -1) || ((val != -1) && (Server_VerifyLock() != FAIL))) {
    		switch(ind) {

    		// io delay
#ifdef EIGERD
    		case IO_DELAY:
    			retval[0] = setIODelay(val);
    			FILE_LOG(logDEBUG5, ("IODelay: %d\n", retval[0]));
    			break;
#endif

    		// high voltage
    		case HV_POT:
    		case HV_NEW:
    			retval[0] = setHighVoltage(val);
    			FILE_LOG(logDEBUG5, ("High Voltage: %d\n", retval[0]));
#ifdef EIGERD
    			if ((retval[0] != SLAVE_HIGH_VOLTAGE_READ_VAL) && (retval[0] < 0)) {
    				ret = FAIL;
    				if(retval[0] == -1)
    					sprintf(mess, "Setting high voltage failed. Bad value %d. "
    							"The range is from 0 to 200 V.\n",val);
    				else if(retval[0] == -2)
    					strcpy(mess, "Setting high voltage failed. "
    							"Serial/i2c communication failed.\n");
    				else if(retval[0] == -3)
    					strcpy(mess, "Getting high voltage failed. "
    							"Serial/i2c communication failed.\n");
    				FILE_LOG(logWARNING,(mess));
    			}
#endif
    			break;

    		// power
#ifdef MYTHEN3D
    		case V_POWER_A:
    		case V_POWER_B:
    		case V_POWER_C:
    		case V_POWER_D:
    		case V_POWER_IO:
    		case V_POWER_CHIP:
    		case V_LIMIT:
    			FILE_LOG(logDEBUG5, ("Setting a power %d to %d\n",ind, val);
                if (!mV) {
                    ret = FAIL;
                    strcpy(mess, "Power of index %d should be set in mV instead of DACu",
                    		serverDacIndex);
                    FILE_LOG(logWARNING,(mess));
                    val = -1;
                }

                int lim = getVLimit();
                if (ind != V_LIMIT && lim != -1 && val > lim) {
                    ret = FAIL;
                    strcpy(mess, "Power of index %d is %d, should be less than %dmV\n",
                    		serverDacIndex, val, lim);
                    FILE_LOG(logWARNING,(mess));
                    val = -1;
                }

                retval[1] = retval[0] = setPower(serverDacIndex,val);
                validate(val, retval[1], "power", 0);
                FILE_LOG(logDEBUG5, ("Power (%d): %d\n", serverDacIndex, retval[1]));
                break;
#endif

                // dacs
    			default:
#ifdef MYTHEN3D
                    if( mV && val > MAX_DACVOLTVAL) {
                        ret = FAIL;
                        strcpy(mess, "Dac of index %d should be less than %dmV\n",
                        		serverDacIndex, val, MAX_DACVOLTVAL);
                        FILE_LOG(logWARNING,(mess));
                        val = -1;
                    }
                    else if( !mV && val >= MAX_DACVAL) {
                        ret = FAIL;
                        strcpy(mess, "Dac of index %d should be less than %d (dac value)\n",
                        		serverDacIndex, val, MAX_DACVAL);
                        FILE_LOG(logWARNING,(mess));
                        val = -1;
                    }
                    if (val >= 0) {
                        // conver to mV
                        int v = val;
                        if (!mV)
                            v = dacToVoltage(val);

                        //checkvlimit compliant
                        int lim = getVLimit();
                        if (lim!= -1 && v > lim) {
                            ret = FAIL;
                            strcpy(mess, "Dac of index %d should be less than %dmV (%d dac value)\n",
                            		serverDacIndex, lim, voltageToDac(lim));
                            FILE_LOG(logWARNING,(mess));
                            val = -1;
                        }
                    }
#endif
                    setDAC(serverDacIndex, val, mV, retval);
#ifdef EIGERD
                    if(val != -1) {
                        //changing dac changes settings to undefined
                        switch(serverDacIndex){
                        case VCMP_LL:
                        case VCMP_LR:
                        case VCMP_RL:
                        case VCMP_RR:
                        case VRF:
                        case VCP:
                            setSettings(UNDEFINED);
                            FILE_LOG(logWARNING, ("Settings has been changed "
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
                        if(mV)
                            temp = retval[1];
                        else
                            temp = retval[0];
                        if ((abs(temp-val) <= 5) || val == -1) {
                            ret = OK;
                        } else {
                            ret = FAIL;
                            sprintf(mess,"Setting dac %d : wrote %d but read %d\n", serverDacIndex, val, temp);
                            FILE_LOG(logWARNING,(mess));
                        }
                    }
                    FILE_LOG(logDEBUG5, ("Dac (%d): %d dac units and %d mV\n", serverDacIndex, retval[0], retval[1]));
    				break;
    		}
    	}
    }

    Server_SendResult(file_des, INT32, 1, retval, sizeof(retval));

    return ret;
}






int get_adc(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum dacIndex ind = 0;
	int retval = -1;

	if (receiveData(file_des, &ind, sizeof(ind), INT32) < 0)
		return printSocketReadError();

#ifdef MYTHEN3D
	functionNotImplemented();
#else
	enum ADCINDEX serverAdcIndex = 0;
	// get
	switch (ind) {
#if defined(GOTTHARD) || defined(JUNGFRAUD)
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
		FILE_LOG(logDEBUG5, ("Getting ADC %d\n", serverAdcIndex));
		retval = getADC(serverAdcIndex);
		FILE_LOG(logDEBUG5, ("ADC(%d): %d\n", retval));
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}





int write_register(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	uint32_t retval=-1;
	sprintf(mess,"write to register failed\n");

	// receive arguments
	int arg[2]={-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	int addr=arg[0];
	uint32_t val=arg[1];

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif
		retval=writeRegister(addr,val);
		if (retval!=val) {
			ret = FAIL;
			sprintf(mess,"Writing to register 0x%x failed: wrote 0x%x but read 0x%x\n", addr, val, retval);
			cprintf(RED, "Warning: %s", mess);
		}
	}
#ifdef VERBOSE
	printf("Data set to 0x%x\n",  retval);
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}





int read_register(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	uint32_t retval=-1;
	sprintf(mess,"read register failed\n");

	// receive arguments
	int arg=0;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	int addr=arg;

	// execute action
#ifdef VERBOSE
	printf("reading  register 0x%x\n", addr);
#endif
	retval=readRegister(addr);
#ifdef VERBOSE
	printf("Returned value 0x%x\n",  retval);
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int set_module(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sls_detector_module myModule;
	int retval=-1;
#ifdef EIGERD
	int myIODelay=-1;
	int myTau=-1;
	int myEV=-1;
#endif
	sprintf(mess,"set module failed\n");


#ifdef MYTHEN3D
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Set Module) is not implemented for this detector\n");
    cprintf(RED, "Warning: %s", mess);
#else


	int *myDac=NULL;
	int *myAdc=NULL;
	int *myChip = NULL;
	int *myChan = NULL;

	myDac=(int*)malloc(getNumberOfDACs()*sizeof(int));
	if (getNumberOfDACs() > 0 && myDac == NULL) {
		ret = FAIL;
		sprintf(mess,"could not allocate dacs\n");
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		myModule.dacs=myDac;
		myAdc=(int*)malloc(getNumberOfADCs()*sizeof(int));
		if (getNumberOfADCs() > 0 && myAdc == NULL) {
			ret = FAIL;
			sprintf(mess,"could not allocate adcs\n");
			cprintf(RED, "Warning: %s", mess);
		}
		else {
			myModule.adcs=myAdc;
			//no chips and chans allocated for jungfrau, too much memory
#ifdef JUNGFRAUD
			myModule.chipregs=NULL;
			myModule.chanregs=NULL;
#else
			myChip=(int*)malloc(getNumberOfChips()*sizeof(int));
			if (getNumberOfChips() > 0 && myChip == NULL) {
				ret = FAIL;
				sprintf(mess,"could not allocate chips\n");
				cprintf(RED, "Warning: %s", mess);
			}
			else {
				myModule.chipregs=myChip;
				myChan=(int*)malloc(getTotalNumberOfChannels()*sizeof(int));
				if (getTotalNumberOfChannels() > 0 && myChan == NULL) {
					ret = FAIL;
					sprintf(mess,"could not allocate chans\n");
					cprintf(RED, "Warning: %s", mess);
				}
				else {
					myModule.chanregs=myChan;
#endif
					myModule.nchip=getNumberOfChips();
					myModule.nchan=getTotalNumberOfChannels();
					myModule.ndac=getNumberOfDACs();
					myModule.nadc=getNumberOfADCs();


					// receive arguments
#ifdef VERBOSE
					printf("Setting module\n");
#endif
					n=receiveModuleGeneral(file_des, &myModule,
#ifdef JUNGFRAUD
							0	//0 is to receive partially (without trimbits etc.)
#else
							1
#endif
					);
					if (n<0) return FAIL;
#ifdef VERBOSE
		printf("module number register is %d, nchan %d, nchip %d, ndac %d, nadc %d, gain %f, offset %f\n",
				myModule.reg, myModule.nchan, myModule.nchip, myModule.ndac,  myModule.nadc, myModule.gain,myModule.offset);
#endif
#ifdef EIGERD
					n = receiveData(file_des,&myIODelay,sizeof(myIODelay),INT32);
					if (n<0) return FAIL;
					n = receiveData(file_des,&myTau,sizeof(myTau),INT32);
					if (n<0) return FAIL;
					n = receiveData(file_des,&myEV,sizeof(myEV),INT32);
					if (n<0) return FAIL;
#ifdef VERBOSE
					printf("IO Delay:%d\n",myIODelay);
					printf("Tau:%d\n",myTau);
					printf("eV:%d\n",myEV);
#endif
#endif
#ifndef JUNGFRAUD
				}
			}
#endif
		}
	}

	//check settings index
	if (ret==OK) {
#if defined(JUNGFRAUD) || defined(EIGERD)
		switch(myModule.reg){
		case GET_SETTINGS:
		case UNINITIALIZED:
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
#endif
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Setting (%d) is not implemented for this detector\n", myModule.reg);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
	}
#endif


	// execute action
	if (ret==OK) {
			if (differentClients && lockStatus) {
				ret = FAIL;
				sprintf(mess,"Detector locked by %s\n",lastClientIP);
				cprintf(RED, "Warning: %s", mess);
			}
#ifdef EIGERD
			//set dacs, trimbits and iodelay
			ret=setModule(myModule, myIODelay);
			//set threshhold
			if (myEV >= 0)
				setThresholdEnergy(myEV);
			else {
				//changes settings to undefined (loading a random trim file)
				setSettings(UNDEFINED);
				cprintf(RED,"Settings has been changed to undefined (random trim file)\n");
			}
			//rate correction
			//switch off rate correction: no value read from load calib/load settings)
			if(myTau == -1){
				if(getRateCorrectionEnable()){
					setRateCorrection(0);
					ret = FAIL;
					strcat(mess,"Cannot set Rate correction. No default tau provided. Deactivating Rate Correction\n");
					cprintf(RED, "Warning: %s", mess);
				}
			}
			//normal tau value (only if enabled)
			else{
				setDefaultSettingsTau_in_nsec(myTau);
				if (getRateCorrectionEnable()){
					int64_t retvalTau = setRateCorrection(myTau);
					if(myTau != retvalTau){
						cprintf(RED,"%s",mess);
						ret=FAIL;
					}
				}
			}
			retval = getSettings();
#else
			retval=setModule(myModule);
			if (retval != myModule.reg)
				ret = FAIL;
#endif
			if(myChip != NULL) 	free(myChip);
			if(myChan != NULL) 	free(myChan);
			if(myDac != NULL) 	free(myDac);
			if(myAdc != NULL) 	free(myAdc);
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}








int get_module(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sls_detector_module myModule;
	sprintf(mess,"get module failed\n");

#ifdef MYTHEN3D
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Get Module) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// execute action
	int *myDac=NULL;
	int *myAdc=NULL;
	int *myChip = NULL;
	int *myChan = NULL;

	myDac=(int*)malloc(getNumberOfDACs()*sizeof(int));
	if (getNumberOfDACs() > 0 && myDac == NULL) {
		ret = FAIL;
		sprintf(mess,"could not allocate dacs\n");
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		myModule.dacs=myDac;
		myAdc=(int*)malloc(getNumberOfADCs()*sizeof(int));
		if (getNumberOfADCs() > 0 && myAdc == NULL) {
			ret = FAIL;
			sprintf(mess,"could not allocate adcs\n");
			cprintf(RED, "Warning: %s", mess);
		}
		else {
			myModule.adcs=myAdc;
			//no chips and chans allocated for jungfrau, too much memory
#ifdef JUNGFRAUD
			myModule.chipregs=NULL;
			myModule.chanregs=NULL;
#else
			myChip=(int*)malloc(getNumberOfChips()*sizeof(int));
			if (getNumberOfChips() > 0 && myChip == NULL) {
				ret = FAIL;
				sprintf(mess,"could not allocate chips\n");
				cprintf(RED, "Warning: %s", mess);
			}
			else {
				myModule.chipregs=myChip;
				myChan=(int*)malloc(getTotalNumberOfChannels()*sizeof(int));
				if (getTotalNumberOfChannels() > 0 && myChan == NULL) {
					ret = FAIL;
					sprintf(mess,"could not allocate chans\n");
					cprintf(RED, "Warning: %s", mess);
				}
				else {
					myModule.chanregs=myChan;
#endif
					myModule.nchip=getNumberOfChips();
					myModule.nchan=getTotalNumberOfChannels();
					myModule.ndac=getNumberOfDACs();
					myModule.nadc=getNumberOfADCs();
					getModule(&myModule);
#ifdef VERBOSE
					printf("Returning module of register %x\n",  myModule.reg);
#endif
#ifndef JUNGFRAUD
				}
			}
#endif
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	// send module, 0 is to receive partially (without trimbits etc)
	if (ret != FAIL) {
		ret = sendModuleGeneral(file_des, &myModule, (myDetectorType == JUNGFRAU) ? 0 : 1);
	}
	if(myChip != NULL) 	free(myChip);
	if(myChan != NULL) 	free(myChan);
	if(myDac != NULL) 	free(myDac);
	if(myAdc != NULL) 	free(myAdc);

	return ret;
}






int set_settings(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int arg=-1;
	int retval=-1;
	enum detectorSettings isett=-1;
	sprintf(mess,"set settings failed\n");

#ifdef MYTHEN3D
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Set Settings) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	isett=arg;

	// execute action
	if (differentClients && lockStatus && isett!=GET_SETTINGS) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}

	switch(isett) {
	case GET_SETTINGS:
	case UNINITIALIZED:
#ifdef JUNGFRAUD
	case DYNAMICGAIN:
	case DYNAMICHG0:
	case FIXGAIN1:
	case FIXGAIN2:
	case FORCESWITCHG1:
	case FORCESWITCHG2:
		break;
	default:
		ret = FAIL;
		sprintf(mess,"Setting (%d) is not implemented for this detector.\n"
				"Options are dynamicgain, dynamichg0, fixgain1, fixgain2, "
				"forceswitchg1 and forceswitchg2.\n", isett);
		cprintf(RED, "Warning: %s", mess);
		break;
// other detectors
// #elif GOTTHARDD, MOENCHD, PROPIXD
#else
		break;
	default:
		ret = FAIL;
#ifdef EIGERD
		sprintf(mess,"Cannot set settings via SET_SETTINGS, use SET_MODULE\n");
#else
		sprintf(mess,"Setting (%d) is not implemented for this detector\n", isett);
#endif
		cprintf(RED, "Warning: %s", mess);
		break;
#endif
	}

	if (ret != FAIL) {
#ifdef VERBOSE
		printf("Changing settings to %d\n", isett);
#endif
		retval=setSettings(isett);
#ifdef VERBOSE
		printf("Settings changed to %d\n",  isett);
#endif
		if (retval == isett || isett < 0) {
			ret=OK;
		} else {
			ret = FAIL;
			sprintf(mess,"Changing settings : wrote %d but read %d\n", isett, retval);
			cprintf(RED, "Warning: %s", mess);
		}
	}
	// set to default dacs,
//# also for #elif GOTTHARDD, MOENCHD, PROPIXD
#ifdef JUNGFRAUD
		if (ret == OK && isett >= 0) {
			ret = setDefaultDacs();
			if (ret == FAIL) {
				strcpy(mess,"Could change settings, but could not set to default dacs\n");
				cprintf(RED, "Warning: %s", mess);
			}
		}
#endif
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}





int get_threshold_energy(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"get threshold energy failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Get Threshold Energy) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// execute action
#ifdef VERBOSE
	printf("Getting threshold energy \n");
#endif
	retval=getThresholdEnergy();
#ifdef VERBOSE
	printf("Threshold is %d eV\n",  retval);
#endif
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}






int start_acquisition(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"start acquisition failed\n");

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	 else {
		printf("Starting acquisition\n");
		ret=startStateMachine();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}



int stop_acquisition(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"stop acquisition failed\n");

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		printf("Stopping acquisition\n");
		ret=stopStateMachine();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}





int start_readout(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"start readout failed\n");

#ifdef JUNGFRAUD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Start Readout) is not implemented for this detector\n");
	cprintf(RED, "%s", mess);
#else

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		printf("Starting readout\n");
		ret=startReadOut();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}






int get_run_status(int file_des) {
	int ret=OK,ret1=OK;
	enum runStatus retval=ERROR;

	// execute action
#ifdef VERBOSE
	printf("Getting status\n");
#endif
	retval = getRunStatus();

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}





int start_and_read_all(int file_des) {
	ret = FAIL;
#ifdef VERBOSE
	printf("Starting and reading all frames\n");
#endif

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);


		Server_SendResult(file_des, INT32, 1 , NULL, 0);

		return ret;
	}
	startStateMachine();
	read_all(file_des);
	return ret;
}




int read_all(int file_des) {
	ret = FAIL;
	int n=0;
	sprintf(mess, "read all frame failed\n");

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);

		Server_SendResult(file_des, INT32, 1 , NULL, 0);

		return ret;
	}

	readFrame(&ret, mess);
	if(ret == FAIL)
		cprintf(RED,"%s\n",mess);
	else
		cprintf(GREEN,"%s",mess);

	Server_SendResult(file_des, INT32, 1 , NULL, 0);

	return ret;
}






int set_timer(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum timerIndex ind=0;
	int64_t tns=-1;
	int64_t retval=-1;
	sprintf(mess,"set timer failed\n");

	// receive arguments
	n = receiveData(file_des,&ind,sizeof(ind),INT32);
	if (n < 0) return printSocketReadError();

	n = receiveData(file_des,&tns,sizeof(tns),INT64);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && tns!=-1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("setting timer %d to %lld ns\n",ind,tns);
#endif
#ifdef EIGERD
		int64_t subexptime = 0;
#endif
		switch(ind) {
#ifdef JUNGFRAUD
        case STORAGE_CELL_NUMBER:
            if (tns > MAX_STORAGE_CELL_VAL) {
                ret=FAIL;
                strcpy(mess,"Max Storage cell number should not exceed 15\n");
            	cprintf(RED, "Warning: %s", mess);
                break;
            }
#endif
#ifdef EIGERD
		case SUBFRAME_ACQUISITION_TIME:
			if (tns > ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) ){
				ret=FAIL;
				strcpy(mess,"Sub Frame exposure time should not exceed 5.368 seconds\n");
				cprintf(RED, "Warning: %s", mess);
				break;
			}
			retval = setTimer(ind,tns);
			break;
		case SUBFRAME_DEADTIME:
			subexptime = setTimer(SUBFRAME_ACQUISITION_TIME, -1);
			if ((tns + subexptime) > ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) ){
				ret=FAIL;
				sprintf(mess,"Sub Frame Period should not exceed 5.368 seconds. "
						"So sub frame dead time should not exceed %lfu seconds (subexptime = %lf seconds)\n",
						((((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) - subexptime)/1E9), (subexptime/1E9));
				cprintf(RED, "Warning: %s", mess);
				break;
			}
			retval = setTimer(ind,tns);
			break;
#endif
#ifdef JUNGFRAUD
		case DELAY_AFTER_TRIGGER:
#elif MYTHEN3D
		case DELAY_AFTER_TRIGGER:
		case GATES_NUMBER:
		case PROBES_NUMBER:
		case SAMPLES_JCTB:
#endif
		case FRAME_NUMBER:
		case ACQUISITION_TIME:
		case FRAME_PERIOD:
		case CYCLES_NUMBER:
			retval = setTimer(ind,tns);
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Timer Index (%d) is not implemented for this detector\n", (int) ind);
			cprintf(RED, "%s", mess);
			break;
		}

	}

	Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));

	return ret;
}









int get_time_left(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int64_t retval=-1;
	sprintf(mess,"get timer left failed\n");


	// receive arguments
	enum timerIndex ind=0;
	n = receiveData(file_des,&ind,sizeof(ind),INT32);
	if (n < 0) return printSocketReadError();

#ifdef VERBOSE
	printf("getting time left on timer %d \n",ind);
#endif

#ifdef JUNGFRAUD
	if (ind == DELAY_AFTER_TRIGGER) {
		ret = FAIL;
		sprintf(mess,"Timer Left Index (%d) is not implemented for this release.\n", (int)ind);
		cprintf(RED, "%s", mess);
	} else {
#endif

		switch(ind) {
#ifdef EIGERD
		case MEASURED_PERIOD:
		case MEASURED_SUBPERIOD:
#elif JUNGFRAUD
		case FRAMES_FROM_START:
		case FRAMES_FROM_START_PG:
#elif MYTHEN3D
		case GATES_NUMBER:
		case PROBES_NUMBER:
		case SAMPLES_JCTB:
#endif
#ifndef EIGERD
#ifndef JUNGFRAUD
		case GATES_NUMBER:
#endif
		case FRAME_NUMBER:
		case ACQUISITION_TIME:
		case FRAME_PERIOD:
		case DELAY_AFTER_TRIGGER:
		case CYCLES_NUMBER:
		case PROGRESS:
		case ACTUAL_TIME:
		case MEASUREMENT_TIME:
#endif
			retval=getTimeLeft(ind);
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Timer Left Index (%d) is not implemented for this detector\n", (int)ind);
			cprintf(RED, "%s", mess);
			break;
		}
#ifdef VERBOSE
		printf("Time left on timer %d is %lld\n",ind, retval);
#endif

#ifdef JUNGFRAUD
	}	// end of if (ind == DELAY_AFTER_TRIGGER)
#endif

	Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));

	return ret;
}






int set_dynamic_range(int file_des) {
	int retval[2];
	int ret=OK,ret1=OK;
	int rateret=OK,rateret1=OK;
	int n=0;
	int dr=-1;
	int retval=-1;
	sprintf(mess,"set dynamic range failed\n");

	// receive arguments
	n = receiveData(file_des,&dr,sizeof(dr),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && dr>=0) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		switch(dr){
		case -1:
		case 16:
#ifdef EIGERD
		case 4:	case 8:	case 32:
#endif
		break;
		default:
			ret = FAIL;
			sprintf(mess,"Dynamic Range (%d) is not implemented for this detector\n", dr);
			cprintf(RED, "Warning: %s", mess);
		}
	}
	if(ret == OK){
#ifdef EIGERD
		int old_dr = setDynamicRange(-1);
		retval=setDynamicRange(dr);
		if (dr>=0 && retval!=dr)
			ret=FAIL;
		//look at rate correction only if dr change worked
		if((ret==OK)  && (dr!=32) && (dr!=16)   && (dr!=-1) && (getRateCorrectionEnable())){
			setRateCorrection(0);
			rateret = FAIL;
			strcpy(mess,"Switching off Rate Correction. Must be in 32 or 16 bit mode\n");
			cprintf(RED,"%s",mess);
		}else{
			//setting it if dr changed from 16 to 32 or vice versa with tau value as in rate table
			if((dr!=-1) && (old_dr != dr) && getRateCorrectionEnable() && (dr == 16 || dr == 32)){
				setRateCorrection(-1); //tau_ns will not be -1 here
				if(!getRateCorrectionEnable()){
					ret = FAIL;
					strcpy(mess,"Deactivating Rate Correction. Could not set it.\n");
					cprintf(RED,"%s",mess);
				}
			}
		}

#else
		retval = setDynamicRange(dr);
#endif
		if (dr>=0) dataBytes=calculateDataBytes();
	}
	if ((ret == OK) && dr>=0 && retval!=dr) {
		ret = FAIL;
		cprintf(RED,"%s",mess);
	}


	Server_SendResult(file_des, INT32, 1, retval, sizeof(retval));

	return ret;
}






int set_readout_flags(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum readOutFlags retval=-1;
	sprintf(mess,"set readout flags failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Set Read Out Flags) is not implemented for this detector\n");
	cprintf(RED, "%s",mess);
#else

	// receive arguments
	enum readOutFlags arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && arg!=GET_READOUT_FLAGS) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("setting readout flags  to %d\n",arg);
#endif
		switch(arg) {
		case  GET_READOUT_FLAGS:
#ifdef EIGERD
		case STORE_IN_RAM:
		case CONTINOUS_RO:
		case PARALLEL:
		case NONPARALLEL:
		case SAFE:
		case SHOW_OVERFLOW:
		case NOOVERFLOW:
#endif
			retval=setReadOutFlags(arg);
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Readout Flag Index (%d) is not implemented for this detector\n", (int)arg);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
		if (ret==OK && ((retval == -1) || ((arg!=-1) && ((retval&arg)!=arg)))){
			ret = FAIL;
			sprintf(mess,"Could not change readout flag: should be 0x%x but is 0x%x\n", arg, retval);
			cprintf(RED, "Warning: %s", mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}






int set_roi(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	strcpy(mess,"set nroi failed\n");

#ifndef GOTTHARDD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Set ROI) is not implemented for this detector\n");
	cprintf(RED, "%s",mess);
#else

	ROI* retval=0;
	int retvalsize=0;

	// receive arguments
	int nroi=-1;
	ROI arg[MAX_ROIS];
	n = receiveData(file_des,&nroi,sizeof(nroi),INT32);
	if (n < 0) return printSocketReadError();

	{
		int i;
		if(nroi!=-1){
			for(i=0;i<nroi;i++){
				n = receiveData(file_des,&arg[i].xmin,sizeof(int),INT32);
				if (n < 0) return printSocketReadError();
				n = receiveData(file_des,&arg[i].xmax,sizeof(int),INT32);
				if (n < 0) return printSocketReadError();
				n = receiveData(file_des,&arg[i].ymin,sizeof(int),INT32);
				if (n < 0) return printSocketReadError();
				n = receiveData(file_des,&arg[i].ymax,sizeof(int),INT32);
				if (n < 0) return printSocketReadError();
			}
			//n = receiveData(file_des,arg,nroi*sizeof(ROI));
			if (n != (nroi*sizeof(ROI))) {
				ret = FAIL;
				sprintf(mess,"Received wrong number of bytes for ROI\n");
				cprintf(RED, "Warning: %s", mess);
			}
		}
	}

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("Setting ROI to:");
		for( i=0;i<nroi;i++)
			printf("%d\t%d\t%d\t%d\n",arg[i].xmin,arg[i].xmax,arg[i].ymin,arg[i].ymax);
#endif
		retval=setROI(nroi,arg,&retvalsize,&ret);
		if (ret==FAIL){
			sprintf(mess,"Could not set all roi, should have set %d rois, but only set %d rois\n",nroi,retvalsize);
			cprintf(RED, "%s",mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

#ifdef GOTTHARDD
	if (ret != FAIL) {
		//retvalsize could be swapped during sendData
		int retvalsize1 = retvalsize;
		sendData(file_des, &retvalsize1, sizeof(retvalsize1), INT32);
		int i = 0;
		for(i = 0; i < retvalsize; ++i){
			n = sendData(file_des, &retval[i].xmin, sizeof(int), INT32);
			n = sendData(file_des, &retval[i].xmax, sizeof(int), INT32);
			n = sendData(file_des, &retval[i].ymin, sizeof(int), INT32);
			n = sendData(file_des, &retval[i].ymax, sizeof(int), INT32);
		}
	}
#endif

	return ret;
}





int set_speed(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum speedVariable arg=-1;
	int val=-1;
	int retval=-1;
	sprintf(mess,"set speed failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	n = receiveData(file_des,&val,sizeof(val),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus  && val>=0) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("setting speed variable %d  to %d\n",arg,val);
#endif 
		switch (arg) {
#ifdef JUNGFRAUD
		case ADC_PHASE:
			retval = adcPhase(val);
            if ((val != 100000) && (retval!=val) && (val>=0)) {
                ret=FAIL;
                sprintf(mess,"could not change set adc phase: should be %d but is %d \n", val, retval);
                cprintf(RED, "Warning: %s", mess);
            }
			break;
#endif
#ifdef EIGERD
		case CLOCK_DIVIDER:
#elif JUNGFRAUD
		case CLOCK_DIVIDER:
#elif MYTHEN3D
		case DBIT_CLOCK:
		case DBIT_PHASE:
#endif
			retval=setSpeed(arg, val);
			if ((retval!=val) && (val>=0)) {
				ret=FAIL;
				sprintf(mess,"could not change speed variable %d: should be %d but is %d \n",arg, val, retval);
				cprintf(RED, "Warning: %s", mess);
			}
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Speed Index (%d) is not implemented for this detector\n",(int) arg);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
	}

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}






int exit_server(int file_des) {
	ret = OK;
	cprintf(BG_RED,"Closing Server\n");
	Server_SendResult(file_des, INT32, 1, NULL, 0);
	return GOODBYE;
}




int lock_server(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int lock=0;
	sprintf(mess,"lock server failed\n");

	// receive arguments
	n = receiveData(file_des,&lock,sizeof(lock),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (lock>=0) {
		if (lockStatus==0 || strcmp(lastClientIP,thisClientIP)==0 || strcmp(lastClientIP,"none")==0) {
			lockStatus=lock;
			strcpy(lastClientIP,thisClientIP);
		}   else {
			ret = FAIL;
			sprintf(mess,"Server already locked by %s\n", lastClientIP);
			cprintf(RED, "Warning: %s", mess);
		}
	}
	Server_SendResult(file_des, INT32, 1, &lockStatus, sizeof(lockStatus));

	return ret;
}





int get_last_client_ip(int file_des) {
	int ret=OK,ret1=OK;

	Server_SendResult(file_des, INT32, 1, lastClientIP, sizeof(lastClientIP));

	return ret;
}




int set_port(int file_des) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int n=0;
	int p_number=-1;
	sprintf(mess,"set port failed\n");

	// receive argumets
	n = receiveData(file_des,&p_number,sizeof(p_number),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	int sd=-1;
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		 if (p_number < 1024) {
			ret = FAIL;
			sprintf(mess,"%s port Number (%d) too low\n",
					(isControlServer ? "control":"stop"), p_number);
			FILE_LOG(logWARNING, (mess));
		} else {
			FILE_LOG(logINFO, ("Setting %s port to %d\n",
					(isControlServer ? "control":"stop"), p_number));
			sd=bindSocket(p_number);
		}
	}

	Server_SendResult(file_des, INT32, 1, &p_number, sizeof(p_number));

	if (ret!=FAIL) {
		closeConnection(file_des);
		exitServer(sockfd);
		sockfd=sd;
	}

	return ret;
}




int update_client(int file_des) {
	ret = OK;
	Server_SendResult(file_des, INT32, 0, NULL, 0);
	return send_update(file_des);
}




int send_update(int file_des) {
	int n=0;	// if (n<0) should fail to stop talking to a closed client socket
	int nm=0;
	int64_t retval = 0;
	enum detectorSettings t;

	n = sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
	if (n < 0) return printSocketReadError();


	nm=setDynamicRange(GET_FLAG);
	n = sendData(file_des,&nm,sizeof(nm),INT32);
	if (n < 0) return printSocketReadError();

	dataBytes=calculateDataBytes();
	n = sendData(file_des,&dataBytes,sizeof(dataBytes),INT32);
	if (n < 0) return printSocketReadError();

	t=setSettings(GET_SETTINGS);
	n = sendData(file_des,&t,sizeof(t),INT32);
	if (n < 0) return printSocketReadError();

#ifdef EIGERD
	nm=getThresholdEnergy(GET_FLAG);
	n = sendData(file_des,&nm,sizeof(nm),INT32);
	if (n < 0) return printSocketReadError();
#endif

	retval=setTimer(FRAME_NUMBER,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();

	retval=setTimer(ACQUISITION_TIME,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();

#ifdef EIGERD
	retval=setTimer(SUBFRAME_ACQUISITION_TIME,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();

	retval=setTimer(SUBFRAME_DEADTIME,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif

	retval=setTimer(FRAME_PERIOD,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();

#ifndef EIGERD
	retval=setTimer(DELAY_AFTER_TRIGGER,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif

#if !defined(EIGERD) && !defined(JUNGFRAUD)
	retval=setTimer(GATES_NUMBER,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif

	retval=setTimer(CYCLES_NUMBER,GET_FLAG);
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();

	if (lockStatus==0) {
		strcpy(lastClientIP,thisClientIP);
	}

	return OK;
}






int configure_mac(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-100;
	sprintf(mess,"configure mac failed\n");

	// receive arguments
	char arg[6][50];
	memset(arg, 0, sizeof(arg));
	n = receiveData(file_des,arg,sizeof(arg),OTHER);
#if defined(JUNGFRAUD) || defined(EIGERD)
	int pos[3]={0,0,0};
	n = receiveData(file_des,pos,sizeof(pos),INT32);
#endif
	if (n < 0) return printSocketReadError();

	uint32_t ipad;
	uint64_t imacadd;
	uint64_t idetectormacadd;
	uint32_t udpport;
	uint32_t udpport2;
	uint32_t detipad;
	sscanf(arg[0], "%x", 	&ipad);
#ifdef VIRTUAL
	sscanf(arg[1], "%lx", 	&imacadd);
#else
	sscanf(arg[1], "%llx", 	&imacadd);
#endif
	sscanf(arg[2], "%x", 	&udpport);
#ifdef VIRTUAL
	sscanf(arg[3], "%lx",	&idetectormacadd);
#else
	sscanf(arg[3], "%llx",	&idetectormacadd);
#endif
	sscanf(arg[4], "%x",	&detipad);
	sscanf(arg[5], "%x", 	&udpport2);

	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		int i;
		//#ifdef GOTTHARD
		//printf("\ndigital_test_bit in server %d\t",digitalTestBit);
		//#endif
		printf("\nipadd %x\t",ipad);
		printf("destination ip is %d.%d.%d.%d = 0x%x \n",(ipad>>24)&0xff,(ipad>>16)&0xff,(ipad>>8)&0xff,(ipad)&0xff,ipad);
		printf("macad:%llx\n",imacadd);
		for (i=0;i<6;i++)
			printf("mac adress %d is 0x%x \n",6-i,(unsigned int)(((imacadd>>(8*i))&0xFF)));
		printf("udp port:0x%x\n",udpport);
		printf("detector macad:%llx\n",idetectormacadd);
		for (i=0;i<6;i++)
			printf("detector mac adress %d is 0x%x \n",6-i,(unsigned int)(((idetectormacadd>>(8*i))&0xFF)));
		printf("detipad %x\n",detipad);
		printf("udp port2:0x%x\n",udpport2);
		printf("\n");
		printf("Configuring MAC at port %x\n", udpport);

#if defined(JUNGFRAUD) || defined(EIGERD)
		printf("Position: [%d,%d,%d]\n", pos[0],pos[1],pos[2]);
#endif
#endif
		if(getRunStatus() == RUNNING){
			ret = stopStateMachine();
		}
			if(ret==FAIL) {
				sprintf(mess,"Could not stop detector acquisition to configure mac\n");
				cprintf(RED, "Warning: %s", mess);
			}
			else {
#ifdef EIGERD
			    // change mac to hardware mac, (for 1 gbe) change ip to hardware ip
			    if (idetectormacadd != getDetectorMAC()){
			        printf("*************************************************\n");
			        printf("WARNING: actual detector mac address %llx does not match "
			        		"the one from client %llx\n",
							(long long unsigned int)getDetectorMAC(),
							(long long unsigned int)idetectormacadd);
			        idetectormacadd = getDetectorMAC();
			        printf("WARNING: Matched detectormac to the hardware mac now\n");
			        printf("*************************************************\n");
			    }

			    // always remember the ip sent from the client (could be for 10g(if not dhcp))
			    if (detipad != getDetectorIP())
			        dhcpipad = detipad;

			    //only for 1Gbe
			    if(!enableTenGigabitEthernet(-1)){
			        printf("*************************************************\n");
                    printf("WARNING: Using DHCP IP for Configuring MAC\n");
                    printf("*************************************************\n");
                    detipad = getDetectorIP();
			    } else
			        detipad = dhcpipad;
#endif
				retval=configureMAC(ipad,imacadd,idetectormacadd,detipad,udpport,udpport2,0);	//digitalTestBit);
				if(retval==-1) {
					ret = FAIL;
					sprintf(mess,"Configure Mac failed\n");
					cprintf(RED, "Warning: %s", mess);
				}
				else {
					printf("Configure MAC successful\n");
#if defined(JUNGFRAUD) || defined(EIGERD)
					ret = setDetectorPosition(pos);
					if (ret == FAIL) {
						sprintf(mess,"could not set detector position\n");
						cprintf(RED, "Warning: %s", mess);
					}
#endif
				}
#ifdef VERBOSE
				printf("Configured MAC with retval %d\n",  retval);
#endif
			}

	}

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

#ifdef EIGERD
	if (ret != FAIL) {
		char arg[2][50];
		memset(arg, 0, sizeof(arg));
		sprintf(arg[0],"%llx",(long long unsigned int)idetectormacadd);
		sprintf(arg[1],"%x",detipad);
		n += sendData(file_des,arg,sizeof(arg),OTHER);
	}
#endif

	return ret;
}





int load_image(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"Loading image failed\n");

#ifndef GOTTHARDD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Load Image) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	enum imageType index=0;
	char ImageVals[dataBytes] = {0};
	n = receiveData(file_des,&index,sizeof(index),INT32);
	if (n < 0) return printSocketReadError();

	n = receiveData(file_des,ImageVals,dataBytes,OTHER);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		switch (index) {
		case DARK_IMAGE :
#ifdef VERBOSE
			printf("Loading Dark image\n");
#endif
		case GAIN_IMAGE :
#ifdef VERBOSE
			printf("Loading Gain image\n");
#endif
			retval=loadImage(index,ImageVals);
			if (retval==-1) {
				ret = FAIL;
				cprintf(RED, "Warning: %s", mess);
			}
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Load Image Index (%d) is not implemented for this detector\n", (int)index);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}






int read_counter_block(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	char CounterVals[dataBytes];
	memset(CounterVals, 0, dataBytes);
	sprintf(mess,"Read counter block failed\n");

#ifndef GOTTHARDD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Read Counter Block) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int startACQ=-1;
	n = receiveData(file_des,&startACQ,sizeof(startACQ),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret=readCounterBlock(startACQ,CounterVals);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
#ifdef VERBOSE
		int i;
		for(i=0;i<6;i++)
			printf("%d:%d\t",i,CounterVals[i]);
#endif
	}
#endif

	Server_SendResult(file_des, OTHER, 1, CounterVals, dataBytes);

	return ret;
}





int reset_counter_block(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"Reset counter block failed\n");

#ifndef GOTTHARDD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Reset Counter Block) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int startACQ=-1;
	n = receiveData(file_des,&startACQ,sizeof(startACQ),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret=resetCounterBlock(startACQ);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}





int calibrate_pedestal(int file_des){
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"calibrate pedestal failed\n");


#ifndef GOTTHARDD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Calibrate Pedestal) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int frames=-1;
	n = receiveData(file_des,&frames,sizeof(frames),INT32);
	if (n < 0) return printSocketReadError();


	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret=calibratePedestal(frames);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}








int enable_ten_giga(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"Enabling/disabling 10GbE failed\n");

	// execute action
#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Enable 10 GbE) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && arg!=-1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
	printf("Enabling/Disabling 10Gbe :%d \n",arg);
#endif
		retval=enableTenGigabitEthernet(arg);
		if((arg != -1) && (retval != arg)) {
			ret=FAIL;
			cprintf(RED, "Warning: %s", mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int set_all_trimbits(int file_des){
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"setting all trimbits failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Set All Trimbits) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();


	// execute action
	if (differentClients && lockStatus && arg!=-1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("setting all trimbits to %d\n",arg);
#endif
		if(arg < -1){
			ret = FAIL;
			strcpy(mess,"Cant set trimbits to this value\n");
			cprintf(RED, "Warning: %s", mess);
		}else {
			if(arg >= 0){
				ret = setAllTrimbits(arg);
				//changes settings to undefined
				setSettings(UNDEFINED);
				cprintf(RED,"Settings has been changed to undefined (change all trimbits)\n");
			}
			retval = getAllTrimbits();
			if (arg!=-1 && arg!=retval) {
				ret=FAIL;
				sprintf(mess,"Could not set all trimbits: should be %d but is %d\n", arg, retval);
				cprintf(RED, "Warning: %s", mess);
			}
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}


int set_ctb_pattern(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    sprintf(mess,"Could not set pattern\n");

#ifndef MYTHEN3D
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

    ret = FAIL;
    sprintf(mess,"Function (Set CTB Pattern) is not implemented for this detector\n");
    cprintf(RED, "Error: %s", mess);

    Server_SendResult(file_des, INT32, 0, NULL, 0);// make sure client doesnt expect a retval if not jctb

    return ret;

#else

    int retval=-1;
    int mode = -1;
    int addr = -1, level = -1, nl = -1, start = -1, stop = -1;
    uint64_t word = -1,retval64 = -1, t = -1;

    n = receiveDataOnly(file_des, &mode, sizeof(mode));
    printf("pattern mode is %d\n",mode);

    switch (mode) {

    case 0: //sets word
        n = receiveDataOnly(file_des,&addr,sizeof(addr));
        n = receiveDataOnly(file_des,&word,sizeof(word));
        ret=OK;

        printf("pattern addr is %d %llx\n",addr, word);
        switch (addr) {
        case -1:
            retval64=writePatternIOControl(word);
            break;
        case -2:
            retval64=writePatternClkControl(word);
            break;
        default:
            retval64=writePatternWord(addr,word);
        };


        //write word;
        //@param addr address of the word, -1 is I/O control register,  -2 is clk control register
        //@param word 64bit word to be written, -1 gets

    	Server_SendResult(file_des, INT32, 0, &retval64, sizeof(retval64));

        break;

        case 1: //pattern loop
            // printf("loop\n");
            n = receiveDataOnly(file_des,&level,sizeof(level));
            n = receiveDataOnly(file_des,&start,sizeof(start));
            n = receiveDataOnly(file_des,&stop,sizeof(stop));
            n = receiveDataOnly(file_des,&nl,sizeof(nl));



            //       printf("level %d start %x stop %x nl %d\n",level, start, stop, nl);
            /** Sets the pattern or loop limits in the CTB
          @param level -1 complete pattern, 0,1,2, loop level
          @param start start address if >=0
          @param stop stop address if >=0
          @param n number of loops (if level >=0)
          @returns OK/FAIL
             */
            ret=setPatternLoop(level, &start, &stop, &nl);

            Server_SendResult(file_des, INT32, 0, retval, sizeof(retval));
/*                n += sendDataOnly(file_des,&start,sizeof(start));
                n += sendDataOnly(file_des,&stop,sizeof(stop));
                n += sendDataOnly(file_des,&nl,sizeof(nl));
   */
            break;



        case 2: //wait address
            printf("wait\n");
            n = receiveDataOnly(file_des,&level,sizeof(level));
            n = receiveDataOnly(file_des,&addr,sizeof(addr));



            /** Sets the wait address in the CTB
          @param level  0,1,2, wait level
          @param addr wait address, -1 gets
          @returns actual value
             */
            printf("wait addr %d %x\n",level, addr);
            retval=setPatternWaitAddress(level,addr);
            printf("ret: wait addr %d %x\n",level, retval);
            ret=OK;

            Server_SendResult(file_des, INT32, 0, &retval, sizeof(retval));

            break;


        case 3: //wait time
            printf("wait time\n");
            n = receiveDataOnly(file_des,&level,sizeof(level));
            n = receiveDataOnly(file_des,&t,sizeof(t));


            /** Sets the wait time in the CTB
          @param level  0,1,2, wait level
          @param t wait time, -1 gets
          @returns actual value
             */

            ret=OK;

            retval64=setPatternWaitTime(level,t);

            Server_SendResult(file_des, INT32, 0, &retval64, sizeof(retval64));
            break;



        case 4:
            n = receiveDataOnly(file_des,pat,sizeof(pat));
            for (addr=0; addr<1024; addr++)
                writePatternWord(addr,word);
            ret=OK;
            retval=0;

            Server_SendResult(file_des, INT32, 0, &retval64, sizeof(retval64));

            break;



        default:
            ret=FAIL;
            printf(mess);
            sprintf(mess,"%s - wrong mode %d\n",mess, mode);

            Server_SendResult(file_des, INT32, 0, NULL, 0);
    }


    return ret;
#endif
}



int write_adc_register(int file_des) {
	int ret=OK, ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"write to adc register failed\n");

#ifndef JUNGFRAUD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Write ADC Register) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg[2]={-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	int addr=arg[0];
	int val=arg[1];

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
	printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif
	setAdc(addr,val);
#ifdef VERBOSE
	printf("Data set to 0x%x\n",  retval);
#endif
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int set_counter_bit(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"set counter bit failed \n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	strcpy(mess,"Function (Set Counter Bit) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && arg!=-1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("Getting/Setting/Resetting counter bit :%d \n",arg);
#endif
		retval=setCounterBit(arg);
		if((arg != -1) && (retval != arg)) {
			ret=FAIL;
			cprintf(RED, "Warning: %s", mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int pulse_pixel(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"pulse pixel failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	strcpy(mess,"Function (Pulse Pixel) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg[3]={-1,-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret=pulsePixel(arg[0],arg[1],arg[2]);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}




int pulse_pixel_and_move(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"pulse pixel and move failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	strcpy(mess,"Function (Pulse Pixel and Move) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg[3]={-1,-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret=pulsePixelNMove(arg[0],arg[1],arg[2]);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}






int pulse_chip(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"pulse chip failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	strcpy(mess,"Function (Pulse Chip) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg = -1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret=pulseChip(arg);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}





int set_rate_correct(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"Set rate correct failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Rate Correction) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int64_t tau_ns=-1;
	n = receiveData(file_des,&tau_ns,sizeof(tau_ns),INT64);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		printf("Setting rate correction to %lld ns\n",(long long int)tau_ns);
		//set rate
		//wrong bit mode
		if((setDynamicRange(-1)!=32) && (setDynamicRange(-1)!=16) && (tau_ns!=0)){
			ret=FAIL;
			strcpy(mess,"Rate correction Deactivated, must be in 32 or 16 bit mode\n");
			cprintf(RED, "Warning: %s", mess);
		}
		//16 or 32 bit mode
		else{
			if(tau_ns < 0)
				tau_ns = getDefaultSettingsTau_in_nsec();
			else if(tau_ns > 0){
				//changing tau to a user defined value changes settings to undefined
				setSettings(UNDEFINED);
				cprintf(RED,"Settings has been changed to undefined (tau changed)\n");
			}

			int64_t retval = setRateCorrection(tau_ns);
			if(tau_ns != retval){
				ret=FAIL;
				cprintf(RED, "Warning: %s", mess);
			}
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}





int get_rate_correct(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int64_t retval=-1;
	sprintf(mess,"Get Rate correct failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Get Rate Correction) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else


	// execute action
	retval = getCurrentTau();
	printf("Getting rate correction %lld\n",(long long int)retval);

#endif

	Server_SendResult(file_des, INT64, 1, &retval, sizeof(retval));

	return ret;
}





int set_network_parameter(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"set network parameter failed\n");

#if !defined(EIGERD) && !defined(JUNGFRAUD)
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function(Set Network Parmaeter) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	enum NETWORKINDEX index;

	// receive arguments
	enum networkParameter mode=0;
	int value=-1;
	n = receiveData(file_des,&mode,sizeof(mode),INT32);
	if (n < 0) return printSocketReadError();

	n = receiveData(file_des,&value,sizeof(value),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && value >= 0) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("setting network parameter mode %d to %d\n",(int)mode,value);
#endif
		switch (mode) {

#ifdef EIGERD
        case FLOW_CONTROL_10G:
            index = FLOWCTRL_10G;
            break;
		case DETECTOR_TXN_DELAY_LEFT:
			index = TXN_LEFT;
			break;
		case DETECTOR_TXN_DELAY_RIGHT:
			index = TXN_RIGHT;
			break;
#endif
		case DETECTOR_TXN_DELAY_FRAME:
			index = TXN_FRAME;
#ifdef JUNGFRAUD
			if (value > MAX_TIMESLOT_VAL)	{
			    ret=FAIL;
			    sprintf(mess,"Transmission delay %d should be in range: 0 - %d\n", value, MAX_TIMESLOT_VAL);
			    cprintf(RED, "Warning: %s", mess);
			}
#endif
			break;
		default:
			ret=FAIL;
			sprintf(mess,"Network Parameter Index (%d) is not implemented for this detector\n",(int) mode);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
		if (ret==OK) {
			retval=setNetworkParameter(index, value);
			if ((retval!=value) && (value>=0)) {
				ret=FAIL;
				sprintf(mess,"could not change network parameter mode %d: should be %d but is %d \n",index, value, retval);
				cprintf(RED, "Warning: %s", mess);
			}
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}






int program_fpga(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"program FPGA failed\n");

#ifndef JUNGFRAUD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Program FPGA) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else
	if (!debugflag) {
		//to receive any arguments
		while (n > 0)
			n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
		ret=FAIL;
		sprintf(mess,"FPGA cannot be programmed in this mode. "
				"Restart on-board detector server with -update for update mode to continue.\n");
		cprintf(RED, "Warning: %s", mess);
	}

	else {
		printf("Programming FPGA...");
		size_t filesize = 0;
		size_t totalsize = 0;
		size_t unitprogramsize = 0;
		char* fpgasrc = NULL;
		FILE* fp = NULL;

		// receive arguments - filesize
		n = receiveData(file_des,&filesize,sizeof(filesize),INT32);
		if (n < 0) return printSocketReadError();
		totalsize = filesize;
#ifdef VERY_VERBOSE
		printf("\n\n Total size is:%d\n",totalsize);
#endif

		// execute action
		if (differentClients && lockStatus) {
			ret = FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
			cprintf(RED, "Warning: %s", mess);
		}
		else {
			//opening file pointer to flash and telling FPGA to not touch flash
			if(startWritingFPGAprogram(&fp) != OK) {
				ret=FAIL;
				sprintf(mess,"Could not write to flash. Error at startup.\n");
				cprintf(RED,"%s",mess);
			}

			//---------------- first ret ----------------
			// ret could be swapped during sendData
			ret1 = ret;
			// send ok / fail
			n = sendData(file_des,&ret1,sizeof(ret),INT32);
			// send return argument
			if (ret==FAIL) {
				n += sendData(file_des,mess,sizeof(mess),OTHER);
			}
			//---------------- first ret ----------------

			if(ret!=FAIL) {
				//erasing flash
				eraseFlash();
				fpgasrc = (char*)malloc(MAX_FPGAPROGRAMSIZE);
			}

			//writing to flash part by part
			while(ret != FAIL && filesize){

				unitprogramsize = MAX_FPGAPROGRAMSIZE;  //2mb
				if(unitprogramsize > filesize) //less than 2mb
					unitprogramsize = filesize;
#ifdef VERY_VERBOSE
				printf("unit size to receive is:%d\n",unitprogramsize);
				printf("filesize:%d currentpointer:%d\n",filesize,currentPointer);
#endif

				//receive
				n = receiveData(file_des,fpgasrc,unitprogramsize,OTHER);
				if (n < 0) return printSocketReadError();

				if(!(unitprogramsize - filesize)){
					fpgasrc[unitprogramsize]='\0';
					filesize-=unitprogramsize;
					unitprogramsize++;
				}else
					filesize-=unitprogramsize;

				ret = writeFPGAProgram(fpgasrc,unitprogramsize,fp);

				//---------------- middle rets ----------------
				// ret could be swapped during sendData
				ret1 = ret;
				// send ok / fail
				n = sendData(file_des,&ret1,sizeof(ret),INT32);
				// send return argument
				if (ret==FAIL) {
					n += sendData(file_des,mess,sizeof(mess),OTHER);
					cprintf(RED,"Failure: Breaking out of program receiving\n");
				}
				//---------------- middle rets ----------------

				if(ret != FAIL){
					//print progress
					printf("Writing to Flash:%d%%\r",(int) (((double)(totalsize-filesize)/totalsize)*100) );
					fflush(stdout);
				}
			}

			printf("\n");

			//closing file pointer to flash and informing FPGA
			stopWritingFPGAprogram(fp);

			//free resources
			if(fpgasrc != NULL)
				free(fpgasrc);
			if(fp!=NULL)
				fclose(fp);
#ifdef VERY_VERBOSE
			printf("Done with program receiving command\n");
#endif

			if (isControlServer) {
				basictests(debugflag);
				initControlServer();
			}
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}





int reset_fpga(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"Reset FPGA unsuccessful\n");

#ifndef JUNGFRAUD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Reset FPGA) is not implemented for this detector\n");	cprintf(RED, "%s", mess);
#else

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
	    if (isControlServer) {
	        basictests(debugflag);	// mapping of control server at lease
#ifdef JUNGFRAUD
	    if (debugflag != PROGRAMMING_MODE)
#endif
	    	initControlServer();
	    }
	    else initStopServer(); //remapping of stop server
		ret = FORCE_UPDATE;
	}
#endif

	Server_SendResult(file_des, INT32, 1, NULL, 0);

	return ret;
}



int power_chip(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"power chip failed\n");

#ifndef JUNGFRAUD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Power Chip) is not implemented for this detector\n");
	cprintf(RED, "%s", mess);
#else

	// receive arguments
	int arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && arg!=-1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
	printf("Power chip to %d\n", arg);
#endif
		retval=powerChip(arg);

#ifdef VERBOSE
		printf("Chip powered: %d\n",retval);
#endif
		if (retval==arg || arg<0) {
			ret=OK;
		} else {
			ret=FAIL;
			if(setTemperatureEvent(-1) == 1)
			    sprintf(mess,"Powering chip failed due to over-temperature event. Clear event & power chip again. Wrote %d, read %d \n", arg, retval);
			else
			    sprintf(mess,"Powering chip failed, wrote %d but read %d\n", arg, retval);
			cprintf(RED, "Warning: %s", mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int set_activate(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"Activate/Deactivate failed\n");

#ifndef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Set Activate) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && arg!=-1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("Setting activate mode of detector to %d\n",arg);
#endif
		retval=activate(arg);
		if ((retval!=arg) && (arg!=-1)) {
			ret=FAIL;
			sprintf(mess,"Could not set activate mode to %d, is set to %d\n",arg, retval);
			cprintf(RED, "Warning: %s", mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

	return ret;
}




int prepare_acquisition(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	strcpy(mess,"prepare acquisition failed\n");

#if !defined(GOTTHARDD) && !defined(EIGERD)
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Prepare Acquisition) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		ret = prepareAcquisition();
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1 , NULL, 0);

	return ret;
}





int threshold_temp(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    int retval=-1;
    sprintf(mess,"could not set/get threshold temperature\n");

#ifndef JUNGFRAUD
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Threshold Temp) is not implemented for this detector\n");
    cprintf(RED, "%s", mess);
#else
    int arg=-1;
    int val=-1;

    // receive arguments
    n = receiveData(file_des,&arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();

    val=arg;
    if (val > MAX_THRESHOLD_TEMP_VAL)   {
        ret=FAIL;
        sprintf(mess,"Threshold Temp %d should be in range: 0 - %d\n", val, MAX_THRESHOLD_TEMP_VAL);
        cprintf(RED, "Warning: %s", mess);
    }


    if (ret==OK) {
#ifdef VERBOSE
    printf("Setting Threshold Temperature to  %d\n", val);
#endif
        retval=setThresholdTemperature(val);
    }
#ifdef VERBOSE
    printf("Threshold temperature is %d\n",  retval);
#endif
#endif

    Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

    return ret;
}



int temp_control(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    int retval=-1;
    sprintf(mess,"could not set/get temperature control\n");

#ifndef JUNGFRAUD
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Temperature control) is not implemented for this detector\n");
    cprintf(RED, "%s", mess);
#else
    int arg=-1;
    int val=-1;

    // receive arguments
    n = receiveData(file_des,&arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();
    val=arg;

    if (ret==OK) {
#ifdef VERBOSE
    printf("Setting Temperature control to  %d\n", val);
#endif
        retval=setTemperatureControl(val);
    }
#ifdef VERBOSE
    printf("Temperature control is %d\n",  retval);
#endif
#endif

    Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

    return ret;
}




int temp_event(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    int retval=-1;
    sprintf(mess,"could not set/get temperature event\n");

#ifndef JUNGFRAUD
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Temperature Event) is not implemented for this detector\n");
    cprintf(RED, "%s", mess);
#else
    int arg=-1;
    int val=-1;

    // receive arguments
    n = receiveData(file_des,&arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();
    val=arg;

    if (ret==OK) {
#ifdef VERBOSE
    printf("Setting Temperature Event to  %d\n", val);
#endif
        retval=setTemperatureEvent(val);
    }
#ifdef VERBOSE
    printf("Temperature Event is %d\n",  retval);
#endif

#endif

    Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

    return ret;
}





int auto_comp_disable(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    int retval=-1;
    sprintf(mess,"auto comp disable failed\n");

#ifndef JUNGFRAUD
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Auto Comp Disable) is not implemented for this detector\n");
    cprintf(RED, "%s", mess);
#else

    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Auto Comp Disable) is not yet implemented for this detector\n");
    cprintf(RED, "%s", mess);

    // receive arguments
    int arg=-1;
    n = receiveData(file_des,&arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();

    // execute action
    if (differentClients && lockStatus && arg!=-1) {
        ret = FAIL;
        sprintf(mess,"Detector locked by %s\n",lastClientIP);
        cprintf(RED, "Warning: %s", mess);
    }
    else {
#ifdef VERBOSE
    printf("Auto Comp Disable to %d\n", arg);
#endif
        retval=autoCompDisable(arg);

#ifdef VERBOSE
        printf("Auto comp disable set to: %d\n",retval);
#endif
        if (retval==arg || arg<0) {
            ret=OK;
        } else {
            ret=FAIL;
            sprintf(mess,"Atuo Comp Disable failed, wrote %d but read %d\n", arg, retval);
            cprintf(RED, "Warning: %s", mess);
        }
    }
#endif

    Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

    return ret;
}





int storage_cell_start(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    int retval=-1;
    sprintf(mess,"storage cell start failed\n");

#ifndef JUNGFRAUD
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Storage cell start) is not implemented for this detector\n");
    cprintf(RED, "%s", mess);
#else

    // receive arguments
    int arg=-1;
    n = receiveData(file_des,&arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();

    // execute action
    if (differentClients && lockStatus && arg!=-1) {
        ret = FAIL;
        sprintf(mess,"Detector locked by %s\n",lastClientIP);
        cprintf(RED, "Warning: %s", mess);
    }
    else if (arg > MAX_STORAGE_CELL_VAL) {
        ret=FAIL;
        strcpy(mess,"Max Storage cell number should not exceed 15\n");
        cprintf(RED, "Warning: %s", mess);
    } else {
#ifdef VERBOSE
    printf("Storage cell start to %d\n", arg);
#endif
        retval=selectStoragecellStart(arg);

#ifdef VERBOSE
        printf("Storage cell start: %d\n",retval);
#endif
        if (retval==arg || arg<0) {
            ret=OK;
        } else {
            sprintf(mess,"Storage cell start select failed, wrote %d but read %d\n", arg, retval);
            cprintf(RED, "Warning: %s", mess);
        }
    }
#endif

    Server_SendResult(file_des, INT32, 1, &retval, sizeof(retval));

    return ret;
}




int check_version(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"check version failed\n");

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(GOTTHARD)
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret=FAIL;
	sprintf(mess,"Function (Check Version Compatibility) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int64_t arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT64);
	if (n < 0) return printSocketReadError();

	// execute action

	// check software- firmware compatibility and basic tests
	if (isControlServer) {
#ifdef VERBOSE
		printf("Checking software-firmware compatibility and basic test result\n");
#endif
		// check if firmware check is done
		if (!isFirmwareCheckDone()) {
			usleep(3 * 1000 * 1000);
			if (!isFirmwareCheckDone()) {
				ret = FAIL;
				strcpy(mess,"Firmware Software Compatibility Check (Server Initialization) "
						"still not done done in server. Unexpected.\n");
				cprintf(RED, "Warning: %s", mess);
			}
		}
		// check firmware check result
		if (ret == OK) {
			char* firmware_message = NULL;
			if (getFirmwareCheckResult(&firmware_message) == FAIL) {
				ret = FAIL;
				strcpy(mess, firmware_message);
				cprintf(RED, "Warning: %s", mess);
			}
		}
	}

	if (ret == OK) {
#ifdef VERBOSE
		printf("Checking versioning compatibility with value %d\n",arg);
#endif
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
			cprintf(RED, "Warning: %s", mess);
		}

		// old software
		else if (client_requiredVersion > det_version) {
			ret = FAIL;
			sprintf(mess,"Detector SW Version: (0x%llx). "
					"Client's detector SW API Version: (0x%llx). "
					"Incompatible, update detector software!\n",
					(long long int)det_version, (long long int)client_requiredVersion);
			cprintf(RED, "Warning: %s", mess);
		}
	}
#endif

	Server_SendResult(file_des, INT32, 1 , NULL, 0);
	return ret;
}




int software_trigger(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"software trigger failed\n");

#ifndef EIGERD
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Software Trigger) is not implemented for this detector\n");
    cprintf(RED, "%s", mess);
#else

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	 else {
		printf("Software Trigger\n");
		ret=softwareTrigger();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif

	Server_SendResult(file_des, INT32, 1 , NULL, 0);

	return ret;
}

