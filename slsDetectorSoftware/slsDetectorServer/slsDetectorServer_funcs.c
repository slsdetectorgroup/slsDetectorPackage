
#include "sls_detector_defs.h"
#include "slsDetectorServer_funcs.h"
#include "slsDetectorFunctionList.h"
#include "communication_funcs.h"


#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
int sockfd;
extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;



// Global variables
int (*flist[256])(int);
//defined in the detector specific file
#ifdef MYTHEND
const enum detectorType myDetectorType=MYTHEN;
#elif GOTTHARDD
const enum detectorType myDetectorType=GOTTHARD;
#elif EIGERD
const enum detectorType myDetectorType=EIGER;
#elif PICASSOD
const enum detectorType myDetectorType=PICASSO;
#else
const enum detectorType myDetectorType=GENERIC;
#endif

extern enum detectorSettings thisSettings;

//global variables for optimized readout
char mess[MAX_STR_LENGTH];
char *dataretval=NULL;
int dataret;
//extern 
int dataBytes = 10;



void checkFirmwareCompatibility(){
	cprintf(BLUE,"\n\n********************************************************\n"
			   "**********************EIGER Server**********************\n"
			   "********************************************************\n");
	cprintf(BLUE,"\nFirmware Version: %lld\nSoftware Version: %llx\n\n",
			getDetectorId(DETECTOR_FIRMWARE_VERSION), getDetectorId(DETECTOR_SOFTWARE_VERSION));

	//check for firmware version compatibility
	if(getDetectorId(DETECTOR_FIRMWARE_VERSION) < REQUIRED_FIRMWARE_VERSION){
		cprintf(RED,"FATAL ERROR: This firmware version is incompatible.\n"
				"Please update it to v%d to be compatible with this server\n\n",
				REQUIRED_FIRMWARE_VERSION);

		cprintf(RED,"Exiting Server. Goodbye!\n\n");
		exit(-1);
	}
}


int init_detector(int b) {
#ifdef VIRTUAL
	printf("This is a VIRTUAL detector\n");
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if(b)	initDetector();
	else initDetectorStop();
#endif
	strcpy(mess,"dummy message");
	strcpy(lastClientIP,"none");
	strcpy(thisClientIP,"none1");
	lockStatus=0;
	return OK;
}


int decode_function(int file_des) {
	int fnum,n;
	int ret=FAIL;
#ifdef VERBOSE
	printf( "receive data\n");
#endif
	n = receiveData(file_des,&fnum,sizeof(fnum),INT32);
	if (n <= 0) {
#ifdef VERBOSE
		printf("ERROR reading from socket %d, %d %d\n", n, fnum, file_des);
#endif
		return FAIL;
	}
#ifdef VERBOSE
	else
		printf("size of data received %d\n",n);
#endif

#ifdef VERBOSE
	printf( "calling function fnum = %d %x\n",fnum,(unsigned int)flist[fnum]);
#endif
	if (fnum<0 || fnum>255)
		fnum=255;
	ret=(*flist[fnum])(file_des);
	if (ret==FAIL)
		cprintf( RED, "Error executing the function = %d \n",fnum);
	return ret;
}


int function_table() {
	int i;
	for (i=0;i<256;i++){
		flist[i]=&M_nofunc;
	}
	flist[F_EXIT_SERVER]=&exit_server;
	flist[F_EXEC_COMMAND]=&exec_command;

	flist[F_LOCK_SERVER]=&lock_server;
	flist[F_GET_LAST_CLIENT_IP]=&get_last_client_ip;
	flist[F_SET_PORT]=&set_port;
	flist[F_UPDATE_CLIENT]=&update_client;
	flist[F_SET_MASTER]=&set_master;
	flist[F_SET_SYNCHRONIZATION_MODE]=&set_synchronization;

	//F_GET_ERROR
	flist[F_GET_DETECTOR_TYPE]=&get_detector_type;
	flist[F_SET_NUMBER_OF_MODULES]=&set_number_of_modules;
	flist[F_GET_MAX_NUMBER_OF_MODULES]=&get_max_number_of_modules;
	flist[F_SET_EXTERNAL_SIGNAL_FLAG]=&set_external_signal_flag;
	flist[F_SET_EXTERNAL_COMMUNICATION_MODE]=&set_external_communication_mode;
	flist[F_GET_ID]=&get_id;
	flist[F_DIGITAL_TEST]=&digital_test;
	//F_ANALOG_TEST
	//F_ENABLE_ANALOG_OUT
	//F_CALIBRATION_PULSE
	flist[F_SET_DAC]=&set_dac;
	flist[F_GET_ADC]=&get_adc;
	flist[F_WRITE_REGISTER]=&write_register;
	flist[F_READ_REGISTER]=&read_register;
	//F_WRITE_MEMORY
	//F_READ_MEMORY
	flist[F_SET_CHANNEL]=&set_channel;
	flist[F_GET_CHANNEL]=&get_channel;
	//F_SET_ALL_CHANNELS
	flist[F_SET_CHIP]=&set_chip;
	flist[F_GET_CHIP]=&get_chip;
	//F_SET_ALL_CHIPS
	flist[F_SET_MODULE]=&set_module;
	flist[F_GET_MODULE]=&get_module;
	//F_SET_ALL_MODULES
	flist[F_SET_SETTINGS]=&set_settings;
	flist[F_GET_THRESHOLD_ENERGY]=&get_threshold_energy;
	flist[F_SET_THRESHOLD_ENERGY]=&set_threshold_energy;
	flist[F_START_ACQUISITION]=&start_acquisition;
	flist[F_STOP_ACQUISITION]=&stop_acquisition;
	flist[F_START_READOUT]=&start_readout;
	flist[F_GET_RUN_STATUS]=&get_run_status;
	flist[F_START_AND_READ_ALL]=&start_and_read_all;
	flist[F_READ_FRAME]=&read_frame;
	flist[F_READ_ALL]=&read_all;
	flist[F_SET_TIMER]=&set_timer;
	flist[F_GET_TIME_LEFT]=&get_time_left;
	flist[F_SET_DYNAMIC_RANGE]=&set_dynamic_range;
	flist[F_SET_READOUT_FLAGS]=&set_readout_flags;
	flist[F_SET_ROI]=&set_roi;
	flist[F_SET_SPEED]=&set_speed;
	flist[F_EXECUTE_TRIMMING]=&execute_trimming;
	flist[F_CONFIGURE_MAC]=&configure_mac;
	flist[F_LOAD_IMAGE]=&load_image;
	flist[F_READ_COUNTER_BLOCK]=&read_counter_block;
	flist[F_RESET_COUNTER_BLOCK]=&reset_counter_block;
	flist[F_START_RECEIVER]=&start_receiver;
	flist[F_STOP_RECEIVER]=&stop_receiver;
	flist[F_CALIBRATE_PEDESTAL]=&calibrate_pedestal;
	flist[F_ENABLE_TEN_GIGA]=&enable_ten_giga;
	flist[F_SET_ALL_TRIMBITS]=&set_all_trimbits;
	flist[F_SET_COUNTER_BIT]=&set_counter_bit;
	flist[F_PULSE_PIXEL]=&pulse_pixel;
	flist[F_PULSE_PIXEL_AND_MOVE]=&pulse_pixel_and_move;
	flist[F_PULSE_CHIP]=&pulse_chip;
	flist[F_SET_RATE_CORRECT]=&set_rate_correct;
	flist[F_GET_RATE_CORRECT]=&get_rate_correct;


#ifdef VERBOSE
	/*  for (i=0;i<256;i++){
    printf("function %d located at %x\n",i,flist[i]);
    }*/
#endif
	return OK;
}


int  M_nofunc(int file_des){

	int ret=FAIL;
	sprintf(mess,"Unrecognized Function\n");
	printf(mess);
	sendData(file_des,&ret,sizeof(ret),INT32);
	sendData(file_des,mess,sizeof(mess),OTHER);
	return GOODBYE;
}


int exit_server(int file_des) {
	int ret=FAIL;
	sendData(file_des,&ret,sizeof(ret),INT32);
	printf("closing server.");
	sprintf(mess,"closing server");
	sendData(file_des,mess,sizeof(mess),OTHER);
	return GOODBYE;
}

int exec_command(int file_des) {
	char cmd[MAX_STR_LENGTH];
	char answer[MAX_STR_LENGTH];
	int ret=OK,ret1=OK;
	int sysret=0;
	int n=0;

	/* receive arguments */
	n = receiveData(file_des,cmd,MAX_STR_LENGTH,OTHER);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	/* execute action if the arguments correctly arrived*/
	if (ret==OK) {
#ifdef VERBOSE
		printf("executing command %s\n", cmd);
#endif
		if (lockStatus==0 || differentClients==0)
			sysret=system(cmd);

		//should be replaced by popen
		if (sysret==0) {
			sprintf(answer,"Succeeded\n");
			if (lockStatus==1 && differentClients==1)
				sprintf(answer,"Detector locked by %s\n", lastClientIP);
		} else {
			sprintf(answer,"Failed\n");
			ret=FAIL;
		}
	} else {
		sprintf(answer,"Could not receive the command\n");
	}

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	n = sendData(file_des,answer,MAX_STR_LENGTH,OTHER);
	if (n < 0) {
		sprintf(mess,"Error writing to socket");
		ret=FAIL;
	}


	/*return ok/fail*/
	return ret;

}





