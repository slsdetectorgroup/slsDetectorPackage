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


// Global variables

int (*flist[256])(int);



#ifdef MCB_FUNCS
extern const enum detectorType myDetectorType;
#endif
#ifndef MCB_FUNCS
const enum detectorType myDetectorType=GOTTHARD;
#endif


extern int nModX;
extern int nModY;
extern int dataBytes;
extern int dynamicRange;
extern int storeInRAM;


/* global variables for optimized readout */
extern int *ram_values;
char *dataretval=NULL;
int nframes, iframes, dataret;
char mess[1000]; 




int init_detector( int b) {
#ifndef PICASSOD
  printf("This is a GOTTHARD detector with %d chips per module\n", NCHIP);
#else
  printf("This is a PICASSO detector with %d chips per module\n", NCHIP);
#endif
  int res =mapCSP0();
  /*
#ifndef VIRTUAL  
  system("bus -a 0xb0000000 -w 0xd0008");
#ifdef VERBOSE
  printf("setting wait states \n");
  system("bus -a 0xb0000000");
#endif
#endif
  testFpga(); shouldnt this be inside virtual as well?!
  
  */  
  if (res<0) { printf("Could not map memory\n");
    exit(1);  
  }

  //testFpga();
#ifdef MCB_FUNCS
  if (b) {
    initDetector();
    printf("\ninitdetector done! \n");
    setDummyRegister(); 
    setSettings(GET_SETTINGS);

    //testRAM();
  }
  #endif
 

  strcpy(mess,"dummy message");
  return OK;
}


int decode_function() {
  int fnum,n;
  int retval=FAIL;
#ifdef VERBOSE
  printf( "receive data\n");
#endif 
  n = receiveDataOnly(&fnum,sizeof(fnum));
  if (n <= 0) {
    printf("ERROR reading from socket %d", n);
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
  retval=(*flist[fnum])(fnum);
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
#ifdef VERBOSE
  /*  for (i=0;i<256;i++){
    printf("function %d located at %x\n",i,flist[i]);
    }*/
#endif
  return OK;
}


int  M_nofunc(int fnum){
  
  int retval=FAIL;
  sprintf(mess,"Unrecognized Function %d\n",fnum);
  printf(mess);
  sendDataOnly(&retval,sizeof(retval));
  sendDataOnly(mess,sizeof(mess));
  return GOODBYE;
}


int exit_server(int fnum) {
  int retval=FAIL;
  sendDataOnly(&retval,sizeof(retval));
  printf("closing server.");
  sprintf(mess,"closing server");
  sendDataOnly(mess,sizeof(mess));
  return GOODBYE;
}

int exec_command(int fnum) {
  char cmd[MAX_STR_LENGTH];
  char answer[MAX_STR_LENGTH];
  int retval=OK;
  int sysret=0;
  int n=0;

 /* receive arguments */
  n = receiveDataOnly(cmd,MAX_STR_LENGTH);
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    retval=FAIL;
  }

  /* execute action if the arguments correctly arrived*/
  if (retval==OK) {
#ifdef VERBOSE
    printf("executing command %s\n", cmd);
#endif
    sysret=system(cmd);
    //should be replaced by popen
    if (sysret==0)
      sprintf(answer,"Succeeded\n");
    else {
      sprintf(answer,"Failed\n");
      retval=FAIL;
    }
  } else {
    sprintf(answer,"Could not receive the command\n");
  }
  
  /* send answer */
  n = sendDataOnly(answer,MAX_STR_LENGTH);
  if (n < 0) {
    sprintf(mess,"Error writing to socket");
    retval=FAIL;
  }


  /*return ok/fail*/
  return retval; 
 
}



int get_detector_type(int fnum) {
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
  n += sendDataOnly(&retval,sizeof(retval));
  if (retval==OK) {
    /* send return argument */
    n += sendDataOnly(&ret,sizeof(ret));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }
  /*return ok/fail*/
  return retval; 
  

}


