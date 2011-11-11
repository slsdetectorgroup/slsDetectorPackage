/*******************************************************************

Date:       $Date$
Revision:   $Rev$
Author:     $Author$
URL:        $URL$
ID:         $Id$

********************************************************************/



#include "multiSlsDetector.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>




int multiSlsDetector::freeSharedMemory() {
  // Detach Memory address
    if (shmdt(thisMultiDetector) == -1) {
      perror("shmdt failed\n");
      return FAIL;
    }
    printf("Shared memory %d detached\n", shmId);
    // remove shared memory
    if (shmctl(shmId, IPC_RMID, 0) == -1) {
      perror("shmctl(IPC_RMID) failed\n");
      return FAIL;
    }
    printf("Shared memory %d deleted\n", shmId);
    return OK;

}



int multiSlsDetector::initSharedMemory(int id=0) {
  
  key_t     mem_key=DEFAULT_SHM_KEY+MAXDET+id;
  int       shm_id;
  int sz;



  sz=sizeof(sharedMultiSlsDetector);


   #ifdef VERBOSE
   std::cout<<"multiSlsDetector: Size of shared memory is "<< sz << std::endl;
#endif
   shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    std::cout<<"*** shmget error (server) ***"<< shm_id << std::endl;
    return shm_id;
  }
  
   /**
      thisMultiDetector pointer is set to the memory address of the shared memory
   */

  thisMultiDetector = (sharedMultiSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (thisMultiDetector == (void*)-1) {
    std::cout<<"*** shmat error (server) ***" << std::endl;
    return shm_id;
  }
    /**
      shm_id returns -1 is shared memory initialization fails
   */ 

  return shm_id;

}




multiSlsDetector::multiSlsDetector(int id) :  shmId(-1) 
{
  while (shmId<0) {
    shmId=initSharedMemory(id);
    id++;
  }
  id--;
  if (thisMultiDetector->alreadyExisting==0) {


    thisMultiDetector->onlineFlag=ONLINE;
    thisMultiDetector->numberOfDetectors=0;
    for (int id=0; id<MAXDET; id++) {
      thisMultiDetector->detectorIds[id]=-1;
    }
    thisMultiDetector->masterId=-1;
    thisMultiDetector->dataBytes=0;

    thisMultiDetector->alreadyExisting=1;
  }


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    detectors[i]=new slsDetector(thisMultiDetector->detectorIds[i]);
  }
  for (int i=thisMultiDetector->numberOfDetectors; i<MAXDET; i++)
    detectors[i]=NULL;
}

multiSlsDetector::~multiSlsDetector() {
  //removeSlsDetector();

}

int multiSlsDetector::addSlsDetector(int id, int pos) {
  int j=thisMultiDetector->numberOfDetectors;

  if (pos<0)
    pos=j;

  if (pos>j)
    return thisMultiDetector->numberOfDetectors;

  for (int ip=thisMultiDetector->numberOfDetectors-1; ip>pos; ip--) {
    thisMultiDetector->detectorIds[ip+1]=thisMultiDetector->detectorIds[ip];
    detectors[ip+1]=detectors[ip];
  }

  detectorType t=slsDetector::getDetectorType(id);
  detectors[pos]=new slsDetector(t,id);

  thisMultiDetector->numberOfDetectors++;
  thisMultiDetector->dataBytes+=detectors[pos]->getDataBytes();
  
  return thisMultiDetector->numberOfDetectors;

}



int multiSlsDetector::removeSlsDetector(int pos) {
  int j, found=0;
  
  if (pos<0 )
    pos=thisMultiDetector->numberOfDetectors-1;

  if (pos>=thisMultiDetector->numberOfDetectors)
    return thisMultiDetector->numberOfDetectors;

  j=pos;

  if (detectors[j]) {
    delete detectors[j];
    thisMultiDetector->numberOfDetectors--;
    
    

    
    for (int i=j+1; i<thisMultiDetector->numberOfDetectors+1; i++) {
      detectors[i-1]=detectors[i];
      thisMultiDetector->detectorIds[i-1]=thisMultiDetector->detectorIds[i];
    }
    detectors[thisMultiDetector->numberOfDetectors]=NULL;
    thisMultiDetector->detectorIds[thisMultiDetector->numberOfDetectors]=-1;
  }
  return thisMultiDetector->numberOfDetectors;
}







 
  int setMaster(int i=-1) {
    if (i>=0 && i<thisMultiDetector->numberOfDetectors) 
      if (detectors[i]) 
	thisMultiDetector->masterPosition=i;

    switch (thisMultiDetector->syncMode) {
    case MASTER_GATES:
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (i!=thisMultiDetector->masterPosition) {
	  if (detector[i]) {
	    detector[i]->setExternalSignalFlags(GATE_IN_ACTIVE_HIGH, 0);
	    detector[i]->setTimer(NUMBER_OF_GATES, 1);
	    detector[i]->setExternalSignalFlags(OFF, 1);
	  }
	} else {
	  detector[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	}
      }
      break;

    case MASTER_TRIGGERS:
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (i!=thisMultiDetector->masterPosition) {
	  if (detector[i]) {
	    detector[i]->setExternalSignalFlags(OFF, 0);
	    detector[i]->setExternalSignalFlags(TRIGGER_IN_RISING_EDGE, 1);
	  }
	} else {
	  detector[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	}
      }
      break;  
 
    case SLAVE_STARTS_WHEN_MASTER_STOPS:
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (detector[i]) {
	    detector[i]->setExternalSignalFlags(OFF, 0);
	    detector[i]->setExternalSignalFlags(TRIGGER_IN_FALLING_EDGE, 1);
	  }
	} else {
	  detector[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	}
      }
      break;

    
    default:  
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (detector[i]) {
	  detector[i]->setExternalSignalFlags(OFF, 0);
	  detector[i]->setExternalSignalFlags(OFF, 1);
	}
      }
      
    }

  
 
    return thisMultiDetector->masterPosition;
    }

