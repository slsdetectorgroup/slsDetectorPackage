#include "slsDetectorUtils.h"
#include "usersFunctions.h"
#include "slsDetectorCommand.h"
#include "postProcessing.h"
#include "enCalLogClass.h"
#include "angCalLogClass.h"

#include  <cstdlib>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <time.h> //clock()
using namespace std;
slsDetectorUtils::slsDetectorUtils()  {


#ifdef VERBOSE
  cout << "setting callbacks" << endl;
#endif
  acquisition_finished=NULL;
  acqFinished_p=NULL;
  measurement_finished=NULL;
  measFinished_p=NULL;
  progress_call=0;
  pProgressCallArg=0;
  registerGetPositionCallback(&defaultGetPosition, NULL);
  registerConnectChannelsCallback(&defaultConnectChannels,NULL);
  registerDisconnectChannelsCallback(&defaultDisconnectChannels,NULL);
  registerGoToPositionCallback(&defaultGoToPosition,NULL);
  registerGoToPositionNoWaitCallback(&defaultGoToPositionNoWait,NULL);
  registerGetI0Callback(&defaultGetI0,NULL);
#ifdef VERBOSE

  registerAcquisitionFinishedCallback(&dummyAcquisitionFinished,this);
  registerMeasurementFinishedCallback(&dummyMeasurementFinished,this);
  cout << "done " << endl;
#endif

};
 




int  slsDetectorUtils::acquire(int delflag){
	struct timespec begin,end;
	clock_gettime(CLOCK_REALTIME, &begin);


  //ensure acquire isnt started multiple times by same client
  if(getAcquiringFlag() == false)
    setAcquiringFlag(true);
  else{
	  std::cout << "Error: Acquire has already been started." << std::endl;
	  return FAIL;
  }

	//not in the loop for real time acqusition yet,
	//in the real time acquisition loop, processing thread will wait for a post each time
	sem_init(&sem_newRTAcquisition,1,0);


  bool receiver = (setReceiverOnline()==ONLINE_FLAG);
  if(!receiver){
	  setDetectorIndex(-1);
  }else{
	  //put receiver read frequency to random if no gui
	  int ret = setReadReceiverFrequency(0);
	  if(ret>0 && (dataReady == NULL)){
		  ret = setReadReceiverFrequency(1,0);
		  std::cout << "No Data call back and hence receiver read frequency reset to " << ret <<" (Random)" << std::endl;
	  }

	  //start/stop data streaming threads if threads in client enabled/disabled
	  ret = enableDataStreamingFromReceiver(-1);
	 // cout<<"getting datastream:"<<ret<<endl;
	 // cout<<"result of enabledatastream:"<<enableDataStreamingFromReceiver(ret)<<endl;
  }

  int nc=setTimer(CYCLES_NUMBER,-1);
  int nf=setTimer(FRAME_NUMBER,-1);
  if (nc==0) nc=1;
  if (nf==0) nf=1;

  int multiframe = nc*nf;

  // setTotalProgress();
  //moved these 2 here for measurement change
  progressIndex=0;
  *stoppedFlag=0;

  angCalLogClass *aclog=NULL;
  enCalLogClass *eclog=NULL;
  //  int lastindex=startindex, nowindex=startindex;
  int connectChannels=0;

#ifdef VERBOSE
  cout << "Acquire function "<< delflag << endl;
  cout << "Stopped flag is "<< stoppedFlag << delflag << endl;
#endif

  void *status;


  if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)) || getActionMode(angCalLog) || (getScanMode(0)==positionScan)|| (getScanMode(1)==positionScan)) {
    if (connectChannels==0)
      if (connect_channels) {
	connect_channels(CCarg);
	connectChannels=1;
      }      
  }
  

  if (getActionMode(angCalLog)) {
    aclog=new angCalLogClass(this);
  }
  if (getActionMode(enCalLog)) {
    eclog=new enCalLogClass(this);
    
  }
  


  setJoinThread(0);
  positionFinished(0);




  int nm=timerValue[MEASUREMENTS_NUMBER];
  if (nm<1)
    nm=1;

 
  int  np=getNumberOfPositions();
  if (np<1)
    np=1;
  
  
  int ns0=1;
  if (*actionMask & (1 << MAX_ACTIONS)) {
    ns0=getScanSteps(0);
    if (ns0<1)
      ns0=1;
  }

  int ns1=1;
  if (*actionMask & (1 << (MAX_ACTIONS+1))) {
    ns1=getScanSteps(1);
    if (ns1<1)
      ns1=1;
  }

  if(receiver){
    pthread_mutex_lock(&mg); //cout << "lock"<< endl;
	  if(getReceiverStatus()!=IDLE)
		  stopReceiver();
	  //multi detectors shouldnt have different receiver read frequencies enabled/disabled
    	if(setReadReceiverFrequency(0) < 0){
	  		std::cout << "Error: The receiver read frequency is invalid:" << setReadReceiverFrequency(0) << std::endl;
	  		*stoppedFlag=1;
	  	}
	  if(setReceiverOnline()==OFFLINE_FLAG)
		  *stoppedFlag=1;
	  pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;
  }


  if (*threadedProcessing)
	    startThread(delflag);