int set_number_of_modules(int fnum) {
  int n;
  int arg[2], ret=0; 
  int retval=OK;
  int dim, nm;
  
  sprintf(mess,"Can't set number of modules\n");

  /* receive arguments */
  n = receiveDataOnly(&arg,sizeof(arg));
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
      if (dim!=X) {
	retval=FAIL;
	sprintf(mess,"Can't change module number in dimension %d\n",dim);
      } else  {
	ret=setNMod(nm);
	if (nModX==nm || nm==GET_FLAG)
	  retval=OK;
	else
	  retval=FAIL;
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
  n = sendDataOnly(&retval,sizeof(retval));
  if (retval==OK) {
    /* send return argument */
    n += sendDataOnly(&ret,sizeof(ret));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }
  /*return ok/fail*/
  return retval; 
  
}


int get_max_number_of_modules(int fnum) {
  int n;
  int ret; 
  int retval=OK;
  enum dimension arg;
  
  sprintf(mess,"Can't get max number of modules\n");
  /* receive arguments */
  n = receiveDataOnly(&arg,sizeof(arg));
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




  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&retval,sizeof(retval));
  if (retval==OK) {
    /* send return argument */
    n += sendDataOnly(&ret,sizeof(ret));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }



  /*return ok/fail*/
  return retval; 
}


//index 0 is in gate
//index 1 is in trigger
//index 2 is out gate
//index 3 is out trigger

int set_external_signal_flag(int fnum) {
  int n;
  int arg[2]; 
  int ret=OK;
  int signalindex;
  enum externalSignalFlag flag, retval;
  
  sprintf(mess,"Can't set external signal flag\n");

  /* receive arguments */
  n = receiveDataOnly(&arg,sizeof(arg));
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
      retval=setExtSignal(signalindex,flag);

    }

#ifdef VERBOSE
    printf("Setting external signal %d to flag %d\n",signalindex,flag );
    printf("Set to flag %d\n",retval);
#endif

  } else {
    ret=FAIL;
  }


  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }


  /*return ok/fail*/
  return ret; 
  
}


int set_external_communication_mode(int fnum) {
  int n;
  enum externalCommunicationMode arg, ret;
  int retval=OK;
  
  sprintf(mess,"Can't set external communication mode\n");


  /* receive arguments */
  n = receiveDataOnly(&arg,sizeof(arg));
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
  ret=AUTO;
  if (retval==OK) {
  /* execute action */
    switch(arg) {
    default:
      sprintf(mess,"The meaning of single signals should be set\n");
      retval=FAIL;
    }


#ifdef VERBOSE
      printf("Setting external communication mode to %d\n", arg);
#endif
  } else
    ret=FAIL;

  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&retval,sizeof(retval));
  if (retval==OK) {
    /* send return argument */
    n += sendDataOnly(&ret,sizeof(ret));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  /*return ok/fail*/
  return retval; 
  

}



int get_id(int fnum) {
  // sends back 64 bits!
  int64_t retval;
  int ret=OK;
  int imod=-1;
  int n=0;
  enum idMode arg;
  
  sprintf(mess,"Can't return id\n");

  /* receive arguments */
  n = receiveDataOnly(&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }

#ifdef VERBOSE
      printf("Getting id %d\n", arg);
#endif  

  switch (arg) {
  case  MODULE_SERIAL_NUMBER:
    n = receiveDataOnly(&imod,sizeof(imod));
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
    retval=THIS_SOFTWARE_VERSION;
    break;
  default:
    printf("Required unknown id %d \n", arg);
    ret=FAIL;
    retval=FAIL;
  }
 
#ifdef VERBOSE
      printf("Id is %llx\n", retval);
#endif  
 
  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  /*return ok/fail*/
  return ret; 

}

