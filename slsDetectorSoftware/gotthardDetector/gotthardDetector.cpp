#include "gotthardDetector.h"
#include <unistd.h>

#include <sstream>

//using namespace std;


string gotthardDetector::executeLine(int narg, char *args[], int action) {


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
      printf("gonna start processing\n");
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
  } else if (var=="ffdir") {
    if (action==PUT_ACTION) {
      sval=string(args[1]);
      if (sval=="none")
	sval="";
      setFlatFieldCorrectionDir(sval); 
    }
    return string(getFlatFieldCorrectionDir());
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
  
  if (var=="settingsdir") {
    if (action==PUT_ACTION) {
      sval=string(args[1]);
      setSettingsDir(sval);
    } 
    return string(getSettingsDir());
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
      //std::cout<< ival << std::endl ;
      if (ival>narg-2)
	ival=narg-2;
      int ene[ival];
      for (int ie=0; ie<ival; ie++) {
	sscanf(args[2+ie],"%d",ene+ie);
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
  else if (var=="startscript") {
    ival=startScript;
    if (action==PUT_ACTION) {
      setAction(ival,args[1]);
    }
    if (getActionMode(ival)==0)
      sprintf(answer,"none");
    else
      sprintf(answer,"%s",getActionScript(ival).c_str());
    return string(answer);
  } else if (var=="startscriptpar") {
    ival=startScript;
    if (action==PUT_ACTION) {
      setActionParameter(ival,args[1]);
    }
    
    return getActionParameter(ival);


  } else if (var=="stopscript") {
    ival=stopScript;
    if (action==PUT_ACTION) {
      setAction(ival,args[1]);
    }
    if (getActionMode(ival)==0)
      sprintf(answer,"none");
    else
      sprintf(answer,"%s",getActionScript(ival).c_str());
    return string(answer);
  } else if (var=="stopscriptpar") {
    ival=stopScript;
    if (action==PUT_ACTION) {
      setActionParameter(ival,args[1]);
    }
    return getActionParameter(ival);

  } else if (var=="scriptbefore") {
    ival=scriptBefore;
    if (action==PUT_ACTION) {
      setAction(ival,args[1]);
    }
    if (getActionMode(ival)==0)
      sprintf(answer,"none");
    else
      sprintf(answer,"%s",getActionScript(ival).c_str());
    return string(answer);
  } else if (var=="scriptbeforepar") {
    ival=scriptBefore;
    if (action==PUT_ACTION) {
      setActionParameter(ival,args[1]);
    }

    return getActionParameter(ival);

  } else if (var=="scriptafter") {
    ival=scriptAfter;
    if (action==PUT_ACTION) {
      setAction(ival,args[1]);
    }
    if (getActionMode(ival)==0)
      sprintf(answer,"none");
    else
      sprintf(answer,"%s",getActionScript(ival).c_str());
    return string(answer);
  } else if (var=="scriptafterpar") {
    ival=scriptAfter;
    if (action==PUT_ACTION) {
      setActionParameter(ival,args[1]);
    }
    return getActionParameter(ival);
    


  } else if (var=="headerafter") {
    ival=headerAfter;
    if (action==PUT_ACTION) {
      setAction(ival,args[1]);
    }
    if (getActionMode(ival)==0)
      sprintf(answer,"none");
    else
      sprintf(answer,"%s", getActionScript(ival).c_str());
    return string(answer);
  } else if (var=="headerafterpar") {
    ival=headerAfter;
    if (action==PUT_ACTION) {
      setActionParameter(ival,args[1]);
    }
    return getActionParameter(ival);
    


  } else if (var=="headerbefore") {
    ival=headerBefore;
    if (action==PUT_ACTION) {
      setAction(ival,args[1]);
    }
    if (getActionMode(ival)==0)
      sprintf(answer,"none");
    else
      sprintf(answer,"%s", getActionScript(ival).c_str());
    return string(answer);
  } else if (var=="headerbeforepar") {
    ival=headerBefore;
    if (action==PUT_ACTION) {
      setActionParameter(ival,args[1]);
    }
    return getActionParameter(ival);
  } 

  else if (var=="scan0script") {
    int ind=0;
    if (action==PUT_ACTION) {
      setScanScript(ind,args[1]);
    }
    return getScanScript(ind);
  } else if (var=="scan0par") {
    int ind=0;
    if (action==PUT_ACTION) {
      setScanParameter(ind,args[1]);
    }
    return getScanParameter(ind);
  } else if (var=="scan0prec") {
    int ind=0;
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      setScanPrecision(ind,ival);
    }
    sprintf(answer,"%d",getScanPrecision(ind));
    return string(answer);
  } else if (var=="scan0steps") {
    int ind=0;
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      //cout << ival << " " << narg << endl;
      if (ival>narg-2)
	ival=narg-2;
      float ene[ival];
      for (int ie=0; ie<ival; ie++) {
	sscanf(args[2+ie],"%f",ene+ie);
      }
      setScanSteps(ind,ival,ene);
    }  
    int nen=getScanSteps(ind);
    sprintf(answer,"%d",nen);
    float oen[nen];
    getScanSteps(ind,oen);
    char form[20];
    sprintf(form,"%%s %%0.%df",getScanPrecision(ind));
    for (int ie=0; ie<nen;ie++) {
      sprintf(answer,form,answer,oen[ie]);
    }
    return string(answer);
  } 


  else if (var=="scan1script") {
    int ind=1;
    if (action==PUT_ACTION) {
      setScanScript(ind,args[1]);
    }
    return getScanScript(ind);
  } else if (var=="scan1par") {
    int ind=1;
    if (action==PUT_ACTION) {
      setScanParameter(ind,args[1]);
    }
    return getScanParameter(ind);
  } else if (var=="scan1prec") {
    int ind=1;
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      setScanPrecision(ind,ival);
    }
    sprintf(answer,"%d",getScanPrecision(ind));
    return string(answer);
  } else if (var=="scan1steps") {
    int ind=1;
    if (action==PUT_ACTION) {
      sscanf(args[1],"%d",&ival);
      if (ival>narg-2)
	ival=narg-2;
      float ene[ival];
      for (int ie=0; ie<ival; ie++) {
	sscanf(args[2+ie],"%f",ene+ie);
      }
      setScanSteps(ind,ival,ene);
    }  
    int nen=getScanSteps(ind);
    sprintf(answer,"%d",nen);
    float oen[nen];
    getScanSteps(ind,oen);
    char form[20];
    sprintf(form,"%%s %%0.%df",getScanPrecision(ind));
    for (int ie=0; ie<nen;ie++) {
      sprintf(answer,form,answer,oen[ie]);
    }
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
    setNumberOfModules(ival);
    sprintf(answer,"%d",setNumberOfModules(GET_FLAG));
    return string(answer);
  } else if (var=="maxmod") {
    if (action==PUT_ACTION) {
      return string("cannot set");
    } 
    sprintf(answer,"%d",getMaxNumberOfModules());
    return string(answer);
  } else if (var.find("extsig")==0) {
    if (var.size()<=7)
      return string("syntax is extsig:i where signal is signal number");
    istringstream vvstr(var.substr(7));
    vvstr >> ival;
    if (vvstr.fail())
      return string("syntax is extsig:i where signal is signal number");
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
    if (var.size()<=13)
      return string("syntax is modulenumber:i where is is module number");
    istringstream vvstr(var.substr(13));
    vvstr >> ival; 
    if (vvstr.fail())
      return string("syntax is modulenumber:i where is is module number");
    //cout << var.substr(13) << endl;
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
    if (var.size()<=9)
      return string("syntax is digitest:i where i is the module number");
   
  
    istringstream vvstr(var.substr(9));
    vvstr >> ival;
    if (vvstr.fail()) 
      return string("syntax is digitest:i where i is the module number");
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
	if (sval=="highgain")
	  sett=HIGHGAIN;
	else if (sval=="dynamicgain")
	  sett=DYNAMICGAIN;
	else if (sval=="gain1")
	  sett=GAIN1;
	else if (sval=="gain2")
	  sett=GAIN2;
	else if (sval=="gain3")
	  sett=GAIN3;
	else {
	  sprintf(answer,"%s not defined for this detector",sval.c_str());
	  return string(answer);
	}
      } 
    switch (setSettings(sett)) {
      case STANDARD:
	return string("standard");
      case FAST:
	return string("fast");
      case HIGHGAIN:
	return string("highgain");
      case DYNAMICGAIN:
	return string("dynamicgain");
      case GAIN1:
	return string("gain1");
      case GAIN2:
	return string("gain2");
      case GAIN3:
	return string("gain3");
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
    } else if (var=="vref_ds") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%f",&fval);
	setDAC(fval,G_VREF_DS );
      } 
      sprintf(answer,"%f",setDAC(-1,G_VREF_DS));
      return string(answer);
    } else if (var=="vcascn_pb") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval,G_VCASCN_PB );
       }
       sprintf(answer,"%f",setDAC(-1,G_VCASCN_PB));
       return string(answer);
      } else if (var=="vcascp_pb") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, G_VCASCP_PB);
       }
       sprintf(answer,"%f",setDAC(-1,G_VCASCP_PB));
      return string(answer);
      } else if (var=="vout_cm") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval,G_VOUT_CM );
       }
       sprintf(answer,"%f",setDAC(-1,G_VOUT_CM));
       return string(answer);
      } else if (var=="vcasc_out") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval,G_VCASC_OUT );
       }
       sprintf(answer,"%f",setDAC(-1,G_VCASC_OUT));
       return string(answer);
      } else if (var=="vin_cm") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval,G_VIN_CM );
       }
       sprintf(answer,"%f",setDAC(-1,G_VIN_CM));
       return string(answer);
      } else if (var=="vref_comp") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval,G_VREF_COMP);
      }
      sprintf(answer,"%f",setDAC(-1,G_VREF_COMP));
       return string(answer);
      } else if (var=="ib_test_c") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval,G_IB_TESTC );
       }
       sprintf(answer,"%f",setDAC(-1,G_IB_TESTC));
       return string(answer);
      }

  //////////////////////////////////////////////////////////////////////enter new stuff here if required
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

  else if (var=="exptimel") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",(float)getTimeLeft(ACQUISITION_TIME)*1E-9);
      return string(answer);
    } else if (var=="periodl") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",(float)getTimeLeft(FRAME_PERIOD)*1E-9);
      return string(answer);
    } else if (var=="delayl") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",(float)getTimeLeft(DELAY_AFTER_TRIGGER)*1E-9);
      return string(answer);
    } else if (var=="gatesl") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",(float)getTimeLeft(GATES_NUMBER));
      return string(answer);
    } else if (var=="framesl") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",(float)getTimeLeft(FRAME_NUMBER)+2);
      return string(answer);
    } else if (var=="cyclesl") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",(float)getTimeLeft(CYCLES_NUMBER)+2);
      return string(answer);
    } else if (var=="progress") {
      if (action==PUT_ACTION) {
	setTotalProgress();
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%f",getCurrentProgress());
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
      } 
  /*else if (var=="settingsbits") {
	if (narg>=2) {
	  int nm=setNumberOfModules(GET_FLAG,X)*setNumberOfModules(GET_FLAG,Y);
	  sls_detector_module  *myMod=NULL;
	  sval=string(args[1]);
	  std::cout<< " settingsfile " << sval << std::endl;
	  
	  for (int im=0; im<nm; im++) {
	    ostringstream ostfn, oscfn;
	   //create file names
	    if (action==GET_ACTION) {   
	      ostfn << sval << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	      if ((myMod=getModule(im))) {
		writeSettingsFile(ostfn.str(),*myMod);
		deleteModule(myMod);
	      }
	    } else if (action==PUT_ACTION) {
	      ostfn << sval ;
	      if (sval.find('.',sval.length()-7)<string::npos)
		ostfn << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	      myMod=readSettingsFile(ostfn.str());
	      if (myMod) {
		myMod->module=im;
		setModule(*myMod);
		deleteModule(myMod);
	      } //else		cout << "myMod NULL" << endl; 
	    } 
	  }
	} 
	std::cout<< "Returning settingsfile " << std::endl;
	return string(getSettingsFile());
      }  else if (var.find("trim")==0) {
	if (action==GET_ACTION) {
	  trimMode mode=NOISE_TRIMMING;
	  int par1=0, par2=0;
	  if (var.size()<=5)
	    return string("trim:mode fname");

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
	    writeSettingsFile(ostfn.str(),*myMod);
	    deleteModule(myMod);
	  }
	  }
      }	else if (action==PUT_ACTION) {
	  return string("cannot set ");
	}
      }
*/ else if (var=="clkdivider") {
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


string gotthardDetector::helpLine( int action) {
  

  ostringstream os;
  
  if (action==READOUT_ACTION) {
    os << "Usage is "<< std::endl << "gotthard_acquire  id " << std::endl;
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
    os << "settingsdir path \t Sets path of the settings files " << std::endl;
    os << std::endl;
    os << "trimen nen [e0 e1...en] \t sets the number of energies for which trimbit files exist and their value"<< std::endl;
    os << std::endl;








    os << "startscript script \t sets script to execute at the beginning of the measurements - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex par=startscriptpar"<< std::endl;
    os << std::endl;
    os << "startscriptpar par \t sets start script parameter (see startscript)"<< std::endl;
    os << std::endl;

    os << "scan0script script \t sets script to launch at level 0 scan. If \"energy\" energy scan, if \"threshold\" threshold scan, if \"trimbits\" trimbits scan, \"none\" unsets otherwise will be launched as a system call with arguments nrun=fileindex fn=filename var=scan0var par=scan0par"<< std::endl;
    os << std::endl;
    os << "scan0par par\t sets the level 0 scan parameter. See scan0script"<< std::endl;
    os << std::endl;
    os << "scan0prec n \t sets the level 0 scan precision for the output file name. See scan0script"<< std::endl;
    os << std::endl;
    os << "scan0steps nsteps [s0 s1...] \t sets the level 0 scan steps. See scan0script - nsteps=0 unsets the scan level"<< std::endl;
    os << std::endl;


    os << "scan1script script \t sets script to launch at level 1 scan. If \"energy\" energy scan, if \"threshold\" threshold scan, if \"trimbits\" trimbits scan, \"none\" unsets otherwise will be launched as a system call with arguments nrun=fileindex fn=filename var=scan1var par=scan1par"<< std::endl;
    os << std::endl;

    os << "scan1par par\t sets the level 1 scan parameter. See scan1script"<< std::endl;
    os << std::endl;
    os << "scan1prec n \t sets the level 1 scan precision for the output file name. See scan1script"<< std::endl;
    os << std::endl;
    os << "scan1steps nsteps [s0 s1...] \t sets the level 1 scan steps. See scan1script - nsteps=0 unsets the scan level"<< std::endl;
    os << std::endl;


    os << "scriptbefore script \t sets script to execute at the beginning of the realtime acquisition - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename par=scriptbeforepar sv0=scan0var sv1=scan1var p0=scan0par p1=scan1par"<< std::endl;
    os << std::endl;
    os << "scriptbeforepar \t sets  script before parameter (see scriptbefore)"<< std::endl;
    os << std::endl;

    os << "headerbefore script \n script to launch to acquire the headerfile just before  the acquisition -  \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename acqtime=t gainmode=sett threshold=thr badfile=badf angfile=angf bloffset=bloffset fineoffset=fineoffset fffile=ffile tau=taucorr par=headerbeforepar"<< std::endl;
    os << std::endl;
    os << "headerbeforepar \t sets header before parameter (see headerbefore)"<< std::endl;
    os << std::endl;


    os << "headerafter script \n script to launch to acquire the headerfile just after the acquisition -  \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename acqtime=t gainmode=sett threshold=thr badfile=badf angfile=angf bloffset=bloffset fineoffset=fineoffset fffile=ffile tau=taucorr par=headerafterpar"<< std::endl;
    os << std::endl;
    os << "headerafterpar par \t sets header after parameter (see headerafter)"<< std::endl;
    os << std::endl;

    os << "scriptafter script \t sets script to execute at the end of the realtime acquisition - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename par=scriptafterpar sv0=scan0var sv1=scan1var p0=scan0par p1=scan1par"<< std::endl;
    os << std::endl;
    os << "scriptafterpar par \t sets  script after parameter (see scriptafter)"<< std::endl;
    os << std::endl;


    os << "stopscript script \t sets script to execute at the end of the measurements - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex par=stopscriptpar"<< std::endl;
    os << std::endl;
    os << "stopscriptpar par\t sets stop script parameter (see stopscript)"<< std::endl;
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
    os << "\t standard \t fast \t highgain \t dynamicgain \t gain1 \t gain2 \t gain3" << std::endl;
    os << "\t depending on trheshold energy and maximum count rate: please refere to manual for limit values!"<< std::endl;
    os << std::endl;
    os << "threshold ev \t Sets detector threshold in eV. Should be half of the beam energy. It is precise only if the detector is calibrated"<< std::endl;
    os << std::endl;
    os << "vthreshold dacu\t sets the detector threshold in dac units (0-1024). The energy is approx 800-15*keV" << std::endl;
    os << std::endl;

    os << "vcalibration " << "dacu\t sets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
    os << std::endl;
      os << "vtrimbit " << "dacu\t sets the trimbit amplitude in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vpreamp " << "dacu\t sets the preamp feedback voltage in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vshaper1 " << "dacu\t sets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vshaper2 " << "dacu\t sets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vhighvoltage " << "dacu\t CHIPTEST BOARD ONLY - sets the detector HV in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vapower " << "dacu\t CHIPTEST BOARD ONLY - sets the analog power supply in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vddpower " << "dacu\t CHIPTEST BOARD ONLY - sets the digital power supply in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vshpower " << "dacu\t CHIPTEST BOARD ONLY - sets the comparator power supply in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "viopower " << "dacu\t CHIPTEST BOARD ONLY - sets the FPGA I/O power supply in dac units (0-1024)." << std::endl;

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
    os << "ffdir dir \t Sets the default directory where the flat field are located"<< std::endl;
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
    os << "online  b\t sets the detector in online (1) or offline (0) state " << std::endl;
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
    os << "setup rootname\t writes the complete detector setup (including configuration, trimbits, flat field coefficients, badchannels etc.) in a set of files for which the extension is automatically generated " << std::endl;
    os << std::endl;
    os << "hostname \t Gets the detector hostname (or IP address) " << std::endl;
    os << std::endl;
    os << "caldir \t Gets path of the calibration files " << std::endl;
    os << std::endl;
    os << "settingsdir \t Gets path of the settings files " << std::endl;
    os << std::endl;
    os << "trimen \t returns the number of energies for which trimbit files exist and their values"<< std::endl;






    os << "startscript \t gets script to execute at the beginning of the measurements - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex par=startscriptpar"<< std::endl;
    os << std::endl;
    os << "startscriptpar \t gets start script parameter (see startscript)"<< std::endl;
    os << std::endl;

    os << "scan0script \t gets script to launch at level 0 scan. If \"energy\" energy scan, if \"threshold\" threshold scan, if \"trimbits\" trimbits scan, \"none\" unsets otherwise will be launched as a system call with arguments nrun=fileindex fn=filename var=scan0var par=scan0par"<< std::endl;
    os << std::endl;
    os << "scan0par \t gets the level 0 scan parameter. See scan0script"<< std::endl;
    os << std::endl;
    os << "scan0prec \t gets the level 0 scan precision for the output file name. See scan0script"<< std::endl;
    os << std::endl;
    os << "scan0steps \t gets the level 0 scan steps. See scan0script - nsteps=0 unsets the scan level"<< std::endl;
    os << std::endl;


    os << "scan1script \t gets script to launch at level 1 scan. If \"energy\" energy scan, if \"threshold\" threshold scan, if \"trimbits\" trimbits scan, \"none\" unsets otherwise will be launched as a system call with arguments nrun=fileindex fn=filename var=scan1var par=scan1par"<< std::endl;
    os << std::endl;

    os << "scan1par \t gets the level 1 scan parameter. See scan1script"<< std::endl;
    os << std::endl;
    os << "scan1prec \t gets the level 1 scan precision for the output file name. See scan1script"<< std::endl;
    os << std::endl;
    os << "scan1steps \t gets the level 1 scan steps. See scan1script - nsteps=0 unsets the scan level"<< std::endl;
    os << std::endl;


    os << "scriptbefore \t gets script to execute at the beginning of the realtime acquisition - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename par=scriptbeforepar sv0=scan0var sv1=scan1var p0=scan0par p1=scan1par"<< std::endl;
    os << std::endl;
    os << "scriptbeforepar \t gets  script before parameter (see scriptbefore)"<< std::endl;
    os << std::endl;

    os << "headerbefore \n gets the script to launch to acquire the headerfile just before  the acquisition -  \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename acqtime=t gainmode=sett threshold=thr badfile=badf angfile=angf bloffset=bloffset fineoffset=fineoffset fffile=ffile tau=taucorr par=headerbeforepar"<< std::endl;
    os << std::endl;
    os << "headerbeforepar \t gets header before parameter (see headerbefore)"<< std::endl;
    os << std::endl;


    os << "headerafter \n gets the script to launch to acquire the headerfile just after the acquisition -  \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename acqtime=t gainmode=sett threshold=thr badfile=badf angfile=angf bloffset=bloffset fineoffset=fineoffset fffile=ffile tau=taucorr par=headerafterpar"<< std::endl;
    os << std::endl;
    os << "headerafterpar \t gets header after parameter (see headerafter)"<< std::endl;
    os << std::endl;

    os << "scriptafter \t gets script to execute at the end of the realtime acquisition - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex fn=filename par=scriptafterpar sv0=scan0var sv1=scan1var p0=scan0par p1=scan1par"<< std::endl;
    os << std::endl;
    os << "scriptafterpar \t gets  script after parameter (see scriptafter)"<< std::endl;
    os << std::endl;


    os << "stopscript \t gets script to execute at the end of the measurements - \"none\" to unset - will be launched as a system call with arguments nrun=fileindex par=stopscriptpar"<< std::endl;
    os << std::endl;
    os << "stopscriptpar\t gets stop script parameter (see stopscript)"<< std::endl;
    os << std::endl;





















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
    os << "modulenumber:i \t Gets the serial number of module i" << std::endl;
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
    os << "digitest:i\t Makes a digital test of the detector module i. Returns 0 if it succeeds " << std::endl;
    os << std::endl;
    os << "bustest\t Makes a test of the detector bus. Returns 0 if it succeeds " << std::endl; 
    os << std::endl;
    os << "settings\t Gets detector settings. Can be: " << std::endl;
    os << "\t 0 standard \t 1 fast \t 2 highgain \t dynamicgain \t gain1 \t gain2 \t gain3 \t else undefined" << std::endl;
    os << std::endl;
    os << "threshold\t Gets detector threshold in eV. It is precise only if the detector is calibrated"<< std::endl;
    os << std::endl;
    os << "vthreshold \t Gets the detector threshold in dac units (0-1024). The energy is approx 800-15*keV" << std::endl;
    os << std::endl;

    os << "vcalibration " << "dacu\t gets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
    os << std::endl;
      os << "vtrimbit " << "dacu\t gets the trimbit amplitude in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vpreamp " << "dacu\t gets the preamp feedback voltage in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vshaper1 " << "dacu\t gets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vshaper2 " << "dacu\t gets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vhighvoltage " << "dacu\t CHIPTEST BOARD ONLY - gets the detector HV in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vapower " << "dacu\t CHIPTEST BOARD ONLY - gets the analog power supply in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vddpower " << "dacu\t CHIPTEST BOARD ONLY - gets the digital power supply in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "vshpower " << "dacu\t CHIPTEST BOARD ONLY - gets the comparator power supply in dac units (0-1024)." << std::endl;
    os << std::endl;
  os << "viopower " << "dacu\t CHIPTEST BOARD ONLY - gets the FPGA I/O power supply in dac units (0-1024)." << std::endl;
    os << std::endl;


    os << "exptime\t Gets the exposure time per frame (in s)"<< std::endl;
    os << std::endl;
    os << "period \t Gets the frames period (in s)"<< std::endl;
    os << std::endl;
    os << "delay \t Gets the delay after trigger (in s)"<< std::endl;
    os << std::endl;    
    os << "gates \t Gets the number of gates per frame"<< std::endl;
    os << std::endl;
    os << "frames \t Gets the number of frames per cycle (e.g. after each trigger)"<< std::endl;
    os << std::endl;
    os << "cycles \t Gets the number of cycles (e.g. number of triggers)"<< std::endl;
    os << std::endl;
    os << "probes \t Gets the number of probes to accumulate (max 3)"<< std::endl;




    os << "exptimel\t Gets the exposure time left in the current frame (in s)"<< std::endl;
    os << std::endl;
    os << "periodl \t Gets the period left in the current frame (in s)"<< std::endl;
    os << std::endl;
    os << "delayl \t Gets the delay after current trigger left (in s)"<< std::endl;
    os << std::endl;    
    os << "gatesl \t Gets the number of gates left in the current frame"<< std::endl;
    os << std::endl;
    os << "framesl \t Gets the number of frames left (after the current trigger)"<< std::endl;
    os << std::endl;
    os << "cyclesl \t Gets the number of cycles left (e.g. number of triggers)"<< std::endl;
    //os << std::endl;
    //os << "progress \t Gets acquisition progress - to be implemented"<< std::endl;







    os << std::endl;
    os << "dr \t Gets the dynamic range"<< std::endl;
    os << std::endl;
    os << "trim:mode fname \t trims the detector and writes the trimfile fname.snxx "<< std::endl;
    os << "\t mode can be:\t noise\t beam\t improve\t fix\t offline "<< std::endl;
    os << "Check that the start conditions are OK!!!"<< std::endl;
    os << std::endl;
    os << "ffdir \t Returns the default directory where the flat field are located"<< std::endl;
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
    os << "online  \t gets the detector online (1) or offline (0) state " << std::endl;
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

int gotthardDetector::readConfigurationFile(string const fname){
  
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


int gotthardDetector::writeConfigurationFile(string const fname){
  
  string names[]={\
    "hostname",\
    "caldir",\
    "settingsdir",\
    "trimen",\
    "outdir",\
    "ffdir",\
    "headerbefore",\
    "headerafter",\
    "headerbeforepar",\
    "headerafterpar",\
    "nmod",\
    "badchannels",\
    "angconv",\
    "globaloff",\
    "binsize",\
    "threaded",\
    "waitstates",\
    "setlength",\
    "clkdivider"};
  int nvar=19;
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
int gotthardDetector::dumpDetectorSetup(string fname, int level){  
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
    "startscript",\
    "startscriptpar",\
    "stopscript",\
    "stopscriptpar",\
    "scriptbefore",\
    "scriptbeforepar",\
    "scriptafter",\
    "scriptafterpar",\
    "headerbefore",\
    "headerbeforepar",\
    "headerafter",\
    "headerafterpar",\
    "scan0script",\
    "scan0par",\
    "scan0prec",\
    "scan0steps",\
    "scan1script",\
    "scan1par",\
    "scan1prec",\
    "scan1steps",\
    "flatfield",\
    "badchannels",\
    "angconv",\
    "trimbits",\
    "extsig"
  };
  int nvar=40;
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



int gotthardDetector::retrieveDetectorSetup(string fname1, int level){ 

   
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
    //cout << "config file read" << endl;
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


 sls_detector_module* gotthardDetector::readSettingsFile(string fname,  sls_detector_module *myMod){
 
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
  //   string names[]={"Vref", "VcascN","VcascP", "Vout", "Vcasc", "Vin", "Vref_comp", "Vib_test", "config", "HV"};
  string sargname;
  int ival;
  int ichan=0, ichip=0, idac=0;

 

#ifdef VERBOSE
  std::cout<<   "reading settings file for module number "<< myMod->module << std::endl;
#endif
      myfname=fname;
#ifdef VERBOSE
    std::cout<< "settings file name is "<< myfname <<   std::endl;
#endif
    infile.open(myfname.c_str(), ios_base::in);
    if (infile.is_open()) {
	for (int iarg=0; iarg<thisDetector->nDacs; iarg++) {
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " dac nr. " << idac << " is " << ival << std::endl;
#endif
	  myMod->dacs[idac]=ival;
	  idac++;
	}

	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " (config) is " << ival << std::endl;
#endif
	   int configval = ival;//myMod->dacs[idac]=ival;


	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  istringstream sstr(str);
	  sstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " (HV) is " << ival << std::endl;
#endif
	   int HVval = ival;//myMod->dacs[idac]=ival;


      infile.close();
      strcpy(thisDetector->settingsFile,fname.c_str());
      return myMod;
    } else {
      std::cout<< "could not open settings file " <<  myfname << std::endl;
      if (nflag)
	deleteModule(myMod);
      return NULL;
    }
 
};


int gotthardDetector::writeSettingsFile(string fname, sls_detector_module mod){
  
  ofstream outfile;
  string names[]={"Vref", "VcascN","VcascP", "Vout", "Vcasc", "Vin", "Vref_comp", "Vib_test", "config", "HV"};
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




int gotthardDetector::writeSettingsFile(string fname, int imod){

  return writeSettingsFile(fname,detectorModules[imod]);

};


int gotthardDetector::writeDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){

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
int gotthardDetector::writeDataFile(string fname, int *data){

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





int gotthardDetector::readDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){


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
	if (!ssstr.good()) {
	  interrupt=1;
	  break;
	}
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

int gotthardDetector::readDataFile(string fname, int *data){
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




int gotthardDetector::readCalibrationFile(string fname, float &gain, float &offset){

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

int gotthardDetector::writeCalibrationFile(string fname, float gain, float offset){

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

int gotthardDetector::setCalibration(int imod,  detectorSettings isettings, float gain, float offset){
  std::cout<< "function not yet implemented " << std::endl; 
  
  

  return OK;

}
int gotthardDetector::getCalibration(int imod,  detectorSettings isettings, float &gain, float &offset){

  std::cout<< "function not yet implemented " << std::endl; 



}
*/

/*

int gotthardDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/

//Corrections

int gotthardDetector::setAngularConversion(string fname) {

  if (fname=="") {
    thisDetector->correctionMask&=~(1<< ANGULAR_CONVERSION);
    //strcpy(thisDetector->angConvFile,"none");
    //#ifdef VERBOSE
     std::cout << "Unsetting angular conversion" <<  std::endl;
    //#endif
  } else {
    if (fname=="default") {
      fname=string(thisDetector->angConvFile);
    }
    
    //#ifdef VERBOSE
     std::cout << "Setting angular conversion to" << fname << std:: endl;
    //#endif
    if (readAngularConversion(fname)>=0) {
      thisDetector->correctionMask|=(1<< ANGULAR_CONVERSION);
      strcpy(thisDetector->angConvFile,fname.c_str());
    }
  }
  return thisDetector->correctionMask&(1<< ANGULAR_CONVERSION);

}







int gotthardDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {

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



int gotthardDetector::readAngularConversion(string fname) {
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


int gotthardDetector:: writeAngularConversion(string fname) {

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
}

int gotthardDetector::resetMerging(float *mp, float *mv, float *me, int *mm) {
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


int gotthardDetector::finalizeMerging(float *mp, float *mv, float *me, int *mm) {
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

int  gotthardDetector::addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm) {
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

void  gotthardDetector::acquire(int delflag){

  void *status;
  //#ifdef VERBOSE
  //int iloop=0;
  //#endif
  int trimbit;
  int startindex=thisDetector->fileIndex;
  int lastindex=startindex;
  char cmd[MAX_STR_LENGTH];
  //string sett;

  thisDetector->progressIndex=0;
  thisDetector->stoppedFlag=0;



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

  int ns0=1;
  if (thisDetector->actionMask & (1 << MAX_ACTIONS)) {
    ns0=thisDetector->nScanSteps[0];
  }
  if (ns0<1)
    ns0=1;


  int ns1=1;
  if (thisDetector->actionMask & (1 << (MAX_ACTIONS+1))) {
    ns1=thisDetector->nScanSteps[1];
  }
  if (ns1<1)
    ns1=1;




  //action at start
  if (thisDetector->stoppedFlag==0) {
      if (thisDetector->actionMask & (1 << startScript)) {
	//"Custom start script. The arguments are passed as nrun=n par=p.");
	sprintf(cmd,"%s nrun=%d par=%s",thisDetector->actionScript[startScript],thisDetector->fileIndex,thisDetector->actionParameter[startScript]);
#ifdef VERBOSE
	cout << "Executing start script " << cmd << endl;
#endif
	system(cmd);
      }
  }

  for (int is0=0; is0<ns0; is0++) {//scan0 loop

  if (thisDetector->stoppedFlag==0) {

    currentScanVariable[0]=thisDetector->scanSteps[0][is0];
    currentScanIndex[0]=is0;

    switch(thisDetector->scanMode[0]) {
    case 1:
      setThresholdEnergy((int)currentScanVariable[0]); //energy scan
      break;
    case 2:
      setDAC(currentScanVariable[0],THRESHOLD); // threshold scan
      break;
    case 3:
      trimbit=(int)currentScanVariable[0];
      setChannel((trimbit<<((int)TRIMBIT_OFF))|((int)COMPARATOR_ENABLE)); // trimbit scan
      break;
    case 0:
       currentScanVariable[0]=0;
       break;
    default:
    //Custom scan script level 0. The arguments are passed as nrun=n fn=filename var=v par=p"
      sprintf(cmd,"%s nrun=%d fn=%s var=%f par=%s",thisDetector->scanScript[0],thisDetector->fileIndex,createFileName().c_str(),currentScanVariable[0],thisDetector->scanParameter[0]);
#ifdef VERBOSE
      cout << "Executing scan script 0 " << cmd << endl;
#endif
      system(cmd);
     

    }
  } else
    break;
  

  for (int is1=0; is1<ns1; is1++) {//scan0 loop

     if (thisDetector->stoppedFlag==0) {

    currentScanVariable[1]=thisDetector->scanSteps[1][is1];
    currentScanIndex[1]=is1;

    switch(thisDetector->scanMode[1]) {
    case 1:
      setThresholdEnergy((int)currentScanVariable[1]); //energy scan
      break;
    case 2:
      setDAC(currentScanVariable[1],THRESHOLD); // threshold scan
      break;
    case 3:
      trimbit=(int)currentScanVariable[1];
      setChannel((trimbit<<((int)TRIMBIT_OFF))|((int)COMPARATOR_ENABLE)); // trimbit scan
      break;
    case 0:
       currentScanVariable[1]=0;
       break;
    default:
    //Custom scan script level 1. The arguments are passed as nrun=n fn=filename var=v par=p"
      sprintf(cmd,"%s nrun=%d fn=%s var=%f par=%s",thisDetector->scanScript[1],thisDetector->fileIndex,createFileName().c_str(),currentScanVariable[1],thisDetector->scanParameter[1]);
#ifdef VERBOSE
      cout << "Executing scan script 1 " << cmd << endl;
#endif
      system(cmd);
    }
  
     } else
       break;

     if (thisDetector->stoppedFlag==0) {
       if (thisDetector->actionMask & (1 << scriptBefore)) {
	 //Custom script before each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	 sprintf(cmd,"%s nrun=%d fn=%s par=%s sv0=%f sv1=%f p0=%s p1=%s",thisDetector->actionScript[scriptBefore],thisDetector->fileIndex,createFileName().c_str(),thisDetector->actionParameter[scriptBefore],currentScanVariable[0],currentScanVariable[1],thisDetector->scanParameter[0],thisDetector->scanParameter[1]);
#ifdef VERBOSE
	 cout << "Executing script before " << cmd << endl;
#endif
	 system(cmd);
       }
     } else
       break;

     currentPositionIndex=0;
     
     for (int ip=0; ip<np; ip++) {
       if (thisDetector->stoppedFlag==0) {
	 if  (thisDetector->numberOfPositions>0) {
	   go_to_position (thisDetector->detPositions[ip]);
	   currentPositionIndex=ip+1;
#ifdef VERBOSE
	   std::cout<< "moving to position" << std::endl;
#endif
	 } 
       } else
	 break;
       
	 //write header before?
	 //cmd=headerBeforeScript;
	 //Custom script to write the header. \n The arguments will be passed as nrun=n fn=filenam acqtime=t gainmode=g threshold=thr badfile=badf angfile=angf bloffset=blo fineoffset=fo fffile=fffn tau=deadtau par=p")
       
       if (thisDetector->stoppedFlag==0) {
	 if (thisDetector->correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0();
	 if (thisDetector->actionMask & (1 << headerBefore)) {
	   //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	   sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s",thisDetector->actionScript[headerBefore],thisDetector->fileIndex,createFileName().c_str(),((float)thisDetector->timerValue[ACQUISITION_TIME])*1E-9, thisDetector->currentSettings, thisDetector->currentThresholdEV, getBadChannelCorrectionFile().c_str(), getAngularConversion().c_str(), thisDetector->globalOffset, thisDetector->fineOffset,getFlatFieldCorrectionDir(),getFlatFieldCorrectionFile(), getRateCorrectionTau(), thisDetector->actionParameter[headerBefore]);
#ifdef VERBOSE
	   cout << "Executing header after " << cmd << endl;
#endif
	   system(cmd);
	 }
       } else
	 break;
       
       
       
       if (thisDetector->stoppedFlag==0) {
	 startAndReadAll();
	 
	 if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION))
	   currentPosition=get_position();  
	 
	 if (thisDetector->correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0()-currentI0;
	 
	 
	 if (thisDetector->threadedProcessing==0)
	   processData(delflag); 
	 
	 
       } else
	 break;
       
       //while (!dataQueue.empty()){
       while (queuesize){
	 usleep(1000);
       }
    

    if (thisDetector->fileIndex>lastindex)
      lastindex=thisDetector->fileIndex;

    if (thisDetector->stoppedFlag==0) {
      if (thisDetector->actionMask & (1 << headerAfter)) {
	//Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s", \
		thisDetector->actionScript[headerAfter],		\
	      thisDetector->fileIndex,\
		createFileName().c_str(),				\
		((float)thisDetector->timerValue[ACQUISITION_TIME])*1E-9, \
		thisDetector->currentSettings,				\
		thisDetector->currentThresholdEV,			\
		getBadChannelCorrectionFile().c_str(),			\
		getAngularConversion().c_str(),				\
		thisDetector->globalOffset,				\
		thisDetector->fineOffset,				\
		getFlatFieldCorrectionDir(),				\
		getFlatFieldCorrectionFile(),				\
		getRateCorrectionTau(),					\
		thisDetector->actionParameter[headerAfter]);
#ifdef VERBOSE
	cout << "Executing header after " << cmd << endl;
#endif
	system(cmd);
      }
    } else
      break;
     

    
    if (thisDetector->stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (ip<(np-1)) {
      thisDetector->fileIndex=startindex; 
    }
     } // loop on position finished

  //script after
        if (thisDetector->stoppedFlag==0) {
	  if (thisDetector->actionMask & (1 << scriptAfter)) {
	    //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	    sprintf(cmd,"%s nrun=%d fn=%s par=%s sv0=%f sv1=%f p0=%s p1=%s",thisDetector->actionScript[scriptAfter],thisDetector->fileIndex,createFileName().c_str(),thisDetector->actionParameter[scriptAfter],currentScanVariable[0],currentScanVariable[1],thisDetector->scanParameter[0],thisDetector->scanParameter[1]);
#ifdef VERBOSE
	    cout << "Executing script after " << cmd << endl;
#endif
	    system(cmd);
	  } 
	} else
	    break;
  

    if (thisDetector->stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is1<(ns1-1)) {
      thisDetector->fileIndex=startindex; 
    }


  } 

  //end scan1 loop is1
  //currentScanVariable[MAX_SCAN_LEVELS];
  

    if (thisDetector->stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is0<(ns0-1)) {
      thisDetector->fileIndex=startindex; 
    }
  } //end scan0 loop is0

  thisDetector->fileIndex=lastindex;
  if (thisDetector->stoppedFlag==0) {
    if (thisDetector->actionMask & (1 << stopScript)) {
      //Custom stop script. The arguments are passed as nrun=n par=p.
      sprintf(cmd,"%s nrun=%d par=%s",thisDetector->actionScript[stopScript],thisDetector->fileIndex,thisDetector->actionParameter[stopScript]);
#ifdef VERBOSE
      cout << "Executing stop script " << cmd << endl;
#endif
      system(cmd);
    }
  } 


   if (thisDetector->threadedProcessing) { 
#ifdef VERBOSE
    std::cout<< " ***********************waiting for data processing thread to finish " << queuesize << std::endl ;
#endif
     jointhread=1;
     pthread_join(dataProcessingThread, &status);
   }
}


void* gotthardDetector::processData(int delflag) {


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
  string ext;


#ifdef ACQVERBOSE
  std::cout<< " processing data - threaded mode " << thisDetector->threadedProcessing;
#endif

  if (thisDetector->correctionMask!=0) {
    ext=".dat";
  } else {
    ext=".raw";
  }
  while(dum | thisDetector->threadedProcessing) { // ????????????????????????
    
    
    while( !dataQueue.empty() ) {
      queuesize=dataQueue.size();

      /** Pop data queue */
      myData=dataQueue.front(); // get the data from the queue 
      if (myData) {



	thisDetector->progressIndex++;
#ifdef VERBOSE
	cout << "Progress is " << getCurrentProgress() << " \%" << endl;
#endif

	//process data
	/** decode data */
	fdata=decodeData(myData);

	/** write raw data file */	   
	if (thisDetector->correctionMask==0 && delflag==1) {


	  writeDataFile (createFileName().append(".raw"), fdata, NULL, NULL, 'i'); 
	  delete [] fdata;
	} else {
	  writeDataFile (createFileName().append(".raw"), fdata, NULL, NULL, 'i');

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
	      writeDataFile (createFileName().append(".dat"), ffcdata, ffcerr,ang);
	    addToMerging(ang, ffcdata, ffcerr, mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	    if ((currentPositionIndex==thisDetector->numberOfPositions) || (currentPositionIndex==0)) {
	      np=finalizeMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	      /** file writing */
	      currentPositionIndex++;
	      if (thisDetector->correctionMask!=0)
		writeDataFile (createFileName().append(".dat"),mergingCounts, mergingErrors, mergingBins,'f',np);
	      if (delflag) {
		delete [] mergingBins;
		delete [] mergingCounts;
		delete [] mergingErrors;
		delete [] mergingMultiplicity;
	      } else {
		thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(createFileName().append(ext)).c_str(),np);/*
		if (thisDetector->correctionMask!=0) {
		  //thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,thisDetector->progressIndex+1,(createFileName().append(".dat")).c_str(),np);
		  thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(createFileName().append(".dat")).c_str(),np);
		} else {
		  thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(createFileName().append(".raw")).c_str(),np);
		  //thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,thisDetector->progressIndex+1,(createFileName().append(".raw")).c_str(),np);
		  }*/
		finalDataQueue.push(thisData);
	      }
	    }
	    
	    if (ffcdata)
	      delete [] ffcdata;
	    if (ffcerr)
	      delete [] ffcerr;
	    if (ang)
	      delete [] ang;
	  } else {
	    if (thisDetector->correctionMask!=0) {
	      writeDataFile (createFileName().append(".dat"), ffcdata, ffcerr);
	    }
	    if (delflag) {
	      if (ffcdata)
		delete [] ffcdata;
	      if (ffcerr)
		delete [] ffcerr;
	      if (ang)
		delete [] ang;
	    } else {
	      thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(createFileName().append(ext)).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);/*
	      if (thisDetector->correctionMask!=0) {
		thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(createFileName().append(".dat")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
		//thisData=new detectorData(ffcdata,ffcerr,NULL,thisDetector->progressIndex+1,(createFileName().append(".dat")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
	      }	else {
		thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(createFileName().append(".raw")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
		//thisData=new detectorData(ffcdata,ffcerr,NULL,thisDetector->progressIndex+1,(createFileName().append(".raw")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
		}*/
	      finalDataQueue.push(thisData);  
	    }  
	  }
	}
	thisDetector->fileIndex++;

	/*
	thisDetector->progressIndex++;
#ifdef VERBOSE
	cout << "Progress is " << getCurrentProgress() << " \%" << endl;
#endif
	*/

	delete [] myData;
	myData=NULL;
	dataQueue.pop(); //remove the data from the queue
	queuesize=dataQueue.size();
      }
    }
    if (jointhread) {
      if (dataQueue.size()==0)
	break;
    }
    dum=0;
  } // ????????????????????????
  return 0;

}



void gotthardDetector::startThread(int delflag) {
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
   gotthardDetector *myDet=(gotthardDetector*)n;
   myDet->processData(1);
   pthread_exit(NULL);

}

void* startProcessDataNoDelete(void *n) {

   //void* processData(void *n) {
  gotthardDetector *myDet=(gotthardDetector*)n;
  myDet->processData(0);
  pthread_exit(NULL);

}
 
runStatus gotthardDetector::getRunStatus(){


  int fnum=F_GET_RUN_STATUS;
  int ret=FAIL;
  char mess[100];
  runStatus retval=ERROR;
#ifdef VERBOSE
  std::cout<< "GOTTHARD Getting status "<< std::endl;
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

int64_t gotthardDetector::getTimeLeft(timerIndex index){


  int fnum=F_GET_TIME_LEFT;
  int64_t retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "GOTTHARD Getting  timer  "<< index <<  std::endl;
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







  /*
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
int gotthardDetector::setPositions(int nPos, float *pos){

  if (nPos>=0)
    thisDetector->numberOfPositions=nPos; 
  for (int ip=0; ip<nPos; ip++) 
    thisDetector->detPositions[ip]=pos[ip]; 


  setTotalProgress();
  
  return thisDetector->numberOfPositions;

}
/* 
   get  positions for the acquisition
   \param pos array which will contain the encoder positions
   \returns number of positions
*/
int gotthardDetector::getPositions(float *pos){ 

  if (pos ) {
    for (int ip=0; ip<thisDetector->numberOfPositions; ip++) 
      pos[ip]=thisDetector->detPositions[ip];
  } 
  setTotalProgress();

  
  return     thisDetector->numberOfPositions;
};
  