int lock_server(int file_des) {


	int n;
	int ret=OK,ret1=OK;


	int lock;
	n = receiveData(file_des,&lock,sizeof(lock),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (lock)\n");
		ret=FAIL;
	}
	if (lock>=0) {
		if (lockStatus==0 || strcmp(lastClientIP,thisClientIP)==0 || strcmp(lastClientIP,"none")==0) {
			lockStatus=lock;
			strcpy(lastClientIP,thisClientIP);
		}   else {
			ret=FAIL;
			sprintf(mess,"Server already locked by %s\n", lastClientIP);
		}
	}
	if (differentClients && ret==OK)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	}  else
		n = sendData(file_des,&lockStatus,sizeof(lockStatus),INT32);

	return ret;

}




int get_last_client_ip(int file_des) {
	int ret=OK,ret1=OK;
	if (differentClients )
		ret=FORCE_UPDATE;
	sendData(file_des,&ret1,sizeof(ret),INT32);
	sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
	return ret;
}




int set_port(int file_des) {
	int n;
	int ret=OK,ret1=OK;
	int sd=-1;

	enum portType p_type; /** data? control? stop? Unused! */
	int p_number; /** new port number */

	n = receiveData(file_des,&p_type,sizeof(p_type),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (ptype)\n");
		ret=FAIL;
	}

	n = receiveData(file_des,&p_number,sizeof(p_number),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (pnum)\n");
		ret=FAIL;
	}
	if (differentClients==1 && lockStatus==1 ) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		if (p_number<1024) {
			sprintf(mess,"Too low port number %d\n", p_number);
			printf("\n");
			ret=FAIL;
		}

		printf("set port %d to %d\n",p_type, p_number);

		sd=bindSocket(p_number);
	}
	if (sd>=0) {
		ret=OK;
		if (differentClients )
			ret=FORCE_UPDATE;
	} else {
		ret=FAIL;
		sprintf(mess,"Could not bind port %d\n", p_number);
		printf("Could not bind port %d\n", p_number);
		if (sd==-10) {
			sprintf(mess,"Port %d already set\n", p_number);
			printf("Port %d already set\n", p_number);

		}
	}
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&p_number,sizeof(p_number),INT32);
		closeConnection(file_des);
		exitServer(sockfd);
		sockfd=sd;

	}

	return ret;

}



int send_update(int file_des) {

	enum detectorSettings t;
	int n = 0, nm = 0;
	int64_t retval = 0;

	n += sendData(file_des,lastClientIP,sizeof(lastClientIP),OTHER);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=setNMod(GET_FLAG,X);
#endif
	n += sendData(file_des,&nm,sizeof(nm),INT32);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=setNMod(GET_FLAG,Y);
#endif
	n += sendData(file_des,&nm,sizeof(nm),INT32);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=setDynamicRange(GET_FLAG);
#endif
	n += sendData(file_des,&nm,sizeof(nm),INT32);
	nm = dataBytes;
	n += sendData(file_des,&nm,sizeof(nm),INT32);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	t=setSettings(GET_SETTINGS, GET_FLAG);
#endif
	n += sendData(file_des,&t,sizeof(t),INT32);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	nm=getThresholdEnergy(GET_FLAG);
#endif
	n += sendData(file_des,&nm,sizeof(nm),INT32);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(FRAME_NUMBER,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(ACQUISITION_TIME,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(SUBFRAME_ACQUISITION_TIME,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(FRAME_PERIOD,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(DELAY_AFTER_TRIGGER,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(GATES_NUMBER,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);
/*	retval=setTimer(PROBES_NUMBER,GET_FLAG);
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);*/
#ifdef	SLS_DETECTOR_FUNCTION_LIST
	retval=setTimer(CYCLES_NUMBER,GET_FLAG);
#endif
	n += sendData(file_des,&retval,sizeof(int64_t),INT64);

	if (lockStatus==0) {
		strcpy(lastClientIP,thisClientIP);
	}

	return OK;

}


int update_client(int file_des) {

	int ret=OK;
	sendData(file_des,&ret,sizeof(ret),INT32);
	return send_update(file_des);

}




int set_master(int file_des) {

	enum masterFlags retval=GET_MASTER;
	enum masterFlags arg;
	int n;
	int ret=OK,ret1=OK;
	// int regret=OK;


	sprintf(mess,"can't set master flags\n");


	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef VERBOSE
	printf("setting master flags  to %d\n",arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && ((int)arg!=(int)GET_READOUT_FLAGS)) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		retval=setMaster(arg);

	}
#endif
	if (retval==GET_MASTER) {
		ret=FAIL;
	}

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT32);
	}
	return ret;
}






int set_synchronization(int file_des) {

	enum synchronizationMode retval=GET_SYNCHRONIZATION_MODE;
	enum synchronizationMode arg;
	int n;
	int ret=OK,ret1=OK;
	//int regret=OK;


	sprintf(mess,"can't set synchronization mode\n");


	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifdef VERBOSE
	printf("setting master flags  to %d\n",arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && ((int)arg!=(int)GET_READOUT_FLAGS)) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		retval=setSynchronization(arg);
	}
#endif

	if (retval==GET_SYNCHRONIZATION_MODE) {
		ret=FAIL;
	}

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT32);
	}
	return ret;
}







int get_detector_type(int file_des) {
	int n=0;
	enum detectorType retval;
	int ret=OK,ret1=OK;

	sprintf(mess,"Can't return detector type\n");


	/* receive arguments */
	/* execute action */
	retval=myDetectorType;

#ifdef VERBOSE
	printf("Returning detector type %d\n",retval);
#endif

	/* send answer */
	/* send OK/failed */
	if (differentClients==1)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n += sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	/*return ok/fail*/
	return ret;


}


int set_number_of_modules(int file_des) {
	int n;
	int arg[2], retval=0;
	int ret=OK,ret1=OK;
	int nm;
#ifdef SLS_DETECTOR_FUNCTION_LIST
	enum dimension dim;
#endif
	sprintf(mess,"Can't set number of modules\n");

	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket %d", n);
		ret=GOODBYE;
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		dim=arg[0];
		nm=arg[1];

		/* execute action */
#ifdef VERBOSE
		printf("Setting the number of modules in dimension %d to %d\n",dim,nm );
#endif
		if (lockStatus==1 && differentClients==1 && nm!=GET_FLAG) {
			sprintf(mess,"Detector locked by %s\n", lastClientIP);
			ret=FAIL;
		} else
			retval=setNMod(nm, dim);
	}
	dataBytes=calculateDataBytes();
#endif

	if (retval==nm || nm==GET_FLAG) {
		ret=OK;
		if (differentClients==1)
			ret=FORCE_UPDATE;
	} else
		ret=FAIL;

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;
}


int get_max_number_of_modules(int file_des) {
	int n;
	int retval;
	int ret=OK,ret1=OK;
	enum dimension arg;

	sprintf(mess,"Can't get max number of modules\n");
	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	/* execute action */
#ifdef VERBOSE
	printf("Getting the max number of modules in dimension %d \n",arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST

	retval=getNModBoard(arg);
#endif
#ifdef VERBOSE
	printf("Max number of module in dimension %d is %d\n",arg,retval );
#endif  

	if (differentClients==1 && ret==OK) {
		ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;
}


//index 0 is in gate
//index 1 is in trigger
//index 2 is out gate
//index 3 is out trigger

int set_external_signal_flag(int file_des) {
	int n;
	int arg[2];
	int ret=OK,ret1=OK;
	enum externalSignalFlag retval=SIGNAL_OFF;
#ifdef SLS_DETECTOR_FUNCTION_LIST
	int signalindex;
	enum externalSignalFlag flag;
#endif

	sprintf(mess,"Can't set external signal flag\n");

	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		signalindex=arg[0];
		flag=arg[1];
		/* execute action */
		switch (flag) {
		case GET_EXTERNAL_SIGNAL_FLAG:
			retval=getExtSignal(signalindex);
			break;
		default:
			if (differentClients==0 || lockStatus==0) {
				retval=setExtSignal(signalindex,flag);
				if (retval!=flag) {
					ret=FAIL;
					sprintf(mess,"External signal %d flag should be 0x%04x but is 0x%04x\n", signalindex, flag, retval);
				}

			} else if (lockStatus!=0) {
					ret=FAIL;
					sprintf(mess,"Detector locked by %s\n", lastClientIP);
			}
			break;
		}
#ifdef VERBOSE
		printf("Setting external signal %d to flag %d\n",signalindex,flag );
		printf("Set to flag %d\n",retval);
#endif
	} else {
		ret=FAIL;
	}
#endif

	if (ret==OK && differentClients!=0)
		ret=FORCE_UPDATE;


	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}


	/*return ok/fail*/
	return ret;

}


