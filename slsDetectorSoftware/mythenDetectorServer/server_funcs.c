#include "sls_detector_defs.h"
#include "server_funcs.h"
#ifndef PICASSOD
#include "server_defs.h"
#else
#include "picasso_defs.h"
#endif
#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "trimming_funcs.h"
#include "gitInfoMythen.h"

// Global variables

int (*flist[256])(int);



#ifdef MCB_FUNCS
extern const enum detectorType myDetectorType;
#endif
#ifndef MCB_FUNCS
const enum detectorType myDetectorType=MYTHEN;
#endif
extern int nModX;
extern int nModY;
extern int dataBytes;
extern int dynamicRange;
extern int  storeInRAM;

extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;

/* global variables for optimized readout */
extern int *ram_values;
char *dataretval=NULL;
int nframes, iframes, dataret;
char mess[1000]; 





int init_detector( int b) {
#ifndef PICASSOD
  printf("This is a MYTHEN detector with %d chips per module\n", NCHIP);
#else
  printf("This is a PICASSO detector with %d chips per module\n", NCHIP);
#endif
  mapCSP0();
#ifndef VIRTUAL  
  system("bus -a 0xb0000000 -w 0xd0008");
#ifdef VERBOSE
  printf("setting wait states \n");
  system("bus -a 0xb0000000");
#endif
#endif
  testFpga();
  if (b) {
#ifdef MCB_FUNCS
    initDetector();
    setSettings(GET_SETTINGS);
    testRAM();
#endif
    setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
    setMaster(GET_MASTER);
    setSynchronization(GET_SYNCHRONIZATION_MODE);
  }
  strcpy(mess,"dummy message");
  strcpy(lastClientIP,"none");
  strcpy(thisClientIP,"none1");
  lockStatus=0;
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
  flist[F_SET_MASTER]=&set_master;
  flist[F_SET_SYNCHRONIZATION_MODE]=&set_synchronization;
#ifdef VERBOSE
  /*  for (i=0;i<256;i++){
    printf("function %d located at %x\n",i,flist[i]);
    }*/
#endif
  return OK;
}