int digital_test(int fnum) {

  int retval;
  int ret=OK;
  int imod=-1;
  int n=0;
  int ibit=0;
  int ow;
  enum digitalTestMode arg; 
  
  sprintf(mess,"Can't send digital test\n");

  n = receiveDataOnly(&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }

#ifdef VERBOSE
  printf("Digital test mode %d\n",arg );
#endif  

  switch (arg) {
  case  CHIP_TEST:
    n = receiveDataOnly(&imod,sizeof(imod));
    if (n < 0) {
      sprintf(mess,"Error reading from socket\n");
      retval=FAIL;
    }
#ifdef VERBOSE
      printf("of module %d\n", imod);
#endif  
      retval=0;
#ifdef MCB_FUNCS
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
 
  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  /*return ok/fail*/
  return ret; 

}

int write_register(int fnum) {

  int retval;
  int ret=OK;
  int arg[2]; 
  int addr, val;
  int n;

  
  sprintf(mess,"Can't write to register\n");

  n = receiveDataOnly(arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  addr=arg[0];
  val=arg[1];

#ifdef VERBOSE
  printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif  

  retval=bus_w(addr,val);



#ifdef VERBOSE
  printf("Data set to 0x%x\n",  retval);
#endif  
  if (retval==val)
    ret=OK;
  else {
    ret=FAIL;
    sprintf(mess,"Writing to register 0x%x failed: wrote 0x%x but read 0x%x\n", addr, val, retval);
  }

  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  /*return ok/fail*/
  return ret; 

}

int read_register(int fnum) {

  int retval;
  int ret=OK;
  int arg; 
  int addr;
  int n;

  
  sprintf(mess,"Can't read register\n");

  n = receiveDataOnly(&arg,sizeof(arg));
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
  }


  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  /*return ok/fail*/
  return ret; 

}

int set_dac(int fnum) {
  //everything in here does for all mods
  float retval;
  int ret=OK;
  int arg[2];
  enum dacIndex ind;
  int imod;
  int n;
  float val;
  int idac=0;
  int ireg=-1;

  sprintf(mess,"Can't set DAC/TEMP/HV\n");


  n = receiveDataOnly(arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  ind=arg[0];
  imod=arg[1];

  n = receiveDataOnly(&val,sizeof(val));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }

#ifdef VERBOSE
  printf("Setting DAC/TEMP/HV %d of module %d to %f V\n", ind, imod, val);
#endif 

  if (imod>=getNModBoard())
    ret=FAIL;
  if (imod<0)
    imod=ALLMOD;

#ifdef MCB_FUNCS
  switch (ind) {
  case G_VREF_DS :
    idac=VREF_DS;
    break;
  case G_VCASCN_PB:
    idac=VCASCN_PB;
    break;
  case G_VCASCP_PB:
    idac=VCASCP_PB;
    break;
  case G_VOUT_CM:
    idac=VOUT_CM;
    break;
  case G_VCASC_OUT:
    idac=VCASC_OUT;
    break;
  case G_VIN_CM:
    idac=VIN_CM;
    break;
  case G_VREF_COMP:
    idac=VREF_COMP;
    break;
  case G_IB_TESTC:
    idac=IB_TESTC;
    break;
  case TEMPERATURE_FPGA:
    ireg=TEMP_FPGA;
    break;
  case TEMPERATURE_ADC:
    ireg=TEMP_ADC;
    break;
  case HV_POT:
    ireg=HIGH_VOLTAGE;
    break;
  case G_CONF_GAIN:
    ireg=CONFGAIN;
    break;
  default:
    printf("Unknown DAC/TEMP/HV index %d\n",ind);
    sprintf(mess,"Unknown DAC/TEMP/HV index %d\n",ind);
    ret=FAIL;
  }
 
  if (ret==OK) {
    //dac
    if(ireg==-1)
      retval=initDACbyIndexDACU(idac,val,imod);
    else
      {//Conf gain
	if (ireg==CONFGAIN)
	  retval=initConfGainByModule(val,imod);
	//HV or conf gain
	else if(ireg==HIGH_VOLTAGE)
	  retval=initHighVoltageByModule(val,imod);
	//Temp
	else
	  retval=getTemperatureByModule(ireg,imod);
      }
  }
#endif
#ifdef VERBOSE
  printf("DAC/TEMP/HV set to %f V\n",  retval);
#endif  
  ret=FAIL;
  if((ireg==HIGH_VOLTAGE)||(ireg==CONFGAIN)){
    if(retval==-2)
      strcpy(mess,"Invalid Voltage.Valid values are 0,90,110,120,150,180,200");
    else if(retval==-3)
      strcpy(mess,"Weird value read back\n");
    else
      ret=OK;
  }
  else if (retval==val || val==-1)
    ret=OK;
   
  if(ret==FAIL)
    printf("Setting dac/hv %d of module %d: wrote %f but read %f\n", ind, imod, val, retval);
    
  /* send answer */
/* send OK/failed */
n = sendDataOnly(&ret,sizeof(ret));
if (ret==OK) {
  /* send return argument */
  n += sendDataOnly(&retval,sizeof(retval));
 } else {
  n += sendDataOnly(mess,sizeof(mess));
 }
/*return ok/fail*/
return ret; 
}



int get_adc(int fnum) {

  float retval;
  int ret=OK;
  int arg[2];
  enum dacIndex ind;
  int imod;
  int n;
  int idac=0;
  
  sprintf(mess,"Can't read ADC\n");


  n = receiveDataOnly(arg,sizeof(arg));
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
 case G_VREF_DS :
    idac=VREF_DS;
    break;
  case G_VCASCN_PB:
    idac=VCASCN_PB;
    break;
  case G_VCASCP_PB:
    idac=VCASCP_PB;
    break;
  case G_VOUT_CM:
    idac=VOUT_CM;
    break;
  case G_VCASC_OUT:
    idac=VCASC_OUT;
    break;
  case G_VIN_CM:
    idac=VIN_CM;
    break;
  case G_VREF_COMP:
    idac=VREF_COMP;
    break;
  case G_IB_TESTC:
    idac=IB_TESTC;
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


  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  /*return ok/fail*/
  return ret; 

}

int set_channel(int fnum) {
  int ret=OK;
  sls_detector_channel myChan;
  int retval;
  int n;
  

  sprintf(mess,"Can't set channel\n");

#ifdef VERBOSE
  printf("Setting channel\n");
#endif
  ret=receiveChannel(&myChan);
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
#ifdef MCB_FUNCS
    retval=initChannelbyNumber(myChan);
#endif
  }
  /* Maybe this is done inside the initialization funcs */
  //copyChannel(detectorChans[myChan.module][myChan.chip]+(myChan.chan), &myChan);
  


  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }


  /*return ok/fail*/
  return ret; 
  
}




