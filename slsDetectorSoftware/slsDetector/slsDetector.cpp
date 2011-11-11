#include "slsDetector.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>


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
   default:
     nch=65535; // one EIGER module
     nm=1; //modules/detector
     nc=8; //chips
     nd=16; //dacs+adcs
   }
   /**
      The size of the shared memory is:
       size of shared structure + ffcoefficents +fferrors + modules+ dacs+adcs+chips+chans 
   */


   sz=sizeof(sharedSlsDetector)+nm*(2*nch*nc*sizeof(float)+sizeof(sls_detector_module)+sizeof(int)*nc+sizeof(float)*nd+sizeof(int)*nch*nc);
#ifdef VERBOSE
   std::cout<<"Size of shared memory is "<< sz << std::endl;
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





















slsDetector::slsDetector(detectorType type, int id):
  thisDetector(NULL),  
  detId(0),
  shmId(-1), 
  controlSocket(NULL),
  stopSocket(NULL),
  dataSocket(NULL),
  currentPosition(0),
  currentPositionIndex(0),
  currentI0(0),
  mergingBins(NULL),
  mergingCounts(NULL),
  mergingErrors(NULL),
  mergingMultiplicity(NULL),
  ffcoefficients(NULL),
  fferrors(NULL),
  detectorModules(NULL),
  dacs(NULL),
  adcs(NULL),
  chipregs(NULL),
  chanregs(NULL),
  badChannelMask(NULL)
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
     std::cout<< "Detector id is " << id << std::endl;
#endif
     detId=id;


     /**Initializes the detector stucture \sa initializeDetectorSize
      */
     initializeDetectorSize(type);




     pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER; 
    
     mp=mp1;

     pthread_mutex_init(&mp, NULL);

}


slsDetector::~slsDetector(){};

slsDetector::slsDetector(char *name, int id, int cport) :
  thisDetector(NULL),  
  detId(0),
  shmId(-1), 
  controlSocket(NULL),
  stopSocket(NULL),
  dataSocket(NULL),
  currentPosition(0),
  currentPositionIndex(0),
  currentI0(0),
  mergingBins(NULL),
  mergingCounts(NULL),
  mergingErrors(NULL),
  mergingMultiplicity(NULL),
  ffcoefficients(NULL),
  fferrors(NULL),
  detectorModules(NULL),
  dacs(NULL),
  adcs(NULL),
  chipregs(NULL),
  chanregs(NULL),
  badChannelMask(NULL) 
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




  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER; 
  
  mp=mp1;
  
  pthread_mutex_init(&mp, NULL);
  
  setTCPSocket(name, cport);

}

