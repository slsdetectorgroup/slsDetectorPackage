#include "multiSlsDetector.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>




int multiSlsDetector::freeSharedMemory(int i) {
  int nd=nDetectors;
  for (int id=0; id<nd; id++) {
    detectors[id]->freeSharedMemory();
    delete detectors[id];
    detectors[id]=NULL;
    nDetectors--;
  }
  return OK;
}


multiSlsDetector::multiSlsDetector(detectorType type, int ndet, int id): nDetectors(0)
{
  for (int i=0; i<ndet; i++) {
    addSlsDetector(type,id+i);
  }
  for (int i=ndet; i<MAXDET; i++)
    detectors[i]=NULL;
}

multiSlsDetector::~multiSlsDetector() {
  removeSlsDetector();
}

int multiSlsDetector::addSlsDetector(detectorType type, int id) {

    detectors[nDetectors]=new slsDetector(type,id);
    if (detectors[nDetectors])
      nDetectors++;
    return nDetectors;
}



int multiSlsDetector::removeSlsDetector(int i) {
  int imin=0, imax=nDetectors;
  if (i>=0) {
    imin=i;
    imax=i;
  }
  for (int j=imin; j<imax; j++) {
    if (detectors[j]) {
      delete detectors[j];
      detectors[j]=NULL;
      nDetectors--;
    }
  }
  return nDetectors;
}



int multiSlsDetector::setOnline(int off, int i) {
  int imin=0, imax=nDetectors;
  int ret=GET_ONLINE_FLAG;
  int err=0, ans;
  
  if (nDetectors<1)
    return GET_ONLINE_FLAG;
  if (i>=0) {
    imin=i;
    imax=i;
  }

  for (int j=imin; j<imax; j++) {
    if (detectors[j]) {
      ans=detectors[j]->setOnline(off);
      if (ret==GET_ONLINE_FLAG && err==0)
	ret=ans;
      if (ret!=ans) {
	err=1;
	ret=GET_ONLINE_FLAG;
      }
    }
  }
  return ret;
};


int multiSlsDetector::exists(int id) {

  for (int i=0; i<nDetectors; i++) {
    if (detectors[i]) {
      if ((detectors[i]->getId())==id)
	return 1;
    }
  }
  return 0;

}



















  /* 
     configure the socket communication and check that the server exists 
     enum communicationProtocol{
     TCP,
     UDP
     }{};

  */

int multiSlsDetector::setTCPSocket(int i, string const name, int const control_port, int const stop_port,  int const data_port){

  if (i<0)
    return FAIL;
  if (detectors[i])
    return detectors[i]->setTCPSocket(name, control_port, stop_port, data_port);
  else
    return FAIL;
};




char* multiSlsDetector::getHostname(int i) {
  if (i<0)
    return FAIL;
  if (detectors[i])
    return detectors[i]->getHostname();
  else
    return NULL;
}

int multiSlsDetector::getControlPort(int i) {


  int imin=0, imax=nDetectors;
  int ret=-1, err=0;
 if (i>=0) {
    imin=i;
    imax=i;
  }
  for (int j=imin; j<imax; j++) {
    if (detectors[j]) {
      if (ret==-1 && err==0)
	ret=detectors[j]->getControlPort();
      else if (detectors[j]->getControlPort()!=ret) {
	ret=-1;
	err=1;
      }
    }  else {
      ret=-1;
      err=1;
    }
  }
  return ret;
}

int multiSlsDetector::getDataPort(int i) {




  int imin=0, imax=nDetectors;
  int ret=-1, err=0;
 if (i>=0) {
    imin=i;
    imax=i;
  }
  for (int j=imin; j<imax; j++) {
    if (detectors[j]) {
      if (ret==-1 && err==0)
	ret=detectors[j]->getDataPort();
      else if (detectors[j]->getDataPort()!=ret) {
	ret=-1;
	err=1;
      }
    }  else {
      ret=-1;
      err=1;
    }
  }
  return ret;




}


int multiSlsDetector::getStopPort(int i) {

  int imin=0, imax=nDetectors;
  int ret=-1, err=0;
 if (i>=0) {
    imin=i;
    imax=i;
  }
  for (int j=imin; j<imax; j++) {
    if (detectors[j]) {
      if (ret==-1 && err==0)
	ret=detectors[j]->getStopPort();
      else if (detectors[j]->getStopPort()!=ret) {
	ret=-1;
	err=1;
      }
    }  else {
      ret=-1;
      err=1;
    }
  }
  return ret;

}


  /* I/O */

/* generates file name without extension*/

string multiSlsDetector::createFileName() {
  
  string ret=string("error"), ans;
  int err=0;

  for (int i=0; i<ndetectors; i++) {
    if (detectors[i])
      ans=detectors[i]->createFileName();
    else {
      ans=string("error");
      err=1;
    }
    if (ret==string("error") && err==0)
      ret=ans;
    else if (ans!=ret) {
      ret=string("error");
      err=1;
    } 
  }
  return ret;

}




























  /* Communication to server */

  // General purpose functions

  /* 
     executes a system command on the server 
     e.g. mount an nfs disk, reboot and returns answer etc.
  */
int multiSlsDetector::execCommand(string cmd, string answer){

  char arg[MAX_STR_LENGTH], retval[MAX_STR_LENGTH];
  int fnum=F_EXEC_COMMAND;
  
  int ret=FAIL;

  strcpy(arg,cmd.c_str());

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Sending command " << arg << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if (controlSocket->Connect()>=0) {
	if (controlSocket->SendDataOnly(&fnum,sizeof(fnum))>=0) {
	  if (controlSocket->SendDataOnly(arg,MAX_STR_LENGTH)>=0) {
	    if (controlSocket->ReceiveDataOnly(retval,MAX_STR_LENGTH)>=0) {
	      ret=OK;
	      answer=retval;
	    }
	  }
	}
	controlSocket->Disconnect();
      }
    }
#ifdef VERBOSE
    std::cout<< "Detector answer is " << answer << std::endl; 
#endif
  }
  return ret;
};

// Detector configuration functions

  /* 
     the detector knows what type of detector it is 

     enum detectorType{
     GET_DETECTOR_TYPE,
     GENERIC,
     MYTHEN,
     PILATUS,
     EIGER,
     GOTTHARD,
     AGIPD
     };

  */
int multiSlsDetector::setDetectorType(detectorType const type){
  
  int arg, retval=FAIL;
  int fnum=F_GET_DETECTOR_TYPE;
  arg=int(type);
  detectorType retType=type;
  char mess[100];
  strcpy(mess,"dummy");


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting detector type to " << arg << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	if (retval==OK)
	  controlSocket->ReceiveDataOnly(&retType,sizeof(retType));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    if (type==GET_DETECTOR_TYPE)
      retType=thisDetector->myDetectorType;
    else {
      retType=type;
      thisDetector->myDetectorType=type;
   }
    retval=OK;
  }
#ifdef VERBOSE
  std::cout<< "Detector type set to " << retType << std::endl; 
#endif
  if (retval==FAIL) {
    std::cout<< "Set detector type failed " << std::endl;
    retType=GENERIC;
  }
  else
    thisDetector->myDetectorType=retType;


  return retType;
};

int multiSlsDetector::setDetectorType(string const type){
  detectorType dtype=GENERIC;
  if (type=="Mythen")
    dtype=MYTHEN;
  else if  (type=="Pilatus")
      dtype=PILATUS;
  else if  (type=="Eiger")
    dtype=EIGER;
  else if  (type=="Gotthard")
    dtype=GOTTHARD;
  else if  (type=="Agipd")
    dtype=AGIPD;
  return setDetectorType(dtype);
};

void multiSlsDetector::getDetectorType(char *type){

  switch (thisDetector->myDetectorType) {
  case MYTHEN:
    strcpy(type,"Mythen");
    break;
  case PILATUS:
    strcpy(type,"Pilatus");
    break;
  case EIGER:
    strcpy(type,"Eiger");
    break;
  case GOTTHARD:
    strcpy(type,"Gotthard");
    break;
  case AGIPD:
    strcpy(type,"Agipd");
    break;
  default:
    strcpy(type,"Unknown");
    break;
  }
};




  /* needed to set/get the size of the detector */
// if n=GET_FLAG returns the number of installed modules,
int multiSlsDetector::setNumberOfModules(int n, dimension d){
  
  int arg[2], retval;
  int fnum=F_SET_NUMBER_OF_MODULES;
  int ret=FAIL;
  char mess[100]; 


  arg[0]=d;
  arg[1]=n;


  if (d<X || d>Y) {
    std::cout<< "Set number of modules in wrong dimension " << d << std::endl;
    return ret;
  }


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting number of modules of dimension "<< d <<  " to " << n << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Deterctor returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=OK;
    if (n==GET_FLAG)
      ;
    else {
      if (n<=0 || n>thisDetector->nModMax[d]) {
	ret=FAIL;
      } else {
	thisDetector->nMod[d]=n;
      }
    }
    retval=thisDetector->nMod[d];
  }