int set_external_communication_mode(int file_des) {
	int n;
	enum externalCommunicationMode arg, retval=GET_EXTERNAL_COMMUNICATION_MODE;
	int ret=OK,ret1=OK;

	sprintf(mess,"Can't set external communication mode\n");


	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	/*
enum externalCommunicationMode{
  GET_EXTERNAL_COMMUNICATION_MODE,
  AUTO,
  TRIGGER_EXPOSURE_SERIES,
  TRIGGER_EXPOSURE_BURST,
  TRIGGER_READOUT,
  TRIGGER_COINCIDENCE_WITH_INTERNAL_ENABLE,
  GATE_FIX_NUMBER,
  GATE_FIX_DURATION,
  GATE_WITH_START_TRIGGER,
 // GATE_COINCIDENCE_WITH_INTERNAL_ENABLE,
  BURST_TRIGGER
};
	 */
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		/* execute action */
		switch(arg){
#ifdef EIGERD
		case GET_EXTERNAL_COMMUNICATION_MODE:
		case AUTO_TIMING:
		case TRIGGER_EXPOSURE:
		case BURST_TRIGGER:
		case GATE_FIX_NUMBER:
			break;
#endif
		default:
			ret = FAIL;
			sprintf(mess,"This timing mode %d not implemented in this detector\n",(int)arg);
			break;
		}
	}
	if (ret==OK) {
#ifdef VERBOSE
		printf("Setting external communication mode to %d\n", arg);
#endif
		retval=setTiming(arg);

		if (differentClients==1)
			ret=FORCE_UPDATE;
	}
#endif


	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;


}



int get_id(int file_des) {
	// sends back 64 bits!
	int64_t retval;
	int ret=OK,ret1=OK;
#ifndef EIGERD
	int imod=-1;
#endif
	int n=0;
	enum idMode arg;

	sprintf(mess,"Can't return id\n");

	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Getting id %d\n", arg);
#endif

	switch (arg) {
#ifndef EIGERD
	case  MODULE_SERIAL_NUMBER:
	case MODULE_FIRMWARE_VERSION:
		n = receiveData(file_des,&imod,sizeof(imod),INT32);
		if (n < 0) {
			sprintf(mess,"Error reading from socket\n");
			ret=FAIL;
		}
#ifdef SLS_DETECTOR_FUNCTION_LIST
		if (ret==OK) {
#ifdef VERBOSE
			printf("of module %d\n", imod);
#endif
			if (imod>=0 && imod<getTotalNumberOfModules())
				retval=getModuleId(arg, imod);
			else {
				ret=FAIL;
				sprintf(mess,"Module number %d out of range\n", imod);
				printf("%s\n",mess);
			}
		}
#endif
		break;
#endif
	case DETECTOR_SERIAL_NUMBER:
	case DETECTOR_FIRMWARE_VERSION:
	case DETECTOR_SOFTWARE_VERSION:
#ifdef SLS_DETECTOR_FUNCTION_LIST
		retval=getDetectorId(arg);
#endif
		break;
	default:
		printf("Required unknown id %d \n", arg);
		ret=FAIL;
		retval=FAIL;
		break;
	}

#ifdef VERBOSE
	printf("ret is %d\n",ret);
	printf("Id is %llx\n", retval);
#endif

	if (differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT64);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;

}

int digital_test(int file_des) {

	int retval=-1;
	int ret=OK,ret1=OK;
	int imod=-1;
	int n=0;
	int ival;
	enum digitalTestMode arg;

	sprintf(mess,"Can't send digital test\n");

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Digital test mode %d\n",arg );
#endif  

	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		switch (arg) {
		case  CHIP_TEST:
			n = receiveData(file_des,&imod,sizeof(imod),INT32);
			if (n < 0) {
				sprintf(mess,"Error reading from socket\n");
				retval=FAIL;
			}
#ifdef VERBOSE
			printf("of module %d\n", imod);
#endif   
#ifndef MYTHEND
			ret = FAIL;
			strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
			if (imod>=0 && imod<getTotalNumberOfModules())
				retval=moduleTest(arg,imod);
			else {
				ret=FAIL;
				sprintf(mess,"Module number %d out of range\n", imod);
			}
#endif
#endif
			break;
		case MODULE_FIRMWARE_TEST:
		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_MEMORY_TEST:
		case DETECTOR_BUS_TEST:
#ifndef MYTHEND
			ret = FAIL;
			strcpy(mess,"Not applicable/implemented for this detector\n");
			break;
#endif
		case DETECTOR_SOFTWARE_TEST:
#ifdef SLS_DETECTOR_FUNCTION_LIST
			retval=detectorTest(arg);
#endif
			break;
		case DIGITAL_BIT_TEST:
			n = receiveData(file_des,&ival,sizeof(ival),INT32);
			if (n < 0) {
				sprintf(mess,"Error reading from socket\n");
				retval=FAIL;
			}
#ifndef GOTTHARDD
			ret = FAIL;
			strcpy(mess,"Not applicable/implemented for this detector\n");
#else
			retval=0;
#endif
			break;
		default:
			printf("Unknown digital test required %d\n",arg);
			ret=FAIL;
			retval=FAIL;
			break;
		}
	}
#ifdef VERBOSE
	printf("digital test result is 0x%x\n", retval);
#endif  
	//Always returns force update such that the dynamic range is always updated on the client
	// if (differentClients==1 && ret==OK)
	ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;

}






int set_dac(int file_des) {

	int retval[2];retval[1]=-1;
	int temp;
	int ret=OK,ret1=OK;
	int arg[3];
	enum dacIndex ind;
	int imod;
	int n;
	int val;
	int mV;
	enum detDacIndex idac=0;

	sprintf(mess,"Can't set DAC\n");


	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	imod=arg[1];
	mV=arg[2];

	n = receiveData(file_des,&val,sizeof(val),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}
#endif

	// check if dac exists for this detector
	switch (ind) {
#ifdef MYTHEND
	case  TRIMBIT_SIZE:			//ind = VTRIM;
			break;
	case THRESHOLD:
			break;
	case SHAPER1:
			break;
	case SHAPER2:
			break;
	case CALIBRATION_PULSE:
			break;
	case PREAMP:
			break;
#endif
#ifdef GOTTHARDD
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
#endif
#ifdef EIGERD
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
#endif
	default:
		printf("Unknown DAC index %d\n",(int)ind);
		sprintf(mess,"Unknown DAC index %d for this detector\n",ind);
		ret=FAIL;
		break;
	}
#ifdef VERBOSE
		printf("Setting DAC %d of module %d to %d \n", idac, imod, val);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1 && val!=-1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
			if((ind == HV_POT) ||(ind == HV_NEW))
				retval[0] = setHighVoltage(val,imod);
			else if(ind == IO_DELAY)
				retval[0] = setIODelay(val,imod);
			else
				setDAC(idac,val,imod,mV,retval);
		}



	}
#endif
#ifdef VERBOSE
	printf("DAC set to %d in dac units and %d mV\n",  retval[0],retval[1]);
#endif

	if(ret == OK){
		if(mV)
			temp = retval[1];
		else
			temp = retval[0];
		if ((abs(temp-val)<=5) || val==-1) {
			ret=OK;
			if (differentClients)
				ret=FORCE_UPDATE;
		} else {
			ret=FAIL;
			printf("Setting dac %d of module %d: wrote %d but read %d\n", idac, imod, val, temp);
		}
	}


	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/* Maybe this is done inside the initialization funcs */
	//detectorDacs[imod][ind]=val;
	/*return ok/fail*/
	return ret;

}



int get_adc(int file_des) {

	int retval=-1;
	int ret=OK,ret1=OK;
	int arg[2];
	enum dacIndex ind;
	int imod;
	int n;
	enum detDacIndex idac=0;

	sprintf(mess,"Can't read ADC\n");


	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	imod=arg[1];

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules() || imod<0) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}
#endif

	switch (ind) {
#ifdef EIGERD
	case TEMPERATURE_FPGA:	//dac = TEMP_FPGA;
		retval=getBebFPGATemp();
		printf("Temperature: %d°C\n",retval);
		break;
#endif
#ifdef GOTTHARDD
	case TEMPERATURE_FPGA:	//dac = TEMP_FPGA;
		break;
	case TEMPERATURE_ADC:
		break;
#endif
	default:
		printf("Unknown DAC index %d\n",ind);
		ret=FAIL;
		sprintf(mess,"Unknown ADC index %d\n",ind);
		break;
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if ((ret==OK) && (retval==-1)) {
		retval=getADC(idac,imod);
	}
#endif
#ifdef VERBOSE
	printf("Getting ADC %d of module %d\n", idac, imod);
#endif 

#ifdef VERBOSE
	printf("ADC is %f V\n",  retval);
#endif  
	if (ret==FAIL) {
		printf("Getting adc %d of module %d failed\n", idac, imod);
	}


	if (differentClients)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;

}






