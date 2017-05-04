#include "slsDetector.h"
#include "usersFunctions.h"
#include "slsDetectorCommand.h"
#include "postProcessingFuncs.h"
#include  <sys/types.h>
#include  <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bitset>
#include <cstdlib>
#include <math.h>
#include "gitInfoLib.h"

int slsDetector::initSharedMemory(detectorType type, int id) {


  /**
     the shared memory key is set to DEFAULT_SHM_KEY+id
  */
  key_t     mem_key=DEFAULT_SHM_KEY+id;
  int       shm_id;
  int nch, nm, nc, nd, ng, no;
  int sz;

  //shmId=-1;
#ifdef VERBOSE
  cout << "init shm"<< endl;
#endif
  switch(type) {
  case MYTHEN:
    nch=128; // complete mythen system
    nm=24;
    nc=10;
    nd=6; // dacs+adcs
    ng=0;
    no=0;
    break;
  case PICASSO:
    nch=128; // complete mythen system
    nm=24;
    nc=12;
    nd=6; // dacs+adcs
    ng=0;
    no=0;
    break;
  case GOTTHARD:
    nch=128;
    nm=1;
    nc=10;
    nd=13; // dacs+adcs
    ng=0;
    no=0;
    break;
  case PROPIX:
    nch=22*22;
    nm=1;
    nc=1;
    nd=13; // dacs+adcs
    break;
  case EIGER:
    nch=256*256; // one EIGER half module
    nm=1; //modules/detector
    nc=4; //chips
    nd=16; //dacs+adcs
    ng=4;
    no=4;
    break;
  case MOENCH:
    nch=160*160;
    nm=1; //modules/detector
    nc=1; //chips
    nd=9; //dacs+adcs
    ng=0;
    no=0;
    break;
  case JUNGFRAU:
    nch=256*256;
    nm=1; //modules/detector
    nc=8; //chips
    nd=16; //dacs+adcs
    ng=0;
    no=0;
    break;
  case JUNGFRAUCTB:
    nch=36; //36? is using digital value as well
    nm=1; //modules/detector
    nc=1; //chips
    nd=16; //dacs+adcs
    ng=0;
    no=0;
    break;
  default:
    nch=0; // dum!
    nm=0; //modules/detector
    nc=0; //chips
    nd=0; //dacs+adcs
    ng=0;
    no=0;
     break;
  }
  /**
     The size of the shared memory is:
     size of shared structure + ffcoefficents +fferrors + modules+ dacs+adcs+chips+chans+gain+offset
  */


  sz=sizeof(sharedSlsDetector)+nm*(2*nch*nc*sizeof(double)+sizeof(sls_detector_module)+sizeof(int)*nc+sizeof(dacs_t)*nd+sizeof(int)*nch*nc+sizeof(int)*ng+sizeof(int)*no);
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
#ifdef VERBOSE
  cout <<"shm done"<<endl;
#endif

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



slsDetector::slsDetector(int pos, int id, multiSlsDetector *p) :slsDetectorUtils(),
						      thisDetector(NULL),
						      detId(id),
							  posId(pos),
							  parentDet(p),
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
						      chanregs(NULL),
							  gain(NULL),
							  offset(NULL),
						      thisReceiver(NULL)


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





slsDetector::slsDetector(int pos, detectorType type, int id, multiSlsDetector *p): slsDetectorUtils(),
									 thisDetector(NULL),
									 detId(id),
									 posId(pos),
								     parentDet(p),
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
									 chanregs(NULL),
									  gain(NULL),
									  offset(NULL),
								     thisReceiver(NULL)

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
  cout << "init det size"<< endl;
  initializeDetectorSize(type);



}


slsDetector::~slsDetector(){

  // Detach Memory address
  if (shmdt(thisDetector) == -1) {
    perror("shmdt failed\n");
    printf("Could not detach shared memory %d\n", shmId);
  } else
    printf("Shared memory %d detached\n", shmId);

delete thisReceiver;
};

slsDetector::slsDetector(int pos, char *name, int id, int cport,multiSlsDetector *p) : slsDetectorUtils(),
									      thisDetector(NULL),
									      detId(id),
										  posId(pos),
									      parentDet(p),
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
									      chanregs(NULL),
										  gain(NULL),
										  offset(NULL),
									      thisReceiver(NULL)


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


/*
  //receiver
  if((t != GENERIC) && (setReceiverOnline()==ONLINE_FLAG)) {
	  int k;
	  retval = FAIL;
	  if(setReceiverOnline(ONLINE_FLAG)==ONLINE_FLAG){
#ifdef VERBOSE
		  std::cout << "Sending detector type to Receiver " << (int)thisDetector->myDetectorType << std::endl;
#endif
		  if (connectData() == OK)
			  retval=thisReceiver->sendInt(fnum2,k,(int)t);
			   disconnectData();
		  if(retval==FAIL){
			  cout << "ERROR: Could not send detector type to receiver" << endl;
			  setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTTYPE_NOT_SET));
		  }
	  }
  }
*/
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

    /** set receiver tcp port */
    thisDetector->receiverTCPPort=DEFAULT_PORTNO+2;
    /** set receiver udp port */
    thisDetector->receiverUDPPort=DEFAULT_UDP_PORTNO;
    /** set receiver udp port for Eiger */
    thisDetector->receiverUDPPort2=DEFAULT_UDP_PORTNO+1;
    /** set receiver ip address/hostname */
    strcpy(thisDetector->receiver_hostname,"none");
    /** set receiver udp ip address */
    strcpy(thisDetector->receiverUDPIP,"none");
    /** set receiver udp mac address */
    strcpy(thisDetector->receiverUDPMAC,"none");
    /** set detector mac address */
    strcpy(thisDetector->detectorMAC,DEFAULT_DET_MAC);
    /** set detector ip address */
    strcpy(thisDetector->detectorIP,DEFAULT_DET_IP);

    /** sets onlineFlag to OFFLINE_FLAG */
    thisDetector->onlineFlag=OFFLINE_FLAG;
    /** set ports to defaults */
    thisDetector->controlPort=DEFAULT_PORTNO;
    thisDetector->stopPort=DEFAULT_PORTNO+1;

    /** set thisDetector->myDetectorType to type and according to this set nChans, nChips, nDacs, nAdcs, nModMax, dynamicRange, nMod*/
    thisDetector->myDetectorType=type;
    switch(thisDetector->myDetectorType) {
    case MYTHEN:
      thisDetector->nChan[X]=128;
      thisDetector->nChan[Y]=1;
      thisDetector->nChip[X]=10;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=6;
      thisDetector->nAdcs=0;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=24;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=24;
      thisDetector->moveFlag=1;
#ifdef VERBOSE
      cout << "move flag" << thisDetector->moveFlag<< endl;
#endif
      break;
    case PICASSO:
      thisDetector->nChan[X]=128;
      thisDetector->nChan[Y]=1;
      thisDetector->nChip[X]=12;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=6;
      thisDetector->nAdcs=0;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=6;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=24;
      break;
    case GOTTHARD:
      thisDetector->nChan[X]=128;
      thisDetector->nChan[Y]=1;
      thisDetector->nChip[X]=10;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=8;
      thisDetector->nAdcs=5;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=1;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=16;
      break;
    case PROPIX:
      thisDetector->nChan[X]=22;
      thisDetector->nChan[Y]=22;
      thisDetector->nChip[X]=1;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=8;
      thisDetector->nAdcs=5;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=1;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=16;
      break;
    case MOENCH:
      thisDetector->nChan[X]=160;
      thisDetector->nChan[Y]=160;
      thisDetector->nChip[X]=1;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=8;
      thisDetector->nAdcs=1;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=1;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=16;
      break;
    case JUNGFRAU:
      thisDetector->nChan[X]=256;
      thisDetector->nChan[Y]=256;
      thisDetector->nChip[X]=4;
      thisDetector->nChip[Y]=2;
      thisDetector->nDacs=16;
      thisDetector->nAdcs=0;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=1;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=16;
      break;
    case JUNGFRAUCTB:
      thisDetector->nChan[X]=36;
      thisDetector->nChan[Y]=1;
      thisDetector->nChip[X]=1;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=16;
      thisDetector->nAdcs=9;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=1;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=16;
      break;
    case EIGER:
      thisDetector->nChan[X]=256;
      thisDetector->nChan[Y]=256;
      thisDetector->nChip[X]=4;
      thisDetector->nChip[Y]=1;
      thisDetector->nDacs=16;
      thisDetector->nAdcs=0;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=1;
      thisDetector->nModMax[Y]=1;
      thisDetector->dynamicRange=16;
      break;
    default:
      thisDetector->nChan[X]=0;
      thisDetector->nChan[Y]=0;
     thisDetector->nChip[X]=0;
     thisDetector->nChip[Y]=0;
      thisDetector->nDacs=0;
      thisDetector->nAdcs=0;
      thisDetector->nGain=0;
      thisDetector->nOffset=0;
      thisDetector->nModMax[X]=0;
      thisDetector->nModMax[Y]=0;
      thisDetector->dynamicRange=32;
    }
    thisDetector->nChans=thisDetector->nChan[X]*thisDetector->nChan[Y];
    thisDetector->nChips=thisDetector->nChip[X]*thisDetector->nChip[Y];
    thisDetector->nModsMax=thisDetector->nModMax[X]*thisDetector->nModMax[Y];
    /** number of modules is initally the maximum number of modules */
    thisDetector->nMod[X]=thisDetector->nModMax[X];
    thisDetector->nMod[Y]=thisDetector->nModMax[Y];
    thisDetector->nMods=thisDetector->nModsMax;
    /** calculates the expected data size */
    thisDetector->timerValue[PROBES_NUMBER]=0;
    thisDetector->timerValue[FRAME_NUMBER]=1;
    thisDetector->timerValue[MEASUREMENTS_NUMBER]=1;
    thisDetector->timerValue[CYCLES_NUMBER]=1;
    thisDetector->timerValue[SAMPLES_JCTB]=1;

    thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*thisDetector->dynamicRange/8;

    if(thisDetector->myDetectorType==JUNGFRAUCTB) {
      cout << "here1" << endl;
      getTotalNumberOfChannels();
      //      thisDetector->dataBytes=getTotalNumberOfChannels()*thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
    }
    if(thisDetector->myDetectorType==MYTHEN){
    	if (thisDetector->dynamicRange==24 || thisDetector->timerValue[PROBES_NUMBER]>0)
    		thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
    }

    /** set trimDsdir, calDir to default to home directory*/
    strcpy(thisDetector->settingsDir,getenv("HOME"));
    strcpy(thisDetector->calDir,getenv("HOME"));

    /** sets trimbit file */
    strcpy(thisDetector->settingsFile,"none");
    /** set progress Index to default to 0*/
    thisDetector->progressIndex=0;
    /** set total number of frames to be acquired to default to 1*/
    thisDetector->totalProgress=1;

    /** set trimDsdir, calDir and filePath to default to root directory*/
    strcpy(thisDetector->filePath,"/");

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
    thisDetector->binSize=0.001;
    thisDetector->stoppedFlag=0;
    thisDetector->threadedProcessing=1;

    thisDetector->actionMask=0;

    thisDetector->tenGigaEnable=0;
    thisDetector->acquiringFlag = false;
    thisDetector->flippedData[0] = 0;
    thisDetector->flippedData[1] = 0;

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

    /* receiver*/
    /** sets receiver onlineFlag to OFFLINE_FLAG */
    thisDetector->receiverOnlineFlag=OFFLINE_FLAG;


    /** calculates the memory offsets for flat field coefficients and errors, module structures, dacs, adcs, chips and channels */
    thisDetector->ffoff=sizeof(sharedSlsDetector);
    thisDetector->fferroff=thisDetector->ffoff+sizeof(double)*thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
    thisDetector->modoff= thisDetector->fferroff+sizeof(double)*thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
    thisDetector->dacoff=thisDetector->modoff+sizeof(sls_detector_module)*thisDetector->nModsMax;
    thisDetector->adcoff=thisDetector->dacoff+sizeof(dacs_t)*thisDetector->nDacs*thisDetector->nModsMax;
    thisDetector->chipoff=thisDetector->adcoff+sizeof(dacs_t)*thisDetector->nAdcs*thisDetector->nModsMax;
    thisDetector->chanoff=thisDetector->chipoff+sizeof(int)*thisDetector->nChips*thisDetector->nModsMax;
    thisDetector->gainoff=thisDetector->chanoff+sizeof(int)*thisDetector->nGain*thisDetector->nModsMax;
    thisDetector->offsetoff=thisDetector->gainoff+sizeof(int)*thisDetector->nOffset*thisDetector->nModsMax;

    //update?!?!?!?

    if(thisDetector->myDetectorType==JUNGFRAUCTB) {
      //  cout << "here2" << endl;
      getTotalNumberOfChannels();
	//thisDetector->nChan[X]=32;
	//thisDetector->nChans=thisDetector->nChan[X]*thisDetector->nChan[Y];

	//thisDetector->dataBytes=getTotalNumberOfChannels()*thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
      
    }

  }


  /** also in case thisDetector alread existed initialize the pointer for flat field coefficients and errors, module structures, dacs, adcs, chips and channels */
  ffcoefficients=(double*)(goff+thisDetector->ffoff);
  fferrors=(double*)(goff+thisDetector->fferroff);
  detectorModules=(sls_detector_module*)(goff+ thisDetector->modoff);
#ifdef VERBOSE
  //   for (int imod=0; imod< thisDetector->nModsMax; imod++)
  //  std::cout<< hex << detectorModules+imod << dec <<std::endl;
#endif

  nBadChans=&thisDetector->nBadChans;
  badChansList=thisDetector->badChansList;
  badChanFile=thisDetector->badChanFile;
  nBadFF=&thisDetector->nBadFF;
  badFFList=thisDetector->badFFList;

  dacs=(dacs_t*)(goff+thisDetector->dacoff);
  adcs=(dacs_t*)(goff+thisDetector->adcoff);
  chipregs=(int*)(goff+thisDetector->chipoff);
  chanregs=(int*)(goff+thisDetector->chanoff);
  gain=(int*)(goff+thisDetector->gainoff);
  offset=(int*)(goff+thisDetector->offsetoff);
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
  expTime=&timerValue[ACQUISITION_TIME];

  currentSettings=&thisDetector->currentSettings;
  currentThresholdEV=&thisDetector->currentThresholdEV;
  moveFlag=&thisDetector->moveFlag;
  sampleDisplacement=NULL;
  settingsFile=thisDetector->settingsFile;

  filePath=thisDetector->filePath;
  pthread_mutex_lock(&ms);
  fileName=parentDet->fileName;
  fileIndex=parentDet->fileIndex;
  framesPerFile=parentDet->framesPerFile;
  fileFormatType=parentDet->fileFormatType;
  if((thisDetector->myDetectorType==GOTTHARD)||(thisDetector->myDetectorType==PROPIX)){
	  fileIO::setFramesPerFile(MAX_FRAMES_PER_FILE);
	  pthread_mutex_unlock(&ms);
	  setFileFormat(BINARY);
  }else if (thisDetector->myDetectorType==EIGER){
	  fileIO::setFramesPerFile(EIGER_MAX_FRAMES_PER_FILE);
	  pthread_mutex_unlock(&ms);
	  setFileFormat(BINARY);
  }else if (thisDetector->myDetectorType==MOENCH){
	  fileIO::setFramesPerFile(MOENCH_MAX_FRAMES_PER_FILE);
	  pthread_mutex_unlock(&ms);
	  setFileFormat(BINARY);
  }else if (thisDetector->myDetectorType==JUNGFRAU){
	  fileIO::setFramesPerFile(JFRAU_MAX_FRAMES_PER_FILE);
	  pthread_mutex_unlock(&ms);
	  setFileFormat(BINARY);
  }else if (thisDetector->myDetectorType==JUNGFRAUCTB){
	  setFramesPerFile(JFCTB_MAX_FRAMES_PER_FILE);
	  fileIO::setFramesPerFile(JFRAU_MAX_FRAMES_PER_FILE);
	  pthread_mutex_unlock(&ms);
	  setFileFormat(BINARY);
  }else
	  pthread_mutex_unlock(&ms);
  if (thisReceiver != NULL)
	  delete thisReceiver;
  thisReceiver = new receiverInterface(dataSocket);

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
  //char *p2;
  //p2=(char*)thisDetector;

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
      *(dacs+idac+thisDetector->nDacs*imod)=0;
    }


    /** initializes the adc values to 0 */
    for (int iadc=0; iadc<thisDetector->nAdcs; iadc++) {
      *(adcs+iadc+thisDetector->nAdcs*imod)=0;
    }



    /** initializes the chip registers to 0 */
    for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
      *(chipregs+ichip+thisDetector->nChips*imod)=-1;
    }


    /** initializes the channel registers to 0 */
    for (int ichan=0; ichan<thisDetector->nChans*thisDetector->nChips; ichan++) {
      *(chanregs+ichan+thisDetector->nChips*thisDetector->nChans*imod)=-1;
    }

    /** initializes the gain values to 0 */
    for (int igain=0; igain<thisDetector->nGain; igain++) {
      *(gain+igain+thisDetector->nGain*imod)=0;
    }


    /** initializes the offset values to 0 */
    for (int ioffset=0; ioffset<thisDetector->nOffset; ioffset++) {
      *(offset+ioffset+thisDetector->nOffset*imod)=0;
    }

    /** initialize gain and offset to -1 */
    thisMod->gain=-1.;
    thisMod->offset=-1.;
  }
  return 0;
}

