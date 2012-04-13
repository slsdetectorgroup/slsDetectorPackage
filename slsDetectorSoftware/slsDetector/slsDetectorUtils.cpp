#include "slsDetectorUtils.h"
#include "usersFunctions.h"

#include  <sys/ipc.h>
#include  <sys/shm.h>


slsDetectorUtils::slsDetectorUtils()   {};
  





void  slsDetectorUtils::acquire(int delflag){



#ifdef VERBOSE
  cout << "Acquire function "<< delflag << endl;
  cout << "Stopped flag is "<< stoppedFlag << delflag << endl;
#endif

  void *status;

  setStartIndex(*fileIndex);
  
  //  int lastindex=startindex, nowindex=startindex;


  if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)))
       connect_channels();
       


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
	   go_to_position (detPositions[ip]);
	   currentPositionIndex=ip+1;
#ifdef VERBOSE
	   std::cout<< "moving to position" << std::endl;
#endif
	 } 
       } else
	 break;
       

       if (*stoppedFlag==0) {
	 executeAction(scriptBefore);
       } else
	 break;
       
       
       
       if (*stoppedFlag==0) {

	 executeAction(headerBefore);
	 
       if (*correctionMask&(1<< ANGULAR_CONVERSION)) {
	 pthread_mutex_lock(&mp);
	 currentPosition=get_position();  
	 posfinished=0;
	 pthread_mutex_unlock(&mp);
       }

       if (*correctionMask&(1<< I0_NORMALIZATION))
	 get_i0(0);
       
       startAndReadAll();
       
       if (*correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0(1); // this is the correct i0!!!!!
       
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
	 usleep(10000);
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