int get_channel(int fnum) {

  int ret=OK;
  sls_detector_channel retval;

  int arg[3];
  int ichan, ichip, imod;
  int n;
  
  sprintf(mess,"Can't get channel\n");



  n = receiveDataOnly(arg,sizeof(arg));
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
  } 

#ifdef VERBOSE
  printf("Returning channel %d %d %d, 0x%llx\n", ichan, ichip, imod, (retval.reg));
#endif 

  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    ret=sendChannel(&retval);
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }


  
  /*return ok/fail*/
  return ret; 


}


int set_chip(int fnum) {

  sls_detector_chip myChip;
  int ch[NCHAN];
  int n, retval;
  int ret=OK;
  

  myChip.nchan=NCHAN;
  myChip.chanregs=ch;





#ifdef VERBOSE
  printf("Setting chip\n");
#endif
  ret=receiveChip(&myChip);
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
#ifdef MCB_FUNCS
  retval=initChipbyNumber(myChip);
#endif
  /* Maybe this is done inside the initialization funcs */
  //copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }


  return ret;
}

int get_chip(int fnum) {

  
  int ret=OK;
  sls_detector_chip retval;
  int arg[2];
  int  ichip, imod;
  int n;
  


  n = receiveDataOnly(arg,sizeof(arg));
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
  } 

#ifdef VERBOSE
  printf("Returning chip %d %d\n",  ichip, imod);
#endif 


  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */  
    ret=sendChip(&retval);
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }


  
  /*return ok/fail*/
  return ret; 


}
int set_module(int fnum) {
  sls_detector_module myModule;
  int *myChip=malloc(NCHIP*sizeof(int));
  int *myChan=malloc(NCHIP*NCHAN*sizeof(int));
  float *myDac=malloc(NDAC*sizeof(int));
  float *myAdc=malloc(NADC*sizeof(int));
  int retval, n;
  int ret=OK;
  int dr, ow;

  //  dr=setDynamicRange(-1);  commented out by dhanya

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
  ret=receiveModule(&myModule);
 

  if (ret>=0)
    ret=OK;
  else
    ret=FAIL;


#ifdef VERBOSE
  printf("module number is %d,register is %d, nchan %d, nchip %d, ndac %d, nadc %d, gain %f, offset %f\n",myModule.module, myModule.reg, myModule.nchan, myModule.nchip, myModule.ndac,  myModule.nadc, myModule.gain,myModule.offset);
#endif
  
  if (ret==OK) {
    // if (myModule.module>=getNModBoard()) {//commented out by dhanya
       if (myModule.module>=1) {
      ret=FAIL;
      printf("Module number is too large %d\n",myModule.module);
    }
    if (myModule.module<0) 
      myModule.module=ALLMOD;
  }
    
  if (ret==OK) {
#ifdef MCB_FUNCS
    retval=initModulebyNumber(myModule);
#endif
  }

  /* Maybe this is done inside the initialization funcs */
  //copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */
    n += sendDataOnly(&retval,sizeof(retval));
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }
  free(myChip);
  free(myChan);
  free(myDac);
  free(myAdc);

  //  setDynamicRange(dr);commentedout by dhanya

  
  return ret;
}




