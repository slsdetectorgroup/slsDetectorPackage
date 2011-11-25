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


    thisMultiDetector->onlineFlag = slsDetector::ONLINE_FLAG;
    thisMultiDetector->numberOfDetectors=0;
    for (int id=0; id<MAXDET; id++) {
      thisMultiDetector->detectorIds[id]=-1;
      thisMultiDetector->offsetX[id]=0;
      thisMultiDetector->offsetY[id]=0;
    }
    thisMultiDetector->masterPosition=-1;
    thisMultiDetector->dataBytes=0;
    thisMultiDetector->numberOfChannels=0;

    thisMultiDetector->alreadyExisting=1;
  }


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    detectors[i]=new slsDetector(thisMultiDetector->detectorIds[i]);
  }
  for (int i=thisMultiDetector->numberOfDetectors; i<MAXDET; i++)
    detectors[i]=NULL;



   /** modifies the last PID accessing the detector system*/
  thisMultiDetector->lastPID=getpid();

}

multiSlsDetector::~multiSlsDetector() {
  //removeSlsDetector();

}

int multiSlsDetector::addSlsDetector(int id, int pos, int ox, int oy) {
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
  thisMultiDetector->numberOfChannels+=detectors[pos]->getNChans()*detectors[pos]->getNChips()*detectors[pos]->getNMods();


  return thisMultiDetector->numberOfDetectors;

}

int multiSlsDetector::getDetectorOffset(int pos, int &ox, int &oy) {
  ox=-1;
  oy=-1;
  int ret=FAIL;
  if (pos>=0 && pos<thisMultiDetector->numberOfDetectors) {
    if (detectors[pos]) {
      ox=thisMultiDetector->offsetX[pos];
      oy=thisMultiDetector->offsetY[pos];
      ret=OK;
    }
  }
  return ret;
}

int multiSlsDetector::setDetectorOffset(int pos, int ox, int oy) {
 
 
  int ret=FAIL;
 
  if (pos>=0 && pos<thisMultiDetector->numberOfDetectors) {
    if (detectors[pos]) {
      if (ox!=-1)
	thisMultiDetector->offsetX[pos]=ox;
      if (oy!=-1) 
	thisMultiDetector->offsetY[pos]=oy;
      ret=OK;
    }
  }
  return ret;
}






int multiSlsDetector::removeSlsDetector(int pos) {
  int j;
  
  if (pos<0 )
    pos=thisMultiDetector->numberOfDetectors-1;

  if (pos>=thisMultiDetector->numberOfDetectors)
    return thisMultiDetector->numberOfDetectors;

  j=pos;

  if (detectors[j]) {

  thisMultiDetector->dataBytes-=detectors[j]->getDataBytes();
  thisMultiDetector->numberOfChannels-=detectors[j]->getNChans()*detectors[pos]->getNChips()*detectors[pos]->getNMods();

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







 
int multiSlsDetector::setMaster(int i) {
  if (i>=0 && i<thisMultiDetector->numberOfDetectors) 
    if (detectors[i]) 
      thisMultiDetector->masterPosition=i;

  switch (thisMultiDetector->syncMode) {
  case MASTER_GATES:
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (i!=thisMultiDetector->masterPosition) {
	  if (detectors[i]) {
	    detectors[i]->setExternalSignalFlags(GATE_IN_ACTIVE_HIGH, 0);
	    detectors[i]->setTimer(GATES_NUMBER, 1);
	    detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 1);
	  }
      } else {
	detectors[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	}
    }
    break;
    
  case MASTER_TRIGGERS:
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (i!=thisMultiDetector->masterPosition) {
	  if (detectors[i]) {
	    detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 0);
	    detectors[i]->setExternalSignalFlags(TRIGGER_IN_RISING_EDGE, 1);
	  }
	} else {
	  detectors[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	}
      }
      break;  
      
  case SLAVE_STARTS_WHEN_MASTER_STOPS:
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	if (i!=thisMultiDetector->masterPosition) {
	  detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 0);
	  detectors[i]->setExternalSignalFlags(TRIGGER_IN_FALLING_EDGE, 1);
	} else {
	  detectors[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	}
      }
    }
    break;
  
  
  default:  
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 0);
	detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 1);
      }
    }
   
  }

  

  return thisMultiDetector->masterPosition;
}

