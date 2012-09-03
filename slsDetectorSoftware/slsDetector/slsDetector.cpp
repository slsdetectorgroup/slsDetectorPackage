
#include "slsDetector.h"
#include "usersFunctions.h"
#include "slsDetectorCommand.h"
#include  <sys/types.h>
#include  <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bitset>
#include <cstdlib>

int slsDetector::initSharedMemory(detectorType type, int id) {


    /**
      the shared memory key is set to DEFAULT_SHM_KEY+id
   */
   key_t     mem_key=DEFAULT_SHM_KEY+id;
   int       shm_id;
   int nch, nm, nc, nd;
   int sz;

   //shmId=-1;

   switch(type) {
   case MYTHEN:
     nch=128; // complete mythen system
     nm=24;
     nc=10;
     nd=6; // dacs+adcs
     break;
   case PICASSO:
     nch=128; // complete mythen system
     nm=24;
     nc=12;
     nd=6; // dacs+adcs
     break;
   case GOTTHARD:
     nch=128; 
     nm=1;
     nc=10;
     nd=13; // dacs+adcs
     break;
   case EIGER:
     nch=65536; // one EIGER half module
     nm=1; //modules/detector
     nc=4; //chips
     nd=16; //dacs+adcs
   default:
     nch=0; // dum!
     nm=0; //modules/detector
     nc=0; //chips
     nd=0; //dacs+adcs
     
   }
   /**
      The size of the shared memory is:
       size of shared structure + ffcoefficents +fferrors + modules+ dacs+adcs+chips+chans 
   */


   sz=sizeof(sharedSlsDetector)+nm*(2*nch*nc*sizeof(double)+sizeof(sls_detector_module)+sizeof(int)*nc+sizeof(dacs_t)*nd+sizeof(int)*nch*nc);
#ifdef VERBOSE
   std::cout<<"Size of shared memory is "<< sz << "(type " << type << " - id " << mem_key << ")"<< std::endl;
#endif
   shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    std::cout<<"*** shmget error (server) ***"<< shm_id << std::endl;
    return shm_id;
  }
  
   /**
      thisDetector pointer is set to the memory address of the shared memory
   */

  thisDetector = (sharedSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (thisDetector == (void*)-1) {
    std::cout<<"*** shmat error (server) ***" << std::endl;
    return shm_id;
  }


    /**
      shm_id returns -1 is shared memory initialization fails
   */
  //shmId=shm_id;
  return shm_id;

}


int slsDetector::freeSharedMemory() {
  // Detach Memory address
    if (shmdt(thisDetector) == -1) {
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



slsDetector::slsDetector(int id) :slsDetectorUtils(),
  thisDetector(NULL),  
  detId(id),
  shmId(-1), 
  controlSocket(NULL),
  stopSocket(NULL),
  dataSocket(NULL),
  ffcoefficients(NULL),
  fferrors(NULL),
  detectorModules(NULL),
  dacs(NULL),
  adcs(NULL),
  chipregs(NULL),
  chanregs(NULL)


{


  detectorType type=(detectorType)getDetectorType(id);

  while (shmId<0) {
       /**Initlializes shared memory \sa initSharedMemory
	  
       if it fails the detector id is incremented until it succeeds
       */
    shmId=initSharedMemory(type,id);
    id++;
  }
  id--;
#ifdef VERBOSE
  std::cout<< "Detector id is " << id << std::endl;
#endif
  detId=id;
  
  
  /**Initializes the detector stucture \sa initializeDetectorSize
      */
  initializeDetectorSize(type);



};





slsDetector::slsDetector(detectorType type, int id): slsDetectorUtils(),
  thisDetector(NULL),  
  detId(id),
  shmId(-1), 
  controlSocket(NULL),
  stopSocket(NULL),
  dataSocket(NULL),
  ffcoefficients(NULL),
  fferrors(NULL),
  detectorModules(NULL),
  dacs(NULL),
  adcs(NULL),
  chipregs(NULL),
  chanregs(NULL)
 {
     while (shmId<0) {
       /**Initlializes shared memory \sa initSharedMemory

       if it fails the detector id is incremented until it succeeds
       */
       shmId=initSharedMemory(type,id);
       id++;
     }
     id--;
#ifdef VERBOSE
     std::cout<< "Detector id is " << id << " type is " << type << std::endl;
#endif
     detId=id;


     /**Initializes the detector stucture \sa initializeDetectorSize
      */
     initializeDetectorSize(type);



}


slsDetector::~slsDetector(){

  // Detach Memory address
    if (shmdt(thisDetector) == -1) {
      perror("shmdt failed\n");
      printf("Could not detach shared memory %d\n", shmId);
    } else
      printf("Shared memory %d detached\n", shmId);


};

slsDetector::slsDetector(char *name, int id, int cport) : slsDetectorUtils(),
  thisDetector(NULL),  
  detId(id),
  shmId(-1), 
  controlSocket(NULL),
  stopSocket(NULL),
  dataSocket(NULL),
  ffcoefficients(NULL),
  fferrors(NULL),
  detectorModules(NULL),
  dacs(NULL),
  adcs(NULL),
  chipregs(NULL),
  chanregs(NULL)
{
  detectorType type=(detectorType)getDetectorType(name, cport);
  
  
  while (shmId<0) {
       /**Initlializes shared memory \sa initSharedMemory
	  
       if it fails the detector id is incremented until it succeeds
       */
    shmId=initSharedMemory(type,id);
    id++;
  }
  id--;
#ifdef VERBOSE
  std::cout<< "Detector id is " << id << std::endl;
#endif
  detId=id;
  
  
  /**Initializes the detector stucture \sa initializeDetectorSize
      */
  initializeDetectorSize(type);




  
  setTCPSocket(name, cport);
  updateDetector();

}

slsDetectorDefs::detectorType slsDetector::getDetectorType(const char *name, int cport) {
  
  int retval=FAIL;
  detectorType t=GENERIC;
  int fnum=F_GET_DETECTOR_TYPE;
  MySocketTCP *s= new MySocketTCP(name, cport);
  char m[100];
#ifdef VERBOSE
  cout << "Getting detector type " << endl;
#endif
 if (s->Connect()>=0) {
    s->SendDataOnly(&fnum,sizeof(fnum));
    s->ReceiveDataOnly(&retval,sizeof(retval));
    
    if (retval!=FAIL) {
      s->ReceiveDataOnly(&t,sizeof(t));

#ifdef VERBOSE
      cout << "Detector type is "<< t << endl;
#endif
      
    } else {
      s->ReceiveDataOnly(m,sizeof(m));
      std::cout<< "Detector returned error: " << m << std::endl;
    }
    s->Disconnect();
 } else {
   cout << "Cannot connect to server " << name << " over port " << cport << endl;
 }

 delete s;
 return t;

}








int slsDetector::exists(int id) {

     key_t     mem_key=DEFAULT_SHM_KEY+id;
   int       shm_id;
   int sz;

   sz=sizeof(sharedSlsDetector);


#ifdef VERBOSE
   cout << "getDetectorType: generic shared memory of size " << sz << endl;
#endif
   shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    std::cout<<"*** shmget error (server) ***"<< shm_id << std::endl;
    return -1;
  }
  
   /**
      thisDetector pointer is set to the memory address of the shared memory
   */

  sharedSlsDetector* det = (sharedSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (det == (void*)-1) {
    std::cout<<"*** shmat error (server) ***" << std::endl;
    return -1;
  }
    /**
      shm_id returns -1 is shared memory initialization fails
   */
  //shmId=shm_id;




  if (det->alreadyExisting==0) {
  // Detach Memory address
    if (shmdt(det) == -1) {
      perror("shmdt failed\n");
      return 0;
    }
#ifdef VERBOSE
    printf("Shared memory %d detached\n", shm_id);
#endif
    // remove shared memory
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
      perror("shmctl(IPC_RMID) failed\n");
      return 0;
    }
#ifdef VERBOSE
    printf("Shared memory %d deleted\n", shm_id);
#endif
    return 0;
  }

  return 1;



}





slsDetectorDefs::detectorType slsDetector::getDetectorType(int id) {
  
  detectorType t=GENERIC;


   key_t     mem_key=DEFAULT_SHM_KEY+id;
   int       shm_id;
   int sz;

   sz=sizeof(sharedSlsDetector);


#ifdef VERBOSE
   cout << "getDetectorType: generic shared memory of size " << sz << endl;
#endif
   shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    std::cout<<"*** shmget error (server) ***"<< shm_id << std::endl;
    return t;
  }
  
   /**
      thisDetector pointer is set to the memory address of the shared memory
   */

  sharedSlsDetector* det = (sharedSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (det == (void*)-1) {
    std::cout<<"*** shmat error (server) ***" << std::endl;
    return t;
  }
    /**
      shm_id returns -1 is shared memory initialization fails
   */
  //shmId=shm_id;

  t=det->myDetectorType;


  if (det->alreadyExisting==0) {
  // Detach Memory address
    if (shmdt(det) == -1) {
      perror("shmdt failed\n");
      return t;
    }
#ifdef VERBOSE
    printf("Shared memory %d detached\n", shm_id);
#endif
    // remove shared memory
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
      perror("shmctl(IPC_RMID) failed\n");
      return t;
    }
#ifdef VERBOSE
    printf("Shared memory %d deleted\n", shm_id);
#endif
  }

#ifdef VERBOSE
  cout << "Detector type is " << t << endl;
#endif

  return t;


}


