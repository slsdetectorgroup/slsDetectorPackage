#include "slsDetectorUtils.h"
#include "usersFunctions.h"

#include  <sys/ipc.h>
#include  <sys/shm.h>


slsDetectorUtils::slsDetectorUtils()   {};
  





void  slsDetectorUtils::acquire(int delflag){



#ifdef VERBOSE
  cout << "Acquire function "<< delflag << endl;
#endif

#ifdef VERBOSE
  cout << "Stopped flag is "<< stoppedFlag << delflag << endl;
#endif


  posfinished=0;


  void *status;
  int trimbit;




  char cmd[MAX_STR_LENGTH];
  int  startindex=*fileIndex;
  int nowindex=startindex, lastindex=startindex;
  string fn;




  //string sett;
  if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)))
       connect_channels();
       


 // setTotalProgress();
  progressIndex=0;
  *stoppedFlag=0;



  resetFinalDataQueue();
  resetDataQueue();

  
  //cout << "main mutex lock line 6188" << endl;
  pthread_mutex_lock(&mp);
  jointhread=0;
  queuesize=0;
  pthread_mutex_unlock(&mp);
  //cout << "main mutex unlock line 6188" << endl;



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
      if (*actionMask & (1 << startScript)) {
	//"Custom start script. The arguments are passed as nrun=n par=p.");
	sprintf(cmd,"%s nrun=%d par=%s",getActionScript(startScript).c_str(),*fileIndex,getActionParameter(startScript).c_str());
#ifdef VERBOSE
	cout << "Executing start script " << cmd << endl;
#endif
	system(cmd);
      }
  }

  for (int is0=0; is0<ns0; is0++) {
    //  cout << "scan0 loop" << endl;

  if (*stoppedFlag==0) {

    currentScanVariable[0]=getScanStep(0,is0);
    currentScanIndex[0]=is0;

    switch(scanMode[0]) {
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
      sprintf(cmd,"%s nrun=%d fn=%s var=%f par=%s",getScanScript(0).c_str(),*fileIndex,createFileName().c_str(),currentScanVariable[0],getScanParameter(0).c_str());
#ifdef VERBOSE
      cout << "Executing scan script 0 " << cmd << endl;
#endif
      system(cmd);
     

    }
  } else
    break;
  

  for (int is1=0; is1<ns1; is1++) {
    // cout << "scan1 loop" << endl;

     if (*stoppedFlag==0) {

       currentScanVariable[1]=getScanStep(1,is1);
    currentScanIndex[1]=is1;

    switch(scanMode[1]) {
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
      sprintf(cmd,"%s nrun=%d fn=%s var=%f par=%s",getScanScript(1).c_str(),*fileIndex,createFileName().c_str(),currentScanVariable[1],getScanParameter(1).c_str());
#ifdef VERBOSE
      cout << "Executing scan script 1 " << cmd << endl;
#endif
      system(cmd);
    }
  
     } else
       break;

     if (*stoppedFlag==0) {
       if (*actionMask & (1 << scriptBefore)) {
	 //Custom script before each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	 sprintf(cmd,"%s nrun=%d fn=%s par=%s sv0=%f sv1=%f p0=%s p1=%s",getActionScript(scriptBefore).c_str(),*fileIndex,createFileName().c_str(),getActionParameter(scriptBefore).c_str(),currentScanVariable[0],currentScanVariable[1],getScanParameter(0).c_str(),getScanParameter(1).c_str());
#ifdef VERBOSE
	 cout << "Executing script before " << cmd << endl;
#endif
	 system(cmd);
       }
     } else
       break;

     currentPositionIndex=0;
     
     for (int ip=0; ip<np; ip++) {
       //   cout << "positions " << endl;
       if (*stoppedFlag==0) {
	 if  (*numberOfPositions>0) {
	   go_to_position (detPositions[ip]);
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
       
       //   cout << "aaaaa" << endl;
       //cout << createFileName() << endl;
       //cout << "bbbbb" << endl;
       fn=createFileName();
       //cout << fn << endl;
       nowindex=*fileIndex;

       if (*stoppedFlag==0) {


	 if (*correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0();
	 
	 // cout << "header " << endl;
	 if (*actionMask & (1 << headerBefore)) {
	   //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	   sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s",\
		   getActionScript(headerBefore).c_str(),\
		   nowindex,\
		   fn.c_str(),				       \
		   ((float)timerValue[ACQUISITION_TIME])*1E-9, \
		   *currentSettings, \
		   *currentThresholdEV, \
		   getBadChannelCorrectionFile().c_str(), \
		   angularConversion::getAngularConversionFile().c_str(), \
		   *globalOffset, \
		   *fineOffset,\
		   getFlatFieldCorrectionDir().c_str(),\
		   getFlatFieldCorrectionFile().c_str(), \
		   getRateCorrectionTau(),\
		   getActionParameter(headerBefore).c_str()\
		   );
#ifdef VERBOSE
	   cout << "Executing header before " << cmd << endl;
#endif
	   system(cmd);
	 }
       } else
	 break;
       
       
       
       if (*stoppedFlag==0) {

	 if (*correctionMask&(1<< ANGULAR_CONVERSION)) {
	   pthread_mutex_lock(&mp);
	   currentPosition=get_position();  
	   posfinished=0;
	   pthread_mutex_unlock(&mp);
	 }


	 //	 cout << "starting???? " << endl;
	 startAndReadAll();

	 pthread_mutex_lock(&mp);
	 posfinished=1;
	 pthread_mutex_unlock(&mp);

#ifdef VERBOSE	
	 cout << "returned " << endl;
	 pthread_mutex_lock(&mp);
	 cout << "AAAA queue size " << dataQueue.size()<< endl;
	 pthread_mutex_unlock(&mp);
#endif

	 
	 if (*correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0()-currentI0;
	 
	 ////////////////// Reput in in case of unthreaded processing
 	 if (*threadedProcessing==0){
#ifdef VERBOSE
	   cout << "start unthreaded process data " << endl;
#endif
 	   processData(delflag); 
	 }
	 
	 
       } else
	 break;
       
       pthread_mutex_lock(&mp);
       while (queuesize){
	 pthread_mutex_unlock(&mp);
	 usleep(10000);
	 pthread_mutex_lock(&mp);
       }
       pthread_mutex_unlock(&mp);
    
    if (*stoppedFlag==0) {
      if (*actionMask & (1 << headerAfter)) {
	//Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s", \
		getActionScript(headerAfter).c_str(),			\
		nowindex,				\
		fn.c_str(),				\
		((float)timerValue[ACQUISITION_TIME])*1E-9, \
		*currentSettings,			\
		*currentThresholdEV,			\
		getBadChannelCorrectionFile().c_str(),	\
		angularConversion::getAngularConversionFile().c_str(),	\
		*globalOffset,				\
		*fineOffset,				\
		getFlatFieldCorrectionDir().c_str(),		\
		getFlatFieldCorrectionFile().c_str(),		\
		getRateCorrectionTau(),			\
		getActionParameter(headerAfter).c_str());
#ifdef VERBOSE
	cout << "Executing header after " << cmd << endl;
#endif
	system(cmd);
	
      }
      if (*fileIndex>lastindex)
	lastindex=*fileIndex;
    } else {

      
      if (*fileIndex>lastindex)
	lastindex=*fileIndex;

      break;
    }
     

    
    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (ip<(np-1)) {
      *fileIndex=startindex; 
    }
     } // loop on position finished
  
  //script after
        if (*stoppedFlag==0) {
	  if (*actionMask & (1 << scriptAfter)) {
	    //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	    sprintf(cmd,"%s nrun=%d fn=%s par=%s sv0=%f sv1=%f p0=%s p1=%s",getActionScript(scriptAfter).c_str(),*fileIndex,createFileName().c_str(),getActionParameter(scriptAfter).c_str(),currentScanVariable[0],currentScanVariable[1],getScanParameter(0).c_str(),getScanParameter(1).c_str());
#ifdef VERBOSE
	    cout << "Executing script after " << cmd << endl;
#endif
	    system(cmd);
	  } 
	} else
	    break;
  

    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is1<(ns1-1)) {
      *fileIndex=startindex; 
    }


  } 

  //end scan1 loop is1
  //currentScanVariable[MAX_SCAN_LEVELS];
  

    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is0<(ns0-1)) {
      *fileIndex=startindex; 
    }
  } //end scan0 loop is0

  *fileIndex=lastindex;
  if (*stoppedFlag==0) {
    if (*actionMask & (1 << stopScript)) {
      //Custom stop script. The arguments are passed as nrun=n par=p.
      sprintf(cmd,"%s nrun=%d par=%s",getActionScript(stopScript).c_str(),*fileIndex,getActionParameter(stopScript).c_str());
#ifdef VERBOSE
      cout << "Executing stop script " << cmd << endl;
#endif
      system(cmd);
    }
  } 


   if (*threadedProcessing) { 
     //#ifdef VERBOSE
     //  std::cout<< " ***********************waiting for data processing thread to finish " << queuesize <<" " << thisDetector->fileIndex << std::endl ;
    //#endif
     //cout << "main mutex lock line 6488" << endl;
     pthread_mutex_lock(&mp);
     jointhread=1;
     pthread_mutex_unlock(&mp);
     //cout << "main mutex unlock line 6488" << endl;
     pthread_join(dataProcessingThread, &status);
     // std::cout<< " ***********************data processing  finished " << queuesize <<" " << thisDetector->fileIndex << std::endl ;
   }

      if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)))
	disconnect_channels();
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
		std::cout<<std::endl<<dec<<i+1<<": \t"<<statusval<<"\t";
usleep(10000);

		startAcquisition();

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

		}
		else {
			if (s==RUNNING){
			count=0;
			while(s==RUNNING){
				count++;
				if(count==5){
					sprintf(statusval,"%x",readRegister(0x25));


					std::cout<<"STUCK: "<<statusval<<std::endl;
					exit(-1);
				}
				usleep(2);
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