//   enum synchronyzationMode {
//     GET_SYNHRONIZATION_MODE=-1, /**< the multidetector will return its synchronization mode */
//     NONE, /**< all detectors are independent (no cabling) */
//     MASTER_GATES, /**< the master gates the other detectors */
//     MASTER_TRIGGERS, /**< the master triggers the other detectors */
//     SLAVE_STARTS_WHEN_MASTER_STOPS /**< the slave acquires when the master finishes, to avoid deadtime */
//   }



  
  /** 
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode
      \returns current syncronization mode
  */
synchronizationMode setSyncronization(synchronizationMode sync=GET_SYNHRONIZATION_MODE) {
  if (sync>GET_SYNHRONIZATION_MODE) {
  
    
    switch (sync) {
    case MASTER_GATES:

      if (thisMultiDetector->masterPosition>=0 && thisMultiDetector->masterPosition<thisMultiDetector->numberOfDetectors) {
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (i!=thisMultiDetector->masterPosition) {
	    if (detector[i]) {
	      detector[i]->setExternalSignalFlags(GATE_IN_ACTIVE_HIGH, 0);
	      detector[i]->setTimer(NUMBER_OF_GATES, 1);
	      detector[i]->setExternalSignalFlags(OFF, 1);
	    }
	  } else {
	    detector[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	  }
	}
	thisMultiDetector->syncMode=sync;
      }
      break;

    case MASTER_TRIGGERS:
      if (thisMultiDetector->masterPosition>=0 && thisMultiDetector->masterPosition<thisMultiDetector->numberOfDetectors) {
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (i!=thisMultiDetector->masterPosition) {
	    if (detector[i]) {
	    detector[i]->setExternalSignalFlags(OFF, 0);
	    detector[i]->setExternalSignalFlags(TRIGGER_IN_RISING_EDGE, 1);
	    }
	  } else {
	    detector[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	  }
	}
	thisMultiDetector->syncMode=sync;
      }
      break;  
 
    case SLAVE_STARTS_WHEN_MASTER_STOPS:
      if (thisMultiDetector->masterPosition>=0 && thisMultiDetector->masterPosition<thisMultiDetector->numberOfDetectors) {
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (detector[i]) {
	    detector[i]->setExternalSignalFlags(OFF, 0);
	    detector[i]->setExternalSignalFlags(TRIGGER_IN_FALLING_EDGE, 1);
	  }
	} else {
	  detector[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
      }
      }
      thisMultiDetector->syncMode=sync;
    }
    break;
      
      
  default:  
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detector[i]) {
	detector[i]->setExternalSignalFlags(OFF, 0);
	detector[i]->setExternalSignalFlags(OFF, 1);
      }
    }
    thisMultiDetector->syncMode=sync; 
  }
  
  return thisMultiDetector->syncMode;
  
}



























int multiSlsDetector::setOnline(int off) {
  
  if (off!=GET_ONLINE_FLAG) {
    thisMultiDetector->onlineFlag=off;
    for (int i=0; i<thisMultiDetector->numberOfDetectors+1; i++) {
      if (detectors[i])
	detectors[i]->setOnlineFlag(off);
    }
  return thisMultiDetector->onlineFlag;

};


int multiSlsDetector::exists() {
  return thisMultiDetector->alreadyExisting;
}



  /* I/O */

/* generates file name without extension*/

string multiSlsDetector::createFileName() {
  return slsDetector::createFileName(thisMultiDetector->filePath, thisMultiDetector->fileName, thisMultiDetector->actionMask, currentScanVariable[0], thisMultiDetector->scanPrecision[0], currentScanVariable[1], thisMultiDetector->scanPrecision[1], currentPositionIndex, thisMultiDetector->numberOfPositions, thisMultiDetector->fileIndex);
  
}

/* I/O */



int multiSlsDetector::getFileIndexFromFileName(string fname) {
  return slsDetector::getFileIndexFromFileName(fname);
}

int multiSlsDetector::getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1) {
  return slsDetector::getVariablesFromFileName(fname, index, p_index, sv0, sv1);
}






























  /* Communication to server */



// Acquisition functions
/* change these funcs accepting also ok/fail */

int multiSlsDetector::startAcquisition(){
 
  int i=0;
  int ret=OK, ret1=OK;
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (i!=thisMultiDetector->masterPosition)
      if (detectors[i]) {
	ret=detectors[i]->startAcquisition();
	if (ret!=OK)
	  ret1=FAIL;
      }
  }
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;
     
};




int multiSlsDetector::stopAcquisition(){

  int i=0;
  int ret=OK, ret1=OK;  
 
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->stopAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->startAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;


};

int multiSlsDetector::startReadOut(){

 int i=0;
  int ret=OK, ret1=OK;
  i=thisMultiDetector->masterPosition;
  if (i>=0) 
    if (detectors[i]) {
      ret=detectors[i]->startReadOut();
      if (ret!=OK)
	ret1=FAIL;
    }
    }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->startReadOut();
      if (ret!=OK)
	ret1=FAIL;
    }
  }

  return ret1;
     

};




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



