//   enum synchronyzationMode {
//     GET_SYNCHRONIZATION_MODE=-1, /**< the multidetector will return its synchronization mode */
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
synchronizationMode multiSlsDetector::setSynchronization(synchronizationMode sync) {
  if (sync>GET_SYNCHRONIZATION_MODE) {
  
    
    switch (sync) {
    case MASTER_GATES:

      if (thisMultiDetector->masterPosition>=0 && thisMultiDetector->masterPosition<thisMultiDetector->numberOfDetectors) {
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (i!=thisMultiDetector->masterPosition) {
	    if (detectors[i]) {
	      detectors[i]->setExternalSignalFlags(GATE_IN_ACTIVE_HIGH, 0);
	      detectors[i]->setTimer(GATES_NUMBER, 1);
	      detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 1);
	    }
	  } else {
	    detectors[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	  }
	}
	thisMultiDetector->syncMode=sync;
      }
      break;

    case MASTER_TRIGGERS:
      if (thisMultiDetector->masterPosition>=0 && thisMultiDetector->masterPosition<thisMultiDetector->numberOfDetectors) {
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (i!=thisMultiDetector->masterPosition) {
	    if (detectors[i]) {
	    detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 0);
	    detectors[i]->setExternalSignalFlags(TRIGGER_IN_RISING_EDGE, 1);
	    }
	  } else {
	    detectors[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	  }
	}
	thisMultiDetector->syncMode=sync;
      }
      break;  
 
    case SLAVE_STARTS_WHEN_MASTER_STOPS:
      if (thisMultiDetector->masterPosition>=0 && thisMultiDetector->masterPosition<thisMultiDetector->numberOfDetectors) {
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (detectors[i]) {
	    if (i!=thisMultiDetector->masterPosition) {
	      detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 0);
	      detectors[i]->setExternalSignalFlags(TRIGGER_IN_FALLING_EDGE, 1);
	    }
	  } else {
	    detectors[i]->setExternalSignalFlags(GATE_OUT_ACTIVE_HIGH, 2);
	  }
	}
	thisMultiDetector->syncMode=sync;
      }
      break;
      
      
    default:  
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (detectors[i]) {
	  detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 0);
	  detectors[i]->setExternalSignalFlags(SIGNAL_OFF, 1);
	}
      }
      thisMultiDetector->syncMode=sync; 
    }
  }  

  return thisMultiDetector->syncMode;
  
}



























int multiSlsDetector::setOnline(int off) {
  
  if (off!=slsDetector::GET_ONLINE_FLAG) {
    thisMultiDetector->onlineFlag=off;
    for (int i=0; i<thisMultiDetector->numberOfDetectors+1; i++) {
      if (detectors[i])
	detectors[i]->setOnline(off);
    }
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





  // Initialization functions



  

int multiSlsDetector::getThresholdEnergy(int pos) {
 

  int i, posmin, posmax;
  int ret1=-100, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getThresholdEnergy();
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      
    }
   
  }
  thisMultiDetector->currentThresholdEV=ret1;
  return ret1;


}  


int multiSlsDetector::setThresholdEnergy(int e_eV, int pos, detectorSettings isettings) {

  int i, posmin, posmax;
  int ret1=-100, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setThresholdEnergy(e_eV,-1,isettings);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      
    }
   
  }
  thisMultiDetector->currentThresholdEV=ret1;
  return ret1;

}
 
detectorSettings multiSlsDetector::getSettings(int pos) {

  int i, posmin, posmax;
  detectorSettings ret1=GET_SETTINGS, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getSettings();
      if (ret1==GET_SETTINGS)
	ret1=ret;
      else if (ret!=ret1)
	ret1=GET_SETTINGS;
      
    }
   
  }
  thisMultiDetector->currentSettings=ret1;
  return ret1;
}

