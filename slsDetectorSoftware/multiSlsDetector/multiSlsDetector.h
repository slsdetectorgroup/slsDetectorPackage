/*******************************************************************

Date:       $Date$
Revision:   $Rev$
Author:     $Author$
URL:        $URL$
ID:         $Id$

********************************************************************/



#ifndef MULTI_SLS_DETECTOR_H
#define MULTI_SLS_DETECTOR_H

#include "slsDetector.h"

#include "sls_detector_defs.h"

#define MAXDET 100


//using namespace std;

   /** synchronization of the various detectors (should be set for each detector individually?!?!?) */
      
  enum synchronizationMode {
    GET_SYNCHRONIZATION_MODE=-1, /**< the multidetector will return its synchronization mode */
    NONE, /**< all detectors are independent (no cabling) */
    MASTER_GATES, /**< the master gates the other detectors */
    MASTER_TRIGGERS, /**< the master triggers the other detectors */
    SLAVE_STARTS_WHEN_MASTER_STOPS /**< the slave acquires when the master finishes, to avoid deadtime */
  };




/**
 * 
 *
@libdoc The multiSlsDetector class is used to operate several slsDetectors in parallel.
 *
 * @short This is the base class for multi detector system functionalities
 * @author Anna Bergamaschi
 * @version 0.1alpha

 */

class multiSlsDetector  {


  
  typedef  struct sharedMultiSlsDetector {




    /** already existing flag. If the detector does not yet exist (alreadyExisting=0) the sharedMemory will be created, otherwise it will simly be linked */
    int alreadyExisting;


    /** last process id accessing the shared memory */
    pid_t lastPID;


    /** online flag - is set if the detector is connected, unset if socket connection is not possible  */
    int onlineFlag;
    
  
    /** stopped flag - is set if an acquisition error occurs or the detector is stopped manually. Is reset to 0 at the start of the acquisition */
    int stoppedFlag;
    
    
    /** Number of detectors operated at once */
    int numberOfDetectors;

    /** Ids of the detectors to be operated at once */
    int detectorIds[MAXDET];


    /** Detectors offset in the X direction (in number of channels)*/
    int offsetX[MAXDET];
    /** Detectors offsets  in the Y direction (in number of channels) */
    int offsetY[MAXDET];

    /** position of the master detector */
    int masterPosition;
    
    /** type of synchronization between detectors */
    synchronizationMode syncMode;
      
    /**  size of the data that are transfered from all detectors */
    int dataBytes;
  
    /**  total number of channels for all detectors */
    int numberOfChannels;
  


    /** timer values */
    int64_t timerValue[MAX_TIMERS];
    /** detector settings (standard, fast, etc.) */
    detectorSettings currentSettings;
    /** detector threshold (eV) */
    int currentThresholdEV;

 
    /** indicator for the acquisition progress - set to 0 at the beginning of the acquisition and incremented every time that the data are written to file */   
    int progressIndex;	
    /** total number of frames to be acquired */   
    int totalProgress;	   
 


   /** current index of the output file */   
    int fileIndex;
    /** path of the output files */  
    char filePath[MAX_STR_LENGTH];
    /** name root of the output files */  
    char fileName[MAX_STR_LENGTH];
    

    /** corrections  to be applied to the data \see ::correctionFlags */
    int correctionMask;
    /** threaded processing flag (i.e. if data are processed and written to file in a separate thread)  */
    int threadedProcessing;
    /** dead time (in ns) for rate corrections */
    float tDead;



    /** directory where the flat field files are stored */
    char flatFieldDir[MAX_STR_LENGTH];
    /** file used for flat field corrections */
    char flatFieldFile[MAX_STR_LENGTH];


    /** file with the bad channels */
    char badChanFile[MAX_STR_LENGTH];

     /** array of angular conversion constants for each module \see ::angleConversionConstant */
    angleConversionConstant angOff[MAXMODS];
    /** angular direction (1 if it corresponds to the encoder direction i.e. channel 0 is 0, maxchan is positive high angle, 0 otherwise  */
    int angDirection;
     /** beamline fine offset (of the order of mdeg, might be adjusted for each measurements)  */
    float fineOffset;
     /** beamline offset (might be a few degrees beacuse of encoder offset - normally it is kept fixed for a long period of time)  */
    float globalOffset;
     /** number of positions at which the detector should acquire  */
    int numberOfPositions;
     /** list of encoder positions at which the detector should acquire */
    float detPositions[MAXPOS];
    /** bin size for data merging */
    float binSize;


    /** Scans and scripts */

    int actionMask;
    
    int actionMode[MAX_ACTIONS];
    char actionScript[MAX_ACTIONS][MAX_STR_LENGTH];
    char actionParameter[MAX_ACTIONS][MAX_STR_LENGTH];
    

    int scanMode[MAX_SCAN_LEVELS];
    char scanScript[MAX_SCAN_LEVELS][MAX_STR_LENGTH];
    char scanParameter[MAX_SCAN_LEVELS][MAX_STR_LENGTH];
    int nScanSteps[MAX_SCAN_LEVELS];
    float scanSteps[MAX_SCAN_LEVELS][MAX_SCAN_STEPS];
    int scanPrecision[MAX_SCAN_LEVELS];
    
    

  };
















