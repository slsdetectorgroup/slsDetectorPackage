
#ifndef SLS_DETECTOR_BASE_H
#define SLS_DETECTOR_BASE_H


#include "sls_detector_defs.h"

#include <string>


using namespace std;
/** 

This class contains the functions accessible by the users to control the slsDetectors (both multiSlsDetector and slsDetector)

*/


class slsDetectorBase { 

 public:

  /** default constructor */
   slsDetectorBase(){};


   /** virtual destructor */
   virtual ~slsDetectorBase(){};

  /** sets the onlineFlag
      \param off can be: <BR> GET_ONLINE_FLAG, returns wether the detector is in online or offline state;<BR> OFFLINE_FLAG, detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!);<BR> ONLINE_FLAG  detector in online state (i.e. communication to the detector updating the local structure) 
      \returns ONLINE_FLAG or OFFLINE_FLAG
  */
  virtual int setOnline(int const online=GET_ONLINE_FLAG)=0;

 

  /** performs a complete acquisition including scansand data processing 
     moves the detector to next position <br>
     starts and reads the detector <br>
     reads the IC (if required) <br>
     reads the encoder (iof required for angualr conversion) <br>
     processes the data (flat field, rate, angular conversion and merging ::processData())
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
  */
  virtual void acquire(int delflag)=0;


  /**
   asks and  receives all data  from the detector  and puts them in a data queue
    \returns pointer to the front of the queue  or NULL. 
    \sa getDataFromDetector  dataQueue
  */ 
  virtual int* readAll()=0;  

  /**
   asks and  receives a data frame from the detector and puts it in the data queue
    \returns pointer to the data or NULL. 
    \sa getDataFromDetector
  */ 
  virtual int* readFrame()=0;

  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  virtual void* processData(int delflag)=0;


  /**
    start detector acquisition
    \returns OK/FAIL
  */
  virtual int startAcquisition()=0;
  /**
    stop detector acquisition
    \returns OK/FAIL
  */
  virtual int stopAcquisition()=0;

  /**
     get run status
    \returns status mask
  */
  virtual runStatus getRunStatus()=0;

  /** Frees the shared memory  -  should not be used except for debugging*/
  virtual int freeSharedMemory()=0;

  /** adds the detector with ID id in postion pos
   \param id of the detector to be added (should already exist!)
   \param pos position where it should be added (normally at the end of the list (default to -1)
   \returns the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int addSlsDetector(int id, int pos=-1){return -1;};


  /** adds the detector name in position pos
   \param name of the detector to be added (should already exist in shared memory or at least be online) 
   \param pos position where it should be added (normally at the end of the list (default to -1)
   \return the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int addSlsDetector(char* name, int pos=-1){return -1;};


  /**
     removes the detector in position pos from the multidetector
     \param pos position of the detector to be removed from the multidetector system (defaults to -1 i.e. last detector)
     \returns the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int removeSlsDetector(int pos=-1){return -1;};

 /**removes the detector in position pos from the multidetector
    \param name is the name of the detector
    \returns the actual number of detectors or -1 if it failed  (always for slsDetector)
 */
  virtual int removeSlsDetector(char* name){return -1;};

  
  
  /** returns the detector hostname 
      \param pos position in the multi detector structure (is -1 returns concatenated hostnames divided by a +)
      \retruns hostname
  */
  virtual string getHostname(int pos=-1)=0;

  
  /** sets the detector hostname   
      \param name hostname
      \param pos position in the multi detector structure (is -1 expects concatenated hostnames divided by a +)
      \returns  hostname  
  */
  virtual string setHostname(char* name, int pos=-1)=0;

  /** Gets the detector id (shared memory id) of an slsDetector
      \param i position in the multiSlsDetector structure
      \return id or -1 if FAIL
  */
  virtual int getDetectorId(int i=-1) =0;

  /** Sets the detector id (shared memory id) of an slsDetector in a multiSlsDetector structure
      \param ival id to be set
      \param i position in the multiSlsDetector structure
      \return id or -1 if FAIL  (e.g. in case of an slsDetector)
  */
  virtual int setDetectorId(int ival, int i=-1){return -1;};

 /** sets/gets position of the master in a multi detector structure
      \param i position of the detector in the multidetector structure
      \returns position of the master in a multi detector structure (-1 no master or always in slsDetector)
  */
  virtual int setMaster(int i=-1){return -1;};