detectorSettings multiSlsDetector::setSettings(detectorSettings isettings, int pos) {


  int i, posmin, posmax;
  detectorSettings ret1=GET_SETTINGS, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setSettings(isettings);
      if (ret1==GET_SETTINGS)
	ret1=ret;
      else if (ret!=ret1)
	ret1=GET_SETTINGS;
      
    }
   
  }
  thisMultiDetector->currentSettings=ret1;
  return ret1;

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
  if (i>=0) {
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



int* multiSlsDetector::getDataFromDetector() {

  int nel=thisMultiDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int *retdet, *p=retval;
  
 

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      retdet=detectors[id]->getDataFromDetector();
      if (retdet) {
	n=detectors[id]->getDataBytes();
	memcpy(p,retdet,n);
	delete [] retdet;
	p+=n/sizeof(int);
      } else {
	cout << "Detector " << id << " does not have data left " << endl;
	delete [] retval;
	return NULL;
      }
    }
  }
  return retval;
};


int* multiSlsDetector::readFrame(){
  int nel=thisMultiDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int *retdet, *p=retval;
  
  /** probably it's always better to have one integer per channel in any case! */

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      retdet=detectors[id]->readFrame();
      if (retdet) {
	n=detectors[id]->getDataBytes();
	memcpy(p,retdet,n);
	delete [] retdet;
	p+=n/sizeof(int);
	
      } else {
	cout << "Detector " << id << " does not have data left " << endl;
	delete [] retval;
	return NULL;
      }
    }
  }
  return retval;

};



int* multiSlsDetector::readAll(){
  
  /** Thread for each detector?!?!?! */

  // int fnum=F_READ_ALL;
  int* retval; // check what we return!
  // int ret=OK, ret1=OK;

  int i=0;
#ifdef VERBOSE
  std::cout<< "Reading all frames "<< std::endl;
#endif
  if (thisMultiDetector->onlineFlag==slsDetector::ONLINE_FLAG) {
    
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->readAllNoWait();
      }
    }
    while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      // std::cout<< i << std::endl;
#endif
      dataQueue.push(retval);
    }
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->disconnectControl();
      }
    }  
 
 }

#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
#endif
  return dataQueue.front(); // check what we return!

};

int* multiSlsDetector::startAndReadAll(){

  /** Thread for each detector?!?!?! */
 
  int* retval;
  int i=0;
  if (thisMultiDetector->onlineFlag==slsDetector::ONLINE_FLAG) {
    
    startAndReadAllNoWait();
   
    while ((retval=getDataFromDetector())){
      i++;
      //#ifdef VERBOSE
      std::cout<< i << std::endl;
      //#endif
      dataQueue.push(retval);
    }

    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->disconnectControl();
      }
    }  
 
 }

  //#ifdef VERBOSE
  std::cout<< "recieved "<< i<< " frames" << std::endl;
  //#endif
  return dataQueue.front(); // check what we return!

  
};


int multiSlsDetector::startAndReadAllNoWait(){


  int i=0;
  int ret=OK, ret1=OK;

 for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (i!=thisMultiDetector->masterPosition)
	if (detectors[i]) {
	  ret=detectors[i]->startAndReadAllNoWait();
	if (ret!=OK)
	  ret1=FAIL;
	}
    }
    i=thisMultiDetector->masterPosition;
    if (thisMultiDetector->masterPosition>=0) {
      if (detectors[i]) {
	ret=detectors[i]->startAndReadAllNoWait();
	if (ret!=OK)
	  ret1=FAIL;
      }
    }
    return ret1;

}



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
  int i;
  int64_t ret1=-100, ret;


  if (index!=ACQUISITION_TIME) {
    for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	ret=detectors[i]->setTimer(index,t);
	if (ret1==-100)
	  ret1=ret;
	else if (ret!=ret1)
	  ret1=FAIL;
	    
      }
    }
  } else {
    switch(thisMultiDetector->syncMode) {
    case MASTER_GATES:      
      for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (i!=thisMultiDetector->masterPosition)
	  if (detectors[i]) {
	    ret=detectors[i]->setTimer(GATES_NUMBER,1);
	  }
      }
      
      i=thisMultiDetector->masterPosition;
      if (thisMultiDetector->masterPosition>=0) {
	if (detectors[i]) {
	  ret=detectors[i]->setTimer(index,t);
	  ret1=ret;
	}
      }
      break;
      
    default:
      for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (detectors[i]) {
	  ret=detectors[i]->setTimer(index,t);
	if (ret1==-100)
	  ret1=ret;
	else if (ret!=ret1)
	  ret1=FAIL;
	}
      }
    }
  }