int slsDetector::initializeDetectorSize(detectorType type) {
  char  *goff;
  goff=(char*)thisDetector;
  
  //  cout << "init detector size" << endl;

  /** if the shared memory has newly be created, initialize the detector variables */
   if (thisDetector->alreadyExisting==0) {

     // cout << "detector not existing " << endl;

     /** set hostname to default */
     strcpy(thisDetector->hostname,DEFAULT_HOSTNAME);

     /** set client ip address */
     strcpy(thisDetector->clientIP,"none");
     /** set client mac address */
     strcpy(thisDetector->clientMAC,"none");
     /** set server mac address */
     strcpy(thisDetector->serverMAC,"00:aa:bb:cc:dd:ee");

     /** sets onlineFlag to OFFLINE_FLAG */
     thisDetector->onlineFlag=OFFLINE_FLAG;
     /** set ports to defaults */
     thisDetector->controlPort=DEFAULT_PORTNO;
     thisDetector->stopPort=DEFAULT_PORTNO+1;
     thisDetector->dataPort=DEFAULT_PORTNO+2;
     /** set thisDetector->myDetectorType to type and according to this set nChans, nChips, nDacs, nAdcs, nModMax, dynamicRange, nMod*/
     thisDetector->myDetectorType=type;
     switch(thisDetector->myDetectorType) {
     case MYTHEN:
       thisDetector->nChans=128;
       thisDetector->nChips=10;
       thisDetector->nDacs=6;
       thisDetector->nAdcs=0;
       thisDetector->nModMax[X]=24;
       thisDetector->nModMax[Y]=1;
       thisDetector->dynamicRange=24;
       thisDetector->moveFlag=1;
#ifdef VERBOSE
       cout << "move flag" << thisDetector->moveFlag<< endl;
#endif
       break;
     case PICASSO:
       thisDetector->nChans=128;
       thisDetector->nChips=12;
       thisDetector->nDacs=6;
       thisDetector->nAdcs=0;
       thisDetector->nModMax[X]=6;
       thisDetector->nModMax[Y]=1;
       thisDetector->dynamicRange=24;
       break;
     case GOTTHARD:
       thisDetector->nChans=128;
       thisDetector->nChips=10;
       thisDetector->nDacs=8;
       thisDetector->nAdcs=5;
       thisDetector->nModMax[X]=1;
       thisDetector->nModMax[Y]=1;
       thisDetector->dynamicRange=16;
       break;
     default:
       thisDetector->nChans=0;
       thisDetector->nChips=0;
       thisDetector->nDacs=0;
       thisDetector->nAdcs=0;
       thisDetector->nModMax[X]=0;
       thisDetector->nModMax[Y]=0;  
       thisDetector->dynamicRange=32;
     }
     thisDetector->nModsMax=thisDetector->nModMax[0]*thisDetector->nModMax[1];
     /** number of modules is initally the maximum number of modules */
     thisDetector->nMod[X]=thisDetector->nModMax[X];
     thisDetector->nMod[Y]=thisDetector->nModMax[Y];  
     thisDetector->nMods=thisDetector->nModsMax;
     /** calculates the expected data size */
     thisDetector->timerValue[PROBES_NUMBER]=0;
     thisDetector->timerValue[FRAME_NUMBER]=1;
     thisDetector->timerValue[MEASUREMENTS_NUMBER]=1;
     thisDetector->timerValue[CYCLES_NUMBER]=1;
     

     if (thisDetector->dynamicRange==24 || thisDetector->timerValue[PROBES_NUMBER]>0)
       thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
     else
       thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*thisDetector->dynamicRange/8;
     /** set trimDsdir, calDir and filePath to default to home directory*/
     strcpy(thisDetector->settingsDir,getenv("HOME"));
     strcpy(thisDetector->calDir,getenv("HOME"));
     strcpy(thisDetector->filePath,getenv("HOME"));
     /** sets trimbit file */
     strcpy(thisDetector->settingsFile,"none");
     /** set fileName to default to run*/
     strcpy(thisDetector->fileName,"run");
     /** set fileIndex to default to 0*/
     thisDetector->fileIndex=0;
     /** set progress Index to default to 0*/
     thisDetector->progressIndex=0;
     /** set total number of frames to be acquired to default to 1*/
     thisDetector->totalProgress=1;

     /** set number of trim energies to 0*/
     thisDetector->nTrimEn=0;
     /** set correction mask to 0*/
     thisDetector->correctionMask=0;
     /** set deat time*/
     thisDetector->tDead=0;
     /** sets bad channel list file to none */
     strcpy(thisDetector->badChanFile,"none");
     /** sets flat field correction directory */
     strcpy(thisDetector->flatFieldDir,getenv("HOME"));
     /** sets flat field correction file */
     strcpy(thisDetector->flatFieldFile,"none");
     /** set number of bad chans to 0*/
     thisDetector->nBadChans=0;
     /** set number of bad flat field chans to 0*/
     thisDetector->nBadFF=0;
     /** set angular direction to 1*/
     thisDetector->angDirection=1;
     /** set fine offset to 0*/
     thisDetector->fineOffset=0;
     /** set global offset to 0*/
     thisDetector->globalOffset=0;
     /** set number of rois to 0*/
     thisDetector->nROI=0;
     /** set readoutflags to none*/
     thisDetector->roFlags=NORMAL_READOUT;
     /** set current settings to uninitialized*/
     thisDetector->currentSettings=UNINITIALIZED;
     /** set threshold to -1*/
     thisDetector->currentThresholdEV=-1;
     // /** set clockdivider to 1*/
     // thisDetector->clkDiv=1;
     /** set number of positions to 0*/
     thisDetector->numberOfPositions=0;
     /** sets angular conversion file to none */
     strcpy(thisDetector->angConvFile,"none");
     /** set binsize*/
     thisDetector->binSize=0;
     thisDetector->stoppedFlag=0;
     thisDetector->threadedProcessing=1;

     thisDetector->actionMask=0;


     for (int ia=0; ia<MAX_ACTIONS; ia++) {
       strcpy(thisDetector->actionScript[ia],"none");
       strcpy(thisDetector->actionParameter[ia],"none");
     }


     for (int iscan=0; iscan<MAX_SCAN_LEVELS; iscan++) {
       
       thisDetector->scanMode[iscan]=0;
       strcpy(thisDetector->scanScript[iscan],"none");
       strcpy(thisDetector->scanParameter[iscan],"none");
       thisDetector->nScanSteps[iscan]=0;
       thisDetector->scanPrecision[iscan]=0;
     }


     
     /** calculates the memory offsets for flat field coefficients and errors, module structures, dacs, adcs, chips and channels */ 
     thisDetector->ffoff=sizeof(sharedSlsDetector);
     thisDetector->fferroff=thisDetector->ffoff+sizeof(double)*thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
     thisDetector->modoff= thisDetector->fferroff+sizeof(double)*thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
     thisDetector->dacoff=thisDetector->modoff+sizeof(sls_detector_module)*thisDetector->nModsMax;
     thisDetector->adcoff=thisDetector->dacoff+sizeof(dacs_t)*thisDetector->nDacs*thisDetector->nModsMax;
     thisDetector->chipoff=thisDetector->adcoff+sizeof(dacs_t)*thisDetector->nAdcs*thisDetector->nModsMax;
     thisDetector->chanoff=thisDetector->chipoff+sizeof(int)*thisDetector->nChips*thisDetector->nModsMax;
     
     
   } 


     /** also in case thisDetector alread existed initialize the pointer for flat field coefficients and errors, module structures, dacs, adcs, chips and channels */ 
   ffcoefficients=(double*)(goff+thisDetector->ffoff);
   fferrors=(double*)(goff+thisDetector->fferroff);
   detectorModules=(sls_detector_module*)(goff+ thisDetector->modoff);
#ifdef VERBOSE
   //   for (int imod=0; imod< thisDetector->nModsMax; imod++)
   //  std::cout<< hex << detectorModules+imod << dec <<std::endl;
#endif
   dacs=(dacs_t*)(goff+thisDetector->dacoff);
   adcs=(dacs_t*)(goff+thisDetector->adcoff);
   chipregs=(int*)(goff+thisDetector->chipoff);
   chanregs=(int*)(goff+thisDetector->chanoff);
   if (thisDetector->alreadyExisting==0) {  
     /** if thisDetector is new, initialize its structures \sa initializeDetectorStructure();   */ 
     initializeDetectorStructure();   
     /** set  thisDetector->alreadyExisting=1  */   
     thisDetector->alreadyExisting=1;
   } 

#ifdef VERBOSE
   cout << "passing pointers" << endl;
#endif   


   stoppedFlag=&thisDetector->stoppedFlag;
   threadedProcessing=&thisDetector->threadedProcessing;
   actionMask=&thisDetector->actionMask;
   actionScript=thisDetector->actionScript;		
   actionParameter=thisDetector->actionParameter;      
   nScanSteps=thisDetector->nScanSteps;
   scanMode=thisDetector->scanMode;
   scanScript=thisDetector->scanScript;
   scanParameter=thisDetector->scanParameter;
   scanSteps=thisDetector->scanSteps;
   scanPrecision=thisDetector->scanPrecision;
   numberOfPositions=&thisDetector->numberOfPositions;
   detPositions=thisDetector->detPositions;
   angConvFile=thisDetector->angConvFile;
   correctionMask=&thisDetector->correctionMask;
   binSize=&thisDetector->binSize;
   fineOffset=&thisDetector->fineOffset;
   globalOffset=&thisDetector->globalOffset;
   angDirection=&thisDetector->angDirection;
   flatFieldDir=thisDetector->flatFieldDir;
   flatFieldFile=thisDetector->flatFieldFile;
   badChanFile=thisDetector->badChanFile;
   timerValue=thisDetector->timerValue;
   currentSettings=&thisDetector->currentSettings;
   currentThresholdEV=&thisDetector->currentThresholdEV;
   filePath=thisDetector->filePath;
   fileName=thisDetector->fileName;
   fileIndex=&thisDetector->fileIndex;
   moveFlag=&thisDetector->moveFlag;

   settingsFile=thisDetector->settingsFile;

   //  setAngularConversionPointer(thisDetector->angOff,&thisDetector->nMods, thisDetector->nChans*thisDetector->nChips);

#ifdef VERBOSE
   cout << "done" << endl;
#endif   


   /** modifies the last PID accessing the detector */
   thisDetector->lastPID=getpid();

#ifdef VERBOSE
   cout << "Det size initialized " << endl;
#endif

   return OK;
}

int slsDetector::initializeDetectorStructure() {
  sls_detector_module *thisMod;
  char *p2;
  p2=(char*)thisDetector;
 
  /** for each of the detector modules up to the maximum number which can be installed initlialize the sls_detector_module structure \sa ::sls_detector_module*/
  for (int imod=0; imod<thisDetector->nModsMax; imod++) {

   

    thisMod=detectorModules+imod;
    thisMod->module=imod;
   
    /** sets the size of the module to nChans, nChips etc. */
    thisMod->nchan=thisDetector->nChans*thisDetector->nChips;
    thisMod->nchip=thisDetector->nChips;
    thisMod->ndac=thisDetector->nDacs;
    thisMod->nadc=thisDetector->nAdcs;
    
    
    /** initializes the serial number and register to 0 */
    thisMod->serialnumber=0;
    thisMod->reg=0;

    /** initializes the dacs values to 0 */
    for (int idac=0; idac<thisDetector->nDacs; idac++) {
      *(dacs+idac+thisDetector->nDacs*imod)=0.;
    }
    

    /** initializes the adc values to 0 */
  for (int iadc=0; iadc<thisDetector->nAdcs; iadc++) {
    *(adcs+iadc+thisDetector->nAdcs*imod)=0.;
    }



    /** initializes the chip registers to 0 */
    for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
      *(chipregs+ichip+thisDetector->nChips*imod)=-1;
    }
    
    
    /** initializes the channel registers to 0 */
    for (int ichan=0; ichan<thisDetector->nChans*thisDetector->nChips; ichan++) {
      *(chanregs+ichan+thisDetector->nChips*thisDetector->nChans*imod)=-1;
    }
    /** initialize gain and offset to -1 */
    thisMod->gain=-1.;
    thisMod->offset=-1.;
  }
  return 0;
}

slsDetectorDefs::sls_detector_module*  slsDetector::createModule(detectorType t) {

  sls_detector_module *myMod=(sls_detector_module*)malloc(sizeof(sls_detector_module));


  int nch, nm, nc, nd, na=0;

   switch(t) {
   case MYTHEN:
     nch=128; // complete mythen system
     nm=24;
     nc=10;
     nd=6; // dacs
     break;
   case PICASSO:
     nch=128; // complete mythen system
     nm=24;
     nc=12;
     nd=6; // dacs+adcs
     break;
   case GOTTHARD:
     nch=128; 
     nm=1;
     nc=10;
     nd=8; // dacs+adcs
     na=5;
     break;
   case EIGER:
     nch=65536; // one EIGER half module
     nm=1; //modules/detector
     nc=4; //chips
     nd=16; //dacs
     na=16; 
   default:
     nch=0; // dum!
     nm=0; //modules/detector
     nc=0; //chips
     nd=0; //dacs+adcs
     na=0;
   }

  dacs_t *dacs=new dacs_t[nd];
  dacs_t *adcs=new dacs_t[na];
  int *chipregs=new int[nc];
  int *chanregs=new int[nch*nc];
  myMod->ndac=nd;
  myMod->nadc=na;
  myMod->nchip=nc;
  myMod->nchan=nch*nc;
 
  myMod->dacs=dacs;
  myMod->adcs=adcs;
  myMod->chipregs=chipregs;
  myMod->chanregs=chanregs;
  return myMod;
}


void  slsDetector::deleteModule(sls_detector_module *myMod) {
  delete [] myMod->dacs;
  delete [] myMod->adcs;
  delete [] myMod->chipregs;
  delete [] myMod->chanregs;
  delete myMod;
}



int slsDetector::sendChannel(sls_detector_channel *myChan) {
  return  controlSocket->SendDataOnly(myChan, sizeof(sls_detector_channel));
}

int slsDetector::sendChip(sls_detector_chip *myChip) {
  int ts=0;
  ts+=controlSocket->SendDataOnly(myChip,sizeof(sls_detector_chip));
#ifdef VERY_VERBOSE
  std::cout<< "chip structure sent" << std::endl;
  std::cout<< "now sending " << myChip->nchan << " channles" << std::endl;
#endif

  ts=controlSocket->SendDataOnly(myChip->chanregs,sizeof(int)*myChip->nchan );

#ifdef VERBOSE
  std::cout<< "chip's channels sent " <<ts  << std::endl;
#endif
  return ts;			  
}

int slsDetector::sendModule(sls_detector_module *myMod) {
  int ts=0;
  ts+=controlSocket->SendDataOnly(myMod,sizeof(sls_detector_module));
  ts+=controlSocket->SendDataOnly(myMod->dacs,sizeof(dacs_t)*(myMod->ndac));
  ts+=controlSocket->SendDataOnly(myMod->adcs,sizeof(dacs_t)*(myMod->nadc));
  ts+=controlSocket->SendDataOnly(myMod->chipregs,sizeof(int)*(myMod->nchip));
  ts+=controlSocket->SendDataOnly(myMod->chanregs,sizeof(int)*(myMod->nchan));
  return ts;
}

int slsDetector::receiveChannel(sls_detector_channel *myChan) {
  return controlSocket->ReceiveDataOnly(myChan,sizeof(sls_detector_channel));
}

int slsDetector::receiveChip(sls_detector_chip* myChip) {
  int *ptr=myChip->chanregs;
  int nchanold=myChip->nchan;
  int ts=0;
  int nch;
  ts+=controlSocket->ReceiveDataOnly(myChip,sizeof(sls_detector_chip));
  myChip->chanregs=ptr;
  if (nchanold<(myChip->nchan)) {
    nch=nchanold;
    printf("number of channels received is too large!\n");
  } else
    nch=myChip->nchan;

  ts+=controlSocket->ReceiveDataOnly(myChip->chanregs,sizeof(int)*nch);
 
  return ts;
}

int  slsDetector::receiveModule(sls_detector_module* myMod) {

  dacs_t *dacptr=myMod->dacs;
  dacs_t *adcptr=myMod->adcs;
  int *chipptr=myMod->chipregs;
  int *chanptr=myMod->chanregs;
  int ts=0;
  ts+=controlSocket->ReceiveDataOnly(myMod,sizeof(sls_detector_module));
  myMod->dacs=dacptr;
  myMod->adcs=adcptr;
  myMod->chipregs=chipptr;
  myMod->chanregs=chanptr;
  
#ifdef VERBOSE
  std::cout<< "received module " << myMod->module << " of size "<< ts << " register " << myMod->reg << std::endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(dacs_t)*(myMod->ndac));
