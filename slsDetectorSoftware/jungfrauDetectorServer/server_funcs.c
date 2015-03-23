#include "sls_detector_defs.h"
#include "sls_receiver_defs.h"
#include "server_funcs.h"
#include "server_defs.h"
#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "trimming_funcs.h"
#include "registers_m.h"
#include "gitInfoMoench.h"

#define FIFO_DATA_REG_OFF     0x50<<11
// Global variables


int (*flist[256])(int);


//defined in the detector specific file
/* #ifdef MYTHEND */
/* const enum detectorType myDetectorType=MYTHEN; */
/* #elif GOTTHARDD */
/* const enum detectorType myDetectorType=GOTTHARD; */
/* #elif EIGERD */
/* const enum detectorType myDetectorType=EIGER; */
/* #elif PICASSOD */
/* const enum detectorType myDetectorType=PICASSO; */
/* #elif MOENCHD */
/* const enum detectorType myDetectorType=MOENCH; */
/* #else */
enum detectorType myDetectorType=GENERIC;
/* #endif */


extern int nModX;
extern int nModY;
extern int dataBytes;
extern int nSamples;
extern int dynamicRange;
extern int  storeInRAM;

extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;

/* global variables for optimized readout */
extern unsigned int *ram_values;
char *dataretval=NULL;
int nframes, iframes, dataret;
char mess[1000]; 

int digitalTestBit = 0;

extern int withGotthard;


int init_detector(int b, int checkType) {
  
  int i;
  if (mapCSP0()==FAIL) { printf("Could not map memory\n");
    exit(1);  
  }

  //
  
  printf("v: 0x%x\n",bus_r(FPGA_VERSION_REG));
  printf("fp: 0x%x\n",bus_r(FIX_PATT_REG));
  
  if (checkType) {
    printf("Bus test... (checktype is %d; b is %d)",checkType,b );
    for (i=0; i<1000000; i++) {
      bus_w(SET_DELAY_LSB_REG, i*100);
      bus_r(FPGA_VERSION_REG);
      if (i*100!=bus_r(SET_DELAY_LSB_REG))
	printf("ERROR: wrote 0x%x, read 0x%x\n",i*100,bus_r(SET_DELAY_LSB_REG));
    }
    printf("Finished\n");
  }else
    printf("(checktype is %d; b is %d)",checkType,b );
  //confirm if it is really moench
  switch ((bus_r(PCB_REV_REG) & DETECTOR_TYPE_MASK)>>DETECTOR_TYPE_OFFSET) {
  case MOENCH03_MODULE_ID:
    myDetectorType=MOENCH;
    printf("This is a MOENCH03 module %d\n",MOENCH);
    break;

  case JUNGFRAU_MODULE_ID:
    myDetectorType=JUNGFRAU;
    printf("This is a Jungfrau module %d\n", JUNGFRAU);
    break;

  case JUNGFRAU_CTB_ID:
    myDetectorType=JUNGFRAUCTB;
    printf("This is a Jungfrau CTB %d\n", JUNGFRAUCTB);
    break;

  default:
    myDetectorType=GENERIC;
    printf("Unknown detector type %02x\n",(bus_r(PCB_REV_REG) & DETECTOR_TYPE_MASK)>>DETECTOR_TYPE_OFFSET);


  }

  printf("Detector type is %d\n", myDetectorType);

  
  //  return OK;

  if (b) {


    resetPLL();

  bus_w16(CONTROL_REG, SYNC_RESET);
  bus_w16(CONTROL_REG, 0);
  bus_w16(CONTROL_REG, GB10_RESET_BIT);
  bus_w16(CONTROL_REG, 0);

#ifdef MCB_FUNCS
	 printf("\nBoard Revision:0x%x\n",(bus_r(PCB_REV_REG)&BOARD_REVISION_MASK));
	 //  initDetector();
    printf("Initializing Detector\n");
    //bus_w16(CONTROL_REG, SYNC_RESET); // reset registers
#endif


    // testFpga();
    // testRAM();
    // printf("ADC_SYNC_REG:%x\n",bus_r(ADC_SYNC_REG));
    //moench specific
  
    //  setPhaseShiftOnce(); //firmware.h

    prepareADC(); // server_funcs
    //setADC(-1); //already does setdaqreg and clean fifo 
    // setSettings(GET_SETTINGS,-1); 

    initDac(0);    initDac(8); //initializes the two dacs
 
    //Initialization
    setFrames(-1);
    setTrains(-1);
    setExposureTime(-1);
    setPeriod(-1);
    setDelay(-1);
    setGates(-1);

    setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
    setMaster(GET_MASTER);
    setSynchronization(GET_SYNCHRONIZATION_MODE);
    startReceiver(0); //firmware
  }
  strcpy(mess,"dummy message");
  strcpy(lastClientIP,"none");
  strcpy(thisClientIP,"none1");
  lockStatus=0;

  allocateRAM();
  return OK;
}


int decode_function(int file_des) {
  int fnum,n;
  int retval=FAIL;
#ifdef VERBOSE
  printf( "receive data\n");
#endif 
  n = receiveDataOnly(file_des,&fnum,sizeof(fnum));
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

  //#ifdef VERBOSE
  printf( "calling function fnum = %d %x %x %x\n",fnum,(unsigned int)(flist[fnum]), (unsigned int)(flist[F_READ_REGISTER]),(unsigned int)(&read_register));
  //#endif
  if (fnum<0 || fnum>255)
    fnum=255;
  retval=(*flist[fnum])(file_des);
  if (retval==FAIL)
    printf( "Error executing the function = %d \n",fnum);
  return retval;
}