detectorType slsDetector::getDetectorType(char *name, int cport) {
  
  int retval=FAIL;
  detectorType t=GENERIC;
  int fnum=F_GET_DETECTOR_TYPE;
  MySocketTCP *s= new MySocketTCP(name, cport);
  char m[1000];
  
 if (s->Connect()>=0) {
    s->SendDataOnly(&fnum,sizeof(fnum));
    s->ReceiveDataOnly(&retval,sizeof(retval));
    
    if (retval==OK)
      s->ReceiveDataOnly(&t,sizeof(t));
    else {
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

detectorType slsDetector::getDetectorType(int id) {
  
  detectorType t=GENERIC;


   key_t     mem_key=DEFAULT_SHM_KEY+id;
   int       shm_id;
   int sz;

   sz=sizeof(sharedSlsDetector);

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
    //printf("Shared memory %d detached\n", shmId);
    // remove shared memory
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
      perror("shmctl(IPC_RMID) failed\n");
      return t;
    }
    //printf("Shared memory %d deleted\n", shmId);
  }
  return t;


}


int slsDetector::initializeDetectorSize(detectorType type) {
  char  *goff;
  goff=(char*)thisDetector;

  /** if the shared memory has newly be created, initialize the detector variables */
   if (thisDetector->alreadyExisting==0) {
     /** set hostname to default */
     strcpy(thisDetector->hostname,DEFAULT_HOSTNAME);
     /** sets onlineFlag to OFFLINE_FLAG */
     thisDetector->onlineFlag=OFFLINE_FLAG;
     /** set ports to defaults */
     switch(type){
     case GOTTHARD:
     thisDetector->controlPort=DEFAULT_PORTNO_GOTTHARD;
     thisDetector->stopPort=DEFAULT_PORTNO_GOTTHARD+1;
     thisDetector->dataPort=DEFAULT_PORTNO_GOTTHARD+2;
     break;
     default:
     thisDetector->controlPort=DEFAULT_PORTNO;
     thisDetector->stopPort=DEFAULT_PORTNO+1;
     thisDetector->dataPort=DEFAULT_PORTNO+2;
     }
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
       break;
     case PICASSO:
       thisDetector->nChans=128;
       thisDetector->nChips=12;
       thisDetector->nDacs=6;
       thisDetector->nAdcs=0;
       thisDetector->nModMax[X]=24;
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
       thisDetector->dynamicRange=1;
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
     
     thisDetector->actionMask=0;


     for (int ia=0; ia<MAX_ACTIONS; ia++) {
       thisDetector->actionMode[ia]=0;
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
     thisDetector->fferroff=thisDetector->ffoff+sizeof(float)*thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
     thisDetector->modoff= thisDetector->fferroff+sizeof(float)*thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
     thisDetector->dacoff=thisDetector->modoff+sizeof(sls_detector_module)*thisDetector->nModsMax;
     thisDetector->adcoff=thisDetector->dacoff+sizeof(float)*thisDetector->nDacs*thisDetector->nModsMax;
     thisDetector->chipoff=thisDetector->adcoff+sizeof(float)*thisDetector->nAdcs*thisDetector->nModsMax;
     thisDetector->chanoff=thisDetector->chipoff+sizeof(int)*thisDetector->nChips*thisDetector->nModsMax;
     
     
   } 


     /** also in case thisDetector alread existed initialize the pointer for flat field coefficients and errors, module structures, dacs, adcs, chips and channels */ 
   ffcoefficients=(float*)(goff+thisDetector->ffoff);
   fferrors=(float*)(goff+thisDetector->fferroff);
   detectorModules=(sls_detector_module*)(goff+ thisDetector->modoff);
#ifdef VERBOSE
   for (int imod=0; imod< thisDetector->nModsMax; imod++)
     std::cout<< hex << detectorModules+imod << dec <<std::endl;
#endif
   dacs=(float*)(goff+thisDetector->dacoff);
   adcs=(float*)(goff+thisDetector->adcoff);
   chipregs=(int*)(goff+thisDetector->chipoff);
   chanregs=(int*)(goff+thisDetector->chanoff);
   if (thisDetector->alreadyExisting==0) {  
     /** if thisDetector is new, initialize its structures \sa initializeDetectorStructure();   */ 
     initializeDetectorStructure();   
     /** set  thisDetector->alreadyExisting=1  */   
     thisDetector->alreadyExisting=1;
   } 
   /** fill the BadChannelMask \sa  fillBadChannelMask */
   fillBadChannelMask();
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

sls_detector_module*  slsDetector::createModule() {

  sls_detector_module *myMod=(sls_detector_module*)malloc(sizeof(sls_detector_module));
  float *dacs=new float[thisDetector->nDacs];
  float *adcs=new float[thisDetector->nAdcs];
  int *chipregs=new int[thisDetector->nChips];
  int *chanregs=new int[thisDetector->nChips*thisDetector->nChans];
  myMod->ndac=thisDetector->nDacs;
  myMod->nadc=thisDetector->nAdcs;
  myMod->nchip=thisDetector->nChips;
  myMod->nchan=thisDetector->nChips*thisDetector->nChans;
 
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
  ts+=controlSocket->SendDataOnly(myMod->dacs,sizeof(float)*(myMod->ndac));
  ts+=controlSocket->SendDataOnly(myMod->adcs,sizeof(float)*(myMod->nadc));
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

  float *dacptr=myMod->dacs;
  float *adcptr=myMod->adcs;
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
  ts+=controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(float)*(myMod->ndac));
#ifdef VERBOSE
  std::cout<< "received dacs " << myMod->module << " of size "<< ts << std::endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(float)*(myMod->nadc));
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
  if (off!=GET_ONLINE_FLAG)
    thisDetector->onlineFlag=off;
  return thisDetector->onlineFlag;
};




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
      retval=FAIL;
#ifdef VERBOSE
    std::cout<< "offline!" << std::endl;
#endif
    }   else {
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





  /* I/O */

/* generates file name without extension*/

string slsDetector::createFileName() {
  createFileName(thisDetector->filePath, thisDetector->fileName, thisDetector->actionMask, currentScanVariable[0], thisDetector->scanPrecision[0], currentScanVariable[1], thisDetector->scanPrecision[1], currentPositionIndex, thisDetector->numberOfPositions, thisDetector->fileIndex);
  
}

string  slsDetector::createFileName(char *filepath, char *filename, int aMask, float sv0, int prec0, float sv1, int prec1, int pindex, int npos, int findex) {
  ostringstream osfn;
  /*directory name +root file name */
  osfn << filepath << "/" << filename;
  
  // scan level 0
  if ( aMask& (1 << (MAX_ACTIONS)))
    osfn << "_S" << fixed << setprecision(prec0) << sv0;

  //scan level 1
  if (aMask & (1 << (MAX_ACTIONS+1)))
    osfn << "_s" << fixed << setprecision(prec1) << sv1;
  

  //position
  if (pindex>0 && pindex<=npos)
    osfn << "_p" << pindex;

  // file index
  osfn << "_" << findex;


#ifdef VERBOSE
  cout << "created file name " << osfn.str() << endl;
#endif

  return osfn.str();

}












int slsDetector::getFileIndexFromFileName(string fname) {
  int i;
  size_t dot=fname.rfind(".");
  if (dot==string::npos)
    return -1;
  size_t uscore=fname.rfind("_");
  if (uscore==string::npos)
    return -1;

  if (sscanf( fname.substr(uscore+1,dot-uscore-1).c_str(),"%d",&i)) {

  return i;
  } 
  //#ifdef VERBOSE
  cout << "******************************** cannot parse file index" << endl;
  //#endif
  return 0;
}

int slsDetector::getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1) {
  
  int i;
  float f;
  string s;


  index=-1;
  p_index=-1;
  sv0=-1;
  sv1=-1;


  //  size_t dot=fname.rfind(".");
  //if (dot==string::npos)
  //  return -1;
  size_t uscore=fname.rfind("_");
  if (uscore==string::npos)
    return -1;
  s=fname;

  //if (sscanf(s.substr(uscore+1,dot-uscore-1).c_str(),"%d",&i)) {
  if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) {
    index=i;
#ifdef VERBOSE
    cout << "******************************** file index is " << index << endl;
#endif
    //return i;
    s=fname.substr(0,uscore);
  }