#ifdef VERBOSE
  std::cout<< "received dacs " << myMod->module << " of size "<< ts << std::endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(dacs_t)*(myMod->nadc));
#ifdef VERBOSE
  std::cout<< "received adcs " << myMod->module << " of size "<< ts << std::endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->chipregs,sizeof(int)*(myMod->nchip));
#ifdef VERBOSE
  std::cout<< "received chips " << myMod->module << " of size "<< ts << std::endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->chanregs,sizeof(int)*(myMod->nchan));
#ifdef VERBOSE
  std::cout<< "nchans= " << thisDetector->nChans << " nchips= " << thisDetector->nChips;
  std::cout<< "mod - nchans= " << myMod->nchan << " nchips= " <<myMod->nchip;
  
  std::cout<< "received chans " << myMod->module << " of size "<< ts << std::endl;
#endif
#ifdef VERBOSE
  std::cout<< "received module " << myMod->module << " of size "<< ts << " register " << myMod->reg << std::endl;
#endif

  return ts;
}


int slsDetector::setOnline(int off) {
  if (off!=GET_ONLINE_FLAG) {
    thisDetector->onlineFlag=off;
    if (thisDetector->onlineFlag==ONLINE_FLAG)
      setTCPSocket();
  }
  return thisDetector->onlineFlag;
}




  /* 
     configure the socket communication and check that the server exists 
     enum communicationProtocol{
     TCP,
     UDP
     }{};

  */

int slsDetector::setTCPSocket(string const name, int const control_port, int const stop_port,  int const data_port){


  char thisName[MAX_STR_LENGTH];
  int thisCP, thisSP, thisDP;
  int retval=OK;

  if (strcmp(name.c_str(),"")!=0) {
#ifdef VERBOSE
    std::cout<< "setting hostname" << std::endl;
#endif
    strcpy(thisName,name.c_str());
    strcpy(thisDetector->hostname,thisName);
    if (controlSocket) {
       delete controlSocket;
       controlSocket=NULL;
    }
    if (stopSocket) {
       delete stopSocket;
       stopSocket=NULL;
    }
    if (dataSocket){
       delete dataSocket; 
       dataSocket=NULL;
    }
  } else
    strcpy(thisName,thisDetector->hostname);
    
  if (control_port>0) {
#ifdef VERBOSE
    std::cout<< "setting control port" << std::endl;
#endif
    thisCP=control_port;
    thisDetector->controlPort=thisCP; 
    if (controlSocket) {
       delete controlSocket;
       controlSocket=NULL;
    }
  } else
    thisCP=thisDetector->controlPort;

  if (stop_port>0) {
#ifdef VERBOSE
    std::cout<< "setting stop port" << std::endl;
#endif
    thisSP=stop_port;
    thisDetector->stopPort=thisSP;
    if (stopSocket) {
       delete stopSocket;
       stopSocket=NULL;
    }
  } else
    thisSP=thisDetector->stopPort;


  if (data_port>0) {
#ifdef VERBOSE
    std::cout<< "setting data port" << std::endl;
#endif
    thisDP=data_port;
    thisDetector->dataPort=thisDP;
    if (dataSocket){
       delete dataSocket; 
       dataSocket=NULL;
    }
  } else
    thisDP=thisDetector->dataPort;


  if (!controlSocket) {
    controlSocket= new MySocketTCP(thisName, thisCP);
    if (controlSocket->getErrorStatus()){
#ifdef VERBOSE
      std::cout<< "Could not connect Control socket " << thisName  << " " << thisCP << std::endl;
#endif 
      delete controlSocket;
      controlSocket=NULL;
      retval=FAIL;
    }
#ifdef VERYVERBOSE
    else
      std::cout<< "Control socket connected " <<thisName  << " " << thisCP << std::endl;
#endif
  }
  if (!stopSocket) {
    stopSocket=new MySocketTCP(thisName, thisSP);
    if (stopSocket->getErrorStatus()){
#ifdef VERBOSE
    std::cout<< "Could not connect Stop socket "<<thisName  << " " << thisSP << std::endl;
#endif
    delete stopSocket;
    stopSocket=NULL;
    retval=FAIL;
    } 
#ifdef VERYVERBOSE
    else
      std::cout<< "Stop socket connected " << thisName << " " << thisSP << std::endl;
#endif
  }
  if (!dataSocket) {
  dataSocket=new MySocketTCP(thisName, thisDP);
  if (dataSocket->getErrorStatus()){
#ifdef VERBOSE
    std::cout<< "Could not connect Data socket "<<thisName  << " " << thisDP << std::endl;
#endif
    

    delete dataSocket;
    dataSocket=NULL;


    retval=FAIL;
  } 
#ifdef VERYVERBOSE
  else
    std::cout<< "Data socket connected "<< thisName << " " << thisDP << std::endl;
#endif
  }
  if (retval!=FAIL) {
    if (controlSocket->Connect()<0) {
      controlSocket->SetTimeOut(5);
      thisDetector->onlineFlag=OFFLINE_FLAG;
      delete controlSocket; 
      controlSocket=NULL;
      retval=FAIL;
#ifdef VERBOSE
    std::cout<< "offline!" << std::endl;
#endif
    }  else {
      thisDetector->onlineFlag=ONLINE_FLAG;
      controlSocket->SetTimeOut(100);
      controlSocket->Disconnect();
#ifdef VERBOSE
      std::cout<< "online!" << std::endl;
#endif
    }
  } else {
      thisDetector->onlineFlag=OFFLINE_FLAG;
#ifdef VERBOSE
    std::cout<< "offline!" << std::endl;
#endif
    
  }


  return retval;
};


  /** connect to the control port */
int slsDetector::connectControl() {
  if (controlSocket)
    return controlSocket->Connect();
  return FAIL;
}
  /** disconnect from the control port */
int slsDetector::disconnectControl() {
  if (controlSocket)
    controlSocket->Disconnect();
  return OK;
}



  /** connect to the data port */
  int slsDetector::connectData() {
  if (dataSocket)
    return dataSocket->Connect();
  return FAIL;
};
  /** disconnect from the data port */
  int slsDetector::disconnectData(){
  if (dataSocket)
    dataSocket->Disconnect();
  return OK;
}
;

  /** connect to the stop port */
  int slsDetector::connectStop() {
  if (stopSocket)
    return stopSocket->Connect();
  return FAIL;
};
  /** disconnect from the stop port */
  int slsDetector::disconnectStop(){
  if (stopSocket)
    stopSocket->Disconnect();
  return OK;
}
;

















  /* Communication to server */

  // General purpose functions

  /* 
     executes a system command on the server 
     e.g. mount an nfs disk, reboot and returns answer etc.
  */
int slsDetector::execCommand(string cmd, string answer){

  char arg[MAX_STR_LENGTH], retval[MAX_STR_LENGTH];
  int fnum=F_EXEC_COMMAND;
  
  int ret=FAIL;

  strcpy(arg,cmd.c_str());

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Sending command " << arg << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if (controlSocket->Connect()>=0) {
	if (controlSocket->SendDataOnly(&fnum,sizeof(fnum))>=0) {
	  if (controlSocket->SendDataOnly(arg,MAX_STR_LENGTH)>=0) {
	    if (controlSocket->ReceiveDataOnly(retval,MAX_STR_LENGTH)>=0) {
	      ret=OK;
	      answer=retval;
	    }
	  }
	}
	controlSocket->Disconnect();
      }
    }
#ifdef VERBOSE
    std::cout<< "Detector answer is " << answer << std::endl; 
#endif
  }
  return ret;
};

// Detector configuration functions

  /* 
     the detector knows what type of detector it is 

     enum detectorType{
     GET_DETECTOR_TYPE,
     GENERIC,
     MYTHEN,
     PILATUS,
     EIGER,
     GOTTHARD,
     AGIPD
     };

  */
int slsDetector::setDetectorType(detectorType const type){
  
  int arg, retval=FAIL;
  int fnum=F_GET_DETECTOR_TYPE;
  arg=int(type);
  detectorType retType=type;
  char mess[100];
  strcpy(mess,"dummy");


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting detector type to " << arg << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	if (retval!=FAIL)
	  controlSocket->ReceiveDataOnly(&retType,sizeof(retType));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (retval==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    if (type==GET_DETECTOR_TYPE)
      retType=thisDetector->myDetectorType;
    else {
      retType=type;
      thisDetector->myDetectorType=type;
   }
    retval=OK;
  }
#ifdef VERBOSE
  std::cout<< "Detector type set to " << retType << std::endl; 
#endif
  if (retval==FAIL) {
    std::cout<< "Set detector type failed " << std::endl;
    retType=GENERIC;
  }
  else
    thisDetector->myDetectorType=retType;


  return retType;
};

int slsDetector::setDetectorType(string const stype){
  return setDetectorType(getDetectorType(stype));
};

slsDetectorDefs::detectorType slsDetector::getDetectorsType(int pos){
  return thisDetector->myDetectorType;
}




  /* needed to set/get the size of the detector */
// if n=GET_FLAG returns the number of installed modules,
int slsDetector::setNumberOfModules(int n, dimension d){
  
  int arg[2], retval;
  int fnum=F_SET_NUMBER_OF_MODULES;
  int ret=FAIL;
  char mess[100]; 


  arg[0]=d;
  arg[1]=n;


  if (d<X || d>Y) {
    std::cout<< "Set number of modules in wrong dimension " << d << std::endl;
    return ret;
  }


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting number of modules of dimension "<< d <<  " to " << n << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL) {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	}	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    ret=OK;
    if (n==GET_FLAG)
      ;
    else {
      if (n<=0 || n>thisDetector->nModMax[d]) {
	ret=FAIL;
      } else {
	thisDetector->nMod[d]=n;
      }
    }
    retval=thisDetector->nMod[d];
  }
#ifdef VERBOSE
    std::cout<< "Number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
    if (ret==FAIL) {
      std::cout<< "Set number of modules failed " << std::endl;
    }  else {
      thisDetector->nMod[d]=retval;
      thisDetector->nMods=thisDetector->nMod[X]*thisDetector->nMod[Y];
      int dr=thisDetector->dynamicRange;
      if (dr==24)
	dr=32;
      
      if (thisDetector->timerValue[PROBES_NUMBER]==0) {
	thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*dr/8;
      } else {
	thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
      }
      
#ifdef VERBOSE
      std::cout<< "Data size is " << thisDetector->dataBytes << std::endl;
      std::cout<< "nModX " << thisDetector->nMod[X] << " nModY " << thisDetector->nMod[Y] << " nChips " << thisDetector->nChips << " nChans " << thisDetector->nChans<< " dr " << dr << std::endl;
#endif
    }
    return thisDetector->nMod[d];
}; 



 
int slsDetector::getMaxNumberOfModules(dimension d){

  int retval;
  int fnum=F_GET_MAX_NUMBER_OF_MODULES;
  int ret=FAIL;
  char mess[100]; 

  if (d<X || d>Y) {
    std::cout<< "Get max number of modules in wrong dimension " << d << std::endl;
    return ret;
  }
#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting max number of modules in dimension "<< d  <<std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&d,sizeof(d));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    ret=OK;
    retval=thisDetector->nModMax[d];
  }
#ifdef VERBOSE
    std::cout<< "Max number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
    if (ret==FAIL) {
      std::cout<< "Get max number of modules failed " << std::endl;
      return retval;
    }  else {
      thisDetector->nModMax[d]=retval;
      thisDetector->nModsMax=thisDetector->nModMax[0]*thisDetector->nModMax[1];
    }
    return thisDetector->nModMax[d];
}; 


 
  /*
    This function is used to set the polarity and meaning of the digital I/O signals (signal index)
    
enum externalSignalFlag {
  GET_EXTERNAL_SIGNAL_FLAG,
  SIGNAL_OFF,
  GATE_ACTIVE_HIGH,
  GATE_ACTIVE_LOW,
  TRIGGER_RISING_EDGE,
  TRIGGER_FALLING_EDGE
}{};
  */

slsDetectorDefs::externalSignalFlag slsDetector::setExternalSignalFlags(externalSignalFlag pol, int signalindex){



  
  int arg[2]; 
  externalSignalFlag  retval;
  int ret=FAIL;
  int fnum=F_SET_EXTERNAL_SIGNAL_FLAG;
  char mess[100];

  arg[0]=signalindex;
  arg[1]=pol;

  retval=GET_EXTERNAL_SIGNAL_FLAG;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting signal "<< signalindex <<  " to flag" << pol << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    retval=GET_EXTERNAL_SIGNAL_FLAG;
    ret=FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Signal "<< signalindex <<  " flag set to" << retval << std::endl;
  if (ret==FAIL) {
    std::cout<< "Set signal flag failed " << std::endl;
  }
#endif
  return retval;






};

  /* 
     this function is used to select wether the detector is triggered or gated and in which mode
    enum externalCommunicationMode{
  GET_EXTERNAL_COMMUNICATION_MODE,
  AUTO,
  TRIGGER_EXPOSURE,
  TRIGGER_READOUT,
  TRIGGER_COINCIDENCE_WITH_INTERNAL_ENABLE,
  GATE_FIX_NUMBER,
  GATE_FIX_DURATION,
  GATE_WITH_START_TRIGGER,
  GATE_COINCIDENCE_WITH_INTERNAL_ENABLE
};

  */