// check return values!!!

  thisMultiDetector->timerValue[index]=ret1;

  return ret1;
};






















// int64_t multiSlsDetector::getTimeLeft(timerIndex index){
  

//   int fnum=F_GET_TIME_LEFT;
//   int64_t retval;
//   char mess[100];
//   int ret=OK;

// #ifdef VERBOSE
//   std::cout<< "Getting  timer  "<< index <<  std::endl;
// #endif
//   if (thisDetector->onlineFlag==ONLINE_FLAG) {
// //     if (controlSocket) {
// //   if  (controlSocket->Connect()>=0) {
// //     controlSocket->SendDataOnly(&fnum,sizeof(fnum));
// //     controlSocket->SendDataOnly(&index,sizeof(index));
// //     controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
// //     if (ret!=OK) {
// //       controlSocket->ReceiveDataOnly(mess,sizeof(mess));
// //       std::cout<< "Detector returned error: " << mess << std::endl;
// //     } else {
// //       controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
// //       // thisDetector->timerValue[index]=retval;
// //     }   
// //     controlSocket->Disconnect();
// //   }
// //     }
//     if (stopSocket) {
//   if  (stopSocket->Connect()>=0) {
//     stopSocket->SendDataOnly(&fnum,sizeof(fnum));
//     stopSocket->SendDataOnly(&index,sizeof(index));
//     stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
//     if (ret!=OK) {
//       stopSocket->ReceiveDataOnly(mess,sizeof(mess));
//       std::cout<< "Detector returned error: " << mess << std::endl;
//     } else {
//       stopSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
//       // thisDetector->timerValue[index]=retval;
//     }   
//     stopSocket->Disconnect();
//   }
//     }
//   }
// #ifdef VERBOSE
//   std::cout<< "Time left is  "<< retval << std::endl;
// #endif
//   return retval;
  
// };


  // Flags
int multiSlsDetector::setDynamicRange(int n, int pos){

  int imi, ima, i;
  int ret, ret1=-100;

  if (pos<0) {
    imi=0;
    ima=thisMultiDetector->numberOfDetectors;
  } else {
    imi=pos;
    ima=pos+1;
  }
  
  for (i=imi; i<ima; i++) {
    if (detectors[i]) {
      thisMultiDetector->dataBytes-=detectors[i]->getDataBytes();
      ret=detectors[i]->setDynamicRange(n);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      thisMultiDetector->dataBytes+=detectors[i]->getDataBytes();
    }
  }

  return thisMultiDetector->dataBytes;
};