slsDetectorDefs::sls_detector_module*  slsDetector::createModule(detectorType t) {

  sls_detector_module *myMod=(sls_detector_module*)malloc(sizeof(sls_detector_module));


  int nch, nc, nd, na=0;
 // int nm = 0;

  switch(t) {
  case MYTHEN:
    nch=128; // complete mythen system
//  nm=24;
    nc=10;
    nd=6; // dacs
    break;
  case PICASSO:
    nch=128; // complete mythen system
 // nm=24;
    nc=12;
    nd=6; // dacs+adcs
    break;
  case GOTTHARD:
    nch=128;
//  nm=1;
    nc=10;
    nd=8; // dacs+adcs
    na=5;
    break;
  case PROPIX:
    nch=22*22;
//   nm=1;
    nc=1;
    nd=8; // dacs+adcs
    na=5;
    break;
  case EIGER:
    nch=256*256; // one EIGER half module
//  nm=1; //modules/detector
    nc=4*1; //chips 
    nd=16; //dacs
    na=0;
    break;
  case MOENCH:
    nch=160*160;
//   nm=1; //modules/detector
    nc=1; //chips
    nd=8; //dacs
    na=1;
    break;
  case JUNGFRAU:
    nch=256*256;//32;
 // nm=1;
    nc=4*2;
    nd=16; // dacs+adcs
    na=0;
    break;
  case JUNGFRAUCTB:
    nch=36;
//  nm=1;
    nc=1;
    nd=8; // dacs+adcs
    na=1;
    break;
  default:
    nch=0; // dum!
//  nm=0; //modules/detector
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
	int ts=0;
	ts+=controlSocket->SendDataOnly(&(myChan->chan),sizeof(myChan->chan));
	ts+=controlSocket->SendDataOnly(&(myChan->chip),sizeof(myChan->chip));
	ts+=controlSocket->SendDataOnly(&(myChan->module),sizeof(myChan->module));
	ts=controlSocket->SendDataOnly(&(myChan->reg),sizeof(myChan->reg));
	return ts;
}

int slsDetector::sendChip(sls_detector_chip *myChip) {
	int ts=0;
	//send chip structure
	ts+=controlSocket->SendDataOnly(&(myChip->chip),sizeof(myChip->chip));
	ts+=controlSocket->SendDataOnly(&(myChip->module),sizeof(myChip->module));
	ts+=controlSocket->SendDataOnly(&(myChip->nchan),sizeof(myChip->nchan));
	ts+=controlSocket->SendDataOnly(&(myChip->reg),sizeof(myChip->reg));
	ts+=controlSocket->SendDataOnly(myChip->chanregs,sizeof(myChip->chanregs));
#ifdef VERY_VERBOSE
	std::cout<< "chip structure sent" << std::endl;
	std::cout<< "now sending " << myChip->nchan << " channles" << std::endl;
#endif
	ts=controlSocket->SendDataOnly(myChip->chanregs,sizeof(int)*myChip->nchan );
#ifdef VERBOSE
	std::cout<< "chip's channels sent " <<ts<< std::endl;
#endif
	return ts;
}

int slsDetector::sendModule(sls_detector_module *myMod) {
	int ts=0;
	//send module structure
	ts+=controlSocket->SendDataOnly(&(myMod->module),sizeof(myMod->module));
	ts+=controlSocket->SendDataOnly(&(myMod->serialnumber),sizeof(myMod->serialnumber));
	ts+=controlSocket->SendDataOnly(&(myMod->nchan),sizeof(myMod->nchan));
	ts+=controlSocket->SendDataOnly(&(myMod->nchip),sizeof(myMod->nchip));
	ts+=controlSocket->SendDataOnly(&(myMod->ndac),sizeof(myMod->ndac));
	ts+=controlSocket->SendDataOnly(&(myMod->nadc),sizeof(myMod->nadc));
	ts+=controlSocket->SendDataOnly(&(myMod->reg),sizeof(myMod->reg));
	ts+=controlSocket->SendDataOnly(myMod->dacs,sizeof(myMod->ndac));
	ts+=controlSocket->SendDataOnly(myMod->adcs,sizeof(myMod->nadc));

	if(thisDetector->myDetectorType != JUNGFRAU){
		ts+=controlSocket->SendDataOnly(myMod->chipregs,sizeof(myMod->nchip));
		ts+=controlSocket->SendDataOnly(myMod->chanregs,sizeof(myMod->nchan));
	}
	ts+=controlSocket->SendDataOnly(&(myMod->gain),sizeof(myMod->gain));
	ts+=controlSocket->SendDataOnly(&(myMod->offset), sizeof(myMod->offset));
	ts+=controlSocket->SendDataOnly(myMod->dacs,sizeof(dacs_t)*(myMod->ndac));
	ts+=controlSocket->SendDataOnly(myMod->adcs,sizeof(dacs_t)*(myMod->nadc));

	if(thisDetector->myDetectorType != JUNGFRAU){
		ts+=controlSocket->SendDataOnly(myMod->chipregs,sizeof(int)*(myMod->nchip));
		ts+=controlSocket->SendDataOnly(myMod->chanregs,sizeof(int)*(myMod->nchan));
	}
	return ts;
}

int slsDetector::receiveChannel(sls_detector_channel *myChan) {
	int ts=0;
	ts+=controlSocket->ReceiveDataOnly(&(myChan->chan),sizeof(myChan->chan));
	ts+=controlSocket->ReceiveDataOnly(&(myChan->chip),sizeof(myChan->chip));
	ts+=controlSocket->ReceiveDataOnly(&(myChan->module),sizeof(myChan->module));
	ts=controlSocket->ReceiveDataOnly(&(myChan->reg),sizeof(myChan->reg));
	return ts;
}

int slsDetector::receiveChip(sls_detector_chip* myChip) {
	int *ptr=myChip->chanregs;
	int nchanold=myChip->nchan;
	int ts=0;
	int nch;

	//receive chip structure
	ts+=controlSocket->ReceiveDataOnly(&(myChip->chip),sizeof(myChip->chip));
	ts+=controlSocket->ReceiveDataOnly(&(myChip->module),sizeof(myChip->module));
	ts+=controlSocket->ReceiveDataOnly(&(myChip->nchan),sizeof(myChip->nchan));
	ts+=controlSocket->ReceiveDataOnly(&(myChip->reg),sizeof(myChip->reg));
	ts+=controlSocket->ReceiveDataOnly(myChip->chanregs,sizeof(myChip->chanregs));

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
	//send module structure
	ts+=controlSocket->ReceiveDataOnly(&(myMod->module),sizeof(myMod->module));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->serialnumber),sizeof(myMod->serialnumber));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->nchan),sizeof(myMod->nchan));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->nchip),sizeof(myMod->nchip));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->ndac),sizeof(myMod->ndac));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->nadc),sizeof(myMod->nadc));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->reg),sizeof(myMod->reg));
	ts+=controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(myMod->ndac));
	ts+=controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(myMod->nadc));

	if(thisDetector->myDetectorType != JUNGFRAU){
		ts+=controlSocket->ReceiveDataOnly(myMod->chipregs,sizeof(myMod->nchip));
		ts+=controlSocket->ReceiveDataOnly(myMod->chanregs,sizeof(myMod->nchan));
	}
	ts+=controlSocket->ReceiveDataOnly(&(myMod->gain), sizeof(myMod->gain));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->offset), sizeof(myMod->offset));


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

  if(thisDetector->myDetectorType != JUNGFRAU){
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
  }

#ifdef VERBOSE
  std::cout<< "received module " << myMod->module << " of size "<< ts << " register " << myMod->reg << std::endl;
#endif

  return ts;
}


int slsDetector::setOnline(int off) {
  int old=thisDetector->onlineFlag;
  if (off!=GET_ONLINE_FLAG) {
    thisDetector->onlineFlag=off;
    if (thisDetector->onlineFlag==ONLINE_FLAG) {
      setTCPSocket();
      if (thisDetector->onlineFlag==ONLINE_FLAG && old==OFFLINE_FLAG) {
	cout << "Detector connecting for the first time - updating!" << endl;
	updateDetector();
      }
      else if(thisDetector->onlineFlag==OFFLINE_FLAG){
		std::cout << "cannot connect to detector" << endl;
		setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
		}
    }
  }
  return thisDetector->onlineFlag;
}



string slsDetector::checkOnline() {
  string retval = string("");
  if(!controlSocket){
    //this already sets the online/offline flag
    setTCPSocket();
    if(thisDetector->onlineFlag==OFFLINE_FLAG)
      return string(thisDetector->hostname);
    else
      return string("");
  }
  //still cannot connect to socket, controlSocket=0
  if(controlSocket){
    if (connectControl() == FAIL) {
      controlSocket->SetTimeOut(5);
      thisDetector->onlineFlag=OFFLINE_FLAG;
      delete controlSocket;
      controlSocket=NULL;
      retval = string(thisDetector->hostname);
#ifdef VERBOSE
      std::cout<< "offline!" << std::endl;
#endif
    }  else {
      thisDetector->onlineFlag=ONLINE_FLAG;
      controlSocket->SetTimeOut(100);
      disconnectControl();
#ifdef VERBOSE
      std::cout<< "online!" << std::endl;
#endif
    }
  }
  return retval;
}



int slsDetector::activate(int const enable){
	int fnum = F_ACTIVATE;
	int retval = -1;
	int arg = enable;
	char mess[MAX_STR_LENGTH]="";
	int ret = OK;

	if(thisDetector->myDetectorType != EIGER){
		std::cout<< "Not implemented for this detector" << std::endl;
		setErrorMask((getErrorMask())|(DETECTOR_ACTIVATE));
		return -1;
	}

#ifdef VERBOSE
	if(!enable)
		std::cout<< "Deactivating Detector" << std::endl;
	else if(enable == -1)
		std::cout<< "Getting Detector activate mode" << std::endl;
	else
		std::cout<< "Activating Detector" << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(DETECTOR_ACTIVATE));
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	if(retval==1)
		std::cout << "Detector Activated"  << std::endl;
	else if(retval==0)
		std::cout << "Detector Deactivated" << std::endl;
	else
		std::cout << "Detector Activation unknown:" << retval << std::endl;
#endif

	if(ret!=FAIL){
		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Activating/Deactivating Receiver: " << retval << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum,retval,retval);
				disconnectData();
			}
			if(ret==FAIL)
				setErrorMask((getErrorMask())|(RECEIVER_ACTIVATE));
		}
	}
#ifdef VERBOSE
	if(retval==1)
		std::cout << "Receiver Activated"  << std::endl;
	else if(retval==0)
		std::cout << "Receiver Deactivated" << std::endl;
	else
		std::cout << "Receiver Activation unknown:" << retval << std::endl;
#endif


	return retval;

}


/*
   configure the socket communication and check that the server exists
   enum communicationProtocol{
   TCP,
   UDP
   }{};

*/

int slsDetector::setTCPSocket(string const name, int const control_port, int const stop_port){

  char thisName[MAX_STR_LENGTH];
  int thisCP, thisSP;
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
  if (retval!=FAIL) {
    checkOnline();
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
	if (controlSocket){
		if (controlSocket->Connect() >= 0)
			return OK;
		else{
			std::cout << "cannot connect to detector" << endl;
			setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
			return FAIL;
		}
	}
	return UNDEFINED;
}
/** disconnect from the control port */
int slsDetector::disconnectControl() {
  if (controlSocket)
    controlSocket->Disconnect();
  return OK;
}



/** connect to the data port */
int slsDetector::connectData() {
	if (dataSocket){
		if (dataSocket->Connect() >= 0)
			return OK;
		else{
			std::cout << "cannot connect to receiver" << endl;
			setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
			return FAIL;}
	}
	return UNDEFINED;
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
	if (stopSocket){
		if (stopSocket->Connect() >= 0)
			return OK;
		else{
			std::cout << "cannot connect to stop server" << endl;
			return FAIL;
		}
	}
	return UNDEFINED;
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
    if (connectControl() == OK){
      if (controlSocket->SendDataOnly(&fnum,sizeof(fnum))>=0) {
	if (controlSocket->SendDataOnly(arg,MAX_STR_LENGTH)>=0) {
	  if (controlSocket->ReceiveDataOnly(retval,MAX_STR_LENGTH)>=0) {
	    ret=OK;
	    answer=retval;
	  }
	}
      }
      disconnectControl();
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
   AGIPD,
   MOENCH
   };

*/
int slsDetector::setDetectorType(detectorType const type){

  int arg, retval=FAIL;
  int fnum=F_GET_DETECTOR_TYPE,fnum2=F_GET_RECEIVER_TYPE;
  arg=int(type);
  detectorType retType=type;
  char mess[MAX_STR_LENGTH]="";
  strcpy(mess,"dummy");


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting detector type to " << arg << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      if (retval!=FAIL)
	controlSocket->ReceiveDataOnly(&retType,sizeof(retType));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (retval==FORCE_UPDATE)
	updateDetector();
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


  //receiver
  if((retType != GENERIC) && (thisDetector->receiverOnlineFlag==ONLINE_FLAG)) {
	  retval = FAIL;
	  if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		  std::cout << "Sending detector type to Receiver " << (int)thisDetector->myDetectorType << std::endl;
#endif
		  if (connectData() == OK){
			  retval=thisReceiver->sendInt(fnum2,arg,(int)thisDetector->myDetectorType);
			  disconnectData();
		  }
		  if(retval==FAIL){
			  cout << "ERROR: Could not send detector type to receiver" << endl;
			  setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTTYPE_NOT_SET));
		  }
	  }
  }



  return retType;
};

int slsDetector::setDetectorType(string const stype){
  return setDetectorType(getDetectorType(stype));
};

slsDetectorDefs::detectorType slsDetector::getDetectorsType(int pos){
  return thisDetector->myDetectorType;
}


    // /** number of rois defined */
    // int nROI;
    // /** list of rois */
    // ROI roiLimits[MAX_ROIS];
  
    // /** readout flags */
    // readOutFlags roFlags;


int slsDetector::getTotalNumberOfChannels() {
#ifdef VERBOSE
  cout << "total number of channels" << endl;
#endif
  if(thisDetector->myDetectorType==JUNGFRAUCTB){
    if (thisDetector->roFlags&DIGITAL_ONLY)
      thisDetector->nChan[X]=4;
    else if (thisDetector->roFlags&ANALOG_AND_DIGITAL)
      thisDetector->nChan[X]=36;
    else
      thisDetector->nChan[X]=32;
      
    if (thisDetector->nChan[X]>=32) {
      if (thisDetector->nROI>0) {
	thisDetector->nChan[X]-=32;
	for (int iroi=0; iroi<thisDetector->nROI; iroi++)
	  thisDetector->nChan[X]+=thisDetector->roiLimits[iroi].xmax-thisDetector->roiLimits[iroi].xmin+1;
      }
    }
    thisDetector->nChans=thisDetector->nChan[X];
    thisDetector->dataBytes=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods*2*thisDetector->timerValue[SAMPLES_JCTB];
  } else {
#ifdef VERBOSE
    cout << "det type is "<< thisDetector->myDetectorType << endl;
  cout << "Total number of channels is "<< thisDetector->nChans*thisDetector->nChips*thisDetector->nMods << " data bytes is " << thisDetector->dataBytes << endl;
#endif
  ;
  }
  return thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
}

int slsDetector::getTotalNumberOfChannels(dimension d) {
  getTotalNumberOfChannels();
  return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nMod[d];
};



int slsDetector::getMaxNumberOfChannels(){
  if(thisDetector->myDetectorType==JUNGFRAUCTB) return 36*thisDetector->nChips*thisDetector->nModsMax;
  return thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
};

int slsDetector::getMaxNumberOfChannels(dimension d){  
  if(thisDetector->myDetectorType==JUNGFRAUCTB) {
	  if (d==X) return 36*thisDetector->nChip[d]*thisDetector->nModMax[d];
	  else return 1*thisDetector->nChip[d]*thisDetector->nModMax[d];
  }
  return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nModMax[d];
};

/* needed to set/get the size of the detector */
// if n=GET_FLAG returns the number of installed modules,
int slsDetector::setNumberOfModules(int n, dimension d){

  int arg[2], retval=1;
  int fnum=F_SET_NUMBER_OF_MODULES;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="dummy";
  int connect;

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
    connect = connectControl();
    if (connect == UNDEFINED)
      cout << "no control socket?" << endl;
    else if (connect == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&arg,sizeof(arg));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      }	else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  } else {
    cout << "offline" << endl;
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


    if (thisDetector->nModsMax<thisDetector->nMods)
      thisDetector->nModsMax=thisDetector->nMods;

    if (thisDetector->nModMax[X]<thisDetector->nMod[X])
      thisDetector->nModMax[X]=thisDetector->nMod[X];

    if (thisDetector->nModMax[Y]<thisDetector->nMod[Y])
      thisDetector->nModMax[Y]=thisDetector->nMod[Y];

    int dr=thisDetector->dynamicRange;
    if ((thisDetector->myDetectorType==MYTHEN) && (dr==24))
      dr=32;

    thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*dr/8;

    if(thisDetector->myDetectorType==MYTHEN){
      if (thisDetector->timerValue[PROBES_NUMBER]!=0)
	thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
    }

    if(thisDetector->myDetectorType==JUNGFRAUCTB){
      getTotalNumberOfChannels();
      //thisDetector->dataBytes=getTotalNumberOfChannels()*thisDetector->nChans*dr/8*thisDetector->nChips*thisDetector->timerValue[SAMPLES_JCTB];

    }

#ifdef VERBOSE
    std::cout<< "Data size is " << thisDetector->dataBytes << std::endl;
    std::cout<< "nModX " << thisDetector->nMod[X] << " nModY " << thisDetector->nMod[Y] << " nChips " << thisDetector->nChips << " nChans " << thisDetector->nChans<< " dr " << dr << std::endl;
#endif
  }

  if(n != GET_FLAG){
	  pthread_mutex_lock(&ms);
	  parentDet->updateOffsets();
	  pthread_mutex_unlock(&ms);
  }

  return thisDetector->nMod[d];
};



int slsDetector::getMaxNumberOfModules(dimension d){

  int retval;
  int fnum=F_GET_MAX_NUMBER_OF_MODULES;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";

  if (d<X || d>Y) {
    std::cout<< "Get max number of modules in wrong dimension " << d << std::endl;
    return ret;
  }
#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting max number of modules in dimension "<< d  <<std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&d,sizeof(d));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  } else {
    ret=OK;
    retval=thisDetector->nModMax[d];
  }
#ifdef VERBOSE
  std::cout<< "Max number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Get max number of modules failed " << retval << std::endl;
    return retval;
  }  else {
    thisDetector->nModMax[d]=retval;
    thisDetector->nModsMax=thisDetector->nModMax[X]*thisDetector->nModMax[Y];



  }
  return thisDetector->nModMax[d];
};





int slsDetector::setFlippedData(dimension d, int value){
	int retval=-1;
	int fnum=F_SET_FLIPPED_DATA_RECEIVER;
	int ret=FAIL;
	int args[2]={X,-1};


	if(thisDetector->myDetectorType!= EIGER){
		std::cout << "Flipped Data is not implemented in this detector" << std::endl;
		setErrorMask((getErrorMask())|(RECEIVER_FLIPPED_DATA_NOT_SET));
		return -1;
	}

#ifdef VERBOSE
	std::cout << std::endl;
	std::cout << "Setting/Getting flipped data across axis " << d <<" with value " << value << std::endl;
#endif
	if(value > -1){
		thisDetector->flippedData[d] = value;
		args[1] = value;
	}else
		args[1] = thisDetector->flippedData[d];

	args[0] = d;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
		if (connectData() == OK){
			ret=thisReceiver->sendIntArray(fnum,retval,args);

			disconnectData();
		}

		if((args[1] != retval && args[1]>=0) || (ret==FAIL)){
			ret = FAIL;
			setErrorMask((getErrorMask())|(RECEIVER_FLIPPED_DATA_NOT_SET));
		}

		if(ret==FORCE_UPDATE)
			updateReceiver();
	}


	return thisDetector->flippedData[d];
}




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
  char mess[MAX_STR_LENGTH]="";

  arg[0]=signalindex;
  arg[1]=pol;

  retval=GET_EXTERNAL_SIGNAL_FLAG;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting signal "<< signalindex <<  " to flag" << pol << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&arg,sizeof(arg));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
   BURST_TRIGGER,
   //GATE_COINCIDENCE_WITH_INTERNAL_ENABLE
   };

*/

slsDetectorDefs::externalCommunicationMode slsDetector::setExternalCommunicationMode( externalCommunicationMode pol){




  int arg[1];
  externalCommunicationMode  retval;
  int fnum=F_SET_EXTERNAL_COMMUNICATION_MODE;
  char mess[MAX_STR_LENGTH]="";

  arg[0]=pol;

  int ret=FAIL;
  retval=GET_EXTERNAL_COMMUNICATION_MODE;

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting communication to mode " << pol << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&arg,sizeof(arg));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
   DETECTOR_SOFTWARE_VERSION,
   THIS_SOFTWARE_VERSION,
   RECEIVER_VERSION
   }{};
*/





