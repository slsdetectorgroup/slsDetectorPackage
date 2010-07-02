#include "mythenDetector.h"
#include <unistd.h>

#include <sstream>

//using namespace std;






string mythenDetector::executeLine(int narg, char *args[], int action) {


#ifdef VERBOSE
  for (int ia=0; ia<narg; ia++)
    std::cout<< args[ia] << " ";
  std::cout<<std::endl;
  std::cout<<narg << std::endl;
#endif
  

  if (action==READOUT_ACTION) {
    setThreadedProcessing(1);
    //setThreadedProcessing(0);
    setTCPSocket();
    acquire(1);
    return string("ok");
  }

  string var(args[0]);

  if (var=="data") {
    if (action==PUT_ACTION) {
      return  string("cannot set");
    } else {
      setTCPSocket();
      //acquire();
      readAll();
      processData(1);
      return string("ok");
    } 
  }
  
  if (var=="frame") {
    if (action==PUT_ACTION) {
      return  string("cannot set");
    } else {
      setTCPSocket();
      readFrame();
      processData(1);
      return string("ok");
    } 
  }
  

  if (var=="status") {
    setTCPSocket();
    if (action==PUT_ACTION) {
      setThreadedProcessing(0);
      //return  string("cannot set");
      if (string(args[1])=="start")
	startAcquisition();
      else if (string(args[1])=="stop")
	stopAcquisition();
      else
	return string("unknown action");
    } 
    runStatus s=getRunStatus();  
    switch (s) {
    case ERROR:
       return string("error");
    case  WAITING:
      return  string("waiting");
    case RUNNING:
      return string("running");
    case TRANSMITTING:
      return string("data");
    case  RUN_FINISHED:
      return string("finished");
   default:
       return string("idle");
    }
  } 

  
  


  char answer[1000];
  float  fval;
  string sval;
  int    ival;
  if (var=="free") {
    freeSharedMemory();
    return("freed");
  }  if (var=="help") {
    std::cout<< helpLine(action);
    return string("more questions? Refere to software documentation!");
  }

  if (var=="hostname") {
    if (action==PUT_ACTION) {
      setTCPSocket(args[1]);
    } 
    strcpy(answer, getHostname());
    return string(answer);
  } else if (var=="port") {
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&ival)) {
	sval="";
	setTCPSocket(sval,ival);
      }
    } 
    sprintf(answer,"%d",getControlPort());
    return string(answer);
  } else if (var=="stopport") {
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&ival)) {
	sval="";
	setTCPSocket(sval,-1,ival);
      }
    } 
    sprintf(answer,"%d",getStopPort());
    return string(answer);
    
  } else if (var=="flatfield") {
    if (action==PUT_ACTION) {
      sval=string(args[1]);
      if (sval=="none")
	sval="";
      setFlatFieldCorrection(sval);
      return string(getFlatFieldCorrectionFile());
    } else if (action==GET_ACTION) {
      if (narg>1)
	sval=string(args[1]);
      else
	sval="none";
      float corr[24*1280], ecorr[24*1280];
      if (getFlatFieldCorrection(corr,ecorr)) {
	if (sval!="none") {
	  writeDataFile(sval,corr,ecorr,NULL,'i');
	  return sval;
	} 
	return string(getFlatFieldCorrectionFile());
      } else {
	return string("none");
      }
    }
  } else if (var=="ratecorr") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%f",&fval);
      setRateCorrection(fval);
    } 
    float t;
    if (getRateCorrection(t)) {
      sprintf(answer,"%f",t);
    } else {
	sprintf(answer,"%f",0.);
    }
    return string(answer);
  } else if (var=="badchannels") {
    if (action==PUT_ACTION) {
      sval=string(args[1]);
      if (sval=="none")
	sval="";
      setBadChannelCorrection(sval);
    } else if (action==GET_ACTION) {
      if (narg>1)
	sval=string(args[1]);
      else
	sval="none";
      int bch[24*1280], nbch;
      if ((nbch=getBadChannelCorrection(bch))) {
	if (sval!="none") {  
	  ofstream outfile;
	  outfile.open (sval.c_str(),ios_base::out);
	  if (outfile.is_open()) {
	    for (int ich=0; ich<nbch; ich++) {
	      outfile << bch[ich] << std::endl;
	    }
	    outfile.close();	
	    return sval;
	  } else 
	    std::cout<< "Could not open file " << sval << " for writing " << std::endl;
	}
      } 
    }
    return string(getBadChannelCorrectionFile());
  } else if (var=="angconv") {
    if (action==PUT_ACTION) {
      sval=string(args[1]);

      if (sval=="none")
	sval="";

      setAngularConversion(sval);

      return string(getAngularConversion());
    } else if (action==GET_ACTION) {
      if (narg>1)
	sval=string(args[1]);
      else
	sval="none";
      int dir;
      if (getAngularConversion(dir)) {
	if (sval!="none") {  
	  writeAngularConversion(sval.c_str());
	  return sval;
	}
	return string(getAngularConversion());
      } else {
	return string("none");
      }
    }
  } else if (var=="globaloff") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%f",&fval);
      setGlobalOffset(fval);
    }
    sprintf(answer,"%f",getGlobalOffset());
    return string(answer);
  } else if (var=="fineoff") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%f",&fval);
      setFineOffset(fval);
    }
    sprintf(answer,"%f",getFineOffset());
    return string(answer); 
  } else if (var=="binsize") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%f",&fval);
      setBinSize(fval);
    } 
    sprintf(answer,"%f",getBinSize());
    return string(answer); 
  } else if (var=="positions") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      float pos[ival];
      for (int ip=0; ip<ival;ip++) {  
	if ((2+ip)<narg) {
	sscanf(args[2+ip],"%f",pos+ip);
#ifdef VERBOSE
	std::cout<< "Setting position " << ip <<" to " << pos[ip] <<  std::endl; 
#endif
	}
      }
      setPositions(ival,pos); 
    }
    int npos=getPositions();
    sprintf(answer,"%d",npos);
    float opos[npos];
    getPositions(opos);
    for (int ip=0; ip<npos;ip++) {
      sprintf(answer,"%s %f",answer,opos[ip]);
    }
    return string(answer);
  } 
  
  if (var=="trimdir") {
    if (action==PUT_ACTION) {
      sval=string(args[1]);
      setTrimDir(sval);
    } 
    return string(getTrimDir());
  } else if (var=="caldir") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
      setCalDir(sval);
    } 
    return string(getCalDir());
  } else if (var=="config") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
      readConfigurationFile(sval);
    } else if (action==GET_ACTION) {
     sval=string(args[1]);
     writeConfigurationFile(sval);
    }  
    return sval;
  } else if (var=="parameters") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
     retrieveDetectorSetup(sval);
    } else if (action==GET_ACTION) {
     sval=string(args[1]);
     dumpDetectorSetup(sval);
    }  
    return sval;
  } else if (var=="setup") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
     retrieveDetectorSetup(sval,2);
    } else if (action==GET_ACTION) {
     sval=string(args[1]);
     dumpDetectorSetup(sval,2);
    }  
    return sval;

  } else if (var=="outdir") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
      setFilePath(sval);
    } 
    return string(getFilePath());
  } else if (var=="fname") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
      setFileName(sval);
      
    } 
    return(getFileName());
  } else if (var=="index") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      setFileIndex(ival);
    } 
    sprintf(answer,"%d",getFileIndex());
    return string(answer);
  }  else if (var=="trimen") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      std::cout<< ival << std::endl ;
      int ene[ival];
      for (int ie=0; ie<ival; ie++) {
	std::cout<< ie << " " << args[2+ie] ;
	std::cout<< sscanf(args[2+ie],"%d",ene+ie)<< std::endl;;
      }
      setTrimEn(ival,ene);
    }
    std::cout<< " trim energies set" << std::endl;
    int nen=getTrimEn();
    std::cout<< "returned " << nen << " trim energies" << std::endl;
    sprintf(answer,"%d",nen);
    int oen[nen];
    getTrimEn(oen);
    for (int ie=0; ie<nen;ie++) {
      sprintf(answer,"%s %d",answer,oen[ie]);
    }
    return string(answer);
  } else if (var=="threaded") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      setThreadedProcessing(ival);
    }
    sprintf(answer,"%d",setThreadedProcessing());
    return string(answer);
  } else if (var=="online") {
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      setOnline(ival);
    }
    sprintf(answer,"%d",setOnline());
    return string(answer);
  }



  if (setOnline())
    setTCPSocket();
  
  if (var=="nmod") {
    if (action==PUT_ACTION) {
     sscanf(args[1],"%d",&ival);
     //setNumberOfModules(ival);
    } else
      ival=GET_FLAG;
    sprintf(answer,"%d",setNumberOfModules(ival));
    return string(answer);
  } else if (var=="maxmod") {
    if (action==PUT_ACTION) {
      return string("cannot set");
    } 
    sprintf(answer,"%d",getMaxNumberOfModules());
    return string(answer);
  } else if (var.find("extsig")==0) {
    istringstream vvstr(var.substr(7));
    vvstr >> ival;
    externalSignalFlag flag=GET_EXTERNAL_SIGNAL_FLAG, ret;
    if (action==PUT_ACTION) {
      sval=string(args[1]);
#ifdef VERBOSE
      std::cout<< "sig " << ival << " flag " << sval;
#endif
      if (sval=="off")      flag=SIGNAL_OFF;
      else if (sval=="gate_in_active_high")      flag=GATE_IN_ACTIVE_HIGH;
      else if  (sval=="gate_in_active_low") flag=GATE_IN_ACTIVE_LOW;
      else if  (sval=="trigger_in_rising_edge") flag=TRIGGER_IN_RISING_EDGE;
      else if  (sval=="trigger_in_falling_edge") flag=TRIGGER_IN_FALLING_EDGE;
      else if  (sval=="ro_trigger_in_rising_edge") flag=RO_TRIGGER_IN_RISING_EDGE;
      else if  (sval=="ro_trigger_in_falling_edge") flag=RO_TRIGGER_IN_FALLING_EDGE;
      else if (sval=="gate_out_active_high")      flag=GATE_OUT_ACTIVE_HIGH;
      else if  (sval=="gate_out_active_low") flag=GATE_OUT_ACTIVE_LOW;
      else if  (sval=="trigger_out_rising_edge") flag=TRIGGER_OUT_RISING_EDGE;
      else if  (sval=="trigger_out_falling_edge") flag=TRIGGER_OUT_FALLING_EDGE;
      else if  (sval=="ro_trigger_out_rising_edge") flag=RO_TRIGGER_OUT_RISING_EDGE;
      else if  (sval=="ro_trigger_out_falling_edge") flag=RO_TRIGGER_OUT_FALLING_EDGE;
     
      
    }
    ret= setExternalSignalFlags(flag,ival);
    switch (ret) {
    case SIGNAL_OFF:
      return string( "off");
    case GATE_IN_ACTIVE_HIGH:
      return string( "gate_in_active_high");
    case GATE_IN_ACTIVE_LOW:
      return string( "gate_in_active_low");
    case TRIGGER_IN_RISING_EDGE:
      return string( "trigger_in_rising_edge");
    case TRIGGER_IN_FALLING_EDGE:
      return string( "trigger_in_falling_edge");
    case RO_TRIGGER_IN_RISING_EDGE:
      return string( "ro_trigger_in_rising_edge");
    case RO_TRIGGER_IN_FALLING_EDGE:
      return string( "ro_trigger_in_falling_edge");
    case GATE_OUT_ACTIVE_HIGH:
      return string( "gate_out_active_high");
    case GATE_OUT_ACTIVE_LOW:
      return string( "gate_out_active_low");
    case TRIGGER_OUT_RISING_EDGE:
      return string( "trigger_out_rising_edge");
    case TRIGGER_OUT_FALLING_EDGE:
      return string( "trigger_out_falling_edge");
    case RO_TRIGGER_OUT_RISING_EDGE:
      return string( "ro_trigger_out_rising_edge");
    case RO_TRIGGER_OUT_FALLING_EDGE:
      return string( "ro_trigger_out_falling_edge");
    default:
      return string( "unknown");
    }
  }  else if (var.find("modulenumber")==0) {//else if (var=="modulenumber") {
    cout << "modulenumber" << endl;
    if (action==PUT_ACTION) {
      return string("cannot set");
    }
    istringstream vvstr(var.substr(13));
    vvstr >> ival; 
    cout << var.substr(13) << endl;
    sprintf(answer,"%llx",getId(MODULE_SERIAL_NUMBER,ival));
    return string(answer);
  } else if (var=="moduleversion") {
    if (action==PUT_ACTION) {
      return string("cannot set" );
    }   
    sprintf(answer,"%llx",getId(MODULE_FIRMWARE_VERSION));
    return string(answer);
  } else if (var=="detectornumber") {
    if (action==PUT_ACTION) {
      return string("cannot set ");
    }    
    sprintf(answer,"%llx",getId(DETECTOR_SERIAL_NUMBER));
    return string(answer);
  } else if (var=="detectorversion") {
    if (action==PUT_ACTION) {
      return string("cannot set ");
    } 
    sprintf(answer,"%llx",getId(DETECTOR_FIRMWARE_VERSION));
    return string(answer);
  } else if (var=="softwareversion") {
    if (action==PUT_ACTION) {
      return string("cannot set ");
    }
    sprintf(answer,"%llx",getId(DETECTOR_SOFTWARE_VERSION));
    return string(answer);
  } else if (var=="thisversion") {
    if (action==PUT_ACTION) {
      return string("cannot set ");
    }
    sprintf(answer,"%llx",getId(THIS_SOFTWARE_VERSION));
    return string(answer);
  } 

  else if (var.find("digitest")==0) {//else if (var=="digitest") {
    cout << "digitest" << endl;
    if (action==PUT_ACTION) {
	return string("cannot set ");
    } 
    cout << var.substr(9) << endl;
    istringstream vvstr(var.substr(9));
    vvstr >> ival;
    sprintf(answer,"%x",digitalTest(CHIP_TEST, ival));
      return string(answer);
  } else if (var=="bustest") {
    if (action==PUT_ACTION) {
      return string("cannot set ");
    } 
    sprintf(answer,"%x",digitalTest(DETECTOR_BUS_TEST));
    return string(answer);
  } else if (var=="settings") {
    detectorSettings sett=GET_SETTINGS;
    if (action==PUT_ACTION) {
     sval=string(args[1]);
	if (sval=="standard")
	  sett=STANDARD;
	else if (sval=="fast")
	  sett=FAST;
	else if (sval=="highgain")
	  sett=HIGHGAIN;
	//setSettings(sett);
      } 
    //switch (setSettings( GET_SETTINGS)) {
    switch (setSettings(sett)) {
      case STANDARD:
	return string("standard");
      case FAST:
	return string("fast");
      case HIGHGAIN:
	return string("highgain");
      default:
	return string("undefined");
      }
    } else if (var=="threshold") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%d",&ival);
	setThresholdEnergy(ival);
      } 
      sprintf(answer,"%d",getThresholdEnergy());
      return string(answer);
    } else if (var=="vthreshold") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%f",&fval);
	setDAC(fval, THRESHOLD);
      } 
      sprintf(answer,"%f",setDAC(-1,THRESHOLD));
      return string(answer);
    }
  //timers

    else if (var=="exptime") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%f",&fval);// in seconds!
	setTimer(ACQUISITION_TIME,(int64_t)(fval*1E+9));
      } 
      sprintf(answer,"%f",(float)setTimer(ACQUISITION_TIME)*1E-9);
      return string(answer);
    } else if (var=="period") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%f",&fval);// in seconds!
	setTimer(FRAME_PERIOD,(int64_t)(fval*1E+9));
      }
      sprintf(answer,"%f",(float)setTimer(FRAME_PERIOD)*1E-9);
      return string(answer);
    } else if (var=="delay") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%f",&fval);// in seconds!
	setTimer(DELAY_AFTER_TRIGGER,(int64_t)(fval*1E+9));
      } 
      sprintf(answer,"%f",(float)setTimer(DELAY_AFTER_TRIGGER)*1E-9);
      return string(answer);
      } else if (var=="gates") {
	if (action==PUT_ACTION) {
	 sscanf(args[1],"%d",&ival); 
	  setTimer( GATES_NUMBER,ival);
	} 
	
	sprintf(answer,"%lld",setTimer(GATES_NUMBER));
	return string(answer);
      } else if (var=="frames") {
	if (action==PUT_ACTION) {
	 sscanf(args[1],"%d",&ival);
	 setTimer(FRAME_NUMBER,ival);
	} 
	sprintf(answer,"%lld",setTimer(FRAME_NUMBER));
	return string(answer);
      } else if (var=="cycles") {
	if (action==PUT_ACTION) {
	 sscanf(args[1],"%d",&ival); 
	  setTimer(CYCLES_NUMBER,ival);
	} 
	sprintf(answer,"%lld",setTimer(CYCLES_NUMBER));
	return string(answer);
	
      } else if (var=="probes") {
	if (action==PUT_ACTION) {
	 sscanf(args[1],"%d",&ival); 
	  setTimer(PROBES_NUMBER,ival);
	} 
	sprintf(answer,"%lld",setTimer(PROBES_NUMBER));
	return string(answer);
      }
      else if (var=="dr") {
	if (action==PUT_ACTION) {
	 sscanf(args[1],"%d",&ival);
	  setDynamicRange(ival);
	  
	}
	  sprintf(answer,"%d",setDynamicRange());
	  return string(answer);
      } else if (var=="flags") {
	if (action==PUT_ACTION) {
	 sval=string(args[1]);
	  readOutFlags flag=GET_READOUT_FLAGS;
	  if (sval=="none")
	    flag=NORMAL_READOUT;
	  //else if (sval=="pumpprobe")
	  // flag=PUMP_PROBE_MODE;
	  else if (sval=="storeinram")
	    flag=STORE_IN_RAM;
	  else if (sval=="tot")
	    flag=TOT_MODE;
	  else if (sval=="continous")
	    flag=CONTINOUS_RO;
	  setReadOutFlags(flag);

	}
  
	switch (setReadOutFlags(GET_READOUT_FLAGS)) {
	case NORMAL_READOUT:
	  return string("none");
	case STORE_IN_RAM:
	  return string("storeinram");
	case TOT_MODE:
	  return string("tot");
	case CONTINOUS_RO:
	  return string("continous");
	default:
	  return string("unknown");
	}  
      } else if (var=="trimbits") {
	if (narg>=2) {
	  std::cout<< " writing trimfile " << std::endl;
	  int nm=setNumberOfModules(GET_FLAG,X)*setNumberOfModules(GET_FLAG,Y);
	  sls_detector_module  *myMod=NULL;
	  sval=string(args[1]);
	  
	  for (int im=0; im<nm; im++) {
	    ostringstream ostfn, oscfn;
	   //create file names
	    if (action==GET_ACTION) {   
	      ostfn << sval << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	      if ((myMod=getModule(im))) {
		writeTrimFile(ostfn.str(),*myMod);
		deleteModule(myMod);
	      }
	    } else if (action==PUT_ACTION) {
	      ostfn << sval ;
	      if (sval.find('.',sval.length()-7)<string::npos)
		ostfn << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	      myMod=readTrimFile(ostfn.str());
	      myMod->module=im;
	      setModule(*myMod);
	      deleteModule(myMod);
	    } 
	  }
	} 
	std::cout<< "Returning trimfile " << std::endl;
	return string(getTrimFile());
      }  else if (var.find("trim")==0) {
	if (action==GET_ACTION) {
	  trimMode mode=NOISE_TRIMMING;
	  int par1=0, par2=0;
	  if (var.substr(5)=="noise") {
	  // par1 is countlim; par2 is nsigma
	    mode=NOISE_TRIMMING;
	    par1=500;
	    par2=4;
	  } else if (var.substr(5)=="beam") {
	    // par1 is countlim; par2 is nsigma
	    mode=BEAM_TRIMMING;
	    par1=1000;
	    par2=4;
	  } else if (var.substr(5)=="improve") {
	    // par1 is maxit; if par2!=0 vthresh will be optimized
	    mode=IMPROVE_TRIMMING;
	    par1=5;
	    par2=0;
	  } else if (var.substr(5)=="fix") {
	    // par1 is countlim; if par2<0 then trimwithlevel else trim with median 
	    mode=FIXEDSETTINGS_TRIMMING;
	    par1=1000;
	    par2=1;
	  } else if (var.substr(5)=="offline") {
	    mode=OFFLINE_TRIMMING;
	} else {
	    return string("Unknown trim mode ")+var.substr(5);
	  } 
	  executeTrimming(mode, par1, par2);
	  sval=string(args[1]);
	  sls_detector_module  *myMod=NULL;
	  int nm=setNumberOfModules(GET_FLAG,X)*setNumberOfModules(GET_FLAG,Y);
	  for (int im=0; im<nm; im++) {
	    ostringstream ostfn, oscfn;
	    //create file names
	  ostfn << sval << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	  if ((myMod=getModule(im))) {
	    writeTrimFile(ostfn.str(),*myMod);
	    deleteModule(myMod);
	  }
	  }
      }	else if (action==PUT_ACTION) {
	  return string("cannot set ");
	}
      } else if (var=="clkdivider") {
	if (action==PUT_ACTION) {
	  sscanf(args[1],"%d",&ival);
	  setSpeed(CLOCK_DIVIDER,ival);
	} 
	sprintf(answer,"%d", setSpeed(CLOCK_DIVIDER));
	return string(answer);
      } else if (var=="setlength") {
	if (action==PUT_ACTION) {
	  sscanf(args[1],"%d",&ival);
	  setSpeed(SET_SIGNAL_LENGTH,ival);
	  
	} 
      
	sprintf(answer,"%d", setSpeed(SET_SIGNAL_LENGTH));
	return string(answer);
	
      } else if (var=="waitstates") {
	if (action==PUT_ACTION) {
	  sscanf(args[1],"%d",&ival);
	  setSpeed(WAIT_STATES,ival);
	
	} 
	sprintf(answer,"%d", setSpeed(WAIT_STATES));
	return string(answer);
	
      } else if (var=="totdivider") {
	if (action==PUT_ACTION) {
	  sscanf(args[1],"%d",&ival);
	  setSpeed(TOT_CLOCK_DIVIDER,ival);
	} 
	sprintf(answer,"%d", setSpeed(TOT_CLOCK_DIVIDER));
	return string(answer);
      } else if (var=="totdutycycle") {
	if (action==PUT_ACTION) {
	  sscanf(args[1],"%d",&ival);
	  setSpeed(TOT_DUTY_CYCLE,ival);
	} 
	sprintf(answer,"%d", setSpeed(TOT_DUTY_CYCLE));
	return string(answer);
      }
  return ("Unknown command");

}


