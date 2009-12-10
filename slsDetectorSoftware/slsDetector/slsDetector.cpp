#include "slsDetector.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>



using namespace std;


int slsDetector::initSharedMemory(detectorType type, int id) {


    /**
      the shared memory key is set to DEFAULT_SHM_KEY+id
   */
   key_t     mem_key=DEFAULT_SHM_KEY+id;
   int       shm_id;
   int nch, nm, nc, nd;
   int sz;


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
     nc=10;
     nd=6; // dacs+adcs
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
   shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    cout <<"*** shmget error (server) ***"<< endl;
    return shm_id;
  }
  
   /**
      thisDetector pointer is set to the memory address of the shared memory
   */

  thisDetector = (sharedSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (thisDetector == (void*)-1) {
    cout <<"*** shmat error (server) ***" << endl;
    return shm_id;
  }
    /**
      shm_id returns -1 is shared memory initialization fails
   */
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
  controlSocket(NULL),
  stopSocket(NULL),
  dataSocket(NULL),
  shmId(-1), 
  detId(0),
  thisDetector(NULL),
  badChannelMask(NULL),
  detectorModules(NULL)
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
   cout << "Detector id is " << id << endl;
#endif
   detId=id;
   /**Initializes the detector stucture \sa initializeDetectorSize
 */
   initializeDetectorSize(type);
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
       break;
     default:
       thisDetector->nChans=65536;
       thisDetector->nChips=8;
       thisDetector->nDacs=16;
       thisDetector->nAdcs=16;
       thisDetector->nModMax[X]=6;
       thisDetector->nModMax[Y]=6;  
       thisDetector->dynamicRange=32;
     }
     thisDetector->nModsMax=thisDetector->nModMax[0]*thisDetector->nModMax[1];
     /** number of modules is initally the maximum number of modules */
     thisDetector->nMod[X]=thisDetector->nModMax[X];
     thisDetector->nMod[Y]=thisDetector->nModMax[Y];  
     thisDetector->nMods=thisDetector->nModsMax;
     /** calculates the expected data size */
     if (thisDetector->dynamicRange==24)
       thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
     else
       thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*thisDetector->dynamicRange/8;
     /** set trimDsdir, calDir and filePath to default to home directory*/
     strcpy(thisDetector->trimDir,getenv("HOME"));
     strcpy(thisDetector->calDir,getenv("HOME"));
     strcpy(thisDetector->filePath,getenv("HOME"));
     /** sets trimbit file */
     strcpy(thisDetector->trimFile,"none");
     /** set fileName to default to run*/
     strcpy(thisDetector->fileName,"run");
     /** set fileIndex to default to 0*/
     thisDetector->fileIndex=0;

     /** set number of trim energies to 0*/
     thisDetector->nTrimEn=0;
     /** set correction mask to 0*/
     thisDetector->correctionMask=0;
     /** set deat time*/
     thisDetector->tDead=0;
     /** sets bad channel list file to none */
     strcpy(thisDetector->badChanFile,"none");
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
     cout << hex << detectorModules+imod << dec <<endl;
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
  char *ptr, *p1, *p2;
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
  cout << "chip structure sent" << endl;
  cout << "now sending " << myChip->nchan << " channles" << endl;
#endif

  ts=controlSocket->SendDataOnly(myChip->chanregs,sizeof(int)*myChip->nchan );

#ifdef VERBOSE
  cout << "chip's channels sent " <<ts  << endl;
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
  cout << "received module " << myMod->module << " of size "<< ts << " register " << myMod->reg << endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(float)*(myMod->ndac));
#ifdef VERBOSE
  cout << "received dacs " << myMod->module << " of size "<< ts << endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(float)*(myMod->nadc));
#ifdef VERBOSE
  cout << "received adcs " << myMod->module << " of size "<< ts << endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->chipregs,sizeof(int)*(myMod->nchip));
#ifdef VERBOSE
  cout << "received chips " << myMod->module << " of size "<< ts << endl;
#endif
  ts+=controlSocket->ReceiveDataOnly(myMod->chanregs,sizeof(int)*(myMod->nchan));
#ifdef VERBOSE
  cout << "nchans= " << thisDetector->nChans << " nchips= " << thisDetector->nChips;
  cout << "mod - nchans= " << myMod->nchan << " nchips= " <<myMod->nchip;
  
  cout << "received chans " << myMod->module << " of size "<< ts << endl;