int  M_nofunc(int file_des){
  
  int retval=FAIL;
  sprintf(mess,"Unrecognized Function\n");
  printf(mess);
  sendDataOnly(file_des,&retval,sizeof(retval));
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
  int64_t retval, rev, dat;
  int ret=OK;
  int imod=-1;
  int n=0;
  int rev1;
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
    n = receiveDataOnly(file_des,&imod,sizeof(imod));
    if (n < 0) {
      sprintf(mess,"Error reading from socket\n");
      retval=FAIL;
    } else {
#ifdef VERBOSE
      printf("of module %d\n", imod);
#endif  
      if (imod>=0 && imod<=getNModBoard()) {
#ifdef MCB_FUNCS
	retval=getModuleNumber(imod);
#endif
	;
      }
      else {
	sprintf(mess,"Module number %d out of range\n",imod);
	ret=FAIL;
      }
    }
    break;
  case MODULE_FIRMWARE_VERSION:
    retval=0x1;
    break;
  case DETECTOR_SERIAL_NUMBER:
    retval=getMcsNumber();
    break;
  case DETECTOR_FIRMWARE_VERSION:
    retval=getMcsVersion();
    break;
  case DETECTOR_SOFTWARE_VERSION:
	retval= SVNREV;
	retval= (retval <<32) | SVNDATE;
/*  sscanf(THIS_REVISION,"$Rev : %x",&rev1);
    rev=((int64_t)rev1);
    dat=THIS_SOFTWARE_VERSION;
    retval=(dat<<32) | rev;
    */
    break;
  default:
    printf("Required unknown id %d \n", arg);
    ret=FAIL;
    retval=FAIL;
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
  default:
    printf("Unknown digital test required %d\n",arg);
    ret=FAIL;
    retval=FAIL;
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

int set_dac(int file_des) {

  dacs_t retval;
  int ret=OK;
  int arg[3];
  enum dacIndex ind;
  int imod;
  int n;
  dacs_t val;
  int idac=0;

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

  if (imod>=getNModBoard())
    ret=FAIL;
  if (imod<0)
    imod=ALLMOD;

#ifdef MCB_FUNCS
  switch (ind) {
  case  TRIMBIT_SIZE:
    idac=VTRIM;
    break;
  case THRESHOLD:
    idac=VTHRESH;
    break;
  case SHAPER1:
    idac=RGSH1;
    break;
  case SHAPER2:
    idac=RGSH2;
    break;
  case CALIBRATION_PULSE:
    idac=VCAL;
    break;
  case PREAMP:
    idac=RGPR;
    break;
    /***************************************************************
add possible potentiometers like in chiptest board!!!!!!!!!!!!!!!

    ****************************************************************/




  default:
    printf("Unknown DAC index %d\n",ind);
    sprintf(mess,"Unknown DAC index %d\n",ind);
    ret=FAIL;
  }
 
  if (ret==OK) {
    if (differentClients==1 && lockStatus==1 && val!=-1) {
      ret=FAIL;
      sprintf(mess,"Detector locked by %s\n",lastClientIP);    
    } else
      retval=initDACbyIndexDACU(idac,val,imod);
  }
#endif

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

  dacs_t retval;
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


  if (imod>=getNModBoard() || imod<0)
    ret=FAIL;

#ifdef MCB_FUNCS
  switch (ind) {
  case  TRIMBIT_SIZE:
    idac=VTRIM;
    break;
  case THRESHOLD:
    idac=VTHRESH;
    break;
  case SHAPER1:
    idac=RGSH1;
    break;
  case SHAPER2:
    idac=RGSH2;
    break;
  case CALIBRATION_PULSE:
    idac=VCAL;
    break;
  case PREAMP:
    idac=RGPR;
    break;
  default:
    printf("Unknown DAC index %d\n",ind);
    ret=FAIL;
    sprintf(mess,"Unknown DAC index %d\n",ind);
  }
 
  if (ret==OK) {
    retval=getDACbyIndexDACU(idac,imod);
  }
 #endif
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

  dacs_t *myDac=malloc(NDAC*sizeof(dacs_t));
  dacs_t *myAdc=malloc(NADC*sizeof(dacs_t));

  int retval, n;
  int ret=OK;
  int dr;//, ow;

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

  setDynamicRange(dr);

  
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
  dacs_t *myDac=malloc(NDAC*sizeof(dacs_t));
  dacs_t *myAdc=malloc(NADC*sizeof(dacs_t));


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

#ifdef MCB_FUNCS
  retval=getThresholdEnergy();
#endif


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
  

#ifdef VERBOSE
  printf("Setting threshold energy of module %d to %d eV with settings %d\n", imod, ethr, isett);
#endif 

  if (differentClients==1 && lockStatus==1) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);  
  } else {
#ifdef MCB_FUNCS
    retval=setThresholdEnergy(ethr);
#endif
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
    retval=setSettings(arg[0]);
#endif
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

  //#ifdef VERBOSE
  printf("********************************* Getting status\n");
  //#endif 

  retval= runState();

  if (retval&0x8000) {
    s=ERROR;
    printf("********* Status error %08x\n",retval); 
  }  else if (retval&0x00000001)
    if (retval&0x00010000)
      s=TRANSMITTING;
    else
      s=RUNNING;
  else if (retval&0x00010000)
    s=RUN_FINISHED;
  else if (retval&0x00000008)
    s=WAITING;
  else
    s=IDLE;



  if (ret!=OK) {
    printf("get status failed\n");
  } //else if (differentClients)
    //ret=FORCE_UPDATE;

  

  //#ifdef VERBOSE
  printf("%d %08x %d\n", ret, retval, s);
  //#endif 


  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n += sendDataOnly(file_des,mess,sizeof(mess));
  } else {
    n += sendDataOnly(file_des,&s,sizeof(s));
  }
  return ret; 



}