 public:

 
  /** 
      @short Structure allocated in shared memory to store detector settings and be accessed in parallel by several applications (take care of possible conflicts!)
      
  */


/** (default) constructor 
    \param  id is the detector index which is needed to define the shared memory id. Different physical detectors should have different IDs in order to work independently
    
    
*/
  multiSlsDetector(int id=0);
  //slsDetector(string  const fname);
  /** destructor */ 
  virtual ~multiSlsDetector();
  
  
  /** frees the shared memory occpied by the sharedMultiSlsDetector structure */
  int freeSharedMemory() ;
  
  /** allocates the shared memory occpied for the sharedMultiSlsDetector structure */
  int initSharedMemory(int) ;
  
  /** adds the detector with ID id in postion pos
   \param id of the detector to be added (should already exist!)
   \param pos position where it should be added (normally at the end of the list (default to -1)
   \return the actual number of detectors*/
  int addSlsDetector(int id, int pos=-1, int oX=-1, int oY=-1);

  /**removes the detector in position pos from the multidetector
     \param pos position of the detector to be removed from the multidetector system (defaults to -1 i.e. last detector)
     \returns the actual number of detectors
  */
  int removeSlsDetector(int pos=-1);

  /** returns the id of the detector in position i
   \param i position of the detector
  \returns detector ID or -1 if detector in position i is empty*/
  int getDetectorId(int i) {if (i>=0) if (detectors[i]) return detectors[i]->getDetectorId(); else return -1;};


  /** returns the number of detectors in the multidetector structure
      \returns number of detectors */
  int getNumberOfDetectors() {return thisMultiDetector->numberOfDetectors;};


  /** returns the detector offset (in number of channels)
      \param pos position of the detector
      \param ox reference to the offset in x
      \param oy reference to the offset in y
      \returns OK/FAIL if the detector does not exist
  */
  int getDetectorOffset(int pos, int &ox, int &oy);

    /** sets the detector offset (in number of channels)
      \param pos position of the detector
      \param ox offset in x (-1 does not change)
      \param oy offset in y (-1 does not change)
      \returns OK/FAIL if the detector does not exist
  */
  int setDetectorOffset(int pos, int ox=-1, int oy=-1);

  

  /** sets the detector in position i as master of the structure (e.g. it gates the other detectors and therefore must be started as last. <BR> Assumes that signal 0 is gate in, signal 1 is trigger in, signal 2 is gate out
      \param i position of master (-1 gets)
      \return master's position (-1 none)
  */
  int setMaster(int i=-1);
  