#ifdef VERBOSE
    std::cout<< "Number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
    if (ret==FAIL) {
      std::cout<< "Set number of modules failed " << std::endl;
    }  else {
      thisDetector->nMod[d]=retval;
      thisDetector->nMods=thisDetector->nMod[X]*thisDetector->nMod[Y];
      int dr=thisDetector->dynamicRange;
      if (dr==24)
	dr=32;
      
      if (thisDetector->timerValue[PROBES_NUMBER]==0) {
	thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*dr/8;
      } else {
	thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
      }
      
#ifdef VERBOSE
      std::cout<< "Data size is " << thisDetector->dataBytes << std::endl;
      std::cout<< "nModX " << thisDetector->nMod[X] << " nModY " << thisDetector->nMod[Y] << " nChips " << thisDetector->nChips << " nChans " << thisDetector->nChans<< " dr " << dr << std::endl;
#endif
    }
    return thisDetector->nMod[d];
}; 



 
int multiSlsDetector::getMaxNumberOfModules(dimension d){

  int retval;
  int fnum=F_GET_MAX_NUMBER_OF_MODULES;
  int ret=FAIL;
  char mess[100]; 

  if (d<X || d>Y) {
    std::cout<< "Get max number of modules in wrong dimension " << d << std::endl;
    return ret;
  }
#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting max number of modules in dimension "<< d  <<std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&d,sizeof(d));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Deterctor returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=OK;
    retval=thisDetector->nModMax[d];
  }
#ifdef VERBOSE
    std::cout<< "Max number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
    if (ret==FAIL) {
      std::cout<< "Get max number of modules failed " << std::endl;
      return retval;
    }  else {
      thisDetector->nModMax[d]=retval;
      thisDetector->nModsMax=thisDetector->nModMax[0]*thisDetector->nModMax[1];
    }
    return thisDetector->nModMax[d];
}; 


 
  /*
    This function is used to set the polarity and meaning of the digital I/O signals (signal index)
    
enum externalSignalFlag {
  GET_EXTERNAL_SIGNAL_FLAG,
  SIGNAL_OFF,
  GATE_ACTIVE_HIGH,
  GATE_ACTIVE_LOW,
  TRIGGER_RISING_EDGE,
  TRIGGER_FALLING_EDGE
}{};
  */

 externalSignalFlag multiSlsDetector::setExternalSignalFlags(externalSignalFlag pol, int signalindex){



  
  int arg[2]; 
  externalSignalFlag  retval;
  int ret=FAIL;
  int fnum=F_SET_EXTERNAL_SIGNAL_FLAG;
  char mess[100];

  arg[0]=signalindex;
  arg[1]=pol;

  retval=GET_EXTERNAL_SIGNAL_FLAG;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting signal "<< signalindex <<  " to flag" << pol << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    retval=GET_EXTERNAL_SIGNAL_FLAG;
    ret=FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Signal "<< signalindex <<  " flag set to" << retval << std::endl;
  if (ret==FAIL) {
    std::cout<< "Set signal flag failed " << std::endl;
  }
#endif
  return retval;






};

  /* 
     this function is used to select wether the detector is triggered or gated and in which mode
    enum externalCommunicationMode{
  GET_EXTERNAL_COMMUNICATION_MODE,
  AUTO,
  TRIGGER_EXPOSURE,
  TRIGGER_READOUT,
  TRIGGER_COINCIDENCE_WITH_INTERNAL_ENABLE,
  GATE_FIX_NUMBER,
  GATE_FIX_DURATION,
  GATE_WITH_START_TRIGGER,
  GATE_COINCIDENCE_WITH_INTERNAL_ENABLE
};

  */

   externalCommunicationMode multiSlsDetector::setExternalCommunicationMode( externalCommunicationMode pol){



  
  int arg[1]; 
  externalCommunicationMode  retval;
  int fnum=F_SET_EXTERNAL_COMMUNICATION_MODE;
  char mess[100];
    
  arg[0]=pol;

  int ret=FAIL;
  retval=GET_EXTERNAL_COMMUNICATION_MODE;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting communication to mode " << pol << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {  
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    retval=GET_EXTERNAL_COMMUNICATION_MODE;
    ret=FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Communication mode "<<   " set to" << retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Setting communication mode failed" << std::endl;
  }
  return retval;

};


  // Tests and identification
  /* 
     Gets versions

     enum idMode{
     MODULE_SERIAL_NUMBER,
     MODULE_FIRMWARE_VERSION,
     DETECTOR_SERIAL_NUMBER,
     DETECTOR_FIRMWARE_VERSION,
     DETECTOR_SOFTWARE_VERSION 
     }{};

  */





int64_t multiSlsDetector::getId( idMode mode, int imod){


  int64_t retval=-1;
  int fnum=F_GET_ID;
  int ret=FAIL;

  char mess[100];

#ifdef VERBOSE
  std::cout<< std::endl;
  if  (mode==MODULE_SERIAL_NUMBER)
    std::cout<< "Getting id  of "<< imod << std::endl; 
  else
     std::cout<< "Getting id type "<< mode << std::endl; 
#endif
  if (mode==THIS_SOFTWARE_VERSION) {
    ret=OK;
    retval=thisSoftwareVersion;
  } else {
    if (thisDetector->onlineFlag==ONLINE_FLAG) {
      if (controlSocket) {
	if  (controlSocket->Connect()>=0) {
	  controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	  controlSocket->SendDataOnly(&mode,sizeof(mode));
	  if (mode==MODULE_SERIAL_NUMBER)
	    controlSocket->SendDataOnly(&imod,sizeof(imod));
	  controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	  if (ret==OK)
	    controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  else {
	    controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	    std::cout<< "Detector returned error: " << mess << std::endl;
	  }
	  controlSocket->Disconnect();
	} else 
	  ret=FAIL;
      } else {
	ret=FAIL;
      }
    }
  }
  if (ret==FAIL) {
    std::cout<< "Get id failed " << std::endl;
    return ret;
  } else {
#ifdef VERBOSE
    if  (mode==MODULE_SERIAL_NUMBER)
      std::cout<< "Id of "<< imod <<" is " << hex <<retval << setbase(10) << std::endl;
    else
      std::cout<< "Id "<< mode <<" is " << hex <<retval << setbase(10) << std::endl;
#endif
    return retval;
  }
};



  /*
    Digital test of the modules

    enum digitalTestMode {
    CHIP_TEST,
    MODULE_FIRMWARE_TEST,
    DETECTOR_FIRMWARE_TEST,
    DETECTOR_MEMORY_TEST,
    DETECTOR_BUS_TEST,
    DETECTOR_SOFTWARE_TEST
    }{};
    returns ok or error mask
  */

int multiSlsDetector::digitalTest( digitalTestMode mode, int imod){


  int retval;
  int fnum=F_DIGITAL_TEST;
  int ret=FAIL;

  char mess[100];

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting id of "<< mode << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {


    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&mode,sizeof(mode));
	if (mode==CHIP_TEST)
	   controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=FAIL;
  }
#ifdef VERBOSE
    std::cout<< "Id "<< mode <<" is " << retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Get id failed " << std::endl;
    return ret;
  } else
    return retval;
};



  /* 
     analog test of the modules
     enum analogTestMode {
     COUNT_CALIBRATION_PULSES,
     I_DON_T_KNOW
     }{};

  */
/*
int* multiSlsDetector::analogTest(analogTestMode mode){
  std::cout<< "function not yet implemented " << std::endl;
};
*/
  /* 
     enable analog output of channel 
  */
/*
int multiSlsDetector::enableAnalogOutput(int ichan){
  int imod=ichan/(nChans*nChips);
  ichan-=imod*(nChans*nChips);
  int ichip=ichan/nChans;
  ichan-=ichip*(nChans);
  enableAnalogOutput(imod,ichip,ichan);
  
};
int multiSlsDetector::enableAnalogOutput(int imod, int ichip, int ichan){
  std::cout<< "function not yet implemented " << std::endl;
};
*/
  /* 
     give a train of calibration pulses 
  */ 
/*
int multiSlsDetector::giveCalibrationPulse(float vcal, int npulses){
  std::cout<< "function not yet implemented " << std::endl;
};
*/
  // Expert low level functions



  /* write or read register */

int multiSlsDetector::writeRegister(int addr, int val){


  int retval;
  int fnum=F_WRITE_REGISTER;
  int ret=FAIL;

  char mess[100];
  
  int arg[2];
  arg[0]=addr;
  arg[1]=val;
  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Writing to register "<< addr <<  " data " << val << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } 
#ifdef VERBOSE
  std::cout<< "Register returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Write to register failed " << std::endl;
  }
  return retval;

};




int multiSlsDetector::readRegister(int addr){


  int retval;
  int fnum=F_READ_REGISTER;
  int ret=FAIL;

  char mess[100];

  int arg;
  arg=addr;
  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Reding register "<< addr  << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
    }
  } 
#ifdef VERBOSE
  std::cout<< "Register returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Read register failed " << std::endl;
  }
  return retval;

};


  // Expert initialization functions
  /*
    set dacs or read ADC for the module
    enum dacIndex {
    TRIMBIT_SIZE,
    THRESHOLD,
    SHAPER1,
    SHAPER2,
    CALIBRATION_PULSE,
    PREAMP,
    TEMPERATURE,
    HUMIDITY,
    DETECTOR_BIAS
}{};
  */