#endif
#ifdef VERBOSE
  cout << "received module " << myMod->module << " of size "<< ts << " register " << myMod->reg << endl;
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
    cout << "setting hostname" << endl;
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
    cout << "setting control port" << endl;
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
    cout << "setting stop port" << endl;
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
    cout << "setting data port" << endl;
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
      cout << "Could not connect Control socket " << thisName  << " " << thisCP << endl;
#endif 
      retval=FAIL;
    }
#ifdef VERYVERBOSE
    else
      cout << "Control socket connected " <<thisName  << " " << thisCP << endl;
#endif
  }
  if (!stopSocket) {
    stopSocket=new MySocketTCP(thisName, thisSP);
    if (stopSocket->getErrorStatus()){
#ifdef VERBOSE
    cout << "Could not connect Stop socket "<<thisName  << " " << thisSP << endl;
#endif
    retval=FAIL;
    } 
#ifdef VERYVERBOSE
    else
      cout << "Stop socket connected " << thisName << " " << thisSP << endl;
#endif
  }
  if (!dataSocket) {
  dataSocket=new MySocketTCP(thisName, thisDP);
  if (dataSocket->getErrorStatus()){
#ifdef VERBOSE
    cout << "Could not connect Data socket "<<thisName  << " " << thisDP << endl;
#endif
    retval=FAIL;
  } 
#ifdef VERYVERBOSE
  else
    cout << "Data socket connected "<< thisName << " " << thisDP << endl;
#endif
  }
  if (retval!=FAIL) {
#ifdef VERBOSE
    cout << "online!" << endl;
#endif
    thisDetector->onlineFlag=ONLINE_FLAG;
  } else {
    thisDetector->onlineFlag=OFFLINE_FLAG;
#ifdef VERBOSE
    cout << "offline!" << endl;
#endif
    
  }


  return retval;
};





  /* I/O */

/* generates file name without extension*/