slsDetectorDefs::externalCommunicationMode slsDetector::setExternalCommunicationMode( externalCommunicationMode pol){



  
  int arg[1]; 
  externalCommunicationMode  retval;
  int fnum=F_SET_EXTERNAL_COMMUNICATION_MODE;
  char mess[100];
    
  arg[0]=pol;

  int ret=FAIL;
  retval=GET_EXTERNAL_COMMUNICATION_MODE;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting communication to mode " << pol << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {  
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    retval=GET_EXTERNAL_COMMUNICATION_MODE;
    ret=FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Communication mode "<<   " set to" << retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Setting communication mode failed" << std::endl;
  }
  return retval;

};


  // Tests and identification
  /* 
     Gets versions

     enum idMode{
     MODULE_SERIAL_NUMBER,
     MODULE_FIRMWARE_VERSION,
     DETECTOR_SERIAL_NUMBER,
     DETECTOR_FIRMWARE_VERSION,
     DETECTOR_SOFTWARE_VERSION 
     }{};

  */





int64_t slsDetector::getId( idMode mode, int imod){


  int64_t retval=-1;
  int fnum=F_GET_ID;
  int ret=FAIL;

  char mess[100];

#ifdef VERBOSE
  std::cout<< std::endl;
  if  (mode==MODULE_SERIAL_NUMBER)
    std::cout<< "Getting id  of "<< imod << std::endl; 
  else
     std::cout<< "Getting id type "<< mode << std::endl; 
#endif
  if (mode==THIS_SOFTWARE_VERSION) {
    ret=OK;
    retval=thisSoftwareVersion;
  } else {
    if (thisDetector->onlineFlag==ONLINE_FLAG) {
      if (controlSocket) {
	if  (controlSocket->Connect()>=0) {
	  controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	  controlSocket->SendDataOnly(&mode,sizeof(mode));
	  if (mode==MODULE_SERIAL_NUMBER)
	    controlSocket->SendDataOnly(&imod,sizeof(imod));
	  controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	  if (ret!=FAIL)
	    controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  else {
	    controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	    std::cout<< "Detector returned error: " << mess << std::endl;
	  }
	  controlSocket->Disconnect();
	  if (ret==FORCE_UPDATE)
	    updateDetector();
	} else 
	  ret=FAIL;
      } else {
	ret=FAIL;
      }
    }
  }
  if (ret==FAIL) {
    std::cout<< "Get id failed " << std::endl;
    return ret;
  } else {
#ifdef VERBOSE
    if  (mode==MODULE_SERIAL_NUMBER)
      std::cout<< "Id of "<< imod <<" is " << hex <<retval << setbase(10) << std::endl;
    else
      std::cout<< "Id "<< mode <<" is " << hex <<retval << setbase(10) << std::endl;
#endif
    return retval;
  }
};



  /*
    Digital test of the modules

    enum digitalTestMode {
    CHIP_TEST,
    MODULE_FIRMWARE_TEST,
    DETECTOR_FIRMWARE_TEST,
    DETECTOR_MEMORY_TEST,
    DETECTOR_BUS_TEST,
    DETECTOR_SOFTWARE_TEST
    }{};
    returns ok or error mask
  */

int slsDetector::digitalTest( digitalTestMode mode, int imod){


  int retval;
  int fnum=F_DIGITAL_TEST;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting id of "<< mode << std::endl; 
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&mode,sizeof(mode));
	if ((mode==CHIP_TEST)|| (mode==DIGITAL_BIT_TEST))
	   controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    ret=FAIL;
  }
#ifdef VERBOSE
    std::cout<< "Id "<< mode <<" is " << retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Get id failed " << std::endl;
    return ret;
  } else
    return retval;
};



  /* 
     analog test of the modules
     enum analogTestMode {
     COUNT_CALIBRATION_PULSES,
     I_DON_T_KNOW
     }{};

  */
/*
int* slsDetector::analogTest(analogTestMode mode){
  std::cout<< "function not yet implemented " << std::endl;
};
*/
  /* 
     enable analog output of channel 
  */
/*
int slsDetector::enableAnalogOutput(int ichan){
  int imod=ichan/(nChans*nChips);
  ichan-=imod*(nChans*nChips);
  int ichip=ichan/nChans;
  ichan-=ichip*(nChans);
  enableAnalogOutput(imod,ichip,ichan);
  
};
int slsDetector::enableAnalogOutput(int imod, int ichip, int ichan){
  std::cout<< "function not yet implemented " << std::endl;
};
*/
  /* 
     give a train of calibration pulses 
  */ 
/*
int slsDetector::giveCalibrationPulse(double vcal, int npulses){
  std::cout<< "function not yet implemented " << std::endl;
};
*/
  // Expert low level functions



  /* write or read register */

int slsDetector::writeRegister(int addr, int val){


  int retval;
  int fnum=F_WRITE_REGISTER;
  int ret=FAIL;

  char mess[100];
  
  int arg[2];
  arg[0]=addr;
  arg[1]=val;
  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Writing to register "<< hex<<addr <<  " data " << hex<<val << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } 
#ifdef VERBOSE
  std::cout<< "Register returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Write to register failed " << std::endl;
  }
  return retval;

};




int slsDetector::readRegister(int addr){


  int retval;
  int fnum=F_READ_REGISTER;
  int ret=FAIL;

  char mess[100];

  int arg;
  arg=addr;
  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Reading register "<< hex<<addr  << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } 
#ifdef VERBOSE
  std::cout<< "Register returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Read register failed " << std::endl;
  }
  return retval;

};


  // Expert initialization functions
  /*
    set dacs or read ADC for the module
    enum dacIndex {
    TRIMBIT_SIZE,
    THRESHOLD,
    SHAPER1,
    SHAPER2,
    CALIBRATION_PULSE,
    PREAMP,
    TEMPERATURE,
    HUMIDITY,
}{};
  */

dacs_t slsDetector::setDAC(dacs_t val, dacIndex index, int imod){


  dacs_t retval;
  int fnum=F_SET_DAC;
  int ret=FAIL;
  char mess[100];
  int arg[2];
  arg[0]=index;
  arg[1]=imod;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting DAC "<< index << " of module " << imod  <<  " to " << val << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->SendDataOnly(&val,sizeof(val));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL) {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  if (index <  thisDetector->nDacs){

	    if (dacs) {
	      if (imod>=0) {
		*(dacs+index+imod*thisDetector->nDacs)=retval;
	      }
	      else {
		for (imod=0; imod<thisDetector->nModsMax; imod++)
		  *(dacs+index+imod*thisDetector->nDacs)=retval;
	      }
	    }
	  }
	} else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
	
    }
  }
#ifdef VERBOSE
  std::cout<< "Dac set to "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Set dac failed " << std::endl;
  }
  return retval;
};


dacs_t slsDetector::getADC(dacIndex index, int imod){

  dacs_t retval;
  int fnum=F_GET_ADC;
  int ret=FAIL;
  char mess[100];

  int arg[2];
  arg[0]=index;
  arg[1]=imod;

  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting ADC "<< index << " of module " << imod  <<   std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL) {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  if (adcs) {
	      *(adcs+index+imod*thisDetector->nAdcs)=retval;
	  }
	} else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	  }
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } 
#ifdef VERBOSE
  std::cout<< "ADC returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Get ADC failed " << std::endl;
  }
  return retval;



}; 

  /* 
     configure single channel 
     enum channelRegisterBit {
     COMPARATOR_ENABLE_OFF,
     ANALOG_SIGNAL_ENABLE_OFF,
     CALIBRATION_ENABLE_OFF,
     TRIMBIT_OFF // should always be the last!
     }

  */

int slsDetector::setChannel(int64_t reg, int ichan, int ichip, int imod){
  sls_detector_channel myChan;
#ifdef VERBOSE
  std::cout<< "Setting channel "<< ichan << " " << ichip << " " << imod << " to " << reg << std::endl;
#endif
  //int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan, chamax=ichan+1;

  int ret;

  /*  if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  if (ichip==-1) {
    chimin=0;
    chimax=thisDetector->nChips;
  }

  if (ichan==-1) {
    chamin=0;
    chamax=thisDetector->nChans;
    }*/

  // for (int im=mmin; im<mmax; im++) {
  //  for (int ichi=chimin; ichi<chimax; ichi++) {
  //    for (int icha=chamin; icha<chamax; icha++) {
  myChan.chan=ichan;//icha;
  myChan.chip=ichip;//ichi;
  myChan.module=imod;//im;
  myChan.reg=reg;
  ret=setChannel(myChan);	 
	//     }
	// }
	// }
  return ret;
}



int slsDetector::setChannel(sls_detector_channel chan){
  int fnum=F_SET_CHANNEL;
  int retval;
  int ret=FAIL;
  char mess[100];

  int ichan=chan.chan;
  int ichip=chan.chip;
  int imod=chan.module;

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendChannel(&chan);
      
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
    }
  }


  if (ret!=FAIL) {
    if (chanregs) {

      int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan, chamax=ichan+1;

      if (imod==-1) {
	mmin=0;
	mmax=thisDetector->nModsMax;
      }

      if (ichip==-1) {
	chimin=0;
	chimax=thisDetector->nChips;
      }
      
      if (ichan==-1) {
	chamin=0;
	chamax=thisDetector->nChans;
      }
      





      for (int im=mmin; im<mmax; im++) {
	for (int ichi=chimin; ichi<chimax; ichi++) {
	  for (int icha=chamin; icha<chamax; icha++) {
	    
	    *(chanregs+im*thisDetector->nChans*thisDetector->nChips+ichi*thisDetector->nChips+icha)=retval;     
	    
	  }
	}
      }
      
    }
  }
#ifdef VERBOSE
  std::cout<< "Channel register returned "<<  retval << std::endl;
#endif
  return retval;
  
}


















slsDetectorDefs::sls_detector_channel  slsDetector::getChannel(int ichan, int ichip, int imod){


  int fnum=F_GET_CHANNEL;
  sls_detector_channel myChan;
  int arg[3];
  int ret=FAIL;
  char mess[100];
  arg[0]=ichan;
  arg[1]=ichip;
  arg[2]=imod;  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
   
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	receiveChannel(&myChan);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
      if (ret==FORCE_UPDATE)
	  updateDetector();
    }
    }
  }
  

  if (ret!=FAIL) {
    if (chanregs) {
      *(chanregs+imod*thisDetector->nChans*thisDetector->nChips+ichip*thisDetector->nChips+ichan)=myChan.reg;    
    }
  }

#ifdef VERBOSE
  std::cout<< "Returned channel "<< ichan << " " << ichip << " " << imod << " " <<  myChan.reg << std::endl;
#endif
  return myChan;
}

    /* 
       configure chip
       enum chipRegisterBit {
       ENABLE_ANALOG_OUTPUT,
       OUTPUT_WIDTH // should always be the last
       }{};
  */
int slsDetector::setChip(int reg, int ichip, int imod){
   sls_detector_chip myChip;

#ifdef VERBOSE
  std::cout<< "Setting chip "<<  ichip << " " << imod << " to " << reg <<  std::endl;
#endif


  int chregs[thisDetector->nChans];
  int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1;
  int ret=FAIL;
  if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  if (ichip==-1) {
    chimin=0;
    chimax=thisDetector->nChips;
  }

  myChip.nchan=thisDetector->nChans;
  myChip.reg=reg;
  for (int im=mmin; im<mmax; im++) {
    for (int ichi=chimin; ichi<chimax; ichi++) {
      myChip.chip=ichi;
      myChip.module=im;
      if (chanregs)
	myChip.chanregs=(chanregs+ichi*thisDetector->nChans+im*thisDetector->nChans*thisDetector->nChips); 
      else {
	for (int i=0; i<thisDetector->nChans; i++)
	  chregs[i]=-1;
	myChip.chanregs=chregs;
      }
      ret=setChip(myChip);
    }
  }
  return ret;
}

int slsDetector::setChip(sls_detector_chip chip){

  int fnum=F_SET_CHIP;
  int retval;
  int ret=FAIL;
  char mess[100];

  int ichi=chip.chip;
  int im=chip.module;





  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendChip(&chip);
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
      if (ret==FORCE_UPDATE)
	  updateDetector();
    }
    }
  }


  if (ret!=FAIL) {
    if (chipregs)
      *(chipregs+ichi+im*thisDetector->nChips)=retval;
  }