  /** 
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode
      \returns current syncronization mode
  */
  synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE);




  /** sets the onlineFlag
      \param off can be:  GET_ONLINE_FLAG, returns wether the detector is in online or offline state; OFFLINE_FLAG, detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!); ONLINE_FLAG  detector in online state (i.e. communication to the detector updating the local structure) 
      \returns online/offline status
  */
  int setOnline(int const online=slsDetector::GET_ONLINE_FLAG);  
  /** sets the onlineFlag
      \returns 1 if the detector structure has already be initlialized with the given idand belongs to this multiDetector instance, 0 otherwise */
  int exists() ;

  /**
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::readConfigurationFile
  */

  int readConfigurationFile(string const fname);  
  /**  
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::writeConfigurationFile
  */
  int writeConfigurationFile(string const fname);


  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
  /** 
    
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::dumpDetectorSetup
  */
  int dumpMultiDetectorSetup(string const fname, int level);  
  /** 
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::retrieveDetectorSetup
  */
  int retrieveMultiDetectorSetup(string const fname, int level);


  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index 
       
      Filenames will be of the form: filepath/filename(_px)_i
      where x is the position index and i is the run index  
      \returns file name without extension
      \sa slsDetector::createFileName 

  */
  string createFileName();

  /**
     function that returns the file index from the file name 
      \param fname file name
      \returns file index
      \sa slsDetector::getFileIndexFromFileName 

  */
  int getFileIndexFromFileName(string fname) ;

  /**

  function that returns the variables from the file name 
  \param fname file name
      \param index reference to index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \returns file index
      \sa slsDetector::getVariablesFromFileName 

  */
  int getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1) ;













  /* I/O */

  /**
     sets the default output files path
  \sa  sharedMultiSlsDetector
  */
  char* setFilePath(string s, int i=-1) {sprintf(thisMultiDetector->filePath, s.c_str()); return thisMultiDetector->filePath;};;

  /**
     sets the default output files root name
  \sa  sharedMultiSlsDetector
  */
  char* setFileName(string s, int i=-1){sprintf(thisMultiDetector->fileName, s.c_str()); return thisMultiDetector->fileName;}; 

  /**
     sets the default output file index
  \sa  sharedMultiSlsDetector
  */
  int setFileIndex(int i, int id=-1){thisMultiDetector->fileIndex=i; return thisMultiDetector->fileIndex;}; 
  
  /**
     returns the default output files path
  \sa  sharedMultiSlsDetector
  */
  char* getFilePath(int id=-1) {return thisMultiDetector->filePath;};
  
  /**
     returns the default output files root name
  \sa  sharedMultiSlsDetector
  */
  char* getFileName(int id=-1) {return thisMultiDetector->fileName;};

  /**
     returns the default output file index
  \sa  sharedMultiSlsDetector
  */
  int getFileIndex(int id=-1) {return thisMultiDetector->fileIndex;};
  

  
    /**
     
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
 
  */
  int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1); 
  
  /**
    
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  int writeDataFile(string fname, int *data);
  
  /**
    
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not read the file or data=NULL
       
       \sa mythenDetector::readDataFile
  */
  int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=0);  

  /**
    
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
       \sa mythenDetector::readDataFile
  */
  int readDataFile(string fname, int *data);


  /**
     
      reads an angular conversion file for all detectors
      \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
  virtual int readAngularConversion(string fname="", int id=-1);



  /**
    
     writes an angular conversion file for all detectors
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
  virtual int writeAngularConversion(string fname="", int id=-1);











  /* Communication to server */















  // Expert Initialization functions
 


  
  /**
    get threshold energy
    \param imod module number (-1 all)
    \returns current threshold value for imod in ev (-1 failed)
  */
  int getThresholdEnergy(int imod=-1);  

  /**
    set threshold energy
    \param e_eV threshold in eV
    \param imod module number (-1 all)
    \param isettings ev. change settings
    \returns current threshold value for imod in ev (-1 failed)
  */
  int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS); 
 
  /**
    get detector settings
    \param imod module number (-1 all)
    \returns current settings
  */
  detectorSettings getSettings(int imod=-1);  

  /**
    set detector settings
    \param isettings  settings
    \param imod module number (-1 all)
    \returns current settings

    in this function trimbits and calibration files are searched in the trimDir and calDir directories and the detector is initialized
  */
  detectorSettings setSettings(detectorSettings isettings, int imod=-1);





