string slsDetector::createFileName() {
  

  ostringstream osfn;
  /*directory name +root file name */
  osfn << thisDetector->filePath << "/" << thisDetector->fileName;
  
  if (currentPositionIndex>0 && currentPositionIndex<=thisDetector->numberOfPositions)
    osfn << "_p" << currentPositionIndex;

  osfn << "_" << thisDetector->fileIndex;

  return osfn.str();

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
  cout << endl;
  cout << "Sending command " << arg << endl; 
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
    cout << "Detector answer is " << answer << endl; 
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

  int ret=FAIL;

#ifdef VERBOSE
  cout << endl;
  cout << "Setting detector type to " << arg << endl; 
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
	  cout << "Detector returned error: " << mess << endl;
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
  cout << "Detector type set to " << retType << endl; 
#endif
  if (retval==FAIL) {
    cout << "Set detector type failed " << endl;
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

void slsDetector::getDetectorType(char *type){

  switch (thisDetector->myDetectorType) {
  case MYTHEN:
    strcpy(type,"Mythen");
    break;
  case PILATUS:
    strcpy(type,"Pilatus");
    break;
  case EIGER:
    strcpy(type,"Eiger");
    break;
  case GOTTHARD:
    strcpy(type,"Gotthard");
    break;
  case AGIPD:
    strcpy(type,"Agipd");
    break;
  default:
    strcpy(type,"Unknown");
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
    cout << "Set number of modules in wrong dimension " << d << endl;
    return ret;
  }


#ifdef VERBOSE
  cout << endl;
  cout << "Setting number of modules of dimension "<< d <<  " to " << n << endl; 
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
	  cout << "Deterctor returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=OK;
    if (n==GET_FLAG)
      retval=thisDetector->nMod[d];
    else {
      if (n<=0 || n>thisDetector->nModMax[d]) {
	retval=thisDetector->nMod[d];
	ret=FAIL;
      } else
	retval=thisDetector->nMod[d];
    }
  }
#ifdef VERBOSE
    cout << "Number of modules in dimension "<< d <<" is " << retval << endl;
#endif
    if (ret==FAIL) {
      cout << "Set number of modules failed " << endl;
    }  else {
      thisDetector->nMod[d]=retval;
      thisDetector->nMods=thisDetector->nMod[X]*thisDetector->nMod[Y];
      int dr=thisDetector->dynamicRange;
      if (dr==24)
	dr=32;
      
      thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*dr/8;
#ifdef VERBOSE
      cout << "Data size is " << thisDetector->dataBytes << endl;
      cout << "nModX " << thisDetector->nMod[X] << " nModY " << thisDetector->nMod[Y] << " nChips " << thisDetector->nChips << " nChans " << thisDetector->nChans<< " dr " << dr << endl;
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
    cout << "Get max number of modules in wrong dimension " << d << endl;
    return ret;
  }
#ifdef VERBOSE
  cout << endl;
  cout << "Getting max number of modules in dimension "<< d  <<endl; 
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
	  cout << "Deterctor returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=OK;
    retval=thisDetector->nModMax[d];
  }
#ifdef VERBOSE
    cout << "Max number of modules in dimension "<< d <<" is " << retval << endl;
#endif
    if (ret==FAIL) {
      cout << "Get max number of modules failed " << endl;
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
  cout << endl;
  cout << "Setting signal "<< signalindex <<  " to flag" << pol << endl; 
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
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    retval=GET_EXTERNAL_SIGNAL_FLAG;
    ret=FAIL;
  }
#ifdef VERBOSE
  cout << "Signal "<< signalindex <<  " flag set to" << retval << endl;
  if (ret==FAIL) {
    cout << "Set signal flag failed " << endl;
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
  cout << endl;
  cout << "Setting communication to mode " << pol << endl; 
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
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    retval=GET_EXTERNAL_COMMUNICATION_MODE;
    ret=FAIL;
  }
#ifdef VERBOSE
  cout << "Communication mode "<<   " set to" << retval << endl;
#endif
  if (ret==FAIL) {
    cout << "Setting communication mode failed" << endl;
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
  cout << endl;
  cout << "Getting id of "<< mode << endl; 
#endif
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
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=FAIL;
  }
  if (ret==FAIL) {
    cout << "Get id failed " << endl;
    return ret;
  } else {
#ifdef VERBOSE
  cout << "Id "<< mode <<" is " << hex <<retval << setbase(10) << endl;
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
  cout << endl;
  cout << "Getting id of "<< mode << endl; 
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
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } else {
    ret=FAIL;
  }
#ifdef VERBOSE
    cout << "Id "<< mode <<" is " << retval << endl;
#endif
  if (ret==FAIL) {
    cout << "Get id failed " << endl;
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
  cout << "function not yet implemented " << endl;
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
  cout << "function not yet implemented " << endl;
};
*/
  /* 
     give a train of calibration pulses 
  */ 
/*
int slsDetector::giveCalibrationPulse(float vcal, int npulses){
  cout << "function not yet implemented " << endl;
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
  cout << endl;
  cout << "Writing to register "<< addr <<  " data " << val << endl; 
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
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } 
#ifdef VERBOSE
  cout << "Register returned "<< retval << endl;
#endif
  if (ret==FAIL) {
    cout << "Write to register failed " << endl;
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
  cout << endl;
  cout << "Reding register "<< addr  << endl; 
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
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
    }
  } 
#ifdef VERBOSE
  cout << "Register returned "<< retval << endl;
#endif
  if (ret==FAIL) {
    cout << "Read register failed " << endl;
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
  cout << endl;
  cout << "Setting DAC "<< index << "of module " << imod  <<  " to " << val << endl; 
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
	  if (dacs) {
	    if (imod>=0) {
	      *(dacs+index+imod*thisDetector->nDacs)=retval;
	    }
	    else {
	      for (imod=0; imod<thisDetector->nModsMax; imod++)
		*(dacs+index+imod*thisDetector->nDacs)=retval;
	    }
	  } 
	} else {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  cout << "Detector returned error: " << mess << endl;
	}
	controlSocket->Disconnect();
      }
	
    }
  }
#ifdef VERBOSE
  cout << "Dac set to "<< retval << endl;
#endif
  if (ret==FAIL) {
    cout << "Set dac failed " << endl;
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
  cout << endl;
  cout << "Getting ADC "<< index << "of module " << imod  <<   endl; 
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
	  cout << "Detector returned error: " << mess << endl;
	  }
	controlSocket->Disconnect();
      }
    }
  } 
#ifdef VERBOSE
  cout << "ADC returned "<< retval << endl;
#endif
  if (ret==FAIL) {
    cout << "Get ADC failed " << endl;
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
  cout << "Setting channel "<< ichan << " " << ichip << " " << imod << " to " << reg << endl;
#endif
  int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan, chamax=ichan+1;

  int ret;

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
	myChan.chan=icha;
	myChan.chip=ichi;
	myChan.module=im;
	myChan.reg=reg;
	ret=setChannel(myChan);	 
      }
    }
  }
  return ret;
}



int slsDetector::setChannel(sls_detector_channel chan){
  int fnum=F_SET_CHANNEL;
  int retval;
  int ret=FAIL;
  char mess[100];

  int icha=chan.chan;
  int ichi=chan.chip;
  int im=chan.module;

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
	cout << "Detector returned error: " << mess << endl;
      }
      controlSocket->Disconnect();
    }
    }
  }


  if (ret==OK) {
    if (chanregs) {
      *(chanregs+im*thisDetector->nChans*thisDetector->nChips+ichi*thisDetector->nChips+icha)=retval;      
    }
  }
#ifdef VERBOSE
  cout << "Channel register returned "<<  retval << endl;
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
	cout << "Detector returned error: " << mess << endl;
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
  cout << "Returned channel "<< ichan << " " << ichip << " " << imod << " " <<  myChan.reg << endl;
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
  cout << "Setting chip "<<  ichip << " " << imod << " to " << reg <<  endl;
#endif


  int chregs[thisDetector->nChans];
  int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1;
  int ret;
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
	cout << "Detector returned error: " << mess << endl;
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
  cout << "Chip register returned "<<  retval << endl;
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
	cout << "Detector returned error: " << mess << endl;
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
  cout << "Returned chip "<<  ichip << " " << imod << " " <<  myChip.reg << endl;
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
  sls_detector_module myModule, *mptr;
  
  int charegs[thisDetector->nChans*thisDetector->nChips];
  int chiregs[thisDetector->nChips];
  float das[thisDetector->nDacs], ads[thisDetector->nAdcs];
  int mmin=imod, mmax=imod+1;
  int ret;
  
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

  int im=module.module;





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
	cout << "Detector returned error: " << mess << endl;
      }
      controlSocket->Disconnect();
    }
    }
  }

  
  if (ret==OK) {
    if (detectorModules)
      (detectorModules+im)->reg=retval;
  }

#ifdef VERBOSE
  cout << "Module register returned "<<  retval << endl;
#endif

  return retval;



};

sls_detector_module  *slsDetector::getModule(int imod){


  int fnum=F_GET_MODULE;
  sls_detector_module *myMod=createModule();


  char *ptr,  *goff=(char*)thisDetector;

  // int chanreg[thisDetector->nChans*thisDetector->nChips];
  //int chipreg[thisDetector->nChips];
  //float dac[thisDetector->nDacs], adc[thisDetector->nAdcs];

  int ret=FAIL;
  char mess[100];
  int n;

#ifdef VERBOSE
  cout << "getting module " << imod << endl;
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
	cout << "Detector returned error: " << mess << endl;
      }
      controlSocket->Disconnect();
    }
    }
  }
  

  if (ret==OK) {


    if (detectorModules) {
#ifdef VERBOSE
      cout << hex << detectorModules+imod <<dec<< endl;
#endif
      (detectorModules+imod)->nchan=myMod->nchan;
      (detectorModules+imod)->nchip=myMod->nchip;
      (detectorModules+imod)->ndac=myMod->ndac;
      (detectorModules+imod)->nadc=myMod->nadc;
#ifdef VERBOSE
      cout << " before " << thisDetector->nChans << " " << thisDetector->nChips << endl << " chan " << myMod->nchan << " chip " << myMod->nchip<< endl;
#endif

      thisDetector->nChips=myMod->nchip;
      thisDetector->nChans=myMod->nchan/myMod->nchip;

#ifdef VERBOSE
      cout << " after " <<  thisDetector->nChans << " " << thisDetector->nChips<< endl;
#endif

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

  } else {
    deleteModule(myMod);
    myMod=NULL;
  }



#ifdef VERBOSE
  cout << "Returned module "<<   myMod->module << " " <<  myMod->reg << endl;
#endif
  return myMod;
}




  // calibration functions
/*
  really needed?

int slsDetector::setCalibration(int imod,  detectorSettings isettings, float gain, float offset){
  cout << "function not yet implemented " << endl; 
  
  

  return OK;

}
int slsDetector::getCalibration(int imod,  detectorSettings isettings, float &gain, float &offset){

  cout << "function not yet implemented " << endl; 



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
  cout << "Getting threshold energy "<< endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&imod,sizeof(imod));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  cout << "Detector returned error: "<< endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  cout <<  mess << endl;
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
  cout << "Getting threshold energy "<< endl;
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
	  cout << "Detector returned error: "<< endl;
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  cout <<  mess << endl;
	} else {
#ifdef VERBOSE
	  cout << "Detector returned OK "<< endl;
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
  cout << "Getting settings "<< endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(arg,sizeof(arg));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  cout << "Detector returned error: " << mess << endl;
	} else{
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  thisDetector->currentSettings=(detectorSettings)retval;
#ifdef VERBOSE
	  cout << "Settings are "<< retval << endl;
#endif
    }
	controlSocket->Disconnect();
      }
    }
  }
  return thisDetector->currentSettings;

};

 detectorSettings slsDetector::setSettings( detectorSettings isettings, int imod){
  sls_detector_module *myMod=createModule();
  int modmi=imod, modma=imod+1, im=imod;
  string trimfname, calfname;
  string ssettings;

  if (isettings>=STANDARD && isettings<=HIGHGAIN) {
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
    default:
      cout << "Unknown settings!" << endl;
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
      ostfn << thisDetector->trimDir << ssettings <<"/noise.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      oscfn << thisDetector->calDir << ssettings << "/calibration.sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im) << setbase(10); 
      //
      trimfname=ostfn.str();
      if (readTrimFile(trimfname,myMod)) {
	calfname=oscfn.str();
	readCalibrationFile(calfname,myMod->gain, myMod->offset);
	setModule(*myMod);
      } else {
	ostringstream ostfn,oscfn;
	ostfn << thisDetector->trimDir << ssettings <<"/noise.snxxx"  ; 
	oscfn << thisDetector->calDir << ssettings << "/calibration.snxxx";
	trimfname=ostfn.str();
	if (readTrimFile(trimfname,myMod)) {
	  calfname=oscfn.str();
	  readCalibrationFile(calfname,myMod->gain, myMod->offset);
	  setModule(*myMod);
	}
      }
    }
  }
  deleteModule(myMod);
  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
    int isett=getSettings(imod);
    float t[]=defaultTDead;
    if (isett>-1 && isett<3) {
      thisDetector->tDead=t[isett];
    }
  }


  return  getSettings(imod);
};

// Acquisition functions
/* change these funcs accepting also ok/fail */

int slsDetector::startAcquisition(){


  int fnum=F_START_ACQUISITION;
  int ret=FAIL;
  char mess[100];

#ifdef VERBOSE
  cout << "Starting acquisition "<< endl;
#endif
  thisDetector->stoppedFlag=0;
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  cout << "Detector returned error: " << mess << endl;
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
  cout << "Stopping acquisition "<< endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (stopSocket) {
  if  (stopSocket->Connect()>=0) {
    stopSocket->SendDataOnly(&fnum,sizeof(fnum));
    stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      stopSocket->ReceiveDataOnly(mess,sizeof(mess));
      cout << "Detector returned error: " << mess << endl;
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
  cout << "Starting readout "<< endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      cout << "Detector returned error: " << mess << endl;
    }
    controlSocket->Disconnect();
  }
    }
  }
  return ret;
};