#ifdef VERBOSE 
  else
    cout << "******************************** cannot parse file index" << endl;
  
  cout << s << endl;
#endif

  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"p%d",&i)) {
    p_index=i;
#ifdef VERBOSE
    cout << "******************************** position index is " << p_index << endl;
#endif
    s=fname.substr(0,uscore);
  }
#ifdef VERBOSE 
  else 
    cout << "******************************** cannot parse position index" << endl;

  cout << s << endl;


#endif


  
  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"s%f",&f)) {
    sv1=f;
#ifdef VERBOSE
    cout << "******************************** scan variable 1 is " << sv1 << endl;
#endif
    s=fname.substr(0,uscore);
  }
#ifdef VERBOSE
  else 
    cout << "******************************** cannot parse scan varable 1" << endl;

  cout << s << endl;


#endif
  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%f",&f)) {
    sv0=f;
#ifdef VERBOSE
    cout << "******************************** scan variable 0 is " << sv0 << endl;
#endif
  } 
#ifdef VERBOSE
  else 
    cout << "******************************** cannot parse scan varable 0" << endl;

#endif



  return index;
}




























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
	if (retval==OK)
	  controlSocket->ReceiveDataOnly(&retType,sizeof(retType));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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

int slsDetector::setDetectorType(string const type){
  detectorType dtype=GENERIC;
  if (type=="Mythen")
    dtype=MYTHEN;
  else if  (type=="Pilatus")
      dtype=PILATUS;
  else if  (type=="Eiger")
    dtype=EIGER;
  else if  (type=="Gotthard")
    dtype=GOTTHARD;
  else if  (type=="Agipd")
    dtype=AGIPD;
  return setDetectorType(dtype);
};

string slsDetector::getDetectorType(){

  switch (thisDetector->myDetectorType) {
  case MYTHEN:
    return string("Mythen");
    break;
  case PILATUS:
    return string("Pilatus");
    break;
  case EIGER:
    return string("Eiger");
    break;
  case GOTTHARD:
    return string("Gotthard");
    break;
  case AGIPD:
    return string("Agipd");
    break;
  default:
    return string("Unknown");
    break;
  }
};




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
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Deterctor returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Deterctor returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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

 externalSignalFlag slsDetector::setExternalSignalFlags(externalSignalFlag pol, int signalindex){



  
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
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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

   externalCommunicationMode slsDetector::setExternalCommunicationMode( externalCommunicationMode pol){



  
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
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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
	  if (ret==OK)
	    controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  else {
	    controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	    std::cout<< "Detector returned error: " << mess << std::endl;
	  }
	  controlSocket->Disconnect();
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
	if (mode==CHIP_TEST)
	   controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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
int slsDetector::giveCalibrationPulse(float vcal, int npulses){
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
  std::cout<< "Writing to register "<< addr <<  " data " << val << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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
  std::cout<< "Reding register "<< addr  << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK)
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	controlSocket->Disconnect();
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
    DETECTOR_BIAS
}{};
  */


float slsDetector::setDAC(float val, dacIndex index, int imod){


  float retval;
  int fnum=F_SET_DAC;
  int ret=FAIL;
  char mess[100];
  int arg[2];
  arg[0]=index;
  arg[1]=imod;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting DAC/POT "<< index << "of module " << imod  <<  " to " << val << std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->SendDataOnly(&val,sizeof(val));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK) {
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
      }
	
    }
  }
#ifdef VERBOSE
  std::cout<< "Dac/Pot set to "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Set dac/pot failed " << std::endl;
  }
  return retval;



};