#ifdef VERBOSE
  cout << " starting thread " << endl;
#endif

  //resets frames caught in receiver
  if(receiver){
    pthread_mutex_lock(&mg); //cout << "lock"<< endl;
    resetFramesCaught();
    pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;
  }


  for(int im=0;im<nm;im++) {

#ifdef VERBOSE
    cout << " starting measurement "<< im << " of " << nm << endl;
#endif


    //cout << "data thread started " << endl;
 

    //loop measurements

//     pthread_mutex_lock(&mp);
//     setStartIndex(*fileIndex);
//     pthread_mutex_unlock(&mp);
 
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

	ResetPositionIndex();
     
	for (int ip=0; ip<np; ip++) {

	  //   cout << "positions " << endl;
	  if (*stoppedFlag==0) {
	    if  (getNumberOfPositions()>0) {
	      moveDetector(detPositions[ip]);
	      IncrementPositionIndex();
#ifdef VERBOSE
	      std::cout<< "moving to position" << std::endl;
#endif
	    } 
	  } else
	    break;
       
       
	  pthread_mutex_lock(&mp);
	  	  createFileName();
	  pthread_mutex_unlock(&mp);

	  if (*stoppedFlag==0) {
	    executeAction(scriptBefore);
	  } else
	    break;
       
       
       

	  if (*stoppedFlag==0) {


	    executeAction(headerBefore);
	 
	    if (*correctionMask&(1<< ANGULAR_CONVERSION) || aclog){// || eclog) { 
	      positionFinished(0);
	      setCurrentPosition(getDetectorPosition());
	    }
      

	    if (aclog)
	    	aclog->addStep(getCurrentPosition(), getCurrentFileName());

	    if (eclog)
		eclog->addStep(setDAC(-1,THRESHOLD,0), getCurrentFileName());


	    if (*correctionMask&(1<< I0_NORMALIZATION)) {
	      if (get_i0)
		get_i0(0, IOarg);
	    }

	    setCurrentFrameIndex(0);

	    if(receiver)
	      pthread_mutex_lock(&mg); //cout << "lock"<< endl;
	    if (multiframe>1)
	      setFrameIndex(0);
	    else
	      setFrameIndex(-1);


	    if(receiver){
	      pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;
	    	pthread_mutex_lock(&mp);
	    	createFileName();
	    	pthread_mutex_unlock(&mp);
	    	//send receiver file name
	    	pthread_mutex_lock(&mg); //cout << "lock"<< endl;
	    	setFileName(fileIO::getFileName());

	    	//start receiver
	    	if(startReceiver() == FAIL) {
	    		cout << "Start receiver failed " << endl;
	    		stopReceiver();
	    		*stoppedFlag=1;
	    		pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;
	    		break;
	    	}
#ifdef VERBOSE
		  cout << "Receiver started " << endl;
#endif
		  pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;

			//let processing thread listen to these packets
			sem_post(&sem_newRTAcquisition);
	    }
	    #ifdef VERBOSE
	    cout << "Acquiring " << endl;
	   #endif
	    startAndReadAll();
#ifdef VERBOSE
	    cout << "detector finished " << endl;
#endif
	   #ifdef VERBOSE
	    cout << "returned! " << endl;
	   #endif
       


	    if (*correctionMask&(1<< I0_NORMALIZATION)) {
	      if (get_i0) 
		currentI0=get_i0(1,IOarg); // this is the correct i0!!!!!
	    }
#ifdef VERBOSE
	    cout << "pos finished? " << endl;
#endif     

	    positionFinished(1);
       
#ifdef VERBOSE
	    cout << "done! " << endl;
#endif
 

	    if (*threadedProcessing==0){
#ifdef VERBOSE
	      cout << "start unthreaded process data " << endl;
#endif

	      processData(delflag); 
	    } 

	  } else
	    break;

	  while (dataQueueSize()) usleep(100000);
	  // cout << "mglock " << endl;;
	  pthread_mutex_lock(&mg); //cout << "lock"<< endl;
	  // cout << "done " << endl;;
	  //offline
	  if(setReceiverOnline()==OFFLINE_FLAG){
		  if ((getDetectorsType()==GOTTHARD) || (getDetectorsType()==MOENCH) || (getDetectorsType()==JUNGFRAU)|| (getDetectorsType()==JUNGFRAUCTB) ){
			  if((*correctionMask)&(1<<WRITE_FILE))
				  closeDataFile();
		  }
	  }
	  //online
	  else{

		  if(setReceiverOnline(ONLINE_FLAG)!=ONLINE_FLAG){
			  stopAcquisition();
			  stopReceiver();
			  pthread_mutex_unlock(&mg);
			  break;
		  }
		  stopReceiver();
		  //	  cout<<"***********receiver stopped"<<endl;
	  }
	  pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;



	  pthread_mutex_lock(&mp);
	  if (*stoppedFlag==0) {
	    executeAction(headerAfter);

		  pthread_mutex_unlock(&mp);
	    // setLastIndex(*fileIndex);
	  } else {
	    //   setLastIndex(*fileIndex);

		pthread_mutex_unlock(&mp);
	    break;
	  }


	  if (*stoppedFlag) {
#ifdef VERBOSE
	    std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
	    break;
	  } else if (ip<(np-1)) {
// 	    pthread_mutex_lock(&mp);
// 	    *fileIndex=setStartIndex(); 
// 	    pthread_mutex_unlock(&mp);
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
// 	  pthread_mutex_lock(&mp);
// 	  *fileIndex=setStartIndex(); 
// 	  pthread_mutex_unlock(&mp);
	}
      } 

      //end scan1 loop is1
  

      if (*stoppedFlag) {
#ifdef VERBOSE
	std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
	break;
      } else if (is0<(ns0-1)) {
// 	pthread_mutex_lock(&mp);
// 	*fileIndex=setStartIndex();  
// 	pthread_mutex_unlock(&mp);
      }

    } //end scan0 loop is0