int slsDetector::getRunStatus(){
  int fnum=F_GET_RUN_STATUS;
  int retval;
  int ret=FAIL;
  char mess[100];
#ifdef VERBOSE
  cout << "Getting status "<< endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      cout << "Detector returned error: " << mess << endl;
    } else
      controlSocket->ReceiveDataOnly(&retval,sizeof(retval));    
    controlSocket->Disconnect();
  }
    }
  }
  return retval;


};

int* slsDetector::readFrame(){

  int fnum=F_READ_FRAME, n;
  int* retval=NULL;

#ifdef VERBOSE
  cout << "slsDetector: Reading frame "<< endl;
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
  //  cout << "getting data "<< endl;
#endif
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      n= controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      if (ret==FAIL) {
	thisDetector->stoppedFlag=1;
	cout << "Detector returned: " << mess << " " << n << endl;
      } else {
	;
#ifdef VERBOSE
	cout << "Detector successfully returned: " << mess << " " << n << endl;
#endif	
      } 
      delete [] retval;
      retval=NULL;
    } else {
      n=controlSocket->ReceiveDataOnly(retval,thisDetector->dataBytes);
	
#ifdef VERBOSE
      cout << "Received "<< n << " data bytes" << endl;
#endif 
      if (n!=thisDetector->dataBytes) {
	cout << "wrong data size received: received " << n << " but expected " << thisDetector->dataBytes << endl;
	thisDetector->stoppedFlag=1;
	ret=FAIL;
	delete [] retval;
	retval=NULL;
      }
    }

    return retval;
};



