
#include "sls_detector_defs.h"
#include "slsDetectorServer_funcs.h"
#include "slsDetectorFunctionList.h"
#include "communication_funcs.h"
#include "slsDetectorServer_defs.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


// Global variables
extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;

//defined in the detector specific Makefile
#ifdef MYTHEND
const enum detectorType myDetectorType=MYTHEN;
#elif GOTTHARDD
const enum detectorType myDetectorType=GOTTHARD;
#elif EIGERD
const enum detectorType myDetectorType=EIGER;
#elif PICASSOD
const enum detectorType myDetectorType=PICASSO;
#elif MOENCHD
const enum detectorType myDetectorType=MOENCH;
#elif JUNGFRAUD
const enum detectorType myDetectorType=JUNGFRAU;
#elif MYTHEN3D
const enum detectorType myDetectorType=MYTHEN3;
#else
const enum detectorType myDetectorType=GENERIC;
#endif

int sockfd;		// (updated in slsDetectorServer) as extern
int (*flist[NUM_DET_FUNCTIONS])(int);
char mess[MAX_STR_LENGTH];
int dataBytes = 10;
int isControlServer = 0;
int debugflag = 0;
#ifdef EIGERD
uint32_t dhcpipad = 0;
#endif

/* initialization functions */

int printSocketReadError() {
	cprintf(BG_RED, "Error reading from socket. Possible socket crash\n");
	return FAIL;
}

void setModeFlag(int flag) {
    debugflag = flag;
}

void basictests() {
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	checkFirmwareCompatibility(debugflag);
#endif
}


void init_detector(int controlserver) {
#ifdef VIRTUAL
	printf("This is a VIRTUAL detector\n");
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (controlserver) {
	    isControlServer = 1;
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
#endif
	strcpy(mess,"dummy message");
	strcpy(lastClientIP,"none");
	strcpy(thisClientIP,"none1");
	lockStatus=0;
}


int decode_function(int file_des) {
	int fnum,n;
	int ret=FAIL;
#ifdef VERBOSE
	printf( "\nreceive data\n");
#endif
	n = receiveData(file_des,&fnum,sizeof(fnum),INT32);
	if (n <= 0) {
#ifdef VERBOSE
		printf("ERROR reading from socket %d, %d %d (%s)\n", n, fnum, file_des, getFunctionName((enum detFuncs)fnum));
#endif
		return FAIL;
	}
#ifdef VERBOSE
	else
		printf("size of data received %d\n",n);
#endif

#ifdef VERBOSE
	printf(" calling function fnum=%d, (%s) located at 0x%x\n", fnum,  getFunctionName((enum detFuncs)fnum), (unsigned int)flist[fnum]);
#endif
#ifdef JUNGFRAUD
	if ((debugflag == PROGRAMMING_MODE) &&
			((fnum != F_PROGRAM_FPGA) && (fnum != F_GET_DETECTOR_TYPE) &&
					(fnum != F_RESET_FPGA) && (fnum != F_UPDATE_CLIENT) && (fnum != F_CHECK_VERSION))) {
		sprintf(mess,"This Function %s cannot be executed. ", getFunctionName((enum detFuncs)fnum));
		ret=(M_nofuncMode)(file_des);
	} else
#endif
	if (fnum<0 || fnum>=NUM_DET_FUNCTIONS) {
		cprintf(BG_RED,"Unknown function enum %d\n", fnum);
		ret=(M_nofunc)(file_des);
	}else
		ret=(*flist[fnum])(file_des);
	if (ret == FAIL)
		cprintf(RED, "Error executing the function = %d (%s)\n", fnum, getFunctionName((enum detFuncs)fnum));
	return ret;
}


const char* getFunctionName(enum detFuncs func) {
	switch (func) {
	case F_EXEC_COMMAND:					return "F_EXEC_COMMAND";
	case F_GET_ERROR:						return "F_GET_ERROR";
	case F_GET_DETECTOR_TYPE:				return "F_GET_DETECTOR_TYPE";
	case F_SET_NUMBER_OF_MODULES:			return "F_SET_NUMBER_OF_MODULES";
	case F_GET_MAX_NUMBER_OF_MODULES:		return "F_GET_MAX_NUMBER_OF_MODULES";
	case F_SET_EXTERNAL_SIGNAL_FLAG:		return "F_SET_EXTERNAL_SIGNAL_FLAG";
	case F_SET_EXTERNAL_COMMUNICATION_MODE:	return "F_SET_EXTERNAL_COMMUNICATION_MODE";
	case F_GET_ID:							return "F_GET_ID";
	case F_DIGITAL_TEST:					return "F_DIGITAL_TEST";
	case F_ANALOG_TEST:						return "F_ANALOG_TEST";
	case F_ENABLE_ANALOG_OUT:				return "F_ENABLE_ANALOG_OUT";
	case F_CALIBRATION_PULSE:				return "F_CALIBRATION_PULSE";
	case F_SET_DAC:							return "F_SET_DAC";
	case F_GET_ADC:							return "F_GET_ADC";
	case F_WRITE_REGISTER:					return "F_WRITE_REGISTER";
	case F_READ_REGISTER:					return "F_READ_REGISTER";
	case F_WRITE_MEMORY:					return "F_WRITE_MEMORY";
	case F_READ_MEMORY:						return "F_READ_MEMORY";
	case F_SET_CHANNEL:						return "F_SET_CHANNEL";
	case F_GET_CHANNEL:						return "F_GET_CHANNEL";
	case F_SET_ALL_CHANNELS:				return "F_SET_ALL_CHANNELS";
	case F_SET_CHIP:						return "F_SET_CHIP";
	case F_GET_CHIP:						return "F_GET_CHIP";
	case F_SET_ALL_CHIPS:					return "F_SET_ALL_CHIPS";
	case F_SET_MODULE:						return "F_SET_MODULE";
	case F_GET_MODULE:						return "F_GET_MODULE";
	case F_SET_ALL_MODULES:					return "F_SET_ALL_MODULES";
	case F_SET_SETTINGS:					return "F_SET_SETTINGS";
	case F_GET_THRESHOLD_ENERGY:			return "F_GET_THRESHOLD_ENERGY";
	case F_SET_THRESHOLD_ENERGY:			return "F_SET_THRESHOLD_ENERGY";
	case F_START_ACQUISITION:				return "F_START_ACQUISITION";
	case F_STOP_ACQUISITION:				return "F_STOP_ACQUISITION";
	case F_START_READOUT:					return "F_START_READOUT";
	case F_GET_RUN_STATUS:					return "F_GET_RUN_STATUS";
	case F_START_AND_READ_ALL:				return "F_START_AND_READ_ALL";
	case F_READ_FRAME:						return "F_READ_FRAME";
	case F_READ_ALL:						return "F_READ_ALL";
	case F_SET_TIMER:						return "F_SET_TIMER";
	case F_GET_TIME_LEFT:					return "F_GET_TIME_LEFT";
	case F_SET_DYNAMIC_RANGE:				return "F_SET_DYNAMIC_RANGE";
	case F_SET_READOUT_FLAGS:				return "F_SET_READOUT_FLAGS";
	case F_SET_ROI:							return "F_SET_ROI";
	case F_SET_SPEED:						return "F_SET_SPEED";
	case F_EXECUTE_TRIMMING:				return "F_EXECUTE_TRIMMING";
	case F_EXIT_SERVER:						return "F_EXIT_SERVER";
	case F_LOCK_SERVER:						return "F_LOCK_SERVER";
	case F_GET_LAST_CLIENT_IP:				return "F_GET_LAST_CLIENT_IP";
	case F_SET_PORT:						return "F_SET_PORT";
	case F_UPDATE_CLIENT:					return "F_UPDATE_CLIENT";
	case F_CONFIGURE_MAC:					return "F_CONFIGURE_MAC";
	case F_LOAD_IMAGE:						return "F_LOAD_IMAGE";
	case F_SET_MASTER:						return "F_SET_MASTER";
	case F_SET_SYNCHRONIZATION_MODE:		return "F_SET_SYNCHRONIZATION_MODE";
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
	case F_CLEANUP_ACQUISITION:				return "F_CLEANUP_ACQUISITION";
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
	flist[F_GET_ERROR]							= &get_error;
	flist[F_GET_DETECTOR_TYPE]					= &get_detector_type;
	flist[F_SET_NUMBER_OF_MODULES]				= &set_number_of_modules;
	flist[F_GET_MAX_NUMBER_OF_MODULES]			= &get_max_number_of_modules;
	flist[F_SET_EXTERNAL_SIGNAL_FLAG]			= &set_external_signal_flag;
	flist[F_SET_EXTERNAL_COMMUNICATION_MODE]	= &set_external_communication_mode;
	flist[F_GET_ID]								= &get_id;
	flist[F_DIGITAL_TEST]						= &digital_test;
	flist[F_ANALOG_TEST]						= &analog_test;
	flist[F_ENABLE_ANALOG_OUT]					= &enable_analog_out;
	flist[F_CALIBRATION_PULSE]					= &calibration_pulse;
	flist[F_SET_DAC]							= &set_dac;
	flist[F_GET_ADC]							= &get_adc;
	flist[F_WRITE_REGISTER]						= &write_register;
	flist[F_READ_REGISTER]						= &read_register;
	flist[F_WRITE_MEMORY]						= &write_memory;
	flist[F_READ_MEMORY]						= &read_memory;
	flist[F_SET_CHANNEL]						= &set_channel;
	flist[F_GET_CHANNEL]						= &get_channel;
	flist[F_SET_ALL_CHANNELS]					= &set_all_channels;
	flist[F_SET_CHIP]							= &set_chip;
	flist[F_GET_CHIP]							= &get_chip;
	flist[F_SET_ALL_CHIPS]						= &set_all_chips;
	flist[F_SET_MODULE]							= &set_module;
	flist[F_GET_MODULE]							= &get_module;
	flist[F_SET_ALL_MODULES]					= &set_all_modules;
	flist[F_SET_SETTINGS]						= &set_settings;
	flist[F_GET_THRESHOLD_ENERGY]				= &get_threshold_energy;
	flist[F_SET_THRESHOLD_ENERGY]				= &set_threshold_energy;
	flist[F_START_ACQUISITION]					= &start_acquisition;
	flist[F_STOP_ACQUISITION]					= &stop_acquisition;
	flist[F_START_READOUT]						= &start_readout;
	flist[F_GET_RUN_STATUS]						= &get_run_status;
	flist[F_START_AND_READ_ALL]					= &start_and_read_all;
	flist[F_READ_FRAME]							= &read_frame;
	flist[F_READ_ALL]							= &read_all;
	flist[F_SET_TIMER]							= &set_timer;
	flist[F_GET_TIME_LEFT]						= &get_time_left;
	flist[F_SET_DYNAMIC_RANGE]					= &set_dynamic_range;
	flist[F_SET_READOUT_FLAGS]					= &set_readout_flags;
	flist[F_SET_ROI]							= &set_roi;
	flist[F_SET_SPEED]							= &set_speed;
	flist[F_EXECUTE_TRIMMING]					= &execute_trimming;
	flist[F_EXIT_SERVER]						= &exit_server;
	flist[F_LOCK_SERVER]						= &lock_server;
	flist[F_GET_LAST_CLIENT_IP]					= &get_last_client_ip;
	flist[F_SET_PORT]							= &set_port;
	flist[F_UPDATE_CLIENT]						= &update_client;
	flist[F_CONFIGURE_MAC]						= &configure_mac;
	flist[F_LOAD_IMAGE]							= &load_image;
	flist[F_SET_MASTER]							= &set_master;
	flist[F_SET_SYNCHRONIZATION_MODE]			= &set_synchronization;
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
	flist[F_CLEANUP_ACQUISITION]				= &cleanup_acquisition;
	flist[F_THRESHOLD_TEMP]                     = &threshold_temp;
	flist[F_TEMP_CONTROL]                       = &temp_control;
	flist[F_TEMP_EVENT]                         = &temp_event;
    flist[F_AUTO_COMP_DISABLE]                  = &auto_comp_disable;
    flist[F_STORAGE_CELL_START]                 = &storage_cell_start;
    flist[F_CHECK_VERSION]                 		= &check_version;
    flist[F_SOFTWARE_TRIGGER]                 	= &software_trigger;

	// check
	if (NUM_DET_FUNCTIONS  >= TOO_MANY_FUNCTIONS_DEFINED) {
		cprintf(BG_RED,"The last detector function enum has reached its limit\nGoodbye!\n");
		exit(EXIT_FAILURE);
	}

#ifdef VERYVERBOSE
	{
		int i=0;
		for (i = 0; i < NUM_DET_FUNCTIONS ; i++) {
			printf("function fnum=%d, (%s) located at 0x%x\n", i,  getFunctionName((enum detFuncs)i), (unsigned int)flist[i]);
		}
	}
#endif
}


int  M_nofunc(int file_des){
	int ret=FAIL,ret1=FAIL;
	int n=0;

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	sprintf(mess,"Unrecognized Function. Please do not proceed.\n");
	cprintf(BG_RED,"Error: %s",mess);
	n = sendData(file_des,&ret1,sizeof(ret1),INT32);
	n = sendData(file_des,mess,sizeof(mess),OTHER);

	// return ok / fail
	return ret;
}



int  M_nofuncMode(int file_des){
	int ret=FAIL,ret1=FAIL;
	int n=0;
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	strcat(mess, "On-board detector server in update mode. Restart detector server in normal mode (without any arguments) to continue.\n");
	cprintf(BG_RED,"Error: %s",mess);
	n = sendData(file_des,&ret1,sizeof(ret1),INT32);
	n = sendData(file_des,mess,sizeof(mess),OTHER); // mess is defined at function call

	// return ok / fail
	return ret;
}



/* functions called by client */



int exec_command(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	char cmd[MAX_STR_LENGTH]="";
	int sysret=0;

	// receive arguments
	n = receiveData(file_des,cmd,MAX_STR_LENGTH,OTHER);
	if (n < 0) return printSocketReadError();

	// execute action if the arguments correctly arrived
#ifdef VERBOSE
	printf("executing command %s\n", cmd);
#endif
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}  else {
		sysret=system(cmd);
		//should be replaced by popen
		if (sysret==0) {
			sprintf(mess,"Succeeded\n");
		} else {
			ret = FAIL;
			sprintf(mess,"Executing Command failed\n");
			cprintf(RED, "Warning: %s", mess);
		}
	}

	ret1=ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}



int get_error(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Get Error) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}



