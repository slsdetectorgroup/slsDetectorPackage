#include "slsDetectorUtils.h"
#include "usersFunctions.h"
#include "slsDetectorCommand.h"

#include  <sys/ipc.h>
#include  <sys/shm.h>


slsDetectorUtils::slsDetectorUtils()   {
#ifdef VERBOSE
  cout << "setting callbacks" << endl;
#endif
  registerGetPositionCallback(&defaultGetPosition);
  registerConnectChannelsCallback(&defaultConnectChannels);
  registerDisconnectChannelsCallback(&defaultDisconnectChannels);
  registerGoToPositionCallback(&defaultGoToPosition);
  registerGoToPositionNoWaitCallback(&defaultGoToPositionNoWait);
  registerGetI0Callback(&defaultGetI0);
#ifdef VERBOSE
  cout << "done " << endl;
#endif

};
  





void  slsDetectorUtils::acquire(int delflag){



#ifdef VERBOSE
  cout << "Acquire function "<< delflag << endl;
  cout << "Stopped flag is "<< stoppedFlag << delflag << endl;
#endif

  void *status;

  setStartIndex(*fileIndex);
  
  //  int lastindex=startindex, nowindex=startindex;


  if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION))) {
    if (connect_channels())
      connect_channels();
  }
       


 // setTotalProgress();
  progressIndex=0;
  *stoppedFlag=0;



  
  pthread_mutex_lock(&mp);
  resetFinalDataQueue();
  resetDataQueue();
  jointhread=0;
  queuesize=0;
  posfinished=0;
  pthread_mutex_unlock(&mp);



#ifdef VERBOSE
  cout << " starting thread " << endl;
#endif

  if (*threadedProcessing) {
    startThread(delflag);
  }

  //cout << "data thread started " << endl;
  int np=1;
  if (*numberOfPositions>0) 
    np=*numberOfPositions;

  int ns0=1;
  if (*actionMask & (1 << MAX_ACTIONS)) {
    ns0=nScanSteps[0];
  }

  if (ns0<1)
    ns0=1;

  int ns1=1;
  if (*actionMask & (1 << (MAX_ACTIONS+1))) {
    ns1=nScanSteps[1];
  }

  if (ns1<1)
    ns1=1;


  //cout << "action at start" << endl;
  if (*stoppedFlag==0) {
    executeAction(startScript);
  }

  for (int is0=0; is0<ns0; is0++) {
    //  cout << "scan0 loop" << endl;

  if (*stoppedFlag==0) {
    executeScan(0,is0);
  } else
    break;
  

  for (int is1=0; is1<ns1; is1++) {
    // cout << "scan1 loop" << endl;

     if (*stoppedFlag==0) {
       executeScan(1,is1);
     } else
       break;

     if (*stoppedFlag==0) {
       executeAction(scriptBefore);
     } else
       break;

     currentPositionIndex=0;
     
     for (int ip=0; ip<np; ip++) {
       //   cout << "positions " << endl;
       if (*stoppedFlag==0) {
	 if  (*numberOfPositions>0) {
	   if (go_to_position)
	     go_to_position (detPositions[ip]);
	   currentPositionIndex=ip+1;
#ifdef VERBOSE
	   std::cout<< "moving to position" << std::endl;
#endif
	 } 
       } else
	 break;
       

       createFileName();

       if (*stoppedFlag==0) {
	 executeAction(scriptBefore);
       } else
	 break;
       
       
       

       if (*stoppedFlag==0) {

	 executeAction(headerBefore);
	 
       if (*correctionMask&(1<< ANGULAR_CONVERSION)) {
	 pthread_mutex_lock(&mp);
	 if (get_position)
	   currentPosition=get_position();  
	 posfinished=0;
	 pthread_mutex_unlock(&mp);
       }

       if (*correctionMask&(1<< I0_NORMALIZATION)) {
	 if (get_i0)
	   get_i0(0);
       }
       
       startAndReadAll();
       
       if (*correctionMask&(1<< I0_NORMALIZATION)) {
	 if (get_i0) 
	   currentI0=get_i0(1); // this is the correct i0!!!!!
       }
       
       pthread_mutex_lock(&mp);
       posfinished=1;
       pthread_mutex_unlock(&mp);
       
 
	 
 	 if (*threadedProcessing==0){
#ifdef VERBOSE
	   cout << "start unthreaded process data " << endl;
#endif
 	   processData(delflag); 
	 } 

       } else
	 break;
       

       // wait until data processing thread has finished the data

       pthread_mutex_lock(&mp);
       while (queuesize){
	 pthread_mutex_unlock(&mp);
	 usleep(100000);
	 pthread_mutex_lock(&mp);
	 
       }
       pthread_mutex_unlock(&mp);
       
       if (*stoppedFlag==0) {
	 executeAction(headerAfter);	 
	 setLastIndex(*fileIndex);
       } else {
	 setLastIndex(*fileIndex);
	 break;
       }
     

    
       if (*stoppedFlag) {
#ifdef VERBOSE
	 std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
	 break;
       } else if (ip<(np-1)) {
	 *fileIndex=setStartIndex(); 
       }
     } // loop on position finished
     
     //script after
     if (*stoppedFlag==0) {
       executeAction(scriptAfter);
     } else
       break;
     
     
    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is1<(ns1-1)) {
      *fileIndex=setStartIndex(); 
    }
  } 
  
  //end scan1 loop is1
  

  if (*stoppedFlag) {
#ifdef VERBOSE
    std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
    break;
  } else if (is0<(ns0-1)) {
    *fileIndex=setStartIndex(); 
  }

  } //end scan0 loop is0

  *fileIndex=setLastIndex();
  if (*stoppedFlag==0) {
    executeAction(stopScript);
  } 

  // waiting for the data processing thread to finish!
   if (*threadedProcessing) { 
     pthread_mutex_lock(&mp);
     jointhread=1;
     pthread_mutex_unlock(&mp);
     pthread_join(dataProcessingThread, &status);
   }

   
   if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION))) {
     if (disconnect_channels)
       disconnect_channels();
   }
}