#ifdef VERBOSE
  std::cout<< "Chip register returned "<<  retval << std::endl;
#endif
  return retval;
};


 slsDetectorDefs::sls_detector_chip slsDetector::getChip(int ichip, int imod){

  int fnum=F_GET_CHIP;
  sls_detector_chip myChip;
  int chanreg[thisDetector->nChans];
 
  int ret=FAIL;
  char mess[100];


  myChip.chip=ichip;
  myChip.module=imod;
  myChip.nchan=thisDetector->nChans;
  myChip.chanregs=chanreg;

  int arg[2];
  arg[0]=ichip;
  arg[1]=imod;




  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
   
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	receiveChip(&myChip);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
    }
    }
  }
  

  if (ret!=FAIL) {
    if (chipregs)
      *(chipregs+ichip+imod*thisDetector->nChips)=myChip.reg;
    if (chanregs) {
      for (int ichan=0; ichan<thisDetector->nChans; ichan++)
	*(chanregs+imod*thisDetector->nChans*thisDetector->nChips+ichip*thisDetector->nChans+ichan)=*((myChip.chanregs)+ichan);
    }
  }
#ifdef VERBOSE
  std::cout<< "Returned chip "<<  ichip << " " << imod << " " <<  myChip.reg << std::endl;
#endif

  return myChip;
};
 
  /* 
     configure module
     enum moduleRegisterBit {
     I_DON_T_KNOW,
     OUTPUT_WIDTH // should always be the last
     }{};
  */

int slsDetector::setModule(int reg, int imod){
  sls_detector_module myModule;
  
#ifdef VERBOSE
    std::cout << "slsDetector set module " << std::endl;
#endif 
  int charegs[thisDetector->nChans*thisDetector->nChips];
  int chiregs[thisDetector->nChips];
  dacs_t das[thisDetector->nDacs], ads[thisDetector->nAdcs];
  int mmin=imod, mmax=imod+1;
  int ret=FAIL;
  
  if (imod==-1) {
    mmin=0;
    mmax=thisDetector->nModsMax;
  }

  
  
  for (int im=mmin; im<mmax; im++) {
     
    myModule.module=im;
    myModule.nchan=thisDetector->nChans;
    myModule.nchip=thisDetector->nChips;
    myModule.ndac=thisDetector->nDacs;
    myModule.nadc=thisDetector->nAdcs;
    
    myModule.reg=reg;
    if (detectorModules) {
      myModule.gain=(detectorModules+im)->gain;
      myModule.offset=(detectorModules+im)->offset;
      myModule.serialnumber=(detectorModules+im)->serialnumber;
    } else {
      myModule.gain=-1;
      myModule.offset=-1;
      myModule.serialnumber=-1;
    }
    
   
    for (int i=0; i<thisDetector->nAdcs; i++)
      ads[i]=-1;
    
    if (chanregs)
      myModule.chanregs=chanregs+im*thisDetector->nChips*thisDetector->nChans;
    else {	
      for (int i=0; i<thisDetector->nChans*thisDetector->nChips; i++)
	charegs[i]=-1;
      myModule.chanregs=charegs;
    }
    if (chipregs)
      myModule.chipregs=chanregs+im*thisDetector->nChips;
    else { 
      for (int ichip=0; ichip<thisDetector->nChips; ichip++)
	chiregs[ichip]=-1;
      myModule.chipregs=chiregs;
    }
    if (dacs)
      myModule.dacs=dacs+im*thisDetector->nDacs;
    else {   
      for (int i=0; i<thisDetector->nDacs; i++)
	das[i]=-1; 
      myModule.dacs=das;
    }
    if (adcs)
      myModule.adcs=adcs+im*thisDetector->nAdcs;
    else {   
      for (int i=0; i<thisDetector->nAdcs; i++)
	ads[i]=-1;
      myModule.adcs=ads; 
    }
    ret=setModule(myModule);
  }
  return ret;
  

};



int slsDetector::setModule(sls_detector_module module){

  int fnum=F_SET_MODULE;
  int retval;
  int ret=FAIL;
  char mess[100];

  int imod=module.module;


#ifdef VERBOSE
    std::cout << "slsDetector set module " << std::endl;
#endif 


  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendModule(&module);
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
    }
    }
  }

  
  if (ret!=FAIL) {
    if (detectorModules) {
      if (imod>=0 && imod<thisDetector->nMod[X]*thisDetector->nMod[Y]) {
	(detectorModules+imod)->nchan=module.nchan;
	(detectorModules+imod)->nchip=module.nchip;
	(detectorModules+imod)->ndac=module.ndac;
	(detectorModules+imod)->nadc=module.nadc;
	thisDetector->nChips=module.nchip;
	thisDetector->nChans=module.nchan/module.nchip;
	thisDetector->nDacs=module.ndac;
	thisDetector->nAdcs=module.nadc;
	
	for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
	  if (chipregs)
	    chipregs[ichip+thisDetector->nChips*imod]=module.chipregs[ichip];
	  
	  if (chanregs) {
	    for (int i=0; i<thisDetector->nChans; i++) {
	      chanregs[i+ichip*thisDetector->nChans+thisDetector->nChips*thisDetector->nChans*imod]=module.chanregs[ichip*thisDetector->nChans+i];
	    }
	  }
	}
	if (dacs) {
	  for (int i=0; i<thisDetector->nDacs; i++)
	    dacs[i+imod*thisDetector->nDacs]=module.dacs[i];
	}
	if (adcs) {
	  for (int i=0; i<thisDetector->nAdcs; i++)
	    adcs[i+imod*thisDetector->nAdcs]=module.adcs[i];
	}
	
	(detectorModules+imod)->gain=module.gain;
	(detectorModules+imod)->offset=module.offset;
	(detectorModules+imod)->serialnumber=module.serialnumber;
	(detectorModules+imod)->reg=module.reg;
      }
    }
  }

#ifdef VERBOSE
  std::cout<< "Module register returned "<<  retval << std::endl;
#endif

  return retval;
};

slsDetectorDefs::sls_detector_module  *slsDetector::getModule(int imod){

#ifdef VERBOSE
  std::cout << "slsDetector get module " << std::endl;
#endif 

  int fnum=F_GET_MODULE;
  sls_detector_module *myMod=createModule();


  //char *ptr,  *goff=(char*)thisDetector;

  // int chanreg[thisDetector->nChans*thisDetector->nChips];
  //int chipreg[thisDetector->nChips];
  //double dac[thisDetector->nDacs], adc[thisDetector->nAdcs];

  int ret=FAIL;
  char mess[100];
  // int n;

#ifdef VERBOSE
  std::cout<< "getting module " << imod << std::endl;
#endif

  myMod->module=imod;
  // myMod.nchan=thisDetector->nChans*thisDetector->nChips;
  //myMod.chanregs=chanreg;
  //myMod.nchip=thisDetector->nChips;
  //myMod.chipregs=chipreg;
  //myMod.ndac=thisDetector->nDacs;
  //myMod.dacs=dac;
  //myMod.ndac=thisDetector->nAdcs;
  //myMod.dacs=adc;

 




  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
    if  (controlSocket->Connect()>=0) {
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&imod,sizeof(imod));
   
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	receiveModule(myMod);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
    }
    }
  }
  

  if (ret!=FAIL) {
    if (detectorModules) {
      if (imod>=0 && imod<thisDetector->nMod[X]*thisDetector->nMod[Y]) {
	(detectorModules+imod)->nchan=myMod->nchan;
	(detectorModules+imod)->nchip=myMod->nchip;
	(detectorModules+imod)->ndac=myMod->ndac;
	(detectorModules+imod)->nadc=myMod->nadc;
	thisDetector->nChips=myMod->nchip;
	thisDetector->nChans=myMod->nchan/myMod->nchip;
	thisDetector->nDacs=myMod->ndac;
	thisDetector->nAdcs=myMod->nadc;
	
	for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
	  if (chipregs)
	    chipregs[ichip+thisDetector->nChips*imod]=myMod->chipregs[ichip];
	  
	  if (chanregs) {
	    for (int i=0; i<thisDetector->nChans; i++) {
	      chanregs[i+ichip*thisDetector->nChans+thisDetector->nChips*thisDetector->nChans*imod]=myMod->chanregs[ichip*thisDetector->nChans+i];
	    }
	  }
	}
	if (dacs) {
	  for (int i=0; i<thisDetector->nDacs; i++)
	    dacs[i+imod*thisDetector->nDacs]=myMod->dacs[i];
	}
	if (adcs) {
	  for (int i=0; i<thisDetector->nAdcs; i++)
	    adcs[i+imod*thisDetector->nAdcs]=myMod->adcs[i];
	}
	
	(detectorModules+imod)->gain=myMod->gain;
	(detectorModules+imod)->offset=myMod->offset;
	(detectorModules+imod)->serialnumber=myMod->serialnumber;
	(detectorModules+imod)->reg=myMod->reg;
      }
    }
  } else {
    deleteModule(myMod);
    myMod=NULL;
  }

  return myMod;
}




  // calibration functions
/*
  really needed?

int slsDetector::setCalibration(int imod,  detectorSettings isettings, double gain, double offset){
  std::cout<< "function not yet implemented " << std::endl; 
  
  

  return OK;

}
int slsDetector::getCalibration(int imod,  detectorSettings isettings, double &gain, double &offset){

  std::cout<< "function not yet implemented " << std::endl; 



}
*/

  /*
    calibrated setup of the threshold
  */

int slsDetector::getThresholdEnergy(int imod){

  int fnum=  F_GET_THRESHOLD_ENERGY;
  int retval;
  int ret=FAIL;
  char mess[100];
#ifdef VERBOSE
  std::cout<< "Getting threshold energy "<< std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  std::cout<< "Detector returned error: "<< std::endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<<  mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	  thisDetector->currentThresholdEV=retval;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } 
  return  thisDetector->currentThresholdEV;
};  

int slsDetector::setThresholdEnergy(int e_eV,  int imod, detectorSettings isettings){

  int fnum=  F_SET_THRESHOLD_ENERGY;
  int retval;
  int ret=FAIL;
  char mess[100];
#ifdef VERBOSE
  std::cout<< "Getting threshold energy "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&e_eV,sizeof(e_eV));
	controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->SendDataOnly(&isettings,sizeof(isettings));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  std::cout<< "Detector returned error: "<< std::endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<<  mess << std::endl;
	} else {
#ifdef VERBOSE
	  std::cout<< "Detector returned OK "<< std::endl;
#endif
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	  thisDetector->currentThresholdEV=retval;   
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    thisDetector->currentThresholdEV=e_eV;
  }
  return   thisDetector->currentThresholdEV;
};  

  /*
    select detector settings
  */
 slsDetectorDefs::detectorSettings  slsDetector::getSettings(int imod){

   
  int fnum=F_SET_SETTINGS;
  int ret=FAIL;
  char mess[100];
  int  retval;
  int arg[2];
  arg[0]=GET_SETTINGS;
  arg[1]=imod;
#ifdef VERBOSE
  std::cout<< "Getting settings "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else{
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  thisDetector->currentSettings=(detectorSettings)retval;
#ifdef VERBOSE
	  std::cout<< "Settings are "<< retval << std::endl;
#endif
    }
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  }
  return thisDetector->currentSettings;

};