int function_table() {
  int i;
  for (i=0;i<256;i++){
    flist[i]=&M_nofunc;
  }
  flist[F_EXIT_SERVER]=&exit_server;
  flist[F_EXEC_COMMAND]=&exec_command;
  flist[F_GET_DETECTOR_TYPE]=&get_detector_type;
  flist[F_SET_NUMBER_OF_MODULES]=&set_number_of_modules;
  flist[F_GET_MAX_NUMBER_OF_MODULES]=&get_max_number_of_modules;
  flist[F_SET_EXTERNAL_SIGNAL_FLAG]=&set_external_signal_flag;
  flist[F_SET_EXTERNAL_COMMUNICATION_MODE]=&set_external_communication_mode;
  flist[F_GET_ID]=&get_id;
  flist[F_DIGITAL_TEST]=&digital_test;
  flist[F_WRITE_REGISTER]=&write_register;
  flist[F_READ_REGISTER]=&read_register;
  flist[F_SET_DAC]=&set_dac;
  flist[F_GET_ADC]=&get_adc;
  flist[F_SET_CHANNEL]=&set_channel;
  flist[F_SET_CHIP]=&set_chip;
  flist[F_SET_MODULE]=&set_module;
  flist[F_GET_CHANNEL]=&get_channel;
  flist[F_GET_CHIP]=&get_chip;
  flist[F_GET_MODULE]=&get_module;
  flist[F_GET_THRESHOLD_ENERGY]=&get_threshold_energy;
  flist[F_SET_THRESHOLD_ENERGY]=&set_threshold_energy;  
  flist[F_SET_SETTINGS]=&set_settings;
  flist[F_START_ACQUISITION]=&start_acquisition;
  flist[F_STOP_ACQUISITION]=&stop_acquisition;
  flist[F_START_READOUT]=&start_readout;
  flist[F_GET_RUN_STATUS]=&get_run_status;
  flist[F_READ_FRAME]=&read_frame;
  flist[F_READ_ALL]=&read_all;
  flist[F_START_AND_READ_ALL]=&start_and_read_all;
  flist[F_SET_TIMER]=&set_timer;
  flist[F_GET_TIME_LEFT]=&get_time_left;
  flist[F_SET_DYNAMIC_RANGE]=&set_dynamic_range;
  flist[F_SET_ROI]=&set_roi;
  flist[F_SET_SPEED]=&set_speed;
  flist[F_SET_READOUT_FLAGS]=&set_readout_flags;
  flist[F_EXECUTE_TRIMMING]=&execute_trimming;
  flist[F_LOCK_SERVER]=&lock_server;
  flist[F_SET_PORT]=&set_port;
  flist[F_GET_LAST_CLIENT_IP]=&get_last_client_ip;
  flist[F_UPDATE_CLIENT]=&update_client;
  flist[F_CONFIGURE_MAC]=&configure_mac;
  flist[F_LOAD_IMAGE]=&load_image;
  flist[F_SET_MASTER]=&set_master;
  flist[F_SET_SYNCHRONIZATION_MODE]=&set_synchronization;
  flist[F_READ_COUNTER_BLOCK]=&read_counter_block;
  flist[F_RESET_COUNTER_BLOCK]=&reset_counter_block;
  flist[F_START_RECEIVER]=&start_receiver;
  flist[F_STOP_RECEIVER]=&stop_receiver;
  flist[F_CALIBRATE_PEDESTAL]=&calibrate_pedestal;
  flist[F_SET_CTB_PATTERN]=&set_ctb_pattern;
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
  int retval=FAIL;
  sendDataOnly(file_des,&retval,sizeof(retval));
  printf("closing server.");
  sprintf(mess,"closing server");
  sendDataOnly(file_des,mess,sizeof(mess));
  return GOODBYE;
}

int exec_command(int file_des) {
  char cmd[MAX_STR_LENGTH];
  char answer[MAX_STR_LENGTH];
  int retval=OK;
  int sysret=0;
  int n=0;

 /* receive arguments */
  n = receiveDataOnly(file_des,cmd,MAX_STR_LENGTH);
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    retval=FAIL;
  }

  /* execute action if the arguments correctly arrived*/
  if (retval==OK) {
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
      retval=FAIL;
    }
  } else {
    sprintf(answer,"Could not receive the command\n");
  }
  
  /* send answer */
  n = sendDataOnly(file_des,&retval,sizeof(retval));
  n = sendDataOnly(file_des,answer,MAX_STR_LENGTH);
  if (n < 0) {
    sprintf(mess,"Error writing to socket");
    retval=FAIL;
  }


  /*return ok/fail*/
  return retval; 
 
}



int get_detector_type(int file_des) {
  int n=0;
  enum detectorType ret;
  int retval=OK;
  
  sprintf(mess,"Can't return detector type\n");


  /* receive arguments */
  /* execute action */
  ret=myDetectorType;

#ifdef VERBOSE
    printf("Returning detector type %d\n",ret);
#endif

  /* send answer */  
  /* send OK/failed */
  if (differentClients==1)
    retval=FORCE_UPDATE;

  n += sendDataOnly(file_des,&retval,sizeof(retval));      
  if (retval!=FAIL) {
    /* send return argument */
    n += sendDataOnly(file_des,&ret,sizeof(ret));
  } else {
    n += sendDataOnly(file_des,mess,sizeof(mess));
  }
  /*return ok/fail*/
  return retval; 
  

}