float multiSlsDetector::setDAC(float val, dacIndex index, int imod){


  float retval;
  int fnum=F_SET_DAC;
  int ret=FAIL;
  char mess[100];
  int arg[2];
  arg[0]=index;
  arg[1]=imod;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting DAC/POT "<< index << "of module " << imod  <<  " to " << val << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->SendDataOnly(&val,sizeof(val));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK) {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  if (index <  thisDetector->nDacs){

	    if (dacs) {
	      if (imod>=0) {
		*(dacs+index+imod*thisDetector->nDacs)=retval;
	      }
	      else {
		for (imod=0; imod<thisDetector->nModsMax; imod++)
		  *(dacs+index+imod*thisDetector->nDacs)=retval;
	      }
	    }
	  }
	} else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
      }
	
    }
  }
#ifdef VERBOSE
  std::cout<< "Dac/Pot set to "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Set dac/pot failed " << std::endl;
  }
  return retval;



};


float multiSlsDetector::getADC(dacIndex index, int imod){

  float retval;
  int fnum=F_GET_ADC;
  int ret=FAIL;
  char mess[100];

  int arg[2];
  arg[0]=index;
  arg[1]=imod;

  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting ADC "<< index << "of module " << imod  <<   std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK) {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  if (adcs) {
	      *(adcs+index+imod*thisDetector->nAdcs)=retval;
	  }
	} else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	  }
	controlSocket->Disconnect();
      }
    }
  } 
#ifdef VERBOSE
  std::cout<< "ADC returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Get ADC failed " << std::endl;
  }
  return retval;



}; 

  /* 
     configure single channel 
     enum channelRegisterBit {
     COMPARATOR_ENABLE_OFF,
     ANALOG_SIGNAL_ENABLE_OFF,
     CALIBRATION_ENABLE_OFF,
     TRIMBIT_OFF // should always be the last!
     }

  */

int multiSlsDetector::setChannel(int64_t reg, int ichan, int ichip, int imod){
  sls_detector_channel myChan;
#ifdef VERBOSE
  std::cout<< "Setting channel "<< ichan << " " << ichip << " " << imod << " to " << reg << std::endl;
#endif
  //int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan, chamax=ichan+1;

  int ret;

  /*  if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  if (ichip==-1) {
    chimin=0;
    chimax=thisDetector->nChips;
  }

  if (ichan==-1) {
    chamin=0;
    chamax=thisDetector->nChans;
    }*/

  // for (int im=mmin; im<mmax; im++) {
  //  for (int ichi=chimin; ichi<chimax; ichi++) {
  //    for (int icha=chamin; icha<chamax; icha++) {
  myChan.chan=ichan;//icha;
  myChan.chip=ichip;//ichi;
  myChan.module=imod;//im;
  myChan.reg=reg;
  ret=setChannel(myChan);	 
	//     }
	// }
	// }
  return ret;
}



int multiSlsDetector::setChannel(sls_detector_channel chan){
  int fnum=F_SET_CHANNEL;
  int retval;
  int ret=FAIL;
  char mess[100];

  int ichan=chan.chan;
  int ichip=chan.chip;
  int imod=chan.module;

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendChannel(&chan);
      
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==OK) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }


  if (ret==OK) {
    if (chanregs) {

int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan, chamax=ichan+1;

 if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  if (ichip==-1) {
    chimin=0;
    chimax=thisDetector->nChips;
  }

  if (ichan==-1) {
    chamin=0;
    chamax=thisDetector->nChans;
  }






  for (int im=mmin; im<mmax; im++) {
   for (int ichi=chimin; ichi<chimax; ichi++) {
     for (int icha=chamin; icha<chamax; icha++) {

      *(chanregs+im*thisDetector->nChans*thisDetector->nChips+ichi*thisDetector->nChips+icha)=retval;     

    }
  }
}
 
    }
  }
#ifdef VERBOSE
  std::cout<< "Channel register returned "<<  retval << std::endl;
#endif
  return retval;
	
}

 sls_detector_channel  multiSlsDetector::getChannel(int ichan, int ichip, int imod){


  int fnum=F_GET_CHANNEL;
  sls_detector_channel myChan;
  int arg[3];
  int ret=FAIL;
  char mess[100];
  arg[0]=ichan;
  arg[1]=ichip;
  arg[2]=imod;  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
   
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==OK) {
	receiveChannel(&myChan);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {
    if (chanregs) {
      *(chanregs+imod*thisDetector->nChans*thisDetector->nChips+ichip*thisDetector->nChips+ichan)=myChan.reg;    
    }
  }

#ifdef VERBOSE
  std::cout<< "Returned channel "<< ichan << " " << ichip << " " << imod << " " <<  myChan.reg << std::endl;
#endif
  return myChan;
}

    /* 
       configure chip
       enum chipRegisterBit {
       ENABLE_ANALOG_OUTPUT,
       OUTPUT_WIDTH // should always be the last
       }{};
  */
int multiSlsDetector::setChip(int reg, int ichip, int imod){
   sls_detector_chip myChip;

#ifdef VERBOSE
  std::cout<< "Setting chip "<<  ichip << " " << imod << " to " << reg <<  std::endl;
#endif


  int chregs[thisDetector->nChans];
  int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1;
  int ret=FAIL;
  if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  if (ichip==-1) {
    chimin=0;
    chimax=thisDetector->nChips;
  }

  myChip.nchan=thisDetector->nChans;
  myChip.reg=reg;
  for (int im=mmin; im<mmax; im++) {
    for (int ichi=chimin; ichi<chimax; ichi++) {
      myChip.chip=ichi;
      myChip.module=im;
      if (chanregs)
	myChip.chanregs=(chanregs+ichi*thisDetector->nChans+im*thisDetector->nChans*thisDetector->nChips); 
      else {
	for (int i=0; i<thisDetector->nChans; i++)
	  chregs[i]=-1;
	myChip.chanregs=chregs;
      }
      ret=setChip(myChip);
    }
  }
  return ret;
}

int multiSlsDetector::setChip(sls_detector_chip chip){

  int fnum=F_SET_CHIP;
  int retval;
  int ret=FAIL;
  char mess[100];

  int ichi=chip.chip;
  int im=chip.module;





  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendChip(&chip);
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==OK) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }


  if (ret==OK) {
    if (chipregs)
      *(chipregs+ichi+im*thisDetector->nChips)=retval;
  }

#ifdef VERBOSE
  std::cout<< "Chip register returned "<<  retval << std::endl;
#endif
  return retval;
};


 sls_detector_chip multiSlsDetector::getChip(int ichip, int imod){

  int fnum=F_GET_CHIP;
  sls_detector_chip myChip;
  int chanreg[thisDetector->nChans];
 
  int ret=FAIL;
  char mess[100];


  myChip.chip=ichip;
  myChip.module=imod;
  myChip.nchan=thisDetector->nChans;
  myChip.chanregs=chanreg;

  int arg[2];
  arg[0]=ichip;
  arg[1]=imod;




  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
   
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==OK) {
	receiveChip(&myChip);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {
    if (chipregs)
      *(chipregs+ichip+imod*thisDetector->nChips)=myChip.reg;
    if (chanregs) {
      for (int ichan=0; ichan<thisDetector->nChans; ichan++)
	*(chanregs+imod*thisDetector->nChans*thisDetector->nChips+ichip*thisDetector->nChans+ichan)=*((myChip.chanregs)+ichan);
    }
  }
#ifdef VERBOSE
  std::cout<< "Returned chip "<<  ichip << " " << imod << " " <<  myChip.reg << std::endl;
#endif

  return myChip;
};
 
  /* 
     configure module
     enum moduleRegisterBit {
     I_DON_T_KNOW,
     OUTPUT_WIDTH // should always be the last
     }{};
  */

int multiSlsDetector::setModule(int reg, int imod){
  sls_detector_module myModule;
  
#ifdef VERBOSE
    std::cout << "slsDetector set module " << std::endl;
#endif 
  int charegs[thisDetector->nChans*thisDetector->nChips];
  int chiregs[thisDetector->nChips];
  float das[thisDetector->nDacs], ads[thisDetector->nAdcs];
  int mmin=imod, mmax=imod+1;
  int ret=FAIL;
  
  if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  
  
  for (int im=mmin; im<mmax; im++) {
     
    myModule.module=im;
    myModule.nchan=thisDetector->nChans;
    myModule.nchip=thisDetector->nChips;
    myModule.ndac=thisDetector->nDacs;
    myModule.nadc=thisDetector->nAdcs;
    
    myModule.reg=reg;
    if (detectorModules) {
      myModule.gain=(detectorModules+im)->gain;
      myModule.offset=(detectorModules+im)->offset;
      myModule.serialnumber=(detectorModules+im)->serialnumber;
    } else {
      myModule.gain=-1;
      myModule.offset=-1;
      myModule.serialnumber=-1;
    }
    
   
    for (int i=0; i<thisDetector->nAdcs; i++)
      ads[i]=-1;
    
    if (chanregs)
      myModule.chanregs=chanregs+im*thisDetector->nChips*thisDetector->nChans;
    else {	
      for (int i=0; i<thisDetector->nChans*thisDetector->nChips; i++)
	charegs[i]=-1;
      myModule.chanregs=charegs;
    }
    if (chipregs)
      myModule.chipregs=chanregs+im*thisDetector->nChips;
    else { 
      for (int ichip=0; ichip<thisDetector->nChips; ichip++)
	chiregs[ichip]=-1;
      myModule.chipregs=chiregs;
    }
    if (dacs)
      myModule.dacs=dacs+im*thisDetector->nDacs;
    else {   
      for (int i=0; i<thisDetector->nDacs; i++)
	das[i]=-1; 
      myModule.dacs=das;
    }
    if (adcs)
      myModule.adcs=adcs+im*thisDetector->nAdcs;
    else {   
      for (int i=0; i<thisDetector->nAdcs; i++)
	ads[i]=-1;
      myModule.adcs=ads; 
    }
    ret=setModule(myModule);
  }
  return ret;
  

};