int get_module(int fnum) {

  
  int ret=OK;


  int arg;
  int   imod;
  int n;

  

  sls_detector_module myModule;
  int *myChip=malloc(NCHIP*sizeof(int));
  int *myChan=malloc(NCHIP*NCHAN*sizeof(int));
  float *myDac=malloc(NDAC*sizeof(int));
  float *myAdc=malloc(NADC*sizeof(int));


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
  




  n = receiveDataOnly(&arg,sizeof(arg));
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
  /* send answer */
  /* send OK/failed */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret==OK) {
    /* send return argument */  
    ret=sendModule(&myModule);
  } else {
    n += sendDataOnly(mess,sizeof(mess));
  }

  

  free(myChip);
  free(myChan);
  free(myDac);
  free(myAdc);








  /*return ok/fail*/
  return ret; 

}
int get_threshold_energy(int fnum) { 
  int retval;
  int ret=OK;
  int n;
  int  imod;


  n = receiveDataOnly(&imod,sizeof(imod));
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


 
  /* send answer */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  } else
    n += sendDataOnly(&retval,sizeof(retval));
  
  
  /* Maybe this is done inside the initialization funcs */
  //detectorDacs[imod][ind]=val;
  /*return ok/fail*/
  return ret; 

}
 
int set_threshold_energy(int fnum) { 
  int retval;
  int ret=OK;
  int arg[3];
  int n;
  int ethr, imod;
  enum detectorSettings isett;


  n = receiveDataOnly(&arg,sizeof(arg));
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

#ifdef MCB_FUNCS
  retval=setThresholdEnergy(ethr);
#endif

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

 
  /* send answer */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  } else
    n += sendDataOnly(&retval,sizeof(retval));
  
  
  /* Maybe this is done inside the initialization funcs */
  //detectorDacs[imod][ind]=val;
  /*return ok/fail*/
  return ret; 

}
 
int set_settings(int fnum) {

  int retval;
  int ret=OK;
  int arg[2];
  int n;
  int  imod;
  enum detectorSettings isett;


  n = receiveDataOnly(&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  imod=arg[1];
  isett=arg[0];
  

#ifdef VERBOSE
  printf("Changing settings of module %d to %d\n", imod,  isett);
#endif 

#ifdef MCB_FUNCS
  retval=setSettings(arg[0]);
#endif

#ifdef VERBOSE
  printf("Settings changed to %d\n",  isett);
#endif  
  if (retval==isett || isett<0)
    ret=OK;
  else {
    ret=FAIL;
    printf("Changing settings of module %d: wrote %d but read %d\n", imod, isett, retval);
  }

  /* send answer */
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  } else
    n += sendDataOnly(&retval,sizeof(retval));
    
 
  
  return ret; 


}

int start_acquisition(int fnum) {

  int ret=OK;
  int n;
  

  sprintf(mess,"can't start acquisition\n");

#ifdef VERBOSE
  printf("Starting acquisition\n");
#endif 

  ret=startStateMachine();
  if (ret!=OK)
    sprintf(mess,"Start acquisition failed\n");

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  }
  return ret; 

}

int stop_acquisition(int fnum) {

  int ret=OK;
  int n;
  

  sprintf(mess,"can't stop acquisition\n");

#ifdef VERBOSE
  printf("Stopping acquisition\n");
#endif 

  ret=stopStateMachine();
  if (ret!=OK)
    sprintf(mess,"Stop acquisition failed\n");

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  }
  return ret; 


}

int start_readout(int fnum) {


  int ret=OK;
  int n;
  

  sprintf(mess,"can't start readout\n");

#ifdef VERBOSE
  printf("Starting readout\n");
#endif 
  ret=startReadOut();
  if (ret!=OK)
    sprintf(mess,"Start readout failed\n");

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  }
  return ret; 



}