int* slsDetector::readAll(){
  
  int fnum=F_READ_ALL, n;
  int* retval; // check what we return!
  int ret=OK;
  char mess[100];

  int i=0;
#ifdef VERBOSE
  cout << "Reading all frames "<< endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    while (retval=getDataFromDetector()){
      i++;
#ifdef VERBOSE
      // cout << i << endl;
#endif
      dataQueue.push(retval);
    }
    controlSocket->Disconnect();
  }
    }
  }
#ifdef VERBOSE
  cout << "received "<< i<< " frames" << endl;
#endif
  return dataQueue.front(); // check what we return!

};

int* slsDetector::startAndReadAll(){

 
  int* retval;
  int i=0;
  startAndReadAllNoWait();  
  while (retval=getDataFromDetector()){
      i++;
#ifdef VERBOSE
      //   cout << i << endl;
#endif
      dataQueue.push(retval);
  }
  controlSocket->Disconnect();

#ifdef VERBOSE
  cout << "recieved "<< i<< " frames" << endl;
#endif
  return dataQueue.front(); // check what we return!
/* while ((retval=getDataFromDetectorNoWait()))
   i++;
   #ifdef VERBOSE
  cout << "Received " << i << " frames"<< endl;
#endif
  return dataQueue.front(); // check what we return!
  */
  
};