int multiSlsDetector::setModule(sls_detector_module module){


  int fnum=F_SET_MODULE;
  int retval;
  int ret=FAIL;
  char mess[100];

  int imod=module.module;



#ifdef VERBOSE
    std::cout << "slsDetector set module " << std::endl;
#endif 


  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendModule(&module);
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==OK) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }

  
  if (ret==OK) {
    if (detectorModules) {
      if (imod>=0 && imod<thisDetector->nMod[X]*thisDetector->nMod[Y]) {
	(detectorModules+imod)->nchan=module.nchan;
	(detectorModules+imod)->nchip=module.nchip;
	(detectorModules+imod)->ndac=module.ndac;
	(detectorModules+imod)->nadc=module.nadc;
	thisDetector->nChips=module.nchip;
	thisDetector->nChans=module.nchan/module.nchip;
	thisDetector->nDacs=module.ndac;
	thisDetector->nAdcs=module.nadc;
	
	for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
	  if (chipregs)
	    chipregs[ichip+thisDetector->nChips*imod]=module.chipregs[ichip];
	  
	  if (chanregs) {
	    for (int i=0; i<thisDetector->nChans; i++) {
	      chanregs[i+ichip*thisDetector->nChans+thisDetector->nChips*thisDetector->nChans*imod]=module.chanregs[ichip*thisDetector->nChans+i];
	    }
	  }
	}
	if (dacs) {
	  for (int i=0; i<thisDetector->nDacs; i++)
	    dacs[i+imod*thisDetector->nDacs]=module.dacs[i];
	}
	if (adcs) {
	  for (int i=0; i<thisDetector->nAdcs; i++)
	    adcs[i+imod*thisDetector->nAdcs]=module.adcs[i];
	}
	
	(detectorModules+imod)->gain=module.gain;
	(detectorModules+imod)->offset=module.offset;
	(detectorModules+imod)->serialnumber=module.serialnumber;
	(detectorModules+imod)->reg=module.reg;
      }
    }
  }

#ifdef VERBOSE
  std::cout<< "Module register returned "<<  retval << std::endl;
#endif

  return retval;
};

sls_detector_module  *multiSlsDetector::getModule(int imod){

#ifdef VERBOSE
  std::cout << "slsDetector get module " << std::endl;
#endif 

  int fnum=F_GET_MODULE;
  sls_detector_module *myMod=createModule();


  //char *ptr,  *goff=(char*)thisDetector;

  // int chanreg[thisDetector->nChans*thisDetector->nChips];
  //int chipreg[thisDetector->nChips];
  //float dac[thisDetector->nDacs], adc[thisDetector->nAdcs];

  int ret=FAIL;
  char mess[100];
  // int n;

#ifdef VERBOSE
  std::cout<< "getting module " << imod << std::endl;
#endif

  myMod->module=imod;
  // myMod.nchan=thisDetector->nChans*thisDetector->nChips;
  //myMod.chanregs=chanreg;
  //myMod.nchip=thisDetector->nChips;
  //myMod.chipregs=chipreg;
  //myMod.ndac=thisDetector->nDacs;
  //myMod.dacs=dac;
  //myMod.ndac=thisDetector->nAdcs;
  //myMod.dacs=adc;

 




  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&imod,sizeof(imod));
   
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==OK) {
	receiveModule(myMod);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {
    if (detectorModules) {
      if (imod>=0 && imod<thisDetector->nMod[X]*thisDetector->nMod[Y]) {
	(detectorModules+imod)->nchan=myMod->nchan;
	(detectorModules+imod)->nchip=myMod->nchip;
	(detectorModules+imod)->ndac=myMod->ndac;
	(detectorModules+imod)->nadc=myMod->nadc;
	thisDetector->nChips=myMod->nchip;
	thisDetector->nChans=myMod->nchan/myMod->nchip;
	thisDetector->nDacs=myMod->ndac;
	thisDetector->nAdcs=myMod->nadc;
	
	for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
	  if (chipregs)
	    chipregs[ichip+thisDetector->nChips*imod]=myMod->chipregs[ichip];
	  
	  if (chanregs) {
	    for (int i=0; i<thisDetector->nChans; i++) {
	      chanregs[i+ichip*thisDetector->nChans+thisDetector->nChips*thisDetector->nChans*imod]=myMod->chanregs[ichip*thisDetector->nChans+i];
	    }
	  }
	}
	if (dacs) {
	  for (int i=0; i<thisDetector->nDacs; i++)
	    dacs[i+imod*thisDetector->nDacs]=myMod->dacs[i];
	}
	if (adcs) {
	  for (int i=0; i<thisDetector->nAdcs; i++)
	    adcs[i+imod*thisDetector->nAdcs]=myMod->adcs[i];
	}
	
	(detectorModules+imod)->gain=myMod->gain;
	(detectorModules+imod)->offset=myMod->offset;
	(detectorModules+imod)->serialnumber=myMod->serialnumber;
	(detectorModules+imod)->reg=myMod->reg;
      }
    }
  } else {
    deleteModule(myMod);
    myMod=NULL;
  }

  return myMod;
}




  // calibration functions
/*
  really needed?

int multiSlsDetector::setCalibration(int imod,  detectorSettings isettings, float gain, float offset){
  std::cout<< "function not yet implemented " << std::endl; 
  
  

  return OK;

}
int multiSlsDetector::getCalibration(int imod,  detectorSettings isettings, float &gain, float &offset){

  std::cout<< "function not yet implemented " << std::endl; 



}
*/

  /*
    calibrated setup of the threshold
  */

int multiSlsDetector::getThresholdEnergy(int imod){

  int fnum=  F_GET_THRESHOLD_ENERGY;
  int retval;
  int ret=FAIL;
  char mess[100];
#ifdef VERBOSE
  std::cout<< "Getting threshold energy "<< std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  std::cout<< "Detector returned error: "<< std::endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<<  mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	  thisDetector->currentThresholdEV=retval;
	}
	controlSocket->Disconnect();
      }
    }
  } 
  return  thisDetector->currentThresholdEV;
};  

int multiSlsDetector::setThresholdEnergy(int e_eV,  int imod, detectorSettings isettings){

  int fnum=  F_SET_THRESHOLD_ENERGY;
  int retval;
  int ret=FAIL;
  char mess[100];
#ifdef VERBOSE
  std::cout<< "Getting threshold energy "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&e_eV,sizeof(e_eV));
	controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->SendDataOnly(&isettings,sizeof(isettings));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  std::cout<< "Detector returned error: "<< std::endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<<  mess << std::endl;
	} else {
#ifdef VERBOSE
	  std::cout<< "Detector returned OK "<< std::endl;
#endif
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	  thisDetector->currentThresholdEV=retval;   
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    thisDetector->currentThresholdEV=e_eV;
  }
  return   thisDetector->currentThresholdEV;
};  

  /*
    select detector settings
  */
 detectorSettings  multiSlsDetector::getSettings(int imod){


  int fnum=F_SET_SETTINGS;
  int ret=FAIL;
  char mess[100];
  int  retval;
  int arg[2];
  arg[0]=GET_SETTINGS;
  arg[1]=imod;
#ifdef VERBOSE
  std::cout<< "Getting settings "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else{
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  thisDetector->currentSettings=(detectorSettings)retval;
#ifdef VERBOSE
	  std::cout<< "Settings are "<< retval << std::endl;
#endif
    }
	controlSocket->Disconnect();
      }
    }
  }
  return thisDetector->currentSettings;

};

 detectorSettings multiSlsDetector::setSettings( detectorSettings isettings, int imod){
#ifdef VERBOSE
  std::cout<< "slsDetector setSettings "<< std::endl;
#endif
  sls_detector_module *myMod=createModule();
  int modmi=imod, modma=imod+1, im=imod;
  string trimfname, calfname;
  string ssettings;

  if (isettings>=STANDARD && isettings<=HIGHGAIN) {
    switch (isettings) {
    case STANDARD:
      ssettings="/standard";
      thisDetector->currentSettings=STANDARD;
      break;
    case FAST:
      ssettings="/fast";
      thisDetector->currentSettings=FAST;
      break;
    case HIGHGAIN:
      ssettings="/highgain";
      thisDetector->currentSettings=HIGHGAIN;
      break;
    default:
      std::cout<< "Unknown settings!" << std::endl;
    }
    
    if (imod<0) {
      modmi=0;
      //  modma=thisDetector->nModMax[X]*thisDetector->nModMax[Y];
      modma=thisDetector->nMod[X]*thisDetector->nMod[Y];
    }

    for (im=modmi; im<modma; im++) {
      ostringstream ostfn, oscfn;
      myMod->module=im;
      //create file names
      ostfn << thisDetector->trimDir << ssettings <<"/noise.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      oscfn << thisDetector->calDir << ssettings << "/calibration.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      //
      trimfname=ostfn.str();
#ifdef VERBOSE
      cout << trimfname << endl;
#endif
      if (readTrimFile(trimfname,myMod)) {
	calfname=oscfn.str();
#ifdef VERBOSE
      cout << calfname << endl;
#endif
	readCalibrationFile(calfname,myMod->gain, myMod->offset);
	setModule(*myMod);
      } else {
	ostringstream ostfn,oscfn;
	ostfn << thisDetector->trimDir << ssettings << ssettings << ".trim"; 
	oscfn << thisDetector->calDir << ssettings << ssettings << ".cal";
	calfname=oscfn.str();
	trimfname=ostfn.str();
#ifdef VERBOSE
      cout << trimfname << endl;
      cout << calfname << endl;
#endif
	if (readTrimFile(trimfname,myMod)) {
	  calfname=oscfn.str();
	  readCalibrationFile(calfname,myMod->gain, myMod->offset);
	  setModule(*myMod);
	}
      }
    }
  }
  deleteModule(myMod);
  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
    int isett=getSettings(imod);
    float t[]=defaultTDead;
    if (isett>-1 && isett<3) {
      thisDetector->tDead=t[isett];
    }
  }


  return  getSettings(imod);
};