float slsDetector::getADC(dacIndex index, int imod){

  float retval;
  int fnum=F_GET_ADC;
  int ret=FAIL;
  char mess[100];

  int arg[2];
  arg[0]=index;
  arg[1]=imod;

  

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting ADC "<< index << "of module " << imod  <<   std::endl; 
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==OK) {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  if (adcs) {
	      *(adcs+index+imod*thisDetector->nAdcs)=retval;
	  }
	} else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	  }
	controlSocket->Disconnect();
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
      if (ret==OK) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }


  if (ret==OK) {
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

 sls_detector_channel  slsDetector::getChannel(int ichan, int ichip, int imod){


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
      if (ret==OK) {
	receiveChannel(&myChan);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {
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
      if (ret==OK) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }


  if (ret==OK) {
    if (chipregs)
      *(chipregs+ichi+im*thisDetector->nChips)=retval;
  }

#ifdef VERBOSE
  std::cout<< "Chip register returned "<<  retval << std::endl;
#endif
  return retval;
};


 sls_detector_chip slsDetector::getChip(int ichip, int imod){

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
      if (ret==OK) {
	receiveChip(&myChip);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {
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
  float das[thisDetector->nDacs], ads[thisDetector->nAdcs];
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
      if (ret==OK) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }

  
  if (ret==OK) {
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

sls_detector_module  *slsDetector::getModule(int imod){

#ifdef VERBOSE
  std::cout << "slsDetector get module " << std::endl;
#endif 

  int fnum=F_GET_MODULE;
  sls_detector_module *myMod=createModule();


  //char *ptr,  *goff=(char*)thisDetector;

  // int chanreg[thisDetector->nChans*thisDetector->nChips];
  //int chipreg[thisDetector->nChips];
  //float dac[thisDetector->nDacs], adc[thisDetector->nAdcs];

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
      if (ret==OK) {
	receiveModule(myMod);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {
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

int slsDetector::setCalibration(int imod,  detectorSettings isettings, float gain, float offset){
  std::cout<< "function not yet implemented " << std::endl; 
  
  

  return OK;

}
int slsDetector::getCalibration(int imod,  detectorSettings isettings, float &gain, float &offset){

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
	if (ret!=OK) {
	  std::cout<< "Detector returned error: "<< std::endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<<  mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	  thisDetector->currentThresholdEV=retval;
	}
	controlSocket->Disconnect();
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
	if (ret!=OK) {
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
 detectorSettings  slsDetector::getSettings(int imod){

   
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
	if (ret!=OK) {
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
      }
    }
  }
  return thisDetector->currentSettings;

};



 detectorSettings slsDetector::setSettings( detectorSettings isettings, int imod){
   //#ifdef VERBOSE
  std::cout<< "slsDetector setSettings "<< std::endl;
  //#endif
  sls_detector_module *myMod=createModule();
  int modmi=imod, modma=imod+1, im=imod;
  string settingsfname, calfname;
  string ssettings;
  detectorSettings minsettings, maxsettings;

  switch(thisDetector->myDetectorType){
    case GOTTHARD:
    minsettings = HIGHGAIN;
    maxsettings = GAIN3;
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
    case GAIN1:
      ssettings="/gain1";
      thisDetector->currentSettings=GAIN1;
      break;
    case GAIN2:
      ssettings="/gain2";
      thisDetector->currentSettings=GAIN2;
      break;
    case GAIN3:
      ssettings="/gain3";
      thisDetector->currentSettings=GAIN3;
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
	ostfn << thisDetector->settingsDir << ssettings <<"/settings.sn";//  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
	oscfn << thisDetector->calDir << ssettings << "/calibration.sn";//  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
	std::cout<< thisDetector->settingsDir<<endl<< thisDetector->calDir <<endl;
      break;
      default:
	ostfn << thisDetector->settingsDir << ssettings <<"/noise.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
	oscfn << thisDetector->calDir << ssettings << "/calibration.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      }
      //oscfn << thisDetector->calDir << ssettings << "/calibration.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      
      settingsfname=ostfn.str();
      //#ifdef VERBOSE
      cout << "the settings name is "<<settingsfname << endl;
      //#endif
      if (readSettingsFile(settingsfname,myMod)) {
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
	if (readSettingsFile(settingsfname,myMod)) {
	  calfname=oscfn.str();
	  readCalibrationFile(calfname,myMod->gain, myMod->offset);
	  setModule(*myMod);
	}
      }
    }
  }
  deleteModule(myMod);
  /*
  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
    int isett=getSettings(imod);
    float t[]=defaultTDead;
    if (isett>-1 && isett<3) {
      thisDetector->tDead=t[isett];
      }
  }
  */

  return  getSettings(imod);
};





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
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
    controlSocket->Disconnect();
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
    if (ret!=OK) {
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
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      std::cout<< "Detector returned error: " << mess << std::endl;
    }
    controlSocket->Disconnect();
  }
    }
  }
  return ret;
};


runStatus slsDetector::getRunStatus(){
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

int* slsDetector::getDataFromDetector(){

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
	
#ifdef VERBOSE
      std::cout<< "Received "<< n << " data bytes" << std::endl;
#endif 
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
#else
      std::cout << "-" ;
#endif
      dataQueue.push(retval);
    }
    controlSocket->Disconnect();
  }
    }
  }
#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
#else
   std::cout << std::endl; 
#endif
  return dataQueue.front(); // check what we return!

};

int* slsDetector::startAndReadAll(){

 
  int* retval;
  int i=0;
  startAndReadAllNoWait();  
  while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      std::cout<< i << std::endl;
#else
      std::cout<< "-" ;
#endif
      dataQueue.push(retval);
  }
  controlSocket->Disconnect();