int slsDetector::startAndReadAllNoWait(){

  int fnum= F_START_AND_READ_ALL;
  int* retval;
  int ret=OK;
  char mess[100];
  int i=0;
  
#ifdef VERBOSE
  cout << "Starting and reading all frames "<< endl;
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
  cout << "Run finished "<< endl;
#endif
  } else {
#ifdef VERBOSE
    cout << "Frame received "<< endl;
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
  cout << "Setting timer  "<< index << " to " <<  t << "ns" << endl;
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
	  cout << "Detector returned error: " << mess << endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
	  thisDetector->timerValue[index]=retval; 
	} 
	controlSocket->Disconnect();
      }
    }
  } else {
    //cout << "offline " << endl;
    if (t>=0)
      thisDetector->timerValue[index]=t;
  }
#ifdef VERBOSE
  cout << "Timer set to  "<< thisDetector->timerValue[index] << "ns"  << endl;
#endif
  return thisDetector->timerValue[index];
  
};

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
  cout << "Setting speed  variable"<< sp << " to " <<  value << endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
      if  (controlSocket->Connect()>=0) {
	controlSocket->SendDataOnly(&fnum,sizeof(fnum));
	controlSocket->SendDataOnly(&sp,sizeof(sp));
	n=controlSocket->SendDataOnly(&value,sizeof(value));
#ifdef VERBOSE
	cout << "Sent  "<< n << " bytes "  << endl;
#endif
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret!=OK) {
	  controlSocket->ReceiveDataOnly(mess,sizeof(mess));
	  cout << "Detector returned error: " << mess << endl;
	} else {
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));  
	} 
	controlSocket->Disconnect();
      }
    }
  }
#ifdef VERBOSE
  cout << "Speed set to  "<< retval  << endl;
#endif
  return retval;
  
}
















int64_t slsDetector::getTimeLeft(timerIndex index){
  

  int fnum=F_GET_TIME_LEFT;
  int64_t retval;
  char mess[100];
  int ret=OK;

#ifdef VERBOSE
  cout << "Getting  timer  "<< index <<  endl;
#endif
  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    controlSocket->SendDataOnly(&index,sizeof(index));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      cout << "Detector returned error: " << mess << endl;
    } else {
      controlSocket->ReceiveDataOnly(&retval,sizeof(retval)); 
      thisDetector->timerValue[index]=retval;
    }   
    controlSocket->Disconnect();
  }
    }
  }