string mythenDetector::helpLine( int action) {
  

  ostringstream os;
  
  if (action==READOUT_ACTION) {
    os << "Usage is "<< std::endl << "mythen_acquire  id " << std::endl;
    os << "where id is the id of the detector " << std::endl;
    os << "the detector will be started, the data acquired, processed and written to file according to the preferences configured " << std::endl;
  } else  if (action==PUT_ACTION) {
    os << "help \t This help " << std::endl;
    os << std::endl;
    os << "config  fname\t reads the configuration file specified and sets the values " << std::endl;
    os << std::endl;
    os << "parameters  fname\t sets the detector parameters specified in the file " << std::endl;
    os << std::endl;
    os << "setup rootname\t reads the files specfied (and that could be created by get setup) and resets the complete detector configuration including flatfield corrections, badchannels, trimbits etc. " << std::endl;
    os << std::endl;
    os << "status s \t either start or stop " << std::endl;
    os << std::endl;
    os << "hostname name \t Sets the detector hostname (or IP address) " << std::endl;
    os << std::endl;
    os << "caldir path \t Sets path of the calibration files " << std::endl;
    os << std::endl;
    os << "trimdir path \t Sets path of the trim files " << std::endl;
    os << std::endl;
    os << "trimen nen [e0 e1...en] \t sets the number of energies for which trimbit files exist and their value"<< std::endl;
    os << std::endl;
    os << "outdir \t directory to which the files will be written by default" << std::endl;
    os << std::endl;
    os <<  "fname \t filename to which the files will be written by default (to which file and position indexes will eventually be attached)" << std::endl;
    os << std::endl;
    os << "index \t start index of the files (automatically incremented by the acquisition functions)" << std::endl;
    os << std::endl;
    os << "nmod n \t Sets number of detector modules " << std::endl;
    os << std::endl;
    os << "extsig:i mode \t Sets usage of the external digital signal i. mode can be: " << std::endl;
    os << "\t off";
    os << std::endl;
    os << "\t gate_in_active_high";
    os << std::endl;
    os << "\t gate_in_active_low";
    os << std::endl;
    os << "\t trigger_in_rising_edge";
    os << std::endl;
    os << "\t trigger_in_falling_edge";
    os << std::endl;
    os << "\t ro_trigger_in_rising_edge";
    os << std::endl;
    os << "\t ro_trigger_in_falling_edge";
    os << std::endl;
    os << "\t gate_out_active_high";
    os << std::endl;
    os << "\t gate_out_active_low";
    os << std::endl;
    os << "\t trigger_out_rising_edge";
    os << std::endl;
    os << "\t trigger_out_falling_edge";
    os << std::endl;
    os << "\t ro_trigger_out_rising_edge";
    os << std::endl;
    os << "\t ro_trigger_out_falling_edge"    << std::endl;
    os << std::endl;
    os << "settings sett \t Sets detector settings. Can be: " << std::endl;
    os << "\t standard \t fast \t highgain" << std::endl;
    os << "\t depending on trheshold energy and maximum count rate: please refere to manual for limit values!"<< std::endl;
    os << std::endl;
    os << "threshold ev \t Sets detector threshold in eV. Should be half of the beam energy. It is precise only if the detector is calibrated"<< std::endl;
    os << std::endl;
    os << "exptime t \t Sets the exposure time per frame (in s)"<< std::endl;
    os << std::endl;
    os << "period t \t Sets the frames period (in s)"<< std::endl;
    os << std::endl;
    os << "delay t \t Sets the delay after trigger (in s)"<< std::endl;
    os << std::endl;
    os << "gates n \t Sets the number of gates per frame"<< std::endl;
    os << std::endl;
    os << "frames n \t Sets the number of frames per cycle (e.g. after each trigger)"<< std::endl;
    os << std::endl;
    os << "cycles n \t Sets the number of cycles (e.g. number of triggers)"<< std::endl;
    os << std::endl;
    os << "probes n \t Sets the number of probes to accumulate (max 3)"<< std::endl;
    os << std::endl;
    os << "dr n \t Sets the dynamic range - can be 1, 4, 8,16 or 24 bits"<< std::endl;
    os << std::endl;
    os << "flags mode \t Sets the readout flags - can be none or storeinram"<< std::endl;
    os << std::endl;
    os << "flatfield fname \t Sets the flatfield file name - none disable flat field corrections"<< std::endl;
    os << std::endl;
    os << "ratecorr t \t Sets the rate corrections with dead time t ns (0 unsets, -1 uses default dead time for chosen settings"<< std::endl;
    os << std::endl;
    os << "badchannels fname \t Sets the badchannels file name - none disable bad channels corrections"<< std::endl;
    os << std::endl;
    os << "angconv fname \t Sets the angular conversion file name"<< std::endl;
    os << std::endl;
    os << "globaloff o \t sets the fixed angular offset of your encoder - should be almost constant!"<< std::endl;
    os << std::endl;
    os << "fineoff o \t sets a possible angular offset of your setup - should be small but can be senseful to modify"<< std::endl;
    os << std::endl;
    os << "binsize s\t sets the binning size of the angular conversion (otherwise defaults from the angualr conversion constants)"<< std::endl;
    os << std::endl;
    os << "positions np [pos0 pos1...posnp] \t sets the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
    os << std::endl;
    os << "threaded b \t sets whether the postprocessing and file writing of the data is done in a separate thread (0 sequencial, 1 threaded). Please remeber to set the threaded mode if you acquire long real time measurements and/or use the storeinram option  otherwise you risk to lose your data"<< std::endl;
    os << std::endl;
  } else if (action==GET_ACTION) {
    os << "help \t This help " << std::endl;
    

  os << "status \t gets the detector status - can be: running, error, transmitting, finished, waiting or idle" << std::endl;
  os << "data \t gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup" << std::endl;
  os << "frame \t gets a single frame from the detector (if any) processes it and writes it to file according to the preferences already setup" << std::endl;
    os << "config  fname\t writes the configuration file" << std::endl;
    os << std::endl;
    os << "parameters  fname\t writes the main detector parameters for the measuremen tin the file " << std::endl;
    os << std::endl;
    os << "setup rootname\t writes the complete detector setup (including configuration, trimbits, flat field coefficients, badchannels etc.) is a set of files for which the extension is automatically generated " << std::endl;
    os << std::endl;
    os << "hostname \t Gets the detector hostname (or IP address) " << std::endl;
    os << std::endl;
    os << "caldir \t Gets path of the calibration files " << std::endl;
    os << std::endl;
    os << "trimdir \t Gets path of the trim files " << std::endl;
    os << std::endl;
    os << "trimen \t returns the number of energies for which trimbit files exist and their values"<< std::endl;
    os << "outdir \t directory to which the files will be written by default" << std::endl;
    os << std::endl;
      os <<  "fname \t filename to which the files will be written by default (to which file and position indexes will eventually be attached)" << std::endl;
    os << std::endl;
    os << "index \t start index of the files (automatically incremented by the acquisition functions)" << std::endl;
    os << std::endl;
    os << "nmod \t Gets number of detector modules " << std::endl;
    os << std::endl;
    os << "maxmod \t Gets maximum number of detector modules " << std::endl;
    os << std::endl;
    os << "extsig:i\t Gets usage of the external digital signal i. The return value can be: " << std::endl;
    os << "\t 0 off";
    os << std::endl;
    os << "\t 1 gate_in_active_high";
    os << std::endl;
    os << "\t 2 gate_in_active_low";
    os << std::endl;
    os << "\t 3 trigger_in_rising_edge";
    os << std::endl;
    os << "\t 4 trigger_in_falling_edge";
    os << std::endl;
    os << "\t 5 ro_trigger_in_rising_edge";
    os << std::endl;
    os << "\t 6 ro_trigger_in_falling_edge";
    os << std::endl;
    os << "\t 7 gate_out_active_high";
    os << std::endl;
    os << "\t 8 gate_out_active_low";
    os << std::endl;
    os << "\t 9 trigger_out_rising_edge";
    os << std::endl;
    os << "\t 10 trigger_out_falling_edge";
    os << std::endl;
    os << "\t 11 ro_trigger_out_rising_edge";
    os << std::endl;
    os << "\t 12 ro_trigger_out_falling_edge" << std::endl;
    os << std::endl;
    os << "modulenumber\t Gets the module serial number " << std::endl;
    os << std::endl;
    os << "moduleversion\t Gets the module version " << std::endl;
    os << std::endl;
    os << "detectornumber\t Gets the detector number (MAC address) " << std::endl;
    os << std::endl;
    os << "detectorversion\t Gets the detector firmware version " << std::endl;
    os << std::endl;
    os << "softwareversion\t Gets the detector software version " << std::endl;
    os << std::endl;
    os << "thisversion\t Gets the version of this software" << std::endl;
    os << std::endl;
    os << "digitest\t Makes a digital test of the detector. Returns 0 if it succeeds " << std::endl;
    os << std::endl;
    os << "bustest\t Makes a test of the detector bus. Returns 0 if it succeeds " << std::endl; 
    os << std::endl;
    os << "settings\t Gets detector settings. Can be: " << std::endl;
    os << "\t 0 standard \t 1 fast \t 2 highgain \t else undefined" << std::endl;
    os << std::endl;
    os << "threshold\t Gets detector threshold in eV. It is precise only if the detector is calibrated"<< std::endl;
    os << std::endl;
    os << "exptime\t Gets the exposure time per frame (in ns)"<< std::endl;
    os << std::endl;
    os << "period \t Gets the frames period (in ns)"<< std::endl;
    os << std::endl;
    os << "delay \t Gets the delay after trigger (in ns)"<< std::endl;
    os << std::endl;    
    os << "gates \t Gets the number of gates per frame"<< std::endl;
    os << std::endl;
    os << "frames \t Gets the number of frames per cycle (e.g. after each trigger)"<< std::endl;
    os << std::endl;
    os << "cycles \t Gets the number of cycles (e.g. number of triggers)"<< std::endl;
    os << std::endl;
    os << "probes \t Gets the number of probes to accumulate (max 3)"<< std::endl;
    os << std::endl;
    os << "dr \t Gets the dynamic range"<< std::endl;
    os << std::endl;
    os << "trim:mode fname \t trims the detector and writes the trimfile fname.snxx "<< std::endl;
    os << "\t mode can be:\t noise\t beam\t improve\t fix\t offline "<< std::endl;
    os << "Check that the start conditions are OK!!!"<< std::endl;
    os << std::endl;
    os << "flatfield fname \t returns wether the flat field corrections are enabled and if so writes the coefficients to the specified filename. If fname is none it is not written"<< std::endl;
    os << std::endl;
    os << "ratecorr \t returns wether teh rate corrections are enabled and what is the dead time used in ns"<< std::endl;
    os << std::endl;
    os << "badchannels fname \t returns wether the bad channels corrections are enabled and if so writes the bad channels to the specified filename. If fname is none it is not written"<< std::endl;
    os << std::endl;
    os << "angconv fname \t returns wether the angular conversion is enabled and if so writes the angular conversion coefficients to the specified filename. If fname is none, it is not written"<< std::endl;
    os << std::endl;
    os << "globaloff \t returns the fixed angular offset of your encoder - should be almost constant!"<< std::endl;
    os << std::endl;
    os << "fineoff \t returns a possible angualr offset of your setup - should be small but can be senseful to modify"<< std::endl;
    os << std::endl;
    os << "binsize \t returns the binning size of the anular conversion"<< std::endl;
    os << std::endl;
    os << "positions \t returns the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
    os << std::endl;
    os << "threaded \t gets whether the postprocessing and file writing of the data is done in a separate thread (0 sequencial, 1 threaded). Check that it is set to 1 if you acquire long real time measurements and/or use the storeinram option otherwise you risk to lose your data"<< std::endl;
    os << std::endl;


  }

   
  return os.str();
}


























  /* detector configuration (no communication to server) */

  /* 
     Every detector should have a basic configuration file containing:
     type (mythen, pilatus etc.)
     hostname
     portnumber
     communication type (default TCP/IP)
     eventually secondary portnumber (e.g. mythen stop function)
     number of modules installed if different from the detector size (x,y)
  */