int get_detector_type(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum detectorType retval=-1;

	// execute action
	retval=myDetectorType;
#ifdef VERBOSE
	printf("Returning detector type %d\n",retval);
#endif

	if (differentClients)
		ret=FORCE_UPDATE;

	// send ok / fail
	n += sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;


}




int set_number_of_modules(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=0;
	int arg[2]={-1,-1};
	sprintf(mess,"set number of modules failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	enum dimension dim=arg[0];
	int nm=arg[1];

	// execute action
#ifdef VERBOSE
	printf("Setting the number of modules in dimension %d to %d\n",dim,nm );
#endif
	if (lockStatus && differentClients && nm!=GET_FLAG) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		retval=setNMod(nm, dim);
		dataBytes=calculateDataBytes();
	}
#endif

	if (retval==nm || nm==GET_FLAG) {
		ret=OK;
		if (differentClients)
			ret=FORCE_UPDATE;
	} else
		ret=FAIL;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int get_max_number_of_modules(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	enum dimension arg=0;
	sprintf(mess,"get max number of modules failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
#ifdef VERBOSE
	printf("Getting the max number of modules in dimension %d \n",arg);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	retval=getNModBoard(arg);
#endif
#ifdef VERBOSE
	printf("Max number of module in dimension %d is %d\n",arg,retval );
#endif
	if (differentClients && ret==OK) {
		ret=FORCE_UPDATE;
	}

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int set_external_signal_flag(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum externalSignalFlag retval=GET_EXTERNAL_SIGNAL_FLAG;
	sprintf(mess,"set external signal flag failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Set External Signal Flag) is not implemented for this detector\n");
	cprintf(RED, "%s", mess);
#else

	// receive arguments
	int arg[2]={-1,-1};
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	int signalindex=arg[0];
	enum externalSignalFlag flag=arg[1];

	// execute action
	if (lockStatus && differentClients && flag!=GET_EXTERNAL_SIGNAL_FLAG) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}

#ifdef SLS_DETECTOR_FUNCTION_LIST
	else{
#ifdef VERBOSE
		printf("Setting external signal %d to flag %d\n",signalindex,flag);
#endif
		switch (flag) {
		case GET_EXTERNAL_SIGNAL_FLAG:
			retval=getExtSignal(signalindex);
			break;
		default:
			retval=setExtSignal(signalindex,flag);
			if (retval!=flag) {
				ret=FAIL;
				sprintf(mess,"External signal %d flag should be 0x%04x but is 0x%04x\n", signalindex, flag, retval);
				cprintf(RED, "%s", mess);
			}
			break;
		}
#ifdef VERBOSE
		printf("Set to flag %d\n",retval);
#endif
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;

}


int set_external_communication_mode(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum externalCommunicationMode arg=GET_EXTERNAL_COMMUNICATION_MODE;
	enum externalCommunicationMode retval=GET_EXTERNAL_COMMUNICATION_MODE;
	sprintf(mess,"set external communication mode failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
#ifdef VERBOSE
	printf("Setting external communication mode to %d\n", arg);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	switch(arg){
#ifdef EIGERD
	case GET_EXTERNAL_COMMUNICATION_MODE:
	case AUTO_TIMING:
	case TRIGGER_EXPOSURE:
	case GATE_FIX_NUMBER:
	case BURST_TRIGGER:
#elif JUNGFRAUD
	case GET_EXTERNAL_COMMUNICATION_MODE:
	case AUTO_TIMING:
	case TRIGGER_EXPOSURE:
#endif
		retval=setTiming(arg);
		break;
	default:
		ret = FAIL;
		sprintf(mess,"Timing mode (%d) is not implemented for this detector\n",(int)arg);
		cprintf(RED, "Warning: %s", mess);
		break;
	}
#ifdef VERBOSE
	if(ret==OK)
		printf("retval:%d\n",retval);
#endif
	if (ret==OK && differentClients==1)
		ret=FORCE_UPDATE;

#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int get_id(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum idMode arg=0;
	int imod=-1;
	int64_t retval=-1;
	sprintf(mess,"get id failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	if (arg == MODULE_SERIAL_NUMBER) {
		n = receiveData(file_des,&imod,sizeof(imod),INT32);
		if (n < 0) return printSocketReadError();
	}

	// execute action
#ifdef VERBOSE
	printf("Getting id %d\n", arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	switch (arg) {
#ifdef MYTHEND
	case MODULE_SERIAL_NUMBER:
	case MODULE_FIRMWARE_VERSION:
#ifdef VERBOSE
		printf("of module %d\n", imod);
#endif

		if (imod>=0 && imod<getTotalNumberOfModules())
			retval=getModuleId(arg, imod);
		else {
			ret = FAIL;
			sprintf(mess,"Module number %d out of range\n", imod);
			cprintf(RED, "Warning: %s", mess);
		}
		break;
#endif
#ifdef EIGERD
	case SOFTWARE_FIRMWARE_API_VERSION:
#endif
	case DETECTOR_SERIAL_NUMBER:
	case DETECTOR_FIRMWARE_VERSION:
	case DETECTOR_SOFTWARE_VERSION:
		retval=getDetectorId(arg);
		break;
	default:
		ret = FAIL;
		sprintf(mess,"ID Index (%d) is not implemented for this detector\n", (int) arg);
		cprintf(RED, "Warning: %s", mess);
		break;
	}
#endif
#ifdef VERBOSE
	printf("ret is %d\n",ret);
	printf("Id is %llx\n", retval);
#endif

	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT64);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}





int digital_test(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"get digital test failed\n");

#ifdef EIGERD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Digital Test) is not implemented for this detector\n");
	cprintf(RED, "%s", mess);
#else

	enum digitalTestMode arg=0;
	int imod=-1;
	int ival=-1;

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	if (arg == CHIP_TEST) {
		n = receiveData(file_des,&imod,sizeof(imod),INT32);
		if (n < 0) return printSocketReadError();
	}

	if (arg == DIGITAL_BIT_TEST) {
		n = receiveData(file_des,&ival,sizeof(ival),INT32);
		if (n < 0) return printSocketReadError();
	}

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	} else {
#ifdef VERBOSE
		printf("Digital test mode %d\n",arg );
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
		switch (arg) {

#ifdef GOTTHARD
		case DIGITAL_BIT_TEST:
			retval=0;
			break;

#elif MYTHEND
		case  CHIP_TEST:
#ifdef VERBOSE
			printf("of module %d\n", imod);
#endif
			if (imod>=0 && imod<getTotalNumberOfModules())
				retval=moduleTest(arg,imod);
			else {
				ret = FAIL;
				sprintf(mess,"Module number %d out of range\n", imod);
				cprintf(RED, "Warning: %s", mess);
			}
			break;
		case MODULE_FIRMWARE_TEST:
		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_BUS_TEST:
		case DETECTOR_MEMORY_TEST:
		case DETECTOR_SOFTWARE_TEST:

#elif JUNGFRAUD
		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_BUS_TEST:
#endif
			retval=detectorTest(arg);
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Digital Test Mode (%d) is not implemented for this detector\n",(int)arg);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
#endif
	}
#ifdef VERBOSE
	printf("digital test result is 0x%x\n", retval);
#endif

	//Always returns force update such that the dynamic range is always updated on the client
#ifndef MYTHEND
	if (differentClients)
#endif
		ret=FORCE_UPDATE;
#endif

	// send ok / fail
	// ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		// send return argument
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;

}



int analog_test(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Analog Test) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}



int enable_analog_out(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Enable Analog Out) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}



int calibration_pulse(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Calibration Pulse) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}