//     pthread_mutex_lock(&mp);
//     *fileIndex=setLastIndex();  
//     pthread_mutex_unlock(&mp);

    if (*stoppedFlag==0) {
      executeAction(stopScript);
    } else{

#ifdef VERBOSE
    cout << "findex incremented " << endl;
 #endif
    if(*correctionMask&(1<<WRITE_FILE))
      IncrementFileIndex();
    pthread_mutex_lock(&mg); //cout << "lock"<< endl;
    setFileIndex(fileIO::getFileIndex());
    pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;

      break;
    }


#ifdef VERBOSE
  cout << "findex incremented " << endl;
#endif
  if(*correctionMask&(1<<WRITE_FILE))
    IncrementFileIndex();
  pthread_mutex_lock(&mg); //cout << "lock"<< endl;
  setFileIndex(fileIO::getFileIndex());
  pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;

    if (measurement_finished){
      pthread_mutex_lock(&mg); //cout << "lock"<< endl;
    	  measurement_finished(im,*fileIndex,measFinished_p);
    	  pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;
    }

    if (*stoppedFlag) {
      break;
    } 


    // loop measurements
  }



  // waiting for the data processing thread to finish!
  if (*threadedProcessing) {
#ifdef VERBOSE
    cout << "wait for data processing thread" << endl;
#endif
    setJoinThread(1);

	//let processing thread continue and checkjointhread
	sem_post(&sem_newRTAcquisition);

    pthread_join(dataProcessingThread, &status);
#ifdef VERBOSE
    cout << "data processing thread joined" << endl;
#endif
  }


  if(progress_call)
	progress_call(getCurrentProgress(),pProgressCallArg);


  if (connectChannels) {
    if (disconnect_channels)
      disconnect_channels(DCarg);
  }
   
  if (aclog)
    delete aclog;
   
  if (eclog)
    delete eclog;