int write_register(int file_des) {

	int retval;
	int ret=OK,ret1=OK;
	int arg[2];
	int addr, val;
	int n;


	sprintf(mess,"Can't write to register\n");

	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	addr=arg[0];
	val=arg[1];

#if defined(MYTHEND) || defined(GOTTHARDD)
#ifdef VERBOSE
	printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else
		retval=writeRegister(addr,val);
#endif
#ifdef VERBOSE
	printf("Data set to 0x%x\n",  retval);
#endif
#else
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#endif
	if (retval==val) {
		ret=OK;
		if (differentClients)
			ret=FORCE_UPDATE;
	} else {
		ret=FAIL;
		sprintf(mess,"Writing to register 0x%x failed: wrote 0x%x but read 0x%x\n", addr, val, retval);
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;

}

int read_register(int file_des) {

	int retval;
	int ret=OK,ret1=OK;
	int arg;
	int addr;
	int n;


	sprintf(mess,"Can't read register\n");

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	addr=arg;

#if defined(MYTHEND) || defined(GOTTHARDD)
#ifdef VERBOSE
	printf("reading  register 0x%x\n", addr);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	retval=readRegister(addr);
#endif
#ifdef VERBOSE
	printf("Returned value 0x%x\n",  retval);
#endif
#else
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#endif

	if (ret==FAIL) {
		printf("Reading register 0x%x failed\n", addr);
	} else if (differentClients)
		ret=FORCE_UPDATE;


	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;

}






int set_channel(int file_des) {
	int ret=OK,ret1=OK;
	sls_detector_channel myChan;
	int retval;
	int n;


	sprintf(mess,"Can't set channel\n");

#ifdef VERBOSE
	printf("Setting channel\n");
#endif
	ret=receiveChannel(file_des, &myChan);
	if (ret>=0)
		ret=OK;
	else
		ret=FAIL;
#ifndef MYTHEND
			ret = FAIL;
			strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef VERBOSE
	printf("channel number is %d, chip number is %d, module number is %d, register is %lld\n", myChan.chan,myChan.chip, myChan.module, myChan.reg);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (myChan.chan>=getNumberOfChannelsPerChip()) {
		ret=FAIL;
		sprintf(mess, "channel number %d too large!\n",myChan.chan);
	}
	if (myChan.chip>=getNumberOfChipsPerModule()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",myChan.chip);
	}

	if (myChan.module>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",myChan.module);
	}


	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else {
			retval=setChannel(myChan);
		}
	}
#endif
#endif
	/* Maybe this is done inside the initialization funcs */
	//copyChannel(detectorChans[myChan.module][myChan.chip]+(myChan.chan), &myChan);

	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}


	/*return ok/fail*/
	return ret;

}




int get_channel(int file_des) {

	int ret=OK,ret1=OK;
	sls_detector_channel retval;

	int arg[3];
#ifdef MYTHEND
	int ichan, ichip, imod;
#endif
	int n;

	sprintf(mess,"Can't get channel\n");



	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifndef MYTHEND
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
	ichan=arg[0];
	ichip=arg[1];
	imod=arg[2];
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ichan>=getNumberOfChannelsPerChip()) {
		ret=FAIL;
		sprintf(mess, "channel number %d too large!\n",ichan);
	} else
		retval.chan=ichan;
	if (ichip>=getNumberOfChipsPerModule()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",ichip);
	} else
		retval.chip=ichip;

	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",imod);
	} else
		retval.module=imod;


	if (ret==OK)
		ret=getChannel(&retval);
#endif
#endif
	if (differentClients && ret==OK)
		ret=FORCE_UPDATE;

#ifdef MYTHEND
#ifdef VERBOSE
	printf("Returning channel %d %d %d, 0x%llx\n", retval.chan, retval.chip, retval.mod, (retval.reg));
#endif 
#endif

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendChannel(file_des, &retval);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}



	/*return ok/fail*/
	return ret;


}


int set_chip(int file_des) {

	int *ch;
	int n, retval;
	int ret=OK,ret1=OK;
#ifdef SLS_DETECTOR_FUNCTION_LIST
	sls_detector_chip myChip;

	myChip.nchan=getNumberOfChannelsPerChip();
	ch=(int*)malloc((myChip.nchan)*sizeof(int));
	myChip.chanregs=ch;



#ifdef VERBOSE
	printf("Setting chip\n");
#endif
	ret=receiveChip(file_des, &myChip);

#ifndef MYTHEND
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef VERBOSE
	printf("Chip received\n");
#endif
	if (ret>=0)
		ret=OK;
	else
		ret=FAIL;
#ifdef VERBOSE
	printf("chip number is %d, module number is %d, register is %d, nchan %d\n",myChip.chip, myChip.module, myChip.reg, myChip.nchan);
#endif

	if (myChip.chip>=getNumberOfChipsPerModule()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",myChip.chip);
	}

	if (myChip.module>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",myChip.module);
	}


	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		retval=setChip(myChip);
	}
#endif
#endif
	/* Maybe this is done inside the initialization funcs */
	//copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

	if (differentClients && ret==OK)
		ret=FORCE_UPDATE;
	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	free(ch);

	return ret;
}




int get_chip(int file_des) {


	int ret=OK,ret1=OK;
	sls_detector_chip retval;
	int arg[2];
	int n, *ch;
#ifdef MYTHEND
	int  ichip, imod;
#endif


#ifdef SLS_DETECTOR_FUNCTION_LIST
	retval.nchan=getNumberOfChannelsPerChip();
	ch=(int*)malloc((retval.nchan)*sizeof(int));
	retval.chanregs=ch;
#endif

	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifndef MYTHEND
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
	ichip=arg[0];
	imod=arg[1];
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ichip>=getNumberOfChipsPerModule()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",ichip);
	} else
		retval.chip=ichip;

	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess, "chip number %d too large!\n",imod);
	} else
		retval.module=imod;

	if (ret==OK)
		ret=getChip(&retval);
#endif

#endif
	if (differentClients && ret==OK)
		ret=FORCE_UPDATE;
#ifdef MYTHEND
#ifdef VERBOSE
	printf("Returning chip %d %d\n",  ichip, imod);
#endif 
#endif

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendChip(file_des, &retval);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	free(ch);

	/*return ok/fail*/
	return ret;


}