int set_dac(int file_des) {
    int ret=OK,ret1=OK;
    int n=0;
    int arg[3]={-1,-1,-1};
    int val=-1;
    enum dacIndex ind=0;
    int imod=-1;
    int retval[2]={-1,-1};
    int mV=0;
    sprintf(mess,"set DAC failed\n");

    // receive arguments
    n = receiveData(file_des,arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();
    ind=arg[0];
    imod=arg[1];
    mV=arg[2];

    n = receiveData(file_des,&val,sizeof(val),INT32);
    if (n < 0) return printSocketReadError();

    // checks
#ifdef MYTHEND
#ifdef SLS_DETECTOR_FUNCTION_LIST
    if (imod>=getTotalNumberOfModules()) {
        ret = FAIL;
        sprintf(mess,"Module number %d out of range\n",imod);
        cprintf(RED, "Warning: %s", mess);
    }
#endif
#endif
    // check if dac exists for this detector
    enum DACINDEX idac=0;
#ifdef JUNGFRAUD
    if ((ind != HV_NEW) && (ind >= NDAC_OLDBOARD)) {	//for compatibility with old board
        ret = FAIL;
        sprintf(mess,"Dac Index (%d) is not implemented for this detector\n",(int)ind);
        cprintf(RED, "Warning: %s", mess);
    }else
        idac = ind;
#else
    switch (ind) {
#ifdef MYTHEND
    case  TRIMBIT_SIZE:			//ind = VTRIM;
    case THRESHOLD:
    case SHAPER1:
    case SHAPER2:
    case CALIBRATION_PULSE:
    case PREAMP:
        break;
#elif GOTTHARDD
    case G_VREF_DS :
        break;
    case G_VCASCN_PB:
        break;
    case G_VCASCP_PB:
        break;
    case G_VOUT_CM:
        break;
    case G_VCASC_OUT:
        break;
    case G_VIN_CM:
        break;
    case G_VREF_COMP:
        break;
    case G_IB_TESTC:
        break;
    case HV_POT:
        break;
#elif EIGERD
    case TRIMBIT_SIZE:
        idac = VTR;
        break;
    case THRESHOLD:
        idac = VTHRESHOLD;
        break;
    case E_SvP:
        idac = SVP;
        break;
    case E_SvN:
        idac = SVN;
        break;
    case E_Vtr:
        idac = VTR;
        break;
    case E_Vrf:
        idac = VRF;
        break;
    case E_Vrs:
        idac = VRS;
        break;
    case E_Vtgstv:
        idac = VTGSTV;
        break;
    case E_Vcmp_ll:
        idac = VCMP_LL;
        break;
    case E_Vcmp_lr:
        idac = VCMP_LR;
        break;
    case E_cal:
        idac = CAL;
        break;
    case E_Vcmp_rl:
        idac = VCMP_RL;
        break;
    case E_Vcmp_rr:
        idac = VCMP_RR;
        break;
    case E_rxb_rb:
        idac = RXB_RB;
        break;
    case E_rxb_lb:
        idac = RXB_LB;
        break;
    case E_Vcp:
        idac = VCP;
        break;
    case E_Vcn:
        idac = VCN;
        break;
    case E_Vis:
        idac = VIS;
        break;
    case HV_NEW:
        break;
    case IO_DELAY:
        break;
        /*
#elif JUNGFRAUD
	case V_DAC0:
		idac = VB_COMP;
		break;
	case V_DAC1:
		idac = VDD_PROT;
		break;
	case V_DAC2:
		idac = VIN_COM;
		break;
	case V_DAC3:
		idac = VREF_PRECH;
		break;
	case V_DAC4:
		idac = VB_PIXBUF;
		break;
	case V_DAC5:
		idac = VB_DS;
		break;
	case V_DAC6:
		idac = VREF_DS;
		break;
	case V_DAC7:
		idac = VREF_COMP;
		break;
	case HV_POT:
		break;
         */
#elif MYTHEN3D
    case M_vIpre:
        idac = vIpre;
        break;
    case M_vIbias:
        idac = vIbias;
        break;
    case PREAMP:
        idac = Vrf;
        break;
    case SHAPER1:
        idac = VrfSh;
        break;
    case M_vIinSh:
        idac = vIinSh;
        break;
    case M_VdcSh:
        idac = VdcSh;
        break;
    case M_Vth2:
        idac = Vth2;
        break;
    case M_VPL:
        idac = VPL;
        break;
    case THRESHOLD:
        idac = Vth1;
        break;
    case M_Vth3:
        idac = Vth3;
        break;
    case TRIMBIT_SIZE:
        idac = Vtrim;
        break;
    case M_casSh:
        idac = casSh;
        break;
    case M_cas:
        idac = cas;
        break;
    case M_vIbiasSh:
        idac = vIbiasSh;
        break;
    case M_vIcin:
        idac = vIcin;
        break;
    case CALIBRATION_PULSE: // !!! pulse height + 1400 DACu
        idac = VPH;
        break;
    case M_vIpreOut:
        idac = vIpreOut;
        break;
    case V_POWER_A:
        idac = V_A;
        break;
    case V_POWER_B:
        ipwr = V_B;
        break;
    case V_POWER_IO:
        idac = V_IO;
        break;
    case V_POWER_CHIP:
        idac = V_CHIP;
        break;
    case V_LIMIT:
        idac = V_LIM;
        break;
#endif
    default:
        ret = FAIL;
        sprintf(mess,"Dac Index (%d) is not implemented for this detector\n",(int)ind);
        cprintf(RED, "Warning: %s", mess);
        break;
    }
#endif

    // execute action
#ifdef VERBOSE
    printf("Setting DAC %d of module %d to %d \n", idac, imod, val);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
    int temp;
    if (ret==OK) {
        if (differentClients && lockStatus && val!=-1) {
            ret = FAIL;
            sprintf(mess,"Detector locked by %s\n",lastClientIP);
            cprintf(RED, "Warning: %s", mess);
        } else {
#ifdef EIGERD
            //iodelay
            if(ind == IO_DELAY)
                retval[0] = setIODelay(val,imod);
            //high voltage
            else
#endif
                if((ind == HV_POT) || (ind == HV_NEW)) {
                    retval[0] = setHighVoltage(val);
#ifdef EIGERD
                    if ((retval[0] != SLAVE_HIGH_VOLTAGE_READ_VAL) && (retval[0] < 0)) {
                        ret = FAIL;
                        if(retval[0] == -1)
                            sprintf(mess, "Setting high voltage failed.Bad value %d. The range is from 0 to 200 V.\n",val);
                        else if(retval[0] == -2)
                            strcpy(mess, "Setting high voltage failed. Serial/i2c communication failed.\n");
                        else if(retval[0] == -3)
                            strcpy(mess, "Getting high voltage failed. Serial/i2c communication failed.\n");
                        cprintf(RED, "Warning: %s", mess);
                    }
#endif
                }
#ifdef MYTHEN3D
                else if	((ind >= V_POWER_A  && ind <= V_POWER_CHIP) || ind == V_LIMIT) {
                    printf("Setting a power %d to %d\n",ind, val);

                    if (!mV) {
                        ret = FAIL;
                        strcpy(mess, "Power of index %d should be set in mV instead of DACu", idac);
                        cprintf(RED, "Warning: %s", mess);
                        val = -1;
                    }

                    int lim = getVLimit();
                    if (ind != V_LIMIT && lim != -1 && val > lim) {
                        ret = FAIL;
                        strcpy(mess, "Power of index %d is %d, should be less than %dmV\n", idac, val, lim);
                        cprintf(RED, "Warning: %s", mess);
                        val = -1;
                    }

                    retval[1] = retval[0] = setPower(idac,val);
                    if (val >= 0 && retval[1] != val) {
                        ret = FAIL;
                        sprintf(mess,"Setting power %d failed: wrote %d but read %d\n", idac, val, retval[1]);
                        cprintf(RED, "Warning: %s", mess);
                    }
                }
#endif
            //dac
                else{
#ifdef MYTHEN3D
                    if( mV && val > MAX_DACVOLTVAL) {
                        ret = FAIL;
                        strcpy(mess, "Dac of index %d should be less than %dmV\n", idac, val, MAX_DACVOLTVAL);
                        cprintf(RED, "Warning: %s", mess);
                        val = -1;
                    }
                    else if( !mV && val >= MAX_DACVAL) {
                        ret = FAIL;
                        strcpy(mess, "Dac of index %d should be less than %d (dac value)\n", idac, val, MAX_DACVAL);
                        cprintf(RED, "Warning: %s", mess);
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
                            strcpy(mess, "Dac of index %d should be less than %dmV (%d dac value)\n", idac, lim, voltageToDac(lim));
                            cprintf(RED, "Warning: %s", mess);
                            val = -1;
                        }
                    }
#endif
                    setDAC(idac,val,imod,mV,retval);
#ifdef EIGERD
                    if(val != -1) {
                        //changing dac changes settings to undefined
                        switch(idac){
                        case VCMP_LL:
                        case VCMP_LR:
                        case VCMP_RL:
                        case VCMP_RR:
                        case VRF:
                        case VCP:
                            setSettings(UNDEFINED,-1);
                            cprintf(RED,"Settings has been changed to undefined (changed specific dacs)\n");
                            break;
                        default:
                            break;
                        }
                    }
#endif
                    //check
                    if (ret == OK) {
                        if(mV)
                            temp = retval[1];
                        else
                            temp = retval[0];
                        if ((abs(temp-val)<=5) || val==-1) {
                            ret = OK;
                        } else {
                            ret = FAIL;
                            sprintf(mess,"Setting dac %d of module %d: wrote %d but read %d\n", idac, imod, val, temp);
                            cprintf(RED, "Warning: %s", mess);
                        }
                    }
                }
        }
    }
#endif
#ifdef VERBOSE
    printf("DAC set to %d in dac units and %d mV\n",  retval[0],retval[1]);
#endif

    if(ret == OK && differentClients)
        ret=FORCE_UPDATE;

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    if (ret!=FAIL) {
        n += sendData(file_des,&retval,sizeof(retval),INT32);
    } else {
        n += sendData(file_des,mess,sizeof(mess),OTHER);
    }

    // return ok / fail
    return ret;
}






