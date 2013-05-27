
#include "sls_detector_defs.h"


#include "slsDetectorServer_funcs.h"
#include "slsDetectorFunctionList.h"

#include "slsDetectorServer_defs.h"
#include "communication_funcs.h"


#include <stdio.h>
#include <string.h>


//#if defined(EIGERD) || defined(GOTHARDD) break;
//#endif


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


//define in communication_funcs
/*
extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;
*/


/* global variables for optimized readout */
char *dataretval=NULL;
int dataret;
char mess[1000]; 
int dataBytes;




int init_detector(int b) {
#ifdef VIRTUAL
	printf("This is a VIRTUAL detector\n");
#endif

	mapCSP0();
	//only for control server
	if(b){
#ifdef SLS_DETECTOR_FUNCTION_LIST
		initializeDetector();
		//testFpga();
		//testRAM();

		//Initialization
		//setSettings(GET_SETTINGS,-1);
		//setFrames(1);
		//setTrains(1);
		//setExposureTime(1e6);
		//setPeriod(1e9);
		//setDelay(0);
		//setGates(0);

		//setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
		//setMaster(GET_MASTER);
		//setSynchronization(GET_SYNCHRONIZATION_MODE);
#endif
	}
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
	n = receiveDataOnly(file_des,&fnum,sizeof(fnum));
	if (n <= 0) {
		printf("ERROR reading from socket %d, %d %d\n", n, fnum, file_des);
		return FAIL;
	}
#ifdef VERBOSE
	else
		printf("size of data received %d\n",n);
#endif

#ifdef VERBOSE
	printf( "calling function fnum = %d %x\n",fnum,flist[fnum]);
#endif
	if (fnum<0 || fnum>255)
		fnum=255;
	ret=(*flist[fnum])(file_des);
	if (ret==FAIL)
		printf( "Error executing the function = %d \n",fnum);
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
	sendDataOnly(file_des,&ret,sizeof(ret));
	sendDataOnly(file_des,mess,sizeof(mess));
	return GOODBYE;
}


int exit_server(int file_des) {
	int ret=FAIL;
	sendDataOnly(file_des,&ret,sizeof(ret));
	printf("closing server.");
	sprintf(mess,"closing server");
	sendDataOnly(file_des,mess,sizeof(mess));
	return GOODBYE;
}

int exec_command(int file_des) {
	char cmd[MAX_STR_LENGTH];
	char answer[MAX_STR_LENGTH];
	int ret=OK;
	int sysret=0;
	int n=0;

	/* receive arguments */
	n = receiveDataOnly(file_des,cmd,MAX_STR_LENGTH);
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
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	n = sendDataOnly(file_des,answer,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error writing to socket");
		ret=FAIL;
	}


	/*return ok/fail*/
	return ret;

}





int lock_server(int file_des) {


	int n;
	int ret=OK;

	int lock;
	n = receiveDataOnly(file_des,&lock,sizeof(lock));
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

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	}  else
		n = sendDataOnly(file_des,&lockStatus,sizeof(lockStatus));

	return ret;

}




int get_last_client_ip(int file_des) {
	int ret=OK;
	int n;
	if (differentClients )
		ret=FORCE_UPDATE;
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));

	return ret;

}




int set_port(int file_des) {
	int n;
	int ret=OK;
	int sd=-1;

	enum portType p_type; /** data? control? stop? Unused! */
	int p_number; /** new port number */

	n = receiveDataOnly(file_des,&p_type,sizeof(p_type));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (ptype)\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,&p_number,sizeof(p_number));
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

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&p_number,sizeof(p_number));
		closeConnection(file_des);
		exitServer(sockfd);
		sockfd=sd;

	}

	return ret;

}



int send_update(int file_des) {

	int ret=OK;
	enum detectorSettings t;
	int thr, n;
	// int it;
	int64_t retval;/*, tns=-1;*/
	int nm;


	n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));
	nm=setNMod(-1,X);
	n = sendDataOnly(file_des,&nm,sizeof(nm));
	nm=setNMod(-1,Y);
	n = sendDataOnly(file_des,&nm,sizeof(nm));
	nm=setDynamicRange(-1);
	n = sendDataOnly(file_des,&nm,sizeof(nm));

	n = sendDataOnly(file_des,&dataBytes,sizeof(dataBytes));

	t=setSettings(GET_SETTINGS, -1);
	n = sendDataOnly(file_des,&t,sizeof(t));
	thr=getThresholdEnergy(-1);
	n = sendDataOnly(file_des,&thr,sizeof(thr));
	/*retval=setFrames(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	/*retval=setExposureTime(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	/*retval=setPeriod(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	/*retval=setDelay(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	/*retval=setGates(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	/*retval=setProbes(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	/*retval=setTrains(tns);*/
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));

	if (lockStatus==0) {
		strcpy(lastClientIP,thisClientIP);
	}

	return ret;


}