int64_t slsDetector::getId( idMode mode, int imod){

  int64_t retval=-1;
  int fnum=F_GET_ID,fnum2 = F_GET_RECEIVER_ID;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<< std::endl;
  if  (mode==MODULE_SERIAL_NUMBER)
    std::cout<< "Getting id  of "<< imod << std::endl;
  else
    std::cout<< "Getting id type "<< mode << std::endl;
#endif
  if (mode==THIS_SOFTWARE_VERSION) {
    ret=OK;
    retval=SVNREVLIB;
    retval=(retval<<32) | SVNDATELIB;
  } else if (mode==RECEIVER_VERSION) {
    if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
    	if (connectData() == OK){
    		ret=thisReceiver->getInt(fnum2,retval);
    		disconnectData();
    	}
      if(ret==FORCE_UPDATE)
	ret=updateReceiver();
    }
  } else {
    if (thisDetector->onlineFlag==ONLINE_FLAG) {
      if (connectControl() != OK)
	ret = FAIL;
      else{
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
	disconnectControl();
	if (ret==FORCE_UPDATE)
	  updateDetector();
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
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting id of "<< mode << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
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
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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

  char mess[MAX_STR_LENGTH]="";

  int arg[2];
  arg[0]=addr;
  arg[1]=val;


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Writing to register "<< hex<<addr <<  " data " << hex<<val << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
/* write or read register */

int slsDetector::writeAdcRegister(int addr, int val){


  int retval;
  int fnum=F_WRITE_ADC_REG;
  int ret=FAIL;

  char mess[MAX_STR_LENGTH]="";

  int arg[2];
  arg[0]=addr;
  arg[1]=val;


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Writing to adc register "<< hex<<addr <<  " data " << hex<<val << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }
#ifdef VERBOSE
  std::cout<< "ADC Register returned "<< retval << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Write ADC to register failed " << std::endl;
  }
  return retval;

};




int slsDetector::readRegister(int addr){


  int retval;
  int fnum=F_READ_REGISTER;
  int ret=FAIL;

  char mess[MAX_STR_LENGTH]="";

  int arg;
  arg=addr;


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Reading register "<< hex<<addr  << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    // if (connectControl() == OK){
    if (stopSocket) {
      if  (connectStop() == OK) {
      stopSocket->SendDataOnly(&fnum,sizeof(fnum));
      stopSocket->SendDataOnly(&arg,sizeof(arg));
      stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	stopSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectStop();
      //  if (ret==FORCE_UPDATE)
      //	updateDetector();
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

dacs_t slsDetector::setDAC(dacs_t val, dacIndex index, int mV, int imod){


  dacs_t retval[2];
  retval[0] = -1;
  retval[1] = -1;
  int fnum=F_SET_DAC;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";
  int arg[3];


  if ( (index==HV_NEW) &&((thisDetector->myDetectorType == GOTTHARD) || (thisDetector->myDetectorType == PROPIX)))
    index=HV_POT;

  arg[0]=index;
  arg[1]=imod;
  arg[2]=mV;


#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Setting DAC "<< index << " of module " << imod  <<  " to " << val << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));
      controlSocket->SendDataOnly(&val,sizeof(val));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(retval,sizeof(retval));
	if (index <  thisDetector->nDacs){

	  if (dacs) {
	    if (imod>=0) {
	      *(dacs+index+imod*thisDetector->nDacs)=retval[0];
	    }
	    else {
	      for (imod=0; imod<thisDetector->nModsMax; imod++)
		*(dacs+index+imod*thisDetector->nDacs)=retval[0];
	    }
	  }
	}
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();

    }
  }
#ifdef VERBOSE
  std::cout<< "Dac set to "<< retval[0] << " dac units (" << retval[1] << "mV)" << std::endl;
#endif
  if (ret==FAIL) {
    std::cout<< "Set dac " << index << " of module " << imod  <<  " to " << val << " failed." << std::endl;
  }
  if(mV)
	  return retval[1];

  return retval[0];
};


dacs_t slsDetector::getADC(dacIndex index, int imod){

  dacs_t retval;
  int fnum=F_GET_ADC;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";

  int arg[2];
  arg[0]=index;
  arg[1]=imod;



#ifdef VERBOSE
  std::cout<< std::endl;
  std::cout<< "Getting ADC "<< index << " of module " << imod  <<   std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
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
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";

  int ichan=chan.chan;
  int ichip=chan.chip;
  int imod=chan.module;

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendChannel(&chan);

      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";
  arg[0]=ichan;
  arg[1]=ichip;
  arg[2]=imod;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));

      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	receiveChannel(&myChan);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";

  int ichi=chip.chip;
  int im=chip.module;





  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      sendChip(&chip);
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";


  myChip.chip=ichip;
  myChip.module=imod;
  myChip.nchan=thisDetector->nChans;
  myChip.chanregs=chanreg;

  int arg[2];
  arg[0]=ichip;
  arg[1]=imod;




  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(arg,sizeof(arg));

      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	receiveChip(&myChip);
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
    ret=setModule(myModule,-1,-1,-1,0,0);
  }
  return ret;


};

int slsDetector::setModule(sls_detector_module module, int iodelay, int tau, int e_eV, int* gainval, int* offsetval){

	int fnum=F_SET_MODULE;
	int retval;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	int imod=module.module;


#ifdef VERBOSE
	std::cout << "slsDetector set module " << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			sendModule(&module);

			//not included in module
			if(gainval && (thisDetector->nGain))
				controlSocket->SendDataOnly(gainval,sizeof(int)*thisDetector->nGain);
			if(offsetval && (thisDetector->nOffset))
				controlSocket->SendDataOnly(offsetval,sizeof(int)*thisDetector->nOffset);
			if(thisDetector->myDetectorType == EIGER) {
				controlSocket->SendDataOnly(&iodelay,sizeof(iodelay));
				controlSocket->SendDataOnly(&tau,sizeof(tau));
				controlSocket->SendDataOnly(&e_eV,sizeof(e_eV));
			}


			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				if(strstr(mess,"default tau")!=NULL)
					setErrorMask((getErrorMask())|(RATE_CORRECTION_NO_TAU_PROVIDED));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
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

				if(thisDetector->myDetectorType != JUNGFRAU){
					for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
						if (chipregs)
							chipregs[ichip+thisDetector->nChips*imod]=module.chipregs[ichip];

						if (chanregs) {
							for (int i=0; i<thisDetector->nChans; i++) {
								chanregs[i+ichip*thisDetector->nChans+thisDetector->nChips*thisDetector->nChans*imod]=module.chanregs[ichip*thisDetector->nChans+i];
							}
						}
					}
					if (adcs) {
						for (int i=0; i<thisDetector->nAdcs; i++)
							adcs[i+imod*thisDetector->nAdcs]=module.adcs[i];
					}
				}

				if (dacs) {
					for (int i=0; i<thisDetector->nDacs; i++)
						dacs[i+imod*thisDetector->nDacs]=module.dacs[i];
				}

				(detectorModules+imod)->gain=module.gain;
				(detectorModules+imod)->offset=module.offset;
				(detectorModules+imod)->serialnumber=module.serialnumber;
				(detectorModules+imod)->reg=module.reg;
			}
		}

		if ((thisDetector->nGain) && (gainval) && (gain)) {
			for (int i=0; i<thisDetector->nGain; i++)
				gain[i+imod*thisDetector->nGain]=gainval[i];
		}

		if ((thisDetector->nOffset) && (offsetval) && (offset))  {
			for (int i=0; i<thisDetector->nOffset; i++)
				offset[i+imod*thisDetector->nOffset]=offsetval[i];
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

	int* gainval=0, *offsetval=0;
	if(thisDetector->nGain)
		gainval=new int[thisDetector->nGain];
	if(thisDetector->nOffset)
		offsetval=new int[thisDetector->nOffset];

	//char *ptr,  *goff=(char*)thisDetector;

	// int chanreg[thisDetector->nChans*thisDetector->nChips];
	//int chipreg[thisDetector->nChips];
	//double dac[thisDetector->nDacs], adc[thisDetector->nAdcs];

	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
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
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&imod,sizeof(imod));

			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				receiveModule(myMod);

				//extra gain and offset - eiger
				if(thisDetector->nGain)
					controlSocket->ReceiveDataOnly(gainval,sizeof(int)*thisDetector->nGain);
				if(thisDetector->nOffset)
					controlSocket->ReceiveDataOnly(offsetval,sizeof(int)*thisDetector->nOffset);
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
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

				if(thisDetector->myDetectorType != JUNGFRAU){
					for (int ichip=0; ichip<thisDetector->nChips; ichip++) {
						if (chipregs)
							chipregs[ichip+thisDetector->nChips*imod]=myMod->chipregs[ichip];

						if (chanregs) {
							for (int i=0; i<thisDetector->nChans; i++) {
								chanregs[i+ichip*thisDetector->nChans+thisDetector->nChips*thisDetector->nChans*imod]=myMod->chanregs[ichip*thisDetector->nChans+i];
							}
						}
					}

					if (adcs) {
						for (int i=0; i<thisDetector->nAdcs; i++)
							adcs[i+imod*thisDetector->nAdcs]=myMod->adcs[i];
					}
				}

				if (dacs) {
					for (int i=0; i<thisDetector->nDacs; i++)
						dacs[i+imod*thisDetector->nDacs]=myMod->dacs[i];
				}
				(detectorModules+imod)->gain=myMod->gain;
				(detectorModules+imod)->offset=myMod->offset;
				(detectorModules+imod)->serialnumber=myMod->serialnumber;
				(detectorModules+imod)->reg=myMod->reg;
			}
		}

		if ((thisDetector->nGain) && (gainval) && (gain)) {
			for (int i=0; i<thisDetector->nGain; i++)
				gain[i+imod*thisDetector->nGain]=gainval[i];
		}

		if ((thisDetector->nOffset) && (offsetval) && (offset))  {
			for (int i=0; i<thisDetector->nOffset; i++)
				offset[i+imod*thisDetector->nOffset]=offsetval[i];
		}

	} else {
		deleteModule(myMod);
		myMod=NULL;
	}

	if(gainval) delete[]gainval;
	if(offsetval) delete[]offsetval;

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
  char mess[MAX_STR_LENGTH]="";
#ifdef VERBOSE
  std::cout<< "Getting threshold energy "<< std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
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
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }
  return  thisDetector->currentThresholdEV;
};

int slsDetector::setThresholdEnergy(int e_eV,  int imod, detectorSettings isettings){

	//currently only for eiger
	if (thisDetector->myDetectorType == EIGER) {
		setThresholdEnergyAndSettings(e_eV,isettings);
			return  thisDetector->currentThresholdEV;
	}

	int fnum=  F_SET_THRESHOLD_ENERGY;
	int retval;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
#ifdef VERBOSE
	std::cout<< "Setting threshold energy "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
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
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		thisDetector->currentThresholdEV=e_eV;
	}
	return   thisDetector->currentThresholdEV;
};



int slsDetector::setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings) {

	//if settings provided, use that, else use the shared memory variable
	detectorSettings is = ((isettings != GET_SETTINGS) ? isettings: thisDetector->currentSettings);
	string ssettings;
	switch (is) {
	case STANDARD:
		ssettings="/standard";
		thisDetector->currentSettings=STANDARD;
		break;
	case HIGHGAIN:
		ssettings="/highgain";
		thisDetector->currentSettings=HIGHGAIN;
		break;
	case LOWGAIN:
		ssettings="/lowgain";
		thisDetector->currentSettings=LOWGAIN;
		break;
	case VERYHIGHGAIN:
		ssettings="/veryhighgain";
		thisDetector->currentSettings=VERYHIGHGAIN;
		break;
	case VERYLOWGAIN:
		ssettings="/verylowgain";
		thisDetector->currentSettings=VERYLOWGAIN;
		break;
	default:
		printf("Error: Unknown settings %s for this detector!\n", getDetectorSettings(is).c_str());
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	//verify e_eV exists in trimEneregies[]
	if (!thisDetector->nTrimEn ||
			(e_eV < thisDetector->trimEnergies[0]) ||
			(e_eV > thisDetector->trimEnergies[thisDetector->nTrimEn-1]) ) {
		printf("Error: This energy %d not defined for this module!\n", e_eV);
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	//find if interpolation required
	bool interpolate = true;
	for (int i = 0; i < thisDetector->nTrimEn; ++i) {
		if (thisDetector->trimEnergies[i] == e_eV) {
			interpolate = false;
			break;
		}
	}

	//fill detector module structure
	sls_detector_module *myMod = NULL;
	int iodelay = -1;			//not included in the module
	int tau = -1;			//not included in the module

	//normal
	if(!interpolate) {
		//find their directory names
		ostringstream ostfn;
		ostfn << thisDetector->settingsDir << ssettings << "/" << e_eV << "eV" << "/noise.sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
		string settingsfname = ostfn.str();
#ifdef VERBOSE
		printf("Settings File is %s\n", settingsfname.c_str());
#endif
		//read the files
		myMod=createModule();
		if (NULL == readSettingsFile(settingsfname,thisDetector->myDetectorType, iodelay, tau, myMod)) {
			if(myMod)deleteModule(myMod);
			return FAIL;
		}
	}


	//interpolate
	else {
		//find the trim values
		int trim1 = -1, trim2 = -1;
		for (int i = 0; i < thisDetector->nTrimEn; ++i) {
			if (e_eV < thisDetector->trimEnergies[i]) {
				trim2 = thisDetector->trimEnergies[i];
				trim1 = thisDetector->trimEnergies[i-1];
				break;
			}
		}
		//find their directory names
		ostringstream ostfn;
		ostfn << thisDetector->settingsDir << ssettings << "/" << trim1 << "eV" << "/noise.sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
		string settingsfname1 = ostfn.str();
		ostfn.str(""); ostfn.clear();
		ostfn << thisDetector->settingsDir << ssettings << "/" << trim2 << "eV" << "/noise.sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
		string settingsfname2 = ostfn.str();
		//read the files
#ifdef VERBOSE
		printf("Settings Files are %s and %s\n",settingsfname1.c_str(), settingsfname2.c_str());
#endif
		sls_detector_module *myMod1=createModule();
		sls_detector_module *myMod2=createModule();
		int iodelay1 = -1;			//not included in the module
		int tau1 = -1;			//not included in the module
		int iodelay2 = -1;			//not included in the module
		int tau2 = -1;			//not included in the module
		if (NULL == readSettingsFile(settingsfname1,thisDetector->myDetectorType, iodelay1, tau1, myMod1)) {
			setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		if (NULL == readSettingsFile(settingsfname2,thisDetector->myDetectorType, iodelay2, tau2, myMod2)) {
			setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		if (iodelay1 != iodelay2) {
			printf("iodelays do not match between files\n");
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		iodelay = iodelay1;

		//interpolate  module
		myMod = interpolateTrim(thisDetector->myDetectorType, myMod1, myMod2, e_eV, trim1, trim2);
		if (myMod == NULL) {
			printf("Could not interpolate, different dac values in files\n");
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		}
		//interpolate tau
		tau = linearInterpolation(e_eV, trim1, trim2, tau1, tau2);
		printf("new tau:%d\n",tau);

		deleteModule(myMod1);
		deleteModule(myMod2);
	}


	myMod->module=0;
	myMod->reg=thisDetector->currentSettings;
	setModule(*myMod, iodelay, tau, e_eV, 0, 0);
	deleteModule(myMod);
	if (getSettings(-1) != is){
		std::cout << "Could not set settings in detector" << endl;
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	return OK;
}



/*
  select detector settings
*/
slsDetectorDefs::detectorSettings  slsDetector::getSettings(int imod){


  int fnum=F_SET_SETTINGS;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";
  int  retval;
  int arg[2];
  arg[0]=GET_SETTINGS;
  arg[1]=imod;
#ifdef VERBOSE
  std::cout<< "Getting settings "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
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
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }
  return thisDetector->currentSettings;

};



slsDetectorDefs::detectorSettings slsDetector::setSettings( detectorSettings isettings, int imod){
#ifdef VERBOSE
	std::cout<< "slsDetector setSettings "<< std::endl;
#endif

	//only set client shared memory variable for Eiger, settings threshold loads the module data (trimbits, dacs etc.)
	if (thisDetector->myDetectorType == EIGER) {
		switch(isettings) {
		case STANDARD:
		case HIGHGAIN:
		case LOWGAIN:
		case VERYHIGHGAIN:
		case VERYLOWGAIN:
			thisDetector->currentSettings = isettings;
			break;
		default:
			printf("Unknown settings %s for this detector!\n", getDetectorSettings(isettings).c_str());
		}
		return thisDetector->currentSettings;
	}

	sls_detector_module *myMod=createModule();
	int modmi=imod, modma=imod+1, im=imod;
	string settingsfname, calfname;
	string ssettings;

	//not included in module structure
	int iodelay = -1;
	int tau = -1;
	int* gainval=0, *offsetval=0;
	if(thisDetector->nGain)
		gainval=new int[thisDetector->nGain];
	if(thisDetector->nOffset)
		offsetval=new int[thisDetector->nOffset];


	int ret=0;


	switch (isettings) {
	case STANDARD:
		if (    (thisDetector->myDetectorType == MYTHEN) ||
				(thisDetector->myDetectorType == EIGER)) {
			ssettings="/standard";
			thisDetector->currentSettings=STANDARD;
		}
		break;
	case FAST:
		if (thisDetector->myDetectorType == MYTHEN) {
			ssettings="/fast";
			thisDetector->currentSettings=FAST;
		}
		break;
	case HIGHGAIN:
		if (    (thisDetector->myDetectorType == MYTHEN) ||
				(thisDetector->myDetectorType == GOTTHARD) ||
				(thisDetector->myDetectorType == PROPIX) ||
				(thisDetector->myDetectorType == MOENCH) ||
				(thisDetector->myDetectorType == EIGER)) {
			ssettings="/highgain";
			thisDetector->currentSettings=HIGHGAIN;
		}
		break;
	case DYNAMICGAIN:
		if ((thisDetector->myDetectorType == GOTTHARD) ||
			(thisDetector->myDetectorType == PROPIX) ||
			(thisDetector->myDetectorType == JUNGFRAU) ||
			(thisDetector->myDetectorType == MOENCH)) {
			ssettings="/dynamicgain";
			thisDetector->currentSettings=DYNAMICGAIN;
		}
		break;
	case LOWGAIN:
		if ((thisDetector->myDetectorType == GOTTHARD) ||
			(thisDetector->myDetectorType == PROPIX) ||
			(thisDetector->myDetectorType == MOENCH) || 
		    (thisDetector->myDetectorType == EIGER)   ) {
			ssettings="/lowgain";
			thisDetector->currentSettings=LOWGAIN;
		}
		break;
	case MEDIUMGAIN:
		if ((thisDetector->myDetectorType == GOTTHARD) ||
			(thisDetector->myDetectorType == PROPIX) ||
			(thisDetector->myDetectorType == MOENCH)) {
			ssettings="/mediumgain";
			thisDetector->currentSettings=MEDIUMGAIN;
		}
		break;
	case VERYHIGHGAIN:
		if ((thisDetector->myDetectorType == GOTTHARD) ||
			(thisDetector->myDetectorType == PROPIX) ||
			(thisDetector->myDetectorType == MOENCH)||
			(thisDetector->myDetectorType == EIGER)) {
			ssettings="/veryhighgain";
			thisDetector->currentSettings=VERYHIGHGAIN;
		}
		break;
	case LOWNOISE:
		break;
	case DYNAMICHG0:
		if (thisDetector->myDetectorType == JUNGFRAU) {
			ssettings="/dynamichg0";
			thisDetector->currentSettings=DYNAMICHG0;
		}
		break;
	case FIXGAIN1:
		if (thisDetector->myDetectorType == JUNGFRAU) {
			ssettings="/fixgain1";
			thisDetector->currentSettings=FIXGAIN1;
		}
		break;
	case FIXGAIN2:
		if (thisDetector->myDetectorType == JUNGFRAU) {
			ssettings="/fixgain2";
			thisDetector->currentSettings=FIXGAIN2;
		}
		break;
	case FORCESWITCHG1:
		if (thisDetector->myDetectorType == JUNGFRAU) {
			ssettings="/forceswitchg1";
			thisDetector->currentSettings=FORCESWITCHG1;
		}
		break;
	case FORCESWITCHG2:
		if (thisDetector->myDetectorType == JUNGFRAU) {
			ssettings="/forceswitchg2";
			thisDetector->currentSettings=FORCESWITCHG2;
		}
		break;
	case VERYLOWGAIN:
		if (thisDetector->myDetectorType == EIGER) {
			ssettings="/verylowgain";
			thisDetector->currentSettings=VERYLOWGAIN;
		}
		break;
	default:
		break;
	}


	if (isettings !=  thisDetector->currentSettings) {
		std::cout<< "Unknown settings for this detector!" << std::endl;
	}else{
		if (imod<0) {
			modmi=0;
			//  modma=thisDetector->nModMax[X]*thisDetector->nModMax[Y];
			modma=thisDetector->nMod[X]*thisDetector->nMod[Y];
		}

		for (im=modmi; im<modma; im++) {
			ostringstream ostfn, oscfn;
			myMod->module=im;

			std::cout << std::endl << "Loading settings for module:" << im << std::endl;

			//create file names
			switch(thisDetector->myDetectorType){
			case EIGER:
				//settings is saved in myMod.reg
				myMod->reg=thisDetector->currentSettings;
				ostfn << thisDetector->settingsDir << ssettings <<"/noise.sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
				oscfn << thisDetector->calDir << ssettings << "/calibration.sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
#ifdef VERBOSE
				std::cout<< thisDetector->settingsDir<<endl<< thisDetector->calDir <<endl;
#endif
				break;
			case MOENCH:
			case GOTTHARD:
			case PROPIX:
			case JUNGFRAU:
			case JUNGFRAUCTB:
				//settings is saved in myMod.reg
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


			//settings file****
			settingsfname=ostfn.str();
#ifdef VERBOSE
			cout << "the settings file name is "<<settingsfname << endl;
#endif
			if (NULL == readSettingsFile(settingsfname,thisDetector->myDetectorType, iodelay, tau, myMod)) {
				//if it didnt open, try default settings file
				ostringstream ostfn_default;
				switch(thisDetector->myDetectorType){
				case MOENCH:
				case GOTTHARD:
				case PROPIX:
				case JUNGFRAU:
				case JUNGFRAUCTB:
					ostfn_default << thisDetector->settingsDir << ssettings << ssettings << ".settings";
					break;
				case EIGER:
				default:
					ostfn_default << thisDetector->settingsDir << ssettings << ssettings << ".trim";
					break;
				}
				settingsfname=ostfn_default.str();
#ifdef VERBOSE
				cout << settingsfname << endl;
#endif
				if (NULL == readSettingsFile(settingsfname,thisDetector->myDetectorType, iodelay, tau, myMod)) {
					//if default doesnt work, return error
					std::cout << "Could not open settings file" << endl;
					setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
					return  thisDetector->currentSettings;
				}
			}



			//calibration file****
			if(thisDetector->myDetectorType != EIGER) {
				calfname=oscfn.str();
#ifdef VERBOSE
				cout << "Specific file:"<< calfname << endl;
#endif
				//extra gain and offset
				if(thisDetector->nGain)
					ret = readCalibrationFile(calfname,gainval, offsetval);
				//normal gain and offset inside sls_detector_module
				else
					ret = readCalibrationFile(calfname,myMod->gain, myMod->offset);

				//if it didnt open, try default
				if(ret != OK){
					ostringstream oscfn_default;
					oscfn_default << thisDetector->calDir << ssettings << ssettings << ".cal";
					calfname=oscfn_default.str();
#ifdef VERBOSE
					cout << "Default file:" << calfname << endl;
#endif
					//extra gain and offset
					if(thisDetector->nGain)
						ret = readCalibrationFile(calfname,gainval, offsetval);
					//normal gain and offset inside sls_detector_module
					else
						ret = readCalibrationFile(calfname,myMod->gain, myMod->offset);
				}
				//if default doesnt work, return error
				if(ret != OK){
					std::cout << "Could not open calibration file" << calfname << endl;
					setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
					return  thisDetector->currentSettings;
				}
			}

			//if everything worked, set module****
			setModule(*myMod,iodelay,tau,-1,gainval,offsetval);
		}
	}



	deleteModule(myMod);
	if(gainval)	delete [] gainval;
	if(offsetval) delete [] offsetval;

	switch(thisDetector->myDetectorType==MYTHEN){
	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
		int isett=getSettings(imod);
		double t[]=defaultTDead;
		if (isett>-1 && isett<3) {
			thisDetector->tDead=t[isett];
		}
	}
	}

	if (getSettings(imod) != isettings){
		std::cout << "Could not set settings" << endl;
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
	}

	return  thisDetector->currentSettings;
};



int slsDetector::getChanRegs(double* retval,bool fromDetector){
  int n=getTotalNumberOfChannels();
  if(fromDetector){
    for(int im=0;im<setNumberOfModules();im++)
      getModule(im);
  }
  //the original array has 0 initialized
  if(chanregs){
    for (int i=0; i<n; i++)
      retval[i] = (double) (chanregs[i] & TRIMBITMASK);
  }
  return n;
}



int slsDetector::updateDetectorNoWait() {

  enum readOutFlags ro;
  // int ret=OK;
  enum detectorSettings t;
  int thr, n = 0, nm;
  // int it;
  int64_t retval;// tns=-1;
  char lastClientIP[INET_ADDRSTRLEN];

  n += 	controlSocket->ReceiveDataOnly(lastClientIP,sizeof(lastClientIP));
#ifdef VERBOSE
  cout << "Updating detector last modified by " << lastClientIP << std::endl;
#endif
  n += 	controlSocket->ReceiveDataOnly(&nm,sizeof(nm));
  thisDetector->nMod[X]=nm;
  n += 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
  /// Should be overcome at a certain point!

  if (thisDetector->myDetectorType==MYTHEN) {
    thisDetector->nModMax[X]=nm;
    thisDetector->nModMax[Y]=1;
    thisDetector->nModsMax=thisDetector->nModMax[Y]*thisDetector->nModMax[X];
    thisDetector->nMod[Y]=1;
  } else {
    thisDetector->nMod[Y]=nm;
  }

  thisDetector->nMods=thisDetector->nMod[Y]*thisDetector->nMod[X];
  if (thisDetector->nModsMax<thisDetector->nMods)
    thisDetector->nModsMax=thisDetector->nMods;

  if (thisDetector->nModMax[X]<thisDetector->nMod[X])
    thisDetector->nModMax[X]=thisDetector->nMod[X];

  if (thisDetector->nModMax[Y]<thisDetector->nMod[Y])
    thisDetector->nModMax[Y]=thisDetector->nMod[Y];

  n += 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
  thisDetector->dynamicRange=nm;

  n += 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
  thisDetector->dataBytes=nm;
  //t=setSettings(GET_SETTINGS);
  n += 	controlSocket->ReceiveDataOnly( &t,sizeof(t));
  thisDetector->currentSettings=t;

  if((thisDetector->myDetectorType!= GOTTHARD)&&
		  (thisDetector->myDetectorType!= PROPIX)&&
		  (thisDetector->myDetectorType!= JUNGFRAU)&&
		  (thisDetector->myDetectorType!= MOENCH) && (thisDetector->myDetectorType!= JUNGFRAUCTB)){
    //thr=getThresholdEnergy();
    n += 	controlSocket->ReceiveDataOnly( &thr,sizeof(thr));
    thisDetector->currentThresholdEV=thr;
  }

  //retval=setFrames(tns);
  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
  thisDetector->timerValue[FRAME_NUMBER]=retval;
  // retval=setExposureTime(tns);
  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
  thisDetector->timerValue[ACQUISITION_TIME]=retval;

  if(thisDetector->myDetectorType == EIGER){
	   //retval=setSubFrameExposureTime(tns);
	  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	  thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME]=retval;
  }
  //retval=setPeriod(tns);
  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
  thisDetector->timerValue[FRAME_PERIOD]=retval;
  //retval=setDelay(tns);
  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
  thisDetector->timerValue[DELAY_AFTER_TRIGGER]=retval;
  // retval=setGates(tns);
  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
  thisDetector->timerValue[GATES_NUMBER]=retval;

  //retval=setProbes(tns);
  if (thisDetector->myDetectorType == MYTHEN){
    n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
    thisDetector->timerValue[PROBES_NUMBER]=retval;
  }


  //retval=setTrains(tns);
  n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
  thisDetector->timerValue[CYCLES_NUMBER]=retval;

  //retval=setProbes(tns);
  if (thisDetector->myDetectorType == JUNGFRAUCTB){
     n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
     if (retval>=0)
       thisDetector->timerValue[SAMPLES_JCTB]=retval;
     n += controlSocket->ReceiveDataOnly( &ro,sizeof(ro));

     thisDetector->roFlags=ro;

     //retval=setProbes(tns);
     getTotalNumberOfChannels();
    
     //    thisDetector->dataBytes=getTotalNumberOfChannels()*thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
     
  }


  if (!n)
	  printf("n: %d\n", n);

  return OK;

}




int slsDetector::updateDetector() {
  int fnum=F_UPDATE_CLIENT;
  int ret=OK;
  char mess[MAX_STR_LENGTH]="";

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      } else
	updateDetectorNoWait();
      disconnectControl();
    }
  }
  return ret;
}



// Acquisition functions
/* change these funcs accepting also ok/fail */

int slsDetector::startAcquisition(){


  int fnum=F_START_ACQUISITION;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<< "Starting acquisition "<< std::endl;
#endif
  thisDetector->stoppedFlag=0;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }
  return ret;



};
int slsDetector::stopAcquisition(){

  int fnum=F_STOP_ACQUISITION;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<< "Stopping acquisition "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
      if  (connectStop() == OK) {
	stopSocket->SendDataOnly(&fnum,sizeof(fnum));
	stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  stopSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	}
	disconnectStop();
      }
    }
  }
  thisDetector->stoppedFlag=1;
  return ret;


};