int read_frame(int file_des) {
  /*
  int *retval=NULL;
  char *ptr=NULL;
  int ret=OK; 
  int f=0, i;
  */
#ifdef VERBOSE
  int n;
#endif

  if (differentClients==1 && lockStatus==1) {
    dataret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);  
    sendDataOnly(file_des,&dataret,sizeof(dataret));
    sendDataOnly(file_des,mess,sizeof(mess));
    printf("dataret %d\n",dataret);
    return dataret;

  }

 
  if (storeInRAM==0) {
    if ((dataretval=(char*)fifo_read_event())) {
      dataret=OK;
#ifdef VERYVERBOSE
      printf("Sending ptr %x %d\n",dataretval, dataBytes);
#endif 
      sendDataOnly(file_des,&dataret,sizeof(dataret));
      sendDataOnly(file_des,dataretval,dataBytes);
#ifdef VERBOSE
      printf("sent %d bytes\n",dataBytes);   
#endif
      printf("dataret OK\n");
      return OK;
    }  else {
      //might add delay????
      if(getFrames()>-2) {
	dataret=FAIL;
	sprintf(mess,"no data and run stopped: %d frames left\n",getFrames()+2); 
	printf("%s\n",mess);
      } else {
	dataret=FINISHED;
	sprintf(mess,"acquisition successfully finished\n");
	printf("%s\n",mess);
      }
#ifdef VERYVERBOSE
      printf("%d %d %x %s\n",sizeof(mess),strlen(mess), mess,mess);
#endif
      sendDataOnly(file_des,&dataret,sizeof(dataret));
      sendDataOnly(file_des,mess,sizeof(mess));//sizeof(mess));//sizeof(mess));
#ifdef VERYVERBOSE
      printf("message sent\n",mess);
#endif 
      printf("dataret %d\n",dataret);
      return dataret;
    }
  } else {
    nframes=0;
    while(fifo_read_event()) {
      nframes++;
    }
    dataretval=(char*)ram_values;
    dataret=OK;
#ifdef VERBOSE
    printf("sending data of %d frames\n",nframes);
#endif 
    for (iframes=0; iframes<nframes; iframes++) {
      sendDataOnly(file_des,&dataret,sizeof(dataret));
#ifdef VERYVERBOSE
      printf("sending pointer %x of size %d\n",dataretval,dataBytes);
#endif 
      sendDataOnly(file_des,dataretval,dataBytes);
      dataretval+=dataBytes;
    }
    if (getFrames()>-2) {
      dataret=FAIL;
      sprintf(mess,"no data and run stopped: %d frames left\n",getFrames()+2);
      printf("%s\n",mess);
    } else {
      dataret=FINISHED;
      sprintf(mess,"acquisition successfully finished\n");
      printf("%s\n",mess);
      if (differentClients)
	dataret=FORCE_UPDATE;
    }
#ifdef VERBOSE
      printf("Frames left %d\n",getFrames());
#endif 
    sendDataOnly(file_des,&dataret,sizeof(dataret));
    sendDataOnly(file_des,mess,sizeof(mess));
    printf("dataret %d\n",dataret);
    return dataret;
  }
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
	retval=setProbes(tns);
	break;
      case CYCLES_NUMBER: 
	retval=setTrains(tns);
	break;
      default:
	ret=FAIL;
	sprintf(mess,"timer index unknown %d\n",ind);
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
    ret=allocateRAM();
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
    default:
      ret=FAIL;
      sprintf(mess,"timer index unknown %d\n",ind);
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
  } else {
    ret=allocateRAM();
    if (ret!=OK)
      sprintf(mess,"Could not allocate RAM for the dynamic range selected\n");
    else  if (differentClients)
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


  int arg=-1;
  int n;
  ROI roiLimits[MAX_ROIS];
  int ret=OK;
  ROI retval;

  int nm=setNMod(-1), nmax=getNModBoard(), nroi;


  sprintf(mess,"can't set ROI\n");

  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if(arg>0){
    n+=receiveDataOnly(file_des,roiLimits,arg*sizeof(ROI));
  }

  if (arg>1) {
    ret=FAIL;
    sprintf(mess,"can't set more than 1 ROI per detector\n");
    
  } else
    ret=OK;

  if (arg>0) {

    nm=(roiLimits[0].xmax-1)/1280+1;
    
    if (roiLimits[0].xmin>0) {
      roiLimits[0].xmin=0;
      ret=FAIL;
      sprintf(mess,"ROI starts at 0\n");
    }
    
    if (nm>nmax) {
      retval.xmax=setNMod(-1)*1280;
      ret=FAIL;
      sprintf(mess,"ROI max larger than detector size\n");
    }
  
  } else if (arg==0) {
    setNMod(nmax);
  }

  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n = sendDataOnly(file_des,mess,sizeof(mess));
  } else {
    retval.xmin=0;
    retval.xmax=setNMod(-1)*1280;
    retval.ymin=0;
    retval.ymax=0;
    if (setNMod(-1)<nmax)
      nroi=1;
    else
      nroi=0;
    
    n = sendDataOnly(file_des,&nroi,sizeof(nroi));
    if (nroi)
      n = sendDataOnly(file_des,&retval,nroi*sizeof(retval));
  }
  return ret; 

}