int update_client(int file_des) {

	int ret=OK;
	sendDataOnly(file_des,&ret,sizeof(ret));
	return send_update(file_des);

}




int set_master(int file_des) {

	enum masterFlags retval=GET_MASTER;
	enum masterFlags arg;
	int n;
	int ret=OK;
	// int regret=OK;


	sprintf(mess,"can't set master flags\n");


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef VERBOSE
	printf("setting master flags  to %d\n",arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		retval=setMaster(arg);

	}
#endif
	if (retval==GET_MASTER) {
		ret=FAIL;
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
	return ret;
}






int set_synchronization(int file_des) {

	enum synchronizationMode retval=GET_MASTER;
	enum synchronizationMode arg;
	int n;
	int ret=OK;
	//int regret=OK;


	sprintf(mess,"can't set synchronization mode\n");


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
#ifdef VERBOSE
	printf("setting master flags  to %d\n",arg);
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		retval=setSynchronization(arg);
	}
#endif

	if (retval==GET_SYNCHRONIZATION_MODE) {
		ret=FAIL;
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
	return ret;
}







int get_detector_type(int file_des) {
	int n=0;
	enum detectorType retval;
	int ret=OK;

	sprintf(mess,"Can't return detector type\n");


	/* receive arguments */
	/* execute action */
	retval=myDetectorType;

#ifdef VERBOSE
	printf("Returning detector type %d\n",ret);
#endif

	/* send answer */
	/* send OK/failed */
	if (differentClients==1)
		retval=FORCE_UPDATE;

	n += sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	/*return ok/fail*/
	return ret;


}


int set_number_of_modules(int file_des) {
	int n;
	int arg[2], retval=0;
	int ret=OK;
	enum dimension dim;
	int nm;

	sprintf(mess,"Can't set number of modules\n");

	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket %d", n);
		ret=GOODBYE;
	}
	if (ret==OK) {
		dim=arg[0];
		nm=arg[1];

		/* execute action */
#ifdef VERBOSE
		printf("Setting the number of modules in dimension %d to %d\n",dim,nm );
#endif

#ifdef SLS_DETECTOR_FUNCTION_LIST
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
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;
}


int get_max_number_of_modules(int file_des) {
	int n;
	int retval;
	int ret=OK;
	enum dimension arg;

	sprintf(mess,"Can't get max number of modules\n");
	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
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
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
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
	int ret=OK;
	int signalindex;
	enum externalSignalFlag flag, retval=SIGNAL_OFF;

	sprintf(mess,"Can't set external signal flag\n");

	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
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
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}


	/*return ok/fail*/
	return ret;

}


int set_external_communication_mode(int file_des) {
	int n;
	enum externalCommunicationMode arg, retval=GET_EXTERNAL_COMMUNICATION_MODE;
	int ret=OK;

	sprintf(mess,"Can't set external communication mode\n");


	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
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
  GATE_COINCIDENCE_WITH_INTERNAL_ENABLE
};
	 */
#ifdef SLS_DETECTOR_FUNCTION_LIST
	if (ret==OK) {
		/* execute action */

		retval=setTiming(arg);

		/*     switch(arg) { */
		/*     default: */
		/*       sprintf(mess,"The meaning of single signals should be set\n"); */
		/*       ret=FAIL; */
		/*     } */


#ifdef VERBOSE
		printf("Setting external communication mode to %d\n", arg);
#endif
	} else
		ret=FAIL;
#endif

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;


}