int slsDetector::startReadOut(){

  int fnum=F_START_READOUT;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<< "Starting readout "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }
  return ret;
};


slsDetectorDefs::runStatus slsDetector::getRunStatus(){
  int fnum=F_GET_RUN_STATUS;
  int ret=FAIL;
  char mess[MAX_STR_LENGTH]="";
  strcpy(mess,"aaaaa");
  runStatus retval=ERROR;
#ifdef VERBOSE
  std::cout<< "Getting status "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
      if  (connectStop() == OK) {
	stopSocket->SendDataOnly(&fnum,sizeof(fnum));
	stopSocket->ReceiveDataOnly(&ret,sizeof(ret));

		//cout << "________:::____________" << ret << endl;

	if (ret==FAIL) {
	  stopSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  	//cout << "____________________" << retval << endl;
	}
	disconnectStop();
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
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      retval=getDataFromDetector();
      if (retval) {
	dataQueue.push(retval);
	disconnectControl();
      }
    }
  }
  return retval;
};




int* slsDetector::getDataFromDetector(int *retval){
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="Nothing";
	int nel=thisDetector->dataBytes/sizeof(int);
	int n;
	int *r=retval;

	int nodatadetectortype = false;
	detectorType types = getDetectorsType();
	if(types == EIGER || types == JUNGFRAU){
		nodatadetectortype = true;
	}


	if (!nodatadetectortype && retval==NULL)
		retval=new int[nel];


#ifdef VERBOSE
	std::cout<< "getting data "<< thisDetector->dataBytes << " " << nel<< std::endl;
#endif
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
#ifdef VERBOSE
	cout << "ret=" << ret << endl;
#endif

	if (ret!=OK) {
		n= controlSocket->ReceiveDataOnly(mess,sizeof(mess));
		// if(thisDetector->receiverOnlineFlag == OFFLINE_FLAG)
		if (ret==FAIL) {
			thisDetector->stoppedFlag=1;
			std::cout<< "Detector returned: " << mess << " " << n << std::endl;
		} else {
			;
#ifdef VERBOSE
			std::cout<< "Detector successfully returned: " << mess << " " << n << std::endl;
#endif
		}
		if ((!nodatadetectortype) && (r==NULL)){
			delete [] retval;
		}
		return NULL;
	} else 	if (!nodatadetectortype){

		n=controlSocket->ReceiveDataOnly(retval,thisDetector->dataBytes);

#ifdef VERBOSE
		std::cout<< "Received "<< n << " data bytes" << std::endl;
#endif
		if (n!=thisDetector->dataBytes) {
			std::cout<< "wrong data size received from detector: received " << n << " but expected " << thisDetector->dataBytes << std::endl;
			thisDetector->stoppedFlag=1;
			ret=FAIL;
			if (r==NULL) {
				delete [] retval;
			}
			return NULL;
		}
		// for (int ib=0; ib<thisDetector->dataBytes/8; ib++)
		//   cout << ((*(((u_int64_t*)retval)+ib))>>17&1) ;


	}
	//	cout << "get data returning " << endl;
	//	cout << endl;
	return retval;

};






int* slsDetector::readAll(){

  int fnum=F_READ_ALL;
  int* retval; // check what we return!

#ifdef VERBOSE
  int i=0;
  std::cout<< "Reading all frames "<< std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));

      while ((retval=getDataFromDetector())){
#ifdef VERBOSE
	i++;
	std::cout<< i << std::endl;
#endif
	dataQueue.push(retval);
      }
      disconnectControl();
    }
  }
#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
#endif
  return dataQueue.front(); // check what we return!

};





int slsDetector::readAllNoWait(){

  int fnum= F_READ_ALL;

#ifdef VERBOSE
  std::cout<< "Reading all frames "<< std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      return OK;
    }
  }
  return FAIL;
};








int* slsDetector::startAndReadAll(){
  //cout << "Start and read all "<< endl;

  int* retval;
  //#ifdef VERBOSE
#ifdef VERBOSE
  int i=0;
#endif
  //#endif
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
    
    //std::cout<< "pushed" << std::endl;
  }
  disconnectControl();

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
    if (connectControl() == OK){
      //std::cout<< "connected" << std::endl;
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      return OK;
    }
  }
  return FAIL;
};

// int* slsDetector::getDataFromDetectorNoWait() {
//   int *retval=getDataFromDetector();
//   if (thisDetector->onlineFlag==ONLINE_FLAG) {
//     if (controlSocket) {
//       if (retval==NULL){
// 	disconnectControl();

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


	int fnum=F_SET_TIMER,fnum2=F_SET_RECEIVER_TIMER;
	int64_t retval = -1;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

	if (index!=MEASUREMENTS_NUMBER) {


#ifdef VERBOSE
		std::cout<< "Setting timer "<< index << " to " <<  t << "ns/value" << std::endl;
#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&index,sizeof(index));
				controlSocket->SendDataOnly(&t,sizeof(t));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(DETECTOR_TIMER_VALUE_NOT_SET));
				} else {
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
					thisDetector->timerValue[index]=retval;
				}
				disconnectControl();
				if (ret==FORCE_UPDATE) {
					updateDetector();
#ifdef VERBOSE
					std::cout<< "Updated!" << std::endl;
#endif

				}
			}
		} else {
			//std::cout<< "offline " << std::endl;
			if (t>=0)
				thisDetector->timerValue[index]=t;
			if((thisDetector->myDetectorType==GOTTHARD)||
			   (thisDetector->myDetectorType==PROPIX)||
			   (thisDetector->myDetectorType==JUNGFRAU)||
			   (thisDetector->myDetectorType==MOENCH))
			  thisDetector->timerValue[PROBES_NUMBER]=0;
			if(thisDetector->myDetectorType==JUNGFRAUCTB && index==SAMPLES_JCTB) {
			  getTotalNumberOfChannels();
			  //			  thisDetector->dataBytes=getTotalNumberOfChannels()*thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
			}
				
		}
	} else {
		if (t>=0)
			thisDetector->timerValue[index]=t;
	}
#ifdef VERBOSE
	std::cout<< "Timer " << index << " set to  "<< thisDetector->timerValue[index] << "ns"  << std::endl;
#endif

	if ((thisDetector->myDetectorType==MYTHEN)&&(index==PROBES_NUMBER)) {
	  setDynamicRange();
	  //cout << "Changing probes: data size = " << thisDetector->dataBytes <<endl;
	}
	if ((thisDetector->myDetectorType==JUNGFRAUCTB) && (index==SAMPLES_JCTB)) {
	  setDynamicRange();
	  cout << "Changing samples: data size = " << thisDetector->dataBytes <<endl;
	}
	
	/* set progress */
	if ((index==FRAME_NUMBER) || (index==CYCLES_NUMBER)) {
	  setTotalProgress();
	}

	if(t!=-1){
		if ((thisDetector->myDetectorType==MYTHEN)&&(index==PROBES_NUMBER)) {
			setDynamicRange();
			//cout << "Changing probes: data size = " << thisDetector->dataBytes <<endl;
		}

		/* set progress */
		if ((index==FRAME_NUMBER) || (index==CYCLES_NUMBER)) {
			setTotalProgress();
		}

		//if eiger, rate corr on, a put statement, dr=32 &setting subexp or dr =16 & setting exptime, set ratecorr to update table
		double r;
		if( (thisDetector->myDetectorType == EIGER) &&
				getRateCorrection(r) &&
				(t>=0) &&

				(((index == SUBFRAME_ACQUISITION_TIME) && (thisDetector->dynamicRange == 32))||
						((index == ACQUISITION_TIME) && (thisDetector->dynamicRange == 16)))

						&& (t>=0) && getRateCorrection(r)){
			setRateCorrection(r);
		}
	}




	//send acquisiton period/frame number to receiver
	if((index==FRAME_NUMBER)||(index==FRAME_PERIOD)||(index==CYCLES_NUMBER)||(index==ACQUISITION_TIME)){
		if(ret != FAIL){
			int64_t args[2];
			retval = -1;
			args[0] = index;
			args[1] = thisDetector->timerValue[index];

			if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){

				//set #frames, #cycles
				if((index==FRAME_NUMBER)||(index==CYCLES_NUMBER)){
#ifdef VERBOSE
					std::cout << "Setting/Getting number of frames*cycles " << index <<" to/from receiver " << args[1] << std::endl;
#endif
					if(thisDetector->timerValue[CYCLES_NUMBER]==0)
						args[1] = thisDetector->timerValue[FRAME_NUMBER];
					else
						args[1] = thisDetector->timerValue[FRAME_NUMBER]*thisDetector->timerValue[CYCLES_NUMBER];
				}
				//set period/exptime
				else{
#ifdef VERBOSE
					if(index==ACQUISITION_TIME)
						std::cout << "Setting/Getting acquisition time " << index << " to/from receiver " << args[1] << std::endl;
					else
						std::cout << "Setting/Getting acquisition period " << index << " to/from receiver " << args[1] << std::endl;
#endif
				}

				char mess[MAX_STR_LENGTH]="";
				if (connectData() == OK){
					ret=thisReceiver->sendIntArray(fnum2,retval,args,mess);
					disconnectData();
				}
				if((args[1] != retval)|| (ret==FAIL)){
					ret = FAIL;
					if(index==ACQUISITION_TIME){
						if(strstr(mess,"receiver not idle")==NULL)
							cout << "ERROR:Acquisition Time in receiver set incorrectly to " << retval << " instead of " << args[1] << endl;
						setErrorMask((getErrorMask())|(RECEIVER_ACQ_TIME_NOT_SET));
					}else if(index==FRAME_PERIOD){
						if(strstr(mess,"receiver not idle")==NULL)
							cout << "ERROR:Acquisition Period in receiver set incorrectly to " << retval << " instead of " << args[1] << endl;
						setErrorMask((getErrorMask())|(RECEIVER_ACQ_PERIOD_NOT_SET));
					}else{
						if(strstr(mess,"receiver not idle")==NULL)
							cout << "ERROR:Number of Frames (* Number of cycles) in receiver set incorrectly to " << retval << " instead of " << args[1] << endl;
						setErrorMask((getErrorMask())|(RECEIVER_FRAME_NUM_NOT_SET));
					}
				}
				if(ret==FORCE_UPDATE)
					updateReceiver();
			}
		}
	}
	return thisDetector->timerValue[index];
};

int slsDetector::lockServer(int lock) {
  int fnum=F_LOCK_SERVER;
  int retval=-1;
  int ret=OK;
  char mess[MAX_STR_LENGTH]="";

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&lock,sizeof(lock));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      } else {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return retval;


}


string slsDetector::getLastClientIP() {

  int fnum=F_GET_LAST_CLIENT_IP;
  char clientName[INET_ADDRSTRLEN];
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      } else {
	controlSocket->ReceiveDataOnly(clientName,sizeof(clientName));
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return string(clientName);

}