int set_module(int file_des) {
	int retval, n;
	int ret=OK,ret1=OK;

	strcpy(mess,"could not set module.");

#ifdef SLS_DETECTOR_FUNCTION_LIST
	sls_detector_module myModule;
#ifdef EIGERD
	int *myGain = (int*)malloc(getNumberOfGainsPerModule()*sizeof(int));
	int *myOffset = (int*)malloc(getNumberOfOffsetsPerModule()*sizeof(int));
	int *myIODelay = (int*)malloc(sizeof(int));
	int64_t myTau=-1;
#endif
	int *myChip=(int*)malloc(getNumberOfChipsPerModule()*sizeof(int));
	int *myChan=(int*)malloc(getNumberOfChannelsPerModule()*sizeof(int));
	int *myDac=(int*)malloc(getNumberOfDACsPerModule()*sizeof(int));
	int *myAdc=(int*)malloc(getNumberOfADCsPerModule()*sizeof(int));


	if (myDac)
		myModule.dacs=myDac;
	else {
		sprintf(mess,"could not allocate dacs\n");
		ret=FAIL;
	}
	if (myAdc)
		myModule.adcs=myAdc;
	else {
		sprintf(mess,"could not allocate adcs\n");
		ret=FAIL;
	}
	if (myChip)
		myModule.chipregs=myChip;
	else {
		sprintf(mess,"could not allocate chips\n");
		ret=FAIL;
	}
	if (myChan)
		myModule.chanregs=myChan;
	else {
		sprintf(mess,"could not allocate chans\n");
		ret=FAIL;
	}
#ifdef EIGERD
	if (!myGain){
		sprintf(mess,"could not allocate gains\n");
		ret=FAIL;
	}
	if (!myOffset){
		sprintf(mess,"could not allocate offsets\n");
		ret=FAIL;
	}
#endif
	myModule.nchip=getNumberOfChipsPerModule();
	myModule.nchan=getNumberOfChannelsPerModule();
	myModule.ndac=getNumberOfDACsPerModule();
	myModule.nadc=getNumberOfADCsPerModule();


#ifdef VERBOSE
	printf("Setting module\n");
#endif
	ret=receiveModule(file_des, &myModule);
#ifdef EIGERD
	n = receiveData(file_des,myGain,sizeof(int)*getNumberOfGainsPerModule(),INT32);
	n = receiveData(file_des,myOffset,sizeof(int)*getNumberOfOffsetsPerModule(),INT32);
	n = receiveData(file_des,myIODelay,sizeof(int),INT32);
	n = receiveData(file_des,&myTau,sizeof(myTau),INT64);
#endif
	if (ret>=0)
		ret=OK;
	else
		ret=FAIL;


#ifdef VERBOSE
	printf("module number is %d,register is %d, nchan %d, nchip %d, ndac %d, nadc %d, gain %f, offset %f\n",myModule.module, myModule.reg, myModule.nchan, myModule.nchip, myModule.ndac,  myModule.nadc, myModule.gain,myModule.offset);
#ifdef EIGERD
	int i;
	for(i=0;i<getNumberOfGainsPerModule();i++)
		printf("gain[%d]:%d\t%f\n",i,myGain[i],((double)myGain[i]/1000));
	for(i=0;i<getNumberOfOffsetsPerModule();i++)
		printf("offset[%d]:%d\t%f\n",i,myOffset[i],((double)myOffset[i]/1000));
	printf("IO Delay:%d\n",*myIODelay);
	printf("Tau:%lld\n",(long long int)myTau);
#endif
#endif

	switch(myModule.reg){
	case STANDARD:
	case HIGHGAIN:
	case LOWGAIN:
		break;
	default:
		sprintf(mess,"This setting %d does not exist for this detector\n",myModule.reg);
		ret = FAIL;
		cprintf(RED,"%s",mess);
		break;
	}


	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else {
#ifdef EIGERD
			ret=setModule(myModule, myGain, myOffset,myIODelay);
			//rate correction
			if(myTau > -2){	//ignore -2: from load settings)

				//set default tau value (-1 or a normal value)
				setDefaultSettingsTau_in_nsec(myTau);

				//switch off rate correction: no value read from load calib/load settings)
				if(myTau == -1){
					ret = FAIL;
					if(getRateCorrectionEnable()){
						setRateCorrection(0);
						strcat(mess," Cannot set Rate correction. No default tau provided. Deactivating Rate Correction\n");
					}else{
						strcat(mess," Cannot set Rate correction. No default tau provided\n");
					}
					cprintf(RED,"%s",mess);
				}


				//normal tau value (only if enabled)
				else if (getRateCorrectionEnable()){
					int64_t retvalTau = setRateCorrection(myTau); //myTau will not be -1 here
					if(myTau != retvalTau){
						if(retvalTau == -1)
							strcat(mess," Rate correction Deactivated, (tau/subexptime) must be < 0.0015\n");
						cprintf(RED,"%s",mess);
						ret=FAIL;
					}
				}
			}
			retval = getSettings();

#else
			ret=setModule(myModule);
			retval = ret;
#endif
		}
	}
#endif
	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* Maybe this is done inside the initialization funcs */
	//copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	free(myChip);
	free(myChan);
	free(myDac);
	free(myAdc);
#ifdef EIGERD
	free(myGain);
	free(myOffset);
#endif
#endif
	return ret;
}




int get_module(int file_des) {


	int ret=OK,ret1=OK;
	int arg;
	int  imod;
	int n;
	sls_detector_module myModule;

#ifdef SLS_DETECTOR_FUNCTION_LIST
#ifdef EIGERD
	int *myGain = (int*)malloc(getNumberOfGainsPerModule()*sizeof(int));
	int *myOffset = (int*)malloc(getNumberOfOffsetsPerModule()*sizeof(int));
#endif
	int *myChip=(int*)malloc(getNumberOfChipsPerModule()*sizeof(int));
	int *myChan=(int*)malloc(getNumberOfChannelsPerModule()*sizeof(int));
	int *myDac=(int*)malloc(getNumberOfDACsPerModule()*sizeof(int));
	int *myAdc=(int*)malloc(getNumberOfADCsPerModule()*sizeof(int));


	if (myDac)
		myModule.dacs=myDac;
	else {
		sprintf(mess,"could not allocate dacs\n");
		ret=FAIL;
	}
	if (myAdc)
		myModule.adcs=myAdc;
	else {
		sprintf(mess,"could not allocate adcs\n");
		ret=FAIL;
	}
	if (myChip)
		myModule.chipregs=myChip;
	else {
		sprintf(mess,"could not allocate chips\n");
		ret=FAIL;
	}
	if (myChan)
		myModule.chanregs=myChan;
	else {
		sprintf(mess,"could not allocate chans\n");
		ret=FAIL;
	}
#ifdef EIGERD
	if (!myGain){
		sprintf(mess,"could not allocate gains\n");
		ret=FAIL;
	}
	if (!myOffset){
		sprintf(mess,"could not allocate offsets\n");
		ret=FAIL;
	}
#endif
	myModule.ndac=getNumberOfDACsPerModule();
	myModule.nchip=getNumberOfChipsPerModule();
	myModule.nchan=getNumberOfChannelsPerModule();
	myModule.nadc=getNumberOfADCsPerModule();

#endif



	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	imod=arg;

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		ret=FAIL;
		if (imod>=0) {
			ret=OK;
			myModule.module=imod;
#ifdef EIGERD
			getModule(&myModule, myGain, myOffset);
#ifdef VERBOSE
			for(i=0;i<getNumberOfGainsPerModule();i++)
				printf("gain[%d]:%d\t%f\n",i,myGain[i],((double)myGain[i]/1000));
			for(i=0;i<getNumberOfOffsetsPerModule();i++)
				printf("offset[%d]:%d\t%f\n",i,myOffset[i],((double)myOffset[i]/1000));
#endif
#else
			getModule(&myModule);
#endif


#ifdef VERBOSE
			printf("Returning module %d of register %x\n",  imod, myModule.reg);
#endif 
		}
	}
#endif
	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendModule(file_des, &myModule);
#ifdef EIGERD
	n = sendData(file_des,myGain,sizeof(int)*getNumberOfGainsPerModule(),INT32);
	n = sendData(file_des,myOffset,sizeof(int)*getNumberOfOffsetsPerModule(),INT32);
#endif
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

#ifdef SLS_DETECTOR_FUNCTION_LIST
	free(myChip);
	free(myChan);
	free(myDac);
	free(myAdc);
#ifdef EIGERD
	free(myGain);
	free(myOffset);
#endif
#endif
	/*return ok/fail*/
	return ret;

}





int set_settings(int file_des) {

	int retval;
	int ret=OK,ret1=OK;
	int arg[2];
	int n;
	int  imod;
	enum detectorSettings isett;


	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	imod=arg[1];
	isett=arg[0];
#ifdef VERBOSE
	printf("In set_settings, isett:%d, imod =%d\n",isett,imod);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}

#ifdef VERBOSE
	printf("Changing settings of module %d to %d\n", imod,  isett);
#endif

	if (differentClients==1 && lockStatus==1 && isett!=GET_SETTINGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		retval=setSettings(isett, imod);
#ifdef VERBOSE
		printf("Settings changed to %d\n",  isett);
#endif

		if (retval==isett || isett<0) {
			ret=OK;
		} else {
			ret=FAIL;
			printf("Changing settings of module %d: wrote %d but read %d\n", imod, isett, retval);
		}

	}
#endif

	if (ret==OK && differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	return ret;


}






int get_threshold_energy(int file_des) { 
	int retval;
	int ret=OK,ret1=OK;
	int n;
	int  imod;


	n = receiveData(file_des,&imod,sizeof(imod),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#if defined(MYTHEND) || defined(EIGERD)
#ifdef VERBOSE
	printf("Getting threshold energy of module %d\n", imod);
#endif 
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}

	retval=getThresholdEnergy(imod);
#endif
#ifdef VERBOSE
	printf("Threshold is %d eV\n",  retval);
#endif  
#endif

	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);


	/* Maybe this is done inside the initialization funcs */
	//detectorDacs[imod][ind]=val;
	/*return ok/fail*/
	return ret;

}




int set_threshold_energy(int file_des) { 
	int retval;
	int ret=OK,ret1=OK;
	int arg[3];
	int n;
#if defined(MYTHEND) || defined(EIGERD)
	int ethr, imod;
	enum detectorSettings isett;
#endif

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#if defined(MYTHEND) || defined(EIGERD)
	ethr=arg[0];
	imod=arg[1];
	isett=arg[2];
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}
	printf("Setting threshold energy of module %d to %d eV with settings %d\n", imod, ethr, isett);
 	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		retval=setThresholdEnergy(ethr, imod);
	}
#ifdef VERBOSE
	printf("Threshold set to %d eV\n",  retval);
#endif  
	if (retval==ethr)
		ret=OK;
	else {
		ret=FAIL;
		printf("Setting threshold of module %d: wrote %d but read %d\n", imod, ethr, retval);
		sprintf(mess,"Setting threshold of module %d: wrote %d but read %d\n", imod, ethr, retval);
	}
#endif
#endif
	if (ret==OK && differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else
		n += sendData(file_des,&retval,sizeof(retval),INT32);


	/* Maybe this is done inside the initialization funcs */
	//detectorDacs[imod][ind]=val;
	/*return ok/fail*/
	return ret;

}