int get_id(int file_des) {
	// sends back 64 bits!
	int64_t retval;
	int ret=OK;
	int imod=-1;
	int n=0;
	enum idMode arg;

	sprintf(mess,"Can't return id\n");

	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Getting id %d\n", arg);
#endif  

	switch (arg) {
	case  MODULE_SERIAL_NUMBER:
	case MODULE_FIRMWARE_VERSION:
		n = receiveDataOnly(file_des,&imod,sizeof(imod));
		if (n < 0) {
			sprintf(mess,"Error reading from socket\n");
			ret=FAIL;
		} else {
#ifdef VERBOSE
			printf("of module %d\n", imod);
#endif  
			if (imod>=0 && imod<getTotalNumberOfModules())
				retval=getModuleId(arg, imod);
			else {
				ret=FAIL;
				sprintf(mess,"Module number %d out of range\n", imod);

			}



		}
		break;
	case DETECTOR_SERIAL_NUMBER:
	case DETECTOR_FIRMWARE_VERSION:
	case DETECTOR_SOFTWARE_VERSION:
		retval=getDetectorId(arg);
		break;
	default:
		printf("Required unknown id %d \n", arg);
		ret=FAIL;
		retval=FAIL;
		break;
	}

#ifdef VERBOSE
	printf("Id is %llx\n", retval);
#endif  

	if (differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}

int digital_test(int file_des) {

	int retval;
	int ret=OK;
	int imod=-1;
	int n=0;
	int ival;
	enum digitalTestMode arg;

	sprintf(mess,"Can't send digital test\n");

	n = receiveDataOnly(file_des,&arg,sizeof(arg));
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
			n = receiveDataOnly(file_des,&imod,sizeof(imod));
			if (n < 0) {
				sprintf(mess,"Error reading from socket\n");
				retval=FAIL;
			}
#ifdef VERBOSE
			printf("of module %d\n", imod);
#endif   
			if (imod>=0 && imod<getTotalNumberOfModules())
				retval=moduleTest(arg,imod);
			else {
				ret=FAIL;
				sprintf(mess,"Module number %d out of range\n", imod);

			}
			break;
		case MODULE_FIRMWARE_TEST:
		case DETECTOR_FIRMWARE_TEST:
		case DETECTOR_MEMORY_TEST:
		case DETECTOR_BUS_TEST:
		case DETECTOR_SOFTWARE_TEST:
			retval=detectorTest(arg);
			break;
		case DIGITAL_BIT_TEST:
			n = receiveDataOnly(file_des,&ival,sizeof(ival));
			if (n < 0) {
				sprintf(mess,"Error reading from socket\n");
				retval=FAIL;
			}

			retval=0;
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
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}






int set_dac(int file_des) {

	double retval;
	int ret=OK;
	int arg[2];
	enum dacIndex ind;
	int imod;
	int n;
	double val;

	sprintf(mess,"Can't set DAC\n");


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	imod=arg[1];

	n = receiveDataOnly(file_des,&val,sizeof(val));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Setting DAC %d of module %d to %f V\n", ind, imod, val);
#endif 


	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}


	// check if dac exists for this detector
	switch (ind) {
	case  TRIMBIT_SIZE:
		//if (myDetectorType==MYTHEN)
			break;
	case THRESHOLD:
		//if (myDetectorType==MYTHEN)
			break;
	case SHAPER1:
		//if (myDetectorType==MYTHEN)
			break;
	case SHAPER2:
		//if (myDetectorType==MYTHEN)
			break;
	case CALIBRATION_PULSE:
		//if (myDetectorType==MYTHEN)
			break;
	case PREAMP:
		//if (myDetectorType==MYTHEN)
			break;
	default:
		printf("Unknown DAC index %d\n",ind);
		sprintf(mess,"Unknown DAC index %d\n",ind);
		ret=FAIL;
		break;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1 && val!=-1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			retval=setDAC(ind,val,imod);
	}

#ifdef VERBOSE
	printf("DAC set to %f V\n",  retval);
#endif  
	if (retval==val || val==-1) {
		ret=OK;
		if (differentClients)
			ret=FORCE_UPDATE;
	} else {
		ret=FAIL;
		printf("Setting dac %d of module %d: wrote %f but read %f\n", ind, imod, val, retval);
	}


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/* Maybe this is done inside the initialization funcs */
	//detectorDacs[imod][ind]=val;
	/*return ok/fail*/
	return ret;

}



int get_adc(int file_des) {

	double retval;
	int ret=OK;
	int arg[2];
	enum dacIndex ind;
	int imod;
	int n;

	sprintf(mess,"Can't read ADC\n");


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	imod=arg[1];


	if (imod>=getTotalNumberOfModules() || imod<0) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}


	switch (ind) {
	case  TRIMBIT_SIZE:
		//if (myDetectorType==MYTHEN)
			break;
	case THRESHOLD:
		//if (myDetectorType==MYTHEN)
			break;
	case SHAPER1:
		//if (myDetectorType==MYTHEN)
			break;
	case SHAPER2:
		//if (myDetectorType==MYTHEN)
			break;
	case CALIBRATION_PULSE:
		//if (myDetectorType==MYTHEN)
			break;
	case PREAMP:
		//if (myDetectorType==MYTHEN)
			break;
	default:
		printf("Unknown DAC index %d\n",ind);
		ret=FAIL;
		sprintf(mess,"Unknown ADC index %d\n",ind);
		break;
	}

	if (ret==OK) {
		retval=getADC(ind,imod);
	}