int mythenDetector::readConfigurationFile(string const fname){
  
  string ans;
  string str;
  ifstream infile;
  int iargval;
  int interrupt=0;
  char *args[100];
  for (int ia=0; ia<100; ia++) {
    args[ia]=new char[1000];
  }


  string sargname, sargval;
  int iline=0;
  std::cout<< "config file name "<< fname << std::endl;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else if (str.length()<2) {
#ifdef VERBOSE
	std::cout<< "Empty line " << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //if (ssstr.good()) {
#ifdef VERBOSE 
	    std::cout<< iargval << " " << sargname  << std::endl;
#endif
	    strcpy(args[iargval],sargname.c_str());
	    iargval++;
	    //}
	}
	ans=executeLine(iargval,args,PUT_ACTION);
#ifdef VERBOSE 
	std::cout<< ans << std::endl;
#endif
      }
      iline++;
    }
    infile.close();
  } else {
    std::cout<< "Error opening configuration file " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif
  return iline;
};


int mythenDetector::writeConfigurationFile(string const fname){
  
  string names[]={\
    "hostname",\
    "caldir",\
    "trimdir",\
    "trimen",\
    "outdir",\
    "nmod",\
    "badchannels",\
    "angconv",\
    "globaloff",\
    "binsize",\
    "threaded",\
    "waitstates",\
    "setlength",\
    "clkdivider"};
  int nvar=14;
  ofstream outfile;
  int iv=0;
  char *args[100];
  for (int ia=0; ia<100; ia++) {
    args[ia]=new char[1000];
  }


  outfile.open(fname.c_str(),ios_base::out);
  if (outfile.is_open()) {
    for (iv=0; iv<nvar; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << executeLine(1,args,GET_ACTION) << std::endl;
    }
    outfile.close();
  }
  else {
    std::cout<< "Error opening configuration file " << fname << " for writing" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "wrote " <<iv << " lines to configuration file " << std::endl;
#endif
  return iv;
};
  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
int mythenDetector::dumpDetectorSetup(string fname, int level){  
  string names[]={
    "fname",\
    "index",\
    "flags",\
    "dr",\
    "settings",\
    "threshold",\
    "exptime",\
    "period",\
    "delay",\
    "gates",\
    "frames",\
    "cycles",\
    "probes",\
    "fineoff",\
    "ratecorr",\
    "flatfield",\
    "badchannels",\
    "angconv",\
    "trimbits",\
    "extsig"
  };
  int nvar=20;
  int iv=0;
  string fname1;
  ofstream outfile;
  char *args[2];
  for (int ia=0; ia<2; ia++) {
    args[ia]=new char[1000];
  }
  int nargs;
  if (level==2)
    nargs=2;
  else
    nargs=1;


  if (level==2) {
    fname1=fname+string(".config");
    writeConfigurationFile(fname1);
    fname1=fname+string(".det");
  } else
    fname1=fname;

  outfile.open(fname1.c_str(),ios_base::out);
  if (outfile.is_open()) {
    for (iv=0; iv<nvar-5; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << executeLine(1,args,GET_ACTION) << std::endl;
    }


    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".ff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".bad");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

      
    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".angoff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;
    
    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      size_t c=fname.rfind('/');
      if (c<string::npos) {
	fname1=fname.substr(0,c+1)+string("trim_")+fname.substr(c+1);
      } else {
	fname1=string("trim_")+fname;
      }
      strcpy(args[1],fname1.c_str());
#ifdef VERBOSE
      std::cout<< "writing to file " << fname1 << std::endl;
#endif
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;
    
    for (int is=0; is<4; is++) {
      sprintf(args[0],"%s:%d",names[iv].c_str(),is);
      outfile << args[0] << " " << executeLine(1,args,GET_ACTION) << std::endl;	
    }
    iv++;
    outfile.close();
  }
  else {
    std::cout<< "Error opening parameters file " << fname1 << " for writing" << std::endl;
    return FAIL;
  }
  
#ifdef VERBOSE
  std::cout<< "wrote " <<iv << " lines to  "<< fname1 << std::endl;
#endif
  return 0;
}



int mythenDetector::retrieveDetectorSetup(string fname1, int level){ 

   
   string fname;
   string str;
   ifstream infile;
   int iargval;
   int interrupt=0;
  char *args[2];
  for (int ia=0; ia<2; ia++) {
    args[ia]=new char[1000];
  }
  string sargname, sargval;
  int iline=0;
  
  if (level==2) {
    fname=fname1+string(".config");
    readConfigurationFile(fname);
    fname=fname1+string(".det");
  }  else
    fname=fname1;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //  if (ssstr.good()) {
	    strcpy(args[iargval],sargname.c_str());
#ifdef VERBOSE
      std::cout<< args[iargval]  << std::endl;
#endif
	    iargval++;
	    // }
	}
	if (level==2) {
	  executeLine(iargval,args,PUT_ACTION);
	} else {
	  if (string(args[0])==string("flatfield"))
	    ;
	  else if  (string(args[0])==string("badchannels"))
	    ;
	  else if  (string(args[0])==string("angconv"))
	    ;
	  else if (string(args[0])==string("trimbits"))
	    ;
	  else
	    executeLine(iargval,args,PUT_ACTION);
	}
      }
      iline++;
    }
    infile.close();
  } else {
    std::cout<< "Error opening  " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read  " << iline << " lines" << std::endl;
#endif
  return iline;

};




  /* I/O */


 sls_detector_module* mythenDetector::readTrimFile(string fname,  sls_detector_module *myMod){
  
   int nflag=0;

   if (myMod==NULL) {
     myMod=createModule();
     nflag=1;
   }
   string myfname;
  string str;
  ifstream infile;
  ostringstream oss;
  int iline=0;
  // string names[]={"Vtrim", "Vthresh", "Rgsh1", "Rgsh2", "Rgpr", "Vcal", "outBuffEnable"};
  string sargname;
  int ival;
  int ichan=0, ichip=0, idac=0;

 

#ifdef VERBOSE
  std::cout<<   "reading trimfile for module number "<< myMod->module << std::endl;
#endif
    //im=myMod->module;
    //myMod->module=im;
    // myMod->serialnumber=getId(MODULE_SERIAL_NUMBER, im);
    //  if (im<0)
      myfname=fname;
      /* else {
      // oss << fname << ".sn" << setfill('0') <<setw(3) << hex << myMod->serialnumber
      oss << fname << ".sn" << setfill('0') <<setw(3) << hex <<  myMod->module;
      myfname=oss.str();
      
      }*/
#ifdef VERBOSE
    std::cout<< "trim file name is "<< myfname <<   std::endl;
#endif
    infile.open(myfname.c_str(), ios_base::in);
    if (infile.is_open()) {
      // while (infile.good() && isgood==1) {
	for (int iarg=0; iarg<thisDetector->nDacs; iarg++) {
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  //  std::cout<< str << std::endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " dac nr. " << idac << " is " << ival << std::endl;
#endif
	  myMod->dacs[idac]=ival;
	  idac++;
	}
	for (ichip=0; ichip<thisDetector->nChips; ichip++) { 
	  getline(infile,str); 
	  iline++;
#ifdef VERBOSE
	  //	  std::cout<< str << std::endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  //	  std::cout<< "chip " << ichip << " " << sargname << " is " << ival << std::endl;
#endif
	 
	  myMod->chipregs[ichip]=ival;
	  for (ichan=0; ichan<thisDetector->nChans; ichan++) {
	    getline(infile,str); 
#ifdef VERBOSE
	    // std::cout<< str << std::endl;
#endif
	    istringstream ssstr(str);

#ifdef VERBOSE
	    //   std::cout<< "channel " << ichan+ichip*thisDetector->nChans <<" iline " << iline<< std::endl;
#endif
	    iline++;
	    myMod->chanregs[ichip*thisDetector->nChans+ichan]=0;
	    for (int iarg=0; iarg<6 ; iarg++) {
	      ssstr >>  ival; 
	      //if (ssstr.good()) {
	      switch (iarg) {
	      case 0:
#ifdef VERBOSE
		//		 std::cout<< "trimbits " << ival ;
#endif
		 myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival&0x3f;
		 break;
	      case 1:
#ifdef VERBOSE
		//std::cout<< " compen " << ival ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<9;
		break;
	      case 2:
#ifdef VERBOSE
		//std::cout<< " anen " << ival ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<8;
		break;
	      case 3:
#ifdef VERBOSE
		//std::cout<< " calen " << ival  ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<7;
		break;
	      case 4:
#ifdef VERBOSE
		//std::cout<< " outcomp " << ival  ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<10;
		break;
	      case 5:
#ifdef VERBOSE
		//std::cout<< " counts " << ival  << std::endl;
#endif
		 myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<11;
		 break;
	      default:
		std::cout<< " too many columns" << std::endl; 
		break;
	      }
	    }
	  }
	  //	}
      }
#ifdef VERBOSE
      std::cout<< "read " << ichan*ichip << " channels" <<std::endl; 
#endif
      infile.close();
      strcpy(thisDetector->trimFile,fname.c_str());
      return myMod;
    } else {
      std::cout<< "could not open file " << std::endl;
      if (nflag)
	deleteModule(myMod);
      return NULL;
    }
 
};


