#include "postProcessing.h"
#include "postProcessingFuncs.h"
#include "angleConversionConstant.h"
#ifdef VERBOSE
#include "usersFunctions.h"
#elif EXTPP
#include "usersFunctions.h"
#endif
#include <time.h>


//#define VERBOSE

static void* startProcessData(void *n){
   postProcessing *myDet=(postProcessing*)n;
   myDet->processData(1);
   pthread_exit(NULL);
   
};

static void* startProcessDataNoDelete(void *n){
  postProcessing *myDet=(postProcessing*)n;
  myDet->processData(0);
  pthread_exit(NULL);

};


int postProcessing::kbhit(){
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}


postProcessing::postProcessing(): expTime(NULL), badChannelMask(NULL), ang(NULL),  val(NULL), err(NULL), numberOfChannels(0) {
  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER;
  mp=mp1;
  pthread_mutex_init(&mp, NULL);  
  mg=mp1;
  pthread_mutex_init(&mg, NULL);
  ms=mp1;
  pthread_mutex_init(&ms, NULL);

  //cout << "reg callback "<< endl;
  dataReady = 0;
  pCallbackArg = 0; 
  //cout << "done "<< endl;
  rawDataReady = 0;
  pRawDataArg = 0;


  //#ifdef VERBOSE
  //  registerDataCallback(&defaultDataReadyFunc,  NULL);
  //#endif
#ifdef EXTPP
  registerRawDataCallback(&defaultRawDataReadyFunc,  NULL);
#endif

  ppFun=new postProcessingFuncs();

}





postProcessing::~postProcessing(){
  delete ppFun;
};








void postProcessing::processFrame(int *myData, int delflag, int jctb) {

  string fname;
  int nn;
  //double *fdata=NULL;
  
#ifdef VERBOSE
  cout << "start processing"<< endl;
#endif
  
  incrementProgress();
  
#ifdef VERBOSE
  cout << "prog incremented"<< endl;
#endif
 
 /** decode data */
 
  // cout << "decode 0"<< endl;
 // if (getDetectorsType()==MYTHEN) {

  
  // for (int ib=0; ib<1000; ib++)
  //   cout << ((*(((u_int64_t*)myData)+ib))>>17&1) ;
  // cout << endl;


  fdata=decodeData(myData,nn, fdata);
    
  //#ifdef VERBOSE
    // cout << "decode 1"<< endl;
    //#endif
    // } else
  //  fdata=NULL;
  if (rawDataReady) {
#ifdef VERBOSE
    cout << "decoded data size is "<<nn << endl;
    cout << "raw data ready..." << endl;
#endif
    // rawDataReady(fdata,numberOfChannels, pRawDataArg);
     rawDataReady(fdata,nn, pRawDataArg);
#ifdef VERBOSE
    cout << "done" << endl;
    cout << "NO FILE WRITING AND/OR DATA PROCESSING DONE BY SLS DETECTOR SOFTWARE!!!" << endl;   
#endif
  } else {
 

    pthread_mutex_lock(&mp);
    fname=createFileName();
    pthread_mutex_unlock(&mp);
#ifdef VERBOSE
    cout << "fname is " << fname << endl;
#endif

    //Checking for write flag
    if((*correctionMask)&(1<<WRITE_FILE)) {
#ifdef VERBOSE
     cout << "writing raw data " << endl;
     
#endif
     if (getDetectorsType()==MYTHEN){
    // if (fdata) {
       //uses static function?!?!?!?
       // cout << "write to file?!?!?!?" << endl;
       writeDataFile (fname+string(".raw"),fdata, NULL, NULL, 'i');
     } else {
       writeDataFile ((void*)myData, currentFrameIndex);
     }
#ifdef VERBOSE
     cout << "done " << endl;    
#endif
    }

    if ((*correctionMask) & ~((1<<WRITE_FILE) | (1<<OVERWRITE_FILE))) {
      //  cout << "cmask is not 0: " << *correctionMask << " - " <<  ((*correctionMask) & (1<<I0_NORMALIZATION)) << "-" <<   ((*correctionMask) & (1<<OVERWRITE_FILE)) << endl;
      doProcessing(fdata,delflag, fname);
    } else
      if (dataReady){

	//	cout << "callback arg "<< getCurrentProgress()<< " " << (fname+string(".raw")).c_str() << " " << getTotalNumberOfChannels() << endl;
	//	cout << "DATAREADY 1" <<endl;
	//	thisData=new detectorData(fdata,NULL,NULL,getCurrentProgress(),(fname+string(".raw")).c_str(),getTotalNumberOfChannels()); //only 1d detectors
	thisData=new detectorData(fdata,NULL,NULL,getCurrentProgress(),(fname+string(".raw")).c_str(),nn); //only 1d detectors
	dataReady(thisData, currentFrameIndex, -1, pCallbackArg);
	delete thisData;
	fdata=NULL;
      }
  }

  incrementCurrentFrameIndex();
  if (getFrameIndex()>=0){
	  if (getDetectorsType()==MYTHEN)
		  incrementFrameIndex(1);
	  else if((currentFrameIndex%getFramesPerFile())==0)
		  incrementFrameIndex(getFramesPerFile());
  }




 if (fdata)
   delete [] fdata;
 fdata=NULL;
 

 // if (jctb==0) {
   delete [] myData;
   myData=NULL;
   
#ifdef VERBOSE
   cout << "Pop data queue " << *fileIndex << endl;
#endif
   
   popDataQueue(); //remove the data from the queue
   
#ifdef VERBOSE
  cout << "Data queue popped"  << endl;
#endif
  
  
#ifdef VERBOSE
  cout << "process frame returning "  << endl;
#endif
  // }
  
}