/*

int multiSlsDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/


float* multiSlsDetector::decodeData(int *datain) {
  float *dataout=new float[thisMultiDetector->numberOfChannels];
  int ich=0;
  float *detp;
  int   *datap=datain;


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      detp=detectors[i]->decodeData(datap);
      datap+=detectors[i]->getDataBytes();
      for (int j=0; j<detectors[i]->getNChans()*detectors[i]->getNChips()*detectors[i]->getNMods(); j++) {
	dataout[ich]=detp[j];
	ich++;
      }
      delete [] detp;
    }
  }

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


///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////







int multiSlsDetector::setFlatFieldCorrection(string fname){
  float data[thisMultiDetector->numberOfChannels],  xmed[thisMultiDetector->numberOfChannels];
  float ffcoefficients[thisMultiDetector->numberOfChannels], fferrors[thisMultiDetector->numberOfChannels];
  int nmed=0;
  int idet=0, ichdet=-1;
  char ffffname[MAX_STR_LENGTH*2];
  int nbad=0, nch;
  int badlist[MAX_BADCHANS];
  int im=0;


  if (fname=="") {
#ifdef VERBOSE
   std::cout<< "disabling flat field correction" << std::endl;
#endif
    thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
    strcpy(thisMultiDetector->flatFieldFile,"none");
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i])
	detectors[i]->setFlatFieldCorrection(NULL, NULL);
    }
  } else { 
#ifdef VERBOSE
   std::cout<< "Setting flat field correction from file " << fname << std::endl;
#endif
   sprintf(ffffname,"%s/%s",thisMultiDetector->flatFieldDir,fname.c_str());
   nch=readDataFile(string(ffffname),data);
    if (nch>0) {
      strcpy(thisMultiDetector->flatFieldFile,fname.c_str());
     


      for (int ichan=0; ichan<nch; ichan++) {
	if (detectors[idet]) {
	  if (ichdet>=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods()) {
	    ichdet=0;
	    detectors[idet]->setBadChannelCorrection(nbad,badlist,1);
	    idet++;
	    nbad=0;
	  } else
	    ichdet++;
	}

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
	  badlist[nbad]=ichdet;
	  nbad++;
	}
      }
      if (detectors[idet]) 
	detectors[idet]->setBadChannelCorrection(nbad,badlist,1);
	
      if (nmed>1 && xmed[nmed/2]>0) {
#ifdef VERBOSE
	std::cout<< "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << std::endl;
#endif
	
	thisMultiDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);

	// add to ff coefficients and errors of single detectors

	  

	idet=0; 
	ichdet=0;

	for (int ichan=0; ichan<nch; ichan++) {

	  if (detectors[idet]) {
	    if (ichdet>=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods()) {
	      detectors[idet]->setFlatFieldCorrection(ffcoefficients+ichdet, fferrors+ichdet);
	      ichdet=ichan;
	      idet++;
	    } 
	  }


	  if (data[ichan]>0) {
	    ffcoefficients[ichan]=xmed[nmed/2]/data[ichan];
	    fferrors[ichan]=ffcoefficients[ichan]*sqrt(data[ichan])/data[ichan];
	  } else {
	    ffcoefficients[ichan]=0.;
	    fferrors[ichan]=1.;
	  }
	}
	if (detectors[idet]) {
	  detectors[idet]->setFlatFieldCorrection(ffcoefficients+ichdet, fferrors+ichdet);
	}
      } else {
	std::cout<< "Flat field data from file " << fname << " are not valid (" << nmed << "///" << xmed[nmed/2] << std::endl;
	thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (detectors[i])
	    detectors[i]->setFlatFieldCorrection(NULL, NULL);
	}
	return -1;
      }
    } else {
      std::cout<< "Flat field from file " << fname << " is not valid " << nch << std::endl;  
      thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (detectors[i])
	  detectors[i]->setFlatFieldCorrection(NULL, NULL);
      }
      return -1;
    } 
  }
  return thisMultiDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}
 












int multiSlsDetector::getFlatFieldCorrection(float *corr, float *ecorr) {
  int ichdet=0;
  float *p, *ep;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (corr!=NULL)
	p=corr+ichdet;
      else
	p=NULL;
      if (ecorr!=NULL)
	ep=ecorr+ichdet;
      else
	ep=NULL;
      detectors[idet]->getFlatFieldCorrection(p, ep);
      ichdet+=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
    }
  }
  return 0;

}


int multiSlsDetector::flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr){

  return slsDetector::flatFieldCorrect(datain, errin, dataout, errout, ffcoefficient, fferr);
};



int multiSlsDetector::flatFieldCorrect(float* datain, float *errin, float* dataout, float *errout){  

  int ichdet=0;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      detectors[idet]->flatFieldCorrect(datain+ichdet, errin+ichdet, dataout+ichdet, errout+ichdet);
      ichdet+=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
    }
  }
  return 0;
};






int multiSlsDetector::setRateCorrection(float t){
  float tdead[]=defaultTDead;

  if (t==0) {
    thisMultiDetector->correctionMask&=~(1<<RATE_CORRECTION);
  } else {
    thisMultiDetector->correctionMask|=(1<<RATE_CORRECTION);
    
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      
      if (detectors[idet]) {
	detectors[idet]->setRateCorrection(t);
      }
    }
#ifdef VERBOSE
    std::cout<< "Setting rate correction with dead time "<< thisDetector->tDead << std::endl;
#endif
  }
  return thisMultiDetector->correctionMask&(1<<RATE_CORRECTION);
}


int multiSlsDetector::getRateCorrection(float &t){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisDetector->tDead << std::endl;
#endif
    //which t should we return if they are all different?
    return 1;
  } else
    t=0;
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    return 0;
};

float multiSlsDetector::getRateCorrectionTau(){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisDetector->tDead << std::endl;
#endif
    //which t should we return if they are all different?
    return 1;
  } else
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    return 0;
};







int multiSlsDetector::getRateCorrection(){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
    return 1;
  } else
    return 0;
};




 int multiSlsDetector::rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t){

   return slsDetector::rateCorrect(datain, errin, dataout, errout, tau, t);

};


int multiSlsDetector::rateCorrect(float* datain, float *errin, float* dataout, float *errout){

  int ichdet=0;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      detectors[idet]->rateCorrect(datain+ichdet, errin+ichdet, dataout+ichdet, errout+ichdet);
      ichdet+=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
    }
  }
  return 0;
};


int multiSlsDetector::setBadChannelCorrection(string fname){

  int badlist[MAX_BADCHANS], badlistdet[MAX_BADCHANS];
  int nbad=0, nbaddet=0, choff=0, idet=0;

  if (fname=="default")
    fname=string(thisMultiDetector->badChanFile);

  int ret=slsDetector::setBadChannelCorrection(fname, nbad, badlist);
  if (ret) {
    thisMultiDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
    
    for (int ich=0; ich<nbad; ich++) {
      if (detectors[idet]) {
	while ((badlist[ich]-choff)>=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods()) {
	  detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
	  choff+=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
	  nbaddet=0;
	  idet++;
	  if (detectors[idet]==NULL)
	    break;
	}
	badlistdet[nbaddet]=(badlist[ich]-choff);
	nbaddet++;
      }
    } 
    nbaddet=0;
    for (int i=idet; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	detectors[i]->setBadChannelCorrection(nbaddet,badlist,0);
      }
    }
    
  }  else {
    nbaddet=0;
    for (int i=0; i<<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[idet]) {
	detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
      }
    }
    thisMultiDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
  }    
    
  return thisMultiDetector->correctionMask&(1<<DISCARD_BAD_CHANNELS);
}








int multiSlsDetector::getBadChannelCorrection(int *bad) {
  int nbad=0, nbaddet=0,  choff=0;
  int detbad[MAX_BADCHANS];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    
      if (detectors[idet]) {
	nbaddet=detectors[idet]->getBadChannelCorrection(detbad);
	for (int ich=0; ich<nbaddet; ich++) {
	  bad[nbad]=detbad[ich]+choff;
	  nbad++;
	}
	choff+=detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
      }
  }
  return nbad;
}










  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable but leaves script unchanged, "none" disables and overwrites)
      \returns 0 if action disabled, >0 otherwise
  */