// Acquisition functions
/* change these funcs accepting also ok/fail */

int multiSlsDetector::startAcquisition(){


  int fnum=F_START_ACQUISITION;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< "Starting acquisition "<< std::endl;
#endif
  thisDetector->stoppedFlag=0;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
    controlSocket->Disconnect();
      }
    }
  }
  return ret;



};
int multiSlsDetector::stopAcquisition(){


  int fnum=F_STOP_ACQUISITION;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< "Stopping acquisition "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
  if  (stopSocket->Connect()>=0) {
    stopSocket->SendDataOnly(&fnum,sizeof(fnum));
    stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      stopSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    }
    stopSocket->Disconnect();
  }
    }
  }
  thisDetector->stoppedFlag=1;
  return ret;


};

int multiSlsDetector::startReadOut(){

  int fnum=F_START_READOUT;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< "Starting readout "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    }
    controlSocket->Disconnect();
  }
    }
  }
  return ret;
};



/*int multiSlsDetector::getRunStatus(){
  int fnum=F_GET_RUN_STATUS;
  int retval;
  int ret=FAIL;
  char mess[100];
#ifdef VERBOSE
  std::cout<< "Getting status "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    } else
      controlSocket->ReceiveDataOnly(&retval,sizeof(retval));    
    controlSocket->Disconnect();
  }
    }
  }
  return retval;


};
*/

int* multiSlsDetector::readFrame(){

  int fnum=F_READ_FRAME;
  int* retval=NULL;

#ifdef VERBOSE
  std::cout<< "slsDetector: Reading frame "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	retval=getDataFromDetector();
	if (retval) {
	  dataQueue.push(retval);
	  controlSocket->Disconnect();
	}
      }
    }
  }
  return retval;
};

int* multiSlsDetector::getDataFromDetector(){

  int nel=thisDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int ret=FAIL;
  char mess[100]="Nothing"; 
#ifdef VERY_VERBOSE
  int i;
#endif     

#ifdef VERBOSE
  //  std::cout<< "getting data "<< std::endl;
#endif
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      n= controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      if (ret==FAIL) {
	thisDetector->stoppedFlag=1;
	std::cout<< "Detector returned: " << mess << " " << n << std::endl;
      } else {
	;
#ifdef VERBOSE
	std::cout<< "Detector successfully returned: " << mess << " " << n << std::endl;
#endif	
      } 
      delete [] retval;
      retval=NULL;
    } else {
      n=controlSocket->ReceiveDataOnly(retval,thisDetector->dataBytes);
	
      //#ifdef VERBOSE
      std::cout<< "Received "<< n << " data bytes" << std::endl;
      //#endif 
      if (n!=thisDetector->dataBytes) {
	std::cout<< "wrong data size received: received " << n << " but expected " << thisDetector->dataBytes << std::endl;
	thisDetector->stoppedFlag=1;
	ret=FAIL;
	delete [] retval;
	retval=NULL;
      }
    }

    return retval;
};



int* multiSlsDetector::readAll(){
  
  int fnum=F_READ_ALL;
  int* retval; // check what we return!

  int i=0;
#ifdef VERBOSE
  std::cout<< "Reading all frames "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      // std::cout<< i << std::endl;
#endif
      dataQueue.push(retval);
    }
    controlSocket->Disconnect();
  }
    }
  }
#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
#endif
  return dataQueue.front(); // check what we return!

};

int* multiSlsDetector::startAndReadAll(){

 
  int* retval;
  int i=0;
  startAndReadAllNoWait();  
  while ((retval=getDataFromDetector())){
      i++;
      //#ifdef VERBOSE
      std::cout<< i << std::endl;
      //#endif
      dataQueue.push(retval);
  }
  controlSocket->Disconnect();

  //#ifdef VERBOSE
  std::cout<< "recieved "<< i<< " frames" << std::endl;
  //#endif
  return dataQueue.front(); // check what we return!
/* while ((retval=getDataFromDetectorNoWait()))
   i++;
   #ifdef VERBOSE
  std::cout<< "Received " << i << " frames"<< std::endl;
#endif
  return dataQueue.front(); // check what we return!
  */
  
};



int multiSlsDetector::startAndReadAllNoWait(){

  int fnum= F_START_AND_READ_ALL;
  
#ifdef VERBOSE
  std::cout<< "Starting and reading all frames "<< std::endl;
#endif
  thisDetector->stoppedFlag=0;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    return OK;
  }
    }
  }
  return FAIL;
};

int* multiSlsDetector::getDataFromDetectorNoWait() {
  int *retval=getDataFromDetector();
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if (retval==NULL){
	controlSocket->Disconnect();

#ifdef VERBOSE
  std::cout<< "Run finished "<< std::endl;
#endif
  } else {
#ifdef VERBOSE
    std::cout<< "Frame received "<< std::endl;
#endif
  }
    }
  }
  return retval; // check what we return!
};





int* multiSlsDetector::popDataQueue() {
  int *retval=NULL;
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  return retval;
}

detectorData* multiSlsDetector::popFinalDataQueue() {
  detectorData *retval=NULL;
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  return retval;
}

void multiSlsDetector::resetDataQueue() {
  int *retval=NULL;
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
 
}

void multiSlsDetector::resetFinalDataQueue() {
  detectorData *retval=NULL;
  while( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
    delete retval;
  }

}

  /* 
     set or read the acquisition timers 
     enum timerIndex {
     FRAME_NUMBER,
     ACQUISITION_TIME,
     FRAME_PERIOD,
     DELAY_AFTER_TRIGGER,
     GATES_NUMBER,
     PROBES_NUMBER
     CYCLES_NUMBER,
     GATE_INTEGRATED_TIME
     }
  */
int64_t multiSlsDetector::setTimer(timerIndex index, int64_t t){
  

  int fnum=F_SET_TIMER;
  int64_t retval;
  uint64_t ut;
  char mess[100];
  int ret=OK;
  int n=0;
  
  

#ifdef VERBOSE
  std::cout<< "Setting timer  "<< index << " to " <<  t << "ns" << std::endl;
#endif
  ut=t;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&index,sizeof(index));
	n=controlSocket->SendDataOnly(&t,sizeof(t));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	  thisDetector->timerValue[index]=retval; 
	} 
	controlSocket->Disconnect();
      }
    }
  } else {
    //std::cout<< "offline " << std::endl;
    if (t>=0)
      thisDetector->timerValue[index]=t;

}
#ifdef VERBOSE
  std::cout<< "Timer " << index << " set to  "<< thisDetector->timerValue[index] << "ns"  << std::endl;
#endif
  if (index==PROBES_NUMBER) {
    setDynamicRange();
    //cout << "Changing probes: data size = " << thisDetector->dataBytes <<endl;
  }
  
  /* set progress */
  if ((index==FRAME_NUMBER) || (index==CYCLES_NUMBER)) {

    setTotalProgress();


  }

  return thisDetector->timerValue[index];
  
};






int multiSlsDetector::setTotalProgress() {

       int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1;
      
      if (thisDetector->timerValue[FRAME_NUMBER])
	nf=thisDetector->timerValue[FRAME_NUMBER];

      if (thisDetector->timerValue[CYCLES_NUMBER]>0)
	nc=thisDetector->timerValue[CYCLES_NUMBER];

      if (thisDetector->numberOfPositions>0)
	npos=thisDetector->numberOfPositions;

      if ((thisDetector->nScanSteps[0]>0) && (thisDetector->actionMask & (1 << MAX_ACTIONS)))
	nscan[0]=thisDetector->nScanSteps[0];

      if ((thisDetector->nScanSteps[1]>0) && (thisDetector->actionMask & (1 << (MAX_ACTIONS+1))))
	nscan[1]=thisDetector->nScanSteps[1];
      
      thisDetector->totalProgress=nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
      cout << "nc " << nc << endl;
      cout << "nf " << nf << endl;
      cout << "npos " << npos << endl;
      cout << "nscan[0] " << nscan[0] << endl;
      cout << "nscan[1] " << nscan[1] << endl;

      cout << "Set total progress " << thisDetector->totalProgress << endl;
#endif
      return thisDetector->totalProgress;
}