#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
#else
   std::cout << std::endl; 
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
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    return OK;
  }
    }
  }
  return FAIL;
};

int* slsDetector::getDataFromDetectorNoWait() {
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





int* slsDetector::popDataQueue() {
  int *retval=NULL;
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  return retval;
}

detectorData* slsDetector::popFinalDataQueue() {
  detectorData *retval=NULL;
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  return retval;
}

void slsDetector::resetDataQueue() {
  int *retval=NULL;
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
 
}

void slsDetector::resetFinalDataQueue() {
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
int64_t slsDetector::setTimer(timerIndex index, int64_t t){
  

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






int slsDetector::setTotalProgress() {

       int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1;
      
      if (thisDetector->timerValue[FRAME_NUMBER])
	nf=thisDetector->timerValue[FRAME_NUMBER];

      if (thisDetector->timerValue[CYCLES_NUMBER]>0)
	nc=thisDetector->timerValue[CYCLES_NUMBER];

      if (thisDetector->numberOfPositions>0)
	npos=thisDetector->numberOfPositions;

      if ((thisDetector->nScanSteps[0]>0) && (thisDetector->actionMask & (1 << MAX_ACTIONS)))
	nscan[0]=thisDetector->nScanSteps[0];

      if ((thisDetector->nScanSteps[1]>0) && (thisDetector->actionMask & (1 << (MAX_ACTIONS+1))))
	nscan[1]=thisDetector->nScanSteps[1];
      
      thisDetector->totalProgress=nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
      cout << "nc " << nc << endl;
      cout << "nf " << nf << endl;
      cout << "npos " << npos << endl;
      cout << "nscan[0] " << nscan[0] << endl;
      cout << "nscan[1] " << nscan[1] << endl;

      cout << "Set total progress " << thisDetector->totalProgress << endl;
#endif
      return thisDetector->totalProgress;
}


float slsDetector::getCurrentProgress() {

  return 100.*((float)thisDetector->progressIndex)/((float)thisDetector->totalProgress);
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
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	} 
	controlSocket->Disconnect();
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
int slsDetector::setDynamicRange(int n){

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

float* slsDetector::decodeData(int *datain) {
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

int slsDetector::setFlatFieldCorrection(string fname){
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
 
int slsDetector::getFlatFieldCorrection(float *corr, float *ecorr) {
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


int slsDetector::flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr){
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

int slsDetector::flatFieldCorrect(float* datain, float *errin, float* dataout, float *errout){
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

int slsDetector::setRateCorrection(float t){
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


int slsDetector::getRateCorrection(float &t){

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

float slsDetector::getRateCorrectionTau(){

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




 int slsDetector::rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t){

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


int slsDetector::rateCorrect(float* datain, float *errin, float* dataout, float *errout){
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


int slsDetector::setBadChannelCorrection(string fname){
  ifstream infile;
  string str;
  int interrupt=0;
  int ich;
  int chmin,chmax;
#ifdef VERBOSE
  std::cout << "Setting bad channel correction to " << fname << std::endl;
#endif

  if (fname=="") {
    thisDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
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


int slsDetector::fillBadChannelMask() {

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










  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable but leaves script unchanged, "none" disables and overwrites)
      \returns 0 if action disabled, >0 otherwise
  */
int slsDetector::setAction(int iaction, string fname, string par) {

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


int slsDetector::setActionScript(int iaction, string fname) {
#ifdef VERBOSE
  
#endif
  return setAction(iaction,fname,"");
}



int slsDetector::setActionParameter(int iaction, string par) {
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
string slsDetector::getActionScript(int iaction){
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
string slsDetector::getActionParameter(int iaction){
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
int slsDetector::getActionMode(int iaction){
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
int slsDetector::setScan(int iscan, string script, int nvalues, float *values, string par, int precision) {
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

int slsDetector::setScanScript(int iscan, string script) {
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



int slsDetector::setScanParameter(int iscan, string par) {


  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (par!="")
	strcpy(thisDetector->scanParameter[iscan],par.c_str());
      return thisDetector->scanMode[iscan];
  } else
    return -1;

}


int slsDetector::setScanPrecision(int iscan, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (precision>=0)
      thisDetector->scanPrecision[iscan]=precision;
    return thisDetector->scanMode[iscan];
  } else
    return -1;

}

int slsDetector::setScanSteps(int iscan, int nvalues, float *values) {

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
string slsDetector::getScanScript(int iscan){
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
string slsDetector::getScanParameter(int iscan){
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
int slsDetector::getScanMode(int iscan){
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
int slsDetector::getScanSteps(int iscan, float *v) {

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


int slsDetector::getScanPrecision(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    return thisDetector->scanPrecision[iscan];
  } else
    return -1;
}








string slsDetector::executeLine(int narg, char *args[], int action) {


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

  /*
   if (var=="gotthardString") {	
     setTCPSocket();
     if (action==PUT_ACTION) 
       {
	 sval=string(args[1]);
	 std::cout<<"in setting\n";
	  sval=string(args[1]);
	  gotthardStringname(sval);
       }
     std::cout<<"in getting\n";
     strcpy(answer, gotthardStringname(""));
      return string(answer);
   } else 
  */

  if (var=="exitserver") {
     setTCPSocket();
     ival=exitServer();
     if(ival!=OK)
       return string("Server shut down.");
     else return string("Error closing server\n");
   } else if (var=="hostname") { 
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
  
  if (var=="trimdir" || var=="settingsdir") {
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
      return string("syntax is modulenumber:i where i is module number");
    istringstream vvstr(var.substr(13));
    vvstr >> ival; 
    if (vvstr.fail())
      return string("syntax is modulenumber:i where i is module number");
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
     switch(thisDetector->myDetectorType) {

     case MYTHEN:
     case EIGER:       
	if (sval=="standard")
	  sett=STANDARD;
	else if (sval=="fast")
	  sett=FAST;
	else if (sval=="highgain")
	  sett=HIGHGAIN;
	else {
	  sprintf(answer,"%s not defined for this detector",sval.c_str());
	  return string(answer);
	}
	break;

     case GOTTHARD:
     case AGIPD:
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
	break;

     default:
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
    } 

  /* MYTHEN POTS */
  else if (var=="vthreshold") {
      if (action==PUT_ACTION) {
	sscanf(args[1],"%f",&fval);
	setDAC(fval, THRESHOLD);
      } 
      sprintf(answer,"%f",setDAC(-1,THRESHOLD));
      return string(answer);
    } else if (var=="vcalibration") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, CALIBRATION_PULSE);
       }
       sprintf(answer,"%f",setDAC(-1,CALIBRATION_PULSE));
       return string(answer);
      } else if (var=="vtrimbit") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, TRIMBIT_SIZE);
       }
       sprintf(answer,"%f",setDAC(-1,TRIMBIT_SIZE));
      return string(answer);
      } else if (var=="vpreamp") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, PREAMP);
       }
       sprintf(answer,"%f",setDAC(-1,PREAMP));
       return string(answer);
      } else if (var=="vshaper1") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, SHAPER1);
       }
       sprintf(answer,"%f",setDAC(-1,SHAPER1));
       return string(answer);
      } else if (var=="vshaper2") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, SHAPER2);
       }
       sprintf(answer,"%f",setDAC(-1,SHAPER2));
       return string(answer);
      } else if (var=="vhighvoltage") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, HV_POT);
      }
      sprintf(answer,"%f",setDAC(-1,HV_POT));
       return string(answer);
      } else if (var=="vapower") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, VA_POT);
       }
       sprintf(answer,"%f",setDAC(-1,VA_POT));
       return string(answer);
      } else if (var=="vddpower") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, VDD_POT);
       }
       sprintf(answer,"%f",setDAC(-1,VDD_POT));
       return string(answer);
      } else if (var=="vshpower") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, VSH_POT);
       }
       sprintf(answer,"%f",setDAC(-1,VSH_POT));
       return string(answer);
      } else if (var=="viopower") {
       if (action==PUT_ACTION) {
         sscanf(args[1],"%f",&fval);
         setDAC(fval, VIO_POT);
       }
       sprintf(answer,"%f",setDAC(-1,VIO_POT));
       return string(answer);
      }
  /* GOTTHARD POTS */