int start_acquisition(int file_des) {

	int ret=OK,ret1=OK;
	int n;


	sprintf(mess,"can't start acquisition\n");

#ifdef VERBOSE
	printf("Starting acquisition\n");
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=startStateMachine();
	}
#endif
	if (ret==FAIL)
		sprintf(mess,"Start acquisition failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	return ret;

}

int stop_acquisition(int file_des) {

	int ret=OK,ret1=OK;
	int n;


	sprintf(mess,"can't stop acquisition\n");

//#ifdef VERBOSE
	printf("Stopping acquisition\n");
//#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=stopStateMachine();
	}
#endif
	if (ret==FAIL)
		sprintf(mess,"Stop acquisition failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	return ret;


}




int start_readout(int file_des) {


	int ret=OK,ret1=OK;
	int n;


	sprintf(mess,"can't start readout\n");

#ifdef VERBOSE
	printf("Starting readout\n");
#endif     
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=startReadOut();
	}
#endif
	if (ret==FAIL)
		sprintf(mess,"Start readout failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}
	return ret;



}




int get_run_status(int file_des) {  

	int ret=OK,ret1=OK;
	int n;
	enum runStatus s;
	sprintf(mess,"getting run status\n");

#ifdef VERBOSE
	printf("Getting status\n");
#endif
//#ifdef SLS_DETECTOR_FUNCTION_LIST
	s= getRunStatus();printf("status:%x\n",s);
//#endif

	if (ret!=OK) {
		printf("get status failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n += sendData(file_des,&s,sizeof(s),INT32);
	}
	return ret;
}





int start_and_read_all(int file_des) {
	int dataret1;
#ifdef VERBOSE
	printf("Starting and reading all frames\n");
#endif

	if (differentClients==1 && lockStatus==1) {
		dataret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		//ret could be swapped during sendData
		dataret1 = dataret;
		sendData(file_des,&dataret1,sizeof(dataret),INT32);
		sendData(file_des,mess,sizeof(mess),OTHER);
		return dataret;

	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	startStateMachine();
	read_all(file_des);
#endif
#ifdef VERBOSE
	printf("Frames finished\n");
#endif


	return OK;


}




int read_frame(int file_des) {

	dataret=OK;
	int dataret1;
	if (differentClients==1 && lockStatus==1) {
		dataret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		//dataret could be swapped during sendData
		dataret1 = dataret;
		sendData(file_des,&dataret1,sizeof(dataret),INT32);
		sendData(file_des,mess,sizeof(mess),OTHER);
		printf("dataret %d\n",dataret);
		return dataret;
	}

#ifdef SLS_DETECTOR_FUNCTION_LIST
	dataretval=readFrame(&dataret, mess);
#endif


	//dataret could be swapped during sendData
	dataret1 = dataret;
	sendData(file_des,&dataret1,sizeof(dataret),INT32);
	if (dataret==FAIL)
		sendData(file_des,mess,sizeof(mess),OTHER);//sizeof(mess));//sizeof(mess));
	else
		sendData(file_des,dataretval,dataBytes,OTHER);

	printf("dataret %d\n",dataret);
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
	printf("Frames finished\n");
#endif   
	return OK;
}






int set_timer(int file_des) {
	enum timerIndex ind;
	int64_t tns;
	int n;
	int64_t retval;
	int ret=OK,ret1=OK;


	sprintf(mess,"can't set timer\n");

	n = receiveData(file_des,&ind,sizeof(ind),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	n = receiveData(file_des,&tns,sizeof(tns),INT64);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if (ret!=OK) {
		printf(mess);
	}

#ifdef VERBOSE
	printf("setting timer %d to %lld ns\n",ind,tns);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1 && tns!=-1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {
			switch(ind) {
#ifdef EIGERD
			case SUBFRAME_ACQUISITION_TIME:
				if (tns > ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS*10) ){
					ret=FAIL;
					strcpy(mess,"Sub Frame exposure time should not exceed 5.368 seconds\n");
					break;
				}
				retval = setTimer(ind,tns);
				break;
#endif
#ifdef MYTHEN
			case PROBES_NUMBER:
#endif
			case FRAME_NUMBER:
			case ACQUISITION_TIME:
			case FRAME_PERIOD:
			case DELAY_AFTER_TRIGGER:
			case GATES_NUMBER:
			case CYCLES_NUMBER:
				retval = setTimer(ind,tns);
				break;
			default:
				ret=FAIL;
				sprintf(mess,"timer index unknown for this detector %d\n",ind);
				break;
			}

		}
	}
#endif
	if (ret!=OK) {
		printf(mess);
		printf("set timer failed\n");
		sprintf(mess, "set timer %d failed\n", ind);
	}
	else{
#if defined(MYTHEND) || defined(GOTTHARD)
	if (ind==FRAME_NUMBER) {
		ret=allocateRAM();
		if (ret!=OK)
			sprintf(mess, "could not allocate RAM for %lld frames\n", tns);
	}
#endif

	if (differentClients)
		ret=FORCE_UPDATE;
}
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
#ifdef VERBOSE
		printf("returning ok %d\n",sizeof(retval));
#endif

		n = sendData(file_des,&retval,sizeof(retval),INT64);
	}

	return ret;

}








int get_time_left(int file_des) {

	enum timerIndex ind;
	int n;
	int64_t retval;
	int ret=OK,ret1=OK;

	sprintf(mess,"can't get timer\n");
	n = receiveData(file_des,&ind,sizeof(ind),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("getting time left on timer %d \n",ind);
#endif 
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		switch(ind) {
		case PROBES_NUMBER:
#ifndef MYTHEND
				ret=FAIL;
				strcpy(mess,"Not applicable/implemented for this detector\n");
				break;
#endif
		case FRAME_NUMBER:
		case ACQUISITION_TIME:
		case FRAME_PERIOD:
		case DELAY_AFTER_TRIGGER:
		case GATES_NUMBER:
		case CYCLES_NUMBER:
		case PROGRESS:
		case ACTUAL_TIME:
		case MEASUREMENT_TIME:
			getTimeLeft(ind);
			break;
		default:
			ret=FAIL;
			sprintf(mess,"timer index unknown %d\n",ind);
			break;
		}
	}
#endif

	if (ret!=OK) {
		printf("get time left failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;

#ifdef VERBOSE
	printf("time left on timer %d is %lld\n",ind, retval);
#endif 

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT64);
	}
#ifdef VERBOSE
	printf("data sent\n");
#endif 
	return ret;
}





int set_dynamic_range(int file_des) {

	int dr;
	int n;
	int retval;
	int ret=OK,ret1=OK;

	sprintf(mess,"can't set dynamic range\n");

	n = receiveData(file_des,&dr,sizeof(dr),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && dr>=0) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
#ifdef EIGERD
		switch(dr){
		case -1:case 4:	case 8:	case 16:case 32:break;
		default:
			strcpy(mess,"could not set dynamic range. Must be 4,8,16 or 32.\n");
			ret = FAIL;
		}
#endif
	}
	if(ret == OK)
		retval=setDynamicRange(dr);
#endif
	if (dr>=0 && retval!=dr)
		ret=FAIL;
	if (ret!=OK) {
		sprintf(mess,"set dynamic range failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (dr>=0) dataBytes=calculateDataBytes();
#endif

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT32);
	}
	return ret;
}






int set_readout_flags(int file_des) {

	enum readOutFlags retval;
	enum readOutFlags arg;
	int n;
	int ret=OK,ret1=OK;

	sprintf(mess,"can't set readout flags\n");

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}



#if !defined(MYTHEND) && !defined(EIGERD)
	sprintf(mess,"Read out flags not implemented for this detector\n");
	cprintf(RED, "%s",mess);
	ret=FAIL;
#else


#ifdef VERBOSE
	printf("setting readout flags  to %d\n",arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		switch(arg) {
		case  GET_READOUT_FLAGS:
#ifdef MYTHEND
		case TOT_MODE:
		case NORMAL_READOUT:
#endif
#if defined(MYTHEND) || defined(EIGERD)
		case STORE_IN_RAM:
		case CONTINOUS_RO:
#endif
#ifdef EIGERD
		case PARALLEL:
		case NONPARALLEL:
		case SAFE:
#endif
			retval=setReadOutFlags(arg);
			break;

		default:
			sprintf(mess,"Unknown readout flag %d for this detector\n", arg);
			cprintf(RED, "%s",mess);
			ret=FAIL;
			break;
		}
	}
#endif

#endif

	if (ret==OK) {
		if ((retval == -1) || ((arg!=-1)&&((retval&arg)!=arg))){
			cprintf(RED,"arg:0x%x, retval:0x%x retval&arg:0x%x\n",(int)arg,(int)retval,retval&arg);
			ret=FAIL;
			sprintf(mess,"Could not change readout flag: should be 0x%x but is 0x%x\n", arg, retval);
			cprintf(RED, "%s",mess);
		}else if (differentClients)
			ret=FORCE_UPDATE;
	}


	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT32);
	}
	return ret;
}