float multiSlsDetector::getCurrentProgress() {

  return 100.*((float)thisDetector->progressIndex)/((float)thisDetector->totalProgress);
}















/*
  important speed parameters

enum speedVariable {
  CLOCK_DIVIDER, 
  WAIT_STATES, 
  SET_SIGNAL_LENGTH 
};
*/

int multiSlsDetector::setSpeed(speedVariable sp, int value) {


  int fnum=F_SET_SPEED;
  int retval=-1;
  char mess[100];
  int ret=OK;
  int n=0;
#ifdef VERBOSE
  std::cout<< "Setting speed  variable"<< sp << " to " <<  value << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&sp,sizeof(sp));
	n=controlSocket->SendDataOnly(&value,sizeof(value));
#ifdef VERBOSE
	std::cout<< "Sent  "<< n << " bytes "  << std::endl;
#endif
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	} 
	controlSocket->Disconnect();
      }
    }
  }
#ifdef VERBOSE
  std::cout<< "Speed set to  "<< retval  << std::endl;
#endif
  return retval;
  
}
















int64_t multiSlsDetector::getTimeLeft(timerIndex index){
  

  int fnum=F_GET_TIME_LEFT;
  int64_t retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Getting  timer  "<< index <<  std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
//     if (controlSocket) {
//   if  (controlSocket->Connect()>=0) {
//     controlSocket->SendDataOnly(&fnum,sizeof(fnum));
//     controlSocket->SendDataOnly(&index,sizeof(index));
//     controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
//     if (ret!=OK) {
//       controlSocket->ReceiveDataOnly(mess,sizeof(mess));
//       std::cout<< "Detector returned error: " << mess << std::endl;
//     } else {
//       controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
//       // thisDetector->timerValue[index]=retval;
//     }   
//     controlSocket->Disconnect();
//   }
//     }
    if (stopSocket) {
  if  (stopSocket->Connect()>=0) {
    stopSocket->SendDataOnly(&fnum,sizeof(fnum));
    stopSocket->SendDataOnly(&index,sizeof(index));
    stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      stopSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    } else {
      stopSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
      // thisDetector->timerValue[index]=retval;
    }   
    stopSocket->Disconnect();
  }
    }
  }
#ifdef VERBOSE
  std::cout<< "Time left is  "<< retval << std::endl;
#endif
  return retval;
  
};


  // Flags
int multiSlsDetector::setDynamicRange(int n){

  int fnum=F_SET_DYNAMIC_RANGE;
  int retval=-1;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting dynamic range to "<< n << std::endl;
#endif
  if (n==24)
    n=32;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&n,sizeof(n));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
    } else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    if (n>0)
      thisDetector->dynamicRange=n;
    retval=thisDetector->dynamicRange;
  }
    
  if (ret==OK && retval>0) {
    /* checking the number of probes to chose the data size */
    if (thisDetector->timerValue[PROBES_NUMBER]==0) {
      thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*retval/8;
    } else {
      thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
    }
      
    if (retval==32)
      thisDetector->dynamicRange=24;
    else 
      thisDetector->dynamicRange=retval;
    
    
#ifdef VERBOSE
    std::cout<< "Dynamic range set to  "<< thisDetector->dynamicRange   << std::endl;
    std::cout<< "Data bytes "<< thisDetector->dataBytes   << std::endl;
#endif
    
  } 
  return thisDetector->dynamicRange;
};

/*

int multiSlsDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/

  /*
   
enum readOutFlags {
  NORMAL_READOUT,
  setReadOutFlags(STORE_IN_RAM,
  READ_HITS,
  ZERO_COMPRESSION,
  BACKGROUND_CORRECTION
}{};
 
  */

int multiSlsDetector::setReadOutFlags(readOutFlags flag){

  
  int fnum=F_SET_READOUT_FLAGS;
  readOutFlags retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting readout flags to "<< flag << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&flag,sizeof(flag));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	  thisDetector->roFlags=retval;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    if (flag!=GET_READOUT_FLAGS)
      thisDetector->roFlags=flag;
  }

#ifdef VERBOSE
  std::cout<< "Readout flag set to  "<< retval   << std::endl;
#endif
  return thisDetector->roFlags;
};

  //Trimming
  /*
enum trimMode {
  NOISE_TRIMMING,
  BEAM_TRIMMING,
  IMPROVE_TRIMMING,
  FIXEDSETTINGS_TRIMMING,
  OFFLINE_TRIMMING
}{};
  */
int multiSlsDetector::executeTrimming(trimMode mode, int par1, int par2, int imod){
  
  int fnum= F_EXECUTE_TRIMMING;
  int retval=FAIL;
  char mess[100];
  int ret=OK;
  int arg[3];
  arg[0]=imod;
  arg[1]=par1;
  arg[2]=par2;


#ifdef VERBOSE
  std::cout<< "Trimming module " << imod << " with mode "<< mode << " parameters " << par1 << " " << par2 << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    std::cout<< "sending mode bytes= "<<  controlSocket->SendDataOnly(&mode,sizeof(mode)) << std::endl;
    controlSocket->SendDataOnly(arg,sizeof(arg));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    } else {
#ifdef VERBOSE
      std::cout<< "Detector trimmed "<< ret   << std::endl;
#endif
  /*
      controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
      thisDetector->roFlags=retval;
  */
      retval=ret;
    }
    controlSocket->Disconnect();
  }
    }
  }
  return retval;

};

float* multiSlsDetector::decodeData(int *datain) {
  float *dataout=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
  const int bytesize=8;

  int ival=0;
  char *ptr=(char*)datain;
  char iptr;

  int nbits=thisDetector->dynamicRange;
  int  ipos=0, ichan=0, ibyte;
  if (thisDetector->timerValue[PROBES_NUMBER]==0) {
    switch (nbits) {
    case 1:
      for (ibyte=0; ibyte<thisDetector->dataBytes; ibyte++) {
	iptr=ptr[ibyte]&0x1;
	for (ipos=0; ipos<8; ipos++) {
	  //	dataout[ibyte*2+ichan]=((iptr&((0xf)<<ichan))>>ichan)&0xf;
	ival=(iptr>>(ipos))&0x1;
	dataout[ichan]=ival;
	ichan++;
	}
    }
      break;
    case 4:
      for (ibyte=0; ibyte<thisDetector->dataBytes; ibyte++) {
	iptr=ptr[ibyte]&0xff;
	for (ipos=0; ipos<2; ipos++) {
	  //	dataout[ibyte*2+ichan]=((iptr&((0xf)<<ichan))>>ichan)&0xf;
	  ival=(iptr>>(ipos*4))&0xf;
	  dataout[ichan]=ival;
	  ichan++;
	}
      }
      break;
    case 8:
      for (ichan=0; ichan<thisDetector->dataBytes; ichan++) {
	ival=ptr[ichan]&0xff;
	dataout[ichan]=ival;
      }
      break;
    case 16:
      for (ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++) {
	// dataout[ichan]=0;
	ival=0;
	for (ibyte=0; ibyte<2; ibyte++) {
	  iptr=ptr[ichan*2+ibyte];
	  ival|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));
	}
	dataout[ichan]=ival;
      }
      break;
    default:    
      for (ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++) {
	ival=datain[ichan]&0xffffff;
	dataout[ichan]=ival;
      }
    }
  } else { 
    for (ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++) {
      dataout[ichan]=datain[ichan];
    }
  }

  /*

  if (nbits==32) {
    for (ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++)
      dataout[ichan]=(datain[ichan]&0xffffff);
  } else {
  for (int ibyte=0; ibyte<thisDetector->dataBytes; ibyte++) {
    for (int ibit=0; ibit<bytesize; ibit++) {
      ival|=(ptr[ibyte]&(one<<ibit)>>ibit)<<ipos++;
      if (ipos==thisDetector->dynamicRange) {
	ipos=0;
	dataout[ichan]=ival;
	ichan++;
	ival=0;
	if (ichan>thisDetector->nChans*thisDetector->nChips*thisDetector->nMods){
	  std::cout<< "error: decoding too many channels!" << ichan;
	  break;
	}
      }
    }
  }
  }
  */
#ifdef VERBOSE
  std::cout<< "decoded "<< ichan << " channels" << std::endl;
#endif


  return dataout;
}
 
//Correction
  /*
    enum correctionFlags {
    DISCARD_BAD_CHANNELS,
    AVERAGE_NEIGHBOURS_FOR_BAD_CHANNELS,
    FLAT_FIELD_CORRECTION,
    RATE_CORRECTION,
    ANGULAR_CONVERSION
    }
  */