slsDetectorDefs::detectorSettings slsDetector::setSettings( detectorSettings isettings, int imod){
#ifdef VERBOSE
  std::cout<< "slsDetector setSettings "<< std::endl;
#endif
  sls_detector_module *myMod=createModule();
  int modmi=imod, modma=imod+1, im=imod;
  string settingsfname, calfname;
  string ssettings;
  detectorSettings minsettings, maxsettings;

  switch(thisDetector->myDetectorType){
  case GOTTHARD:
    minsettings = HIGHGAIN;
    maxsettings = VERYHIGHGAIN;
    break;
  default:
    minsettings = STANDARD;
    maxsettings = HIGHGAIN;
  }
    
  if (isettings>=minsettings && isettings<=maxsettings) {
    switch (isettings) {
    case STANDARD:
      ssettings="/standard";
      thisDetector->currentSettings=STANDARD;
      break;
    case FAST:
      ssettings="/fast";
      thisDetector->currentSettings=FAST;
      break;
    case HIGHGAIN:
      ssettings="/highgain";
      thisDetector->currentSettings=HIGHGAIN;
      break;
    case DYNAMICGAIN:
      ssettings="/dynamicgain";
      thisDetector->currentSettings=DYNAMICGAIN;
      break;
    case LOWGAIN:
      ssettings="/lowgain";
      thisDetector->currentSettings=LOWGAIN;
      break;
    case MEDIUMGAIN:
      ssettings="/mediumgain";
      thisDetector->currentSettings=MEDIUMGAIN;
      break;
    case VERYHIGHGAIN:
      ssettings="/veryhighgain";
      thisDetector->currentSettings=VERYHIGHGAIN;
      break;
    default:
      std::cout<< "Unknown settings!" << std::endl;
    }
       
    if (imod<0) {
      modmi=0;
      //  modma=thisDetector->nModMax[X]*thisDetector->nModMax[Y];
      modma=thisDetector->nMod[X]*thisDetector->nMod[Y];
    }

    for (im=modmi; im<modma; im++) {
      ostringstream ostfn, oscfn;
      myMod->module=im;
      //create file names
      switch(thisDetector->myDetectorType){
      case GOTTHARD:
	//settings is saved in myMod.reg for gotthard
	myMod->reg=thisDetector->currentSettings;
	ostfn << thisDetector->settingsDir << ssettings <<"/settings.sn";//  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
	oscfn << thisDetector->calDir << ssettings << "/calibration.sn";//  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
#ifdef VERBOSE
	std::cout<< thisDetector->settingsDir<<endl<< thisDetector->calDir <<endl;
#endif
	break;
      default:
	ostfn << thisDetector->settingsDir << ssettings <<"/noise.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
	oscfn << thisDetector->calDir << ssettings << "/calibration.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      }
      settingsfname=ostfn.str();
#ifdef VERBOSE
      cout << "the settings name is "<<settingsfname << endl;
#endif
      if (readSettingsFile(settingsfname,thisDetector->myDetectorType, myMod)) {
	calfname=oscfn.str();
#ifdef VERBOSE
	cout << calfname << endl;
#endif
	readCalibrationFile(calfname,myMod->gain, myMod->offset);
	setModule(*myMod);
      } else {
	ostringstream ostfn,oscfn;
	switch(thisDetector->myDetectorType){
	case GOTTHARD:
	  ostfn << thisDetector->settingsDir << ssettings << ssettings << ".settings";
	  break;
	default:
	  ostfn << thisDetector->settingsDir << ssettings << ssettings << ".trim"; 
	}
	oscfn << thisDetector->calDir << ssettings << ssettings << ".cal";
	calfname=oscfn.str();
	settingsfname=ostfn.str();
#ifdef VERBOSE
	cout << settingsfname << endl;
	cout << calfname << endl;
#endif
	if (readSettingsFile(settingsfname,thisDetector->myDetectorType, myMod)) {
	  calfname=oscfn.str();
	  readCalibrationFile(calfname,myMod->gain, myMod->offset);
	  setModule(*myMod);
	}
      }
    }
  }
  deleteModule(myMod);
  switch(thisDetector->myDetectorType==MYTHEN){
    if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
      int isett=getSettings(imod);
      double t[]=defaultTDead;
      if (isett>-1 && isett<3) {
	thisDetector->tDead=t[isett];
      }
    }
  }
  return  getSettings(imod);
};


int slsDetector::updateDetectorNoWait() {

	// int ret=OK;
	enum detectorSettings t;
	int thr, n, nm;
	// int it;
	int64_t retval;// tns=-1;
	char lastClientIP[INET_ADDRSTRLEN];

	n = 	controlSocket->ReceiveDataOnly(lastClientIP,sizeof(lastClientIP));
#ifdef VERBOSE
	cout << "Updating detector last modified by " << lastClientIP << std::endl;
#endif
	n = 	controlSocket->ReceiveDataOnly(&nm,sizeof(nm));
	thisDetector->nMod[X]=nm;
	n = 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
	thisDetector->nMod[Y]=nm;
	thisDetector->nMods=thisDetector->nMod[Y]*thisDetector->nMod[X];

	n = 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
	thisDetector->dynamicRange=nm;

	n = 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
	thisDetector->dataBytes=nm;
	//t=setSettings(GET_SETTINGS);
	n = 	controlSocket->ReceiveDataOnly( &t,sizeof(t));
	thisDetector->currentSettings=t;

	if(thisDetector->myDetectorType!= GOTTHARD){
		//thr=getThresholdEnergy();
		n = 	controlSocket->ReceiveDataOnly( &thr,sizeof(thr));
		thisDetector->currentThresholdEV=thr;
	}

	//retval=setFrames(tns);
	n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[FRAME_NUMBER]=retval;
	// retval=setExposureTime(tns);
	n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[ACQUISITION_TIME]=retval;

	//retval=setPeriod(tns);
	n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[FRAME_PERIOD]=retval;
	//retval=setDelay(tns);
	n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[DELAY_AFTER_TRIGGER]=retval;
	// retval=setGates(tns);
	n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[GATES_NUMBER]=retval;

	//retval=setProbes(tns);
	if(thisDetector->myDetectorType!= GOTTHARD){
		n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		thisDetector->timerValue[PROBES_NUMBER]=retval;
	}

	//retval=setTrains(tns);
	n = 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[CYCLES_NUMBER]=retval;

	return OK;

}




int slsDetector::updateDetector() {
  int fnum=F_UPDATE_CLIENT;
  int ret=OK;
  char mess[100];

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else
	  updateDetectorNoWait();
	controlSocket->Disconnect();
      }
    }
  }
  return ret;
}



// Acquisition functions
/* change these funcs accepting also ok/fail */

int slsDetector::startAcquisition(){


  int fnum=F_START_ACQUISITION;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< "Starting acquisition "<< std::endl;
#endif
  thisDetector->stoppedFlag=0;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  }
  return ret;



};
int slsDetector::stopAcquisition(){


  int fnum=F_STOP_ACQUISITION;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< "Stopping acquisition "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
  if  (stopSocket->Connect()>=0) {
    stopSocket->SendDataOnly(&fnum,sizeof(fnum));
    stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret==FAIL) {
      stopSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    }
    stopSocket->Disconnect();
  }
    }
  }
  thisDetector->stoppedFlag=1;
  return ret;


};

int slsDetector::startReadOut(){

  int fnum=F_START_READOUT;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  std::cout<< "Starting readout "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret==FAIL) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    }
    controlSocket->Disconnect();
    if (ret==FORCE_UPDATE)
      updateDetector();
  }
    }
  }
  return ret;
};


slsDetectorDefs::runStatus slsDetector::getRunStatus(){
  int fnum=F_GET_RUN_STATUS;
  int ret=FAIL;
  char mess[100];
  runStatus retval=ERROR;
#ifdef VERBOSE
  std::cout<< "Getting status "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
      if  (stopSocket->Connect()>=0) {
	stopSocket->SendDataOnly(&fnum,sizeof(fnum));
	stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
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


int* slsDetector::readFrame(){

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




int* slsDetector::getDataFromDetector(int *retval){
	int nel=thisDetector->dataBytes/sizeof(int);
	int n;
	
	int *r=retval;


	//	int* retval=new int[nel];

	if (retval==NULL)
	  retval=new int[nel];

	int ret=FAIL;
	char mess[100]="Nothing";

#ifdef VERBOSE
	std::cout<< "getting data "<< thisDetector->dataBytes << " " << nel<< std::endl;
#endif
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
#ifdef VERBOSE
	cout << "ret=" << ret << endl;
#endif

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
	  if (r==NULL) {
	    delete [] retval;
	  }
	  return NULL;
	} else {
	  n=controlSocket->ReceiveDataOnly(retval,thisDetector->dataBytes);
	  
#ifdef VERBOSE
	  std::cout<< "Received "<< n << " data bytes" << std::endl;
#endif
	  if (n!=thisDetector->dataBytes) {
	    std::cout<< "wrong data size received: received " << n << " but expected " << thisDetector->dataBytes << std::endl;
	    thisDetector->stoppedFlag=1;
	    ret=FAIL;
	    if (r==NULL) {
	      delete [] retval;
	    }
	    return NULL;
	  }
	}

	return retval;

};






int* slsDetector::readAll(){
  
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
      std::cout<< i << std::endl;
      //#else
      //std::cout << "-" << flush ;
#endif
      dataQueue.push(retval);
    }
    controlSocket->Disconnect();
  }
    }
  }
#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
  //#else
  // std::cout << std::endl;
#endif
  return dataQueue.front(); // check what we return!

};





int slsDetector::readAllNoWait(){

  int fnum= F_READ_ALL;
  
#ifdef VERBOSE
  std::cout<< "Reading all frames "<< std::endl;
#endif
  
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








int* slsDetector::startAndReadAll(){

 
  int* retval;
#ifdef VERBOSE
  int i=0;
#endif
  startAndReadAllNoWait();  
  //#ifdef VERBOSE
  // std::cout<< "started" << std::endl;
//#endif
  while ((retval=getDataFromDetector())){
#ifdef VERBOSE
      i++;
      std::cout<< i << std::endl;
      //#else
      //std::cout<< "-" << flush;
#endif
      dataQueue.push(retval);
  }
  controlSocket->Disconnect();

#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
  //#else
  // std::cout << std::endl; 
#endif
  return dataQueue.front(); // check what we return!
/* while ((retval=getDataFromDetectorNoWait()))
   i++;
   #ifdef VERBOSE
  std::cout<< "Received " << i << " frames"<< std::endl;
#endif
  return dataQueue.front(); // check what we return!
  */
  
};



int slsDetector::startAndReadAllNoWait(){

  int fnum= F_START_AND_READ_ALL;
  
#ifdef VERBOSE
  std::cout<< "Starting and reading all frames "<< std::endl;
#endif
  thisDetector->stoppedFlag=0;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	//std::cout<< "connected" << std::endl;
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	return OK;
      }
    }
  }
  return FAIL;
};

// int* slsDetector::getDataFromDetectorNoWait() {
//   int *retval=getDataFromDetector();
//   if (thisDetector->onlineFlag==ONLINE_FLAG) {
//     if (controlSocket) {
//       if (retval==NULL){
// 	controlSocket->Disconnect();

// #ifdef VERBOSE
//   std::cout<< "Run finished "<< std::endl;
// #endif
//   } else {
// #ifdef VERBOSE
//     std::cout<< "Frame received "<< std::endl;
// #endif
//   }
//     }
//   }
//   return retval; // check what we return!
// };




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
int64_t slsDetector::setTimer(timerIndex index, int64_t t){
  

  int fnum=F_SET_TIMER;
  int64_t retval;
  uint64_t ut;
  char mess[100];
  int ret=OK;
  int n=0;
  
	if (index!=MEASUREMENTS_NUMBER) {
  

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
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	  thisDetector->timerValue[index]=retval; 
	} 
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    //std::cout<< "offline " << std::endl;
    if (t>=0)
      thisDetector->timerValue[index]=t;
  }
} else {
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

int slsDetector::lockServer(int lock) {
  int fnum=F_LOCK_SERVER;
  int retval=-1;
  int ret=OK;
  char mess[100];
  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&lock,sizeof(lock));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	} 
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  }

  return retval;


}

 
string slsDetector::getLastClientIP() {

  int fnum=F_GET_LAST_CLIENT_IP;
  char clientName[INET_ADDRSTRLEN];
  char mess[100];
  int ret=OK;
  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(clientName,sizeof(clientName)); 
	} 
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  }

  return string(clientName);

}


int slsDetector::setPort(portType index, int num){
  

  int fnum=F_SET_PORT;
  int retval;
  //  uint64_t ut;
  char mess[100];
  int ret=FAIL;
  int n=0;
  
  MySocketTCP *s;

  if (num>1024) {

  switch(index) {
  case CONTROL_PORT:
    s=controlSocket;
    retval=thisDetector->controlPort;
#ifdef VERBOSE
    cout << "s="<< s<< endl;
      cout << thisDetector->controlPort<< " " << thisDetector->dataPort << " " << thisDetector->stopPort << endl;
#endif 
    if (s==NULL) {
      
#ifdef VERBOSE
      cout << "s=NULL"<< endl;
      cout << thisDetector->controlPort<< " " << thisDetector->dataPort << " " << thisDetector->stopPort << endl;
#endif 
      setTCPSocket("",DEFAULT_PORTNO);
    }
    if (controlSocket) {
      s=controlSocket;
    } else {
#ifdef VERBOSE
      cout << "still cannot connect!"<< endl;
      cout << thisDetector->controlPort<< " " << thisDetector->dataPort << " " << thisDetector->stopPort << endl;
#endif 
  


      setTCPSocket("",retval);
    }
    break;
  case DATA_PORT:
    s=dataSocket;
    retval=thisDetector->dataPort;
    if (s==NULL) setTCPSocket("",-1,-1,DEFAULT_PORTNO+2);
    if (dataSocket) s=dataSocket;
    else setTCPSocket("",-1,-1,retval);
    break;
  case STOP_PORT:
    s=stopSocket;
    retval=thisDetector->stopPort;
    if (s==NULL) setTCPSocket("",-1,DEFAULT_PORTNO+1);
    if (stopSocket) s=stopSocket;
    else setTCPSocket("",-1,retval);
    break;
  default:
    s=NULL;
  }

    if (thisDetector->onlineFlag==ONLINE_FLAG) {
      if (s) {
	if  (s->Connect()>=0) {
	  s->SendDataOnly(&fnum,sizeof(fnum));
	s->SendDataOnly(&index,sizeof(index));
	n=s->SendDataOnly(&num,sizeof(num));
	s->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  s->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  s->ReceiveDataOnly(&retval,sizeof(retval)); 
	} 
	s->Disconnect();
	}
      } 
      
  }
    if (ret!=FAIL) {
      
      switch(index) {
      case CONTROL_PORT:
	thisDetector->controlPort=retval;
	break;
      case DATA_PORT:
	thisDetector->dataPort=retval;
	break;
      case STOP_PORT:
	thisDetector->stopPort=retval;
      break;
      default:
	;
      }
    
      
#ifdef VERBOSE
      cout << "ret is ok" << endl;
#endif 
      
    } else {
      switch(index) {
      case CONTROL_PORT:
	thisDetector->controlPort=num;
	break;
      case DATA_PORT:
	thisDetector->dataPort=num;
	break;
      case STOP_PORT:
	thisDetector->stopPort=num;
	break;
      default:
	;
      }
    }
  }
    switch(index) {
    case CONTROL_PORT:
      retval=thisDetector->controlPort;
      break;
    case DATA_PORT:
      retval=thisDetector->dataPort;
      break;
    case STOP_PORT:
      retval=thisDetector->stopPort;
    break;
    default:
      retval=-1;
    }
    
    // setTCPSocket();
    
    