void postProcessing::doProcessing(double *lfdata, int delflag, string fname) {
  
  #ifdef VERBOSE
  cout << "??????????????????????????????????????????? do processing - data size is " << arraySize << endl;
 #endif


  int np;

#ifdef VERBOSE
      cout << "arrays allocated " << endl;
#endif
  int npos=getNumberOfPositions();

  string ext=".dat";
  
#ifdef VERBOSE
  cout << "npos is "<< npos << endl;
#endif

  double t=0;

  if (expTime)
    t=(*expTime)*1E-9;
  else
   cout << "no pointer to exptime" << endl;
#ifdef VERBOSE
  cout << "exptime is "<< t << endl;
#endif

  if (GetCurrentPositionIndex()<=1 || npos<2) {
#ifdef VERBOSE
      cout << "init dataset" << endl;
#endif


  if (*correctionMask&(1<< ANGULAR_CONVERSION))
    ang=new double[arraySize];
  else
    ang=NULL;
  
  val=new double[arraySize];
  err=new double[arraySize];

  initDataset(0);
  }
    
#ifdef VERBOSE
      cout << "add frame" << endl;
#endif

      /**ot them
start processing
prog incremented
decode
fname is //run_f0_0
??????????????????????????????????????????? do processing - data size is 30720
arrays allocated
npos is 0
exptime is 10.00
init dataset
add frame
data queue size lock
data queue size unlock
      **/

      addFrame(lfdata,currentPosition, currentI0, t, fname, 0);    
      //  cout << "++++++++++++++++++++" << GetCurrentPositionIndex() << " " << npos << " " << positionFinished() << " " << dataQueueSize() << endl;
      if ((GetCurrentPositionIndex()>=npos  && dataQueueSize()) || npos<2) {
	//&& 

	while(positionFinished()==0 && npos>1) {
	  ;
	}
#ifdef VERBOSE
	cout << "finalize dataset" << endl;
#endif
	
	finalizeDataset(ang, val, err, np);
	//if (npos<2) {
	IncrementPositionIndex();
	
	pthread_mutex_lock(&mp);
	fname=createFileName();
	pthread_mutex_unlock(&mp);
	  //}
      
	if((*correctionMask)&(1<<WRITE_FILE)) {  
	  writeDataFile (fname+ext,np,val, err,ang,'f');
	}   


	if (dataReady) {
	  //	  cout << "callback arg "<< getCurrentProgress()<< " " << (fname+ext).c_str() << " " << np << endl;
	  //cout << "ADATREADY 2 " << endl;
	  thisData=new detectorData(val,err,ang,getCurrentProgress(),(fname+ext).c_str(),np);//only 1d detectors
	  dataReady(thisData, currentFrameIndex, -1, pCallbackArg);
	  delete thisData;
	  ang=NULL;
	  val=NULL;
	  err=NULL;
	}

	if (ang)
	  delete [] ang;
	if (val)
	  delete [] val;
	if (err)
	  delete [] err;
	
      }
}