// Acquisition functions


  /**
    start detector acquisition (master is started as last)
    \returns OK if all detectors are properly started, FAIL otherwise
  */
  int startAcquisition();

  /**
    stop detector acquisition (master firtst)
    \returns OK/FAIL
  */
  int stopAcquisition();
  
  /**
    start readout (without exposure or interrupting exposure) (master first)
    \returns OK/FAIL
  */
  int startReadOut();

  /**
     get run status <BR> Does it make sense to ask the status for all detectors?!?!?!
    \returns status mask
  */
  runStatus  getRunStatus();

  /**
    start detector acquisition and read all data putting them a data queue
    \returns pointer to the front of the data queue
    \sa startAndReadAllNoWait getDataFromDetector dataQueue
  */ 
  int* startAndReadAll();
  
  /**
    start detector acquisition and read out, but does not read data from socket 
   
  */ 
  int startAndReadAllNoWait(); 

  /**
    receives a data frame from the detector socket
    \returns pointer to the data or NULL. If NULL disconnects the socket
    \sa getDataFromDetector
  */ 
  //int* getDataFromDetectorNoWait(); 
  /**
    receives a data frame from the detector socket
    \returns pointer to the data or NULL. If NULL disconnects the socket
    \sa getDataFromDetector
  */ 
  int* getDataFromDetector(); 

  /**
   asks and  receives a data frame from the detector and puts it in the data queue
    \returns pointer to the data or NULL. 
    \sa getDataFromDetector
  */ 
  int* readFrame(); 

  /**
   asks and  receives all data  from the detector  and puts them in a data queue
    \returns pointer to the front of the queue  or NULL. 
    \sa getDataFromDetector  dataQueue
  */ 
  int* readAll();

  
  /**
   pops the data from the data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  dataQueue
  */ 
  int* popDataQueue();

  /**
   pops the data from thepostprocessed data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  finalDataQueue
  */ 
  detectorData* popFinalDataQueue();




  /**
  resets the raw data queue
    \sa  dataQueue
  */ 
  void resetDataQueue();

  /**
  resets the postprocessed  data queue
    \sa  finalDataQueue
  */ 
  void resetFinalDataQueue();









  /** 
      set/get timer value
      \param index timer index
      \param t time in ns or number of...(e.g. frames, gates, probes)
      \returns timer set value in ns or number of...(e.g. frames, gates, probes)
  */
  int64_t setTimer(timerIndex index, int64_t t=-1);