#ifdef VERBOSE
	printf("Getting ADC %d of module %d\n", ind, imod);
#endif 

#ifdef VERBOSE
	printf("ADC is %f V\n",  retval);
#endif  
	if (ret==FAIL) {
		printf("Getting adc %d of module %d failed\n", ind, imod);
	}


	if (differentClients)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}






int write_register(int file_des) {

	int retval;
	int ret=OK;
	int arg[2];
	int addr, val;
	int n;


	sprintf(mess,"Can't write to register\n");

	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	addr=arg[0];
	val=arg[1];

#ifdef VERBOSE
	printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif

	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else
		retval=bus_w(addr,val);



#ifdef VERBOSE
	printf("Data set to 0x%x\n",  retval);
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
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}

int read_register(int file_des) {

	int retval;
	int ret=OK;
	int arg;
	int addr;
	int n;


	sprintf(mess,"Can't read register\n");

	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	addr=arg;



#ifdef VERBOSE
	printf("reading  register 0x%x\n", addr);
#endif

	retval=bus_r(addr);


#ifdef VERBOSE
	printf("Returned value 0x%x\n",  retval);
#endif
	if (ret==FAIL) {
		ret=FAIL;
		printf("Reading register 0x%x failed\n", addr);
	} else if (differentClients)
		ret=FORCE_UPDATE;


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}






int set_channel(int file_des) {
	int ret=OK;
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
#ifdef VERBOSE
	printf("channel number is %d, chip number is %d, module number is %d, register is %lld\n", myChan.chan,myChan.chip, myChan.module, myChan.reg);
#endif

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
	/* Maybe this is done inside the initialization funcs */
	//copyChannel(detectorChans[myChan.module][myChan.chip]+(myChan.chan), &myChan);



	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}


	/*return ok/fail*/
	return ret;

}




int get_channel(int file_des) {

	int ret=OK;
	sls_detector_channel retval;

	int arg[3];
	int ichan, ichip, imod;
	int n;

	sprintf(mess,"Can't get channel\n");



	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ichan=arg[0];
	ichip=arg[1];
	imod=arg[2];

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


	if (ret==OK) {
		ret=getChannel(&retval);
		if (differentClients && ret==OK)
			ret=FORCE_UPDATE;
	}

#ifdef VERBOSE
	printf("Returning channel %d %d %d, 0x%llx\n", retval.chan, retval.chip, retval.mod, (retval.reg));
#endif 

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendChannel(file_des, &retval);
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}



	/*return ok/fail*/
	return ret;


}


int set_chip(int file_des) {

	sls_detector_chip myChip;
	int *ch;
	int n, retval;
	int ret=OK;


	myChip.nchan=getNumberOfChannelsPerChip();
	ch=malloc((myChip.nchan)*sizeof(int));
	myChip.chanregs=ch;




#ifdef VERBOSE
	printf("Setting chip\n");
#endif
	ret=receiveChip(file_des, &myChip);
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
	/* Maybe this is done inside the initialization funcs */
	//copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

	if (differentClients && ret==OK)
		ret=FORCE_UPDATE;
	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	free(ch);

	return ret;
}