#ifdef VERBOSE
  cout << "acquisition finished callback " << endl;
#endif
  if (acquisition_finished)
    acquisition_finished(getCurrentProgress(),getDetectorStatus(),acqFinished_p);
#ifdef VERBOSE
  cout << "acquisition finished callback done " << endl;
#endif

  setAcquiringFlag(false);
  sem_destroy(&sem_newRTAcquisition);

  clock_gettime(CLOCK_REALTIME, &end);
  cout << "Elapsed time for acquisition:" << (( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0) << " seconds" << endl;

  return OK;

}





//Naveen change

int slsDetectorUtils::setTotalProgress() {
  
  int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1, nm=1;

  if (timerValue[FRAME_NUMBER])
    nf=timerValue[FRAME_NUMBER];

  if (timerValue[CYCLES_NUMBER]>0)
    nc=timerValue[CYCLES_NUMBER];

  if (timerValue[MEASUREMENTS_NUMBER]>0)
    nm=timerValue[MEASUREMENTS_NUMBER];

  if (*numberOfPositions>0)
    npos=*numberOfPositions;

  if ((nScanSteps[0]>0) && (*actionMask & (1 << MAX_ACTIONS)))
    nscan[0]=nScanSteps[0];

  if ((nScanSteps[1]>0) && (*actionMask & (1 << (MAX_ACTIONS+1))))
    nscan[1]=nScanSteps[1];
      
  totalProgress=nm*nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
  cout << "nc " << nc << endl;
  cout << "nm " << nm << endl;
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
  //int interrupt=0;
  //int ich;
  //int chmin,chmax;
#ifdef VERBOSE
  std::cout << "utils: Setting bad channel correction to " << fname << std::endl;
#endif
 // int modmi=0;
  int modma=1;
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




double slsDetectorUtils::getCurrentProgress() {
  pthread_mutex_lock(&mp);
#ifdef VERBOSE
  cout << progressIndex << " / " << totalProgress << endl;
#endif
  
  double p=100.*((double)progressIndex)/((double)totalProgress);
  pthread_mutex_unlock(&mp);
  return p;
}



void slsDetectorUtils::incrementProgress()  {
  pthread_mutex_lock(&mp);
  progressIndex++;
  cout << fixed << setprecision(2) << setw (6) << 100.*((double)progressIndex)/((double)totalProgress) << " \%";
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout << endl;
#else
  cout << "\r" << flush;
#endif

};


void slsDetectorUtils::setCurrentProgress(int i){
  pthread_mutex_lock(&mp);
  progressIndex=i;
  cout << fixed << setprecision(2) << setw (6) << 100.*((double)progressIndex)/((double)totalProgress) << " \%";
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout << endl;
#else
  cout << "\r" << flush;
#endif
}


int slsDetectorUtils::retrieveDetectorSetup(string const fname1, int level){



  slsDetectorCommand *cmd;


 // char ext[100];
  int skip=0;
  string fname;
  string str;
  ifstream infile;
  int iargval;
  int interrupt=0;
  char *args[10];

  char myargs[10][1000];

  //args[0]=myargs[0];
  //args[1]=myargs[1];

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
	  args[iargval]=myargs[iargval];
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

  if (getErrorMask())
	  return FAIL;

  return OK;


}


int slsDetectorUtils::dumpDetectorSetup(string const fname, int level){

  slsDetectorCommand *cmd;
  string names[100];
  int nvar=0;

  names[nvar++]="fname";
  names[nvar++]="index";
  names[nvar++]="dr";
  names[nvar++]="settings";
  names[nvar++]="exptime";
  names[nvar++]="period";
  names[nvar++]="frames";
  names[nvar++]="cycles";
  names[nvar++]="measurements";
  names[nvar++]="timing";

  switch (getDetectorsType()) {
  case EIGER:
    names[nvar++]="flags";
		names[nvar++]="threshold";
		names[nvar++]="ratecorr";
		break;
  case GOTTHARD:
  case PROPIX:
  case JUNGFRAU:
  names[nvar++]="flags";
		names[nvar++]="delay";
		names[nvar++]="gates";
		names[nvar++]="ratecorr";
		break;
  case MYTHEN:
    names[nvar++]="flags";
		names[nvar++]="threshold";
		names[nvar++]="delay";
		names[nvar++]="gates";
		names[nvar++]="probes";
		names[nvar++]="fineoff";
		names[nvar++]="ratecorr";
		names[nvar++]="trimbits";
		break;
  case JUNGFRAUCTB:

  names[nvar++]="dac:0";
  names[nvar++]="dac:1";
  names[nvar++]="dac:2";
  names[nvar++]="dac:3";
  names[nvar++]="dac:4";
  names[nvar++]="dac:5";
  names[nvar++]="dac:6";
  names[nvar++]="dac:7";
  names[nvar++]="dac:8";
  names[nvar++]="dac:9";
  names[nvar++]="dac:10";
  names[nvar++]="dac:11";
  names[nvar++]="dac:12";
  names[nvar++]="dac:13";
  names[nvar++]="dac:14";
  names[nvar++]="dac:15";
  names[nvar++]="adcvpp";



  names[nvar++]="adcclk";
  names[nvar++]="clkdivider";
  names[nvar++]="adcphase";
  names[nvar++]="adcpipeline";
  names[nvar++]="adcinvert"; //
  names[nvar++]="adcdisable"; 
  names[nvar++]="patioctrl"; 
  names[nvar++]="patclkctrl"; 
  names[nvar++]="patlimits";
  names[nvar++]="patloop0"; 
  names[nvar++]="patnloop0";
  names[nvar++]="patwait0"; 
  names[nvar++]="patwaittime0"; 
  names[nvar++]="patloop1"; 
  names[nvar++]="patnloop1";
  names[nvar++]="patwait1";
  names[nvar++]="patwaittime1";
  names[nvar++]="patloop2"; 
  names[nvar++]="patnloop2";
  names[nvar++]="patwait2"; 
  names[nvar++]="patwaittime2"; 
  break;
  default:
	  break;
  }



  names[nvar++]="startscript";
  names[nvar++]="startscriptpar";
  names[nvar++]="stopscript";
  names[nvar++]="stopscriptpar";
  names[nvar++]="scriptbefore";
  names[nvar++]="scriptbeforepar";
  names[nvar++]="scriptafter";
  names[nvar++]="scriptafterpar";
  names[nvar++]="scan0script";
  names[nvar++]="scan0par";
  names[nvar++]="scan0prec";
  names[nvar++]="scan0steps";
  names[nvar++]="scan1script";
  names[nvar++]="scan1par";
  names[nvar++]="scan1prec";
  names[nvar++]="scan1steps";

  switch (getDetectorsType()) {
  case EIGER:
  case MYTHEN:
  case GOTTHARD:
  case PROPIX:
  case JUNGFRAU:
  names[nvar++]="flatfield";
  names[nvar++]="badchannels";
  break;
  default:
	  break;
  }

 switch (getDetectorsType()) {
  case EIGER:
  case MYTHEN:
  names[nvar++]="trimbits";
  break;
  default:
	  break;
  }

 // char ext[100];

  int iv=0;
  string fname1;



  ofstream outfile;
  char *args[4];
  for (int ia=0; ia<4; ia++) {
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
    cmd=new slsDetectorCommand(this);
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



    if (level==2) {
      strcpy(args[0],names[iv].c_str());
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
      outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
      iv++;
  

    }

 
    delete cmd;

    outfile.close();
  }
  else {
    std::cout<< "Error opening parameters file " << fname1 << " for writing" << std::endl;
    return FAIL;
  }
  
#ifdef VERBOSE
  std::cout<< "wrote " <<iv << " lines to  "<< fname1 << std::endl;
#endif
  
  return OK;

} 