#ifdef VERBOSE
  cout << "Time left is  "<< retval << endl;
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
  cout << "Setting dynamic range to "<< n << endl;
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
	  cout << "Detector returned error: " << mess << endl;
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
    thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*retval/8;
    if (retval==32)
      thisDetector->dynamicRange=24;
    else 
      thisDetector->dynamicRange=retval;
    
    
#ifdef VERBOSE
    cout << "Dynamic range set to  "<< thisDetector->dynamicRange   << endl;
    cout << "Data bytes "<< thisDetector->dataBytes   << endl;
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
  cout << "Setting readout flags to "<< flag << endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
   controlSocket->SendDataOnly(&flag,sizeof(flag));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      cout << "Detector returned error: " << mess << endl;
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
  cout << "Readout flag set to  "<< retval   << endl;
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
  int retval;
  char mess[100];
  int ret=OK;
  int arg[3];
  arg[0]=imod;
  arg[1]=par1;
  arg[2]=par2;


#ifdef VERBOSE
  cout << "Trimming module " << imod << " with mode "<< mode << " parameters " << par1 << " " << par2 << endl;
#endif

  if (thisDetector->onlineFlag==ONLINE_FLAG) {
    if (controlSocket) {
  if  (controlSocket->Connect()>=0) {
    controlSocket->SendDataOnly(&fnum,sizeof(fnum));
    cout << "sending mode bytes= "<<  controlSocket->SendDataOnly(&mode,sizeof(mode)) << endl;
    controlSocket->SendDataOnly(arg,sizeof(arg));
    controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
    if (ret!=OK) {
      controlSocket->ReceiveDataOnly(mess,sizeof(mess));
      cout << "Detector returned error: " << mess << endl;
    } else {
#ifdef VERBOSE
      cout << "Detector trimmed "<< ret   << endl;
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
  const char one=1;
  const int bytesize=8;

  int ival=0;
  char *ptr=(char*)datain;
  char iptr;

  int nbits=thisDetector->dynamicRange;
  int  ipos=0, ichan=0, ibyte;
  int nch, boff=0;

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
	  cout << "error: decoding too many channels!" << ichan;
	  break;
	}
      }
    }
  }
  }
  */
#ifdef VERBOSE
  cout << "decoded "<< ichan << " channels" << endl;
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
  int interrupt=0;
  float data[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
  float err[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
   float xmed[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*thisDetector->nChans*thisDetector->nChips];
   int nmed=0;
   int im=0;
   int nch;
   thisDetector->nBadFF=0;

  if (fname=="") {
#ifdef VERBOSE
   cout << "disabling flat field correction" << endl;
#endif
    thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
    strcpy(thisDetector->flatFieldFile,"none");
  } else { 
#ifdef VERBOSE
   cout << "Setting flat field correction from file " << fname << endl;
#endif
    nch=readDataFile(fname,data);
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
	    cout << "Channel " << ichan << " added to the bad channel list" << endl;
#endif
	  } else
	    cout << "Too many bad channels " << endl;
	    
	  }
	}
    
      if (nmed>1 && xmed[nmed/2]>0) {
#ifdef VERBOSE
	cout << "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << endl;
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
	cout << "Flat field data from file " << fname << " are not valid " << endl;
	return -1;
      }
    } else {
	cout << "Flat field from file " << fname << " is not valid " << endl;
	return -1;
    } 
  }
  return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}
 
int slsDetector::getFlatFieldCorrection(float *corr, float *ecorr) {
  if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
#ifdef VERBOSE
    cout << "Flat field correction is enabled" << endl;
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
    cout << "Flat field correction is disabled" << endl;
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
    cout << "Flat field correcting data" << endl;
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
    cout << "unsetting rate correction" << endl;
#endif
    thisDetector->correctionMask&=~(1<<RATE_CORRECTION);
  } else {
    thisDetector->correctionMask|=(1<<RATE_CORRECTION);
    if (t>0)
      thisDetector->tDead=t;
    else {
      if (thisDetector->currentSettings<3 && thisDetector->currentSettings>-1)
	thisDetector->tDead=tdead[thisDetector->currentSettings];
    }
#ifdef VERBOSE
    cout << "Setting rate correction with dead time "<< thisDetector->tDead << endl;
#endif
  }
  return thisDetector->correctionMask&(1<<RATE_CORRECTION);
}