int multiSlsDetector::setAction(int iaction, string fname, string par) {

  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if (fname=="") {
      thisMultiDetector->actionMode[iaction]=0;
    } else if (fname=="none") {
      thisMultiDetector->actionMode[iaction]=0;
      strcpy(thisMultiDetector->actionScript[iaction],fname.c_str());
    } else {
      strcpy(thisMultiDetector->actionScript[iaction],fname.c_str());
      thisMultiDetector->actionMode[iaction]=1;
    }
    
    if (par!="") {
      strcpy(thisMultiDetector->actionParameter[iaction],par.c_str());
    }
    
    if (thisMultiDetector->actionMode[iaction]) {

#ifdef VERBOSE
      cout << iaction << "  " << hex << (1 << iaction) << " " << thisMultiDetector->actionMask << dec;
#endif
    
      thisMultiDetector->actionMask |= (1 << iaction);

#ifdef VERBOSE
    cout << " set " << hex << thisMultiDetector->actionMask << dec << endl;
#endif
    
    } else {
#ifdef VERBOSE
    cout << iaction << "  " << hex << thisMultiDetector->actionMask << dec;
#endif
    
    thisMultiDetector->actionMask &= ~(1 << iaction);

#ifdef VERBOSE
    cout << "  unset " << hex << thisMultiDetector->actionMask << dec << endl;
#endif
    }
#ifdef VERBOSE
    cout << iaction << " Action mask set to " << hex << thisMultiDetector->actionMask << dec << endl;
#endif
    
    return thisMultiDetector->actionMode[iaction]; 
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
      strcpy(thisMultiDetector->actionParameter[iaction],par.c_str());
    }
    
    if (thisMultiDetector->actionMode[iaction]) {
      thisMultiDetector->actionMask |= (1 << iaction);
    } else {
      thisMultiDetector->actionMask &= ~(1 << iaction);
    }
    
    return thisMultiDetector->actionMode[iaction]; 
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
    return string(thisMultiDetector->actionScript[iaction]);
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
    return string(thisMultiDetector->actionParameter[iaction]);
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
    cout << "slsDetetctor : action " << iaction << " mode is " <<  thisMultiDetector->actionMode[iaction] << endl;