int multiSlsDetector::setFlatFieldCorrection(string fname){
  float data[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
  //float err[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
   float xmed[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
   int nmed=0;
   int im=0;
   int nch;
   thisDetector->nBadFF=0;

   char ffffname[MAX_STR_LENGTH*2];

  if (fname=="") {
#ifdef VERBOSE
   std::cout<< "disabling flat field correction" << std::endl;
#endif
    thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
    strcpy(thisDetector->flatFieldFile,"none");
  } else { 
#ifdef VERBOSE
   std::cout<< "Setting flat field correction from file " << fname << std::endl;
#endif
   sprintf(ffffname,"%s/%s",thisDetector->flatFieldDir,fname.c_str());
   nch=readDataFile(string(ffffname),data);
    if (nch>0) {
      strcpy(thisDetector->flatFieldFile,fname.c_str());
      for (int ichan=0; ichan<nch; ichan++) {
	if (data[ichan]>0) {
	  /* add to median */
	  im=0;
	  while ((im<nmed) && (xmed[im]<data[ichan])) 
	    im++;
	  for (int i=nmed; i>im; i--) 
	    xmed[i]=xmed[i-1];
	  xmed[im]=data[ichan];
	  nmed++;
        } else {
	  //add the channel to the ff bad channel list
	  if (thisDetector->nBadFF<MAX_BADCHANS) {
	    thisDetector->badFFList[thisDetector->nBadFF]=ichan;
	    (thisDetector->nBadFF)++;
#ifdef VERBOSE
	    std::cout<< "Channel " << ichan << " added to the bad channel list" << std::endl;
#endif
	  } else
	    std::cout<< "Too many bad channels " << std::endl;
	    
	  }
	}
    
      if (nmed>1 && xmed[nmed/2]>0) {
#ifdef VERBOSE
	std::cout<< "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << std::endl;
#endif
	
	thisDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);
	for (int ichan=0; ichan<nch; ichan++) {
	  if (data[ichan]>0) {
	    ffcoefficients[ichan]=xmed[nmed/2]/data[ichan];
	    fferrors[ichan]=ffcoefficients[ichan]*sqrt(data[ichan])/data[ichan];
	  } else {
	    ffcoefficients[ichan]=0.;
	    fferrors[ichan]=1.;
	  }
	} 
	for (int ichan=nch; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
	    ffcoefficients[ichan]=1.;
	    fferrors[ichan]=0.;
	}

	fillBadChannelMask();

      } else {
	std::cout<< "Flat field data from file " << fname << " are not valid (" << nmed << "///" << xmed[nmed/2] << std::endl;
	return -1;
      }
    } else {
      std::cout<< "Flat field from file " << fname << " is not valid " << nch << std::endl;
	return -1;
    } 
  }
  return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}
 
int multiSlsDetector::getFlatFieldCorrection(float *corr, float *ecorr) {
  if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Flat field correction is enabled" << std::endl;
#endif
    if (corr) {
      for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
	corr[ichan]=(ffcoefficients[ichan]*ffcoefficients[ichan])/(fferrors[ichan]*fferrors[ichan]);
	if (ecorr) {
	  ecorr[ichan]=ffcoefficients[ichan]/fferrors[ichan];
	}
      }
    }
    return 1;
  } else {
#ifdef VERBOSE
    std::cout<< "Flat field correction is disabled" << std::endl;
#endif
    if (corr)
      for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
	corr[ichan]=1;
	if (ecorr)
	  ecorr[ichan]=0;
      }
    return 0;
  }

}


int multiSlsDetector::flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr){
  float e;

  dataout=datain*ffcoefficient;

  if (errin==0 && datain>=0) 
    e=sqrt(datain);
  else
    e=errin;
  
  if (dataout>0)
    errout=sqrt(e*ffcoefficient*e*ffcoefficient+datain*fferr*datain*fferr);
  else
    errout=1.;
  
  return 0;
};

int multiSlsDetector::flatFieldCorrect(float* datain, float *errin, float* dataout, float *errout){
#ifdef VERBOSE
    std::cout<< "Flat field correcting data" << std::endl;
#endif
  float e, eo;
  if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
    for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
      if (errin==NULL) 
	e=0;
      else
	e=errin[ichan];
      
      flatFieldCorrect(datain[ichan],e,dataout[ichan],eo,ffcoefficients[ichan],fferrors[ichan]);
      if (errout)
	errout[ichan]=eo;
    }
  }
  return 0;

};

int multiSlsDetector::setRateCorrection(float t){
  float tdead[]=defaultTDead;

  if (t==0) {
#ifdef VERBOSE
    std::cout<< "unsetting rate correction" << std::endl;
#endif
    thisDetector->correctionMask&=~(1<<RATE_CORRECTION);
  } else {
    thisDetector->correctionMask|=(1<<RATE_CORRECTION);
    if (t>0)
      thisDetector->tDead=t;
    else {
      if (thisDetector->currentSettings<3 && thisDetector->currentSettings>-1)
	thisDetector->tDead=tdead[thisDetector->currentSettings];
      else
	thisDetector->tDead=0;
    }
#ifdef VERBOSE
    std::cout<< "Setting rate correction with dead time "<< thisDetector->tDead << std::endl;
#endif
  }
  return thisDetector->correctionMask&(1<<RATE_CORRECTION);
}


int multiSlsDetector::getRateCorrection(float &t){

  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisDetector->tDead << std::endl;
#endif
    t=thisDetector->tDead;
    return 1;
  } else
    t=0;
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    return 0;
};

float multiSlsDetector::getRateCorrectionTau(){

  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisDetector->tDead << std::endl;
#endif
    return thisDetector->tDead;
    //return 1;
  } else
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    return 0;
};







int multiSlsDetector::getRateCorrection(){

  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
    return 1;
  } else
    return 0;
};




 int multiSlsDetector::rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t){

   // float data;
   float e;
 
   dataout=(datain*exp(tau*datain/t));
   
   if (errin==0 && datain>=0) 
     e=sqrt(datain);
   else
     e=errin;
   
   if (dataout>0)
     errout=e*dataout*sqrt((1/(datain*datain)+tau*tau/(t*t)));
   else 
     errout=1.;
   return 0;

};


int multiSlsDetector::rateCorrect(float* datain, float *errin, float* dataout, float *errout){
  float tau=thisDetector->tDead;
  float t=thisDetector->timerValue[ACQUISITION_TIME];
  // float data;
  float e;
  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correcting data with dead time "<< tau << " and acquisition time "<< t << std::endl;
#endif
    for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
      
      if (errin==NULL) {
	e=sqrt(datain[ichan]);
      } else
	e=errin[ichan];
      
      rateCorrect(datain[ichan], e, dataout[ichan], errout[ichan], tau, t);
    }
  }
  
  return 0;
};


int multiSlsDetector::setBadChannelCorrection(string fname){
  ifstream infile;
  string str;
  int interrupt=0;
  int ich;
  int chmin,chmax;
#ifdef VERBOSE
  std::cout << "Setting bad channel correction to " << fname << std::endl;
#endif

  if (fname=="") {
    thisDetector->correctionMask&=~(1<< DISCARD_BAD_CHANNELS);
    thisDetector->nBadChans=0;
  } else { 
    if (fname=="default")
      fname=string(thisDetector->badChanFile);
    infile.open(fname.c_str(), ios_base::in);
    if (infile.is_open()==0) {
      std::cout << "could not open file " << fname <<std::endl;
      return -1;
    }
    thisDetector->nBadChans=0;
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout << str << std::endl;
#endif
      istringstream ssstr;
      ssstr.str(str);
      if (!ssstr.good() || infile.eof()) {
	interrupt=1;
	break;
      }
      if (str.find('-')!=string::npos) {
	ssstr >> chmin ;
	ssstr.str(str.substr(str.find('-')+1,str.size()));
	ssstr >> chmax;
#ifdef VERBOSE
	std::cout << "channels between"<< chmin << " and " << chmax << std::endl;
#endif
	for (ich=chmin; ich<=chmax; ich++) {
	  if (thisDetector->nBadChans<MAX_BADCHANS) {
	    thisDetector->badChansList[thisDetector->nBadChans]=ich;
	    thisDetector->nBadChans++;
#ifdef VERBOSE
	    std::cout<< thisDetector->nBadChans << " Found bad channel "<< ich << std::endl;
#endif
	  } else
	    interrupt=1;
	}
      } else {
	ssstr >> ich;
#ifdef VERBOSE
	std::cout << "channel "<< ich << std::endl;
#endif
	if (thisDetector->nBadChans<MAX_BADCHANS) {
	  thisDetector->badChansList[thisDetector->nBadChans]=ich;
	  thisDetector->nBadChans++;
#ifdef VERBOSE
	  std::cout << thisDetector->nBadChans << " Found bad channel "<< ich << std::endl;
#endif
	} else
	  interrupt=1;
      }


    }
    if (thisDetector->nBadChans>0 && thisDetector->nBadChans<MAX_BADCHANS) {
      thisDetector->correctionMask|=(1<< DISCARD_BAD_CHANNELS);
      strcpy(thisDetector->badChanFile,fname.c_str());
    }
  }
  infile.close();
#ifdef VERBOSE
  std::cout << "found " << thisDetector->nBadChans << " badchannels "<< std::endl;
#endif
  fillBadChannelMask();
#ifdef VERBOSE
  std::cout << " badchannels mask filled"<< std::endl;
#endif
  return thisDetector->nBadChans;
}

int multiSlsDetector::getBadChannelCorrection(int *bad) {
  int ichan;
  if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    if (bad) {
      for (ichan=0; ichan<thisDetector->nBadChans; ichan++)
	bad[ichan]=thisDetector->badChansList[ichan];
      for (int ich=0; ich<thisDetector->nBadFF; ich++)
	bad[ichan+ich]=thisDetector->badFFList[ich];
    }
    return thisDetector->nBadChans+thisDetector->nBadFF;
  } else
   return 0;
}