int get_chip(int file_des) {


	int ret=OK;
	sls_detector_chip retval;
	int arg[2];
	int  ichip, imod;
	int n;
	int *ch;


	retval.nchan=getNumberOfChannelsPerChip();
	ch=malloc((retval.nchan)*sizeof(int));
	retval.chanregs=ch;


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ichip=arg[0];
	imod=arg[1];


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

	if (ret==OK) {
		ret=getChip(&retval);
		if (differentClients && ret==OK)
			ret=FORCE_UPDATE;
	}

#ifdef VERBOSE
	printf("Returning chip %d %d\n",  ichip, imod);
#endif 


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendChip(file_des, &retval);
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	free(ch);

	/*return ok/fail*/
	return ret;


}
int set_module(int file_des) {
	sls_detector_module myModule;
	int *myChip=malloc(getNumberOfChipsPerModule()*sizeof(int));
	int *myChan=malloc(getNumberOfChannelsPerModule()*sizeof(int));
	int *myDac=malloc(getNumberOfDACsPerModule()*sizeof(int));
	int *myAdc=malloc(getNumberOfADCsPerModule()*sizeof(int));
	int retval, n;
	int ret=OK;


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

	myModule.ndac=getNumberOfADCsPerModule();
	myModule.nchip=getNumberOfChipsPerModule();
	myModule.nchan=getNumberOfChannelsPerModule();
	myModule.nadc=getNumberOfADCsPerModule();


#ifdef VERBOSE
	printf("Setting module\n");
#endif 
	ret=receiveModule(file_des, &myModule);


	if (ret>=0)
		ret=OK;
	else
		ret=FAIL;


#ifdef VERBOSE
	printf("module number is %d,register is %d, nchan %d, nchip %d, ndac %d, nadc %d, gain %f, offset %f\n",myModule.module, myModule.reg, myModule.nchan, myModule.nchip, myModule.ndac,  myModule.nadc, myModule.gain,myModule.offset);
#endif


	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else {
			retval=setModule(myModule);
		}
	}

	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* Maybe this is done inside the initialization funcs */
	//copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	free(myChip);
	free(myChan);
	free(myDac);
	free(myAdc);

	return ret;
}




int get_module(int file_des) {


	int ret=OK;


	int arg;
	int  imod;
	int n;



	sls_detector_module myModule;
	int *myChip=malloc(getNumberOfChipsPerModule()*sizeof(int));
	int *myChan=malloc(getNumberOfChannelsPerModule()*sizeof(int));
	int *myDac=malloc(getNumberOfDACsPerModule()*sizeof(int));
	int *myAdc=malloc(getNumberOfADCsPerModule()*sizeof(int));


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

	myModule.ndac=getNumberOfDACsPerModule();
	myModule.nchip=getNumberOfChipsPerModule();
	myModule.nchan=getNumberOfChannelsPerModule();
	myModule.nadc=getNumberOfADCsPerModule();





	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	imod=arg;

	if (ret==OK) {
		ret=FAIL;
		if (imod>=0) {
			ret=OK;
			myModule.module=imod;
			getModule(&myModule);

#ifdef VERBOSE
			printf("Returning module %d of register %x\n",  imod, myModule.reg);
#endif 
		}
	}

	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendModule(file_des, &myModule);
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}



	free(myChip);
	free(myChan);
	free(myDac);
	free(myAdc);








	/*return ok/fail*/
	return ret;

}





int set_settings(int file_des) {

	int retval;
	int ret=OK;
	int arg[2];
	int n;
	int  imod;
	enum detectorSettings isett;


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	imod=arg[1];
	isett=arg[0];


	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}

#ifdef VERBOSE
	printf("Changing settings of module %d to %d\n", imod,  isett);
#endif

	if (differentClients==1 && lockStatus==1 && arg[0]!=GET_SETTINGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		retval=setSettings(arg[0], imod);
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
	if (ret==OK && differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else
		n += sendDataOnly(file_des,&retval,sizeof(retval));



	return ret;


}






int get_threshold_energy(int file_des) { 
	int retval;
	int ret=OK;
	int n;
	int  imod;


	n = receiveDataOnly(file_des,&imod,sizeof(imod));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef VERBOSE
	printf("Getting threshold energy of module %d\n", imod);
#endif 

	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}

	retval=getThresholdEnergy(imod);


#ifdef VERBOSE
	printf("Threshold is %d eV\n",  retval);
#endif  


	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else
		n += sendDataOnly(file_des,&retval,sizeof(retval));


	/* Maybe this is done inside the initialization funcs */
	//detectorDacs[imod][ind]=val;
	/*return ok/fail*/
	return ret;

}




int set_threshold_energy(int file_des) { 
	int retval;
	int ret=OK;
	int arg[3];
	int n;
	int ethr, imod;
	enum detectorSettings isett;


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ethr=arg[0];
	imod=arg[1];
	isett=arg[2];

	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number %d out of range\n",imod);
	}


#ifdef VERBOSE
	printf("Setting threshold energy of module %d to %d eV with settings %d\n", imod, ethr, isett);
#endif 

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
	if (ret==OK && differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else
		n += sendDataOnly(file_des,&retval,sizeof(retval));


	/* Maybe this is done inside the initialization funcs */
	//detectorDacs[imod][ind]=val;
	/*return ok/fail*/
	return ret;

}




int start_acquisition(int file_des) {

	int ret=OK;
	int n;


	sprintf(mess,"can't start acquisition\n");

#ifdef VERBOSE
	printf("Starting acquisition\n");
#endif

	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=startStateMachine();
	}
	if (ret==FAIL)
		sprintf(mess,"Start acquisition failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	return ret;

}