int get_roi(int file_des) {
  return set_roi(file_des);

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

    if (val>=0) {
      if (differentClients==1 && lockStatus==1 && val>=0) {
	ret=FAIL;
	sprintf(mess,"Detector locked by %s\n",lastClientIP);
      }  else {
	switch (arg) {
	case CLOCK_DIVIDER:
	  retval=setClockDivider(val);
	  break;
	case WAIT_STATES:
	  retval=setWaitStates(val);
	  break;
	case SET_SIGNAL_LENGTH:
	  retval=setSetLength(val);
	  break;
	case TOT_CLOCK_DIVIDER:
	  retval=setTotClockDivider(val);
	  break;
	case TOT_DUTY_CYCLE:
	  retval=setTotDutyCycle(val);
	  break;
	default:
	  ret=FAIL;
	}
      }
    } else {

      switch (arg) {
      case CLOCK_DIVIDER:
	retval=getClockDivider();
	break;
      case WAIT_STATES:
	retval=getWaitStates();
	break;
      case SET_SIGNAL_LENGTH:
	retval=getSetLength();
	break;
      case TOT_CLOCK_DIVIDER:
	retval=getTotClockDivider();
	break;
      case TOT_DUTY_CYCLE:
	retval=getTotDutyCycle();
	break;
      default:
	ret=FAIL;
      }
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

  enum readOutFlags retval;
  enum readOutFlags arg;
  int n;
  int ret=OK;
  int regret=OK;
  

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
    //ret=setStoreInRAM(0);
    // initChipWithProbes(0,0,0, ALLMOD);
    switch(arg) {
    case  GET_READOUT_FLAGS:
      break;
    case STORE_IN_RAM:
      if (setStoreInRAM(1)==OK)
	ret=OK;
      else 
	ret=FAIL;
      break;
    case TOT_MODE:
      if(setToT(1))
	ret=OK;
      else 
	ret=FAIL;
      break;
    case CONTINOUS_RO:
      if (setContinousReadOut(1))
	ret=OK;
      else 
	ret=FAIL;
      break;
      // case PUMP_PROBE_MODE:
      //set number of probes
      //initChipWithProbes(0,0,2, ALLMOD);
      //break;
    default:
      ret=setStoreInRAM(0);
      regret=setConfigurationRegister(0);
      ret=OK;       
    } 
  }
  retval=NORMAL_READOUT;
  
  if (storeInRAM)
    retval=STORE_IN_RAM;
  //else if (getProbes())
  //  retval=PUMP_PROBE_MODE;
  //else
  if (setToT(-1))
    retval|=TOT_MODE;
  if (setContinousReadOut(-1))
    retval|=CONTINOUS_RO;
  if (ret!=OK) {
    printf("set readout flags failed\n");
    sprintf(mess,"Could not allocate RAM\n");
  } else if (differentClients)
    ret=FORCE_UPDATE;
    
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
  int ret=OK;
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

  if (imod>=getNModBoard())
    ret=FAIL;

  if (imod<0)
    imod=ALLMOD;

  par1=arg[1];
  par2=arg[2];

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
	// par1 is countlim; par2 is nsigma
	ret=trim_with_noise(par1, par2, imod);
	break;
      case BEAM_TRIMMING:
	// par1 is countlim; par2 is nsigma
	ret=trim_with_beam(par1,par2,imod);
	break;
      case IMPROVE_TRIMMING:
	// par1 is maxit; if par2!=0 vthresh will be optimized
	ret=trim_improve(par1, par2,imod);
	break;
      case FIXEDSETTINGS_TRIMMING:
	// par1 is countlim; if par2<0 then trimwithlevel else trim with median 
	ret=trim_fixed_settings(par1,par2,imod);
	break;
	// case OFFLINE_TRIMMING:
	
	//break;
      default:
	printf("Unknown trimming mode\n");
	sprintf(mess,"Unknown trimming mode\n");
	ret=FAIL;
      }
    } 
  }

  
  if (ret<0) {
    sprintf(mess,"can't set execute trimming\n");
    ret=FAIL;
  } else if (ret>0) {
    sprintf(mess,"Could not trim %d channels\n", ret);
    ret=FAIL;
  } else if (differentClients)
    ret=FORCE_UPDATE;
  
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n = sendDataOnly(file_des,mess,sizeof(mess));
  } 
    
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
    if (sd>=0 || sd==-10) {
      ret=OK;
      if (differentClients )
	ret=FORCE_UPDATE;
    } else {
      ret=FAIL;
      sprintf(mess,"Could not bind port %d\n", p_number);
      printf("Could not bind port %d\n", p_number); 

    }

    n = sendDataOnly(file_des,&ret,sizeof(ret));
    if (ret==FAIL) {
      n = sendDataOnly(file_des,mess,sizeof(mess));
    } else {
      n = sendDataOnly(file_des,&p_number,sizeof(p_number));
      if (sd>=0) {
	closeConnection(file_des);
	exitServer(sockfd);
	sockfd=sd;
      }
      
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
  int thr, n;
  // int it;
  int64_t retval, tns=-1;

 
  n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));
  n = sendDataOnly(file_des,&nModX,sizeof(nModX));
  // n = sendDataOnly(file_des,&nModY,sizeof(nModY));
  //sends back max modules instead of nmodulesY!

  thr = getNModBoard();
  sendDataOnly(file_des,&thr,sizeof(thr));

  n = sendDataOnly(file_des,&dynamicRange,sizeof(dynamicRange));
  n = sendDataOnly(file_des,&dataBytes,sizeof(dataBytes));
  t=setSettings(GET_SETTINGS);
  n = sendDataOnly(file_des,&t,sizeof(t));
  thr=getThresholdEnergy();
  n = sendDataOnly(file_des,&thr,sizeof(thr));
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
  retval=setProbes(tns);
  n = sendDataOnly(file_des,&retval,sizeof(int64_t));
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