#ifdef VERBOSE
    cout << thisDetector->controlPort<< " " << thisDetector->dataPort << " " << thisDetector->stopPort << endl;
#endif 
  


  return retval;
  
};












//Naveen change

int slsDetector::setTotalProgress() {

       int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1, nm=1;
      
      if (thisDetector->timerValue[FRAME_NUMBER])
	nf=thisDetector->timerValue[FRAME_NUMBER];

      if (thisDetector->timerValue[CYCLES_NUMBER]>0)
	nc=thisDetector->timerValue[CYCLES_NUMBER];

      if (thisDetector->numberOfPositions>0)
	npos=thisDetector->numberOfPositions;

      if (timerValue[MEASUREMENTS_NUMBER]>0)
	nc=timerValue[MEASUREMENTS_NUMBER];


      if ((thisDetector->nScanSteps[0]>0) && (thisDetector->actionMask & (1 << MAX_ACTIONS)))
	nscan[0]=thisDetector->nScanSteps[0];

      if ((thisDetector->nScanSteps[1]>0) && (thisDetector->actionMask & (1 << (MAX_ACTIONS+1))))
	nscan[1]=thisDetector->nScanSteps[1];
      
      thisDetector->totalProgress=nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
      cout << "nc " << nc << endl;
      cout << "nm " << nm << endl;
      cout << "nf " << nf << endl;
      cout << "npos " << npos << endl;
      cout << "nscan[0] " << nscan[0] << endl;
      cout << "nscan[1] " << nscan[1] << endl;

      cout << "Set total progress " << thisDetector->totalProgress << endl;
#endif
      return thisDetector->totalProgress;
}


double slsDetector::getCurrentProgress() {

  return 100.*((double)thisDetector->progressIndex)/((double)thisDetector->totalProgress);
}















/*
  important speed parameters

enum speedVariable {
  CLOCK_DIVIDER, 
  WAIT_STATES, 
  SET_SIGNAL_LENGTH 
};
*/

int slsDetector::setSpeed(speedVariable sp, int value) {


  int fnum=F_SET_SPEED;
  int retval=-1;
  char mess[100];
  int ret=OK;
  int n=0;
#ifdef VERBOSE
  std::cout<< "Setting speed  variable"<< sp << " to " <<  value << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&sp,sizeof(sp));
	n=controlSocket->SendDataOnly(&value,sizeof(value));
#ifdef VERBOSE
	std::cout<< "Sent  "<< n << " bytes "  << std::endl;
#endif
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	} 
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  }
#ifdef VERBOSE
  std::cout<< "Speed set to  "<< retval  << std::endl;
#endif
  return retval;
  
}
















int64_t slsDetector::getTimeLeft(timerIndex index){
  

  int fnum=F_GET_TIME_LEFT;
  int64_t retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Getting  timer  "<< index <<  std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
      if  (stopSocket->Connect()>=0) {
	stopSocket->SendDataOnly(&fnum,sizeof(fnum));
	stopSocket->SendDataOnly(&index,sizeof(index));
	stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  stopSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  stopSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
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
int slsDetector::setDynamicRange(int n){

  // cout << "single "  << endl;
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
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
    } else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } else {
    if (n>0)
      thisDetector->dynamicRange=n;
    retval=thisDetector->dynamicRange;
  }
    
  if (ret!=FAIL && retval>0) {
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

int slsDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


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

int slsDetector::setReadOutFlags(readOutFlags flag){

  
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
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	  thisDetector->roFlags=retval;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
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
int slsDetector::executeTrimming(trimMode mode, int par1, int par2, int imod){
  
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
#ifdef VERBOSE
    std::cout<< "sending mode bytes= "<<  controlSocket->SendDataOnly(&mode,sizeof(mode)) << std::endl;
#endif
    controlSocket->SendDataOnly(&mode,sizeof(mode));
    controlSocket->SendDataOnly(arg,sizeof(arg));

    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret==FAIL) {
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
    if (ret==FORCE_UPDATE)
      updateDetector();
  }
    }
  }
  return retval;

};

double* slsDetector::decodeData(int *datain, double *fdata) {
  
  double *dataout;
  if (fdata)
    dataout=fdata;
  else
    dataout=new double[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
  
  const int bytesize=8;

  int ival=0;
  char *ptr=(char*)datain;
  char iptr;

  int nbits=thisDetector->dynamicRange;
  int nch=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
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
      for (ichan=0; ichan<nch; ichan++) {
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
      for (ichan=0; ichan<nch; ichan++) {
	ival=datain[ichan]&0xffffff;
	dataout[ichan]=ival;
      }
    }
  } else { 
    for (ichan=0; ichan<nch; ichan++) {
      dataout[ichan]=datain[ichan];
    }
  }


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

int slsDetector::setFlatFieldCorrection(string fname)

{
  double data[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
  //double err[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
   double xmed[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
   int nmed=0;
   int im=0;
   int nch;
   thisDetector->nBadFF=0;

   char ffffname[MAX_STR_LENGTH*2];
   if (fname=="default") {
     fname=string("thisDetector->flatFieldFile");
   }

   if (fname=="") {
#ifdef VERBOSE
     std::cout<< "disabling flat field correction" << std::endl;
#endif
     thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
     //strcpy(thisDetector->flatFieldFile,"none");
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





int slsDetector::setFlatFieldCorrection(double *corr, double *ecorr) {
  if (corr!=NULL) {
    for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nChans*thisDetector->nChips; ichan++) {
// #ifdef VERBOSE
//       std::cout<< ichan << " "<< corr[ichan] << std::endl;
// #endif
      ffcoefficients[ichan]=corr[ichan];
      if (ecorr!=NULL)
	fferrors[ichan]=ecorr[ichan];
      else
	fferrors[ichan]=1;
    }
    thisDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);
  } else
   thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
  
  return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}









 
int slsDetector::getFlatFieldCorrection(double *corr, double *ecorr) {
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


int slsDetector::flatFieldCorrect(double* datain, double *errin, double* dataout, double *errout){
#ifdef VERBOSE
    std::cout<< "Flat field correcting data" << std::endl;
#endif
  double e, eo;
  if (thisDetector->correctionMask & (1<<FLAT_FIELD_CORRECTION)) {
    for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nChans*thisDetector->nChips; ichan++) {
      if (errin==NULL) {
	e=0;
      } else {
	e=errin[ichan];
      }
      flatFieldCorrect(datain[ichan],e,dataout[ichan],eo,ffcoefficients[ichan],fferrors[ichan]);
      if (errout)
	errout[ichan]=eo;
// #ifdef VERBOSE
//       cout << ichan << " " <<datain[ichan]<< " " << ffcoefficients[ichan]<< " " << dataout[ichan] << endl;
// #endif
    }
  }
  return 0;

};

int slsDetector::setRateCorrection(double t){
  double tdead[]=defaultTDead;

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


int slsDetector::getRateCorrection(double &t){

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

double slsDetector::getRateCorrectionTau(){

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







int slsDetector::getRateCorrection(){

  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
    return 1;
  } else
    return 0;
};




int slsDetector::rateCorrect(double* datain, double *errin, double* dataout, double *errout){
  double tau=thisDetector->tDead;
  double t=thisDetector->timerValue[ACQUISITION_TIME];
  // double data;
  double e;
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







int slsDetector::setBadChannelCorrection(string fname){
  

  int nbadmod,ret=0;
  int badchanlist[MAX_BADCHANS];
  int off;

  string fn=fname;
  
  if (fname=="default")
    fname=string(badChanFile);

  if (nBadChans && badChansList)
    ret=setBadChannelCorrection(fname, *nBadChans, badChansList);

  if (ret) {
    *correctionMask|=(1<<DISCARD_BAD_CHANNELS);
    strcpy(badChanFile,fname.c_str());
  } else
    *correctionMask&=~(1<<DISCARD_BAD_CHANNELS);

  fillBadChannelMask();
  return  (*correctionMask)&(1<<DISCARD_BAD_CHANNELS);
}







int slsDetector::setBadChannelCorrection(int nch, int *chs, int ff) {
#ifdef VERBOSE
  cout << "setting " << nch << " bad chans " << endl;
#endif
  if (ff==0) {
    if (nch<MAX_BADCHANS && nch>0) {
      thisDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
      thisDetector->nBadChans=0;
      for (int ich=0 ;ich<nch; ich++) {
	if (chs[ich]<getMaxNumberOfChannels()) {
	  thisDetector->badChansList[ich]=chs[ich];
	  thisDetector->nBadChans++;
	}
      }
    } else
      thisDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
  } else {
    if (nch<MAX_BADCHANS && nch>0) {
      thisDetector->nBadFF=nch;
      for (int ich=0 ;ich<nch; ich++) {
	thisDetector->badFFList[ich]=chs[ich];
      }
    }
  }
#ifdef VERBOSE
  cout << "badchans flag is "<< (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) << endl;
#endif
  fillBadChannelMask();
  if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    return thisDetector->nBadChans+thisDetector->nBadFF;
  } else
   return 0;

}








int slsDetector::getBadChannelCorrection(int *bad) {
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










int slsDetector::exitServer(){  
  
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
  if (retval!=OK) {
    std::cout<< std::endl;
    std::cout<< "Shutting down the server" << std::endl;
    std::cout<< std::endl;
  }
  return retval;

};


char* slsDetector::setNetworkParameter(networkParameter index, string value) {
  
  switch (index) {
  case CLIENT_IP:
    return setClientIP(value);
    break;
  case CLIENT_MAC:
    return setClientMAC(value);
    break;
  case SERVER_MAC:
    return setServerMAC(value);
    break;
  default:
    return ("unknown network parameter");
  }

}

char* slsDetector::getNetworkParameter(networkParameter index) {
  
  switch (index) {
  case CLIENT_IP:
    return getClientIP();
    break;
  case CLIENT_MAC:
    return getClientMAC();
    break;
  case SERVER_MAC:
    return getServerMAC();
    break;
  default:
    return ("unknown network parameter");
  }

}






char* slsDetector::setClientIP(string clientIP){
  int wrongFormat=1;

  struct sockaddr_in sa;
  if(clientIP.length()<16){
    if((clientIP[3]=='.')&&(clientIP[7]=='.')&&(clientIP[11]=='.')){
      int result = inet_pton(AF_INET, clientIP.c_str(), &(sa.sin_addr));
      if(result!=0){
	sprintf(thisDetector->clientIP,clientIP.c_str()); 
	wrongFormat=0;
      }
    }
  }
    if(!wrongFormat)
      return thisDetector->clientIP;
    else 
      return ("IP Address should be VALID and in xxx.xxx.xxx.xxx format");
}


char* slsDetector::setClientMAC(string clientMAC){
  if(clientMAC.length()==17){
    if((clientMAC[2]==':')&&(clientMAC[5]==':')&&(clientMAC[8]==':')&&
       (clientMAC[11]==':')&&(clientMAC[14]==':'))
      sprintf(thisDetector->clientMAC,clientMAC.c_str()); 
    else
      return("MAC Address should be in xx:xx:xx:xx:xx:xx format");  
  }
  else
    return("MAC Address should be in xx:xx:xx:xx:xx:xx format");  

  return thisDetector->clientMAC;
};


char* slsDetector::setServerMAC(string serverMAC){
  if(serverMAC.length()==17){
    if((serverMAC[2]==':')&&(serverMAC[5]==':')&&(serverMAC[8]==':')&&
       (serverMAC[11]==':')&&(serverMAC[14]==':'))
      sprintf(thisDetector->serverMAC,serverMAC.c_str()); 
    else
      return("server MAC Address should be in xx:xx:xx:xx:xx:xx format");  
  }
  else
    return("server MAC Address should be in xx:xx:xx:xx:xx:xx format");  

  return thisDetector->serverMAC;
};


int slsDetector::configureMAC(){
  int retval,i;
  int ret=FAIL;
  int fnum=F_CONFIGURE_MAC;
  char mess[100];
  char arg[3][50];
  char cword[50]="", *pcword;
  string sword;
  strcpy(arg[0],getClientIP());
  strcpy(arg[1],getClientMAC());
  strcpy(arg[2],getServerMAC());


#ifdef VERBOSE
  std::cout<< "slsDetector configureMAC "<< std::endl;
#endif

  for(i=0;i<3;i++){
    if(!strcmp(arg[i],"none"))
      return -1;
  }

#ifdef VERBOSE
  std::cout<< "IP/MAC Addresses in valid format "<< std::endl;
#endif

  //converting IPaddress to hex. 
  pcword = strtok (arg[0],".");
  while (pcword != NULL) {
    sprintf(arg[0],"%02x",atoi(pcword));
    strcat(cword,arg[0]);
    pcword = strtok (NULL, ".");
  }
  strcpy(arg[0],cword);
  std::cout<<"arg0:"<<arg[0]<<"."<<std::endl;

  //converting MACaddress to hex.
  sword.assign(arg[1]);
  strcpy(arg[1],"");
  stringstream sstr(sword);
  while(getline(sstr,sword,':'))
    strcat(arg[1],sword.c_str());
  std::cout<<"arg1:"<<arg[1]<<"."<<std::endl; 

  //converting server MACaddress to hex.
  sword.assign(arg[2]);
  strcpy(arg[2],"");
  stringstream ssstr(sword);
  while(getline(ssstr,sword,':'))
    strcat(arg[2],sword.c_str());
  std::cout<<"arg2:"<<arg[2]<<"."<<std::endl; 

  //send to server
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=FAIL)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }	
    }
  }
#ifdef VERBOSE
  std::cout<< "Configuring MAC - returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Configuring MAC failed " << std::endl;
  }
  return retval;
}



//Corrections







int slsDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {
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



 int slsDetector::readAngularConversionFile(string fname) {

   return readAngularConversion(fname,thisDetector->nModsMax,  thisDetector->angOff);

 }

 int slsDetector::readAngularConversion(ifstream& ifs) {

   return readAngularConversion(ifs,thisDetector->nModsMax,  thisDetector->angOff);

 }












// double* slsDetector::convertAngles(double pos) {
//   int imod;
//   double    *ang=new double[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
//   for (int ip=0; ip<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ip++) {
//     imod=ip/(thisDetector->nChans*thisDetector->nChips);
//     ang[ip]=angle(ip%(thisDetector->nChans*thisDetector->nChips),\
// 		  pos,							\
// 		  thisDetector->fineOffset+thisDetector->globalOffset,	\
// 		  thisDetector->angOff[imod].r_conversion,		\
// 		  thisDetector->angOff[imod].center,			\
// 		  thisDetector->angOff[imod].offset,			\
// 		  thisDetector->angOff[imod].tilt,			\
// 		  thisDetector->angDirection
// 		  );
//   }
//   return ang;
// }





int slsDetector:: writeAngularConversion(string fname) {

  return writeAngularConversion(fname, thisDetector->nMods, thisDetector->angOff);

}


int slsDetector:: writeAngularConversion(ofstream &ofs) {

  return writeAngularConversion(ofs, thisDetector->nMods, thisDetector->angOff);

}





int slsDetector::loadImageToDetector(imageType index,string const fname){

	int ret=FAIL;
	short int arg[thisDetector->nChans*thisDetector->nChips];

#ifdef VERBOSE
	std::cout<< std::endl<< "Loading ";
	if(!index)
		std::cout<<"Dark";
	else
		std::cout<<"Gain";
	std::cout<<" image from file " << fname << std::endl;
#endif

	if(readDataFile(fname,arg)){
		ret = sendImageToDetector(index,arg);
		return ret;
	}
	std::cout<< "Could not open file "<< fname << std::endl;
	return ret;
}


int slsDetector::sendImageToDetector(imageType index,short int imageVals[]){

	int ret=FAIL;
	int retval;
	int fnum=F_LOAD_IMAGE;
	char mess[100];

#ifdef VERBOSE
	std::cout<<"Sending image to detector " <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (controlSocket) {
			if  (controlSocket->Connect()>=0) {
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&index,sizeof(index));
				controlSocket->SendDataOnly(imageVals,thisDetector->dataBytes);
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL)
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				else {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				controlSocket->Disconnect();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}

		}
	}

	return ret;
}
int slsDetector::getCounterBlock(short int arg[],int startACQ){

	int ret=FAIL;
	int fnum=F_READ_COUNTER_BLOCK;
	char mess[100];

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (controlSocket) {
			if  (controlSocket->Connect()>=0) {
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&startACQ,sizeof(startACQ));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL)
					controlSocket->ReceiveDataOnly(arg,thisDetector->dataBytes);
				else {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				controlSocket->Disconnect();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}
	}

	return ret;
}