int slsDetector::setPort(portType index, int num){

	int fnum=F_SET_PORT, fnum2 = F_SET_RECEIVER_PORT;
	int retval;
	//  uint64_t ut;
	char mess[MAX_STR_LENGTH]="";
	int ret=FAIL;
	bool online=false;
	MySocketTCP *s;

	if (num>1024) {
		switch(index) {
		case CONTROL_PORT:
			s=controlSocket;
			retval=thisDetector->controlPort;
#ifdef VERBOSE
			cout << "s="<< s<< endl;
			cout << thisDetector->controlPort<< " " << " " << thisDetector->stopPort << endl;
#endif
			if (s==NULL) {

#ifdef VERBOSE
				cout << "s=NULL"<< endl;
				cout << thisDetector->controlPort<< " " << " " << thisDetector->stopPort << endl;
#endif
				setTCPSocket("",DEFAULT_PORTNO);
			}
			if (controlSocket) {
				s=controlSocket;
			} else {
#ifdef VERBOSE
				cout << "still cannot connect!"<< endl;
				cout << thisDetector->controlPort<< " " << " " << thisDetector->stopPort << endl;
#endif

				setTCPSocket("",retval);
			}
			online =  (thisDetector->onlineFlag==ONLINE_FLAG);

			//not an error.could be from config file
			if(num==thisDetector->controlPort)
				return thisDetector->controlPort;
			//reusable port, so print error
			else if((num==thisDetector->stopPort)||(num==thisDetector->receiverTCPPort)){
				std::cout<< "Can not connect to port in use " << std::endl;
				setErrorMask((getErrorMask())|(COULDNOT_SET_CONTROL_PORT));
				return thisDetector->controlPort;
			}
			break;


		case DATA_PORT:
			s=dataSocket;
			retval=thisDetector->receiverTCPPort;
			if(strcmp(thisDetector->receiver_hostname,"none")){
				if (s==NULL) setReceiverTCPSocket("",retval);
				if (dataSocket)s=dataSocket;
			}
			online =  (thisDetector->receiverOnlineFlag==ONLINE_FLAG);

			//not an error. could be from config file
			if(num==thisDetector->receiverTCPPort)
				return thisDetector->receiverTCPPort;
			//reusable port, so print error
			else if((num==thisDetector->stopPort)||(num==thisDetector->controlPort)){
				std::cout<< "Can not connect to port in use " << std::endl;
				setErrorMask((getErrorMask())|(COULDNOT_SET_DATA_PORT));
				return thisDetector->receiverTCPPort;
			}
			break;


		case STOP_PORT:
			s=stopSocket;
			retval=thisDetector->stopPort;
			if (s==NULL) setTCPSocket("",-1,DEFAULT_PORTNO+1);
			if (stopSocket) s=stopSocket;
			else setTCPSocket("",-1,retval);
			online =  (thisDetector->onlineFlag==ONLINE_FLAG);

			//not an error. could be from config file
			if(num==thisDetector->stopPort)
				return thisDetector->stopPort;
			//reusable port, so print error
			else if((num==thisDetector->receiverTCPPort)||(num==thisDetector->controlPort)){
				std::cout<< "Can not connect to port in use " << std::endl;
				setErrorMask((getErrorMask())|(COULDNOT_SET_STOP_PORT));
				return thisDetector->stopPort;
			}
			break;

		default:
			s=NULL;
			break;
		}




		//send to current port to change port
		if (online) {
			if (s) {
				if  (s->Connect()>=0) {
					if(s==dataSocket)
						fnum = fnum2;
					s->SendDataOnly(&fnum,sizeof(fnum));
					s->SendDataOnly(&index,sizeof(index));
					s->SendDataOnly(&num,sizeof(num));
					s->ReceiveDataOnly(&ret,sizeof(ret));
					if (ret==FAIL) {
						s->ReceiveDataOnly(mess,sizeof(mess));
						std::cout<< "Detector returned error: " << mess << std::endl;
					} else {
						s->ReceiveDataOnly(&retval,sizeof(retval));
					}
					s->Disconnect();
				}else{
				  if (index == CONTROL_PORT){
				    std::cout << "cannot connect to detector" << endl;
				    setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
				  }else if (index == DATA_PORT){
				    std::cout << "cannot connect to receiver" << endl;
				    setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
				  }
				}
			}
		}



		if (ret!=FAIL) {
			switch(index) {
			case CONTROL_PORT:
				thisDetector->controlPort=retval;
				break;
			case DATA_PORT:
				if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
					thisDetector->receiverTCPPort=retval;
					setReceiverOnline(ONLINE_FLAG);
					setReceiver(thisDetector->receiver_hostname);
				}
				break;
			case STOP_PORT:
				thisDetector->stopPort=retval;
				break;
			default:
				break;
			}
#ifdef VERBOSE
			cout << "ret is ok" << endl;
#endif

		} else {
			switch(index) {
			case CONTROL_PORT:
				thisDetector->controlPort=num;
				setErrorMask((getErrorMask())|(COULDNOT_SET_CONTROL_PORT));
				break;
			case DATA_PORT:
				if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
					thisDetector->receiverTCPPort=retval;
					setErrorMask((getErrorMask())|(COULDNOT_SET_DATA_PORT));
				}else{
					thisDetector->receiverTCPPort=num;
					if(strcmp(thisDetector->receiver_hostname,"none"))
						setReceiver(thisDetector->receiver_hostname);
				}
				break;
			case STOP_PORT:
				thisDetector->stopPort=num;
				setErrorMask((getErrorMask())|(COULDNOT_SET_STOP_PORT));
				break;
			default:
				break;
			}
		}
	}




	switch(index) {
	case CONTROL_PORT:
		retval=thisDetector->controlPort;
		break;
	case DATA_PORT:
		retval=thisDetector->receiverTCPPort;
		break;
	case STOP_PORT:
		retval=thisDetector->stopPort;
		break;
	default:
		retval=-1;
		break;
	}



#ifdef VERBOSE
	cout << thisDetector->controlPort<< " " << thisDetector->receiverTCPPort << " " << thisDetector->stopPort << endl;
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
    nm=timerValue[MEASUREMENTS_NUMBER];


  if ((thisDetector->nScanSteps[0]>0) && (thisDetector->actionMask & (1 << MAX_ACTIONS)))
    nscan[0]=thisDetector->nScanSteps[0];

  if ((thisDetector->nScanSteps[1]>0) && (thisDetector->actionMask & (1 << (MAX_ACTIONS+1))))
    nscan[1]=thisDetector->nScanSteps[1];

  thisDetector->totalProgress=nf*nc*npos*nm*nscan[0]*nscan[1];

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
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting speed  variable"<< sp << " to " <<  value << std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&sp,sizeof(sp));
      controlSocket->SendDataOnly(&value,sizeof(value));
#ifdef VERBOSE
      std::cout<< "Sent  "<< n << " bytes "  << std::endl;
#endif
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
	setErrorMask((getErrorMask())|(COULD_NOT_SET_SPEED_PARAMETERS));
      } else {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Getting  timer  "<< index <<  std::endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
      if  (connectStop() == OK) {
	stopSocket->SendDataOnly(&fnum,sizeof(fnum));
	stopSocket->SendDataOnly(&index,sizeof(index));
	stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
	  stopSocket->ReceiveDataOnly(mess,sizeof(mess));
	  std::cout<< "Detector returned error: " << mess << std::endl;
	} else {
	  stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
	}
	disconnectStop();
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
  int fnum=F_SET_DYNAMIC_RANGE,fnum2=F_SET_RECEIVER_DYNAMIC_RANGE;
  int retval=-1,retval1;
  char mess[MAX_STR_LENGTH]="";
  int ret=OK, rateret=OK;

#ifdef VERBOSE
  std::cout<< "Setting dynamic range to "<< n << std::endl;
#endif
  if ((thisDetector->myDetectorType == MYTHEN) &&(n==24))
    n=32;



  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&n,sizeof(n));
      //rate correction is switched off if not 32 bit mode
      if(thisDetector->myDetectorType == EIGER){
    	  controlSocket->ReceiveDataOnly(&rateret,sizeof(rateret));
    	  if (rateret==FAIL) {
    		  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
    		  std::cout<< "Detector returned error: " << mess << std::endl;
    		  if(strstr(mess,"Rate Correction")!=NULL){
    			  if(strstr(mess,"32")!=NULL)
    				  setErrorMask((getErrorMask())|(RATE_CORRECTION_NOT_32or16BIT));
    			  else
    				  setErrorMask((getErrorMask())|(COULD_NOT_SET_RATE_CORRECTION));
    		  }
    	  }
      }
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      } else {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  } else if(thisDetector->myDetectorType==MYTHEN){
    if (n>0)
      thisDetector->dynamicRange=n;
    retval=thisDetector->dynamicRange;
  }
  //cout << "detector returned dynamic range " << retval << endl;
  if (ret!=FAIL && retval>0) {
    /* checking the number of probes to chose the data size */



    thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*retval/8;

    if (thisDetector->myDetectorType==JUNGFRAUCTB) {
      // thisDetector->nChip[X]=retval/16;
      // thisDetector->nChips=thisDetector->nChip[X]*thisDetector->nChip[Y];
      // cout << thisDetector->nMod[X]*thisDetector->nMod[Y] << " " << thisDetector->nChans*thisDetector->nChips << " " << retval<< " ";
      getTotalNumberOfChannels();
      //thisDetector->dataBytes=getTotalNumberOfChannels()*retval/8*thisDetector->timerValue[SAMPLES_JCTB];
      //cout << "data bytes: "<< thisDetector->dataBytes << endl;
    } 
    if(thisDetector->myDetectorType==MYTHEN){
      if (thisDetector->timerValue[PROBES_NUMBER]!=0)
	thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
      
      if (retval==32)
	thisDetector->dynamicRange=24;
    }


    thisDetector->dynamicRange=retval;


#ifdef VERBOSE
    std::cout<< "Dynamic range set to  "<< thisDetector->dynamicRange   << std::endl;
    std::cout<< "Data bytes "<< thisDetector->dataBytes   << std::endl;
#endif
  }


  //receiver
  if(ret != FAIL){
	  retval = thisDetector->dynamicRange;
	  if((n==-1) && (ret!= FORCE_UPDATE)) n =-1;
	  if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			  std::cout << "Sending/Getting dynamic range to/from receiver " << n << std::endl;
#endif
			  if (connectData() == OK){
				  ret=thisReceiver->sendInt(fnum2,retval1,n);
				  disconnectData();
			  }
		  if ((ret==FAIL) || (retval1 != retval)){
			  ret = FAIL;
			  cout << "ERROR:Dynamic range in receiver set incorrectly to " << retval1 << " instead of " << retval << endl;
			  setErrorMask((getErrorMask())|(RECEIVER_DYNAMIC_RANGE));
		  }
		  if(ret==FORCE_UPDATE)
			  updateReceiver();
	  }
  }

  return thisDetector->dynamicRange;
};




int slsDetector::setROI(int n,ROI roiLimits[]){
	int ret = FAIL;
	//sort ascending order
	int temp;

	for(int i=0;i<n;i++){
	  
	  //	  cout << "*** ROI "<< i << " xmin " << roiLimits[i].xmin << " xmax "<< roiLimits[i].xmax << endl;
		for(int j=i+1;j<n;j++){
			if(roiLimits[j].xmin<roiLimits[i].xmin){
	
			  temp=roiLimits[i].xmin;roiLimits[i].xmin=roiLimits[j].xmin;roiLimits[j].xmin=temp;

			  temp=roiLimits[i].xmax;roiLimits[i].xmax=roiLimits[j].xmax;roiLimits[j].xmax=temp;

			  temp=roiLimits[i].ymin;roiLimits[i].ymin=roiLimits[j].ymin;roiLimits[j].ymin=temp;

			  temp=roiLimits[i].ymax;roiLimits[i].ymax=roiLimits[j].ymax;roiLimits[j].ymax=temp;
			}
		}
		//	cout << "UUU ROI "<< i << " xmin " << roiLimits[i].xmin << " xmax "<< roiLimits[i].xmax  << endl;
	}

	ret = sendROI(n,roiLimits);
	if(ret==FAIL)
		setErrorMask((getErrorMask())|(COULDNOT_SET_ROI));

	
	if(thisDetector->myDetectorType==JUNGFRAUCTB) getTotalNumberOfChannels();
	return ret;
}


slsDetectorDefs::ROI* slsDetector::getROI(int &n){
  sendROI(-1,NULL);
  n=thisDetector->nROI;
  if(thisDetector->myDetectorType==JUNGFRAUCTB) getTotalNumberOfChannels();
  return thisDetector->roiLimits;
}


int slsDetector::sendROI(int n,ROI roiLimits[]){
  int ret=FAIL;
  int fnum=F_SET_ROI;
  char mess[MAX_STR_LENGTH]="";
  int arg = n;
  int retvalsize=0;
  ROI retval[MAX_ROIS];
  int nrec=-1;
  if (roiLimits==NULL)
    roiLimits=thisDetector->roiLimits;

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&arg,sizeof(arg));
      if(arg==-1){;
#ifdef VERBOSE
	cout << "Getting ROI from detector" << endl;
#endif
      }else{
#ifdef VERBOSE
	cout << "Sending ROI of size " << arg << " to detector" << endl;
#endif
	controlSocket->SendDataOnly(roiLimits,arg*sizeof(ROI));
      }
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));

      if (ret!=FAIL){
	controlSocket->ReceiveDataOnly(&retvalsize,sizeof(retvalsize));
	nrec = controlSocket->ReceiveDataOnly(retval,retvalsize*sizeof(ROI));
	if(nrec!=(retvalsize*(int)sizeof(ROI))){
	  ret=FAIL;
	  std::cout << " wrong size received: received " << nrec << "but expected " << retvalsize*sizeof(ROI) << endl;
	}
      }else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  //update client
  if(ret!=FAIL){
    for(int i=0;i<retvalsize;i++)
      thisDetector->roiLimits[i]=retval[i];
    thisDetector->nROI = retvalsize;
  }

  //#ifdef VERBOSE
  for(int j=0;j<thisDetector->nROI;j++)
    cout<<"get"<< roiLimits[j].xmin<<"\t"<<roiLimits[j].xmax<<"\t"<<roiLimits[j].ymin<<"\t"<<roiLimits[j].ymax<<endl;
  //#endif

  return ret;
}



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
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting readout flags to "<< flag << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&flag,sizeof(flag));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
	setErrorMask((getErrorMask())|(COULD_NOT_SET_READOUT_FLAGS));
      } else {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	thisDetector->roFlags=retval;
	if (thisDetector->myDetectorType==JUNGFRAUCTB) {

	  getTotalNumberOfChannels();
	  //thisDetector->dataBytes=getTotalNumberOfChannels()*thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
	}
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;
  int arg[3];
  arg[0]=imod;
  arg[1]=par1;
  arg[2]=par2;


#ifdef VERBOSE
  std::cout<< "Trimming module " << imod << " with mode "<< mode << " parameters " << par1 << " " << par2 << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
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
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }
  return retval;

};

double* slsDetector::decodeData(int *datain, int &nn, double *fdata) {


  double *dataout;
  if (fdata) {
    dataout=fdata;
    nn=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
    //    printf("not allocating fdata!\n");
    if (thisDetector->myDetectorType==JUNGFRAUCTB) nn=thisDetector->dataBytes/2;
  } else { 
    if (thisDetector->myDetectorType==JUNGFRAUCTB) {
      nn=thisDetector->dataBytes/2;
      dataout=new double[nn];
      
      //  std::cout<< "nn is "<< nn  << std::endl;
    }    else {
      dataout=new double[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
      nn=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
    }
    
    //  printf("allocating fdata!\n");
  }
 // const int bytesize=8;
  
  int ival=0;
  char *ptr=(char*)datain;
  char iptr;
  
  int nbits=thisDetector->dynamicRange;
  int nch=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
  int  ipos=0, ichan=0, ibyte;
  
  if (thisDetector->timerValue[PROBES_NUMBER]==0) {
    if (thisDetector->myDetectorType==JUNGFRAUCTB) {
      
      for (ichan=0; ichan<nn; ichan++) {
	//   //	}
	dataout[ichan]=*((u_int16_t*)ptr);
	ptr+=2;
      }

      std::cout<< "decoded "<< ichan << " channels" << std::endl;		
    } else {
			switch (nbits) {
			case 1:
				for (ibyte=0; ibyte<thisDetector->dataBytes; ibyte++) {
				  iptr=ptr[ibyte];//&0x1;
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
					iptr=ptr[ibyte];
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
					// ival=0;
					// for (ibyte=0; ibyte<2; ibyte++) {
					// 	iptr=ptr[ichan*2+ibyte];
					// 	ival|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));
					// }
					dataout[ichan]=*((u_int16_t*)ptr);
					ptr+=2;
				}
				break;
			default:
			  int mask=0xffffffff;
			  if(thisDetector->myDetectorType == MYTHEN) mask=0xffffff;
			  for (ichan=0; ichan<nch; ichan++) {
			    dataout[ichan]=datain[ichan]&mask;
			  }
			}
    }
  } else {
    for (ichan=0; ichan<nch; ichan++) {
      dataout[ichan]=datain[ichan];
    }
  }


    //#ifdef VERBOSE
	//#endif

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
  //double xmed[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
 // int nmed=0;
  int im=0;
  int nch;
  thisDetector->nBadFF=0;

  char ffffname[MAX_STR_LENGTH*2];
  if (fname=="default") {
    fname=string(thisDetector->flatFieldFile);
  }

  if (fname=="") {
#ifdef VERBOSE
    std::cout<< "disabling flat field correction" << std::endl;
#endif
    thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);



  } else {
#ifdef VERBOSE
    std::cout<< "Setting flat field correction from file " << fname << std::endl;
#endif


    sprintf(ffffname,"%s/%s",thisDetector->flatFieldDir,fname.c_str());
    nch=readDataFile(string(ffffname),data);
    if (nch>0) {

      //???? bad ff chans?
      int nm=getNMods();
      int chpm[nm];
      int mMask[nm];
      for (int i=0; i<nm; i++) {
	chpm[im]=getChansPerMod(im);
	mMask[im]=im;
      }
      fillModuleMask(mMask);

      if ((postProcessingFuncs::calculateFlatField(&nm, chpm, mMask, badChannelMask, data, ffcoefficients, fferrors))>=0) {
	strcpy(thisDetector->flatFieldFile,fname.c_str());


	thisDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);

	setFlatFieldCorrection(ffcoefficients, fferrors);

      }
    }  else {
      std::cout<< "Flat field from file " << fname << " is not valid " << nch << std::endl;

    }
  }

  return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);

}

int slsDetector::fillModuleMask(int *mM){
  if (mM)
    for (int i=0; i<getNMods(); i++)
      mM[i]=i;

  return getNMods();
}


int slsDetector::setFlatFieldCorrection(double *corr, double *ecorr) {
  if (corr!=NULL) {
    for (int ichan=0; ichan<thisDetector->nMod[Y]*thisDetector->nMod[X]*thisDetector->nChans*thisDetector->nChips; ichan++) {
      // #ifdef VERBOSE
      //       std::cout<< ichan << " "<< corr[ichan] << std::endl;
      // #endif
      ffcoefficients[ichan]=corr[ichan];
      if (ecorr!=NULL)
	fferrors[ichan]=ecorr[ichan];
      else
	fferrors[ichan]=1;

      cout << ichan << " " <<  ffcoefficients[ichan] << endl;
    }
    thisDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);
  } else
    thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
#ifdef VERBOSE
  cout << "set ff corrections " << ((thisDetector->correctionMask)&(1<<FLAT_FIELD_CORRECTION)) << endl;
#endif


  return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}










int slsDetector::getFlatFieldCorrection(double *corr, double *ecorr) {
  if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Flat field correction is enabled" << std::endl;
#endif
    if (corr) {
      for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
	//	corr[ichan]=(ffcoefficients[ichan]*ffcoefficients[ichan])/(fferrors[ichan]*fferrors[ichan]);
	corr[ichan]=ffcoefficients[ichan];
	if (ecorr) {
	  //ecorr[ichan]=ffcoefficients[ichan]/fferrors[ichan];
	  ecorr[ichan]=fferrors[ichan];
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
      postProcessingFuncs::flatFieldCorrect(datain[ichan],e,dataout[ichan],eo,ffcoefficients[ichan],fferrors[ichan]);
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

	if (getDetectorsType() == EIGER){
		int fnum=F_SET_RATE_CORRECT;
		int ret=FAIL;
		char mess[MAX_STR_LENGTH]="";
		int64_t arg = t;

#ifdef VERBOSE
		std::cout<< "Setting Rate Correction to " << arg << endl;
#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&arg,sizeof(arg));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					if(strstr(mess,"default tau")!=NULL)
						setErrorMask((getErrorMask())|(RATE_CORRECTION_NO_TAU_PROVIDED));
					if(strstr(mess,"32")!=NULL)
						setErrorMask((getErrorMask())|(RATE_CORRECTION_NOT_32or16BIT));
					else
						setErrorMask((getErrorMask())|(COULD_NOT_SET_RATE_CORRECTION));
				}
				disconnectControl();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}
		return ret;  //only success/fail
	}


	//mythen
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

	if (thisDetector->myDetectorType == EIGER){
		t = getRateCorrectionTau();
		return t;
	}

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

	if(thisDetector->myDetectorType == EIGER){
		int fnum=F_GET_RATE_CORRECT;
		int ret=FAIL;
		char mess[MAX_STR_LENGTH]="";
		int64_t retval = -1;
	#ifdef VERBOSE
		std::cout<< "Setting Rate Correction to " << arg << endl;
	#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL)
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				else {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				disconnectControl();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}

		return double(retval);
	}


	//mythen only
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

	if (thisDetector->myDetectorType == EIGER){
		double t = getRateCorrectionTau();
		return (int)t;
	}

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

      postProcessingFuncs::rateCorrect(datain[ichan], e, dataout[ichan], errout[ichan], tau, t);
    }
  }

  return 0;
};