  /** 
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode
      \returns current syncronization mode   
  */   
  virtual synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE)=0;

  /** 
      Turns off the server -  do not use except for debugging!
  */   
  virtual int exitServer()=0;

  /** 
      returns the detector trimbit/settings directory   
  */
  virtual char* getSettingsDir()=0;

  /** sets the detector trimbit/settings directory  */
  virtual char* setSettingsDir(string s)=0; 

  /**
     returns the location of the calibration files
  */
  virtual char* getCalDir()=0; 

  /**
      sets the location of the calibration files
  */
  virtual char* setCalDir(string s)=0;

  /**
     returns the default output files path
  */
  virtual char* getFilePath()=0;

 /**
     sets the default output files path
     \param s file path
     \returns file path
  */
  virtual char* setFilePath(string s)=0;  

  /**
     returns the default output files root name
  */
  virtual char* getFileName()=0;  

  /**
     sets the default output files path
  */
  virtual char* setFileName(string s)=0;  
  
  /**
     returns the default output file index
  */
  virtual int getFileIndex()=0;
  
  /**
     sets the default output file index
  */
  virtual int setFileIndex(int i)=0;

  /** 
    get flat field corrections file directory
    \returns flat field correction file directory
  */
  virtual char *getFlatFieldCorrectionDir()=0; 
  
  /** 
      set flat field corrections file directory
      \param flat field correction file directory
      \returns flat field correction file directory
  */
  virtual char *setFlatFieldCorrectionDir(string dir)=0;
  
  /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  virtual char *getFlatFieldCorrectionFile()=0;
  
  /** 
      set flat field corrections
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
  */
  virtual int setFlatFieldCorrection(string fname="")=0; 
  
  /** 
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
  */
  virtual int getFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL)=0;
  
  /** 
      set flat field corrections
      \param corr if !=NULL the flat field corrections will be filled with corr (NULL usets ff corrections)
      \param ecorr if !=NULL the flat field correction errors will be filled with ecorr (1 otherwise)
      \returns 0 if ff correction disabled, >0 otherwise
  */
  virtual int setFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL)=0;
  
  /** 
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int setRateCorrection(float t=0)=0;

  /** 
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int getRateCorrection(float &t)=0;
  
  /** 
      get rate correction tau
      \returns 0 if rate correction disabled, otherwise the tau used for the correction
  */
  virtual float getRateCorrectionTau()=0;
  
  /** 
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int getRateCorrection()=0;
  
  /** 
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(string fname="")=0;  
  
  /** 
      set bad channels correction
      \param nch number of bad channels
      \param chs array of channels
      \param ff 0 if normal bad channels, 1 if ff bad channels
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(int nch, int *chs, int ff=0)=0; 
  
  /** 
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  virtual int getBadChannelCorrection(int *bad=NULL)=0;
  
  /** 
      returns the bad channel list file 
  */
  virtual string getBadChannelCorrectionFile()=0;
  
  /** 
      set angular conversion
      \param fname file with angular conversion constants ("" disable)
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  virtual int setAngularConversion(string fname="")=0;
  
  /** 
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  virtual int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL)=0;
  
  /**
     pure virtual function
      returns the angular conversion file 
  */
  virtual string getAngularConversion()=0;


   /**
     sets the value of s angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \param v the value to be set
     \returns the actual value
   */
  
  virtual float setAngularConversionParameter(angleConversionParameter c, float v)=0;

  /**
     returns the value of an angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \returns the actual value
  */
  virtual float getAngularConversionParameter(angleConversionParameter c)=0;

  /**   
	writes a data file
	\param name of the file to be written
	\param data array of data values
	\param err array of arrors on the data. If NULL no errors will be written
	
	\param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
	\param dataformat format of the data: can be 'i' integer or 'f' float (default)
	\param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
	\returns OK or FAIL if it could not write the file or data=NULL
  */
  virtual int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1)=0; 

  /**
     writes an angular conversion file
     \param fname file to be written
     \return OK/FAIL
  */
  virtual int writeAngularConversion(string fname)=0;

  /** 
      set/get if the data processing and file writing should be done by a separate thread - do not use except for debugging!
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
  */
  virtual int setThreadedProcessing(int i=-1)=0;
  
  /** 
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
  virtual int setPositions(int nPos, float *pos)=0;
  
  /** 
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
  virtual int getPositions(float *pos=NULL)=0;
  
 /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  virtual int setActionScript(int iaction, string fname="")=0;

  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param par for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  virtual int setActionParameter(int iaction, string par="")=0;

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
  virtual string getActionScript(int iaction)=0;

  /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
  virtual string getActionParameter(int iaction)=0;

 /** 
      set scan script
      \param index is the scan index (0 or 1)
      \param fname for script ("" disable, "none" disables and overwrites current, "threshold" makes threshold scan, "trimbits" make trimbits scan, "energy" makes energy scan)
      \returns 0 if scan disabled, >0 otherwise
  */
  virtual int setScanScript(int index, string script="")=0;

 /** 
      set scan script parameter
      \param index is the scan index (0 or 1)
      \param spar parameter to be passed to the scan script with syntax par=spar
      \returns 0 if scan disabled, >0 otherwise
  */
  virtual int setScanParameter(int index, string spar="")=0;

 /** 
      set scan precision
      \param index is the scan index (0 or 1)
      \param precision number of decimals to use for the scan variable in the file name
      \returns 0 if scan disabled, >0 otherwise
  */
  virtual int setScanPrecision(int index, int precision=-1)=0; 

  /** 
      set scan steps (passed to the scan script as var=step)
      \param index is the scan index (0 or 1)
      \param nvalues is the number of steps
      \param values array of steps
      \returns 0 if scan disabled, >0 otherwise
  */
  virtual int setScanSteps(int index, int nvalues=-1, float *values=NULL)=0;

   /** 
      get scan script
      \param index is the scan index (0 or 1)
      \returns "none" if disables, "threshold" threshold scan, "trimbits" trimbits scan, "energy"  energy scan or scan script name
  */
  virtual string getScanScript(int index)=0;

     /** 
      get scan script
      \param index is the scan index (0 or 1)
      \returns scan script parameter
  */
  virtual string getScanParameter(int index)=0;

   /** 
      get scan precision
      \param index is the scan index (0 or 1)
      \returns precision i.e. number of decimals to use for the scan variable in the file name
  */
  virtual int getScanPrecision(int index)=0;

     /** 
      get scan steps
      \param index is the scan index (0 or 1)
      \param values pointer to array of values (must be allocated in advance)
      \returns number of steps
  */
  virtual int getScanSteps(int index, float *values=NULL)=0; 

  /**
     gets the network parameters (implemented for gotthard)
     \param i network parameter type can be CLIENT_IP, CLIENT_MAC, SERVER_MAC
     \returns parameter

  */
  virtual char *getNetworkParameter(networkParameter i)=0;

  /**
     sets the network parameters  (implemented for gotthard)
     \param i network parameter type can be CLIENT_IP, CLIENT_MAC, SERVER_MAC
     \param s value to be set
     \returns parameter

  */
  virtual char *setNetworkParameter(networkParameter i, string s)=0; 