/*   /\**  */
/*       get current timer value */
/*       \param index timer index */
/*       \returns elapsed time value in ns or number of...(e.g. frames, gates, probes) */
/*   *\/ */
/*   int64_t getTimeLeft(timerIndex index); */



  // Flags
  /** 
      set/get dynamic range and updates the number of dataBytes
      \param n dynamic range (-1 get)
      \param pos detector position (-1 all detectors)
      \returns current dynamic range
      updates the size of the data expected from the detector
      \sa sharedSlsDetector
  */
  int setDynamicRange(int n=-1, int pos=-1);


 
  /** 
      set roi

      not yet implemented
  */
  int setROI(int nroi=-1, int *xmin=NULL, int *xmax=NULL, int *ymin=NULL, int *ymax=NULL);
  


 
  //Corrections  

  /** 
      set/get if the data processing and file writing should be done by a separate thread
s
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
  */

  int setThreadedProcessing(int b=-1) {if (b>=0) thisMultiDetector->threadedProcessing=b; return  thisMultiDetector->threadedProcessing;}

  /** 
      set flat field corrections
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
  */
  int setFlatFieldCorrection(string fname=""); 

  /** 
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
  */
  int getFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL);

 /** 
      get flat field corrections file directory
      \returns flat field correction file directory
  */
  char *getFlatFieldCorrectionDir(){return thisMultiDetector->flatFieldDir;};
 /** 
      set flat field corrections file directory
      \param flat field correction file directory
  */
  void setFlatFieldCorrectionDir(string dir){strcpy(thisMultiDetector->flatFieldDir,dir.c_str());};
  
 /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  char *getFlatFieldCorrectionFile(){  if (thisMultiDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) return thisMultiDetector->flatFieldFile; else return "none";};

  /** 
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int setRateCorrection(float t=0);

  
  /** 
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int getRateCorrection(float &t);

  
  /** 
      get rate correction tau
      \returns 0 if rate correction disabled, otherwise the tau used for the correction
  */
  float getRateCorrectionTau();
  /** 
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int getRateCorrection();
  
  /** 
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \returns 0 if bad channel disabled, >0 otherwise
  */
  int setBadChannelCorrection(string fname="");
  
  /** 
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  int getBadChannelCorrection(int *bad=NULL);

  /** returns the bad channel list file */
  string getBadChannelCorrectionFile() {if (thisMultiDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) return string(thisMultiDetector->badChanFile); else return string("none");};

  
  /** 
      pure virtual function
      set angular conversion
      \param fname file with angular conversion constants ("" disable)
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::setAngularConversion
  */
  virtual int setAngularConversion(string fname="")=0;

  /** 
      pure virtual function
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::getAngularConversion
  */
  virtual int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL)=0;
  
  
  /**
      pure virtual function
      returns the angular conversion file
      \sa mythenDetector::getAngularConversion */
  virtual string getAngularConversion()=0;
  
  /** 
      pure virtual function
      set detector global offset
      \sa mythenDetector::setGlobalOffset
  */
  virtual float setGlobalOffset(float f)=0; 

  /** 
      pure virtual function
      set detector fine offset
      \sa mythenDetector::setFineOffset
  */
  virtual float setFineOffset(float f)=0;
  /** 
      pure virtual function
      get detector fine offset
      \sa mythenDetector::getFineOffset
  */
  virtual float getFineOffset()=0;
  
  /** 
      pure virtual function
      get detector global offset
      \sa mythenDetector::getGlobalOffset
  */
  virtual float getGlobalOffset()=0;

  /** 
      pure virtual function
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
      \sa mythenDetector::setPositions
  */
  virtual int setPositions(int nPos, float *pos)=0;
   /** 
      pure virtual function
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
      \sa mythenDetector::getPositions
  */
  virtual int getPositions(float *pos=NULL)=0;
  
  
  /** pure virtual function
      set detector bin size used for merging (approx angular resolution)
      \param bs bin size in degrees
      \returns current bin size
      \sa mythenDetector::setBinSize
*/
  virtual float setBinSize(float bs)=0;

  /** pure virtual function
      return detector bin size used for merging (approx angular resolution)
      \sa mythenDetector::getBinSize
  */
  virtual float getBinSize()=0;





  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \param par for script 
      \returns 0 if action disabled, >0 otherwise
  */
  int setAction(int iaction, string fname="", string par="");

  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionScript(int iaction, string fname="");
  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param par for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionParameter(int iaction, string par="");

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
  string getActionScript(int iaction);

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
  string getActionParameter(int iaction);

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
  int getActionMode(int iaction);


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disables, "none" disables and overwrites current)
      \param nvalues number of steps (0 disables, -1 leaves current value)
      \param values pointer to steps (if NULL leaves current values)
      \param par parameter for the scan script ("" leaves unchanged)
      \returns 0 is scan disabled, >0 otherwise
  */
  int setScan(int index, string script="", int nvalues=-1, float *values=NULL, string par="", int precision=-1);
  
  int setScanScript(int index, string script="");
  int setScanParameter(int index, string par="");
  int setScanPrecision(int index, int precision=-1);
  int setScanSteps(int index, int nvalues=-1, float *values=NULL);
  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
  string getScanScript(int iscan);

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
  string getScanParameter(int iscan);

   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
  int getScanMode(int iscan);

   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
  int getScanSteps(int iscan, float *v=NULL);


   /** 
	returns scan precision
	\param iscan can be (0,1)
	\returns scan precision
    */
  int getScanPrecision(int iscan);




  /** 
     decode data from the detector converting them to an array of floats, one for each channle
     \param datain data from the detector
     \returns pointer to a float array with a data per channel
  */
  float* decodeData(int *datain);

  
  
  
  /** 
     flat field correct data
     \param datain data
     \param errin error on data (if<=0 will default to sqrt(datain)
     \param dataout corrected data
     \param errout error on corrected data
     \param ffcoefficient flat field correction coefficient
     \param fferr erro on ffcoefficient
     \returns 0
  */
  int flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr);
  
  /** 
     flat field correct data
     \param datain data array
     \param errin error array on data (if NULL will default to sqrt(datain)
     \param dataout array of corrected data
     \param errout error on corrected data (if not NULL)
     \returns 0
  */
  int flatFieldCorrect(float* datain, float *errin, float* dataout, float *errout);
 

  
  /** 
     rate correct data
     \param datain data
     \param errin error on data (if<=0 will default to sqrt(datain)
     \param dataout corrected data
     \param errout error on corrected data
     \param tau dead time 9in ns)
     \param t acquisition time (in ns)
     \returns 0
  */
  int rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t);
  
  /** 
     rate correct data
     \param datain data array
     \param errin error array on data (if NULL will default to sqrt(datain)
     \param dataout array of corrected data
     \param errout error on corrected data (if not NULL)
     \returns 0
  */
  int rateCorrect(float* datain, float *errin, float* dataout, float *errout);

  
  /** 
      pure virtual function
  sets the arrays of the merged data to 0. NB The array should be created with size >= 360./getBinSize(); 
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns OK or FAIL
      \sa mythenDetector::resetMerging
  */
  virtual int resetMerging(float *mp, float *mv,float *me, int *mm)=0;
  /** 
      pure virtual function
  merge dataset
      \param p1 angular positions of dataset
      \param v1 data
      \param e1 errors
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \sa mythenDetector::addToMerging
  */
  virtual int addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm)=0;

  /** pure virtual function
      calculates the "final" positions, data value and errors for the emrged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns FAIL or the number of non empty bins (i.e. points belonging to the pattern)
      \sa mythenDetector::finalizeMerging
  */
  int finalizeMerging(float *mp, float *mv,float *me, int *mm);

  /** 
      turns off server
  */
  int exitServer();

  /** pure virtual function
     function for processing data
     /param delflag if 1 the data are processed, written to file and then deleted. If 0 they are added to the finalDataQueue
     \sa mythenDetector::processData
  */
  virtual void* processData(int delflag=1)=0; // thread function

  
  virtual void acquire(int delflag=1)=0;

  /** calcualtes the total number of steps of the acquisition.
      called when number of frames, number of cycles, number of positions and scan steps change
  */
  int setTotalProgress();

  /** returns the current progress in % */
  float getCurrentProgress();
  

 protected:
 

  /** Shared memory ID */
  int shmId;

  /** pointers to the slsDetector structures */
  slsDetector *detectors[MAXDET];

  /** Shared memory structure */
  sharedMultiSlsDetector *thisMultiDetector;






  /**
     data queue
  */
  queue<int*> dataQueue;
  /**
     queue containing the postprocessed data
  */
  queue<detectorData*> finalDataQueue;
  
  






  /**
     current position of the detector
  */
  float currentPosition;
  
  /**
     current position index of the detector
  */
  int currentPositionIndex;
  
  /**
     I0 measured
  */
  float currentI0;
  
  


  /**
     current scan variable of the detector
  */
  float currentScanVariable[MAX_SCAN_LEVELS];
  
  /**
     current scan variable index of the detector
  */
  int currentScanIndex[MAX_SCAN_LEVELS];
  
  


  /** merging bins */
  float *mergingBins;

  /** merging counts */
  float *mergingCounts;

  /** merging errors */
  float *mergingErrors;

  /** merging multiplicity */
  int *mergingMultiplicity;
  
 
};


#endif