int slsDetector::writeCounterBlockFile(string const fname,int startACQ){

	int ret=FAIL;
	short int counterVals[thisDetector->nChans*thisDetector->nChips];

#ifdef VERBOSE
		std::cout<< std::endl<< "Reading Counter to \""<<fname;
		if(startACQ==1)
			std::cout<<"\" and Restarting Acquisition";
		std::cout<<std::endl;
#endif

	ret=getCounterBlock(counterVals,startACQ);
	if(ret==OK)
		ret=writeDataFile(fname,counterVals);
	return ret;
}



int slsDetector::resetCounterBlock(int startACQ){

	int ret=FAIL;
	int fnum=F_RESET_COUNTER_BLOCK;
	char mess[100];

#ifdef VERBOSE
	std::cout<< std::endl<< "Resetting Counter";
	if(startACQ==1)
		std::cout<<" and Restarting Acquisition";
	std::cout<<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (controlSocket) {
			if  (controlSocket->Connect()>=0) {
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&startACQ,sizeof(startACQ));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL){
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				controlSocket->Disconnect();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}
	}

	return ret;
}











int slsDetector::readConfigurationFile(string const fname){
  


    string ans;
    string str;
    ifstream infile;
    //    int iargval;
    // int interrupt=0;
    char *args[1000];
//     for (int ia=0; ia<100; ia++) {
//       args[ia]=new char[1000];
//     }
    
    string sargname, sargval;
    int iline=0;
#ifdef VERBOSE
    std::cout<< "config file name "<< fname << std::endl;
#endif
    infile.open(fname.c_str(), ios_base::in);
    if (infile.is_open()) {
      iline=readConfigurationFile(infile);
      infile.close();
    } else {
      std::cout<< "Error opening configuration file " << fname << " for reading" << std::endl;
      return FAIL;
    }
#ifdef VERBOSE
    std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif
    return iline;

}


int slsDetector::readConfigurationFile(ifstream &infile){
  



  slsDetectorCommand *cmd=new slsDetectorCommand(this);

    string ans;
    string str;
    int iargval;
    int interrupt=0;
    char *args[100];
    char myargs[1000][1000];
    
    string sargname, sargval;
    int iline=0;
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
	    strcpy(myargs[iargval],sargname.c_str());
	    args[iargval]=myargs[iargval];
	    iargval++;
	    //}
	}
	  ans=cmd->executeLine(iargval,args,PUT_ACTION);
#ifdef VERBOSE 
	  std::cout<< ans << std::endl;
#endif
	}
	iline++;
      }
      delete cmd;
    return iline;

}










int slsDetector::writeConfigurationFile(string const fname){
  

  ofstream outfile;
  int ret;
  
  outfile.open(fname.c_str(),ios_base::out);
  if (outfile.is_open()) {
    ret=writeConfigurationFile(outfile);
    outfile.close();
  }
  else {
    std::cout<< "Error opening configuration file " << fname << " for writing" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "wrote " <<ret << " lines to configuration file " << std::endl;
#endif
  return ret;


}


int slsDetector::writeConfigurationFile(ofstream &outfile, int id){
  
  slsDetectorCommand *cmd=new slsDetectorCommand(this);
  int nvar;
  int nsig=-1;

  string names[]={				\
    "hostname",					\
    "port",					\
    "stopport",					\
    "settingsdir",				\
    "outdir",					\
    "angdir",					\
    "moveflag",					\
    "lock",					\
    "caldir",					\
    "ffdir",					\
    "nmod",					\
    "waitstates",				\
    "setlength",				\
    "clkdivider",				\
    "extsig"					  };

  // to be added in the future
  //    "trimen",
  //   "dataport",						
	
  switch (thisDetector->myDetectorType) {
  case GOTTHARD:
    names[9]="clientip";
    names[10]="clientmac";
    names[11]="servermac";
    nvar=12;
    break;
  case MYTHEN:
    nsig=4;
  default:
    nvar=15;
    
  }

  int iv=0;
  char *args[100];
  char myargs[100][1000];
  
  for (int ia=0; ia<100; ia++) {
    //args[ia]=new char[1000];
    
    args[ia]=myargs[ia];
  }
  
  
  for (iv=0; iv<nvar; iv++) {
     cout << iv << " " << names[iv] << endl;
     if (names[iv]=="extsig") {
       for (int is=0; is<nsig; is++) {
	 sprintf(args[0],"%s:%d",names[iv].c_str(),is);

	 if (id>=0)
	   outfile << id << ":";
	 
	 outfile << args[0] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
       }
     } else {
       strcpy(args[0],names[iv].c_str());
       if (id>=0)
	 outfile << id << ":";
       outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
     }
  }
  delete cmd;
  return iv;
}
































































int slsDetector::writeSettingsFile(string fname, int imod){

  return writeSettingsFile(fname,thisDetector->myDetectorType, detectorModules[imod]);

};




int slsDetector::loadSettingsFile(string fname, int imod) {

  sls_detector_module  *myMod=NULL;
  string fn=fname;
  fn=fname;
  int mmin=0, mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    if (fname.find(".sn")==string::npos && fname.find(".trim")) {
      ostringstream ostfn;
      ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
      fn=ostfn.str();
    }
    myMod=readSettingsFile(fn, thisDetector->myDetectorType);
    if (myMod) {
      myMod->module=im;
      setModule(*myMod);
      deleteModule(myMod);
    } else	
      return FAIL;
  }
  return OK;
}


int slsDetector::saveSettingsFile(string fname, int imod) {


  sls_detector_module  *myMod=NULL;
  int ret=FAIL;

  int mmin=0,  mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    ostringstream ostfn;
    ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
    if ((myMod=getModule(im))) {
      ret=writeSettingsFile(ostfn.str(), thisDetector->myDetectorType, *myMod);
      deleteModule(myMod);
    }
  }
  return ret;
}





int slsDetector::loadCalibrationFile(string fname, int imod) {

  sls_detector_module  *myMod=NULL;
  string fn=fname;
  fn=fname;
  int mmin=0, mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    if (fname.find(".sn")==string::npos && fname.find(".cal")) {
      ostringstream ostfn;
      ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im);
      fn=ostfn.str();
    }
    if(myMod=getModule(im)){
      if(readCalibrationFile(fn, myMod->gain, myMod->offset)==FAIL)
    	  return FAIL;
      setModule(*myMod);
      deleteModule(myMod);
    } else
      return FAIL;
  }
  return OK;
}


int slsDetector::saveCalibrationFile(string fname, int imod) {


  sls_detector_module  *myMod=NULL;
  int ret=FAIL;

  int mmin=0,  mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    ostringstream ostfn;
    ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im);
    if ((myMod=getModule(im))) {
      ret=writeCalibrationFile(ostfn.str(), myMod->gain, myMod->offset);
      deleteModule(myMod);
    }else
     return FAIL;
  }
  return ret;
}




  /* returns if the detector is Master, slave or nothing 
      \param flag can be GET_MASTER, NO_MASTER, IS_MASTER, IS_SLAVE
      \returns master flag of the detector
  */
slsDetectorDefs::masterFlags  slsDetector::setMaster(masterFlags flag) {
  

  int fnum=F_SET_MASTER;
  masterFlags retval=GET_MASTER;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting master flags to "<< flag << std::endl;
#endif
  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&flag,sizeof(flag));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } 

#ifdef VERBOSE
  std::cout<< "Master flag set to  "<< retval   << std::endl;
#endif
  return retval;
}




  /*
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode can be GET_SYNCHRONIZATION_MODE, NO_SYNCHRONIZATION, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns current syncronization mode   
  */   
slsDetectorDefs::synchronizationMode slsDetector::setSynchronization(synchronizationMode flag) {
  

  
  int fnum=F_SET_SYNCHRONIZATION_MODE;
  synchronizationMode retval=GET_SYNCHRONIZATION_MODE;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting synchronization mode to "<< flag << std::endl;
#endif
  
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&flag,sizeof(flag));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	}
	controlSocket->Disconnect();
	if (ret==FORCE_UPDATE)
	  updateDetector();
      }
    }
  } 

#ifdef VERBOSE
  std::cout<< "Readout flag set to  "<< retval   << std::endl;
#endif
  return retval;

}
