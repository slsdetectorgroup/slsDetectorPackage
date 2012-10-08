#include "postProcessing.h"
#include "postProcessingFuncs.h"
#include "angleConversionConstant.h"
#ifdef VERBOSE
#include "usersFunctions.h"
#endif



postProcessing::postProcessing(){							
  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER;
  mp=mp1;
  pthread_mutex_init(&mp, NULL);  
  mg=mp1;
  pthread_mutex_init(&mg, NULL);  
  //cout << "reg callback "<< endl;
  dataReady = 0;
  pCallbackArg = 0; 
#ifdef VERBOSE
  registerDataCallback(&defaultDataReadyFunc,  NULL);
#endif
  //cout << "done "<< endl;
  rawDataReady = 0;
  pRawDataArg = 0; 

  ppFun=new postProcessingFuncs();
}





postProcessing::~postProcessing(){delete ppFun;};








void postProcessing::processFrame(int *myData, int delflag) {

  string fname;
  // double *fdata=NULL;
  
#ifdef VERBOSE
      cout << "start processing"<< endl;
#endif

  incrementProgress();

#ifdef VERBOSE
      cout << "prog incremented"<< endl;
#endif
 
 /** decode data */
 
 fdata=decodeData(myData, fdata);
 
#ifdef VERBOSE
      cout << "decode"<< endl;
#endif
 
 
 fname=createFileName();
 
 //Checking for write flag
 if(*correctionMask&(1<<WRITE_FILE)) {
   
#ifdef VERBOSE
   cout << "writing raw data " << endl;
   
#endif
   //uses static function?!?!?!?
   writeDataFile (fname+string(".raw"),fdata, NULL, NULL, 'i'); 
   
#ifdef VERBOSE
   cout << "done " << endl;
   
#endif
 } 
 if (rawDataReady)
   rawDataReady(fdata,pRawDataArg);
 
 if (*correctionMask & ~(1<<WRITE_FILE))
   doProcessing(fdata,delflag, fname);

 delete [] myData;
 delete [] fdata;
 myData=NULL;
 fdata=NULL;

#ifdef VERBOSE
  cout << "Pop data queue " << *fileIndex << endl;
#endif
  
  if(*correctionMask&(1<<WRITE_FILE))
    IncrementFileIndex();
 
  popDataQueue(); //remove the data from the queue
  queuesize=dataQueueSize();
  
  
}




void postProcessing::doProcessing(double *lfdata, int delflag, string fname) {

  double *ang=new double[arraySize];
  double *val=new double[arraySize];
  double *err=new double[arraySize];
  int np;
  detectorData *thisData;

  int npos=getNumberOfPositions();

  string ext=".dat";
  
    double t=(*expTime)*1E-9;
    if (GetCurrentPositionIndex()<=1) {
#ifdef VERBOSE
      cout << "init dataset" << endl;
#endif
      ppFun->initDataset();
    }
    
#ifdef VERBOSE
      cout << "add frame" << endl;
#endif
      
    ppFun->addFrame(lfdata, &currentPosition, &currentI0, &t, (fname+ext).c_str(), NULL);      
  
    if ((GetCurrentPositionIndex()>=npos && positionFinished() && dataQueueSize()) || npos==0) {
      
#ifdef VERBOSE
      cout << "finalize dataset" << endl;
#endif
      
	ppFun->finalizeDataset(ang, val, err, &np);
	IncrementPositionIndex();
	
	pthread_mutex_lock(&mp);
	fname=createFileName();
	pthread_mutex_unlock(&mp);
	
	if(*correctionMask&(1<<WRITE_FILE))
	  writeDataFile (fname+ext,np,ang, val, err,'f');

	thisData=new detectorData(ang,val,err,getCurrentProgress(),(fname+ext).c_str(),np);

	if (dataReady) {
	  dataReady(thisData, pCallbackArg);
	}
	delete thisData;
    }
}





int postProcessing::fillBadChannelMask() {

  int nbad=0;

  if (*correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    nbad=getBadChannelCorrection();
#ifdef VERBOSE
    cout << "number of bad channels is " << nbad << endl;
#endif
    if (nbad>0) {
      
      int *badChansList=new int[nbad];
      getBadChannelCorrection(badChansList);

      if (badChannelMask) 
	delete [] badChannelMask;
      badChannelMask=new int[getTotalNumberOfChannels()];

#ifdef VERBOSE
      cout << " pointer to bad channel mask is " << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<getTotalNumberOfChannels(); ichan++)
	badChannelMask[ichan]=0;
#ifdef VERBOSE
      cout << " badChanMask has be reset" << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<nbad; ichan++) {
	if (badChansList[ichan]<getTotalNumberOfChannels() && badChansList[ichan]>=0 ) {
	  if (badChannelMask[badChansList[ichan]]==0)
	    nbad++;
	  badChannelMask[badChansList[ichan]]=1;
	  
	}
      }
      delete [] badChansList;

    } else {
      if (badChannelMask) {
#ifdef VERBOSE
      cout << "deleting bad channel mask beacuse number of bad channels is 0" << endl;
#endif
      
      delete [] badChannelMask;
      badChannelMask=NULL;
      }
    }
    
  } else {
#ifdef VERBOSE
    cout << "bad channel correction is disabled " << nbad << endl;
#endif
    if (badChannelMask) {
#ifdef VERBOSE
      cout << "deleting bad channel mask beacuse no bad channel correction is selected" << endl;
#endif
      //delete [] badChannelMask;
      //badChannelMask=NULL;
    }
  }
  
#ifdef VERBOSE
  cout << "number of bad channels is " << nbad << endl;
#endif
  return  nbad;

}