int slsDetector::setBadChannelCorrection(string fname){

 // int nbadmod;
  int ret=0;
  //int badchanlist[MAX_BADCHANS];
  //int off;

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
	if (chs[ich]>=0 && chs[ich]<getMaxNumberOfChannels()) {
	  thisDetector->badChansList[ich]=chs[ich];
	  thisDetector->nBadChans++;
	  //  cout << "det : " << thisDetector->nBadChans << " " << thisDetector->badChansList[ich] << endl;
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
  // fillBadChannelMask();
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
      disconnectControl();
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
	int i;

	switch (index) {
	case DETECTOR_MAC:
		return setDetectorMAC(value);
	case DETECTOR_IP:
		return setDetectorIP(value);
	case RECEIVER_HOSTNAME:
		return setReceiver(value);
	case RECEIVER_UDP_IP:
		return setReceiverUDPIP(value);
	case RECEIVER_UDP_MAC:
		return setReceiverUDPMAC(value);
	case RECEIVER_UDP_PORT:
		sscanf(value.c_str(),"%d",&i);
		setReceiverUDPPort(i);
		return getReceiverUDPPort();
	case RECEIVER_UDP_PORT2:
		sscanf(value.c_str(),"%d",&i);
		if(thisDetector->myDetectorType == EIGER)
			setReceiverUDPPort2(i);
		else
			setReceiverUDPPort(i);
		if(thisDetector->myDetectorType == EIGER)
			return getReceiverUDPPort2();
		return getReceiverUDPPort();
	case DETECTOR_TXN_DELAY_LEFT:
	case DETECTOR_TXN_DELAY_RIGHT:
	case DETECTOR_TXN_DELAY_FRAME:
	case FLOW_CONTROL_10G:
		sscanf(value.c_str(),"%d",&i);
		return setDetectorNetworkParameter(index, i);
  default:
    return (char*)("unknown network parameter");
  }

}



char* slsDetector::getNetworkParameter(networkParameter index) {

  switch (index) {
  case DETECTOR_MAC:
    return getDetectorMAC();
    break;
  case DETECTOR_IP:
    return getDetectorIP();
    break;
  case RECEIVER_HOSTNAME:
    return getReceiver();
    break;
  case RECEIVER_UDP_IP:
    return getReceiverUDPIP();
    break;
  case RECEIVER_UDP_MAC:
    return getReceiverUDPMAC();
    break;
  case RECEIVER_UDP_PORT:
    return getReceiverUDPPort();
    break;
  case RECEIVER_UDP_PORT2:
    return getReceiverUDPPort2();
    break;
  case DETECTOR_TXN_DELAY_LEFT:
  case DETECTOR_TXN_DELAY_RIGHT:
  case DETECTOR_TXN_DELAY_FRAME:
  case FLOW_CONTROL_10G:
	  return setDetectorNetworkParameter(index, -1);
  default:
    return (char*)("unknown network parameter");
  }

}

char* slsDetector::setDetectorMAC(string detectorMAC){
  if(detectorMAC.length()==17){
    if((detectorMAC[2]==':')&&(detectorMAC[5]==':')&&(detectorMAC[8]==':')&&
       (detectorMAC[11]==':')&&(detectorMAC[14]==':')){
      strcpy(thisDetector->detectorMAC,detectorMAC.c_str());
      if(!strcmp(thisDetector->receiver_hostname,"none"))
    	  std::cout << "Warning: Receiver hostname not set yet." << endl;
      else if(setUDPConnection()==FAIL)
	    std::cout<< "Warning: UDP connection set up failed" << std::endl;
    }else{
      setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
      std::cout << "Warning: server MAC Address should be in xx:xx:xx:xx:xx:xx format" << endl;
    }
  }
  else{
	  setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
	  std::cout << "Warning: server MAC Address should be in xx:xx:xx:xx:xx:xx format" << std::endl;
  }

  return thisDetector->detectorMAC;
};



char* slsDetector::setDetectorIP(string detectorIP){
	struct sockaddr_in sa;
	//taking function arguments into consideration
	if(detectorIP.length()){
		if(detectorIP.length()<16){
			int result = inet_pton(AF_INET, detectorIP.c_str(), &(sa.sin_addr));
			if(result!=0){
				strcpy(thisDetector->detectorIP,detectorIP.c_str());
				if(!strcmp(thisDetector->receiver_hostname,"none"))
					std::cout << "Warning: Receiver hostname not set yet." << endl;
				else if(setUDPConnection()==FAIL)
					std::cout<< "Warning: UDP connection set up failed" << std::endl;
			}else{
				 setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
				std::cout << "Warning: Detector IP Address should be VALID and in xxx.xxx.xxx.xxx format" << std::endl;
			}
		}
	}
	return thisDetector->detectorIP;
}



char* slsDetector::setReceiver(string receiverIP){
	if(getRunStatus()==RUNNING){
		cprintf(RED,"Acquisition already running, Stopping it.\n");
		stopAcquisition();
	}
	updateDetector();

	strcpy(thisDetector->receiver_hostname,receiverIP.c_str());

	if(setReceiverOnline(ONLINE_FLAG)==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Setting up receiver with" << endl;
		std::cout << "detector type:" << slsDetectorBase::getDetectorType(thisDetector->myDetectorType) << endl;
		std::cout << "detector id:" << posId << endl;
		std::cout << "detector hostname:" << thisDetector->hostname << endl;
		std::cout << "file path:" << fileIO::getFilePath() << endl;
		std::cout << "file name:" << fileIO::getFileName() << endl;
		std::cout << "file index:" << fileIO::getFileIndex() << endl;
		pthread_mutex_lock(&ms);
		std::cout << "write enable:" << parentDet->enableWriteToFileMask() << endl;
		std::cout << "overwrite enable:" << parentDet->enableOverwriteMask() << endl;
		pthread_mutex_unlock(&ms);
		std::cout << "frame index needed:" <<  ((thisDetector->timerValue[FRAME_NUMBER]*thisDetector->timerValue[CYCLES_NUMBER])>1) << endl;
		std::cout << "frame period:" << thisDetector->timerValue[FRAME_PERIOD] << endl;
		std::cout << "frame number:" << thisDetector->timerValue[FRAME_NUMBER] << endl;
		std::cout << "dynamic range:" << thisDetector->dynamicRange << endl << endl;
		std::cout << "10GbE:" << thisDetector->tenGigaEnable << endl << endl;
		//std::cout << "dataStreaming:" << enableDataStreamingFromReceiver(-1) << endl << endl;
/** enable compresison, */
#endif
		if(setDetectorType()!= GENERIC){
			if(!posId)
				sendMultiDetectorSize();
			setDetectorId();
			setDetectorHostname();
			setFilePath(fileIO::getFilePath());
			setFileName(fileIO::getFileName());
			setFileIndex(fileIO::getFileIndex());
			setFileFormat(fileIO::getFileFormat());
			pthread_mutex_lock(&ms);
			int imask = parentDet->enableWriteToFileMask();
			pthread_mutex_unlock(&ms);
			enableWriteToFile(imask);
			pthread_mutex_lock(&ms);
			imask = parentDet->enableOverwriteMask();
			pthread_mutex_unlock(&ms);
			overwriteFile(imask);
			if ((thisDetector->timerValue[FRAME_NUMBER]*thisDetector->timerValue[CYCLES_NUMBER])>1)
				setFrameIndex(0);
			else
				setFrameIndex(-1);

			setTimer(FRAME_PERIOD,thisDetector->timerValue[FRAME_PERIOD]);
			setTimer(FRAME_NUMBER,thisDetector->timerValue[FRAME_NUMBER]);
			setTimer(ACQUISITION_TIME,thisDetector->timerValue[ACQUISITION_TIME]);
			setDynamicRange(thisDetector->dynamicRange);
			if(thisDetector->myDetectorType == EIGER){
				setFlippedData(X,-1);
				activate(-1);
			}
			//std::cout << "***********************************dataStreaming:" << parentDet->enableDataStreamingFromReceiver(-1) << endl << endl;
			//parentDet->enableDataStreamingFromReceiver(parentDet->enableDataStreamingFromReceiver(-1));
			//set scan tag
			setUDPConnection();
			if(thisDetector->myDetectorType == EIGER)
				enableTenGigabitEthernet(thisDetector->tenGigaEnable);
		}
	}

  return thisDetector->receiver_hostname;
}




char* slsDetector::setReceiverUDPIP(string udpip){
	struct sockaddr_in sa;
	//taking function arguments into consideration
	if(udpip.length()){
		if(udpip.length()<16){
			int result = inet_pton(AF_INET, udpip.c_str(), &(sa.sin_addr));
			if(result==0){
				setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
				std::cout << "Warning: Receiver UDP IP Address should be VALID and in xxx.xxx.xxx.xxx format" << std::endl;
			}else{
				strcpy(thisDetector->receiverUDPIP,udpip.c_str());
				if(!strcmp(thisDetector->receiver_hostname,"none"))
					std::cout << "Warning: Receiver hostname not set yet." << endl;
				else if(setUDPConnection()==FAIL){
					std::cout<< "Warning: UDP connection set up failed" << std::endl;
				}
			}
		}
	}
	return thisDetector->receiverUDPIP;
}




char* slsDetector::setReceiverUDPMAC(string udpmac){
  if(udpmac.length()!=17){
	setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
	std::cout << "Warning: receiver udp mac address should be in xx:xx:xx:xx:xx:xx format" << std::endl;
  }
  else{
    if((udpmac[2]==':')&&(udpmac[5]==':')&&(udpmac[8]==':')&&
       (udpmac[11]==':')&&(udpmac[14]==':')){
      strcpy(thisDetector->receiverUDPMAC,udpmac.c_str());
      if(!strcmp(thisDetector->receiver_hostname,"none"))
    	  std::cout << "Warning: Receiver hostname not set yet." << endl;
      else  if(setUDPConnection()==FAIL){
    	  std::cout<< "Warning: UDP connection set up failed" << std::endl;
      }
    }else{
      setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
      std::cout << "Warning: receiver udp mac address should be in xx:xx:xx:xx:xx:xx format" << std::endl;
    }
  }


  return thisDetector->receiverUDPMAC;
}



int slsDetector::setReceiverUDPPort(int udpport){
	thisDetector->receiverUDPPort = udpport;
	if(!strcmp(thisDetector->receiver_hostname,"none"))
		std::cout << "Warning: Receiver hostname not set yet." << endl;
	else if(setUDPConnection()==FAIL){
		std::cout<< "Warning: UDP connection set up failed" << std::endl;
    }
	return thisDetector->receiverUDPPort;
}

int slsDetector::setReceiverUDPPort2(int udpport){
	thisDetector->receiverUDPPort2 = udpport;
	if(!strcmp(thisDetector->receiver_hostname,"none"))
		std::cout << "Warning:  Receiver hostname not set yet." << endl;
	else if(setUDPConnection()==FAIL){
      std::cout<< "Warning: UDP connection set up failed" << std::endl;
    }
	return thisDetector->receiverUDPPort2;
}


char* slsDetector::setDetectorNetworkParameter(networkParameter index, int delay){
	int fnum = F_SET_NETWORK_PARAMETER;
	char* cretval = new char[MAX_STR_LENGTH];
	int ret = FAIL;
	int retval = -1;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< "Setting Transmission delay of mode "<< index << " to " <<  delay << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&index,sizeof(index));
			controlSocket->SendDataOnly(&delay,sizeof(delay));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(DETECTOR_NETWORK_PARAMETER));
			} else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	std::cout<< "Speed set to  "<< retval  << std::endl;
#endif


	sprintf(cretval,"%d",retval);
	return cretval;
}


int slsDetector::setUDPConnection(){

	int ret = FAIL;
	int fnum = F_SETUP_RECEIVER_UDP;
	char args[3][MAX_STR_LENGTH]={"","",""};
	char retval[MAX_STR_LENGTH]="";

	//called before set up
	if(!strcmp(thisDetector->receiver_hostname,"none")){
		std::cout << "Warning: Receiver hostname not set yet." << endl;
		return FAIL;
	}

	//if no udp ip given, use hostname
	if(!strcmp(thisDetector->receiverUDPIP,"none")){
		//hostname is an ip address
		if(strchr(thisDetector->receiver_hostname,'.')!=NULL)
			strcpy(thisDetector->receiverUDPIP,thisDetector->receiver_hostname);
		//if hostname not ip, convert it to ip
		else{
			struct hostent *he = gethostbyname(thisDetector->receiver_hostname);
			if (he == NULL){
				cout<<"rx_hostname:"<<thisDetector->receiver_hostname<<endl;
				std::cout << "no rx_udpip given and could not convert receiver hostname to ip" << endl;
				return FAIL;
			}else
				strcpy(thisDetector->receiverUDPIP,inet_ntoa(*(struct in_addr*)he->h_addr));
		}
	}


	//copy arguments to args[][]
	strcpy(args[0],thisDetector->receiverUDPIP);
	sprintf(args[1],"%d",thisDetector->receiverUDPPort);
	sprintf(args[2],"%d",thisDetector->receiverUDPPort2);
#ifdef VERBOSE
	std::cout << "Receiver udp ip address: " << thisDetector->receiverUDPIP << std::endl;
	std::cout << "Receiver udp port: " << thisDetector->receiverUDPPort << std::endl;
	std::cout << "Receiver udp port2: " << thisDetector->receiverUDPPort2 << std::endl;
#endif

	//set up receiver for UDP Connection and get receivermac address
	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Setting up UDP Connection for Receiver " << args[0] << "\t" << args[1] << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendUDPDetails(fnum,retval,args);
			disconnectData();
		}
		if(ret!=FAIL){
			strcpy(thisDetector->receiverUDPMAC,retval);

#ifdef VERBOSE
			std::cout << "Receiver mac address: " << thisDetector->receiverUDPMAC << std::endl;
#endif
			if(ret==FORCE_UPDATE)
				updateReceiver();

			//configure detector with udp details, -100 is so it doesnt overwrite the previous value
			if(configureMAC()==FAIL){
				setReceiverOnline(OFFLINE_FLAG);
				std::cout << "could not configure mac" << endl;
			}
		}
	}else
		ret=FAIL;

	printReceiverConfiguration();

	return ret;
}



int slsDetector::configureMAC(){
	int i;
	int ret=FAIL;
	int fnum=F_CONFIGURE_MAC,fnum2=F_RECEIVER_SHORT_FRAME;
	char mess[MAX_STR_LENGTH]="";
	char arg[6][50]={"","","","","",""};
	int retval=-1;


	//if udpip wasnt initialized in config file
	if(!(strcmp(thisDetector->receiverUDPIP,"none"))){
		//hostname is an ip address
		if(strchr(thisDetector->receiver_hostname,'.')!=NULL)
			strcpy(thisDetector->receiverUDPIP,thisDetector->receiver_hostname);
		//if hostname not ip, convert it to ip
		else{
			struct hostent *he = gethostbyname(thisDetector->receiver_hostname);
			if (he != NULL)
				strcpy(thisDetector->receiverUDPIP,inet_ntoa(*(struct in_addr*)he->h_addr));
			else{
				std::cout << "configure mac failed. no rx_udpip given and invalid receiver hostname" << endl;
				setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
				return FAIL;
			}
		}
	}
	strcpy(arg[0],thisDetector->receiverUDPIP);
	strcpy(arg[1],thisDetector->receiverUDPMAC);
	sprintf(arg[2],"%x",thisDetector->receiverUDPPort);
	strcpy(arg[3],thisDetector->detectorMAC);
	strcpy(arg[4],thisDetector->detectorIP);
	sprintf(arg[5],"%x",thisDetector->receiverUDPPort2);

#ifdef VERBOSE
	std::cout<< "Configuring MAC"<< std::endl;
#endif


	for(i=0;i<2;i++){
		if(!strcmp(arg[i],"none")){
			std::cout<< "Configure MAC Error. IP/MAC Addresses not set"<< std::endl;
			setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
			return FAIL;
		}
	}

#ifdef VERBOSE
	std::cout<< "IP/MAC Addresses valid "<< std::endl;
#endif


	{
		//converting IPaddress to hex
		stringstream ss(arg[0]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, '.')) {
			sprintf(cword,"%s%02x",cword,atoi(s.c_str()));
		}
		bzero(arg[0], 50);
		strcpy(arg[0],cword);
#ifdef VERBOSE
	std::cout<<"receiver udp ip:"<<arg[0]<<"."<<std::endl;
#endif
	}

	{
		//converting MACaddress to hex
		stringstream ss(arg[1]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, ':')) {
			sprintf(cword,"%s%s",cword,s.c_str());
		}
		bzero(arg[1], 50);
		strcpy(arg[1],cword);
#ifdef VERBOSE
	std::cout<<"receiver mac:"<<arg[1]<<"."<<std::endl;
#endif
	}

	#ifdef VERBOSE
	std::cout<<"receiver udp port:"<<arg[2]<<"."<<std::endl;
#endif

	{
		stringstream ss(arg[3]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, ':')) {
			sprintf(cword,"%s%s",cword,s.c_str());
		}
		bzero(arg[3], 50);
		strcpy(arg[3],cword);
#ifdef VERBOSE
	std::cout<<"detecotor mac:"<<arg[3]<<"."<<std::endl;
#endif
	}

	{
		//converting IPaddress to hex
		stringstream ss(arg[4]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, '.')) {
			sprintf(cword,"%s%02x",cword,atoi(s.c_str()));
		}
		bzero(arg[4], 50);
		strcpy(arg[4],cword);
#ifdef VERBOSE
	std::cout<<"detector ip:"<<arg[4]<<"."<<std::endl;
#endif
	}

#ifdef VERBOSE
	std::cout<<"receiver udp port2:"<<arg[5]<<"."<<std::endl;
#endif

	//send to server
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
			}
			else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	if (ret==FAIL) {
		ret=FAIL;
		std::cout<< "Configuring MAC failed " << std::endl;
		setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
	}
	else if (thisDetector->myDetectorType==GOTTHARD){
		//set frames per file - only for gotthard
		pthread_mutex_lock(&ms);
		if(retval==-1)
			setFramesPerFile(MAX_FRAMES_PER_FILE);
		else
			setFramesPerFile(SHORT_MAX_FRAMES_PER_FILE);
		pthread_mutex_unlock(&ms);
		//connect to receiver
		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Sending adc val to receiver " << retval << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum2,retval,retval);
				disconnectData();
			}
			if(ret==FAIL)
				setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
		}
	}

	return ret;
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


int slsDetector:: writeAngularConversion(string fname) {

  return writeAngularConversion(fname, thisDetector->nMods, thisDetector->angOff);

}


int slsDetector:: writeAngularConversion(ofstream &ofs) {

  return writeAngularConversion(ofs, thisDetector->nMods, thisDetector->angOff);

}





int slsDetector::loadImageToDetector(imageType index,string const fname){

  int ret=FAIL;
  short int arg[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];

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
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<<"Sending image to detector " <<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
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
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return ret;
}
int slsDetector::getCounterBlock(short int arg[],int startACQ){

  int ret=FAIL;
  int fnum=F_READ_COUNTER_BLOCK;
  char mess[MAX_STR_LENGTH]="";

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&startACQ,sizeof(startACQ));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(arg,thisDetector->dataBytes);
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return ret;
}


int slsDetector::writeCounterBlockFile(string const fname,int startACQ){

  int ret=FAIL;
  short int counterVals[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];

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
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<< std::endl<< "Resetting Counter";
  if(startACQ==1)
    std::cout<<" and Restarting Acquisition";
  std::cout<<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&startACQ,sizeof(startACQ));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL){
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return ret;
}



int slsDetector::setCounterBit(int i){
	int fnum=F_SET_COUNTER_BIT;
	int ret = FAIL;
	int retval=-1;
	char mess[MAX_STR_LENGTH]="";

	if(thisDetector->onlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		if(i ==-1)
			std::cout<< "Getting counter bit from detector" << endl;
		else if(i==0)
			std::cout<< "Resetting counter bit in detector " << endl;
		else
			std::cout<< "Setting counter bit in detector " <<  endl;
#endif
		if (connectControl() == OK){

			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&i,sizeof(i));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_SET_COUNTER_BIT));
			}
			controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();

		}
	}
	return retval;
}







int slsDetector::printReceiverConfiguration(){

	std::cout << "Detector IP:\t\t" << getNetworkParameter(DETECTOR_IP) << std::endl;
	std::cout << "Detector MAC:\t\t" << getNetworkParameter(DETECTOR_MAC) << std::endl;

	std::cout << "Receiver Hostname:\t" << getNetworkParameter(RECEIVER_HOSTNAME) << std::endl;
	std::cout << "Receiver UDP IP:\t" << getNetworkParameter(RECEIVER_UDP_IP) << std::endl;
	std::cout << "Receiver UDP MAC:\t" << getNetworkParameter(RECEIVER_UDP_MAC) << std::endl;


	std::cout << "Receiver UDP Port:\t" << getNetworkParameter(RECEIVER_UDP_PORT) << std::endl;
	if(thisDetector->myDetectorType == EIGER)
		std::cout << "Receiver UDP Port2:\t" << getNetworkParameter(RECEIVER_UDP_PORT2) << std::endl;

	std::cout << std::endl;

	return OK;
}