int set_roi(int file_des) {

	int ret=OK,ret1=OK;
	ROI arg[MAX_ROIS];
	ROI* retval=0;
	int nroi=-1, n=0, retvalsize=0,retvalsize1,i;
	strcpy(mess,"Could not set/get roi\n");

	n = receiveData(file_des,&nroi,sizeof(nroi),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if(nroi!=-1){
		for(i=0;i<nroi;i++){
			n = receiveData(file_des,&arg[i].xmin,sizeof(int),INT32);
			n = receiveData(file_des,&arg[i].xmax,sizeof(int),INT32);
			n = receiveData(file_des,&arg[i].ymin,sizeof(int),INT32);
			n = receiveData(file_des,&arg[i].ymax,sizeof(int),INT32);
		}
		//n = receiveData(file_des,arg,nroi*sizeof(ROI));
		if (n != (nroi*sizeof(ROI))) {
			sprintf(mess,"Received wrong number of bytes for ROI\n");
			ret=FAIL;
		}
	}
#ifndef GOTTHARDD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
	printf("Error:Set ROI-%s",mess);
#else
#ifdef VERBOSE
	printf("Setting ROI to:");
	for( i=0;i<nroi;i++)
		printf("%d\t%d\t%d\t%d\n",arg[i].xmin,arg[i].xmax,arg[i].ymin,arg[i].ymax);
#endif
	/* execute action if the arguments correctly arrived*/
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Detector locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else{
		retval=setROI(nroi,arg,&retvalsize,&ret);
		if (ret==FAIL){
			printf("mess:%s\n",mess);
			sprintf(mess,"Could not set all roi, should have set %d rois, but only set %d rois\n",nroi,retvalsize);
		}
	}

#endif
#endif


	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if(ret==FAIL)
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	else{
		//retvalsize could be swapped during sendData
		retvalsize1=retvalsize;
		sendData(file_des,&retvalsize1,sizeof(retvalsize),INT32);
		for(i=0;i<retvalsize;i++){
			n = sendData(file_des,&retval[i].xmin,sizeof(int),INT32);
			n = sendData(file_des,&retval[i].xmax,sizeof(int),INT32);
			n = sendData(file_des,&retval[i].ymin,sizeof(int),INT32);
			n = sendData(file_des,&retval[i].ymax,sizeof(int),INT32);
		}
		//sendData(file_des,retval,retvalsize*sizeof(ROI));
	}
	/*return ok/fail*/
	return ret;

#ifdef SLS_DETECTOR_FUNCTION_LIST
	dataBytes=calculateDataBytes();
#endif
	return FAIL;

}


int set_speed(int file_des) {

	enum speedVariable arg;
	int val, n;
	int ret=OK,ret1=OK;
	int retval;

	sprintf(mess,"can't set speed variable\n");

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	n = receiveData(file_des,&val,sizeof(val),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("setting speed variable %d  to %d\n",arg,val);
#endif 
	if (ret==OK) {

		if (differentClients==1 && lockStatus==1 && val>=0) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {
#ifdef SLS_DETECTOR_FUNCTION_LIST
			switch (arg) {
#ifdef MYTHEND
			case CLOCK_DIVIDER:
			case WAIT_STATES:
			case SET_SIGNAL_LENGTH:
			case TOT_CLOCK_DIVIDER:
			case TOT_DUTY_CYCLE:
				retval=setSpeed(arg, val);
				break;
#elif EIGERD
			case CLOCK_DIVIDER:
				retval=setSpeed(arg, val);
				break;
#endif
			default:
				sprintf(mess,"unknown speed variable %d for this detector\n",arg);
				ret=FAIL;
				break;
			}
#endif
		}
		if (ret==OK){
			if ((retval!=val) && (val>=0)) {
				ret=FAIL;
				sprintf(mess,"could not change speed variable %d: should be %d but is %d \n",arg, val, retval);
			}else if (differentClients)
				ret=FORCE_UPDATE;

		}
	}


	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT32);
	}
	return ret;
}




int execute_trimming(int file_des) {

	int arg[3];
	int n;
	int ret=OK, ret1=OK, retval=0;
#if defined(MYTHEND) || defined(EIGERD)
	int imod, par1,par2;
#endif
	enum trimMode mode;

	printf("called function execute trimming\n");

	sprintf(mess,"can't set execute trimming\n");

	n = receiveData(file_des,&mode,sizeof(mode),INT32);
	printf("mode received\n");
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (mode)\n");
		ret=FAIL;
	}

	n = receiveData(file_des,arg,sizeof(arg),INT32);
	printf("arg received\n");
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (args)\n");
		ret=FAIL;
	}

#if !defined(MYTHEND) && !defined(EIGERD)
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else

	imod=arg[0];
	par1=arg[1];
	par2=arg[2];
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number out of range %d\n",imod);
	}
#endif
	if (ret==OK) {

#ifdef VERBOSE
		printf("trimming module %d mode %d, parameters %d %d \n",imod,mode, par1, par2);
#endif  

		if (differentClients==1 && lockStatus==1 ) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {
#ifdef SLS_DETECTOR_FUNCTION_LIST
			if (ret==OK) {
				switch(mode) {
				case NOISE_TRIMMING:
				case BEAM_TRIMMING:
				case IMPROVE_TRIMMING:
				case FIXEDSETTINGS_TRIMMING:
					if (myDetectorType==MYTHEN) {
						retval=executeTrimming(mode, par1, par2, imod);
						break;
					}
					break;
				default:
					printf("Unknown trimming mode %d\n",mode);
					sprintf(mess,"Unknown trimming mode %d\n",mode);
					ret=FAIL;
					break;
				}
			}
#endif
		}
	}
#endif

	if (ret!=OK) {
		sprintf(mess,"can't set execute trimming\n");
		ret=FAIL;
	} else if (retval>0) {
		sprintf(mess,"Could not trim %d channels\n", retval);
		ret=FAIL;
	} else if (differentClients)
		ret=FORCE_UPDATE;

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	}

	return ret;
}









int configure_mac(int file_des) {

	int retval=-100;
	int ret=OK,ret1=OK;
	char arg[6][50];
	int n;

#ifndef MYTHEND
	int imod=0;//should be in future sent from client as -1, arg[2]
	int ipad;
	long long int imacadd;
	long long int idetectormacadd;
	int udpport;
	int udpport2;
	int detipad;
#endif

	sprintf(mess,"Can't configure MAC\n");

	n = receiveData(file_des,arg,sizeof(arg),OTHER);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef MYTHEND
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
	sscanf(arg[0], "%x", 	&ipad);
	sscanf(arg[1], "%llx", 	&imacadd);
	sscanf(arg[2], "%x", 	&udpport);
	sscanf(arg[3], "%llx",	&idetectormacadd);
	sscanf(arg[4], "%x",	&detipad);
	sscanf(arg[5], "%x", 	&udpport2);


#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number out of range %d\n",imod);
		printf("mess:%s\n",mess);
	}
#endif
#ifdef VERBOSE
	int i;
	/*printf("\ndigital_test_bit in server %d\t",digitalTestBit);for gotthard*/
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
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if(getRunStatus() == RUNNING){
			stopStateMachine();
		}

		retval=configureMAC(ipad,imacadd,idetectormacadd,detipad,udpport,udpport2,0);	/*digitalTestBit);*/
		if(retval==-1) 	ret=FAIL;
	}
#endif
#ifdef VERBOSE
	printf("Configured MAC with retval %d\n",  retval);
#endif
	if (ret==FAIL) {
		printf("configuring MAC of mod %d failed\n", imod);
	}

	if (differentClients)
		ret=FORCE_UPDATE;
#endif
	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;

}



int load_image(int file_des) {
	int retval;
	int ret=OK,ret1=OK;
	int n;
	enum imageType index;
	char ImageVals[dataBytes];

	sprintf(mess,"Loading image failed\n");

	n = receiveData(file_des,&index,sizeof(index),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	n = receiveData(file_des,ImageVals,dataBytes,OTHER);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifndef GOTTHARDD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}

#ifdef SLS_DETECTOR_FUNCTION_LIST
		switch (index) {
		case DARK_IMAGE :
#ifdef VERBOSE
			printf("Loading Dark image\n");
#endif
		case GAIN_IMAGE :
#ifdef VERBOSE
			printf("Loading Gain image\n");
#endif
			if (myDetectorType==GOTTHARD) {
				retval=loadImage(index,ImageVals);
				if (retval==-1)
					ret = FAIL;
			}
			break;
		default:
			printf("Unknown index %d\n",index);
			sprintf(mess,"Unknown index %d\n",index);
			ret=FAIL;
			break;
		}
#endif
	}
#endif


	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,&retval,sizeof(retval),INT32);
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;
}