int set_number_of_modules(int file_des) {
  int n;
  int arg[2], ret=0; 
  int retval=OK;
  int dim, nm;
  
  sprintf(mess,"Can't set number of modules\n");

  /* receive arguments */
  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket %d", n);
    retval=GOODBYE;
  }
  if (retval==OK) {
    dim=arg[0];
    nm=arg[1];

    /* execute action */
#ifdef VERBOSE
    printf("Setting the number of modules in dimension %d to %d\n",dim,nm );
#endif
    
    //if (nm!=GET_FLAG) {
      if (dim!=X && nm!=GET_FLAG) {
	retval=FAIL;
	sprintf(mess,"Can't change module number in dimension %d\n",dim);
      } else  {
	if (lockStatus==1 && differentClients==1 && nm!=GET_FLAG) {
	  sprintf(mess,"Detector locked by %s\n", lastClientIP);
	  retval=FAIL;
	} else {
	  ret=setNMod(nm);
	  if (nModX==nm || nm==GET_FLAG) {
	    retval=OK;
	    if (differentClients==1)
	      retval=FORCE_UPDATE;
	  } else
	    retval=FAIL;
	}
      }
  }
  /*} else {
    if (dim==Y) {
      ret=nModY;
    } else if (dim==X) {
      ret=setNMod(-1);
    }
  }
  */
  
  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(file_des,&retval,sizeof(retval));
  if (retval!=FAIL) {
    /* send return argument */
    n += sendDataOnly(file_des,&ret,sizeof(ret));
  } else {
    n += sendDataOnly(file_des,mess,sizeof(mess));
  }
  /*return ok/fail*/
  return retval; 
  
}


int get_max_number_of_modules(int file_des) {
  int n;
  int ret; 
  int retval=OK;
  enum dimension arg;
  
  sprintf(mess,"Can't get max number of modules\n");
  /* receive arguments */
  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    retval=FAIL;
  }
    /* execute action */
#ifdef VERBOSE
    printf("Getting the max number of modules in dimension %d \n",arg);
#endif


    switch (arg) {
    case X:
      ret=getNModBoard();
      break;
    case Y:
      ret=NMAXMODY;
      break;
    default:
      ret=FAIL;
      retval=FAIL;
      break;
    }
#ifdef VERBOSE
    printf("Max number of module in dimension %d is %d\n",arg,ret );
#endif  



    if (differentClients==1 && retval==OK) {
      retval=FORCE_UPDATE;
    }

  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(file_des,&retval,sizeof(retval));
  if (retval!=FAIL) {
    /* send return argument */
    n += sendDataOnly(file_des,&ret,sizeof(ret));
  } else {
    n += sendDataOnly(file_des,mess,sizeof(mess));
  }



  /*return ok/fail*/
  return retval; 
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
	enum externalSignalFlag flag, retval;

	sprintf(mess,"Can't set external signal flag\n");

	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	retval=SIGNAL_OFF;
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
			} else {
				if (lockStatus!=0) {
					ret=FAIL;
					sprintf(mess,"Detector locked by %s\n", lastClientIP);
				}
			}

		}

#ifdef VERBOSE
		printf("Setting external signal %d to flag %d\n",signalindex,flag );
		printf("Set to flag %d\n",retval);
#endif

	} else {
		ret=FAIL;
	}

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
	enum externalCommunicationMode arg, ret=GET_EXTERNAL_COMMUNICATION_MODE;
	int retval=OK;

	sprintf(mess,"Can't set external communication mode\n");


	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		retval=FAIL;
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
	if (retval==OK) {
		/* execute action */

		ret=setTiming(arg);

		/*     switch(arg) { */
		/*     default: */
		/*       sprintf(mess,"The meaning of single signals should be set\n"); */
		/*       retval=FAIL; */
		/*     } */


#ifdef VERBOSE
		printf("Setting external communication mode to %d\n", arg);
#endif
	} else
		ret=FAIL;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	if (retval!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&ret,sizeof(ret));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return retval;
}