int slsDetectorUtils::setTotalProgress() {
  
  int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1;

  if (timerValue[FRAME_NUMBER])
    nf=timerValue[FRAME_NUMBER];
  
  if (timerValue[CYCLES_NUMBER]>0)
	nc=timerValue[CYCLES_NUMBER];

      if (*numberOfPositions>0)
	npos=*numberOfPositions;

      if ((nScanSteps[0]>0) && (*actionMask & (1 << MAX_ACTIONS)))
	nscan[0]=nScanSteps[0];

      if ((nScanSteps[1]>0) && (*actionMask & (1 << (MAX_ACTIONS+1))))
	nscan[1]=nScanSteps[1];
      
      totalProgress=nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
      cout << "nc " << nc << endl;
      cout << "nf " << nf << endl;
      cout << "npos " << npos << endl;
      cout << "nscan[0] " << nscan[0] << endl;
      cout << "nscan[1] " << nscan[1] << endl;

      cout << "Set total progress " << totalProgress << endl;
#endif
      return totalProgress;
}









int slsDetectorUtils::setBadChannelCorrection(string fname, int &nbadtot, int *badchanlist, int off){

  int nbad;
  int badlist[MAX_BADCHANS];

  ifstream infile;
  string str;
  int interrupt=0;
  int ich;
  int chmin,chmax;
#ifdef VERBOSE
  std::cout << "Setting bad channel correction to " << fname << std::endl;
#endif
  int modmi=0, modma=1;
  int singlefile=0;

  string fn;
  int offset=off;


  nbadtot=0;

  if (fname=="" || fname=="none") {
    ;
  } else { 

    if (fname.find(".sn")==string::npos && fname.find(".chans")==string::npos) {
      modma=setNumberOfModules();
      singlefile=1;
    }

    for (int im=0; im<modma; im++) {
      

      if (singlefile) {

	ostringstream ostfn;
	ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	fn=ostfn.str();
  
      } else
	fn=fname;
      


    infile.open(fn.c_str(), ios_base::in);
    if (infile.is_open()==0) {
      std::cout << "could not open file " << fname <<std::endl;
      return -1;
    }


    nbad=setBadChannelCorrection(infile, nbad, badlist, offset);
    infile.close();
    
    for (int ich=0; ich<nbad; ich++) {
      if (nbadtot<MAX_BADCHANS) {
	badchanlist[nbadtot]=badlist[ich];
	nbadtot++;
      }
    }
    
    offset+=getChansPerMod(im); 
    
  }
  
  }
  if (nbadtot>0 && nbadtot<MAX_BADCHANS) {
    return nbadtot;
  } else
    return 0;

}




float slsDetectorUtils::getCurrentProgress() {
#ifdef VERBOSE
  cout << progressIndex << " / " << totalProgress << endl;
#endif
  return 100.*((float)progressIndex)/((float)totalProgress);
}