int get_run_status(int fnum) {  

  int ret=OK;
  int n;
  
  int retval;
  enum runStatus s;
  sprintf(mess,"getting run status\n");

#ifdef VERBOSE
  printf("Getting status\n");
#endif 

  retval= runState();

  printf("\n\nSTATUS=%x\n",retval);

  if (retval&0x8000)
    s=ERROR;
  else if (retval&0x00000001)
    //    if (retval&0x00010000)
    if (retval&0x00000002)
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
  }

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  } else {
    n += sendDataOnly(&s,sizeof(s));
  }
  return ret; 



}

int read_frame(int fnum) {
  /*
  int *retval=NULL;
  char *ptr=NULL;
  int ret=OK; 
  int f=0, i;
  */
#ifdef VERBOSE
  int n;
#endif

  if (storeInRAM==0) {
    if ((dataretval=(char*)fifo_read_event())) {
      dataret=OK;
#ifdef VERYVERBOSE
      printf("Sending ptr %x %d\n",dataretval, dataBytes);
#endif 
      sendDataOnly(&dataret,sizeof(dataret));
#ifdef VERBOSE
      n=sendDataOnly(dataretval,dataBytes);
      printf("sent %d bytes\n",n);
#else
      
      sendDataOnly(dataretval,dataBytes);
#endif
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
      sendDataOnly(&dataret,sizeof(dataret));
      sendDataOnly(mess,sizeof(mess));//sizeof(mess));//sizeof(mess));
#ifdef VERYVERBOSE
      printf("message sent\n",mess);
#endif 
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
      sendDataOnly(&dataret,sizeof(dataret));
#ifdef VERYVERBOSE
      printf("sending pointer %x of size %d\n",dataretval,dataBytes);
#endif 
      sendDataOnly(dataretval,dataBytes);
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
    }
#ifdef VERBOSE
      printf("Frames left %d\n",getFrames());
#endif 
    sendDataOnly(&dataret,sizeof(dataret));
    sendDataOnly(mess,sizeof(mess));
    return dataret;
  }
  
  return dataret; 
}








int read_all(int fnum) {

 
  while(read_frame(0)==OK) {
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

int start_and_read_all(int fnum) {
  //  int ret=OK;
#ifdef VERBOSE
  printf("Starting and reading all frames\n");
#endif 

  startStateMachine();
  /*  ret=startStateMachine();  
      if (ret!=OK) {
      sprintf(mess,"could not start state machine\n");
    sendDataOnly(&ret,sizeof(ret));
    sendDataOnly(mess,sizeof(mess));
    
    #ifdef VERBOSE
    printf("could not start state machine\n");
#endif
} else {*/
  read_all(1);
#ifdef VERBOSE
  printf("Frames finished\n");
#endif   
  //}


  return OK; 


}

int set_timer(int fnum) {
  enum timerIndex ind;
  int64_t tns;
  int n;
  int64_t retval;
  int ret=OK;
  

  sprintf(mess,"can't set timer\n");
  
  n = receiveDataOnly(&ind,sizeof(ind));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  
  n = receiveDataOnly(&tns,sizeof(tns));
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
  if (ret!=OK) {
    printf(mess);
  }
  //if (tns>=0 && retval!=tns && (retval+1)!=tns) {
    //  printf("wrote %lld, read %lld\n",tns,retval);
  // ret=FAIL;
  // }
  if (ret!=OK) {
    printf(mess);
    printf("set timer failed\n");
    sprintf(mess, "set timer %d failed\n", ind);
  } else if (ind==FRAME_NUMBER) {
    ret=allocateRAM();
    if (ret!=OK) 
      sprintf(mess, "could not allocate RAM for %lld frames\n", tns);
  }

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
#ifdef VERBOSE
  printf("returning error\n");
#endif 

    n = sendDataOnly(mess,sizeof(mess));
  } else {
#ifdef VERBOSE
  printf("returning ok %d\n",sizeof(retval));
#endif 

    n = sendDataOnly(&retval,sizeof(retval));
  }

  return ret; 

}








int get_time_left(int fnum) {

  enum timerIndex ind;
  int n;
  int64_t retval;
  int ret=OK;
  
  sprintf(mess,"can't get timer\n");
  n = receiveDataOnly(&ind,sizeof(ind));
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
    default:
      ret=FAIL;
      sprintf(mess,"timer index unknown %d\n",ind);
    }
  }


  if (ret!=OK) {
    printf("get time left failed\n");
  }

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n += sendDataOnly(mess,sizeof(mess));
  } else {
    n = sendDataOnly(&retval,sizeof(retval));
  }

  return ret; 


}