int mythenDetector::writeTrimFile(string fname, sls_detector_module mod){

  ofstream outfile;
  string names[]={"Vtrim", "Vthresh", "Rgsh1", "Rgsh2", "Rgpr", "Vcal", "outBuffEnable"};
  int iv, ichan, ichip;
  int iv1, idac;
  int nb;
    outfile.open(fname.c_str(), ios_base::out);

    if (outfile.is_open()) {
      for (idac=0; idac<mod.ndac; idac++) {
	iv=(int)mod.dacs[idac];
	outfile << names[idac] << " " << iv << std::endl;
      }
      
	for (ichip=0; ichip<mod.nchip; ichip++) {
	  iv1=mod.chipregs[ichip]&1;
	  outfile << names[idac] << " " << iv1 << std::endl;
	  for (ichan=0; ichan<thisDetector->nChans; ichan++) {
	    iv=mod.chanregs[ichip*thisDetector->nChans+ichan];
	    iv1= (iv&0x3f);
	    outfile <<iv1 << " ";
	    nb=9;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile << iv1 << " ";
	    nb=8;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile << iv1 << " ";
	    nb=7;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile <<iv1  << " ";
	    nb=10;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile << iv1 << " ";
	    nb=11;
	    iv1= ((iv&0xfffff800)>>nb);
	    outfile << iv1  << std::endl;
	  }
	}
      outfile.close();
      return OK;
    } else {
      std::cout<< "could not open trim file " << fname << std::endl;
      return FAIL;
    }

};