int stop_acquisition(int file_des) {

	int ret=OK;
	int n;


	sprintf(mess,"can't stop acquisition\n");

#ifdef VERBOSE
	printf("Stopping acquisition\n");
#endif 


	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=stopStateMachine();
	}

	if (ret==FAIL)
		sprintf(mess,"Stop acquisition failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	return ret;


}




int start_readout(int file_des) {


	int ret=OK;
	int n;


	sprintf(mess,"can't start readout\n");

#ifdef VERBOSE
	printf("Starting readout\n");
#endif     
	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=startReadOut();
	}
	if (ret==FAIL)
		sprintf(mess,"Start readout failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	return ret;



}




int get_run_status(int file_des) {  

	int ret=OK;
	int n;

	enum runStatus s;
	sprintf(mess,"getting run status\n");

#ifdef VERBOSE
	printf("Getting status\n");
#endif 

	s= getRunStatus();




	if (ret!=OK) {
		printf("get status failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n += sendDataOnly(file_des,&s,sizeof(s));
	}
	return ret;



}





int start_and_read_all(int file_des) {
	//int dataret=OK;
#ifdef VERBOSE
	printf("Starting and reading all frames\n");
#endif

	if (differentClients==1 && lockStatus==1) {
		dataret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		sendDataOnly(file_des,&dataret,sizeof(dataret));
		sendDataOnly(file_des,mess,sizeof(mess));
		return dataret;

	}


	startStateMachine();
	read_all(file_des);
#ifdef VERBOSE
	printf("Frames finished\n");
#endif


	return OK;


}




int read_frame(int file_des) {

#ifdef VERBOSE
	int n;
#endif

	dataret=OK;

	if (differentClients==1 && lockStatus==1) {
		dataret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		sendDataOnly(file_des,&dataret,sizeof(dataret));
		sendDataOnly(file_des,mess,sizeof(mess));
		printf("dataret %d\n",dataret);
		return dataret;
	}


	dataretval=readFrame(&dataret, mess);

	sendDataOnly(file_des,&dataret,sizeof(dataret));
	if (dataret==FAIL)
		sendDataOnly(file_des,mess,sizeof(mess));//sizeof(mess));//sizeof(mess));
	else
		sendDataOnly(file_des,dataretval,dataBytes);

	printf("dataret %d\n",dataret);
	return dataret;
}







int read_all(int file_des) {


	while(read_frame(file_des)==OK) {
#ifdef VERBOSE
		printf("frame read\n");
#endif   
		;
	}

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
	int ret=OK;


	sprintf(mess,"can't set timer\n");

	n = receiveDataOnly(file_des,&ind,sizeof(ind));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,&tns,sizeof(tns));
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
	if (ret==OK) {

		if (differentClients==1 && lockStatus==1 && tns!=-1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {
			switch(ind) {
			case FRAME_NUMBER:
				retval=setFrames(tns);
				break;
			case ACQUISITION_TIME:
				retval=setExposureTime(tns);
				break;
			case FRAME_PERIOD:
				retval=setPeriod(tns);
				break;
			case DELAY_AFTER_TRIGGER:
				retval=setDelay(tns);
				break;
			case GATES_NUMBER:
				retval=setGates(tns);
				break;
			case PROBES_NUMBER:
				retval=setProbes(tns);
				break;
			case CYCLES_NUMBER:
				retval=setTrains(tns);
				break;
			default:
				ret=FAIL;
				sprintf(mess,"timer index unknown %d\n",ind);
				break;
			}
		}
	}
	if (ret!=OK) {
		printf(mess);
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	if (ret!=OK) {
		printf(mess);
		printf("set timer failed\n");
		sprintf(mess, "set timer %d failed\n", ind);
	} else if (ind==FRAME_NUMBER) {
		/*ret=allocateRAM();*/
		if (ret!=OK)
			sprintf(mess, "could not allocate RAM for %lld frames\n", tns);
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
#ifdef VERBOSE
		printf("returning ok %d\n",sizeof(retval));
#endif

		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}

	return ret;

}








int get_time_left(int file_des) {

	enum timerIndex ind;
	int n;
	int64_t retval;
	int ret=OK;

	sprintf(mess,"can't get timer\n");
	n = receiveDataOnly(file_des,&ind,sizeof(ind));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef VERBOSE

	printf("getting time left on timer %d \n",ind);
#endif 

	if (ret==OK) {
		switch(ind) {
		case FRAME_NUMBER:
		case ACQUISITION_TIME:
		case FRAME_PERIOD:
		case DELAY_AFTER_TRIGGER:
		case GATES_NUMBER:
		case CYCLES_NUMBER:
		case PROGRESS:
			retval=getTimeLeft(ind);
			break;
		case PROBES_NUMBER:
		case ACTUAL_TIME:
		case MEASUREMENT_TIME:
			if (myDetectorType==MYTHEN) {
				retval=getTimeLeft(ind);
				break;
			}
			break;
		default:
			ret=FAIL;
			sprintf(mess,"timer index unknown %d\n",ind);
			break;
		}
	}


	if (ret!=OK) {
		printf("get time left failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;

#ifdef VERBOSE

	printf("time left on timer %d is %lld\n",ind, retval);
#endif 

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
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
	int ret=OK;


	sprintf(mess,"can't set dynamic range\n");


	n = receiveDataOnly(file_des,&dr,sizeof(dr));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


	if (differentClients==1 && lockStatus==1 && dr>=0) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		retval=setDynamicRange(dr);
	}

	if (dr>=0 && retval!=dr)
		ret=FAIL;
	if (ret!=OK) {
		sprintf(mess,"set dynamic range failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;


	dataBytes=calculateDataBytes();

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
	return ret;
}






int set_readout_flags(int file_des) {

	enum readOutFlags retval;
	enum readOutFlags arg;
	int n;
	int ret=OK;


	sprintf(mess,"can't set readout flags\n");


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef VERBOSE
	printf("setting readout flags  to %d\n",arg);
#endif

	if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {



		switch(arg) {
		case  GET_READOUT_FLAGS:
		case STORE_IN_RAM:
		case TOT_MODE:
		case CONTINOUS_RO:
		case NORMAL_READOUT:
			if (myDetectorType==MYTHEN) {
				retval=setReadOutFlags(arg);
				break;
			}
			break;
		default:
			sprintf(mess,"Unknown readout flag %d\n", arg);
			ret=FAIL;
			break;
		}
	}



	if (ret==OK) {
		if (differentClients)
			ret=FORCE_UPDATE;
		if (arg!=GET_READOUT_FLAGS && arg!=retval) {
			ret=FAIL;
			sprintf(mess,"Could not change readout flag: should be %d but is %d\n", arg, retval);
		}
	}


	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
	return ret;
}





int set_roi(int file_des) {

	dataBytes=calculateDataBytes();

	return FAIL;

}


int set_speed(int file_des) {

	enum speedVariable arg;
	int val, n;
	int ret=OK;
	int retval;

	sprintf(mess,"can't set speed variable\n");


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	n = receiveDataOnly(file_des,&val,sizeof(val));
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
			switch (arg) {
			case CLOCK_DIVIDER:
			case WAIT_STATES:
			case SET_SIGNAL_LENGTH:
			case TOT_CLOCK_DIVIDER:
			case TOT_DUTY_CYCLE:
				if (myDetectorType==MYTHEN) {
					retval=setSpeed(arg, val);
					break;
				}
				break;
			default:
				sprintf(mess,"unknown speed variable %d\n",arg);
				ret=FAIL;
				break;
			}
		}
		if (ret==OK && val>=0) {
			if (retval!=val) {
				ret=FAIL;
				sprintf(mess,"could not change speed variable %d: should be %d but is %d \n",arg, val, retval);
			}
		}
		if (differentClients && ret==OK)
			ret=FORCE_UPDATE;
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
	return ret;
}




int execute_trimming(int file_des) {

	int arg[3];
	int n;
	int ret=OK, retval;
	int imod, par1,par2;
	enum trimMode mode;

	printf("called function execute trimming\n");

	sprintf(mess,"can't set execute trimming\n");

	n = receiveDataOnly(file_des,&mode,sizeof(mode));
	printf("mode received\n");
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (mode)\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,arg,sizeof(arg));
	printf("arg received\n");
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (args)\n");
		ret=FAIL;
	}

	imod=arg[0];
	par1=arg[1];
	par2=arg[2];

	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number out of range %d\n",imod);
	}

	if (ret==OK) {

#ifdef VERBOSE
		printf("trimming module %d mode %d, parameters %d %d \n",imod,mode, par1, par2);
#endif  

		if (differentClients==1 && lockStatus==1 ) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {

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
		}
	}

	if (ret!=OK) {
		sprintf(mess,"can't set execute trimming\n");
		ret=FAIL;
	} else if (retval>0) {
		sprintf(mess,"Could not trim %d channels\n", retval);
		ret=FAIL;
	} else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	}

	return ret;
}