void* postProcessing::processData(int delflag) {


#ifdef VERBOSE
  std::cout<< " processing data - threaded mode " << *threadedProcessing << endl;
#endif


 
  queuesize=dataQueueSize();

  int *myData;
  int dum=1;

  fdata=NULL;

  while(dum | *threadedProcessing) { // ????????????????????????
    /* IF THERE ARE DATA PROCESS THEM*/
    while((queuesize=dataQueueSize())>0) {
      /** Pop data queue */
#ifdef VERBOSE
      cout << "data found"<< endl;
#endif

      myData=dataQueueFront(); // get the data from the queue 
#ifdef VERBOSE
      cout << "got them"<< endl;
#endif

      if (myData) {
	processFrame(myData,delflag);
      }
    }
   
    /* IF THERE ARE NO DATA look if acquisition is finished */
    if (checkJoinThread()) {
      if (dataQueueSize()==0) {
	break;
      }
    } 
    dum=0;
  }

  if (fdata) {
    delete [] fdata;
  }
  return 0;
}



int postProcessing::checkJoinThread() {
  int retval;
  pthread_mutex_lock(&mp);
  retval=jointhread;
  pthread_mutex_unlock(&mp);
  return retval;
}

void postProcessing::setJoinThread( int v) {
  pthread_mutex_lock(&mp);
  jointhread=v;
  pthread_mutex_unlock(&mp);
}

int* postProcessing::dataQueueFront() {
  int *retval=NULL;
  pthread_mutex_lock(&mp);
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
  }
  pthread_mutex_unlock(&mp);
  return retval;
}
int postProcessing::dataQueueSize() {
  int retval;
  pthread_mutex_lock(&mp);
  retval=dataQueue.size();
  pthread_mutex_unlock(&mp);
  return retval;
}


int* postProcessing::popDataQueue() {
  int *retval=NULL;
  pthread_mutex_lock(&mp);
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  pthread_mutex_unlock(&mp);
  return retval;
}

detectorData* postProcessing::popFinalDataQueue() {
  detectorData *retval=NULL;
  pthread_mutex_unlock(&mg);
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  pthread_mutex_unlock(&mg);
  return retval;
}

void postProcessing::resetDataQueue() {
  int *retval=NULL;
  pthread_mutex_lock(&mp);
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
  pthread_mutex_unlock(&mp);
 
}

void postProcessing::resetFinalDataQueue() {
  detectorData *retval=NULL;
  pthread_mutex_lock(&mg);
  while( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
    delete retval;
  }
  pthread_mutex_unlock(&mg);
}


void postProcessing::startThread(int delflag) {

  /////////////////////////////////// Initialize dataset

  //resetDataQueue();

  setTotalProgress();

  int nmod=getNMods();
  int *chPM=new int[nmod];
  int *mM=new int[nmod];
  int totch=0;
#ifdef VERBOSE
  cout << "init dataset stuff" << endl;
#endif

  for (int im=0; im<nmod; im++) {
#ifdef VERBOSE
    cout << "MODULE " << im << endl;
#endif
    chPM[im]=getChansPerMod(im);
#ifdef VERBOSE
    cout << "chanspermod"  << endl;
#endif
    mM[im]=getMoveFlag(im);
#ifdef VERBOSE
    cout << "moveflag"  << endl;
#endif
    totch+=chPM[im];
  }

#ifdef VERBOSE
  cout << "total channels is " << totch << endl;
#endif
  double *ffcoeff=NULL, *fferr=NULL;

  if (*correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
#ifdef VERBOSE
  cout << "get ff "  << endl;
#endif
    ffcoeff=new double[totch];
    fferr=new double[totch];

    getFlatFieldCorrection(ffcoeff,fferr);
  }

  double tdead;
  if (*correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
  cout << "get tau "  << endl;
#endif
  tdead=getRateCorrectionTau();
  }   else
    tdead=0;


  int angdir=getAngularDirection();

  double to=0;
  double bs=0;
  double sx=0, sy=0;
  double *angRad=NULL;
  double *angOff=NULL;
  double *angCenter=NULL;
  angleConversionConstant *p=NULL;
  
  if (*correctionMask&(1<< ANGULAR_CONVERSION)) {
#ifdef VERBOSE
  cout << "get angconv "  << endl;
#endif
    bs=getBinSize();
    to=getGlobalOffset()+getFineOffset();
    angRad=new double[nmod];
    angOff=new double[nmod];
    angCenter=new double[nmod];
    for (int im=0; im<nmod; im++) {
      p=getAngularConversionPointer(im);
      angRad[im]=p->r_conversion;
      angOff[im]=p->offset;
      angCenter[im]=p->center;
    }
    sx=getAngularConversionParameter(SAMPLE_X);
    sy=getAngularConversionParameter(SAMPLE_Y);

  }


#ifdef VERBOSE
  cout << "init dataset"  << endl;
#endif
  ppFun->initDataset(&nmod,chPM,mM,badChannelMask, ffcoeff, fferr, &tdead, &angdir, angRad, angOff, angCenter, &to, &bs, &sx, &sy);
  
#ifdef VERBOSE
  cout << "done"  << endl;
#endif


  if (*correctionMask&(1<< ANGULAR_CONVERSION)) {
    arraySize=getNumberOfAngularBins();
    if (arraySize<=0)
      arraySize=totch;
  } else {
    arraySize=totch;
  }

  queuesize=dataQueueSize();

  resetFinalDataQueue();
  resetDataQueue();


  /////////////////////////////////// Start thread ////////////////////////////////////////////////////////
#ifdef VERBOSE
  cout << "start thread stuff"  << endl;
#endif
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