int mythenDetector::writeTrimFile(string fname, int imod){

  return writeTrimFile(fname,detectorModules[imod]);

};


int mythenDetector::writeDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){
  if (nch==-1)
    nch=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;


  ofstream outfile;
  int idata;
  if (data==NULL)
    return FAIL;

  //  args|=0x10; // one line per channel!

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
#ifdef VERBOSE
    std::cout<< "writeDataFile Writing to file " << fname << std::endl;
#endif
    for (int ichan=0; ichan<nch; ichan++) {
      if (ang==NULL) {
	outfile << ichan << " ";
      } else {
	outfile << ang[ichan] << " ";
      }
	
      switch (dataformat) {
      case 'f':
	outfile << *(data+ichan)<< " ";
	break;
      case 'i':
      default:
	idata=(int)(*(data+ichan));
	outfile << idata << " ";
      }
     if (err) {
       outfile << *(err+ichan)<< " ";
     }
     //   if (args&0x10) {
       outfile << std::endl;
       // }
    }

    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};







/*writes raw data file */
int mythenDetector::writeDataFile(string fname, int *data){
  ofstream outfile;
  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
#ifdef VERBOSE
    std::cout<< "Writing to file " << fname << std::endl;
#endif
    for (int ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++)
      outfile << ichan << " " << *(data+ichan) << std::endl;
    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};