int set_dynamic_range(int fnum) {


 
  int dr, ow;
  int n;
  int retval;
  int ret=OK;
  

  sprintf(mess,"can't set dynamic range\n");
  

  n = receiveDataOnly(&dr,sizeof(dr));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  
  if (dr>0) {
    /*
#ifdef MCB_FUNCS
    setCSregister(ALLMOD); 
    switch(dr) {
    case 1:
      ow=5;
      break;
    case 4:
      ow=4;
      break;
    case 8:
      ow=3;
      break;
    case 16:
      ow=2;
      break;
    default:
      //   ow=0;
      ow=1;
      break;
    }


    initChip(0, ow,ALLMOD);
#endif
    */
#ifdef VERBOSE
  printf("setting dynamic range to %d\n",dr);
#endif 
  }
  retval=setDynamicRange(dr);


  if (dr>=0 && retval!=dr)
    ret=FAIL;
  if (ret!=OK) {
    sprintf(mess,"set dynamic range failed\n");
  } else {
    ret=allocateRAM();
    if (ret!=OK)
      sprintf(mess,"Could not allocate RAM for the dynamic range selected\n");
  }

  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n = sendDataOnly(mess,sizeof(mess));
  } else {
    n = sendDataOnly(&retval,sizeof(retval));
  }
  return ret; 
}

int set_roi(int fnum) {

  return FAIL;

}

int get_roi(int fnum) {


  return FAIL;
}

int set_speed(int fnum) {

  enum speedVariable arg;
  int val, n;
  int ret=OK;
  int retval;
  
  sprintf(mess,"can't set speed variable\n");
  

  n = receiveDataOnly(&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
   n = receiveDataOnly(&val,sizeof(val));
   if (n < 0) {
     sprintf(mess,"Error reading from socket\n");
     ret=FAIL;
   }
  
#ifdef VERBOSE
   printf("setting speed variable %d  to %d\n",arg,val);
#endif 
  if (ret==OK) {
    switch (arg) {
    case CLOCK_DIVIDER:
      if (val>=0)
	retval=setClockDivider(val);
      else
	retval=getClockDivider();
      break;
    case WAIT_STATES:
      if (val>=0)
	retval=setWaitStates(val);
      else
	retval=getWaitStates();
      break;
    case SET_SIGNAL_LENGTH:
      if (val>=0)
	retval=setSetLength(val);
      else
	retval=getSetLength();
      break;
    case TOT_CLOCK_DIVIDER:
      if (val>=0)
	retval=setTotClockDivider(val);
      else
	retval=getTotClockDivider();
      break;
    case TOT_DUTY_CYCLE:
      if (val>=0)
	retval=setTotDutyCycle(val);
      else
	retval=getTotDutyCycle();
      break;
    default:
      ret=FAIL;
    }
  }
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n = sendDataOnly(mess,sizeof(mess));
  } else {
    n = sendDataOnly(&retval,sizeof(retval));
  }
  return ret; 
}



int set_readout_flags(int fnum) {

  enum readOutFlags retval;
  enum readOutFlags arg;
  int n;
  int ret=OK;
  int regret=OK;
  

  sprintf(mess,"can't set readout flags\n");
  

  n = receiveDataOnly(&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
  

#ifdef VERBOSE
  printf("setting readout flags  to %d\n",arg);
#endif 

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
  } 
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n = sendDataOnly(mess,sizeof(mess));
  } else {
    n = sendDataOnly(&retval,sizeof(retval));
  }
  return ret; 
}





int execute_trimming(int fnum) {
 
  int arg[3];
  int n;
  int ret=OK;
  int imod, par1,par2;
  enum trimMode mode;
  
  sprintf(mess,"can't set execute trimming\n");
  
  n = receiveDataOnly(&mode,sizeof(mode));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    printf("Error reading from socket (mode)\n");
    ret=FAIL;
  }
  
  n = receiveDataOnly(arg,sizeof(arg));
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


  if (ret!=OK) {
    printf("trimming failed\n");
  } 
  n = sendDataOnly(&ret,sizeof(ret));
  if (ret!=OK) {
    n = sendDataOnly(mess,sizeof(mess));
  } 
    
  return ret; 
}