int get_adc(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int arg[2]={-1,-1};
	int retval=-1;
	enum dacIndex ind=0;
	int imod=-1;
	sprintf(mess,"get ADC failed\n");

#ifdef MYTHEN3D
    //to receive any arguments
    while (n > 0)
        n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
    ret = FAIL;
    sprintf(mess,"Function (Get ADC) is not implemented for this detector\n");
    cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	ind=arg[0];
	imod=arg[1];

#ifdef MYTHEND
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules() || imod<0) {
		ret = FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
		cprintf(RED, "Warning: %s", mess);
	}
#endif
#endif

	enum ADCINDEX iadc=0;
	switch (ind) {
#ifdef EIGERD
	case TEMPERATURE_FPGAEXT:
		iadc = TEMP_FPGAEXT;
		break;
	case TEMPERATURE_10GE:
		iadc = TEMP_10GE;
		break;
	case TEMPERATURE_DCDC:
		iadc = TEMP_DCDC;
		break;
	case TEMPERATURE_SODL:
		iadc = TEMP_SODL;
		break;
	case TEMPERATURE_SODR:
		iadc = TEMP_SODR;
		break;
	case TEMPERATURE_FPGA:
		iadc = TEMP_FPGA;
		break;
	case TEMPERATURE_FPGA2:
		iadc = TEMP_FPGAFEBL;
		break;
	case TEMPERATURE_FPGA3:
		iadc = TEMP_FPGAFEBR;
		break;
#endif
#if defined(GOTTHARD) || defined(JUNGFRAUD)
	case TEMPERATURE_FPGA:
		iadc = TEMP_FPGA;
		break;
	case TEMPERATURE_ADC:
		iadc = TEMP_ADC;
		break;
#endif
	default:
		ret = FAIL;
		sprintf(mess,"Dac Index (%d) is not implemented for this detector\n",(int)ind);
		cprintf(RED, "Warning: %s", mess);
		break;
	}

#ifdef VERBOSE
	printf("Getting ADC %d of module %d\n", iadc, imod);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK)
		retval=getADC(iadc,imod);