int slsDetector::readConfigurationFile(string const fname){



  string ans;
  string str;
  ifstream infile;
  //char *args[1000];

  string sargname, sargval;
#ifdef VERBOSE
  int iline=0;
  std::cout<< "config file name "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
#ifdef VERBOSE
    iline=readConfigurationFile(infile);
#else
    readConfigurationFile(infile);
#endif
    infile.close();
  } else {
    std::cout<< "Error opening configuration file " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif
  return OK;

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
  return OK;

}










int slsDetector::writeConfigurationFile(string const fname){


  ofstream outfile;
#ifdef VERBOSE
  int ret;
#endif
  outfile.open(fname.c_str(),ios_base::out);
  if (outfile.is_open()) {
#ifdef VERBOSE
	ret=writeConfigurationFile(outfile);
#else
    writeConfigurationFile(outfile);
#endif
    outfile.close();
  }
  else {
    std::cout<< "Error opening configuration file " << fname << " for writing" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "wrote " <<ret << " lines to configuration file " << std::endl;
#endif
  return OK;


}


int slsDetector::writeConfigurationFile(ofstream &outfile, int id){

  slsDetectorCommand *cmd=new slsDetectorCommand(this);
  int nvar=15;

  string names[20]={				\
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
  //   "receiverTCPPort",

  if ((thisDetector->myDetectorType==GOTTHARD)||
		  (thisDetector->myDetectorType==PROPIX)||
		  (thisDetector->myDetectorType==JUNGFRAU)||
		  (thisDetector->myDetectorType==MOENCH)) {
	names[0]= "hostname";
	names[1]= "port";
	names[2]= "stopport";
	names[3]= "settingsdir";
	names[4]= "angdir";
	names[5]= "moveflag";
	names[6]= "lock";
	names[7]= "caldir";
	names[8]= "ffdir";
	names[9]= "extsig";
    names[10]="detectormac";
    names[11]="detectorip";
	names[12]= "rx_tcpport";
	names[13]= "rx_udpport";
    names[14]="rx_udpip";
    names[15]="rx_hostname";
    names[16]="outdir";
    names[17]="vhighvoltage";
    nvar=18;
  }


  int nsig=4;//-1;
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
  return OK;
}















int slsDetector::writeSettingsFile(string fname, int imod, int iodelay, int tau){

  return writeSettingsFile(fname,thisDetector->myDetectorType, detectorModules[imod], iodelay, tau);

};




int slsDetector::programFPGA(string fname){
	int ret=FAIL;
	int fnum=F_PROGRAM_FPGA;
	char mess[MAX_STR_LENGTH]="";
	size_t filesize=0;
	char* fpgasrc = NULL;

	if(thisDetector->myDetectorType != JUNGFRAU){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}


	//check if it exists
	struct stat st;
	if(stat(fname.c_str(),&st)){
		std::cout << "Programming file does not exist" << endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	//create destination file name,replaces original filename with Jungfrau.rawbin
	string destfname;
	size_t found = fname.find_last_of("/\\");
	if(found == string::npos)
		destfname = "";
	else
		destfname = fname.substr(0,found+1);
	destfname.append("Jungfrau_MCB.rawbin");
#ifdef VERBOSE
	std::cout << "Converting " << fname << " to " <<  destfname << std::endl;
#endif
	int filepos,x,y,i;
	FILE* src = fopen(fname.c_str(),"rb");
	FILE* dst = fopen(destfname.c_str(),"wb");
	// Remove header (0...11C)
	for (filepos=0; filepos < 0x11C; filepos++)
		fgetc(src);
	// Write 0x80 times 0xFF (0...7F)
	for (filepos=0; filepos < 0x80; filepos++)
		fputc(0xFF,dst);
	// Swap bits and write to file
	for (filepos=0x80; filepos < 0x1000000; filepos++)	{
		x = fgetc(src);
		if (x < 0) break;
		y=0;
		for (i=0; i < 8; i++)
			y=y| (   (( x & (1<<i) ) >> i)    << (7-i)     );	// This swaps the bits
		fputc(y,dst);
	}
	if (filepos < 0x1000000){
		std::cout << "Could not convert programming file. EOF before end of flash" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
#ifdef VERBOSE
	std::cout << "File has been converted to "  <<  destfname << std::endl;
#endif
	//loading file to memory
	FILE* fp = fopen(destfname.c_str(),"r");
	if(fp == NULL){
		std::cout << "Could not open rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	if(fseek(fp,0,SEEK_END)){
		std::cout << "Seek error in rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	filesize = ftell(fp);
	if(filesize <= 0){
		std::cout << "Could not get length of rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	rewind(fp);
	fpgasrc = (char*)malloc(filesize+1);
	if(fpgasrc == NULL){
		std::cout << "Could not allocate size of program" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	if(fread(fpgasrc, sizeof(char), filesize, fp) != filesize){
		std::cout << "Could not read rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}

	if(fclose(fp)){
		std::cout << "Could not close rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
#ifdef VERBOSE
	std::cout << "Successfully loaded the rawbin file to program memory" << std::endl;
#endif



#ifdef VERBOSE
	std::cout<< "Sending programming binary to detector " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&filesize,sizeof(filesize));
			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				filesize = 0;
			}


			//erasing flash
			if(ret!=FAIL){
				std::cout<< "This can take awhile. Please be patient..." << endl;
				printf("Erasing Flash:%d%%\r",0);
				std::cout << flush;
				//erasing takes 65 seconds, printing here (otherwise need threads in server-unnecessary)
				int count = 66;
				while(count>0){
					usleep(1 * 1000 * 1000);
					count--;
					printf("Erasing Flash:%d%%\r",(int) (((double)(65-count)/65)*100));
					std::cout << flush;
				}
				std::cout<<std::endl;
				printf("Writing to Flash:%d%%\r",0);
				std::cout << flush;
			}


			//sending program in parts of 2mb each
			size_t unitprogramsize = 0;
			int currentPointer = 0;
			size_t totalsize= filesize;
			while(ret != FAIL && (filesize > 0)){

				unitprogramsize = MAX_FPGAPROGRAMSIZE;  //2mb
				if(unitprogramsize > filesize) //less than 2mb
					unitprogramsize = filesize;
#ifdef VERBOSE
				std::cout << "unitprogramsize:" << unitprogramsize << "\t filesize:" << filesize << std::endl;
#endif
				controlSocket->SendDataOnly(fpgasrc+currentPointer,unitprogramsize);
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL) {
					filesize-=unitprogramsize;
					currentPointer+=unitprogramsize;

					//print progress
					printf("Writing to Flash:%d%%\r",(int) (((double)(totalsize-filesize)/totalsize)*100));
					std::cout << flush;
				}else{
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
			}
			std::cout<<std::endl;

			//check ending error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	//free resources
	if(fpgasrc != NULL)
		free(fpgasrc);

	return ret;
}


int slsDetector::resetFPGA(){
	int ret=FAIL;
	int fnum=F_RESET_FPGA;
	char mess[MAX_STR_LENGTH]="";

	if(thisDetector->myDetectorType != JUNGFRAU){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Sending reset to FPGA " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));

			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(RESET_ERROR));
			}

			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return ret;

}



int slsDetector::powerChip(int ival){
	int ret=FAIL;
	int fnum=F_POWER_CHIP;
	char mess[MAX_STR_LENGTH]="";
	int retval=-1;

	if(thisDetector->myDetectorType != JUNGFRAU && thisDetector->myDetectorType != JUNGFRAUCTB ){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Sending power on/off/get to the chip " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&ival,sizeof(ival));
			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(POWER_CHIP));
			}else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return retval;

}
int slsDetector::loadSettingsFile(string fname, int imod) {

  sls_detector_module  *myMod=NULL;

  int iodelay = -1;
  int tau = -1;

  string fn=fname;
  fn=fname;
  int mmin=0, mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    ostringstream ostfn;
    ostfn << fname;
    if(thisDetector->myDetectorType != EIGER){
    	if (fname.find(".sn")==string::npos && fname.find(".trim")==string::npos && fname.find(".settings")==string::npos) {
    		ostfn << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im);
    		fn=ostfn.str();
    	}
    }else if (fname.find(".sn")==string::npos && fname.find(".trim")==string::npos && fname.find(".settings")==string::npos) {
      ostfn << ".sn"  << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER, im);
      fn=ostfn.str();
    }
    myMod=readSettingsFile(fn, thisDetector->myDetectorType,iodelay, tau, myMod);

    if (myMod) {
      myMod->module=im;
      //settings is saved in myMod.reg for all except mythen
      if(thisDetector->myDetectorType!=MYTHEN)
    	myMod->reg=-1;
      setModule(*myMod,iodelay,tau,-1,0,0);
      deleteModule(myMod);
    } else
      return FAIL;
  }
  return OK;
}


int slsDetector::saveSettingsFile(string fname, int imod) {

  sls_detector_module  *myMod=NULL;
  int ret=FAIL;
  int iodelay = -1;
  int tau = -1;

  int mmin=0,  mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    ostringstream ostfn;
    if(thisDetector->myDetectorType == EIGER){
    	ostfn << fname << ".sn"  << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER);
    } else
    	ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER,im);
    if ((myMod=getModule(im))) {

    	 if(thisDetector->myDetectorType == EIGER){
    		 iodelay = (int)setDAC((dacs_t)-1,IO_DELAY,0,-1);
    		 tau = (int64_t)getRateCorrectionTau();
    	 }
   		 ret=writeSettingsFile(ostfn.str(), thisDetector->myDetectorType, *myMod, iodelay, tau);
   		 deleteModule(myMod);
    }
  }
  return ret;
}



int slsDetector::setAllTrimbits(int val, int imod){
	int fnum=F_SET_ALL_TRIMBITS;
	int retval;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Setting all trimbits to "<< val << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&val,sizeof(val));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(ALLTIMBITS_NOT_SET));
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

#ifdef VERBOSE
	std::cout<< "All trimbits were set to "<< retval   << std::endl;
#endif
	return retval;
}




int slsDetector::loadCalibrationFile(string fname, int imod) {

	if(thisDetector->myDetectorType == EIGER) {
		std::cout << "Not required for this detector!" << std::endl;
		return FAIL;
	}

  sls_detector_module  *myMod=NULL;
  string fn=fname;

  int* gainval=0; int* offsetval=0;
  if(thisDetector->nGain){
	  gainval=new int[thisDetector->nGain];
	  for(int i=0;i<thisDetector->nGain;i++)
		  gainval[i] = -1;
  }
  if(thisDetector->nOffset){
	  offsetval=new int[thisDetector->nOffset];
	  for(int i=0;i<thisDetector->nOffset;i++)
		  offsetval[i] = -1;
  }

  fn=fname;


  int mmin=0, mmax=setNumberOfModules();
  if (imod>=0) {
    mmin=imod;
    mmax=imod+1;
  }
  for (int im=mmin; im<mmax; im++) {
    ostringstream ostfn;
    ostfn << fname ;
    if(thisDetector->myDetectorType != EIGER){
    	if (fname.find(".sn")==string::npos && fname.find(".cal")==string::npos) {
    		ostfn << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im);
    	}
    }else if (fname.find(".sn")==string::npos && fname.find(".cal")==string::npos) {
      ostfn << "."  << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER);
    }
    fn=ostfn.str();
    if((myMod=getModule(im))){
      //extra gain and offset
      if(thisDetector->nGain){
    	  if(readCalibrationFile(fn, gainval, offsetval)==FAIL)
    		  return FAIL;
      } //normal gain and offset inside sls_detector_module
      else{
    	  if(readCalibrationFile(fn,myMod->gain, myMod->offset)==FAIL)
    		  return FAIL;
      }
      setModule(*myMod,-1,-1,-1,gainval,offsetval);

      deleteModule(myMod);
      if(gainval) delete[]gainval;
      if(offsetval) delete[] offsetval;
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
    if(thisDetector->myDetectorType == EIGER)
    	ostfn << fname << ".sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER);
    else
    	ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER,im);
    if ((myMod=getModule(im))) {
        //extra gain and offset
        if(thisDetector->nGain)
        	ret=writeCalibrationFile(ostfn.str(),gain, offset);
         //normal gain and offset inside sls_detector_module
        else
        	ret=writeCalibrationFile(ostfn.str(),myMod->gain, myMod->offset);

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
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting master flags to "<< flag << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&flag,sizeof(flag));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      } else {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
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
  char mess[MAX_STR_LENGTH]="";
  int ret=OK;

#ifdef VERBOSE
  std::cout<< "Setting synchronization mode to "<< flag << std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&flag,sizeof(flag));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL) {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      } else {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

#ifdef VERBOSE
  std::cout<< "Readout flag set to  "<< retval   << std::endl;
#endif
  return retval;

}










/*receiver*/
int slsDetector::setReceiverOnline(int off) {
  //	int prev = thisDetector->receiverOnlineFlag;
	if (off!=GET_ONLINE_FLAG) {
		if(strcmp(thisDetector->receiver_hostname,"none")){
			thisDetector->receiverOnlineFlag=off;
			if (thisDetector->receiverOnlineFlag==ONLINE_FLAG){
				setReceiverTCPSocket();
				if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
					std::cout << "cannot connect to receiver" << endl;
					setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
				}
			}
		}
	}
	return thisDetector->receiverOnlineFlag;
}



string slsDetector::checkReceiverOnline() {
  string retval = "";
  //if it doesnt exits, create data socket
  if(!dataSocket){
    //this already sets the online/offline flag
    setReceiverTCPSocket();
    if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG)
      return string(thisDetector->receiver_hostname);
    else
      return string("");
  }
  //still cannot connect to socket, dataSocket=0
  if(dataSocket){
    if (connectData() == FAIL) {
      dataSocket->SetTimeOut(5);
      thisDetector->receiverOnlineFlag=OFFLINE_FLAG;
      delete dataSocket;
      dataSocket=NULL;
#ifdef VERBOSE
      std::cout<< "receiver offline!" << std::endl;
#endif
      return string(thisDetector->receiver_hostname);
    }  else {
      thisDetector->receiverOnlineFlag=ONLINE_FLAG;
      dataSocket->SetTimeOut(100);
      disconnectData();
#ifdef VERBOSE
      std::cout<< "receiver online!" << std::endl;
#endif
      return string("");
    }
  }
  return retval;
}





int slsDetector::setReceiverTCPSocket(string const name, int const receiver_port){

  char thisName[MAX_STR_LENGTH];
  int thisRP;
  int retval=OK;

  //if receiver ip given
  if (strcmp(name.c_str(),"")!=0) {
#ifdef VERBOSE
    std::cout<< "setting receiver" << std::endl;
#endif
    strcpy(thisName,name.c_str());
    strcpy(thisDetector->receiver_hostname,thisName);
    if (dataSocket){
      delete dataSocket;
      dataSocket=NULL;
    }
  } else
    strcpy(thisName,thisDetector->receiver_hostname);

  //if receiverTCPPort given
  if (receiver_port>0) {
#ifdef VERBOSE
    std::cout<< "setting data port" << std::endl;
#endif
    thisRP=receiver_port;
    thisDetector->receiverTCPPort=thisRP;
    if (dataSocket){
      delete dataSocket;
      dataSocket=NULL;
    }
  } else
    thisRP=thisDetector->receiverTCPPort;

  //create data socket
  if (!dataSocket) {
    dataSocket=new MySocketTCP(thisName, thisRP);
    if (dataSocket->getErrorStatus()){
#ifdef VERBOSE
      std::cout<< "Could not connect Data socket "<<thisName  << " " << thisRP << std::endl;
#endif
      delete dataSocket;
      dataSocket=NULL;
      retval=FAIL;
    }
#ifdef VERYVERBOSE
    else
      std::cout<< "Data socket connected "<< thisName << " " << thisRP << std::endl;
#endif
  }
  //check if it connects
  if (retval!=FAIL) {
    if(checkReceiverOnline().empty())
      retval=FAIL;
  } else {
	thisDetector->receiverOnlineFlag=OFFLINE_FLAG;
#ifdef VERBOSE
    std::cout<< "offline!" << std::endl;
#endif
  }
  thisReceiver->setSocket(dataSocket);
  return retval;
};






string slsDetector::setFilePath(string s) {
	int fnum = F_SET_RECEIVER_FILE_PATH;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH]="";
	char retval[MAX_STR_LENGTH] = "";
	struct stat st;

	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(!s.empty()){
			if(stat(s.c_str(),&st)){
				std::cout << "path does not exist" << endl;
				setErrorMask((getErrorMask())|(FILE_PATH_DOES_NOT_EXIST));
			}else{
				pthread_mutex_lock(&ms);
				fileIO::setFilePath(s);
				pthread_mutex_unlock(&ms);
			}
		}
	}

	else{
		strcpy(arg,s.c_str());
#ifdef VERBOSE
		std::cout << "Sending file path to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			fileIO::setFilePath(string(retval));
			pthread_mutex_unlock(&ms);
		}
		else if(!s.empty()){
			std::cout << "file path does not exist" << endl;
			setErrorMask((getErrorMask())|(FILE_PATH_DOES_NOT_EXIST));
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	s = fileIO::getFilePath();
	pthread_mutex_unlock(&ms);

	return s;
}



string slsDetector::setFileName(string s) {
	int fnum=F_SET_RECEIVER_FILE_NAME;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH]="";
	char retval[MAX_STR_LENGTH]="";

	if(!s.empty()){
		pthread_mutex_lock(&ms);
		fileIO::setFileName(s);
		/*if(thisDetector->myDetectorType == EIGER)
			parentDet->setDetectorIndex(posId);
		else if(parentDet->getNumberOfDetectors()>1)
			parentDet->setDetectorIndex(-1);*/
		s=parentDet->createReceiverFilePrefix();
		pthread_mutex_unlock(&ms);
	}

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
		strcpy(arg,s.c_str());
#ifdef VERBOSE
		std::cout << "Sending file name to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
#ifdef VERBOSE
			std::cout << "Complete file prefix from receiver: " << retval << std::endl;
#endif
			pthread_mutex_lock(&ms);
			fileIO::setFileName(parentDet->getNameFromReceiverFilePrefix(string(retval)));
			pthread_mutex_unlock(&ms);

		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	s = fileIO::getFileName();
	pthread_mutex_unlock(&ms);

	return s;
}




slsReceiverDefs::fileFormat slsDetector::setFileFormat(fileFormat f){
	int fnum=F_SET_RECEIVER_FILE_FORMAT;
	int ret = FAIL;
	int arg = -1;
	int retval = -1;



	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(f>=0){
			pthread_mutex_lock(&ms);
			fileIO::setFileFormat(f);
			pthread_mutex_unlock(&ms);
		}
	}

	else{
		arg = (int)f;
#ifdef VERBOSE
		std::cout << "Sending file format to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret == FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_FILE_FORMAT));
		else{
			pthread_mutex_lock(&ms);
			fileIO::setFileFormat(retval);
			pthread_mutex_unlock(&ms);
			if(ret==FORCE_UPDATE)
				updateReceiver();
		}
	}

	return fileIO::getFileFormat();
}





int slsDetector::setFileIndex(int i) {
	int fnum=F_SET_RECEIVER_FILE_INDEX;
	int ret = FAIL;
	int retval=-1;
	int arg = i;


	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(i>=0){
			pthread_mutex_lock(&ms);
			fileIO::setFileIndex(i);
			pthread_mutex_unlock(&ms);
		}
	}

	else{
#ifdef VERBOSE
		std::cout << "Sending file index to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			fileIO::setFileIndex(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return fileIO::getFileIndex();
}




int slsDetector::startReceiver(){
	int fnum=F_START_RECEIVER;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Starting Receiver " << std::endl;
#endif

		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
		else if (ret == FAIL){
			if(strstr(mess,"UDP")!=NULL)
				setErrorMask((getErrorMask())|(COULDNOT_CREATE_UDP_SOCKET));
			else if(strstr(mess,"file")!=NULL)
				setErrorMask((getErrorMask())|(COULDNOT_CREATE_FILE));
			else
				setErrorMask((getErrorMask())|(COULDNOT_START_RECEIVER));
		}
	}

	//let detector prepare anyway even if receiver didnt work (for those not using the receiver)
	if((thisDetector->myDetectorType != JUNGFRAU)) {
		int ret1 = detectorSendToReceiver(true);
		if (ret != FAIL)
			ret = ret1;
	}

	return ret;
}