int configure_mac(int file_des) {

	int retval;
	int ret=OK;
	char arg[3][50];
	int n;

	int imod=0;//should be in future sent from client as -1, arg[2]
	int ipad;
	long long int imacadd;
	long long int iservermacadd;

	sprintf(mess,"Can't configure MAC\n");


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	sscanf(arg[0], "%x", &ipad);
	sscanf(arg[1], "%llx", &imacadd);
	sscanf(arg[2], "%llx", &iservermacadd);



	if (imod>=getTotalNumberOfModules()) {
		ret=FAIL;
		sprintf(mess,"Module number out of range %d\n",imod);
	}


#ifdef VERBOSE
	int i;
	printf("\ndigital_test_bit in server %d\t",digitalTestBit);
	printf("\nipadd %x\t",ipad);
	printf("destination ip is %d.%d.%d.%d = 0x%x \n",(ipad>>24)&0xff,(ipad>>16)&0xff,(ipad>>8)&0xff,(ipad)&0xff,ipad);
	printf("macad:%llx\n",imacadd);
	for (i=0;i<6;i++)
		printf("mac adress %d is 0x%x \n",6-i,(unsigned int)(((imacadd>>(8*i))&0xFF)));
	printf("server macad:%llx\n",iservermacadd);
	for (i=0;i<6;i++)
		printf("server mac adress %d is 0x%x \n",6-i,(unsigned int)(((iservermacadd>>(8*i))&0xFF)));
	printf("\n");
#endif 




#ifdef VERBOSE
	printf("Configuring MAC of module %d\n", imod);
#endif

	if (ret==OK) {
		/*retval=configureMAC(ipad,imacadd,iservermacadd,digitalTestBit);*/
		if(retval==-1) ret=FAIL;
	}

#ifdef VERBOSE
	printf("Configured MAC with retval %d\n",  retval);
#endif  
	if (ret==FAIL) {
		printf("configuring MAC of mod %d failed\n", imod);
	}


	if (differentClients)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}