int get_id(int file_des) {
  // sends back 64 bits!
  int64_t retval=-1;
  int ret=OK;
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
  case DETECTOR_SERIAL_NUMBER:
    retval=getDetectorNumber();
    break;
  case DETECTOR_FIRMWARE_VERSION:
    retval=getFirmwareSVNVersion();
    retval=(retval <<32) | getFirmwareVersion();
    break;
  case DETECTOR_SOFTWARE_VERSION:
	retval= SVNREV;
	retval= (retval <<32) | SVNDATE;
    break;
/*  case DETECTOR_FIRMWARE_SVN_VERSION:
    retval=getFirmwareSVNVersion();
    break;*/
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
  int ibit=0;
  int ow;
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
      retval=0;
#ifdef MCB_FUNCS  
      if (differentClients==1 && lockStatus==1) {
	ret=FAIL;
	sprintf(mess,"Detector locked by %s\n",lastClientIP);
	break;
      }
      if (imod >= nModX) {
	ret=FAIL;
	sprintf(mess,"Module %d disabled\n",imod);
	break;
      }
      if (testShiftIn(imod)) retval|=(1<<(ibit));
      ibit++;
      if (testShiftOut(imod)) retval|=(1<<(ibit));
      ibit++;
      if (testShiftStSel(imod)) retval|=(1<<(ibit));
      ibit++;
      //if ( testDataInOut(0x123456, imod)) retval|=(1<<(ibit++));
      //if ( testExtPulse(imod)) retval|=(1<<(ibit++));
      //  for (ow=0; ow<6; ow++)
      // ow=1;
      //#ifndef PICASSOD
      for (ow=0; ow<5; ow++) {
	//#endif
	if (testDataInOutMux(imod, ow, 0x789abc)) retval|=(1<<ibit);
	ibit++;
      }
      //for (ow=0; ow<6; ow++)
	// ow=1;
      //#ifndef PICASSOD
      for (ow=0; ow<5; ow++) {
	//#endif
	if (testExtPulseMux(imod, ow)) retval|=(1<<ibit);
	ibit++;
      }
      //#ifndef PICASSOD
      if ( testOutMux(imod)) retval|=(1<<(ibit));
      ibit++;
      if (testFpgaMux(imod)) retval|=(1<<(ibit));
      ibit++;
      //#endif
     
#endif 
    break;
  case MODULE_FIRMWARE_TEST:
    retval=0x2;
    break;
  case DETECTOR_FIRMWARE_TEST:
    retval=testFpga();
    break;
  case DETECTOR_MEMORY_TEST:
    ret=testRAM();
    break;
  case DETECTOR_BUS_TEST:
    retval=testBus();
      break;
  case DETECTOR_SOFTWARE_TEST:
    retval=testFpga();
    break;
  case DIGITAL_BIT_TEST:
	  n = receiveDataOnly(file_des,&ival,sizeof(ival));
	  if (n < 0) {
		  sprintf(mess,"Error reading from socket\n");
		  retval=FAIL;
	  }
#ifdef VERBOSE
	  printf("with value %d\n", ival);
#endif
	  if (differentClients==1 && lockStatus==1) {
		  ret=FAIL;
		  sprintf(mess,"Detector locked by %s\n",lastClientIP);
		  break;
	  }
	  digitalTestBit = ival;
	  retval=digitalTestBit;
	  break;
  default:
    printf("Unknown digital test required %d\n",arg);
    ret=FAIL;
    retval=FAIL;
    break;
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

int write_register(int file_des) {

  int retval;
  int ret=OK;
  int arg[2]; 
  int addr, val;
  int n;
  u_int32_t address;

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
  }


  if(ret!=FAIL){
    address=(addr<<11);
    if((address==FIFO_DATA_REG_OFF)||(address==CONTROL_REG))
    	ret = bus_w16(address,val);
    else
    	ret=bus_w(address,val);
    if(ret==OK){
    	if((address==FIFO_DATA_REG_OFF)||(address==CONTROL_REG))
        	retval=bus_r16(address);
        else
        	retval=bus_r(address);
    }
  }
  

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
  u_int32_t address;

  sprintf(mess,"Can't read register\n");

  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  addr=arg;

 

  //#ifdef VERBOSE
  printf("reading  register 0x%x\n", addr);
  //#endif

  if(ret!=FAIL){
	  address=(addr<<11);
	  if((address==FIFO_DATA_REG_OFF)||(address==CONTROL_REG))
		  retval=bus_r16(address);
	  else
		  retval=bus_r(address);
  }



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

int set_dac(int file_des) {
	//default:all mods
  int retval, retval1;
	int ret=OK;
	int arg[3];
	enum dacIndex ind;
	int imod;
	int n;
	int val;
	int idac=0;
	int mV=0;
	sprintf(mess,"Can't set DAC\n");

	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	imod=arg[1];
	mV=arg[3];
	n = receiveDataOnly(file_des,&val,sizeof(val));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	//#ifdef VERBOSE
	printf("Setting DAC %d of module %d to %d , mode %d\n", ind, imod, val, mV);
	//#endif 

	if (imod>=getNModBoard())
		ret=FAIL;
	if (imod<0)
		imod=ALLMOD;

	
	

#ifdef MCB_FUNCS

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
		  
		  if (mV) {
		    if (val>2500)
		      val=-1;
		    printf("%d mV is ",val);
		    if (val>0)
		      val=16535*val/2500;
		    printf("%d DACu\n", val);
		  } else if (val>16535)
		    val=-1;
		 
		  
		  retval=setDac(ind,val);
/* 			if(idac==HIGH_VOLTAGE) */
/* 				retval=initHighVoltageByModule(val,imod); */
/* 			else */
/* 				retval=initDACbyIndexDACU(idac,val,imod); */
		}
	}
	if(ret==OK){
	/* 	ret=FAIL; */
/* 		if(idac==HIGH_VOLTAGE){ */
/* 			if(retval==-2) */
/* 				strcpy(mess,"Invalid Voltage.Valid values are 0,90,110,120,150,180,200"); */
/* 			else if(retval==-3) */
/* 				strcpy(mess,"Weird value read back or it has not been set yet\n"); */
/* 			else */
/* 				ret=OK; */
/* 		}//since v r saving only msb */
/* 		else if ((retval-val)<=3 || val==-1) */
/* 			ret=OK; */
	  
	  if (mV) {
	    
	    printf("%d DACu is ",retval);
	    retval1=2500*retval/16535;
	    printf("%d mV \n",retval1);
	  } else
	    retval1=retval;
	}
#endif

#ifdef VERBOSE
	printf("DAC set to %d V\n",  retval);
#endif  

	if(ret==FAIL)
		printf("Setting dac %d of module %d: wrote %d but read %d\n", ind, imod, val, retval);
	else{
		if (differentClients)
			ret=FORCE_UPDATE;
	}


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
		n += sendDataOnly(file_des,&retval1,sizeof(retval1));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}


	/*return ok/fail*/
	return ret;

}