#endif
#ifdef VERBOSE
	printf("ADC is %f\n",  retval);
#endif
#endif

	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
#endif
#ifdef VERBOSE
	printf("Data set to 0x%x\n",  retval);
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	retval=readRegister(addr);
#endif
#ifdef VERBOSE
	printf("Returned value 0x%x\n",  retval);
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int write_memory(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Write Memory) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}


int read_memory(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Read Memory) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}



int set_channel(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"set channel failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Set Channel) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	sls_detector_channel myChan;
	n=receiveChannel(file_des, &myChan);
	if (n < 0) return printSocketReadError();

	// execute action
#ifdef VERBOSE
	printf("Setting channel\n");
	printf("channel number is %d, chip number is %d, module number is %d, register is %lld\n", myChan.chan,myChan.chip, myChan.module, myChan.reg);
#endif
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else if (myChan.chan>=getNumberOfChannelsPerChip()) {
		ret = FAIL;
		sprintf(mess,"channel number %d too large!\n",myChan.chan);
		cprintf(RED, "Warning: %s", mess);
	}
	else if (myChan.chip>=getNumberOfChipsPerModule()) {
		ret = FAIL;
		sprintf(mess,"chip number %d too large!\n",myChan.chip);
		cprintf(RED, "Warning: %s", mess);
	}
	else if (myChan.module>=getTotalNumberOfModules()) {
		ret = FAIL;
		sprintf(mess,"module number %d too large!\n",myChan.module);
		cprintf(RED, "Warning: %s", mess);
	}
	else
		retval=setChannel(myChan);
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int get_channel(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sls_detector_channel retval;
	sprintf(mess,"get channel failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Get Channel) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg[3]={-1,-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	int ichan=arg[0];
	int ichip=arg[1];
	int imod=arg[2];
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ichan>=getNumberOfChannelsPerChip()) {
		ret=FAIL;
		sprintf(mess, "channel number %d too large!\n",myChan.chan);
		cprintf(RED, "Warning: %s", mess);
	} else
		retval.chan=ichan;
	if (ichip>=getNumberOfChipsPerModule()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",myChan.chip);
		cprintf(RED, "Warning: %s", mess);
	} else
		retval.chip=ichip;

	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess, "module number %d too large!\n",myChan.module);
		cprintf(RED, "Warning: %s", mess);
	} else {
		retval.module=imod;
		ret=getChannel(&retval);
#ifdef VERBOSE
	printf("Returning channel %d %d %d, 0x%llx\n", retval.chan, retval.chip, retval.mod, (retval.reg));
#endif
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		ret=sendChannel(file_des, &retval);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}



int set_all_channels(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Set All Channels) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}