void slsDetectorUtils::incrementProgress()  { 
  progressIndex++;							
  cout << fixed << setprecision(2) << setw (6) << getCurrentProgress() << " \%";
#ifdef VERBOSE
    cout << endl;
#else
  cout << "\r" << flush;
#endif

};



int slsDetectorUtils::testFunction(int times) {
	int i,count=0;
	runStatus s;
	char controlval[1000];
	char statusval[1000];

	int nchans = getTotalNumberOfChannels();
	short int dataVals[nchans];

	for(i=0;i<times;i++){
	    sprintf(statusval,"%x",readRegister(0x25));
		std::cout<<std::endl<<dec<<i+1<<": stat:\t"<<statusval<<"\t";
		sprintf(controlval,"%x",readRegister(0x24));
		std::cout<<"cont:"<<controlval<<"\t"<<std::endl;

		startAcquisition();

		sprintf(controlval,"%x",readRegister(0x24));
		std::cout<<"cont:"<<controlval<<"\t"<<std::endl;
	    //sprintf(statusval,"%x",readRegister(0x25));
	    //std::cout<<statusval<<std::endl;
		s = getRunStatus();
		if(s==IDLE){
			std::cout<<"IDLE\t"<<std::endl;
			s = getRunStatus();
			if(s==IDLE){
				std::cout<<"IDLE"<<std::endl;
				exit(-1);
			}
			;
		}
		else {
			if (s==RUNNING){
			count=0;
			while(s==RUNNING){
				count++;//std::cout<<"count:"<<count<<std::endl;
				if(count==4){
					sprintf(statusval,"%x",readRegister(0x25));


					std::cout<<"STUCK: stat"<<statusval<<std::endl;
					exit(-1);
				}
				usleep(50000);
				//val=readRegister(0x25);
				s = getRunStatus();
			}
		}
		}
/*		else{
			std::cout<<"\nWeird Status. "<<runStatusType(s)<<" Exit\n";
			exit(-1);
		}*/
		system("rm ~/wORKSPACE/scratch/run*  ");
		//system("more ~/wORKSPACE/scratch/run*  ");
		usleep(1000000);

		setFileIndex(0);
		int b;

		b=setThreadedProcessing(-1);
		setThreadedProcessing(0);
	    readAll();
	    processData(1);
	    setThreadedProcessing(b);


		if(!readDataFile("/home/l_maliakal_d/wORKSPACE/scratch/run_1.raw",dataVals)){
			std::cout<< "Could not open file "<< std::endl;
			exit(-1);
		}

		std::cout<<std::endl;
		for(int j=1277;j< (nchans);j++)
			std::cout<<"\t"<<j<<":"<<dataVals[j];

		if(dataVals[1278]!=2558){
			std::cout<< "DATA ERROR!! "<< std::endl;
			exit(-1);
		}
	}
	std::cout<<std::endl;
	return 0;
}




int slsDetectorUtils::retrieveDetectorSetup(string const fname1, int level){



  slsDetectorCommand *cmd;


    char ext[100];
    int skip=0;
   string fname;
   string str;
   ifstream infile;
   int iargval;
   int interrupt=0;
  char *args[2];

  char myargs[2][1000];
  args[0]=myargs[0];
  args[1]=myargs[1];

  string sargname, sargval;
  int iline=0;
  
  if (level==2) {
//     fname=fname1+string(".config");
//     readConfigurationFile(fname);
#ifdef VERBOSE
    cout << "config file read" << endl;
#endif
    fname=fname1+string(".det");
  }  else
    fname=fname1;

  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    cmd=new slsDetectorCommand(this);
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
	    strcpy(myargs[iargval],sargname.c_str());
#ifdef VERBOSE
      std::cout<< args[iargval]  << std::endl;
#endif
	    iargval++;
	    // }
	    skip=0;
	}

	if (level!=2) {
	  if (string(args[0])==string("flatfield"))
	    skip=1;
	  else if  (string(args[0])==string("badchannels"))
	    skip=1;
	  else if (string(args[0])==string("trimbits"))
	    skip=1;
	}
	if (skip==0)
	  cmd->executeLine(iargval,args,PUT_ACTION);
      }
      iline++;
    }
    delete cmd;
    infile.close();

  } else {
    std::cout<< "Error opening  " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read  " << iline << " lines" << std::endl;
#endif
  return iline;


}