int slsDetector::getRateCorrection(float &t){

  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    cout << "Rate correction is enabled with dead time "<< thisDetector->tDead << endl;
#endif
    t=thisDetector->tDead;
    return 1;
  } else
#ifdef VERBOSE
    cout << "Rate correction is disabled " << endl;
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

   float data;
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

};


int slsDetector::rateCorrect(float* datain, float *errin, float* dataout, float *errout){
  float tau=thisDetector->tDead;
  float t=thisDetector->timerValue[ACQUISITION_TIME];
  float data;
  float e;
  if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    cout << "Rate correcting data with dead time "<< tau << " and acquisition time "<< t << endl;
#endif
    for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*thisDetector->nChans*thisDetector->nChips; ichan++) {
      
      if (errin==NULL) {
	e=sqrt(datain[ichan]);
      } else
	e=errin[ichan];
      
      rateCorrect(datain[ichan], e, dataout[ichan], errout[ichan], tau, t);
    }
  }
  
  
};


int slsDetector::setBadChannelCorrection(string fname){
  ifstream infile;
  string str;
  int interrupt=0;
  int ich;
  int chmin,chmax;
  if (fname=="") {
    thisDetector->correctionMask&=~(1<< DISCARD_BAD_CHANNELS);
    thisDetector->nBadChans=0;
  } else { 
    if (fname=="default")
      fname=string(thisDetector->badChanFile);
    infile.open(fname.c_str(), ios_base::in);
    thisDetector->nBadChans=0;
    while (infile.good() and interrupt==0) {
      getline(infile,str);
      istringstream ssstr;
      if (str.find('-')) {
	ssstr.str(str);
	ssstr >> chmin ;
	ssstr.str(str.substr(str.find('-')+1,str.size()));
	ssstr >> chmax;
	for (ich=chmin; ich<=chmax; ich++) {
	  thisDetector->badChansList[thisDetector->nBadChans]=ich;
	  thisDetector->nBadChans++;
#ifdef VERBOSE
	  cout << thisDetector->nBadChans << " Found bad channel "<< ich << endl;
#endif
	}
      } else {
	ssstr.str(str);
	ssstr >> ich;
	if (thisDetector->nBadChans<MAX_BADCHANS) {
	  thisDetector->badChansList[thisDetector->nBadChans]=ich;
	  thisDetector->nBadChans++;
#ifdef VERBOSE
	  cout << thisDetector->nBadChans << " Found bad channel "<< ich << endl;
#endif
	} else
	  interrupt=1;
      }


      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
      
    }
    if (thisDetector->nBadChans>0 && thisDetector->nBadChans<MAX_BADCHANS) {
      thisDetector->correctionMask|=(1<< DISCARD_BAD_CHANNELS);
      strcpy(thisDetector->badChanFile,fname.c_str());
    }
  }
  fillBadChannelMask();
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
      if (thisDetector->badChansList[ichan]<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods)
	badChannelMask[ichan]=1;
    }
    for (int ichan=0; ichan<thisDetector->nBadFF; ichan++) {
      if (thisDetector->badFFList[ichan]<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods)
	badChannelMask[ichan]=1;
    }

  } else {
    if (badChannelMask)
      delete [] badChannelMask;
  }
    

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
  if (retval==OK) {
    cout << endl;
    cout << "Shutting down the server" << endl;
    cout << endl;
  }
  return retval;
};



/*
void slsDetector::startThread() {
  pthread_attr_t tattr, mattr;
  int ret;
  int newprio;
  sched_param param, mparam;
  void *arg;
  int policy= SCHED_OTHER;

  ret = pthread_attr_init(&tattr);

  // set the priority; others are unchanged
  //newprio = 30;
  mparam.sched_priority = 30;
  // scheduling parameters of main thread 
  ret = pthread_setschedparam(pthread_self(), policy, &mparam);


  printf("current priority is %d\n",param.sched_priority);
  ret = pthread_create(&dataProcessingThread, NULL,startProcessData, (void*)this);


  param.sched_priority = 1;
  // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);


}

void* startProcessData(void *n) {

  //void* processData(void *n) {
  void *w;
  slsDetector *myDet=(slsDetector*)n;
  myDet->processData(0);
}
*/