int postProcessing::fillBadChannelMask() {
#ifdef VERBOSE
  cout << "pp fill bad channel mask " << endl;
#endif
  int nbad=0, nb;
  //#define VERBOSE
  if (*correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    nbad=getBadChannelCorrection();
#ifdef VERBOSE
    cout << "number of bad channels is " << nbad << endl;
#endif
    nb=nbad;
    if (nbad>0) {
      
      int *badChansList=new int[nbad];
 #ifdef VERBOSE
    cout << "get badch array " << nbad << endl;
 #endif
      getBadChannelCorrection(badChansList);
 #ifdef VERBOSE
    cout << "done " << nbad << endl;
 #endif

      if (badChannelMask) 
	delete [] badChannelMask;


#ifdef VERBOSE
      cout << " nchans " << getTotalNumberOfChannels()  << endl;
#endif

      badChannelMask=new int[getTotalNumberOfChannels()];

#ifdef VERBOSE
      cout << " pointer to bad channel mask is " << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<getTotalNumberOfChannels(); ichan++)
	badChannelMask[ichan]=0;
#ifdef VERBOSE
      cout << " badChanMask has be reset" << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<nb; ichan++) {
	if (badChansList[ichan]<getTotalNumberOfChannels() && badChansList[ichan]>=0 ) {
	  if (badChannelMask[badChansList[ichan]]==0) {
	    badChannelMask[badChansList[ichan]]=1;
	    //	    cout << "bad: " << ichan << " " << badChansList[ichan] << endl;
	  } else
	    nbad--;
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
      delete [] badChannelMask;
      badChannelMask=NULL;
    }
  }
  
#ifdef VERBOSE
  cout << "number of bad channels is " << nbad << endl;
#endif
  return  nbad;

}