int multiSlsDetector::fillBadChannelMask() {

  if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    if (badChannelMask) 
      delete [] badChannelMask;
    badChannelMask=new int[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
    for (int ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++)
      badChannelMask[ichan]=0;
    for (int ichan=0; ichan<thisDetector->nBadChans; ichan++) {
      if (thisDetector->badChansList[ichan]<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods) {
	badChannelMask[thisDetector->badChansList[ichan]]=1;
#ifdef VERBOSE
      std::cout << ichan << " badchannel "<< ichan << std::endl;
#endif
      }
    }
    for (int ichan=0; ichan<thisDetector->nBadFF; ichan++) {
      if (thisDetector->badFFList[ichan]<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods) {
	badChannelMask[thisDetector->badFFList[ichan]]=1;
#ifdef VERBOSE
      std::cout << ichan << "ff badchannel "<< thisDetector->badFFList[ichan] << std::endl;
#endif
      }
    }
    
  } else {
    if (badChannelMask) {
      delete [] badChannelMask;
      badChannelMask=NULL;
    }
  }
  return  thisDetector->nBadFF;

}

int multiSlsDetector::exitServer(){  
  
  int retval;
  int fnum=F_EXIT_SERVER;
  
  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
  if (controlSocket) {
    controlSocket->Connect();
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
    controlSocket->Disconnect();
  }
  }
  if (retval==OK) {
    std::cout<< std::endl;
    std::cout<< "Shutting down the server" << std::endl;
    std::cout<< std::endl;
  }
  return retval;
};










  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable but leaves script unchanged, "none" disables and overwrites)
      \returns 0 if action disabled, >0 otherwise
  */
int multiSlsDetector::setAction(int iaction, string fname, string par) {

  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if (fname=="") {
      thisDetector->actionMode[iaction]=0;
    } else if (fname=="none") {
      thisDetector->actionMode[iaction]=0;
      strcpy(thisDetector->actionScript[iaction],fname.c_str());
    } else {
      strcpy(thisDetector->actionScript[iaction],fname.c_str());
      thisDetector->actionMode[iaction]=1;
    }
    
    if (par!="") {
      strcpy(thisDetector->actionParameter[iaction],par.c_str());
    }
    
    if (thisDetector->actionMode[iaction]) {

#ifdef VERBOSE
      cout << iaction << "  " << hex << (1 << iaction) << " " << thisDetector->actionMask << dec;
#endif
    
      thisDetector->actionMask |= (1 << iaction);

#ifdef VERBOSE
    cout << " set " << hex << thisDetector->actionMask << dec << endl;
#endif
    
    } else {
#ifdef VERBOSE
    cout << iaction << "  " << hex << thisDetector->actionMask << dec;
#endif
    
    thisDetector->actionMask &= ~(1 << iaction);

#ifdef VERBOSE
    cout << "  unset " << hex << thisDetector->actionMask << dec << endl;
#endif
    }
#ifdef VERBOSE
    cout << iaction << " Action mask set to " << hex << thisDetector->actionMask << dec << endl;
#endif
    
    return thisDetector->actionMode[iaction]; 
  } else
    return -1;
}


int multiSlsDetector::setActionScript(int iaction, string fname) {
#ifdef VERBOSE
  
#endif
  return setAction(iaction,fname,"");
}



int multiSlsDetector::setActionParameter(int iaction, string par) {
  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if (par!="") {
      strcpy(thisDetector->actionParameter[iaction],par.c_str());
    }
    
    if (thisDetector->actionMode[iaction]) {
      thisDetector->actionMask |= (1 << iaction);
    } else {
      thisDetector->actionMask &= ~(1 << iaction);
    }
    
    return thisDetector->actionMode[iaction]; 
  } else
    return -1; 
}

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
string multiSlsDetector::getActionScript(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) 
    return string(thisDetector->actionScript[iaction]);
  else
    return string("wrong index");
};

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
string multiSlsDetector::getActionParameter(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) 
    return string(thisDetector->actionParameter[iaction]);
  else
    return string("wrong index");
}

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
int multiSlsDetector::getActionMode(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) {
#ifdef VERBOSE
    cout << "slsDetetctor : action " << iaction << " mode is " <<  thisDetector->actionMode[iaction] << endl;
#endif
    return thisDetector->actionMode[iaction];
  } else {
#ifdef VERBOSE
    cout << "slsDetetctor : wrong action index " << iaction <<  endl;
#endif
    return -1;
  }
}


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disable)
      \returns 0 if scan disabled, >0 otherwise
  */
int multiSlsDetector::setScan(int iscan, string script, int nvalues, float *values, string par, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {

    if (script=="") {
      thisDetector->scanMode[iscan]=0;
    } else {
      strcpy(thisDetector->scanScript[iscan],script.c_str());
      if (script=="none") {
	thisDetector->scanMode[iscan]=0;
      } else if (script=="energy") {
	thisDetector->scanMode[iscan]=1;
      }  else if (script=="threshold") {
	thisDetector->scanMode[iscan]=2;
      } else if (script=="trimbits") {
	thisDetector->scanMode[iscan]=3;
      } else {
	thisDetector->scanMode[iscan]=4;
      }  
    }
    
  

    

    
    if (par!="")
      strcpy(thisDetector->scanParameter[iscan],par.c_str());
      
      if (nvalues>=0) {    
	if (nvalues==0)
	  thisDetector->scanMode[iscan]=0;
	else {
	  thisDetector->nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    thisDetector->nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values && 	thisDetector->scanMode[iscan]>0 ) {
	for (int iv=0; iv<thisDetector->nScanSteps[iscan]; iv++) {
	  thisDetector->scanSteps[iscan][iv]=values[iv];
	}
      }

      if (precision>=0)
	thisDetector->scanPrecision[iscan]=precision;
      
      if (thisDetector->scanMode[iscan]>0){
	thisDetector->actionMask |= 1<< (iscan+MAX_ACTIONS);
      } else {
	thisDetector->actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }



      setTotalProgress();










      return thisDetector->scanMode[iscan];
  }  else 
    return -1;
  
}

int multiSlsDetector::setScanScript(int iscan, string script) {
 if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (script=="") {
      thisDetector->scanMode[iscan]=0;
    } else {
      strcpy(thisDetector->scanScript[iscan],script.c_str());
      if (script=="none") {
	thisDetector->scanMode[iscan]=0;
      } else if (script=="energy") {
	thisDetector->scanMode[iscan]=1;
      }  else if (script=="threshold") {
	thisDetector->scanMode[iscan]=2;
      } else if (script=="trimbits") {
	thisDetector->scanMode[iscan]=3;
      } else {
	thisDetector->scanMode[iscan]=4;
      }  
    }
    
    if (thisDetector->scanMode[iscan]>0){
      thisDetector->actionMask |= (1 << (iscan+MAX_ACTIONS));
    } else {
      thisDetector->actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
    }

    setTotalProgress();

















    
#ifdef VERBOSE
      cout << "Action mask is " << hex << thisDetector->actionMask << dec << endl;
#endif
    return thisDetector->scanMode[iscan];
    
    
 } else 
   return -1;
 
}



int multiSlsDetector::setScanParameter(int iscan, string par) {


  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (par!="")
	strcpy(thisDetector->scanParameter[iscan],par.c_str());
      return thisDetector->scanMode[iscan];
  } else
    return -1;

}


int multiSlsDetector::setScanPrecision(int iscan, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (precision>=0)
      thisDetector->scanPrecision[iscan]=precision;
    return thisDetector->scanMode[iscan];
  } else
    return -1;

}

int multiSlsDetector::setScanSteps(int iscan, int nvalues, float *values) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
  
      if (nvalues>=0) {    
	if (nvalues==0)
	  thisDetector->scanMode[iscan]=0;
	else {
	  thisDetector->nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    thisDetector->nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values) {
	for (int iv=0; iv<thisDetector->nScanSteps[iscan]; iv++) {
	  thisDetector->scanSteps[iscan][iv]=values[iv];
	}
      }
     
      if (thisDetector->scanMode[iscan]>0){
	thisDetector->actionMask |= (1 << (iscan+MAX_ACTIONS));
      } else {
	thisDetector->actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }

#ifdef VERBOSE
      cout << "Action mask is " << hex << thisDetector->actionMask << dec << endl;
#endif
      setTotalProgress();




      return thisDetector->scanMode[iscan];
  

  } else 
      return -1;
  
  
  
  
}



  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
string multiSlsDetector::getScanScript(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (thisDetector->scanMode[iscan])
	return string(thisDetector->scanScript[iscan]);
      else
	return string("none");
  } else
      return string("wrong index");
      
};

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
string multiSlsDetector::getScanParameter(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (thisDetector->scanMode[iscan])
      return string(thisDetector->scanParameter[iscan]);
    else
      return string("none");
  }   else
      return string("wrong index");
}


   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
int multiSlsDetector::getScanMode(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS)
    return thisDetector->scanMode[iscan];
  else
    return -1;
}


   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
int multiSlsDetector::getScanSteps(int iscan, float *v) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (v) {
      for (int iv=0; iv<thisDetector->nScanSteps[iscan]; iv++) {
	v[iv]=thisDetector->scanSteps[iscan][iv];
      }
    }


    setTotalProgress();















    if (thisDetector->scanMode[iscan])
      return thisDetector->nScanSteps[iscan];
    else
      return 0;
  } else
    return -1;
}


int multiSlsDetector::getScanPrecision(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    return thisDetector->scanPrecision[iscan];
  } else
    return -1;
}



