#endif
    return thisMultiDetector->actionMode[iaction];
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
      thisMultiDetector->scanMode[iscan]=0;
    } else {
      strcpy(thisMultiDetector->scanScript[iscan],script.c_str());
      if (script=="none") {
	thisMultiDetector->scanMode[iscan]=0;
      } else if (script=="energy") {
	thisMultiDetector->scanMode[iscan]=1;
      }  else if (script=="threshold") {
	thisMultiDetector->scanMode[iscan]=2;
      } else if (script=="trimbits") {
	thisMultiDetector->scanMode[iscan]=3;
      } else {
	thisMultiDetector->scanMode[iscan]=4;
      }  
    }
    
  

    

    
    if (par!="")
      strcpy(thisMultiDetector->scanParameter[iscan],par.c_str());
      
      if (nvalues>=0) {    
	if (nvalues==0)
	  thisMultiDetector->scanMode[iscan]=0;
	else {
	  thisMultiDetector->nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    thisMultiDetector->nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values && 	thisMultiDetector->scanMode[iscan]>0 ) {
	for (int iv=0; iv<thisMultiDetector->nScanSteps[iscan]; iv++) {
	  thisMultiDetector->scanSteps[iscan][iv]=values[iv];
	}
      }

      if (precision>=0)
	thisMultiDetector->scanPrecision[iscan]=precision;
      
      if (thisMultiDetector->scanMode[iscan]>0){
	thisMultiDetector->actionMask |= 1<< (iscan+MAX_ACTIONS);
      } else {
	thisMultiDetector->actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }



      setTotalProgress();










      return thisMultiDetector->scanMode[iscan];
  }  else 
    return -1;
  
}

int multiSlsDetector::setScanScript(int iscan, string script) {
 if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (script=="") {
      thisMultiDetector->scanMode[iscan]=0;
    } else {
      strcpy(thisMultiDetector->scanScript[iscan],script.c_str());
      if (script=="none") {
	thisMultiDetector->scanMode[iscan]=0;
      } else if (script=="energy") {
	thisMultiDetector->scanMode[iscan]=1;
      }  else if (script=="threshold") {
	thisMultiDetector->scanMode[iscan]=2;
      } else if (script=="trimbits") {
	thisMultiDetector->scanMode[iscan]=3;
      } else {
	thisMultiDetector->scanMode[iscan]=4;
      }  
    }
    
    if (thisMultiDetector->scanMode[iscan]>0){
      thisMultiDetector->actionMask |= (1 << (iscan+MAX_ACTIONS));
    } else {
      thisMultiDetector->actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
    }

    setTotalProgress();

















    
#ifdef VERBOSE
      cout << "Action mask is " << hex << thisMultiDetector->actionMask << dec << endl;
#endif
    return thisMultiDetector->scanMode[iscan];
    
    
 } else 
   return -1;
 
}