int slsDetector::stopReceiver(){
	int fnum=F_STOP_RECEIVER;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if(thisDetector->myDetectorType != EIGER && thisDetector->myDetectorType != JUNGFRAU)
		detectorSendToReceiver(false);

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Stopping Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
		else if (ret == FAIL)
			setErrorMask((getErrorMask())|(COULDNOT_STOP_RECEIVER));
	}

	return ret;
}




slsDetectorDefs::runStatus slsDetector::startReceiverReadout(){
	int fnum=F_START_RECEIVER_READOUT;
	int ret = FAIL;
	int retval=-1;
	runStatus s=ERROR;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Starting Receiver Readout" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(retval!=-1)
			s=(runStatus)retval;
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return s;
}


int slsDetector::detectorSendToReceiver(bool set){
  int fnum;
  if(set)	fnum=F_START_RECEIVER;
  else	fnum=F_STOP_RECEIVER;
  int ret = FAIL;
  char mess[MAX_STR_LENGTH]="";

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
    std::cout << "Setting detector to send packets via client to: " << set << std::endl;
#endif
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret==FAIL){
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }else
    std::cout << "cannot connect to detector" << endl;

  return ret;
}







slsDetectorDefs::runStatus slsDetector::getReceiverStatus(){
	int fnum=F_GET_RECEIVER_STATUS;
	int ret = FAIL;
	int retval=-1;
	runStatus s=ERROR;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Getting Receiver Status" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(retval!=-1)
			s=(runStatus)retval;
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return s;
}




int slsDetector::getFramesCaughtByReceiver(){
	int fnum=F_GET_RECEIVER_FRAMES_CAUGHT;
	int ret = FAIL;
	int retval=-1;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Getting Frames Caught by Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return retval;
}



int slsDetector::getReceiverCurrentFrameIndex(){
	int fnum=F_GET_RECEIVER_FRAME_INDEX;
	int ret = FAIL;
	int retval=-1;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Getting Current Frame Index of Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return retval;
}




int slsDetector::resetFramesCaught(){
	int fnum=F_RESET_RECEIVER_FRAMES_CAUGHT;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Reset Frames Caught by Receiver" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return ret;
}


//  int* slsDetector::readFrameFromReceiver(char* fName,  int &acquisitionIndex, int &frameIndex, int &subFrameIndex){ 	
//    int fnum=F_READ_RECEIVER_FRAME; 	
//    int nel=thisDetector->dataBytes/sizeof(int); 	
//    int* retval=new int[nel]; 	
//    int ret=FAIL; 	
//    int n; 	
//    char mess[MAX_STR_LENGTH]="Nothing"; 	
//    if (setReceiverOnline(ONLINE_FLAG)==ONLINE_FLAG) { 
// #ifdef VERBOSE 		
//      std::cout<< "slsDetector: Reading frame from receiver "<< thisDetector->dataBytes << " " <<nel <<std::endl; 
// #endif 		
//      if (connectData() == OK){ 			
//        dataSocket->SendDataOnly(&fnum,sizeof(fnum)); 			
//        dataSocket->ReceiveDataOnly(&ret,sizeof(ret)); 			
//        if (ret==FAIL) { 				
// 	 n= dataSocket->ReceiveDataOnly(mess,sizeof(mess)); 				
// 	 std::cout<< "Detector returned: " << mess << " " << n << std::endl; 				
// 	 delete [] retval; 				
// 	 disconnectData(); 				
// 	 return NULL; 			
//        } else { 				
// 	 n=dataSocket->ReceiveDataOnly(fName,MAX_STR_LENGTH); 				
// 	 n=dataSocket->ReceiveDataOnly(&acquisitionIndex,sizeof(acquisitionIndex)); 				
// 	 n=dataSocket->ReceiveDataOnly(&frameIndex,sizeof(frameIndex)); 				
// 	 if(thisDetector->myDetectorType == EIGER) 					
// 	   n=dataSocket->ReceiveDataOnly(&subFrameIndex,sizeof(subFrameIndex)); 				
// 	 n=dataSocket->ReceiveDataOnly(retval,thisDetector->dataBytes); 
// #ifdef VERBOSE 				
// 	 std::cout<< "Received "<< n << " data bytes" << std::endl; 
// #endif 				
// 	 if (n!=thisDetector->dataBytes) { 					
// 	   std::cout<<endl<< "wrong data size received: received " << n << " but expected from receiver " << thisDetector->dataBytes << std::endl; 					
// 	   ret=FAIL; 					
// 	   delete [] retval; 					
// 	   disconnectData(); 					
// 	   return NULL; 				} 				
// 	 //jungfrau masking adcval 				
// 	 if(thisDetector->myDetectorType == JUNGFRAU){ 					
// 	 for(unsigned int i=0;i<nel;i++){ 						
// 	 retval[i] = (retval[i] & 0x3FFF3FFF); 					
//        } 				
//        } 			
//        } 			
// 	 disconnectData(); 		
//        } 	
//        } 	
// 	 return retval; 
//        }; 



int slsDetector::lockReceiver(int lock){
	int fnum=F_LOCK_RECEIVER;
	int ret = FAIL;
	int retval=-1;
	int arg=lock;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Locking or Unlocking Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return retval;
}






string slsDetector::getReceiverLastClientIP(){
	int fnum=F_GET_LAST_RECEIVER_CLIENT_IP;
	int ret = FAIL;
	char retval[INET_ADDRSTRLEN]="";

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Geting Last Client IP connected to Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getLastClientIP(fnum,retval);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return string(retval);
}





int slsDetector::updateReceiverNoWait() {

  int n = 0,ind;
  char path[MAX_STR_LENGTH];
  char lastClientIP[INET_ADDRSTRLEN];

  n += 	dataSocket->ReceiveDataOnly(lastClientIP,sizeof(lastClientIP));
#ifdef VERBOSE
  cout << "Updating receiver last modified by " << lastClientIP << std::endl;
#endif

  n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	pthread_mutex_lock(&ms);
  fileIO::setFileIndex(ind);
	pthread_mutex_unlock(&ms);

  n += 	dataSocket->ReceiveDataOnly(path,MAX_STR_LENGTH);
	pthread_mutex_lock(&ms);
  fileIO::setFilePath(path);
	pthread_mutex_unlock(&ms);

  n += 	dataSocket->ReceiveDataOnly(path,MAX_STR_LENGTH);
  pthread_mutex_lock(&ms);
  fileIO::setFileName(path);
  pthread_mutex_unlock(&ms);

  if (!n) printf("n: %d\n", n);

  return OK;

}





int slsDetector::updateReceiver() {
	int fnum=F_UPDATE_RECEIVER_CLIENT;
	int ret=OK;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
		if (connectData() == OK){
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				updateReceiverNoWait();
			else{
				dataSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
			}
			disconnectData();
		}
	}

	return ret;
}






int slsDetector::exitReceiver(){

  int retval;
  int fnum=F_EXIT_RECEIVER;

  if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
    if (dataSocket) {
    	dataSocket->Connect();
    	dataSocket->SendDataOnly(&fnum,sizeof(fnum));
    	dataSocket->ReceiveDataOnly(&retval,sizeof(retval));
    	disconnectData();
    }
  }
  if (retval!=OK) {
    std::cout<< std::endl;
    std::cout<< "Shutting down the receiver" << std::endl;
    std::cout<< std::endl;
  }
  return retval;

}





int slsDetector::enableWriteToFile(int enable){
	int fnum=F_ENABLE_RECEIVER_FILE_WRITE;
	int ret = FAIL;
	int retval=-1;
	int arg = enable;


	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(enable>=0){
			pthread_mutex_lock(&ms);
			parentDet->enableWriteToFileMask(enable);
			pthread_mutex_unlock(&ms);
		}
	}

	else if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending enable file write to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			parentDet->enableWriteToFileMask(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	retval = parentDet->enableWriteToFileMask();
	pthread_mutex_unlock(&ms);

	return retval;
}




int slsDetector::overwriteFile(int enable){
	int fnum=F_ENABLE_RECEIVER_OVERWRITE;
	int ret = FAIL;
	int retval=-1;
	int arg = enable;


	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(enable>=0){
		    pthread_mutex_lock(&ms);
			parentDet->enableOverwriteMask(enable);
			pthread_mutex_unlock(&ms);
		}
	}

	else if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending enable file write to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			parentDet->enableOverwriteMask(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	retval = parentDet->enableOverwriteMask();
	pthread_mutex_unlock(&ms);

	return retval;
}




int slsDetector::setFrameIndex(int index){
	int fnum=F_SET_RECEIVER_FRAME_INDEX;
	int ret = FAIL;
	int retval=-1;
	int arg = index;

	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		pthread_mutex_lock(&ms);
		fileIO::setFrameIndex(index);
		pthread_mutex_unlock(&ms);
	}

	else if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending frame index to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			fileIO::setFrameIndex(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}
	pthread_mutex_lock(&ms);
	retval = fileIO::getFrameIndex();
	pthread_mutex_unlock(&ms);

	return retval;
}


int slsDetector::calibratePedestal(int frames){
  int ret=FAIL;
  int retval=-1;
  int fnum=F_CALIBRATE_PEDESTAL;
  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<<"Calibrating Pedestal " <<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&frames,sizeof(frames));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return retval;
}



int64_t slsDetector::clearAllErrorMask(){
	clearErrorMask();

	 pthread_mutex_lock(&ms);
	for(int i=0;i<parentDet->getNumberOfDetectors();i++){
		if(parentDet->getDetectorId(i) == getDetectorId())
			parentDet->setErrorMask(parentDet->getErrorMask()|(0<<i));
	}
	 pthread_mutex_unlock(&ms);

	return getErrorMask();
}



int slsDetector::setReadReceiverFrequency(int getFromReceiver, int freq){
	int fnum=F_READ_RECEIVER_FREQUENCY;
	int ret = FAIL;
	int retval=-1;
	int arg = freq;

	if(!getFromReceiver)
		return retval;

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending read frequency to receiver " << arg  << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL)
			retval = -1;
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	if ((freq > 0) && (retval != freq)){
		cout << "could not set receiver read frequency to " << freq <<" Returned:" << retval << endl;
		setErrorMask((getErrorMask())|(RECEIVER_READ_FREQUENCY));
	}
	return retval;
}



int slsDetector::setReceiverReadTimer(int time_in_ms){
	int fnum=F_READ_RECEIVER_TIMER;
	int ret = FAIL;
	int arg = time_in_ms;
	int retval = -1;

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending read timer to receiver " << arg  << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	if ((time_in_ms > 0) && (retval != time_in_ms)){
		cout << "could not set receiver read timer to " << time_in_ms <<" Returned:" << retval << endl;
		setErrorMask((getErrorMask())|(RECEIVER_READ_TIMER));
	}
	return retval;
}



int slsDetector::enableDataStreamingFromReceiver(int enable){
	int fnum=F_STREAM_DATA_FROM_RECEIVER;
	int ret = FAIL;
	int retval=-1;
	int arg = enable;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "***************Sending Data Streaming in Receiver " << arg  << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL)
			retval = -1;
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	if ((enable > 0) && (retval != enable)){
		cout << "could not set data streaming in receiver to " << enable <<" Returned:" << retval << endl;
		setErrorMask((getErrorMask())|(DATA_STREAMING));
	}
	return retval;
}



int slsDetector::enableReceiverCompression(int i){
	int fnum=F_ENABLE_RECEIVER_COMPRESSION;
	int ret = FAIL;
	int retval=-1;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Getting/Enabling/Disabling Receiver Compression with argument " << i << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,i);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(COULDNOT_ENABLE_COMPRESSION));
	}
	return retval;
}



void slsDetector::sendMultiDetectorSize(){
	int fnum=F_SEND_RECEIVER_MULTIDETSIZE;
	int ret = FAIL;
	int retval = -1;
	int arg[2];

	pthread_mutex_lock(&ms);
	parentDet->getNumberOfDetectors(arg[0],arg[1]);
	pthread_mutex_unlock(&ms);

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending multi detector size to Receiver (" << arg[0] << "," << arg[1] << ")" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendIntArray(fnum,retval,arg);
			disconnectData();
		}
		if((ret==FAIL)){
			std::cout << "Could not set position Id" << std::endl;
			setErrorMask((getErrorMask())|(RECEIVER_MULTI_DET_SIZE_NOT_SET));
		}
	}
}


void slsDetector::setDetectorId(){
	int fnum=F_SEND_RECEIVER_DETPOSID;
	int ret = FAIL;
	int retval = -1;
	int arg = posId;

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending detector pos id to Receiver " << posId << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if((ret==FAIL) || (retval != arg)){
			std::cout << "Could not set position Id" << std::endl;
			setErrorMask((getErrorMask())|(RECEIVER_DET_POSID_NOT_SET));
		}
	}
}


void slsDetector::setDetectorHostname(){
	int fnum=F_SEND_RECEIVER_DETHOSTNAME;
	int ret = FAIL;
	char retval[MAX_STR_LENGTH]="";


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending detector hostname to Receiver " << thisDetector->hostname << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,thisDetector->hostname);
			disconnectData();
		}
		if((ret==FAIL) || (strcmp(retval,thisDetector->hostname)))
			setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTNAME_NOT_SET));
	}
}



int slsDetector::enableTenGigabitEthernet(int i){
	int ret=FAIL;
	int retval = -1;
	int fnum=F_ENABLE_TEN_GIGA,fnum2 = F_ENABLE_RECEIVER_TEN_GIGA;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< std::endl<< "Enabling / Disabling 10Gbe" << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&i,sizeof(i));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				 setErrorMask((getErrorMask())|(DETECTOR_TEN_GIGA));
			}
			controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if(ret!=FAIL){
		//must also configuremac
		if((i != -1)&&(retval == i))
			if(configureMAC() != FAIL){
				ret = FAIL;
				retval=-1;
				if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
					std::cout << "Enabling / Disabling 10Gbe in receiver: " << i << std::endl;
#endif
					if (connectData() == OK){
						ret=thisReceiver->sendInt(fnum2,retval,i);
						disconnectData();
					}
					if(ret==FAIL)
						setErrorMask((getErrorMask())|(RECEIVER_TEN_GIGA));
				}
			}
	}

	if(ret != FAIL)
		thisDetector->tenGigaEnable=retval;
	return retval;
}




int slsDetector::setReceiverFifoDepth(int i){
	int fnum=F_SET_RECEIVER_FIFO_DEPTH;
	int ret = FAIL;
	int retval=-1;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		if(i ==-1)
			std::cout<< "Getting Receiver Fifo Depth" << endl;
		else
			std::cout<< "Setting Receiver Fifo Depth to " << i << endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,i);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(COULD_NOT_SET_FIFO_DEPTH));
	}
	return retval;
}





  /******** CTB funcs */

  /** opens pattern file and sends pattern to CTB 
      @param fname pattern file to open
      @returns OK/FAIL
  */
int slsDetector::setCTBPattern(string fname) {


	//int fnum=F_SET_CTB_PATTERN;
	//int ret = FAIL;
	//char retval[MAX_STR_LENGTH]="";


// 	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
// #ifdef VERBOSE
// 		std::cout << "Sending detector hostname to Receiver " << thisDetector->hostname << std::endl;
// #endif
// 		if (connectData() == OK)
// 			ret=thisReceiver->sendString(fnum,retval,thisDetector->hostname);
// 		if((ret==FAIL) || (strcmp(retval,thisDetector->hostname)))
// 			setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTNAME_NOT_SET));
// 	}



	uint64_t word;
 
     int addr=0;

     FILE *fd=fopen(fname.c_str(),"r");
     if (fd>0) {
       while (fread(&word, sizeof(word), 1,fd)) {
	setCTBWord(addr,word);
	// cout << hex << addr << " " << word << dec << endl;
	 addr++;
       }
       
       fclose(fd);
     } else
       return -1;
     




  return addr;


}

  
  /** Writes a pattern word to the CTB
      @param addr address of the word, -1 is I/O control register,  -2 is clk control register
      @param word 64bit word to be written, -1 gets
      @returns actual value
  */
uint64_t slsDetector::setCTBWord(int addr,uint64_t word) {

  //uint64_t ret;

  int ret=FAIL;
   uint64_t retval=-1;
  int fnum=F_SET_CTB_PATTERN;
  int mode=0; //sets word

  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<<"Setting CTB word" <<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&mode,sizeof(mode));
      controlSocket->SendDataOnly(&addr,sizeof(addr));
      controlSocket->SendDataOnly(&word,sizeof(word));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL)
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return retval;


}
  
  /** Sets the pattern or loop limits in the CTB
      @param level -1 complete pattern, 0,1,2, loop level
      @param start start address if >=0
      @param stop stop address if >=0
      @param n number of loops (if level >=0)
      @returns OK/FAIL
  */
int slsDetector::setCTBPatLoops(int level,int &start, int &stop, int &n) {


  int retval[3], args[4];

  args[0]=level;
  args[1]=start;
  args[2]=stop;
  args[3]=n;
  

  int ret=FAIL;
  int fnum=F_SET_CTB_PATTERN;
  int mode=1; //sets loop

  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<<"Setting CTB word" <<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&mode,sizeof(mode));
      controlSocket->SendDataOnly(&args,sizeof(args));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	start=retval[0];
	stop=retval[1];
	n=retval[2];
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return ret;


}


  /** Sets the wait address in the CTB
      @param level  0,1,2, wait level
      @param addr wait address, -1 gets
      @returns actual value
  */
int slsDetector::setCTBPatWaitAddr(int level, int addr) {




  int retval=-1;


  int ret=FAIL;
  int fnum=F_SET_CTB_PATTERN;
  int mode=2; //sets loop

  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<<"Setting CTB word" <<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&mode,sizeof(mode));
      controlSocket->SendDataOnly(&level,sizeof(level));
      controlSocket->SendDataOnly(&addr,sizeof(addr));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return retval;



}

   /** Sets the wait time in the CTB
      @param level  0,1,2, wait level
      @param t wait time, -1 gets
      @returns actual value
  */
int slsDetector::setCTBPatWaitTime(int level, uint64_t t) {





  uint64_t retval=-1;


  int ret=FAIL;
  //   uint64_t retval=-1;
  int fnum=F_SET_CTB_PATTERN;
  int mode=3; //sets loop

  char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
  std::cout<<"Setting CTB word" <<std::endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (connectControl() == OK){
      controlSocket->SendDataOnly(&fnum,sizeof(fnum));
      controlSocket->SendDataOnly(&mode,sizeof(mode));
      controlSocket->SendDataOnly(&level,sizeof(level));
      controlSocket->SendDataOnly(&t,sizeof(t));
      controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
      if (ret!=FAIL) {
	controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
      } else {
	controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	std::cout<< "Detector returned error: " << mess << std::endl;
      }
      disconnectControl();
      if (ret==FORCE_UPDATE)
	updateDetector();
    }
  }

  return retval;


}

int slsDetector::pulsePixel(int n,int x,int y) {
	int ret=FAIL;
	int fnum=F_PULSE_PIXEL;
	char mess[MAX_STR_LENGTH]="";
	int arg[3];
	arg[0] = n; arg[1] = x; arg[2] = y;

#ifdef VERBOSE
	std::cout<< std::endl<< "Pulsing Pixel " << n << " number of times at (" << x << "," << "y)" << endl << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_PULSE_PIXEL));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}


int slsDetector::pulsePixelNMove(int n,int x,int y) {
	int ret=FAIL;
	int fnum=F_PULSE_PIXEL_AND_MOVE;
	char mess[MAX_STR_LENGTH]="";
	int arg[3];
	arg[0] = n; arg[1] = x; arg[2] = y;

#ifdef VERBOSE
	std::cout<< std::endl<< "Pulsing Pixel " << n << " number of times and move by deltax:" << x << " deltay:" << y << endl << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_PULSE_PIXEL_NMOVE));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}



int slsDetector::pulseChip(int n) {
	int ret=FAIL;
	int fnum=F_PULSE_CHIP;
	char mess[MAX_STR_LENGTH]="";


#ifdef VERBOSE
	std::cout<< std::endl<< "Pulsing Pixel " << n << " number of times" << endl << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&n,sizeof(n));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_PULSE_CHIP));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}



void slsDetector::setAcquiringFlag(bool b){
	thisDetector->acquiringFlag = b;
}

bool slsDetector::getAcquiringFlag(){
	return thisDetector->acquiringFlag;
}