int get_adc(int file_des) {
	//default: mod 0
	int retval;
	int ret=OK;
	int arg[2];
	enum dacIndex ind;
	int imod;
	int n;
	int idac=0;

	sprintf(mess,"Can't read ADC\n");


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	imod=arg[1];

#ifdef VERBOSE
	printf("Getting ADC %d of module %d\n", ind, imod);
#endif

	if (imod>=getNModBoard() || imod<0)
		ret=FAIL;

#ifdef MCB_FUNCS
	switch (ind) {
	case TEMPERATURE_FPGA:
		idac=TEMP_FPGA;
		break;
	case TEMPERATURE_ADC:
		idac=TEMP_ADC;
		break;
	default:
		printf("Unknown DAC index %d\n",ind);
		sprintf(mess,"Unknown DAC index %d\n",ind);
		ret=FAIL;
	    break;
	}

	if (ret==OK)
		retval=getTemperatureByModule(idac,imod);
#endif

#ifdef VERBOSE
	printf("ADC is %d V\n",  retval);
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
  
  if (ret==OK) {
    if (myChan.module>=getNModBoard()) 
      ret=FAIL;
    if (myChan.chip>=NCHIP) 
      ret=FAIL;
    if (myChan.chan>=NCHAN) 
      ret=FAIL;
    if (myChan.module<0)
      myChan.module=ALLMOD;
  }

 
  if (ret==OK) {
    if (differentClients==1 && lockStatus==1) {
      ret=FAIL;
      sprintf(mess,"Detector locked by %s\n",lastClientIP);  
    } else {
#ifdef MCB_FUNCS
    retval=initChannelbyNumber(myChan);
#endif
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
 
  if (ret==OK) {
    ret=FAIL;
    if (imod>=0 && imod<getNModBoard()) {
      ret=OK;
    }
  }  
  if (ret==OK) {
    ret=FAIL;
    if (ichip>=0 && ichip<NCHIP) {
      ret=OK;
    }
  }   
  if (ret==OK) {
    ret=FAIL;
    if (ichan>=0 && ichan<NCHAN) {
      ret=OK;
    }
  }   


  if (ret==OK) {
#ifdef MCB_FUNCS
      ret=getChannelbyNumber(&retval);
#endif
    if (differentClients && ret==OK)
      ret=FORCE_UPDATE;
  } 

#ifdef VERBOSE
  printf("Returning channel %d %d %d, 0x%llx\n", ichan, ichip, imod, (retval.reg));
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
  int ch[NCHAN];
  int n, retval;
  int ret=OK;
  

  myChip.nchan=NCHAN;
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
  
  if (ret==OK) {
    if (myChip.module>=getNModBoard()) 
      ret=FAIL;
    if  (myChip.module<0) 
      myChip.module=ALLMOD;
    if (myChip.chip>=NCHIP) 
      ret=FAIL;
  }
    if (differentClients==1 && lockStatus==1) {
      ret=FAIL;
      sprintf(mess,"Detector locked by %s\n",lastClientIP);  
    } else {
#ifdef MCB_FUNCS
  retval=initChipbyNumber(myChip);
#endif
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


  return ret;
}

int get_chip(int file_des) {

  
  int ret=OK;
  sls_detector_chip retval;
  int arg[2];
  int  ichip, imod;
  int n;
  


  n = receiveDataOnly(file_des,arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  ichip=arg[0];
  imod=arg[1];
 if (ret==OK) {
    ret=FAIL;
    if (imod>=0 && imod<getNModBoard()) {
      ret=OK;
    }
  }  
  if (ret==OK) {
    ret=FAIL;
    if (ichip>=0 && ichip<NCHIP) {
      ret=OK;
    }
  }   
 

 
  if (ret==OK) {
#ifdef MCB_FUNCS
    ret=getChipbyNumber(&retval);
#endif
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


  
  /*return ok/fail*/
  return ret; 


}
int set_module(int file_des) {
  sls_detector_module myModule;
  int *myChip=malloc(NCHIP*sizeof(int));
  int *myChan=malloc(NCHIP*NCHAN*sizeof(int));
  int *myDac=malloc(NDAC*sizeof(int));/**dhanya*/
  int *myAdc=malloc(NADC*sizeof(int));/**dhanya*/
  int retval, n;
  int ret=OK;
  int dr;// ow;

  dr=setDynamicRange(-1);

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

  myModule.ndac=NDAC;
  myModule.nchip=NCHIP;
  myModule.nchan=NCHAN*NCHIP;
  myModule.nadc=NADC;
  
  
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
	  if (myModule.module>=getNModBoard()) {
      ret=FAIL;
      printf("Module number is too large %d\n",myModule.module);
    }
    if (myModule.module<0) 
      myModule.module=ALLMOD;
  }
    
  if (ret==OK) {
    if (differentClients==1 && lockStatus==1) {
      ret=FAIL;
      sprintf(mess,"Detector locked by %s\n",lastClientIP);  
    } else {
#ifdef MCB_FUNCS
    retval=initModulebyNumber(myModule);
#endif
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

  //  setDynamicRange(dr);  always 16 commented out

  
  return ret;
}




int get_module(int file_des) {

  
  int ret=OK;


  int arg;
  int   imod;
  int n;

  

  sls_detector_module myModule;
  int *myChip=malloc(NCHIP*sizeof(int));
  int *myChan=malloc(NCHIP*NCHAN*sizeof(int));
  int *myDac=malloc(NDAC*sizeof(int));/**dhanya*/
  int *myAdc=malloc(NADC*sizeof(int));/**dhanya*/


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

  myModule.ndac=NDAC;
  myModule.nchip=NCHIP;
  myModule.nchan=NCHAN*NCHIP;
  myModule.nadc=NADC;
  




  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  imod=arg;

  if (ret==OK) {
   ret=FAIL;
   if (imod>=0 && imod<getNModBoard()) {
     ret=OK;
     myModule.module=imod;
#ifdef MCB_FUNCS
     getModulebyNumber(&myModule);
#endif

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


int get_threshold_energy(int file_des) { 
  int ret=FAIL;
  int n;
  int  imod;

  strcpy(mess,"cannot set threshold for moench");

  n = receiveDataOnly(file_des,&imod,sizeof(imod));
  if (n < 0)
    sprintf(mess,"Error reading from socket\n");


  /* send answer */
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  n += sendDataOnly(file_des,mess,sizeof(mess));

  /*return ok/fail*/
  return OK;

}
 
int set_threshold_energy(int file_des) { 
  int ret=FAIL;
  int arg[3];
  int n;

  strcpy(mess,"cannot set threshold for moench");

  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0)
    sprintf(mess,"Error reading from socket\n");


  /* send answer */
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  n += sendDataOnly(file_des,mess,sizeof(mess));

  /*return ok/fail*/
  return OK;

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
  

#ifdef VERBOSE
  printf("Changing settings of module %d to %d\n", imod,  isett);
#endif 

  if (differentClients==1 && lockStatus==1 && arg[0]!=GET_SETTINGS) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);  
  } else {
#ifdef MCB_FUNCS
    retval=setSettings(arg[0],imod);
#endif
#ifdef VERBOSE
    printf("Settings changed to %d\n",retval);
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
  
  int retval;
  enum runStatus s;
  sprintf(mess,"getting run status\n");

#ifdef VERBOSE
  printf("Getting status\n");
#endif 

  retval= runState();
  printf("\n\nSTATUS=%08x\n",retval);

  //error
  if(retval&SOME_FIFO_FULL_BIT){
	  printf("-----------------------------------ERROR--------------------------------------x%0x\n",retval);
	  s=ERROR;
  }
  //runbusy=0
  // else if(!(retval&RUNMACHINE_BUSY_BIT)){ //commented by Anna 24.10.2012
    else if(!(retval&RUN_BUSY_BIT)){ // by Anna 24.10.2012
    


	  //and readbusy=1, its last frame read
      if((retval&READMACHINE_BUSY_BIT)  ){ //


		  printf("-----------------------------------READ MACHINE BUSY--------------------------\n");
		  s=TRANSMITTING;
      } else if (!(retval&ALL_FIFO_EMPTY_BIT)) {
		  printf("-----------------------------------DATA IN FIFO--------------------------\n");
		  s=TRANSMITTING;
	
      }
	  //and readbusy=0,idle
      else if(!(retval&0xffff)){
		  //if(!(retval&0x00000001)){
		  printf("-----------------------------------IDLE--------------------------------------\n");
		  s=IDLE;
	  } else {
	    printf("-----------------------------------Unknown status %08x--------------------------------------\n", retval);
	    s=ERROR;
	    ret=FAIL;
	  }
  }
  //if runbusy=1
    else {
	  if (retval&WAITING_FOR_TRIGGER_BIT){
		  printf("-----------------------------------WAITING-----------------------------------\n");
		  s=WAITING;
	  }
	  else{
		  printf("-----------------------------------RUNNING-----------------------------------\n");
		  s=RUNNING;
	  }
  }




  if (ret!=OK) {
    printf("get status failed %04x\n",retval);
    sprintf(mess, "get status failed %08x\n", retval);
    
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

int read_frame(int file_des) {


  int ns=0;
  u_int16_t* p=NULL;


  if (differentClients==1 && lockStatus==1) {
    dataret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);  
    sendDataOnly(file_des,&dataret,sizeof(dataret));
    sendDataOnly(file_des,mess,sizeof(mess));
#ifdef VERBOSE
    printf("dataret %d\n",dataret);
#endif
    return dataret;

  }
  p=fifo_read_frame();
  if (p) {
    nframes++;
    dataretval=(char*)ram_values;
    dataret=OK;
#ifdef VERBOSE
      printf("sending data of %d frames\n",nframes);
#endif
	sendDataOnly(file_des,&dataret,sizeof(dataret));
#ifdef VERYVERBOSE
	  printf("sending pointer %x of size %d\n",(unsigned int)(dataretval),dataBytes*nSamples);
#endif
	  sendDataOnly(file_des,dataretval,dataBytes*nSamples);
  } else  {
      if (getFrames()>-1) {
	dataret=FAIL;
	sprintf(mess,"no data and run stopped: %d frames left\n",(int)(getFrames()+2));
	printf("%s\n",mess);
      } else {
	dataret=FINISHED;
	sprintf(mess,"acquisition successfully finished\n");
	printf("%s\n",mess);
	if (differentClients)
	  dataret=FORCE_UPDATE;
      }
#ifdef VERBOSE
      printf("Frames left %d\n",(int)(getFrames()));
#endif
      sendDataOnly(file_des,&dataret,sizeof(dataret));
      sendDataOnly(file_des,mess,sizeof(mess));
  }
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

  /*  ret=startStateMachine();  
      if (ret!=OK) {
      sprintf(mess,"could not start state machine\n");
    sendDataOnly(file_des,&ret,sizeof(ret));
    sendDataOnly(file_des,mess,sizeof(mess));
    
    #ifdef VERBOSE
    printf("could not start state machine\n");
#endif
} else {*/
  read_all(file_des);
#ifdef VERBOSE
  printf("Frames finished\n");
#endif   
  //}


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
    sprintf(mess,"can't set timer for moench\n");
    ret=FAIL;
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
  } else if (ind==FRAME_NUMBER) {
    //  ret=allocateRAM();
    // if (ret!=OK) 
    //   sprintf(mess, "could not allocate RAM for %lld frames\n", tns);
  }

  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n = sendDataOnly(file_des,mess,sizeof(mess));
  } else {
#ifdef VERBOSE
  printf("returning ok %d\n",(int)(sizeof(retval)));
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
  

  //#ifdef VERBOSE

  printf("getting time left on timer %d \n",ind);
  //#endif

  if (ret==OK) {
    switch(ind) {
    case FRAME_NUMBER:
       printf("getting frames \n");
      retval=getFrames(); 
      break;
    case ACQUISITION_TIME: 
      retval=getExposureTime();
      break;
    case FRAME_PERIOD: 
      retval=getPeriod();
      break;
    case DELAY_AFTER_TRIGGER: 
      retval=getDelay();
    break;
    case GATES_NUMBER:
      retval=getGates();
      break;
    case PROBES_NUMBER: 
      retval=getProbes();
      break;
    case CYCLES_NUMBER: 
      retval=getTrains();
      break;
    case PROGRESS: 
      retval=getProgress();
      break;
    case ACTUAL_TIME:
      retval=getActualTime();
      break;
    case MEASUREMENT_TIME: 
      retval=getMeasurementTime();
      break;
    case FRAMES_FROM_START:
    case FRAMES_FROM_START_PG:
      retval=getFramesFromStart();
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

  //#ifdef VERBOSE

  printf("time left on timer %d is %lld\n",ind, retval);
  //#endif

  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n += sendDataOnly(file_des,mess,sizeof(mess));
  } else
    n = sendDataOnly(file_des,&retval,sizeof(retval));

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
  
  printf("Set dynamic range?\n");
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

  //if (dr>=0 && retval!=dr)   ret=FAIL;
  if (ret!=OK) {
    sprintf(mess,"set dynamic range failed\n");
  } else {
  /*   ret=allocateRAM(); */
/*     if (ret!=OK) */
/*       sprintf(mess,"Could not allocate RAM for the dynamic range selected\n"); */
//    else 
 if (differentClients)
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

int set_roi(int file_des) {

	int i;
	int ret=OK;
	int nroi=-1;
	int n=0;
	int retvalsize=0;
	ROI arg[MAX_ROIS];
	ROI* retval=0;

	strcpy(mess,"Could not set/get roi\n");


	n = receiveDataOnly(file_des,&nroi,sizeof(nroi));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if(nroi!=-1){
		n = receiveDataOnly(file_des,arg,nroi*sizeof(ROI));
		if (n != (nroi*sizeof(ROI))) {
			sprintf(mess,"Received wrong number of bytes for ROI\n");
			ret=FAIL;
		}
//#ifdef VERBOSE
		/*
		printf("Setting ROI to:");
		for( i=0;i<nroi;i++)
			printf("%d\t%d\t%d\t%d\n",arg[i].xmin,arg[i].xmax,arg[i].ymin,arg[i].ymax);
*/

		printf("Error: Function 41 or Setting ROI is not yet implemented in Moench!\n");
//#endif
	}
	/* execute action if the arguments correctly arrived*/


	ret = FAIL;
	/* NOT IMPLEMENTED
#ifdef MCB_FUNCS
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
*/
	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	else{
		sendDataOnly(file_des,&retvalsize,sizeof(retvalsize));
		sendDataOnly(file_des,retval,retvalsize*sizeof(ROI));
	}
	/*return ok/fail*/
	return ret;
}

int get_roi(int file_des) {


  return FAIL;
}

int set_speed(int file_des) {

  enum speedVariable arg;
  int val,n;
  int ret=OK;
  int retval;
  
  n=receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  n=receiveDataOnly(file_des,&val,sizeof(val));
   if (n < 0) {
     sprintf(mess,"Error reading from socket\n");
     ret=FAIL;
   }
  
  
  
  if (ret==OK) {

    if (val!=-1) {
      if (differentClients==1 && lockStatus==1 && val>=0) {
	ret=FAIL;
	sprintf(mess,"Detector locked by %s\n",lastClientIP);
      }  else {
	switch (arg) {
	case CLOCK_DIVIDER:
	  retval=setClockDivider(val,0);
	  break;

	case PHASE_SHIFT:
	  retval=phaseStep(val,0);
	  break;

	case OVERSAMPLING:
	  retval=setOversampling(val);
	  break;

	case ADC_CLOCK:
	  retval=setClockDivider(val,1);
	  break;

	case ADC_PHASE:
	  retval=phaseStep(val,1);
	  break;


	case ADC_PIPELINE:
	  retval=adcPipeline(val);
	  break;



	default:
	  ret=FAIL;
	  sprintf(mess,"Unknown speed parameter %d",arg);
	}
      }
    }


    switch (arg) {
    case CLOCK_DIVIDER:
      retval=getClockDivider(0);
      break;

    case PHASE_SHIFT:
      // retval=phaseStep(-1);
      //ret=FAIL;
      //sprintf(mess,"Cannot read phase",arg);
      retval=-1;
      break;

    case OVERSAMPLING:
      retval=setOversampling(-1);
      break;

    case ADC_CLOCK:
      retval=getClockDivider(1);
      break;

    case ADC_PHASE:
      retval=-1;
      break;


    case ADC_PIPELINE:
      retval=adcPipeline(-1);
      break;
      

    default:
      ret=FAIL;
      sprintf(mess,"Unknown speed parameter %d",arg);
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



int set_readout_flags(int file_des) {

  enum readOutFlags arg;
  int n;
  int ret=FAIL;
  

  receiveDataOnly(file_des,&arg,sizeof(arg));

  sprintf(mess,"can't set readout flags for moench\n");
  
  sendDataOnly(file_des,&ret,sizeof(ret));
  sendDataOnly(file_des,mess,sizeof(mess));

  return ret; 
}





int execute_trimming(int file_des) {
 
  int arg[3];
  int ret=FAIL;
  enum trimMode mode;
  
  sprintf(mess,"can't set execute trimming for moench\n");
  
  receiveDataOnly(file_des,&mode,sizeof(mode));
  receiveDataOnly(file_des,arg,sizeof(arg));


  sendDataOnly(file_des,&ret,sizeof(ret));
  sendDataOnly(file_des,mess,sizeof(mess));

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
    if (lockStatus==0 || strcmp(lastClientIP,thisClientIP)==0 || strcmp(lastClientIP,"none")==0)
      lockStatus=lock;
    else {
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

int get_last_client_ip(int file_des) {
  int ret=OK;
  int n;
 if (differentClients )
   ret=FORCE_UPDATE;
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));
  
  return ret;

}


int send_update(int file_des) {

  int ret=OK;
  enum detectorSettings t;
  int n;//int thr, n;
  //int it;
  int64_t retval, tns=-1;
  n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));
  n = sendDataOnly(file_des,&nModX,sizeof(nModX));
  n = sendDataOnly(file_des,&nModY,sizeof(nModY));
  n = sendDataOnly(file_des,&dynamicRange,sizeof(dynamicRange));
  n = sendDataOnly(file_des,&dataBytes,sizeof(dataBytes));
  t=setSettings(GET_SETTINGS,-1);
  n = sendDataOnly(file_des,&t,sizeof(t));
/*  thr=getThresholdEnergy();
  n = sendDataOnly(file_des,&thr,sizeof(thr));*/
  retval=setFrames(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));
  retval=setExposureTime(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));
  retval=setPeriod(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));
  retval=setDelay(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));
  retval=setGates(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));
/*  retval=setProbes(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));*/
  retval=setTrains(tns);
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


int configure_mac(int file_des) {

	int ret=OK;
	char arg[5][50];
	int n;

	int imod=0;//should be in future sent from client as -1, arg[2]
	int ipad;
	long long int imacadd;
	long long int idetectormacadd;
	int udpport;
	int detipad;
	int retval=-100;

	sprintf(mess,"Can't configure MAC\n");


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	sscanf(arg[0], "%x", 		&ipad);
	sscanf(arg[1], "%llx", 	&imacadd);
	sscanf(arg[2], "%x", 		&udpport);
	sscanf(arg[3], "%llx",	&idetectormacadd);
	sscanf(arg[4], "%x",		&detipad);

#ifdef VERBOSE
	int i;
	printf("\ndigital_test_bit in server %d\t",digitalTestBit);
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
	printf("\n");
#endif



	if (imod>=getNModBoard())
		ret=FAIL;
	if (imod<0)
		imod=ALLMOD;

	//#ifdef VERBOSE
	printf("Configuring MAC of module %d at port %x\n", imod, udpport);
	//#endif
#ifdef MCB_FUNCS
	if (ret==OK){
		if(runBusy()){
			ret=stopStateMachine();
			if(ret==FAIL)
				strcpy(mess,"could not stop detector acquisition to configure mac");
		}

		if(ret==OK)
			configureMAC(ipad,imacadd,idetectormacadd,detipad,digitalTestBit,udpport);
		retval=getAdcConfigured();
	}
#endif
	if (ret==FAIL)
		printf("configuring MAC of mod %d failed\n", imod);
	else
		printf("Configuremac successful of mod %d and adc %d\n",imod,retval);

	if (differentClients)
		ret=FORCE_UPDATE;

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



int load_image(int file_des) {
	int retval;
	int ret=OK;
	int n;
	enum imageType index;
	short int ImageVals[NCHAN*NCHIP];

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

	switch (index) {
	case DARK_IMAGE :
#ifdef VERBOSE
		printf("Loading Dark image\n");
#endif
		break;
	case GAIN_IMAGE :
#ifdef VERBOSE
		printf("Loading Gain image\n");
#endif
		break;
	default:
		printf("Unknown index %d\n",index);
		sprintf(mess,"Unknown index %d\n",index);
		ret=FAIL;
	    break;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
			retval=loadImage(index,ImageVals);
			if (retval==-1)
				ret = FAIL;
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

  if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);
  }  else {
    retval=setMaster(arg);

  }
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

  if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);
  }  else {
    //ret=setStoreInRAM(0);
    // initChipWithProbes(0,0,0, ALLMOD);
    retval=setSynchronization(arg);
  }
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






int read_counter_block(int file_des) {

	int ret=OK;
	int n;
	int startACQ;
	//char *retval=NULL;
	short int CounterVals[NCHAN*NCHIP];

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


int set_ctb_pattern(int file_des){

  int ret=OK;//FAIL;
	int retval=-1;
	int n;
	int mode;
	uint64_t word, retval64, t;
	int addr;
	int level, start, stop, nl;
	uint64_t pat[1024];

	sprintf(mess,"Could not set pattern\n");

 	n = receiveDataOnly(file_des,&mode,sizeof(mode)); 
	printf("pattern mode is %d\n",mode);
	switch (mode) {

	case 0: //sets word
	  n = receiveDataOnly(file_des,&addr,sizeof(addr)); 
	  n = receiveDataOnly(file_des,&word,sizeof(word)); 
	  ret=OK;

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
	  n = receiveDataOnly(file_des,&level,sizeof(level)); 
	  n = receiveDataOnly(file_des,&start,sizeof(start)); 
	  n = receiveDataOnly(file_des,&stop,sizeof(stop)); 
	  n = receiveDataOnly(file_des,&nl,sizeof(nl)); 
	  


	  printf("level %d start %x stop %x nl %d\n",level, start, stop, nl);
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


	/*return ok/fail*/
	return ret;
}