/**
     changes/gets the port number
     \param type port type can be CONTROL_PORT, DATA_PORT, STOP_PORT
     \param num new port number (<1024 gets)
     \returns actual port number
  */
  virtual int setPort(portType t, int i=-1)=0; 

  /** Locks/Unlocks the connection to the server
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the server
  */
  virtual int lockServer(int i=-1)=0;  

  /** 
      Returns the IP of the last client connecting to the detector
  */
  virtual string getLastClientIP()=0;



  /**  
     configures mac for gotthard readout
     \returns OK or FAIL
  */

  virtual int configureMAC()=0;


  /** 
      set/get the size of the detector 
      \param n number of modules
      \param d dimension
      \returns current number of modules in direction d
  */
  virtual int setNumberOfModules(int i=-1, dimension d=X)=0;


  /** 
      get the maximum size of the detector 
      \param d dimension
      \returns maximum number of modules that can be installed in direction d
  */
  virtual int getMaxNumberOfModules(dimension d=X)=0;
 

    /** 
      set/get dynamic range
      \param n dynamic range (-1 get)
      \returns current dynamic range
      updates the size of the data expected from the detector
      \sa sharedSlsDetector
  */
  virtual int setDynamicRange(int i=-1)=0;

  /**
    get detector settings
    \param imod module number (-1 all)
    \returns current settings
  */
  virtual detectorSettings getSettings(int imod=-1)=0; 

   /**
    set detector settings
    \param isettings  settings
    \param imod module number (-1 all)
    \returns current settings

    in this function trimbits/settings and calibration files are searched in the settingsDir and calDir directories and the detector is initialized
  */
  virtual detectorSettings setSettings(detectorSettings isettings, int imod=-1)=0;

  /**
    get threshold energy
    \param imod module number (-1 all)
    \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int getThresholdEnergy(int imod=-1)=0;  


  /**
    set threshold energy
    \param e_eV threshold in eV
    \param imod module number (-1 all)
    \param isettings ev. change settings
    \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS)=0;

   /**
     get detector ids/versions for module
     \param mode which id/version has to be read
     \param imod module number for module serial number
     \returns id
  */
  virtual int64_t getId(idMode mode, int imod=0)=0;

  /**
    Digital test of the modules
    \param mode test mode
    \param imod module number for chip test or module firmware test
    \returns OK or error mask
  */
  virtual int digitalTest(digitalTestMode mode, int imod=0)=0;

 /**
       ex cute trimming
     \param mode trim mode
     \param par1 if noise, beam or fixed setting trimming it is count limit, if improve maximum number of iterations
     \param par2 if noise or beam nsigma, if improve par2!=means vthreshold will be optimized, if fixed settings par2<0 trimwith median, par2>=0 trim with level
     \param imod module number (-1 all)
     \returns OK or FAIl (FAIL also if some channel are 0 or 63
  */
  virtual int executeTrimming(trimMode mode, int par1, int par2, int imod=-1)=0;



  /**
     returns currently the loaded trimfile/settingsfile name
  */
  virtual const char *getSettingsFile()=0;

  

  /** loads the modules settings/trimbits reading from a file  
      \param fname file name . If not specified, extension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
 */
  virtual int loadSettingsFile(string fname, int imod=-1)=0;



  /** saves the modules settings/trimbits writing to  a file  
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
 */
  virtual int saveSettingsFile(string fname, int imod=-1)=0;



  /** 
      write  register 
      \param addr address
      \val value
      \returns current register value
      
      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual int writeRegister(int addr, int val)=0; 

 
  /** 
      read  register 
      \param addr address
      \returns current register value

      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual int readRegister(int addr)=0;


  /**
    set dacs value
    \param val value (in V)
    \param index DAC index
    \param imod module number (if -1 alla modules)
    \returns current DAC value
  */
  virtual float setDAC(float , dacIndex, int imod=-1)=0;


  /**
     gets ADC value
     \param index ADC index
     \param imod module number
     \returns current ADC value
  */
  virtual float getADC(dacIndex, int imod=0)=0;


  /** 
      set/get timer value
      \param index timer index
      \param t time in ns or number of...(e.g. frames, gates, probes)
      \returns timer set value in ns or number of...(e.g. frames, gates, probes)
  */
  virtual int64_t setTimer(timerIndex index, int64_t t=-1)=0;


 /** 
      get current timer value
      \param index timer index
      \returns elapsed time value in ns or number of...(e.g. frames, gates, probes)
  */
  virtual int64_t getTimeLeft(timerIndex index)=0;




  /** sets/gets the value of important readout speed parameters
      \param sp is the parameter to be set/get
      \param value is the value to be set, if -1 get value
      \returns current value for the specified parameter
      \sa speedVariable
   */
  virtual int setSpeed(speedVariable sp, int value=-1)=0;



  /** sets the number of trim energies and their value  \sa sharedSlsDetector 
   \param nen number of energies
   \param en array of energies
   \returns number of trim energies

   unused!

  */
  virtual int setTrimEn(int nen, int *en=NULL)=0;

  /** returns the number of trim energies and their value  \sa sharedSlsDetector 
      \param point to the array that will contain the trim energies (in ev)
      \returns number of trim energies

      unused!
  */
  virtual int getTrimEn(int *en=NULL)=0;


  /** 
     set/get the use of an external signal 
      \param pol meaning of the signal \sa externalSignalFlag
      \param signalIndex index of the signal
      \returns current meaning of signal signalIndex
  */
  virtual externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0)=0;

  /**
     set/get readout flags
     \param flag readout flag to be set
     \returns current flag
  */
  virtual int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS)=0;


 /** 
     set/get the external communication mode
     
     obsolete \sa setExternalSignalFlags
      \param pol value to be set \sa externalCommunicationMode
      \returns current external communication mode
  */
  virtual externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE)=0;


  /**
     Reads the configuration file 
     \param fname file name
     \returns OK or FAIL
  */  
  virtual int readConfigurationFile(string const fname)=0;  


  /**  
     Writes the configuration file 
     \param fname file name
     \returns OK or FAIL
  */
  virtual int writeConfigurationFile(string const fname)=0;

  /** 
      Reads the parameters from the detector and writes them to file
      \param fname file to write to
      \param level if 2 reads also trimbits, flat field, angular correction etc. and writes them to files with automatically added extension
      \returns OK or FAIL
  
  */
  virtual int dumpDetectorSetup(string const fname, int level=0)=0; 
  /** 
     Loads the detector setup from file
      \param fname file to read from
      \param level if 2 reads also reads trimbits, angular conversion coefficients etc. from files with default extensions as generated by dumpDetectorSetup
      \returns OK or FAIL
  
  */
  virtual int retrieveDetectorSetup(string const fname, int level=0)=0;


 /**
      Loads dark image or gain image to the detector
      \param index can be DARK_IMAGE or GAIN_IMAGE
      \fname file name to load data from
      \returns OK or FAIL
 */
  virtual int loadImageToDetector(imageType index,string const fname)=0;

  
   /**
      Test function
      \param times number of repetitions
      \returns repetition when it fails
 */
  virtual int testFunction(int times=0)=0;

  /************************************************************************

                           STATIC FUNCTIONS

  *********************************************************************/  

  /** returns string from run status index
      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED
      \returns string error, waiting, running, data, finished
  */
  static string runStatusType(runStatus s){\
    switch (s) {				\
    case ERROR:       return string("error");		\
    case  WAITING:      return  string("waiting");	\
    case RUNNING:      return string("running");\
    case TRANSMITTING:      return string("data");	\
    case  RUN_FINISHED:      return string("finished");	\
    default:       return string("idle");		\
    }};
  
  /** returns detector type string from detector type index
      \param type string can be Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
      \returns MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
  */
  static string getDetectorType(detectorType t){\
    switch (t) {\
    case MYTHEN:    return string("Mythen");	\
    case PILATUS:    return string("Pilatus");	\
    case EIGER:    return string("Eiger");	\
    case GOTTHARD:    return string("Gotthard");	\
    case AGIPD:    return string("Agipd");		\
    default:    return string("Unknown");		\
  }};

  /** returns detector type index from detector type string
      \param t can be MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
      \returns Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
  */
  static detectorType getDetectorType(string const type){\
    if (type=="Mythen")      return MYTHEN;\
    else if  (type=="Pilatus")      return PILATUS;	\
    else if  (type=="Eiger")    return EIGER;		\
    else if  (type=="Gotthard")    return GOTTHARD;	\
    else if  (type=="Agipd")    return AGIPD;		\
    return GENERIC;};



  /** returns synchronization type index from string
      \param t can be none, gating, trigger, complementary
      \returns ONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
  */
  static synchronizationMode getSyncType(string const type){\
    if (type=="none")    return NONE;\
    else if (type=="gating")    return MASTER_GATES;\
    else if (type=="trigger")    return MASTER_TRIGGERS;		\
    else if (type=="complementary") return SLAVE_STARTS_WHEN_MASTER_STOPS; \
    else return GET_SYNCHRONIZATION_MODE;				\
  };

  /** returns synchronization type string from index
      \param t can be NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns none, gating, trigger, complementary
  */
  static string getSyncType(synchronizationMode s ){\
    switch(s) {					    \
    case NONE:    return string("none");	    \
    case MASTER_GATES:    return string("gating");	\
    case MASTER_TRIGGERS:    return string("trigger");			\
    case SLAVE_STARTS_WHEN_MASTER_STOPS:    return string("complementary"); \
    default:    return string("unknown");				\
    }};



  /** returns string from external signal type index
      \param f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE
      \returns string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, unknown
  */
  static string externalSignalType(externalSignalFlag f){\
    switch(f) {						 \
    case SIGNAL_OFF:      return string( "off");			\
    case GATE_IN_ACTIVE_HIGH:    return string( "gate_in_active_high");	\
    case GATE_IN_ACTIVE_LOW:    return string( "gate_in_active_low");	\
    case TRIGGER_IN_RISING_EDGE:    return string( "trigger_in_rising_edge"); \
    case TRIGGER_IN_FALLING_EDGE:    return string( "trigger_in_falling_edge");	\
    case RO_TRIGGER_IN_RISING_EDGE:    return string( "ro_trigger_in_rising_edge"); \
    case RO_TRIGGER_IN_FALLING_EDGE:    return string( "ro_trigger_in_falling_edge"); \
    case GATE_OUT_ACTIVE_HIGH:    return string( "gate_out_active_high"); \
    case GATE_OUT_ACTIVE_LOW:    return string( "gate_out_active_low");	\
    case TRIGGER_OUT_RISING_EDGE:    return string( "trigger_out_rising_edge");	\
    case TRIGGER_OUT_FALLING_EDGE:    return string( "trigger_out_falling_edge"); \
  case RO_TRIGGER_OUT_RISING_EDGE:      return string( "ro_trigger_out_rising_edge");\
    case RO_TRIGGER_OUT_FALLING_EDGE:    return string( "ro_trigger_out_falling_edge");	\
    default:    return string( "unknown");				\
    }    };
  



  /** returns external signal type index from string
      \param string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, unknown
      \returns f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE,GET_EXTERNAL_SIGNAL_FLAG (if unknown)
  */

  static externalSignalFlag externalSignalType(string sval){\
  externalSignalFlag flag=GET_EXTERNAL_SIGNAL_FLAG;\
  if (sval=="off")      flag=SIGNAL_OFF;\
  else if (sval=="gate_in_active_high")      flag=GATE_IN_ACTIVE_HIGH;	\
  else if  (sval=="gate_in_active_low") flag=GATE_IN_ACTIVE_LOW;\
  else if  (sval=="trigger_in_rising_edge") flag=TRIGGER_IN_RISING_EDGE;\
  else if  (sval=="trigger_in_falling_edge") flag=TRIGGER_IN_FALLING_EDGE;\
  else if  (sval=="ro_trigger_in_rising_edge") flag=RO_TRIGGER_IN_RISING_EDGE;\
  else if  (sval=="ro_trigger_in_falling_edge") flag=RO_TRIGGER_IN_FALLING_EDGE;\
  else if (sval=="gate_out_active_high")      flag=GATE_OUT_ACTIVE_HIGH;\
  else if  (sval=="gate_out_active_low") flag=GATE_OUT_ACTIVE_LOW;\
  else if  (sval=="trigger_out_rising_edge") flag=TRIGGER_OUT_RISING_EDGE;\
  else if  (sval=="trigger_out_falling_edge") flag=TRIGGER_OUT_FALLING_EDGE;\
  else if  (sval=="ro_trigger_out_rising_edge") flag=RO_TRIGGER_OUT_RISING_EDGE;\
  else if  (sval=="ro_trigger_out_falling_edge") flag=RO_TRIGGER_OUT_FALLING_EDGE;\
  return flag;};


  /** returns synchronization type string from index
      \param t can be NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns none, gating, trigger, complementary
  */
  static detectorSettings getDetectorSettings(string s){\
    if (s=="standard") return STANDARD;\
    if (s=="fast") return FAST;\
    if (s=="highgain") return HIGHGAIN;		\
    if (s=="dynamicgain") return DYNAMICGAIN;	\
    if (s=="lowgain") return LOWGAIN;		\
    if (s=="mediumgain") return MEDIUMGAIN;	\
    if (s=="veryhighgain") return VERYHIGHGAIN;	\
    return GET_SETTINGS;\
  };

  /** returns detector settings string from index
      \param t can be STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, GET_SETTINGS
      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, undefined
  */
  static string getDetectorSettings(detectorSettings s){\
    switch(s) {\
    case STANDARD:      return string("standard");\
    case FAST:      return string("fast");\
    case HIGHGAIN:      return string("highgain");\
    case DYNAMICGAIN:    return string("dynamicgain");	\
    case LOWGAIN:    return string("lowgain");		\
    case MEDIUMGAIN:    return string("mediumgain");	\
    case VERYHIGHGAIN:    return string("veryhighgain");	\
    default:    return string("undefined");			\
    }};

  

};
#endif