int set_chip(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"set chip failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Set Chip) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else
	sls_detector_chip myChip;

#ifdef SLS_DETECTOR_FUNCTION_LIST
	myChip.nchan=getNumberOfChannelsPerChip();
	int *ch(int*)malloc((myChip.nchan)*sizeof(int));
	myChip.chanregs=ch;

	// receive arguments
	n=receiveChip(file_des, &myChip);
#ifdef VERBOSE
	printf("Chip received\n");
#endif
	if(n < 0) return FAIL;

	// execute action
	if (differentClients==1 && lockStatus==1) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
	else{
#ifdef VERBOSE
		printf("Setting chip\n");
		printf("chip number is %d, module number is %d, register is %d, nchan %d\n",myChip.chip, myChip.module, myChip.reg, myChip.nchan);
#endif
		if (myChip.chip>=getNumberOfChipsPerModule()) {
			ret = FAIL;
			sprintf(mess,"chip number %d too large!\n",myChan.chip);
			cprintf(RED, "Warning: %s", mess);
		}
		else if (myChip.module>=getTotalNumberOfModules()) {
			ret = FAIL;
			sprintf(mess,"module number %d too large!\n",myChan.module);
			cprintf(RED, "Warning: %s", mess);
		}
		else
			retval=setChip(myChip);
	}
	free(ch);
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int get_chip(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sls_detector_chip retval;
	sprintf(mess,"get chip failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Get Chip) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg[2]={-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

#ifdef SLS_DETECTOR_FUNCTION_LIST
	int ichip=arg[0];
	int	imod=arg[1];

	// execute action
	if (ichip>=getNumberOfChipsPerModule()) {
		ret = FAIL;
		sprintf(mess,"channel number %d too large!\n",myChan.chan);
		cprintf(RED, "Warning: %s", mess);
	} else
		retval.chip=ichip;

	if (imod>=getTotalNumberOfModules()) {
		ret = FAIL;
		sprintf(mess,"module number %d too large!\n",imod);
		cprintf(RED, "Warning: %s", mess);
	} else
		retval.module=imod;

	if (ret==OK)
		ret=getChip(&retval);
#endif
#ifdef VERBOSE
	printf("Returning chip %d %d\n",  ichip, imod);
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		ret=sendChip(file_des, &retval);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}




int set_all_chips(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Set All Chips) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
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


#ifdef SLS_DETECTOR_FUNCTION_LIST
	int *myDac=NULL;
	int *myAdc=NULL;
	int *myChip = NULL;
	int *myChan = NULL;

	myDac=(int*)malloc(getNumberOfDACsPerModule()*sizeof(int));
	if (getNumberOfDACsPerModule() > 0 && myDac == NULL) {
		ret = FAIL;
		sprintf(mess,"could not allocate dacs\n");
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		myModule.dacs=myDac;
		myAdc=(int*)malloc(getNumberOfADCsPerModule()*sizeof(int));
		if (getNumberOfADCsPerModule() > 0 && myAdc == NULL) {
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
			myChip=(int*)malloc(getNumberOfChipsPerModule()*sizeof(int));
			if (getNumberOfChipsPerModule() > 0 && myChip == NULL) {
				ret = FAIL;
				sprintf(mess,"could not allocate chips\n");
				cprintf(RED, "Warning: %s", mess);
			}
			else {
				myModule.chipregs=myChip;
				myChan=(int*)malloc(getNumberOfChannelsPerModule()*sizeof(int));
				if (getNumberOfChannelsPerModule() > 0 && myChan == NULL) {
					ret = FAIL;
					sprintf(mess,"could not allocate chans\n");
					cprintf(RED, "Warning: %s", mess);
				}
				else {
					myModule.chanregs=myChan;
#endif
					myModule.nchip=getNumberOfChipsPerModule();
					myModule.nchan=getNumberOfChannelsPerModule();
					myModule.ndac=getNumberOfDACsPerModule();
					myModule.nadc=getNumberOfADCsPerModule();


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
		printf("module number is %d,register is %d, nchan %d, nchip %d, ndac %d, nadc %d, gain %f, offset %f\n",
				myModule.module, myModule.reg, myModule.nchan, myModule.nchip, myModule.ndac,  myModule.nadc, myModule.gain,myModule.offset);
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
#ifdef MYTHEND
		if (myModule.module>=getNModBoard()) {
			ret = FAIL;
			sprintf(mess,"Module Number to Set Module (%d) is too large\n", myModule.module);
			cprintf(RED, "Warning: %s", mess);
		}
		if (myModule.module<0)
			myModule.module=ALLMOD;
#endif
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
				setThresholdEnergy(myEV,-1);
			else {
				//changes settings to undefined (loading a random trim file)
				setSettings(UNDEFINED,-1);
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}








int get_module(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int arg=-1;
	int  imod=-1;
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

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();
	imod=arg;

	// execute action
#ifdef SLS_DETECTOR_FUNCTION_LIST
	int *myDac=NULL;
	int *myAdc=NULL;
	int *myChip = NULL;
	int *myChan = NULL;

	if (imod<0 || imod>getTotalNumberOfModules()) {
		ret = FAIL;
		sprintf(mess,"Module Index (%d) is out of range\n", imod);
		cprintf(RED, "Warning: %s", mess);
	}
	else {
		myDac=(int*)malloc(getNumberOfDACsPerModule()*sizeof(int));
		if (getNumberOfDACsPerModule() > 0 && myDac == NULL) {
			ret = FAIL;
			sprintf(mess,"could not allocate dacs\n");
			cprintf(RED, "Warning: %s", mess);
		}
		else {
			myModule.dacs=myDac;
			myAdc=(int*)malloc(getNumberOfADCsPerModule()*sizeof(int));
			if (getNumberOfADCsPerModule() > 0 && myAdc == NULL) {
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
				myChip=(int*)malloc(getNumberOfChipsPerModule()*sizeof(int));
				if (getNumberOfChipsPerModule() > 0 && myChip == NULL) {
					ret = FAIL;
					sprintf(mess,"could not allocate chips\n");
					cprintf(RED, "Warning: %s", mess);
				}
				else {
					myModule.chipregs=myChip;
					myChan=(int*)malloc(getNumberOfChannelsPerModule()*sizeof(int));
					if (getNumberOfChannelsPerModule() > 0 && myChan == NULL) {
						ret = FAIL;
						sprintf(mess,"could not allocate chans\n");
						cprintf(RED, "Warning: %s", mess);
					}
					else {
						myModule.chanregs=myChan;
#endif
						myModule.nchip=getNumberOfChipsPerModule();
						myModule.nchan=getNumberOfChannelsPerModule();
						myModule.ndac=getNumberOfDACsPerModule();
						myModule.nadc=getNumberOfADCsPerModule();
						myModule.module=imod;
						getModule(&myModule);
#ifdef VERBOSE
						printf("Returning module %d of register %x\n",  imod, myModule.reg);
#endif
#ifndef JUNGFRAUD
					}
				}
#endif
			}
		}
	}
#endif
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret!=FAIL) {
		ret=sendModuleGeneral(file_des, &myModule,
#ifdef JUNGFRAUD
				0	//0 is to receive partially (without trimbits etc.)
#else
				1
#endif
		);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if(myChip != NULL) 	free(myChip);
	if(myChan != NULL) 	free(myChan);
	if(myDac != NULL) 	free(myDac);
	if(myAdc != NULL) 	free(myAdc);
#endif

	// return ok / fail
	return ret;

}



int set_all_modules(int file_des) {
	int ret=FAIL,ret1=FAIL;
	int n=0;
	sprintf(mess,"Function (Set All Modules) is not implemented for this detector\n");
	cprintf(RED, "Error: %s", mess);

	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,mess,MAX_STR_LENGTH,OTHER);

	// return ok / fail
	return ret;
}




int set_settings(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int arg[2]={-1,-1};
	int retval=-1;
	int imod=-1;
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
	isett=arg[0];
	imod=arg[1];

	// execute action
	if (differentClients && lockStatus && isett!=GET_SETTINGS) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST

#ifdef MYTHEND
	if ( (ret != FAIL) && (imod>=getTotalNumberOfModules())) {
		ret = FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
		cprintf(RED, "Warning: %s", mess);
	}
#endif
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
		printf("Changing settings of module %d to %d\n", imod,  isett);
#endif
		retval=setSettings(isett, imod);
#ifdef VERBOSE
		printf("Settings changed to %d\n",  isett);
#endif
		if (retval == isett || isett < 0) {
			ret=OK;
		} else {
			ret = FAIL;
			sprintf(mess,"Changing settings of module %d: wrote %d but read %d\n", imod, isett, retval);
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
#endif

	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}





int get_threshold_energy(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"get threshold energy failed\n");

#if !defined(MYTHEND) && !defined(EIGERD)
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Get Threshold Energy) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int imod=-1;
	n = receiveData(file_des,&imod,sizeof(imod),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
#ifdef VERBOSE
	printf("Getting threshold energy of module %d\n", imod);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}
	else {
		retval=getThresholdEnergy(imod);
#ifdef VERBOSE
	printf("Threshold is %d eV\n",  retval);
#endif
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}




int set_threshold_energy(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"set thhreshold energy failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
#ifdef EIGERD
	sprintf(mess,"Function (Set Threshold Energy) is only implemented via Set Settings for this detector\n");
#else
	sprintf(mess,"Function (Set Threshold Energy) is not implemented for this detector\n");
#endif
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	int arg[3]={-1,-1,-1};
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	int ethr=arg[0];
	int imod=arg[1];
	enum detectorSettings isett=arg[2];
	if (differentClients && lockStatus) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}
	else {
		printf("Setting threshold energy of module %d to %d eV with settings %d\n", imod, ethr, isett);
		retval=setThresholdEnergy(ethr, imod);
#ifdef VERBOSE
	printf("Threshold set to %d eV\n",  retval);
#endif
	if (retval!=ethr) {
		ret=FAIL;
		sprintf(mess,"Setting threshold of module %d: wrote %d but read %d\n", imod, ethr, retval);
		cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	 else {
		printf("Starting acquisition\n");
		ret=startStateMachine();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		printf("Stopping acquisition\n");
		ret=stopStateMachine();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		printf("Starting readout\n");
		ret=startReadOut();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}






int get_run_status(int file_des) {
	int ret=OK,ret1=OK;
	enum runStatus s=ERROR;

	// execute action
#ifdef VERBOSE
	printf("Getting status\n");
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	s= getRunStatus();
#endif
	if (differentClients)
		ret=FORCE_UPDATE;

	// send ok / fail
	sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	sendData(file_des,&s,sizeof(s),INT32);

	// return ok / fail
	return ret;
}





int start_and_read_all(int file_des) {
	int dataret1=FAIL, dataret=FAIL;
#ifdef VERBOSE
	printf("Starting and reading all frames\n");
#endif

	// execute action
	if (differentClients && lockStatus) {
		dataret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
		// ret could be swapped during sendData
		dataret1 = dataret;
		// send fail
		sendData(file_des,&dataret1,sizeof(dataret),INT32);
		// send return argument
		sendData(file_des,mess,sizeof(mess),OTHER);
		// return fail
		return dataret;
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	startStateMachine();
	read_all(file_des);
#endif
	return OK;
}




int read_frame(int file_des) {
	int dataret1=FAIL, dataret=FAIL;
	int n=0;
	sprintf(mess, "read frame failed\n");

	// execute action
	if (differentClients && lockStatus) {
		dataret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
		// ret could be swapped during sendData
		dataret1 = dataret;
		// send fail
		sendData(file_des,&dataret1,sizeof(dataret),INT32);
		// send return argument
		sendData(file_des,mess,sizeof(mess),OTHER);
		// return fail
		return dataret;
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	readFrame(&dataret, mess);
#endif
	if(dataret == FAIL)
		cprintf(RED,"%s\n",mess);
	else
		cprintf(GREEN,"%s",mess);

	if (differentClients)
		dataret=FORCE_UPDATE;

	//dataret could be swapped during sendData
	dataret1 = dataret;
	// send finished / fail
	n=sendData(file_des,&dataret1,sizeof(dataret1),INT32);
	if (n<0) return FAIL;	// if called from read_all, should fail to stop talking to a closed client socket
	// send return argument
	n=sendData(file_des,mess,sizeof(mess),OTHER);
	if (n<0) return FAIL;	// if called from read_all, should fail to stop talking to a closed client socket
	// return finished / fail
	return dataret;
}




int read_all(int file_des) {
#ifdef SLS_DETECTOR_FUNCTION_LIST
	while(read_frame(file_des)==OK) {
#ifdef VERBOSE
		printf("frame read\n");
#endif
		;
	}
#endif
#ifdef VERBOSE
	printf("Frames finished or failed\n");
#endif
	return OK;
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
#ifdef MYTHEN
		case PROBES_NUMBER:
		case GATES_NUMBER:
		case DELAY_AFTER_TRIGGER:
#elif JUNGFRAUD
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


#if defined(MYTHEND) || defined(GOTTHARD)
		if (ret == OK && ind==FRAME_NUMBER) {
			ret=allocateRAM();
			if (ret!=OK) {
				ret = FAIL;
				sprintf(mess,"Could not allocate RAM for %lld frames\n", tns);
				cprintf(RED, "%s", mess);
			}
		}
#endif
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT64);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST

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
#elif MYTHEND
		case PROBES_NUMBER:
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

#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;


	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT64);

	// return ok / fail
	return ret;
}






int set_dynamic_range(int file_des) {
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
#endif
	if ((ret == OK) && dr>=0 && retval!=dr) {
		ret = FAIL;
		cprintf(RED,"%s",mess);
	}
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	//rate correction ret
	// ret could be swapped during sendData
	rateret1 = rateret;
	// send ok / fail
	n = sendData(file_des,&rateret1,sizeof(rateret1),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}






int set_readout_flags(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum readOutFlags retval=-1;
	sprintf(mess,"set readout flags failed\n");

#if !defined(MYTHEND) && !defined(EIGERD)
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
#ifdef VERBOSE
		printf("setting readout flags  to %d\n",arg);
#endif
		switch(arg) {
		case  GET_READOUT_FLAGS:
#ifdef MYTHEND
		case TOT_MODE:
		case NORMAL_READOUT:
		case STORE_IN_RAM:
		case CONTINOUS_RO:
#elif EIGERD
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
#endif
		if (ret==OK && ((retval == -1) || ((arg!=-1) && ((retval&arg)!=arg)))){
			ret = FAIL;
			sprintf(mess,"Could not change readout flag: should be 0x%x but is 0x%x\n", arg, retval);
			cprintf(RED, "Warning: %s", mess);
		}

		if (ret==OK && differentClients)
			ret=FORCE_UPDATE;
	}
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
	int retvalsize=0,retvalsize1=0;

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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	if(ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
#ifdef GOTTHARDD
	else {
		//retvalsize could be swapped during sendData
		retvalsize1=retvalsize;
		sendData(file_des,&retvalsize1,sizeof(retvalsize),INT32);
		int i=0;
		for(i=0;i<retvalsize;i++){
			n = sendData(file_des,&retval[i].xmin,sizeof(int),INT32);
			n = sendData(file_des,&retval[i].xmax,sizeof(int),INT32);
			n = sendData(file_des,&retval[i].ymin,sizeof(int),INT32);
			n = sendData(file_des,&retval[i].ymax,sizeof(int),INT32);
		}
		//sendData(file_des,retval,retvalsize*sizeof(ROI));
	}
#endif
	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
#ifdef MYTHEND
		case CLOCK_DIVIDER:
		case WAIT_STATES:
		case SET_SIGNAL_LENGTH:
		case TOT_CLOCK_DIVIDER:
		case TOT_DUTY_CYCLE:
#elif EIGERD
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
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}





int execute_trimming(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"execute trimming failed\n");

#ifndef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Execute Trimming) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	int retval=-1;

	// receive arguments
	enum trimMode mode=0;
	int arg[3]={-1,-1,-1};
	n = receiveData(file_des,&mode,sizeof(mode),INT32);
	if (n < 0) return printSocketReadError();

	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	int imod, par1,par2;
	imod=arg[0];
	par1=arg[1];
	par2=arg[2];
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else if (imod>=getTotalNumberOfModules()) {
		ret = FAIL;
		sprintf(mess,"Module Number (%d) is out of range\n");
		cprintf(RED, "Warning: %s", mess);
	}
	else {
#ifdef VERBOSE
		printf("trimming module %d mode %d, parameters %d %d \n",imod,mode, par1, par2);
#endif  
		switch(mode) {
		case NOISE_TRIMMING:
		case BEAM_TRIMMING:
		case IMPROVE_TRIMMING:
		case FIXEDSETTINGS_TRIMMING:
			retval=executeTrimming(mode, par1, par2, imod);
			if ((ret!=OK) && (retval>0)) {
				ret=FAIL;
				sprintf(mess,"Could not trim %d channels\n", retval);
				cprintf(RED, "Warning: %s", mess);
			}
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Trimming Mode (%d) is not implemented for this detector\n", (int) mode);
			cprintf(RED, "Warning: %s", mess);
			break;
		}

	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}





int exit_server(int file_des) {
	int ret=FAIL;
	sprintf(mess,"Closing Server\n");
	cprintf(BG_RED,"Error: %s",mess);
	// send ok / fail
	sendData(file_des,&ret,sizeof(ret),INT32);
	// send return argument
	sendData(file_des,mess,sizeof(mess),OTHER);
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&lockStatus,sizeof(lockStatus),INT32);

	// return ok / fail
	return ret;
}





int get_last_client_ip(int file_des) {
	int ret=OK,ret1=OK;
	if (differentClients)
		ret=FORCE_UPDATE;
	// send ok / fail
	sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
	// return ok / fail
	return ret;
}




int set_port(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum portType p_type=0;
	int p_number=-1;
	sprintf(mess,"set port failed\n");

	// receive arguments
	n = receiveData(file_des,&p_type,sizeof(p_type),INT32);
	if (n < 0) return printSocketReadError();

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
		if (p_number<1024) {
			ret = FAIL;
			sprintf(mess,"Port Number (%d) too low\n", p_number);
			cprintf(RED, "Warning: %s", mess);
		}
		printf("set port %d to %d\n",p_type, p_number);
		sd=bindSocket(p_number);
		if (sd<0) {
			ret = FAIL;
			sprintf(mess,"Could not bind port %d\n", p_number);
			cprintf(RED, "Warning: %s", mess);
			if (sd==-10) {
				ret = FAIL;
				sprintf(mess,"Port %d already set\n", p_number);
				cprintf(RED, "Warning: %s", mess);
			}
		}
		if (ret==OK && differentClients)
			ret=FORCE_UPDATE;
	}

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&p_number,sizeof(p_number),INT32);
		closeConnection(file_des);
		exitServer(sockfd);
		sockfd=sd;
	}

	// return ok / fail
	return ret;
}




int update_client(int file_des) {
	int ret=OK;
	sendData(file_des,&ret,sizeof(ret),INT32);
	return send_update(file_des);
}




int send_update(int file_des) {
	int n=0;	// if (n<0) should fail to stop talking to a closed client socket
	int nm=0;
	int64_t retval = 0;
	enum detectorSettings t;

	n = sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
	if (n < 0) return printSocketReadError();


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=setNMod(GET_FLAG,X);
#endif
	n = sendData(file_des,&nm,sizeof(nm),INT32);
	if (n < 0) return printSocketReadError();


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=setNMod(GET_FLAG,Y);
#endif
	n = sendData(file_des,&nm,sizeof(nm),INT32);
	if (n < 0) return printSocketReadError();


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=setDynamicRange(GET_FLAG);
#endif
	n = sendData(file_des,&nm,sizeof(nm),INT32);
	if (n < 0) return printSocketReadError();


#ifdef SLS_DETECTOR_FUNCTION_LIST
	dataBytes=calculateDataBytes();
#endif
	n = sendData(file_des,&dataBytes,sizeof(dataBytes),INT32);
	if (n < 0) return printSocketReadError();


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	t=setSettings(GET_SETTINGS, GET_FLAG);
#endif
	n = sendData(file_des,&t,sizeof(t),INT32);
	if (n < 0) return printSocketReadError();


#if defined(MYTHEND) || defined(EIGERD)
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=getThresholdEnergy(GET_FLAG);
#endif
	n = sendData(file_des,&nm,sizeof(nm),INT32);
	if (n < 0) return printSocketReadError();
#endif


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(FRAME_NUMBER,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(ACQUISITION_TIME,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();


#ifdef EIGERD
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(SUBFRAME_ACQUISITION_TIME,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();

#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(SUBFRAME_DEADTIME,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(FRAME_PERIOD,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();


#ifndef EIGERD
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(DELAY_AFTER_TRIGGER,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif


#if !defined(EIGERD) && !defined(JUNGFRAUD)
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(GATES_NUMBER,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif


#ifdef MYTHEND
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(PROBES_NUMBER,GET_FLAG);
#endif
	n = sendData(file_des,&retval,sizeof(int64_t),INT64);
	if (n < 0) return printSocketReadError();
#endif


#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(CYCLES_NUMBER,GET_FLAG);
#endif
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

#ifdef MYTHEND
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	strcpy(mess,"Function (Configure MAC) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// receive arguments
	char arg[6][50];
	memset(arg,0,sizeof(arg));
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
		printf("Configuring MAC of module %d at port %x\n", imod, udpport);

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
#endif
	if (differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n += sendData(file_des,&retval,sizeof(retval),INT32);
#ifdef EIGERD
		char arg[2][50];
		memset(arg,0,sizeof(arg));
		sprintf(arg[0],"%llx",(long long unsigned int)idetectormacadd);
        sprintf(arg[1],"%x",detipad);
        n += sendData(file_des,arg,sizeof(arg),OTHER);
#endif
	}
	// return ok / fail
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
	char ImageVals[dataBytes];
	memset(ImageVals,0,dataBytes);
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}





int set_master(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum masterFlags arg=GET_MASTER;
	enum masterFlags retval=GET_MASTER;
	sprintf(mess,"set master failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && ((int)arg!=(int)GET_MASTER)) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
#ifdef VERBOSE
		printf("setting master flags  to %d\n",arg);
#endif
		retval=setMaster(arg);
		if (retval==GET_MASTER)
			ret=FAIL;

		if (ret==OK && differentClients)
			ret=FORCE_UPDATE;
	}
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}







int set_synchronization(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	enum synchronizationMode arg=GET_SYNCHRONIZATION_MODE;
	enum synchronizationMode retval=GET_SYNCHRONIZATION_MODE;
	sprintf(mess,"synchronization mode failed\n");

	// receive arguments
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) return printSocketReadError();

	// execute action
	if (differentClients && lockStatus && ((int)arg!=(int)GET_SYNCHRONIZATION_MODE)) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
#ifdef VERBOSE
	printf("setting master flags  to %d\n",arg);
#endif
		retval=setSynchronization(arg);
		if (retval==GET_SYNCHRONIZATION_MODE)
			ret=FAIL;

		if (ret==OK && differentClients)
			ret=FORCE_UPDATE;
	}
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
	return ret;
}





int read_counter_block(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	char CounterVals[dataBytes];
	memset(CounterVals,0,dataBytes);
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,CounterVals,dataBytes,OTHER);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		ret=resetCounterBlock(startACQ);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif


	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		ret=calibratePedestal(frames);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if(ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
#ifdef VERBOSE
	printf("Enabling/Disabling 10Gbe :%d \n",arg);
#endif
		retval=enableTenGigabitEthernet(arg);
		if((arg != -1) && (retval != arg)) {
			ret=FAIL;
			cprintf(RED, "Warning: %s", mess);
		}
		else if (differentClients)
			ret=FORCE_UPDATE;
	}
#endif
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
				setSettings(UNDEFINED,-1);
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    n += sendData(file_des,mess,sizeof(mess),OTHER);

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

        n = sendDataOnly(file_des,&ret,sizeof(ret));
        if (ret==FAIL)
            n += sendDataOnly(file_des,mess,sizeof(mess));
        else
            n += sendDataOnly(file_des,&retval64,sizeof(retval64));
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

            n = sendDataOnly(file_des,&ret,sizeof(ret));
            if (ret==FAIL)
                n += sendDataOnly(file_des,mess,sizeof(mess));
            else {
                n += sendDataOnly(file_des,&start,sizeof(start));
                n += sendDataOnly(file_des,&stop,sizeof(stop));
                n += sendDataOnly(file_des,&nl,sizeof(nl));
            }
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
            n = sendDataOnly(file_des,&ret,sizeof(ret));
            if (ret==FAIL)
                n += sendDataOnly(file_des,mess,sizeof(mess));
            else {
                n += sendDataOnly(file_des,&retval,sizeof(retval));

            }


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

            n = sendDataOnly(file_des,&ret,sizeof(ret));
            if (ret==FAIL)
                n += sendDataOnly(file_des,mess,sizeof(mess));
            else
                n += sendDataOnly(file_des,&retval64,sizeof(retval64));

            break;



        case 4:
            n = receiveDataOnly(file_des,pat,sizeof(pat));
            for (addr=0; addr<1024; addr++)
                writePatternWord(addr,word);
            ret=OK;
            retval=0;
            n = sendDataOnly(file_des,&ret,sizeof(ret));
            if (ret==FAIL)
                n += sendDataOnly(file_des,mess,sizeof(mess));
            else
                n += sendDataOnly(file_des,&retval64,sizeof(retval64));

            break;



        default:
            ret=FAIL;
            printf(mess);
            sprintf(mess,"%s - wrong mode %d\n",mess, mode);
            n = sendDataOnly(file_des,&ret,sizeof(ret));
            n += sendDataOnly(file_des,mess,sizeof(mess));
    }


    // return ok / fail
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
	}
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
#ifdef VERBOSE
		printf("Getting/Setting/Resetting counter bit :%d \n",arg);
#endif
		retval=setCounterBit(arg);
		if((arg != -1) && (retval != arg)) {
			ret=FAIL;
			cprintf(RED, "Warning: %s", mess);
		}
		if (ret==OK && differentClients)
			ret=FORCE_UPDATE;
	}
#endif
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		ret=pulsePixel(arg[0],arg[1],arg[2]);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		ret=pulsePixelNMove(arg[0],arg[1],arg[2]);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if(ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		ret=pulseChip(arg);
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if(ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
				setSettings(UNDEFINED,-1);
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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

#ifdef SLS_DETECTOR_FUNCTION_LIST

	// execute action
	retval = getCurrentTau();
	printf("Getting rate correction %lld\n",(long long int)retval);

#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT64);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
#endif
		if (ret==OK)
			ret=FORCE_UPDATE;
	}
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {
		ret = prepareAcquisition();
		if (ret == FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if(ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}


int cleanup_acquisition(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	strcpy(mess,"prepare acquisition failed\n");

#ifndef GOTTHARDD
	//to receive any arguments
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);
	ret = FAIL;
	sprintf(mess,"Function (Cleanup Acquisition) is not implemented for this detector\n");
	cprintf(RED, "Warning: %s", mess);
#else

	// execute action
	if (differentClients && lockStatus) {
		ret = FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED, "Warning: %s", mess);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	else {//to be implemented when used here
		ret = FAIL;
		sprintf(mess,"Function (Cleanup Acquisition) is not implemented for this detector\n");
		cprintf(RED, "Warning: %s", mess);
	}
#endif
	if(ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
    int arg[2]={-1,-1};
    int val=-1;

    // receive arguments
    n = receiveData(file_des,arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();
    val=arg[0];
    //ignoring imod
    if (val > MAX_THRESHOLD_TEMP_VAL)   {
        ret=FAIL;
        sprintf(mess,"Threshold Temp %d should be in range: 0 - %d\n", val, MAX_THRESHOLD_TEMP_VAL);
        cprintf(RED, "Warning: %s", mess);
    }


#ifdef SLS_DETECTOR_FUNCTION_LIST
    if (ret==OK) {
#ifdef VERBOSE
    printf("Setting Threshold Temperature to  %d\n", val);
#endif
        retval=setThresholdTemperature(val);
    }
#endif
#ifdef VERBOSE
    printf("Threshold temperature is %d\n",  retval);
#endif

    if (ret==OK && differentClients && val >= 0)
        ret=FORCE_UPDATE;
#endif

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    if (ret!=FAIL) {
        n += sendData(file_des,&retval,sizeof(retval),INT32);
    } else {
        n += sendData(file_des,mess,sizeof(mess),OTHER);
    }

    // return ok / fail
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
    int arg[2]={-1,-1};
    int val=-1;

    // receive arguments
    n = receiveData(file_des,arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();
    val=arg[0];
    //ignoring imod


#ifdef SLS_DETECTOR_FUNCTION_LIST
    if (ret==OK) {
#ifdef VERBOSE
    printf("Setting Temperature control to  %d\n", val);
#endif
        retval=setTemperatureControl(val);
    }
#endif
#ifdef VERBOSE
    printf("Temperature control is %d\n",  retval);
#endif
    if (ret==OK && differentClients && val >= 0)
        ret=FORCE_UPDATE;
#endif

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    if (ret!=FAIL) {
        n += sendData(file_des,&retval,sizeof(retval),INT32);
    } else {
        n += sendData(file_des,mess,sizeof(mess),OTHER);
    }

    // return ok / fail
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
    int arg[2]={-1,-1};
    int val=-1;

    // receive arguments
    n = receiveData(file_des,arg,sizeof(arg),INT32);
    if (n < 0) return printSocketReadError();
    val=arg[0];
    //ignoring imod

#ifdef SLS_DETECTOR_FUNCTION_LIST
    if (ret==OK) {
#ifdef VERBOSE
    printf("Setting Temperature Event to  %d\n", val);
#endif
        retval=setTemperatureEvent(val);
    }
#endif
#ifdef VERBOSE
    printf("Temperature Event is %d\n",  retval);
#endif

    if (ret==OK && differentClients && val >= 0)
        ret=FORCE_UPDATE;
#endif

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    if (ret!=FAIL) {
        n += sendData(file_des,&retval,sizeof(retval),INT32);
    } else {
        n += sendData(file_des,mess,sizeof(mess),OTHER);
    }

    // return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
    if (ret==OK && differentClients)
        ret=FORCE_UPDATE;
#endif

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    if (ret==FAIL) {
        n += sendData(file_des,mess,sizeof(mess),OTHER);
    } else
        n += sendData(file_des,&retval,sizeof(retval),INT32);

    // return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
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
    if (ret==OK && differentClients)
        ret=FORCE_UPDATE;
#endif

    // ret could be swapped during sendData
    ret1 = ret;
    // send ok / fail
    n = sendData(file_des,&ret1,sizeof(ret),INT32);
    // send return argument
    if (ret==FAIL) {
        n += sendData(file_des,mess,sizeof(mess),OTHER);
    } else
        n += sendData(file_des,&retval,sizeof(retval),INT32);

    // return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST

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
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
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
#ifdef SLS_DETECTOR_FUNCTION_LIST
	 else {
		printf("Software Trigger\n");
		ret=softwareTrigger();
		if (ret==FAIL)
			cprintf(RED, "Warning: %s", mess);
	}
#endif
	if (ret==OK && differentClients)
		ret=FORCE_UPDATE;
#endif

	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}