int read_counter_block(int file_des) {

	int ret=OK,ret1=OK;
	int n;
	int startACQ;
	//char *retval=NULL;
#ifdef GOTTHARDD
	char CounterVals[NCHAN*NCHIP];
#else
	char CounterVals[dataBytes];
#endif

	sprintf(mess,"Read counter block failed\n");

	n = receiveData(file_des,&startACQ,sizeof(startACQ),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifndef GOTTHARDD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
			ret=readCounterBlock(startACQ,CounterVals);
#ifdef VERBOSE
			int i;
			for(i=0;i<6;i++)
				printf("%d:%d\t",i,CounterVals[i]);
#endif
		}
	}
#endif
#endif

	if(ret!=FAIL){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret!=FAIL) {
		/* send return argument */
		n += sendData(file_des,CounterVals,dataBytes,OTHER);//1280*2
	} else {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	/*return ok/fail*/
	return ret;
}





int reset_counter_block(int file_des) {

	int ret=OK,ret1=OK;
	int n;
	int startACQ;

	sprintf(mess,"Reset counter block failed\n");

	n = receiveData(file_des,&startACQ,sizeof(startACQ),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifndef GOTTHARDD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=resetCounterBlock(startACQ);
	}
#endif
#endif
	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);

	/*return ok/fail*/
	return ret;
}








int start_receiver(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	strcpy(mess,"Could not start receiver\n");

	/* execute action if the arguments correctly arri ved*/
#if defined(GOTTHARDD) || defined(EIGERD)
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Detector locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else
		ret = startReceiver(1);

#endif
#else
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if(ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	/*return ok/fail*/
	return ret;
}






int stop_receiver(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;

	strcpy(mess,"Could not stop receiver\n");

	/* execute action if the arguments correctly arrived*/
#ifndef GOTTHARDD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Detector locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else
		ret=startReceiver(0);

#endif
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if(ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	/*return ok/fail*/
	return ret;
}





int calibrate_pedestal(int file_des){

	int ret=OK,ret1=OK;
	int retval=-1;
	int n;
	int frames;

	sprintf(mess,"Could not calibrate pedestal\n");

	n = receiveData(file_des,&frames,sizeof(frames),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifndef GOTTHARDD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=calibratePedestal(frames);
	}
#endif
#endif
	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	else
		n += sendData(file_des,&retval,sizeof(retval),INT32);

	/*return ok/fail*/
	return ret;
}









int enable_ten_giga(int file_des) {
	int n;
	int retval=-1;
	int ret=OK,ret1=OK;
	int arg = -1;

	sprintf(mess,"Can't enable/disable 10Gbe \n");
	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	/* execute action */
#ifndef EIGERD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef VERBOSE
		printf("Enabling 10Gbe :%d \n",arg);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if(ret != FAIL){

		retval=enableTenGigabitEthernet(arg);
		if((arg != -1) && (retval != arg))
			ret=FAIL;
		else if (differentClients==1) {
			ret=FORCE_UPDATE;
		}
	}
#endif
#endif
	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	/* send return argument */
	n += sendData(file_des,&retval,sizeof(retval),INT32);
	/*return ok/fail*/
	return ret;
}



int set_all_trimbits(int file_des){


	int retval;
	int arg;
	int n;
	int ret=OK,ret1=OK;

	sprintf(mess,"can't set sll trimbits\n");

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifndef EIGERD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef VERBOSE
	printf("setting all trimbits to %d\n",arg);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		if(arg < -1){
			ret = FAIL;
			strcpy(mess,"Cant set trimbits to this value\n");
		}else {
			if(arg >= 0)
				setAllTrimbits(arg);
			retval = getAllTrimbits();
		}
	}
#endif
	if (ret==OK) {
		if (arg!=-1 && arg!=retval) {
			ret=FAIL;
			sprintf(mess,"Could not set all trimbits: should be %d but is %d\n", arg, retval);
		}else if (differentClients)
			ret=FORCE_UPDATE;
	}
#endif

	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		n = sendData(file_des,&retval,sizeof(retval),INT32);
	}
	return ret;

}





int set_counter_bit(int file_des) {
	int n;
	int retval = -1;
	int ret=OK,ret1=OK;
	int arg = -1;

	sprintf(mess,"Can't set/rest counter bit \n");
	/* receive arguments */
	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	/* execute action */
#ifndef EIGERD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef VERBOSE
		printf("Getting/Setting/Resetting counter bit :%d \n",arg);
#endif
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if(ret != FAIL){
		retval=setCounterBit(arg);
		if((arg != -1) && (retval != arg))
			ret=FAIL;
		else if (differentClients==1) {
			ret=FORCE_UPDATE;
		}
	}
#endif
#endif
	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	/* send return argument */
	n += sendData(file_des,&retval,sizeof(retval),INT32);
	/*return ok/fail*/
	return ret;
}


int pulse_pixel(int file_des) {

	int ret=OK,ret1=OK;
	int n;
	int arg[3];
	arg[0]=-1;	arg[1]=-1;	arg[2]=-1;


	sprintf(mess,"pulse pixel failed\n");

	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifndef EIGERD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=pulsePixel(arg[0],arg[1],arg[2]);
	}
#endif
#endif
	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);

	/*return ok/fail*/
	return ret;
}


int pulse_pixel_and_move(int file_des) {

	int ret=OK,ret1=OK;
	int n;
	int arg[3];
	arg[0]=-1;	arg[1]=-1;	arg[2]=-1;


	sprintf(mess,"pulse pixel and move failed\n");

	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifndef EIGERD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=pulsePixelNMove(arg[0],arg[1],arg[2]);
	}
#endif
#endif
	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);

	/*return ok/fail*/
	return ret;

}




int pulse_chip(int file_des) {

	int ret=OK,ret1=OK;
	int n;
	int arg = -1;


	sprintf(mess,"pulse chip failed\n");

	n = receiveData(file_des,&arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifndef EIGERD
	ret = FAIL;
	strcpy(mess,"Not applicable/implemented for this detector\n");
#else
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=pulseChip(arg);
	}
#endif
#endif
	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL)
		n += sendData(file_des,mess,sizeof(mess),OTHER);

	/*return ok/fail*/
	return ret;

}




int set_rate_correct(int file_des) {
	int64_t tau_ns=-1;
	int n;
	int ret=OK,ret1=OK;

	sprintf(mess,"can't set/unset rate correction\n");

	n = receiveData(file_des,&tau_ns,sizeof(tau_ns),INT64);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		cprintf(RED,"%s",mess);
		ret=FAIL;
	}

#ifndef EIGERD
	sprintf(mess,"Rate Correction not implemented for this detector\n");
	cprintf(RED,"%s",mess);
	ret=FAIL;
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST

	if (ret==OK) {
		printf("Setting rate correction to %lld ns\n",tau_ns);

		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {

			//tau = -1, use default tau of settings
			if((ret==OK)&&(tau_ns<0)){
				tau_ns = getDefaultSettingsTau_in_nsec();
			}
			//still negative (not set)
			if(tau_ns < 0){
				ret = FAIL;
				if(getRateCorrectionEnable()){
					setRateCorrection(0);
					strcpy(mess,"Cannot set rate correction as default tau not provided. Switching off Rate Correction\n");
				}else{
					strcpy(mess,"Cannot set rate correction as default tau not provided\n");
				}
				cprintf(RED,"%s",mess);
			}

			//set rate
			else{
				int64_t retval = setRateCorrection(tau_ns); //tau_ns will not be -1 here
				if(tau_ns != retval){
					if(retval == -1)
						strcpy(mess,"Rate correction Deactivated, (tau/subexptime) must be < 0.0015\n");
					cprintf(RED,"%s",mess);
					ret=FAIL;
				}
			}
		}
	}
#endif
	if ((ret==OK) && (differentClients))
		ret=FORCE_UPDATE;


	//ret could be swapped during sendData
	ret1 = ret;
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		n = sendData(file_des,mess,sizeof(mess),OTHER);
	}

	return ret;
}




int get_rate_correct(int file_des) {
	int64_t retval=-1;
	int ret=OK,ret1=OK;

	sprintf(mess,"can't get rate correction\n");


#ifndef EIGERD
	sprintf(mess,"Rate Correction not implemented for this detector\n");
	cprintf(RED,"%s",mess);
	ret=FAIL;
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST

	if (ret==OK) {
		retval = getCurrentTau();
		printf("Getting rate correction %lld\n",(long long int)retval);
	}
#endif
	if ((ret==OK) && (differentClients))
		ret=FORCE_UPDATE;


	//ret could be swapped during sendData
	ret1 = ret;
	sendData(file_des,&ret1,sizeof(ret),INT32);
	if (ret==FAIL) {
		sendData(file_des,mess,sizeof(mess),OTHER);
	} else {
		sendData(file_des,&retval,sizeof(retval),INT64);
	}

	return ret;
}