int mythenDetector::readDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){


  ifstream infile;
  int ichan, iline=0;
  int interrupt=0;
  float fdata, ferr, fang;
  int maxchans;
  int ich;
  string str;

  if (nch==0)
    maxchans=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
  else if (nch<0)
    maxchans=0xfffffff;
  else
    maxchans=nch;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      if (ang==NULL) {
	ssstr >> ichan >> fdata;
	ich=ichan;
	if (ich!=iline) 
	  std::cout<< "Channel number " << ichan << " does not match with line number " << iline << " " << dataformat << std::endl;
      } else {
	ssstr >> fang >> fdata;
	ich=iline;
      }
      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
       if (err)
	 ssstr >> ferr;
      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
      if (ich<maxchans) { 
       if (ang) {
	 ang[ich]=fang;
       } 
       data[ich]=fdata; 
       if (err)
	 err[ich]=ferr;
       iline++;
     } else {
       std::cout<< " too many lines in file: "<< iline << " instead of " << maxchans << std::endl;
       interrupt=1;
       break;
     }
    }
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;





};

int mythenDetector::readDataFile(string fname, int *data){

  ifstream infile;
  int ichan, idata, iline=0;
  int interrupt=0;
  string str;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      ssstr >> ichan >> idata;
      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
      if (ichan!=iline) {
	std::cout<< " Expected channel "<< iline <<" but read channel "<< ichan << std::endl;
	interrupt=1;
	break;
      } else {
	if (iline<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods) {
	  data[iline]=idata;
	  iline++;
	} else {
	  interrupt=1;
	  break;
	}
      }
    }
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
};




int mythenDetector::readCalibrationFile(string fname, float &gain, float &offset){

  string str;
  ifstream infile;
#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    getline(infile,str);
#ifdef VERBOSE
    std::cout<< str << std::endl;
#endif
    istringstream ssstr(str);
    ssstr >> offset >> gain;
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    gain=0.;
    offset=0.;
    return -1;
  }
  return 0;
};

int mythenDetector::writeCalibrationFile(string fname, float gain, float offset){
  //std::cout<< "Function not yet implemented " << std::endl;
  ofstream outfile;

  outfile.open (fname.c_str());

  // >> i/o operations here <<
  if (outfile.is_open()) {
    outfile << offset << " " << gain << std::endl;
  } else {
    std::cout<< "Could not open calibration file "<< fname << " for writing" << std::endl;
    return -1;
  }

  outfile.close();

  return 0;
};


  /* Communication to server */



  // calibration functions
/*
  really needed?

int mythenDetector::setCalibration(int imod,  detectorSettings isettings, float gain, float offset){
  std::cout<< "function not yet implemented " << std::endl; 
  
  

  return OK;

}
int mythenDetector::getCalibration(int imod,  detectorSettings isettings, float &gain, float &offset){

  std::cout<< "function not yet implemented " << std::endl; 



}
*/