void* postProcessing::processData(int delflag) {
  	pthread_mutex_lock(&mg);
	if(setReceiverOnline()==OFFLINE_FLAG){
	  pthread_mutex_unlock(&mg);

#ifdef VERBOSE
		std::cout<< " ??????????????????????????????????????????? processing data - threaded mode " << *threadedProcessing << endl;
#endif




		queuesize=dataQueueSize();


		int *myData;
		int dum=1;
//		int nf=1, ii, nch;
//		int jctb=0;


// 		if (getDetectorsType()==JUNGFRAUCTB) {
// 		  nch=getTotalNumberOfChannels();
// 		  nf= getDataBytes()/(nch*2);
// 		  cout << "WILL PROCESS " << nf << "SAMPLES AND " << nch <<"CHANNELS PER FRAME!" << endl;
// 		  jctb=1;
// 		}// else
// 		 // cout << "NOOT A JCTB" << endl;

		fdata=NULL;

		while(dum | *threadedProcessing) { // ????????????????????????
			/* IF THERE ARE DATA PROCESS THEM*/
		  //	  cout << "loop" << endl;
			while((queuesize=dataQueueSize())>0) {
				/** Pop data queue */
			  #ifdef VERBOSE
				cout << "data found"<< endl<<endl;;
			#endif

				myData=dataQueueFront(); // get the data from the queue
				//#ifdef VERBOSE
				  //	cout << "got them"<< endl;
				//#endif

				if (myData) {
				  

			// 	  if (jctb) {
// 				    char* p=(char*)myData;
// 				    for (ii=0; ii<nf; ii++) {
// 				      processFrame((int*)p,delflag, 1);
// 				      p+=2*nch;
// 				    }
				    
// 				    delete [] myData;
// 				    myData=NULL;
				    
// 				    popDataQueue(); //remove the data from the queue
   
// 				  } else 
				    processFrame(myData,delflag,0);
#ifdef VERBOSE
				  cout << "frame processed"<< endl;
#endif
					}
			
			
			}

			/* IF THERE ARE NO DATA look if acquisition is finished */
			if (checkJoinThread()) {
			  if (dataQueueSize()==0) {
			    break;
			  }
			}
			dum=0;
			usleep(500);
		}
		
		if (fdata) {
		  //cout << "delete fdata "<< endl;
		  delete [] fdata;
		  fdata = NULL;
		  //cout << "delete done "<< endl;
		}
	} //receiver 
	else{
 	  
 		 pthread_mutex_unlock(&mg);
 		//cprintf(RED,"In post processing threads\n");


 		if(dataReady){
 			readFrameFromReceiver();
 		}

 		//only update progress
 		else{
 			int caught = -1;
 			char c;
 			int ifp;
			while(true){

 				//cout.flush();
 				//cout<<flush;
 				usleep(100 * 1000); //20ms need this else connecting error to receiver (too fast)

 				if (checkJoinThread()){
 					break;
 				}



				ifp=kbhit();
				if (ifp!=0){
					c=fgetc(stdin);
					if (c=='q') {
						cout<<"Caught the command to stop acquisition"<<endl;
						stopAcquisition();
					}
				}



 				//get progress
 				pthread_mutex_lock(&mg);
 				if(setReceiverOnline() == ONLINE_FLAG){
 					caught = getFramesCaughtByReceiver();
 				}
 				pthread_mutex_unlock(&mg);


 				//updating progress
 				if(caught!= -1){
 					setCurrentProgress(caught);
 #ifdef VERY_VERY_DEBUG
 				cout << "caught:" << caught << endl;
 #endif
 				}
 				if (checkJoinThread()){
 					break;
 				}



 			}
 		}


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
  
#ifdef VERBOSE
  cout << "data queue size lock" << endl;
#endif


  pthread_mutex_lock(&mp);
  retval=dataQueue.size();
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout << "data queue size unlock" << endl;
#endif

  return retval;
}



int* postProcessing::popDataQueue() {
  int *retval=NULL;
#ifdef VERBOSE
  cout << "Pop data queue lock" << endl;
#endif

  pthread_mutex_lock(&mp);
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  queuesize=dataQueue.size();
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout <<  "Pop data queue lock" << endl;
#endif
  return retval;
}

// detectorData* postProcessing::popFinalDataQueue() {
//   detectorData *retval=NULL;
//   pthread_mutex_unlock(&mg);
//   if( !finalDataQueue.empty() ) {
//     retval=finalDataQueue.front();
//     finalDataQueue.pop();
//   }
//   pthread_mutex_unlock(&mg);
//   return retval;
// }

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

// void postProcessing::resetFinalDataQueue() {
//   detectorData *retval=NULL;
//   pthread_mutex_lock(&mg);
//   while( !finalDataQueue.empty() ) {
//     retval=finalDataQueue.front();
//     finalDataQueue.pop();
//     delete retval;
//   }
//   pthread_mutex_unlock(&mg);
// }
void postProcessing::initDataset(int r) {

  if (r) {
    
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
    cout << "moveflag"  << im << " " <<  mM[im] << endl;
#endif
    totch+=chPM[im];
  }
 

  // uncomment if to be used with antonio's funcs!!!
  //fillModuleMask(mM);



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
  if ((getDetectorsType()==MYTHEN) && (*correctionMask&(1<<RATE_CORRECTION))) {
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

#ifdef VERBOSE
      cout << im << " " << angCenter[im] << " "   << angRad[im] << " " << angOff[im] << endl;
#endif


    }
    sx=getAngularConversionParameter(SAMPLE_X);
    sy=getAngularConversionParameter(SAMPLE_Y);

  }


#ifdef VERBOSE
  cout << "init dataset"  << endl;
#endif
  // cout << "pp bad channel mask " << badChannelMask << endl;
  fillBadChannelMask();
  // cout << "pp bad channel mask " << badChannelMask << endl;
  
  //cout << "EEEEEEEEEEEEEEEEEEEE init dataset " << endl;
  ppFun->initDataset(&nmod,chPM,mM,badChannelMask, ffcoeff, fferr, &tdead, &angdir, angRad, angOff, angCenter, &to, &bs, &sx, &sy);
  
#ifdef VERBOSE
  cout << "done"  << endl;
#endif

  delete [] chPM;
  delete [] mM;
  if (ffcoeff != NULL)  delete [] ffcoeff;
  if (fferr != NULL)  delete [] fferr;
  if (angRad != NULL) delete [] angRad;
  if (angOff != NULL) delete [] angOff;
  if (angCenter != NULL) delete [] angCenter;


  if (*correctionMask&(1<< ANGULAR_CONVERSION)) {
    arraySize=getNumberOfAngularBins();
    if (arraySize<=0)
      arraySize=totch;
  } else {
    arraySize=totch;
  }

  numberOfChannels=totch;

  queuesize=dataQueueSize();

  //  resetFinalDataQueue();
  resetDataQueue();


  } else {

    // cout << "EEEEEEEEEEEEEEEEEEEE init dataset XXXX " << endl;

    ppFun->initDataset();


  }

}






void postProcessing::addFrame(double *data, double pos, double i0, double t, string fname, double var) {
  // cout << "EEEEEEEEEEEEEEEEEEEE add frame " << pos << " " << i0 << endl;

  if (*correctionMask&(1<< I0_NORMALIZATION))
    ppFun->addFrame(data, &pos, &i0, &t, fname.c_str(), &var);
  else
    ppFun->addFrame(data, &pos,NULL, &t, fname.c_str(), &var);
    


}

void postProcessing::finalizeDataset(double *a, double *v, double *e, int &np) {

  // cout << "EEEEEEEEEEEEEEEEEEEE finalize dataset " << endl;
  ppFun->finalizeDataset(a, v, e, &np);

}

















void postProcessing::startThread(int delflag) {

  /////////////////////////////////// Initialize dataset

  //resetDataQueue();

  setTotalProgress();

  initDataset(1);

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
    
  if (ret)
 	  printf("ret %d\n", ret);

  pthread_attr_destroy(&tattr);

  // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);
  
}