else if (var=="vref_ds") {
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

  /*
 else if (var=="temp") { 
       if (action==PUT_ACTION) {
	return string("cannot set");
      }
      sprintf(answer,"%f",getTemperature());
      return string(answer);
  }
  */
 
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
          switch(thisDetector->myDetectorType) {
            case GOTTHARD:
	      sprintf(answer,"Number of gates is always 1 for this detector",sval.c_str());
	      return string(answer);
	      break;
	    default:
	      if (action==PUT_ACTION) {
		sscanf(args[1],"%d",&ival); 
		setTimer( GATES_NUMBER,ival);
	      } 
	      sprintf(answer,"%lld",setTimer(GATES_NUMBER));
	      return string(answer);
	  }
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
    }  else if (var=="now") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%0.9f",(float)getTimeLeft(ACTUAL_TIME)*1E-9);
      //sprintf(answer,"%x",getTimeLeft(ACTUAL_TIME));
      return string(answer);
    }  else if (var=="timestamp") {
      if (action==PUT_ACTION) {
	sprintf(answer,"Cannot set\n");
      } else
	sprintf(answer,"%0.9f",(float)getTimeLeft(MEASUREMENT_TIME)*1E-9);
      //sprintf(answer,"%x",getTimeLeft(MEASUREMENT_TIME));
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
      } else if (var=="trimbits") {
	if (narg>=2) {
	  int nm=setNumberOfModules(GET_FLAG,X)*setNumberOfModules(GET_FLAG,Y);
	  sls_detector_module  *myMod=NULL;
	  sval=string(args[1]);
	  std::cout<< " trimfile " << sval << std::endl;
	  
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
	std::cout<< "Returning trimfile " << std::endl;
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
      } else if (var=="clkdivider") {
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








string slsDetector::helpLine( int action) {
  

  ostringstream os;
  
  if (action==READOUT_ACTION) {
    os << "Usage is "<< std::endl << "mythen_acquire  id " << std::endl;
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
    os << "trimdir path \t Sets path of the trim files " << std::endl;
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
    os << "\t standard \t fast \t highgain" << std::endl;
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
    os << "trimdir \t Gets path of the trim files " << std::endl;
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
    os << "\t 0 standard \t 1 fast \t 2 highgain \t else undefined" << std::endl;
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
    os << "now \t Gets the actual time of the detector (in s)"<< std::endl;


    os << std::endl;
    os << "timestamp \t Gets the  time of the measurements of the detector (in s, -1 if no measurement left)"<< std::endl;





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





//Corrections



int slsDetector::setAngularConversion(string fname) {
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



int slsDetector::readAngularConversion(string fname) {
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


int slsDetector:: writeAngularConversion(string fname) {
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
  return 0;
}

int slsDetector::resetMerging(float *mp, float *mv, float *me, int *mm) {
  
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


int slsDetector::finalizeMerging(float *mp, float *mv, float *me, int *mm) {
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
  return np;
}

int  slsDetector::addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm) {

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
      if ((thisDetector->correctionMask)&(1<< DISCARD_BAD_CHANNELS)) {
	if (badChannelMask[ip]) {
	  continue;
	}
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
	if ((thisDetector->correctionMask)&(1<< DISCARD_BAD_CHANNELS)) {
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

void  slsDetector::acquire(int delflag){
  void *status;
  //#ifdef VERBOSE
  //int iloop=0;
  //#endif
  int trimbit;
  int startindex=thisDetector->fileIndex;
  int lastindex=thisDetector->fileIndex;
  char cmd[MAX_STR_LENGTH];
  int nowindex=thisDetector->fileIndex;
  string fn;


  //string sett;
  if ((thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) || (thisDetector->correctionMask&(1<< I0_NORMALIZATION)))
       connect_channels();
       



  thisDetector->progressIndex=0;
  thisDetector->stoppedFlag=0;



  resetFinalDataQueue();
  resetDataQueue();

  
  //cout << "main mutex lock line 6188" << endl;
  pthread_mutex_lock(&mp);
  jointhread=0;
  queuesize=0;
  pthread_mutex_unlock(&mp);
  //cout << "main mutex unlock line 6188" << endl;





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
       
       fn=createFileName();
       nowindex=thisDetector->fileIndex;

       if (thisDetector->stoppedFlag==0) {


	 if (thisDetector->correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0();
	 if (thisDetector->actionMask & (1 << headerBefore)) {
	   //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	   sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s",thisDetector->actionScript[headerBefore],nowindex,fn.c_str(),((float)thisDetector->timerValue[ACQUISITION_TIME])*1E-9, thisDetector->currentSettings, thisDetector->currentThresholdEV, getBadChannelCorrectionFile().c_str(), getAngularConversion().c_str(), thisDetector->globalOffset, thisDetector->fineOffset,getFlatFieldCorrectionDir(),getFlatFieldCorrectionFile(), getRateCorrectionTau(), thisDetector->actionParameter[headerBefore]);
#ifdef VERBOSE
	   cout << "Executing header before " << cmd << endl;
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
       //cout << "main mutex lock line 6372" << endl;
       pthread_mutex_lock(&mp);
       while (queuesize){
	 pthread_mutex_unlock(&mp);
	 //cout << "main mutex unlock line 6375" << endl;
	 usleep(10000);
	 //cout << "main mutex lock line 6378" << endl;
	 pthread_mutex_lock(&mp);
       }
       pthread_mutex_unlock(&mp);
       //cout << "main mutex unlock line 6381" << endl;
    
    if (thisDetector->stoppedFlag==0) {
      if (thisDetector->actionMask & (1 << headerAfter)) {
	//Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s", \
		thisDetector->actionScript[headerAfter],		\
		nowindex,							\
		fn.c_str(),				\
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
      if (thisDetector->fileIndex>lastindex)
	lastindex=thisDetector->fileIndex;
    } else {

      
      if (thisDetector->fileIndex>lastindex)
	lastindex=thisDetector->fileIndex;

      break;
    }
     

    
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

      if ((thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) || (thisDetector->correctionMask&(1<< I0_NORMALIZATION)))
	  disconnect_channels();
}


void* slsDetector::processData(int delflag) {


  //cout << "thread mutex lock line 6505" << endl;
  pthread_mutex_lock(&mp);
  queuesize=dataQueue.size();
  pthread_mutex_unlock(&mp);
  //cout << "thread mutex unlock line 6505" << endl;

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
  string fname;


#ifdef ACQVERBOSE
  std::cout<< " processing data - threaded mode " << thisDetector->threadedProcessing;
#endif

  if (thisDetector->correctionMask!=0) {
    ext=".dat";
  } else {
    ext=".raw";
  }
  while(dum | thisDetector->threadedProcessing) { // ????????????????????????
    
    
    //  while( !dataQueue.empty() ) {
    //cout << "thread mutex lock line 6539" << endl;
    pthread_mutex_lock(&mp);
    while((queuesize=dataQueue.size())>0) {
      pthread_mutex_unlock(&mp);
      //cout << "thread mutex unlock line 6543" << endl;
	  //queuesize=dataQueue.size();

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
	
	fname=createFileName();
	
	/** write raw data file */	   
	if (thisDetector->correctionMask==0 && delflag==1) {
	  //cout << "line 6570----" << endl;
	  writeDataFile (fname+string(".raw"), fdata, NULL, NULL, 'i'); 
	  delete [] fdata;
	} else {
	  //cout << "line 6574----" << endl;
	  writeDataFile (fname+string(".raw"), fdata, NULL, NULL, 'i');

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
	    
	    if (thisDetector->correctionMask!=0) {
	      //cout << "line 6633----" << endl; 
	      if (thisDetector->numberOfPositions>1)
		writeDataFile (fname+string(".dat"), ffcdata, ffcerr,ang);
	    }
	    addToMerging(ang, ffcdata, ffcerr, mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	    
	    if ((currentPositionIndex==thisDetector->numberOfPositions) || (currentPositionIndex==0)) {
	      np=finalizeMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	      /** file writing */
	      currentPositionIndex++;
	      fname=createFileName();
	      if (thisDetector->correctionMask!=0) {
		//	cout << "line 6643----" << endl; 
		writeDataFile (fname+string(".dat"),mergingCounts, mergingErrors, mergingBins,'f',np);
	      }
	      if (delflag) {
		delete [] mergingBins;
		delete [] mergingCounts;
		delete [] mergingErrors;
		delete [] mergingMultiplicity;
	      } else {
		thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(fname+string(ext)).c_str(),np);/*
		if (thisDetector->correctionMask!=0) {
		  //thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,thisDetector->progressIndex+1,(fname().append(".dat")).c_str(),np);
		  thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(fname().append(".dat")).c_str(),np);
		} else {
		  thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(fname().append(".raw")).c_str(),np);
		  //thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,thisDetector->progressIndex+1,(fname().append(".raw")).c_str(),np);
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
	      // cout << "line 6672----" << endl; 
	      writeDataFile (fname+string(".dat"), ffcdata, ffcerr);
	    }
	    if (delflag) {
	      if (ffcdata)
		delete [] ffcdata;
	      if (ffcerr)
		delete [] ffcerr;
	      if (ang)
		delete [] ang;
	    } else {
	      thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(fname+string(ext)).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);/*
	      if (thisDetector->correctionMask!=0) {
		thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(fname().append(".dat")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
		//thisData=new detectorData(ffcdata,ffcerr,NULL,thisDetector->progressIndex+1,(fname().append(".dat")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
	      }	else {
		thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(fname().append(".raw")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
		//thisData=new detectorData(ffcdata,ffcerr,NULL,thisDetector->progressIndex+1,(fname().append(".raw")).c_str(),thisDetector->nChans*thisDetector->nChips*thisDetector->nMods);
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
	//cout << "thread mutex lock line 6697" << endl;
	pthread_mutex_lock(&mp);
	queuesize=dataQueue.size();
	pthread_mutex_unlock(&mp);
	//cout << "thread mutex unlock line 6697" << endl;
	usleep(1000);
	//pthread_mutex_unlock(&mp);
      }
      pthread_mutex_unlock(&mp);
      //cout << "thread mutex unlock line 6706" << endl;
      usleep(1000);
      //  cout << "PPPPPPPPPPPPPPPPPPPP " << queuesize << " " << thisDetector->fileIndex << endl;
    }    
    pthread_mutex_unlock(&mp);
    //cout << "thread mutex unlock line 6711" << endl;
    //cout << "thread mutex lock line 6711" << endl;
    pthread_mutex_lock(&mp);
    if (jointhread) {
      pthread_mutex_unlock(&mp);
      //cout << "thread mutex unlock line 6715" << endl;
      if (dataQueue.size()==0)
	break;
    } else
      pthread_mutex_unlock(&mp);
    //cout << "thread mutex unlock line 6720" << endl;
      
    dum=0;
  } // ????????????????????????
  return 0;
}



void slsDetector::startThread(int delflag) {
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
   slsDetector *myDet=(slsDetector*)n;
   myDet->processData(1);
   pthread_exit(NULL);
   
}

void* startProcessDataNoDelete(void *n) {
   //void* processData(void *n) {
  slsDetector *myDet=(slsDetector*)n;
  myDet->processData(0);
  pthread_exit(NULL);

}
 




  /*
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
int slsDetector::setPositions(int nPos, float *pos){
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
int slsDetector::getPositions(float *pos){ 
  if (pos ) {
    for (int ip=0; ip<thisDetector->numberOfPositions; ip++) 
      pos[ip]=thisDetector->detPositions[ip];
  } 
  setTotalProgress();

  
  return     thisDetector->numberOfPositions;
}
  