int load_image(int file_des) {
	int retval;
	int ret=OK;
	int n;
	enum imageType index;
	char ImageVals[dataBytes];

	sprintf(mess,"Loading image failed\n");

	n = receiveDataOnly(file_des,&index,sizeof(index));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,ImageVals,dataBytes);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}


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

	}

	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;
}





int read_counter_block(int file_des) {

	int ret=OK;
	int n;
	int startACQ;
	//char *retval=NULL;
	char CounterVals[NCHAN*NCHIP];

	sprintf(mess,"Read counter block failed\n");

	n = receiveDataOnly(file_des,&startACQ,sizeof(startACQ));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

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

	if(ret!=FAIL){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,CounterVals,dataBytes);//1280*2
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;
}





int reset_counter_block(int file_des) {

	int ret=OK;
	int n;
	int startACQ;

	sprintf(mess,"Reset counter block failed\n");

	n = receiveDataOnly(file_des,&startACQ,sizeof(startACQ));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=resetCounterBlock(startACQ);
	}

	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL)
		n += sendDataOnly(file_des,mess,sizeof(mess));

	/*return ok/fail*/
	return ret;
}








int start_receiver(int file_des) {
	int ret=OK;
	int n=0;
	strcpy(mess,"Could not start receiver\n");

	/* execute action if the arguments correctly arrived*/
#ifdef MCB_FUNCS
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Detector locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else
		ret = startReceiver(1);

#endif


	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	/*return ok/fail*/
	return ret;
}






int stop_receiver(int file_des) {
	int ret=OK;
	int n=0;

	strcpy(mess,"Could not stop receiver\n");

	/* execute action if the arguments correctly arrived*/
#ifdef MCB_FUNCS
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Detector locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else
		ret=startReceiver(0);

#endif


	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	/*return ok/fail*/
	return ret;
}





int calibrate_pedestal(int file_des){

	int ret=OK;
	int retval=-1;
	int n;
	int frames;

	sprintf(mess,"Could not calibrate pedestal\n");

	n = receiveDataOnly(file_des,&frames,sizeof(frames));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=calibratePedestal(frames);
	}

	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL)
		n += sendDataOnly(file_des,mess,sizeof(mess));
	else
		n += sendDataOnly(file_des,&retval,sizeof(retval));

	/*return ok/fail*/
	return ret;
}