/*

int mythenDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/

//Corrections

int mythenDetector::setAngularConversion(string fname) {
  if (fname=="") {
    thisDetector->correctionMask&=~(1<< ANGULAR_CONVERSION);
    //strcpy(thisDetector->angConvFile,"none");
  } else {
    if (fname=="default")
      fname=string(thisDetector->angConvFile);
    if (readAngularConversion(fname)>=0) {
      thisDetector->correctionMask|=(1<< ANGULAR_CONVERSION);
      strcpy(thisDetector->angConvFile,fname.c_str());
    }
  }
  return thisDetector->correctionMask&(1<< ANGULAR_CONVERSION);
}

int mythenDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {
  direction=thisDetector->angDirection;
    if (angconv) {
      for (int imod=0; imod<thisDetector->nMods; imod++) {
	(angconv+imod)->center=thisDetector->angOff[imod].center;
	(angconv+imod)->r_conversion=thisDetector->angOff[imod].r_conversion;
	(angconv+imod)->offset=thisDetector->angOff[imod].offset;
	(angconv+imod)->ecenter=thisDetector->angOff[imod].ecenter;
	(angconv+imod)->er_conversion=thisDetector->angOff[imod].er_conversion;
	(angconv+imod)->eoffset=thisDetector->angOff[imod].eoffset;
      }
    }
  if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
    return 1;
  } else {
    return 0;
  }
 
}



int mythenDetector::readAngularConversion(string fname) {
  string str;
  ifstream infile;
  int mod;
  float center, ecenter;
  float r_conv, er_conv;
  float off, eoff;
  string ss;
  int interrupt=0;

  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      ssstr >> ss >> mod;
      ssstr >> ss >> center;
      ssstr >> ss >> ecenter;
      ssstr >> ss >> r_conv;
      ssstr >> ss >> er_conv;
      ssstr >> ss >> off;
      ssstr >> ss >> eoff;
      if (mod<thisDetector->nModsMax && mod>=0) {
	thisDetector->angOff[mod].center=center;
	thisDetector->angOff[mod].r_conversion=r_conv;
	thisDetector->angOff[mod].offset=off;
	thisDetector->angOff[mod].ecenter=ecenter;
	thisDetector->angOff[mod].er_conversion=er_conv;
	thisDetector->angOff[mod].eoffset=eoff;
      }
    }
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    return -1;
  }
  return 0;
}


int mythenDetector:: writeAngularConversion(string fname) {
  ofstream outfile;
  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {  
    for (int imod=0; imod<thisDetector->nMods; imod++) {
      outfile << " module " << imod << " center "<< thisDetector->angOff[imod].center<<"  +- "<< thisDetector->angOff[imod].ecenter<<" conversion "<< thisDetector->angOff[imod].r_conversion << " +- "<< thisDetector->angOff[imod].er_conversion <<  " offset "<< thisDetector->angOff[imod].offset << " +- "<< thisDetector->angOff[imod].eoffset << std::endl;
    }
    outfile.close();
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return -1;
  }
  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
  return 0;
}

int mythenDetector::resetMerging(float *mp, float *mv, float *me, int *mm) {
  
  float binsize;
  if (thisDetector->binSize>0)
    binsize=thisDetector->binSize;
  else 
    return FAIL;
  
  for (int ibin=0; ibin<(360./binsize); ibin++) {
    mp[ibin]=0;
    mv[ibin]=0;
    me[ibin]=0;
    mm[ibin]=0;
  }
  return OK;
}


int mythenDetector::finalizeMerging(float *mp, float *mv, float *me, int *mm) {
  float binsize;
  int np=0;

  if (thisDetector->binSize>0)
    binsize=thisDetector->binSize;
  else 
    return FAIL;
  
  for (int ibin=0; ibin<(360./binsize); ibin++) {
    if (mm[ibin]>0) {
      mp[np]=mp[ibin]/mm[ibin];
      mv[np]=mv[ibin]/mm[ibin];
      me[np]=me[ibin]/mm[ibin];
      me[np]=sqrt(me[ibin]);
      mm[np]=mm[ibin];
      np++;
    }
  }
  return OK;
}

int  mythenDetector::addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm) {

  float binsize;
  float binmi=-180., binma;
  int ibin=0;
  int imod;
  float ang=0;

  if (thisDetector->binSize>0)
    binsize=thisDetector->binSize;
  else 
    return FAIL;
  binmi=-180.;
  binma=binmi+binsize;
  

  if (thisDetector->angDirection>0) {
    for (int ip=0; ip<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ip++) {
      if (thisDetector->correctionMask&DISCARD_BAD_CHANNELS) {
	if (badChannelMask[ip])
	  continue;
      }
      imod=ip/(thisDetector->nChans*thisDetector->nChips);
      if (p1)
	ang=p1[ip];
      else
	ang=angle(ip,currentPosition,thisDetector->fineOffset+thisDetector->globalOffset,thisDetector->angOff[imod].r_conversion,thisDetector->angOff[imod].center, thisDetector->angOff[imod].offset,thisDetector->angOff[imod].tilt,thisDetector->angDirection);
      
      
      
      while (binma<ang) {
	ibin++;
	binmi+=binsize;
	binma+=binsize;
      }
      if (ibin<(360./binsize)) {
	mp[ibin]+=ang;
	mv[ibin]+=v1[ip];
	if (e1)
	  me[ibin]+=(e1[ip]*e1[ip]);
	else
	  me[ibin]+=v1[ip];
	mm[ibin]++;
      } else
	return FAIL;
    }
 
  } else {
    for (int ip=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods-1; ip>=0; ip--) {
      if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
	if (badChannelMask[ip])
	  continue;
      }

      while (binma<ang) {
	ibin++;
	binmi+=binsize;
	binma+=binsize;
      }
      if (ibin<(360./binsize)) {
	mp[ibin]+=ang;
	mv[ibin]+=v1[ip];
	if (e1)
	  me[ibin]+=(e1[ip]*e1[ip]);
	else
	  me[ibin]+=v1[ip];
	mm[ibin]++;
      } else
	return FAIL;
    }
  }
  return OK;
}

void  mythenDetector::acquire(int delflag){
  void *status;
#ifdef VERBOSE
  int iloop=0;
#endif
  thisDetector->progressIndex=0;

  resetFinalDataQueue();
  resetDataQueue();


  jointhread=0;
  queuesize=0;
  if (thisDetector->threadedProcessing) {
    startThread(delflag);
  }

  int np=1;
  if (thisDetector->numberOfPositions>0) 
    np=thisDetector->numberOfPositions;

  currentPositionIndex=0;

  for (int ip=0; ip<np; ip++) {
    if  (thisDetector->numberOfPositions>0) {
      go_to_position (thisDetector->detPositions[ip]);
      currentPositionIndex=ip+1;
#ifdef VERBOSE
      std::cout<< "moving to position" << std::endl;
#endif
    }
    if (thisDetector->correctionMask&(1<< I0_NORMALIZATION))
      currentI0=get_i0();
    //write header before?

    startAndReadAll();

    //write header after?
    if (thisDetector->correctionMask&(1<< I0_NORMALIZATION))
      currentI0=get_i0();
    if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION))
      currentPosition=get_position();  
   

    if (thisDetector->threadedProcessing==0)
      processData(delflag); 
   
#ifdef ACQVERBOSE
    std::cout<< "--------------------------------------------------waiting to empty raw data queue " << queuesize << std::endl ;
#endif
    //while (!dataQueue.empty()){
    while (queuesize){
      usleep(100);
// #ifdef VERBOSE
//        if (iloop%10000==0)
//  	std::cout<< "--------------------------------------------------looping raw data queue " << queuesize << std::endl ;
//        // 	//std::cout<< "--------------------------------------------------looping raw data queue " << dataQueue.size() << std::endl ;
//       iloop++;
//       //usleep(100000);
// #endif
    }
    
#ifdef ACQVERBOSE
    std::cout<< "----------------------------------------------------raw data queue is empty!" << std::endl ;
#endif
    if (thisDetector->stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (ip<(np-1))
      thisDetector->fileIndex=thisDetector->fileIndex-thisDetector->timerValue[FRAME_NUMBER]; 
#ifdef VERBOSE
    cout << "------------------------------------------------------Setting file index to " << thisDetector->fileIndex << endl;
#endif
  }

#ifdef ACQVERBOSE
  std::cout<< "------------------------------------------------------cancelling data processing thread " << std::endl ;
#endif
   if (thisDetector->threadedProcessing) { 
     jointhread=1;
     pthread_join(dataProcessingThread, &status);

     /*  while (pthread_cancel(dataProcessingThread)) {
      ;  
}*/
#ifdef ACQVERBOSE
      std::cout<< "----------------------------------------------------process canceled" << std::endl;
#endif
   }

}


void* mythenDetector::processData(int delflag) {


  int *myData;
  float *fdata;
  float *rcdata=NULL, *rcerr=NULL;
  float *ffcdata=NULL, *ffcerr=NULL;
  float *ang=NULL;
  float bs=0.004;
  int imod;
  int nb;
  int np;
  detectorData *thisData;
  int dum=1;
  //  thisDetector->progressIndex=0;

#ifdef VERBOSE
  int iloop=0;
#endif

#ifdef ACQVERBOSE
  std::cout<< " processing data - threaded mode " << thisDetector->threadedProcessing;
#endif

  //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,&outer_c_s );
  //#ifdef VERBOSE
  //	std::cout<< "impossible to cancel process " << std::endl;
  //#endif
  //    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &outer_c_s);

  while(dum | thisDetector->threadedProcessing) { // ????????????????????????
    
    
    while( !dataQueue.empty() ) {
      queuesize=dataQueue.size();

      /** Pop data queue */
      myData=dataQueue.front(); // get the data from the queue 
      //dataQueue.pop(); //remove the data from the queue
      if (myData) {
	//process data
	/** decode data */
	fdata=decodeData(myData);

      //delete [] myData;
      //	myData=NULL;
	/** write raw data file */	   
	if (thisDetector->correctionMask==0 && delflag==1) {
#ifdef ACQVERBOSE
	  std::cout<< "------------------------------------no processing "<< delflag <<std::endl;
#endif 
#ifdef VERBOSE
	  cout << "********************** no processing: writing file " << createFileName().append(".raw") << endl;
#endif 
	  writeDataFile (createFileName().append(".raw"), fdata, NULL, NULL, 'i'); 
	  delete [] fdata;
	} else {
	  writeDataFile (createFileName().append(".raw"), fdata, NULL, NULL, 'i');
#ifdef VERBOSE
	  std::cout << "********************** if (thisDetector->correctionMask!=0 || delflag==0) { writing raw data file " << createFileName() <<" " << thisDetector->correctionMask<< " " << delflag << endl;
#endif
	  /** rate correction */
	  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
	    rcdata=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods]; 
	    rcerr=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
	    rateCorrect(fdata,NULL,rcdata,rcerr);
	    delete [] fdata;
	  } else {
	    rcdata=fdata;
	    fdata=NULL;
	  }
	  
	  /** flat field correction */
	  if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
	    
	    ffcdata=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods]; 
	    ffcerr=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
	    flatFieldCorrect(rcdata,rcerr,ffcdata,ffcerr);
	    delete [] rcdata;
	    delete [] rcerr;
	  } else {
	    ffcdata=rcdata;
	    ffcerr=rcerr;
	    rcdata=NULL;
	    rcerr=NULL;
	  }
	  
	  if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
	    
	  /** angular conversion */
	  /** data merging */
	  // if (thisDetector->numberOfPositions) {
	    
	    if (currentPositionIndex<=1) {     
	      if (thisDetector->binSize>0)
		bs=thisDetector->binSize;
	      else if (thisDetector->angOff[0].r_conversion>0) {
		bs=180./PI*atan(thisDetector->angOff[0].r_conversion);
		thisDetector->binSize=bs;
	      } else
		thisDetector->binSize=bs;
	      
	      
	      nb=(int)(360./bs);
	      
	      mergingBins=new float[nb];
	      mergingCounts=new float[nb];
	      mergingErrors=new float[nb];
	      mergingMultiplicity=new int[nb];
	      
	      resetMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	    }
	    /* it would be better to create an ang0 with 0 encoder position and add to merging/write to file simply specifying that offset so that when it cycles writing the data or adding to merging it also calculates the angular position */
	    
	    ang=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods]; 
	    for (int ip=0; ip<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ip++) {
	      imod=ip/(thisDetector->nChans*thisDetector->nChips);
	      ang[ip]=angle(ip%(thisDetector->nChans*thisDetector->nChips),currentPosition,thisDetector->fineOffset+thisDetector->globalOffset,thisDetector->angOff[imod].r_conversion,thisDetector->angOff[imod].center, thisDetector->angOff[imod].offset,thisDetector->angOff[imod].tilt,thisDetector->angDirection);
	    }
	    
	    if (thisDetector->correctionMask!=0)