int multiSlsDetector::setScanParameter(int iscan, string par) {


  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (par!="")
	strcpy(thisMultiDetector->scanParameter[iscan],par.c_str());
      return thisMultiDetector->scanMode[iscan];
  } else
    return -1;

}


int multiSlsDetector::setScanPrecision(int iscan, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (precision>=0)
      thisMultiDetector->scanPrecision[iscan]=precision;
    return thisMultiDetector->scanMode[iscan];
  } else
    return -1;

}

int multiSlsDetector::setScanSteps(int iscan, int nvalues, float *values) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
  
      if (nvalues>=0) {    
	if (nvalues==0)
	  thisMultiDetector->scanMode[iscan]=0;
	else {
	  thisMultiDetector->nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    thisMultiDetector->nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values) {
	for (int iv=0; iv<thisMultiDetector->nScanSteps[iscan]; iv++) {
	  thisMultiDetector->scanSteps[iscan][iv]=values[iv];
	}
      }
     
      if (thisMultiDetector->scanMode[iscan]>0){
	thisMultiDetector->actionMask |= (1 << (iscan+MAX_ACTIONS));
      } else {
	thisMultiDetector->actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }

#ifdef VERBOSE
      cout << "Action mask is " << hex << thisMultiDetector->actionMask << dec << endl;
#endif
      setTotalProgress();




      return thisMultiDetector->scanMode[iscan];
  

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
      if (thisMultiDetector->scanMode[iscan])
	return string(thisMultiDetector->scanScript[iscan]);
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
    if (thisMultiDetector->scanMode[iscan])
      return string(thisMultiDetector->scanParameter[iscan]);
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
    return thisMultiDetector->scanMode[iscan];
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
      for (int iv=0; iv<thisMultiDetector->nScanSteps[iscan]; iv++) {
	v[iv]=thisMultiDetector->scanSteps[iscan][iv];
      }
    }


    setTotalProgress();

    if (thisMultiDetector->scanMode[iscan])
      return thisMultiDetector->nScanSteps[iscan];
    else
      return 0;
  } else
    return -1;
}


int multiSlsDetector::getScanPrecision(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    return thisMultiDetector->scanPrecision[iscan];
  } else
    return -1;
}



int multiSlsDetector::setTotalProgress() {

       int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1;


       

      
      if (thisMultiDetector->timerValue[FRAME_NUMBER])
	nf=thisMultiDetector->timerValue[FRAME_NUMBER];

      if (thisMultiDetector->timerValue[CYCLES_NUMBER]>0)
	nc=thisMultiDetector->timerValue[CYCLES_NUMBER];








      if (thisMultiDetector->numberOfPositions>0)
	npos=thisMultiDetector->numberOfPositions;

      if ((thisMultiDetector->nScanSteps[0]>0) && (thisMultiDetector->actionMask & (1 << MAX_ACTIONS)))
	nscan[0]=thisMultiDetector->nScanSteps[0];

      if ((thisMultiDetector->nScanSteps[1]>0) && (thisMultiDetector->actionMask & (1 << (MAX_ACTIONS+1))))
	nscan[1]=thisMultiDetector->nScanSteps[1];
      
      thisMultiDetector->totalProgress=nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
      cout << "nc " << nc << endl;
      cout << "nf " << nf << endl;
      cout << "npos " << npos << endl;
      cout << "nscan[0] " << nscan[0] << endl;
      cout << "nscan[1] " << nscan[1] << endl;

      cout << "Set total progress " << thisMultiDetector->totalProgress << endl;
#endif
      return thisMultiDetector->totalProgress;
}


float multiSlsDetector::getCurrentProgress() {

  return 100.*((float)thisMultiDetector->progressIndex)/((float)thisMultiDetector->totalProgress);
}


