int slsDetectorUtils::dumpDetectorSetup(string const fname, int level){

  slsDetectorCommand *cmd=new slsDetectorCommand(this);

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
    "timing",\
    "fineoff",\
    "startscript",\
    "startscriptpar",\
    "stopscript",\
    "stopscriptpar",\
    "scriptbefore",\
    "scriptbeforepar",\
    "scriptafter",\
    "scriptafterpar",\
    "scan0script",\
    "scan0par",\
    "scan0prec",\
    "scan0steps",\
    "scan1script",\
    "scan1par",\
    "scan1prec",\
    "scan1steps",\
    "ratecorr",\
    "flatfield",\
    "badchannels",\
    "trimbits"
  };
  int nvar=35;



    char ext[100];

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
    for (iv=0; iv<nvar-3; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
    }


    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".ff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".bad");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
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
  outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
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
  
  delete cmd;
  return 0;

} 


// int slsDetectorUtils::setFlatFieldCorrectionFile(string fname){
//   int tch=getTotalNumberOfChannels();



//   float data[tch],  xmed[tch];
//   float ffcoefficients[tch], fferrors[tch];
//   int nmed=0;
//   int idet=0, ichdet=-1;
//   char ffffname[MAX_STR_LENGTH*2];
//   int nbad=0, nch;
//   int badlist[MAX_BADCHANS];
//   int im=0;

//  if (fname=="default") {
//    fname=string(flatFieldFile);
//  }
//   if (fname=="") {
// #ifdef VERBOSE
//    std::cout<< "disabling flat field correction" << std::endl;
// #endif
//    (*correctionMask)&=~(1<<FLAT_FIELD_CORRECTION);
//     //  strcpy(thisMultiDetector->flatFieldFile,"none");
    
//    setFlatFieldCorrection(NULL, NULL);
//   } else { 

// #ifdef VERBOSE
//    std::cout<< "Setting flat field correction from file " << fname << std::endl;
// #endif
//    sprintf(ffffname,"%s/%s",flatFieldDir,fname.c_str());
//    nch=readDataFile(string(ffffname),data);

//    if (nch>tch)
//      nch=tch;
   
//    if (nch>0) {
//      strcpy(flatFieldFile,fname.c_str());
     

//      nbad=0;
//      for (int ichan=0; ichan<nch; ichan++) {
     
//        if (data[ichan]>0) {
// 	 /* add to median */
// 	 im=0;
// 	 while ((im<nmed) && (xmed[im]<data[ichan])) 
// 	   im++;
// 	  for (int i=nmed; i>im; i--) 
// 	    xmed[i]=xmed[i-1];
// 	  xmed[im]=data[ichan];
// 	  nmed++;
//        } else {
// 	 if (nbad<MAX_BADCHANS) {
// 	   badlist[nbad]=ichan;
// 	   nbad++;
// 	 }
//        }
//      }
     
	
//      if (nmed>1 && xmed[nmed/2]>0) {
// #ifdef VERBOSE
//        std::cout<< "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << std::endl;
// #endif
       
//        thisMultiDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);

// 	// add to ff coefficients and errors of single detectors

	  

       
//        for (int ichan=0; ichan<nch; ichan++) {
	
// 	 setFlatFieldCorrection(ffcoefficients, fferrors);
	 
// 	 if (data[ichan]>0) {
// 	   ffcoefficients[ichan]=xmed[nmed/2]/data[ichan];
// 	   fferrors[ichan]=ffcoefficients[ichan]*sqrt(data[ichan])/data[ichan];
// 	  } else {
// 	   ffcoefficients[ichan]=0.;
// 	   fferrors[ichan]=1.;
// 	 }
	
//        }
       
//        setFlatFieldCorrection(ffcoefficients, fferrors);
       
      
//      } else {
// 	std::cout<< "Flat field data from file " << fname << " are not valid (" << nmed << "///" << xmed[nmed/2] << std::endl;
// 	thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
// 	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
// 	  if (detectors[i])
// 	    detectors[i]->setFlatFieldCorrection(NULL, NULL);
// 	}
// 	return -1;
//       }
//    } else {
//       std::cout<< "Flat field from file " << fname << " is not valid " << nch << std::endl;  
//       thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
//       for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
// 	if (detectors[i])
// 	  detectors[i]->setFlatFieldCorrection(NULL, NULL);
//       }
//       return -1;
//     } 
//   }
//   return thisMultiDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
// }
 