#ifdef VERBOSE
	      std::cout << "********************** writing dat data file  if (thisDetector->correctionMask!=0) { " << createFileName() <<" " << thisDetector->correctionMask << endl;
#endif
	    writeDataFile (createFileName().append(".dat"), ffcdata, ffcerr,ang);
	    
	    
	    addToMerging(ang, ffcdata, ffcerr, mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	    
	    
	    
	    
	    if ((currentPositionIndex==thisDetector->numberOfPositions) || (currentPositionIndex==0)) {
	      
	      np=finalizeMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	      
	      /** file writing */
	      currentPositionIndex++;
	      if (thisDetector->correctionMask!=0)
#ifdef VERBOSE
		std::cout << "********************** writing dat data file if ((currentPositionIndex==thisDetector->numberOfPositions) || (currentPositionIndex==0))" << createFileName() << endl;
#endif
		writeDataFile (createFileName().append(".dat"),mergingCounts, mergingErrors, mergingBins,'f',np);
	      if (delflag) {
		delete [] mergingBins;
		delete [] mergingCounts;
		delete [] mergingErrors;
		delete [] mergingMultiplicity;
	      } else {
		if (thisDetector->correctionMask!=0)
		  thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,thisDetector->progressIndex+1,(createFileName().append(".dat")).c_str(),np);
		else
		  thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,thisDetector->progressIndex+1,(createFileName().append(".raw")).c_str(),np);
		
		finalDataQueue.push(thisData);
#ifdef ACQVERBOSE
		std::cout<< "------------------------------------pushing final data queue " << finalDataQueue.size() << " " << createFileName() << std::endl;
#endif
	      }
	    }
	    
	    if (ffcdata)
	      delete [] ffcdata;
	    if (ffcerr)
	      delete [] ffcerr;
	    if (ang)
	      delete [] ang;
	    //}
	  } else {
	    if (thisDetector->correctionMask!=0) {
	      writeDataFile (createFileName().append(".dat"), ffcdata, ffcerr);
#ifdef VERBOSE
	      std::cout << "********************** if (thisDetector->correctionMask!=0) writing dat data file " << createFileName() << endl;
#endif
	    }
	    if (delflag) {
	      if (ffcdata)
		delete [] ffcdata;
	      if (ffcerr)
		delete [] ffcerr;
	      if (ang)
		delete [] ang;
	    } else {
	      if (thisDetector->correctionMask!=0) {
		thisData=new detectorData(ffcdata,ffcerr,NULL,thisDetector->progressIndex+1,(createFileName().append(".dat")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
	      }	      else {
		thisData=new detectorData(ffcdata,ffcerr,NULL,thisDetector->progressIndex+1,(createFileName().append(".raw")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
	      }
	      finalDataQueue.push(thisData);  
#ifdef VERBOSE
	      std::cout<< "------------------------------------pushing final data queue with " << createFileName() << " " <<finalDataQueue.size() << std::endl;
#endif 
	    }  
	  }
	}
	thisDetector->fileIndex++;
#ifdef VERBOSE
	cout << "setting file index to "<< thisDetector->fileIndex << endl;
#endif
	thisDetector->progressIndex++;
#ifdef ACQVERBOSE
	std::cout<< "------------------------------------raw data queueto be popped " << dataQueue.size() << " empty " << !dataQueue.empty() << std::endl;
#endif
	delete [] myData;
	myData=NULL;
	dataQueue.pop(); //remove the data from the queue
	queuesize=dataQueue.size();
#ifdef ACQVERBOSE
	std::cout<< "------------------------------------raw data queue popped " << dataQueue.size() << " var " << queuesize << std::endl;
#endif
      }
#ifdef ACQVERBOSE
      else
	std::cout<< "could not pop data queue " << std::endl;
#endif
      //#ifdef VERBOSE
      //	std::cout<< "possible to cancel process " << std::endl;
      //#endif
      //	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, &outer_c_s);
      //	pthread_testcancel();
    }
    
// #ifdef VERBOSE
//       if (iloop%10000==0) 
// 	std::cout<< "------------------------------------process idle " << dataQueue.size() << " empty " << !dataQueue.empty()  << std::endl;
//       iloop++;
// #endif


      if (jointhread) {
#ifdef VERBOSE
	std::cout<< "acquisition finished " << dataQueue.size() << " empty " << !dataQueue.empty()  << "final data queue size: " << finalDataQueue.size() <<std::endl;
#endif
	if (dataQueue.size()==0)
	  break;
      }
      dum=0;
  } // ????????????????????????
  return 0;
}



void mythenDetector::startThread(int delflag) {
  pthread_attr_t tattr;
  int ret;
  sched_param param, mparam;
  int policy= SCHED_OTHER;


  // set the priority; others are unchanged
  //newprio = 30;
  mparam.sched_priority =1;
  param.sched_priority =1;   


   /* Initialize and set thread detached attribute */
   pthread_attr_init(&tattr);
   pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);



  // param.sched_priority = 5;
  // scheduling parameters of main thread 
  ret = pthread_setschedparam(pthread_self(), policy, &mparam);
  //#ifdef VERBOSE
    // printf("current priority is %d\n",param.sched_priority);
  //#endif
  if (delflag)
    ret = pthread_create(&dataProcessingThread, &tattr,startProcessData, (void*)this);
  else
    ret = pthread_create(&dataProcessingThread, &tattr,startProcessDataNoDelete, (void*)this);
    
  pthread_attr_destroy(&tattr);
   // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);
  
}


void* startProcessData(void *n) {
  //void* processData(void *n) {
   mythenDetector *myDet=(mythenDetector*)n;
   myDet->processData(1);
   pthread_exit(NULL);
   
}

void* startProcessDataNoDelete(void *n) {
   //void* processData(void *n) {
  mythenDetector *myDet=(mythenDetector*)n;
  myDet->processData(0);
  pthread_exit(NULL);

}
 
runStatus mythenDetector::getRunStatus(){
  int fnum=F_GET_RUN_STATUS;
  int ret=FAIL;
  char mess[100];
  runStatus retval=ERROR;
#ifdef VERBOSE
  std::cout<< "MYTHEN Getting status "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
  if  (stopSocket->Connect()>=0) {
    stopSocket->SendDataOnly(&fnum,sizeof(fnum));
    stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      stopSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    } else {
      stopSocket->ReceiveDataOnly(&retval,sizeof(retval));   
    } 
    stopSocket->Disconnect();
  }
    }
  }
  return retval;

  
};

int64_t mythenDetector::getTimeLeft(timerIndex index){
  

  int fnum=F_GET_TIME_LEFT;
  int64_t retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "MYTHEN Getting  timer  "<< index <<  std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
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
      //thisDetector->timerValue[index]=retval;
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
